/* Copyright 1996 Michael Messollen. All rights reserved. */
///////////////////////////////////////////////////////////////////////////////
// emacs edit mode for this file is -*- C++ -*-
//static char * rcsid = "@(#) $Id: Truefactor.cc,v 1.5 2001-06-21 14:57:06 Singular Exp $";
///////////////////////////////////////////////////////////////////////////////
// Factory - Includes
#include <factory.h>
// Factor - Includes
#include "tmpl_inst.h"
#include "helpstuff.h"
// some CC's need this:
#include "Truefactor.h"

#ifdef SINGULAR
#  define HAVE_SINGULAR
   extern "C" { void WerrorS(char *); }
#endif

#ifdef TRUEFACTORDEBUG
#  define DEBUGOUTPUT
#else
#  undef DEBUGOUTPUT
#endif

#include "debug.h"
#include "timing.h"

int hasAlgVar(const CanonicalForm &f, const Variable &v)
{
  if (f.inBaseDomain()) return 0;
  if (f.inCoeffDomain())
  {
    if (f.mvar()==v) return 1;
    return hasAlgVar(f.LC(),v);
  }
  if (f.inPolyDomain())
  {
    if (hasAlgVar(f.LC(),v)) return 1;
    for( CFIterator i=f; i.hasTerms(); i++)
    {
      if (hasAlgVar(i.coeff(),v)) return 1;
    }
  }  
  return 0;
}

int hasAlgVar(const CanonicalForm &f)
{
  if (f.inBaseDomain()) return 0;
  if (f.inCoeffDomain())
  {
    if (f.level()!=0) 
    {
      //cout << "hasAlgVar:" << f.mvar() <<endl;
      return 1;
    }  
    return hasAlgVar(f.LC());
  }
  if (f.inPolyDomain())
  {
    if (hasAlgVar(f.LC())) return 1;
    for( CFIterator i=f; i.hasTerms(); i++)
    {
      if (hasAlgVar(i.coeff())) return 1;
    }
  }  
  return 0;
}

///////////////////////////////////////////////////////////////
// generate all different k-subsets of the set with n        //
// elements and return them in returnlist.                   //
///////////////////////////////////////////////////////////////
static void 
combinat( int k, int n, List<IntList> & returnlist ){
  ListIntList ListofLists;
  IntList intermediate,intermediate2;
  int value,j;

  if ( k == 1 ){                     // k=1
    for ( j=1; j<=n; j++)
      returnlist.append( IntList(j) );
  }
  else{                              // k-1 --> k
    combinat(k-1,n,returnlist);  // generate (k-1,n)
    for ( ListIntListIterator l=returnlist; l.hasItem(); l++ ){
      intermediate = l.getItem();
      value = intermediate.getLast();
      if ( value != n )
	for ( j=value+1; j<=n; j++ ){
	  intermediate2 = intermediate; intermediate2.append(j);
	  ListofLists.append( intermediate2 );
	}
    }
    returnlist = ListofLists;
  }
}

///////////////////////////////////////////////////////////////
// Return the CanonicalForm number nr in  Factorlist.        //
///////////////////////////////////////////////////////////////
static CanonicalForm 
getItemNr(int nr, const CFFList & Factorlist ){
  ListIterator<CFFactor> i=Factorlist;
  int Nr=nr;

  for ( Nr=1; Nr<nr; Nr++ ) i++;
  return i.getItem().factor();
}

///////////////////////////////////////////////////////////////
// Generate all sets of m factors out of LiftedFactors list. //
///////////////////////////////////////////////////////////////
static CFFList 
combine( int m, const CFFList & LiftedFactors ){
  CFFList result;
  ListIntList CombinatList;
  CanonicalForm intermediate;

  combinat(m, LiftedFactors.length(), CombinatList);
  for ( ListIntListIterator j=CombinatList ; j.hasItem(); j++ ){
    intermediate=1;
    for ( IntListIterator k=j.getItem(); k.hasItem(); k++ )
      intermediate *= getItemNr(k.getItem(), LiftedFactors);
    if (!hasAlgVar(intermediate))    
    result.append(CFFactor(intermediate,1));
  }
  return result;
}

///////////////////////////////////////////////////////////////
// Remove element elem from the list L.                      //
///////////////////////////////////////////////////////////////
static CFFList 
Remove_from_List( const CFFList & L, const CanonicalForm & elem ){
  CFFList Returnlist;

  DEBOUTLN(cout, "Remove_from_List called with L= ",L);
  DEBOUTLN(cout, "                     and  elem= ",elem);
  for ( ListIterator<CFFactor> i = L ; i.hasItem(); i++)
    if ( i.getItem().factor() != elem ) 
      Returnlist.append( i.getItem() );

  return Returnlist;
}

///////////////////////////////////////////////////////////////
// Here we solve:          G= F mod ( P, S^h )               //
///////////////////////////////////////////////////////////////
static CanonicalForm 
Multmod_power( const CanonicalForm & F, const SFormList & Substituionlist, int h, int levelF){
  CanonicalForm G;

  G= change_poly(F, Substituionlist, 0);
  G= mod_power(G, h, levelF);
  G= change_poly(G, Substituionlist, 1);

  return G;
}

///////////////////////////////////////////////////////////////
// Determine the right degree for the list of combinations   //
// of factors, i.e. delete any element from list CombL which //
// degree in the main variable levelU exceeeds degU.         //
///////////////////////////////////////////////////////////////
static CFFList 
Rightdegree( const CFFList & CombL, int degU, int levelU ){
  CFFList Returnlist;
  CFFactor factor;

  for ( ListIterator<CFFactor> i= CombL; i.hasItem(); i++ ){
    factor= i.getItem();
    if ( degree(factor.factor(), levelU) <= degU )
      Returnlist.append(factor);
  }

  return Returnlist;
}

///////////////////////////////////////////////////////////////
// We have properly lifted all the specialized factors. See  //
// which one works.                                          //
// We use the (modified) algorithm TRUEFACTORS given by      //
// Paul S. Wang and Linda Preiss Rothschild:                 //
// Factoring Multivariate Polynomials Over the Integers      //
// Math. Comp. V29 Nr131 (July 1975) p. 935-950              //
///////////////////////////////////////////////////////////////
CFFList 
Truefactors( const CanonicalForm Ua, int levelU, const SFormList & SubstitutionList, const CFFList & PiList){
  CanonicalForm U=Ua,a,b,Y;
  CFFactor factor;
  CFFList L,FAC,E_all;
  int c,r = PiList.length(),degU, onemore,M, h = subvardegree(Ua,levelU) + 1;
  ListIterator<CFFactor> i;

  //cout << "SubstitutionList="<< SubstitutionList<<endl;
// step 1: simply test the generated factors alone
  for ( i= PiList; i.hasItem();i++){
    factor = i.getItem();
    //CanonicalForm test_f=change_poly(factor.factor(),SubstitutionList,0);
    CanonicalForm test_f=factor.factor();
    //cout <<"f:" << factor.factor() << " -> test_f:"<<test_f <<endl;
    //cout << "           1:" << change_poly(factor.factor(),SubstitutionList,1) <<endl;
    c= mydivremt(U,test_f,a,b);
    if (  c  && b == U.genZero() && !hasAlgVar(test_f)) // factor.getFactor() divides U
    {
      //cout << " teilt:" << test_f <<endl;
      U= a;
      FAC.append(factor);
    }
    else{
      //cout << " teilt nicht:" << test_f <<endl;
      L.append(factor);
    }
  }
  DEBOUTLN(cout,"Truefactors: (step1) L  = ", L);
  DEBOUTLN(cout,"                     FAC= ", FAC);

// step 2: Do we have to check combinations?
  degU = L.length();
  if ( degU == 0 ) // No elements: Return
    return FAC;
  else
    if ( degU < 4 ){ // Less then four elements: no combinations possible
      FAC.append( CFFactor(U,1) );
      return FAC;
    }
    else {
      M = 1; r = r - FAC.length(); degU = degree(U, levelU)/2;
    }

  DEBOUTLN(cout,"Truefactors: (step2) M   = ", M);
  DEBOUTLN(cout,"                     r   = ", r);
  DEBOUTLN(cout,"                     degU= ", degU);
  
// Now do the real work!
// Test all the combinations of possible factors. 

  onemore=1;
// steps 3 to 6
  while (1){
    // step 3 iff onemore == 1
    if ( onemore ) M+= 1;  else onemore = 1;
    // step 4
    if ( U == U.genOne() ) break; // Return FAC
    if ( ( r == 1 ) || ( M >= ( r-1 ) ) || ( M > degU ) ) {
      FAC.append( CFFactor(U,1) );
      break; // Return FAC union {U}
    }
    // step 5
    E_all = combine(M, L); // generate all combinations of M elements from L
    DEBOUTLN(cout,"Truefactors: (step5) E_all= ", E_all);
    // select combinations with the degree not to exceed degU:
    E_all = Rightdegree( E_all, degU, levelU );
    DEBOUTLN(cout,"Truefactors: (step5) E_all(Rightdegree)= ", E_all);
    if ( E_all.length() == 0 ){
      FAC.append( CFFactor(U,1) );
      break; // Return FAC union {U}
    }
    for ( i=E_all; i.hasItem(); i++){
      factor = i.getItem();
      Y = Multmod_power( factor.factor(), SubstitutionList, h, levelU);
      DEBOUTLN(cout, "Truefactors: (step6) Testing Y  = ", Y);
      c = mydivremt(U,Y,a,b);
      //      if (  c  && b == U.genZero()) { // Y divides U
      if ( c && b.isZero() ){
	DEBOUT(cout,"Truefactors: (step6): ",Y );
	DEBOUTLN(cout, "  divides  ",U);
	U = a;
	FAC.append(Y); // Y is a real factor
	onemore = 0;
	degU = degree(U, levelU)/2; // new degU
	// L = L \ {factor}
	// Hier ist noch etwas faul; wir muessen (f=prod(f_i)) die f_i 
	// entfernen und nicht f!
	L = Remove_from_List( L, factor.factor() );
	r -= 1;
	// delete from L any element with degree greater than degU
	L = Rightdegree( L, degU, levelU );
      }
    }
  }

  return FAC;
}

///////////////////////////////////////////////////////////////
// Check if poly f is in Fp (returns true) or in Fp(a)       //
///////////////////////////////////////////////////////////////
static bool 
is_in_Fp( const CanonicalForm & f ){
  if ( f.inCoeffDomain() )
    return f.inBaseDomain() ;
  else {
    CFIterator i=f;
    bool ok=true;
    while ( ok && i.hasTerms() ){
      ok = is_in_Fp( i.coeff() );
      i++ ;
    }
    return ok;
  }
}

///////////////////////////////////////////////////////////////
// We have factors which possibly lie in an extension of the //
// base field. If one of these is not over the base field,   //
// find its norm by (the theoretically handicapped method    //
// of) multiplying by other elements.                        //
///////////////////////////////////////////////////////////////
CFFList
TakeNorms(const CFFList & PiList){
  CFFList CopyPossibleFactors, PossibleFactors, TrueFactors;
  CFFListIterator i;
  CFFactor Factor;
  CanonicalForm intermediate;
  ListIntList CombinatList;
  ListIntListIterator j;
  IntListIterator k;

  // First check if the factors in PiList already lie in Fp-Domain
  for ( i=PiList; i.hasItem(); i++ ){
    Factor = i.getItem();
    if ( is_in_Fp( Factor.factor() ) )
      TrueFactors.append(Factor);
    else
      PossibleFactors.append(Factor);
  }
  // Now we have to check if combinations of the remaining factors
  // (now in PossibleFactors) do lie in Fp-Domain
  if ( PossibleFactors.length() > 0 ){ // there are (at least two) items
    int n=2;
    if ( PossibleFactors.length() < n ) { // a little check
#ifdef HAVE_SINGULAR
      WerrorS("libfac: ERROR: TakeNorms less then two items remaining!");
#else
      cerr << "libfac: ERROR: TakeNorms less then two items remaining! " 
	   << endl;
#endif
    }
    while ( n < PossibleFactors.length() ){
      // generate all combinations of n elements
      combinat(n, PossibleFactors.length(), CombinatList);
      for ( j=CombinatList ; j.hasItem(); j++ ){
	intermediate=1;
	for ( k=j.getItem(); k.hasItem(); k++ )
	  intermediate *= getItemNr( k.getItem(), PossibleFactors );
	if ( is_in_Fp( intermediate ) ){ 
	  TrueFactors.append(intermediate); // found a true factor
	  CopyPossibleFactors=PossibleFactors; // save list
	  for ( k=j.getItem(); k.hasItem(); k++ ) 
	    //remove combined factors from PossibleFactors
	    PossibleFactors=Remove_from_List(PossibleFactors,
				getItemNr( k.getItem(), CopyPossibleFactors ));
	  n-=1; // look for the same number of combined factors:
	  break;
	}
	else { 
	  //cout << "Schade!" << endl; 
	}
	DEBOUT(cout, "Truefactor: Combined ", n);
	DEBOUTLN(cout, " factors to: ", intermediate);
      }
      n += 1;
    }
  // All remaining factors in PossibleFactors multiplied 
  // should lie in Fp domain
    if ( PossibleFactors.length() >=1 ){
      for ( i=PossibleFactors; i.hasItem(); i++ )
	intermediate *= i.getItem().factor();
      // a last check:
      if ( is_in_Fp(intermediate) ){
	TrueFactors.append(CFFactor(intermediate,1));
      }
      else{ 
#ifdef HAVE_SINGULAR
	WerrorS("libfac: TakeNorms: somethings wrong with remaining factors!");
#else
	cerr << "libfac: TakeNorms: somethings wrong with remaining factors!" 
	     << endl;
#endif
      }
    }
  }
  return TrueFactors;
}

////////////////////////////////////////////////////////////
/*
$Log: not supported by cvs2svn $
Revision 1.4  1997/11/18 16:39:07  Singular
* hannes: moved WerrorS from C++ to C
     (Factor.cc MVMultiHensel.cc SqrFree.cc Truefactor.cc)

Revision 1.3  1997/09/12 07:19:52  Singular
* hannes/michael: libfac-0.3.0

Revision 1.3  1997/04/25 22:39:11  michael
changed cerr and cout messages for use with Singular
Version for libfac-0.2.1

*/
