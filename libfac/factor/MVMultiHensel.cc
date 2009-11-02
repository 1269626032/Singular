///////////////////////////////////////////////////////////////////////////////
// emacs edit mode for this file is -*- C++ -*-
// $Id$
///////////////////////////////////////////////////////////////////////////////
// FACTORY - Includes
#include <factory.h>
#ifndef NOSTREAMIO
#ifdef HAVE_IOSTREAM
#include <iostream>
#define OSTREAM std::ostream
#define ISTREAM std::istream
#define CERR std::cerr
#define CIN std::cin
#elif defined(HAVE_IOSTREAM_H)
#include <iostream.h>
#define OSTREAM ostream
#define ISTREAM istream
#define CERR cerr
#define CIN cin
#endif
#endif
// Factor - Includes
#include "tmpl_inst.h"
#include "helpstuff.h"
// some CC's need this:
#include "MVMultiHensel.h"

#ifdef SINGULAR
#define HAVE_SINGULAR_ERROR
#ifndef NOSTREAMIO
void out_cf(char *s1,const CanonicalForm &f,char *s2);
#endif
#endif

#ifdef HAVE_SINGULAR_ERROR
extern int libfac_interruptflag;
   extern "C"
   {
     void WerrorS(const char *);
     void Werror(const char *fmt, ...);
   }
#endif

#ifdef HENSELDEBUG
#  define DEBUGOUTPUT
#else
#  undef DEBUGOUTPUT
#endif

#include "debug.h"
#include "interrupt.h"
#include "timing.h"

///////////////////////////////////////////////////////////////
// some class definition needed in MVMultiHensel
///////////////////////////////////////////////////////////////
typedef bool Boolean;

class DiophantForm
{
 public:
   CanonicalForm One;
   CanonicalForm Two;
   inline DiophantForm& operator=( const DiophantForm&  value )
   {
     if ( this != &value )
     {
       One = value.One;
       Two = value.Two;
     }
     return *this;
   }
};

// We remember an already calculated value; simple class for RememberArray
class RememberForm
{
public:
  inline RememberForm operator=( CanonicalForm & value )
  {
    this->calculated = true;
    this->poly = value;
    return *this;
  }
  RememberForm() : poly(0), calculated(false) {}
  Boolean calculated;
  CanonicalForm poly;
};

// Array to remember already calculated values; used for the diophantine
// equation s*f + t*g = x^i
class RememberArray
{
public:
// operations performed on arrays
  RememberArray( int sz )
  {
    size = sz;
    ia = new RememberForm[size];
//    assert( ia != 0 ); // test if we got the memory
    init( sz );
  }
  ~RememberArray(){ delete [] ia; }
  inline RememberForm& operator[]( int index )
  {
    return ia[index];
  }
  bool checksize(int i) {return i<size;}
protected:
  void init( int )
  {
    for ( int ix=0; ix < size; ++ix)
    {
      ia[ix].calculated=false;
      ia[ix].poly=0;
    }
  }
// internal data representation
  int size;
  RememberForm *ia;
};

///////////////////////////////////////////////////////////////
// Solve the Diophantine equation: ( levelU == mainvar )     //
//            s*F1 + t*F2 = (mainvar)^i                      //
// Returns s and t.                                          //
///////////////////////////////////////////////////////////////
static DiophantForm
diophant( int levelU , const CanonicalForm & F1 , const CanonicalForm & F2 ,
          int i , RememberArray & A, RememberArray & B,
          const CanonicalForm &alpha )
{
  DiophantForm Retvalue;
  CanonicalForm s,t,q,r;
  Variable x(levelU);

  DEBOUT(CERR, "diophant: called with: ", F1);
  DEBOUT(CERR, "  ", F2); DEBOUTLN(CERR, "  ", i);

  // Did we solve the diophantine equation yet?
  // If so, return the calculated values
  if (A.checksize(i) && A[i].calculated && B[i].calculated )
  {
    Retvalue.One=A[i].poly;
    Retvalue.Two=B[i].poly;
    return Retvalue;
  }

  // Degrees ok? degree(F1,mainvar) + degree(F2,mainvar) <= i ?
  if ( (degree(F1,levelU) + degree(F2,levelU) ) <= i )
  {
#ifdef HAVE_SINGULAR_ERROR
    if (!interrupt_handle()) Werror("libfac: diophant ERROR: degree too large!  (%d + %d <= %d)",degree(F1,levelU), degree(F2,levelU), i);
      //out_cf("F1:",F1,"\n");
      //out_cf("F2:",F2,"\n");
#else
#ifndef NOSTREAMIO
    CERR << "libfac: diophant ERROR: degree too large!  "
         << (degree(F1,levelU) + degree(F2,levelU) ) <<"\n";
#else
    ;
#endif
#endif
    Retvalue.One=F1;Retvalue.Two=F2;
    return Retvalue;
  }

  if ( i == 0 )
  { // call the extended gcd
    r=extgcd(F1,F2,s,t);
    // check if gcd(F1,F2) <> 1 , i.e. F1 and F2 are not relatively prime
    if ( ! r.isOne() )
    {
      if (r.degree()<1) // some constant != 1 ?
      {
        Retvalue.One=s/r;Retvalue.Two=t/r;
        return Retvalue;
      }
      else if (alpha!=0)
      {
        Variable Alpha=alpha.mvar();
        if (r.mvar()==Alpha)   // from field extension ?
        {
          Variable X=rootOf(alpha);
          r=replacevar(r,Alpha,X);
          s=replacevar(s,Alpha,X);
          t=replacevar(t,Alpha,X);
          s/=r;
          t/=r;
          s=replacevar(s,X,Alpha);
          t=replacevar(t,X,Alpha);
          Retvalue.One=s; Retvalue.Two=t;
          return Retvalue;
        }
      } 
#ifdef HAVE_SINGULAR_ERROR
      if (!interrupt_handle()) WerrorS("libfac: diophant ERROR: F1 and F2 are not relatively prime! ");
#ifndef NOSTREAMIO
      out_cf("F1:",F1,"\n");
      out_cf("F2:",F2,"\n");
      out_cf("gcd:",r,"\n");
#endif
#else
#ifndef NOSTREAMIO
      CERR << "libfac: diophant ERROR: " << F1 << "  and  " << F2
           << "  are not relatively prime!" << "\n";
#else
     ;
#endif
#endif
      Retvalue.One=s/r;Retvalue.Two=t/r;
      return Retvalue;
    }
    Retvalue.One = s; Retvalue.Two = t;
  }
  else
  { // recursively call diophant
    Retvalue=diophant(levelU,F1,F2,i-1,A,B,alpha);
    Retvalue.One *= x; // createVar(levelU,1);
    Retvalue.Two *= x; // createVar(levelU,1);

    if (interrupt_handle()) return Retvalue;

    // Check degrees.

    if ( degree(Retvalue.One,levelU) > degree(F2,levelU) )
    {
      // Make degree(Retvalue.one,mainvar) < degree(F2,mainvar)
      divrem(Retvalue.One,F2,q,r);
      Retvalue.One = r; Retvalue.Two += F1*q;
    }
    else
    {
      if ( degree(Retvalue.Two,levelU) >= degree(F1,levelU) )
      {
        // Make degree(Retvalue.Two,mainvar) <= degree(F1,mainvar)
        divrem(Retvalue.Two,F1,q,r);
        Retvalue.One += F2*q; Retvalue.Two = r;
      }
    }

  }
  if (A.checksize(i))
  {
    A[i].poly = Retvalue.One ;
    B[i].poly = Retvalue.Two ;
    A[i].calculated = true ; B[i].calculated = true ;
  }
  DEBOUT(CERR, "diophant: Returnvalue is: ", Retvalue.One);
  DEBOUTLN(CERR, "  ", Retvalue.Two);

  return  Retvalue;
}

///////////////////////////////////////////////////////////////
// A more efficient way to solve s*F1 + t*F2 = W             //
// as in Wang and Rothschild [Wang&Roth75].                  //
///////////////////////////////////////////////////////////////
static CanonicalForm
make_delta( int levelU, const CanonicalForm & W,
            const CanonicalForm & F1, const CanonicalForm & F2,
            RememberArray & A, RememberArray & B,
            const CanonicalForm &alpha)
{
  CanonicalForm Retvalue;
  DiophantForm intermediate;

  DEBOUT(CERR, "make_delta: W= ", W);
  DEBOUTLN(CERR, "  degree(W,levelU)= ", degree(W,levelU) );

  if ( levelU == level(W) ) // same level, good
  {
    for ( CFIterator i=W; i.hasTerms(); i++)
    {
      intermediate=diophant(levelU,F1,F2,i.exp(),A,B,alpha);
      Retvalue += intermediate.One * i.coeff();

      if (interrupt_handle()) return Retvalue;
    }
  }
  else // level(W) < levelU ; i.e. degree(w,levelU) == 0
  {
    intermediate=diophant(levelU,F1,F2,0,A,B,alpha);
    Retvalue = W * intermediate.One;
  }
  DEBOUTLN(CERR, "make_delta: Returnvalue= ", Retvalue);
  return Retvalue;
}

static CanonicalForm
make_square( int levelU, const CanonicalForm & W,
             const CanonicalForm & F1, const CanonicalForm & F2,
             RememberArray & A, RememberArray & B,const CanonicalForm &alpha)
{
  CanonicalForm Retvalue;
  DiophantForm intermediate;

  DEBOUT(CERR, "make_square: W= ", W );
  DEBOUTLN(CERR, "  degree(W,levelU)= ", degree(W,levelU));

  if ( levelU == level(W) ) // same level, good
  {
    for ( CFIterator i=W; i.hasTerms(); i++)
    {
      intermediate=diophant(levelU,F1,F2,i.exp(),A,B,alpha);
      Retvalue += i.coeff() * intermediate.Two;

      if (interrupt_handle()) return Retvalue;
    }
  }
  else // level(W) < levelU ; i.e. degree(w,levelU) == 0
  {
    intermediate=diophant(levelU,F1,F2,0,A,B,alpha);
    Retvalue = W * intermediate.Two;
  }
  DEBOUTLN(CERR, "make_square: Returnvalue= ", Retvalue);

  return Retvalue;
}


///////////////////////////////////////////////////////////////
// Multivariat Hensel routine for two factors F and G .      //
// U is the monic univariat polynomial; we manage two arrays //
// to remember already calculated values for the diophantine //
// equation. This is suggested by Joel Moses [Moses71] .     //
// Return the fully lifted factors.                          //
///////////////////////////////////////////////////////////////
static DiophantForm
mvhensel( const CanonicalForm & U , const CanonicalForm & F ,
          const CanonicalForm & G , const SFormList & Substitutionlist,
          const CanonicalForm &alpha)
{
  CanonicalForm V,Fk=F,Gk=G,Rk,W,D,S;
  int  levelU=level(U), degU=subvardegree(U,levelU); // degree(U,{x_1,..,x_(level(U)-1)})
  DiophantForm Retvalue;
  RememberArray A(degree(F,levelU)+degree(G,levelU)+1);
  RememberArray B(degree(F,levelU)+degree(G,levelU)+1);

  DEBOUTLN(CERR, "mvhensel called with: U= ", U);
  DEBOUTLN(CERR, "                      F= ", F);
  DEBOUTLN(CERR, "                      G= ", G);
  DEBOUTLN(CERR, "                   degU= ", degU);

  V=change_poly(U,Substitutionlist,0); // change x_i <- x_i + a_i for all i
  Rk = F*G-V;
#ifdef HENSELDEBUG2
  CERR << "mvhensel: V = " << V << "\n"
       << "          Fk= " << F << "\n"
       << "          Gk= " << G << "\n"
       << "          Rk= " << Rk << "\n";
#endif
  for ( int k=2; k<=degU+1; k++)
  {
    W = mod_power(Rk,k,levelU);
#ifdef HENSELDEBUG2
    CERR << "mvhensel: Iteration: " << k << "\n";
    CERR << "mvhensel: W= " << W << "\n";
#endif
    D = make_delta(levelU,W,F,G,A,B,alpha);
#ifdef HENSELDEBUG2
    CERR << "mvhensel: D= " << D << "\n";
#endif
    S = make_square(levelU,W,F,G,A,B,alpha);
#ifdef HENSELDEBUG2
    CERR << "mvhensel: S= " << S << "\n";
#endif
    Rk += S*D - D*Fk - S*Gk;
#ifdef HENSELDEBUG2
    CERR << "mvhensel: Rk= " << Rk << "\n";
#endif
    Fk -= S;
#ifdef HENSELDEBUG2
    CERR << "mvhensel: Fk= " << Fk << "\n";
#endif
    Gk -= D;
#ifdef HENSELDEBUG2
    CERR << "mvhensel: Gk= " << Gk << "\n";
#endif
    if ( Rk.isZero() ) break;
    if (interrupt_handle()) break;
  }
  Retvalue.One = change_poly(Fk,Substitutionlist,1);
  Retvalue.Two = change_poly(Gk,Substitutionlist,1);

  DEBOUTLN(CERR, "mvhensel: Retvalue: ", Retvalue.One);
  DEBOUTLN(CERR, "                    ", Retvalue.Two);

  return Retvalue ;
}

///////////////////////////////////////////////////////////////
// Recursive Version of MultiHensel.                         //
///////////////////////////////////////////////////////////////
CFFList
multihensel( const CanonicalForm & mF, const CFFList & Factorlist,
             const SFormList & Substitutionlist,
             const CanonicalForm &alpha)
{
  CFFList Returnlist,factorlist=Factorlist;
  DiophantForm intermediat;
  CanonicalForm Pl,Pr;
  int n = factorlist.length();

  DEBOUT(CERR, "multihensel: called with ", mF);
  DEBOUTLN(CERR, "  ", factorlist);

  if ( n == 1 )
  {
    Returnlist.append(CFFactor(mF,1));
  }
  else
  {
    if ( n == 2 )
    {
      intermediat= mvhensel(mF, factorlist.getFirst().factor(),
                            Factorlist.getLast().factor(),
                            Substitutionlist,alpha);
      Returnlist.append(CFFactor(intermediat.One,1));
      Returnlist.append(CFFactor(intermediat.Two,1));
    }
    else  // more then two factors
    {
#ifdef HENSELDEBUG2
      CERR << "multihensel: more than two factors!" << "\n";
#endif
      Pl=factorlist.getFirst().factor(); factorlist.removeFirst();
      Pr=Pl.genOne();
      for ( ListIterator<CFFactor> i=factorlist; i.hasItem(); i++ )
        Pr *=  i.getItem().factor() ;
#ifdef HENSELDEBUG2
      CERR << "multihensel: Pl,Pr, factorlist: " << Pl << "  " << Pr
           << "  " << factorlist << "\n";
#endif
      intermediat= mvhensel(mF,Pl,Pr,Substitutionlist,alpha);
      Returnlist.append(CFFactor(intermediat.One,1));
      Returnlist=Union( multihensel(intermediat.Two,factorlist,Substitutionlist,alpha), 
                        Returnlist);
    }
  }
  return Returnlist;
}

///////////////////////////////////////////////////////////////
// Generalized Hensel routine.                               //
// mF is the monic multivariat polynomial, Factorlist is the //
// list of factors, Substitutionlist represents the ideal    //
// <x_1+a_1, .. , x_r+a_r>, where r=level(mF)-1 .            //
// Returns the list of fully lifted factors.                 //
///////////////////////////////////////////////////////////////
CFFList
MultiHensel( const CanonicalForm & mF, const CFFList & Factorlist,
             const SFormList & Substitutionlist, const CanonicalForm &alpha)
{
  CFFList Ll;
  CFFList Returnlist,Retlistinter,factorlist=Factorlist;
  CFFListIterator i;
  DiophantForm intermediat;
  CanonicalForm Pl,Pr;
  int n = factorlist.length(),h=n/2, k;

  DEBOUT(CERR, "MultiHensel: called with ", mF);
  DEBOUTLN(CERR, "  ", factorlist);
  DEBOUT(CERR,"           : n,h = ", n);
  DEBOUTLN(CERR,"  ", h);

  if ( n == 1 )
  {
    Returnlist.append(CFFactor(mF,1));
  }
  else
  {
    if ( n == 2 )
    {
      intermediat= mvhensel(mF, factorlist.getFirst().factor(),
                            Factorlist.getLast().factor(),
                            Substitutionlist,alpha);
      Returnlist.append(CFFactor(intermediat.One,1));
      Returnlist.append(CFFactor(intermediat.Two,1));
    }
    else  // more then two factors
    {
      for ( k=1 ; k<=h; k++)
      {
        Ll.append(factorlist.getFirst());
        factorlist.removeFirst();
      }

      DEBOUTLN(CERR, "MultiHensel: Ll= ", Ll);
      DEBOUTLN(CERR, "     factorlist= ", factorlist);

      Pl = 1; Pr = 1;
      for ( i = Ll; i.hasItem(); i++)
        Pl *= i.getItem().factor();
      DEBOUTLN(CERR, "MultiHensel: Pl= ", Pl);
      for ( i = factorlist; i.hasItem(); i++)
        Pr *= i.getItem().factor();
      DEBOUTLN(CERR, "MultiHensel: Pr= ", Pr);
      intermediat = mvhensel(mF,Pl,Pr,Substitutionlist,alpha);
      // divison test for intermediat.One and intermediat.Two ?
      CanonicalForm a,b;
      // we add a division test now for intermediat.One and intermediat.Two
      if ( mydivremt (mF,intermediat.One,a,b) && b == mF.genZero() )
        Retlistinter.append(CFFactor(intermediat.One,1) );
      if ( mydivremt (mF,intermediat.Two,a,b) && b == mF.genZero() )
        Retlistinter.append(CFFactor(intermediat.Two,1)  );

      Ll = MultiHensel(intermediat.One, Ll, Substitutionlist,alpha);
      Returnlist = MultiHensel(intermediat.Two, factorlist, Substitutionlist,alpha);
      Returnlist = Union(Returnlist,Ll);

      Returnlist = Union(Retlistinter,Returnlist);

    }
  }
  return Returnlist;
}

/*
$Log: not supported by cvs2svn $
Revision 1.18  2008/03/18 17:46:15  Singular
*hannes: gcc 4.2

Revision 1.17  2008/03/17 17:44:16  Singular
*hannes: fact.tst

Revision 1.14  2007/06/14 14:16:35  Singular
*hannes: Factorize2 etc.

Revision 1.13  2007/05/25 16:02:02  Singular
*hannes: removed diophant_error, format

Revision 1.12  2007/05/22 14:30:53  Singular
*hannes: diophant_error

Revision 1.11  2007/05/21 16:40:12  Singular
*hannes: Factorize2

Revision 1.10  2007/05/15 14:46:49  Singular
*hannes: factorize in Zp(a)[x...]

Revision 1.9  2006/05/16 14:46:49  Singular
*hannes: gcc 4.1 fixes

Revision 1.8  2002/07/30 15:11:19  Singular
*hannes: minor cleanups

Revision 1.7  2001/08/22 14:21:17  Singular
*hannes: added search for main var to Factorize

Revision 1.6  2001/08/08 14:26:55  Singular
*hannes: Dan's HAVE_SINGULAR_ERROR

Revision 1.5  2001/08/08 11:59:12  Singular
*hannes: Dan's NOSTREAMIO changes

Revision 1.4  1997/11/18 16:39:05  Singular
* hannes: moved WerrorS from C++ to C
     (Factor.cc MVMultiHensel.cc SqrFree.cc Truefactor.cc)

Revision 1.3  1997/09/12 07:19:48  Singular
* hannes/michael: libfac-0.3.0

Revision 1.4  1997/04/25 22:40:02  michael
changed cerr and cout messages for use with Singular
Version for libfac-0.2.1

*/
