/* Copyright 1996 Michael Messollen. All rights reserved. */
////////////////////////////////////////////////////////////
// emacs edit mode for this file is -*- C++ -*-
static char * rcsid = "$Id: csutil.cc,v 1.1.1.1 1997-05-02 17:00:44 Singular Exp $";
/////////////////////////////////////////////////////////////
// FACTORY - Includes
#include <factory.h>
// Factor - Includes
#include <tmpl_inst.h>
#include <Factor.h>
#include <SqrFree.h>
// Charset - Includes
#include "csutil.h"

static bool
lowerRank ( const CanonicalForm & f, const CanonicalForm & g )
{
  int df, dg;
  Variable vf = f.mvar(), vg = g.mvar();
    
  if ( f.inCoeffDomain() ) return true;//( vg > vf );
  else 
    if ( g.inCoeffDomain() ) return false;
    else 
      if ( vf < vg ) return true;
      else 
	if ( vf == vg ) {
	  df = degree( f ); dg = degree( g );
	  if ( df < dg ) return true;
	  else 
	    if ( df == dg ) return lowerRank( LC( f ), LC( g ) );
	    else return false;
	}
  return false;
}

CanonicalForm
lowestRank( const CFList & F )
{
  CFListIterator i = F;
  CanonicalForm f;
  if ( ! i.hasItem() )	return f;
  f = i.getItem(); ++i;
  while ( i.hasItem() ) {
    if ( lowerRank( i.getItem(), f ) ) f = i.getItem();
    ++i;
  }
  return f;
}

// old version
// CanonicalForm
// prem ( const CanonicalForm &f, const CanonicalForm &g )
// {
//   CanonicalForm ff, gg, cg;
//   int df, dg;
//   bool reord;
//   Variable vf, vg, v;
//
//   if ( (vf = f.mvar()) < (vg = g.mvar()) ) return f;
//   else {
//     if ( vf == vg ) {
//       ff = f; gg = g;
//       reord = false;
//       v = vg;
//     }
//     else { 
//       v = Variable(level(f.mvar()) + 1);
//       ff = swapvar(f,vg,v);
//       gg = swapvar(g,vg,v);
//       reord=true;
//     }
//     cg = ini( gg, v );
//     dg = degree( gg, v );
//     while ( ( df = degree( ff, v ) ) >= dg )
//       ff = cg * ff - power( v, df-dg ) * gg * LC( ff, v );
//     if ( reord ) {
//       return swapvar( ff, vg, v );
//     }
//     else
//       return ff;
//   }
// }

CanonicalForm
Prem ( const CanonicalForm &f, const CanonicalForm &g ){
  CanonicalForm ff, gg, cg, l, test, lu, lv, t;
  int df, dg;
  bool reord;
  Variable vf, vg, v;
  
  if ( (vf = f.mvar()) < (vg = g.mvar()) ) return f;
  else {
    if ( vf == vg ) {
      ff = f; gg = g;
      reord = false;
      v = vg;
    }
    else { 
      v = Variable(level(f.mvar()) + 1);
      ff = swapvar(f,vg,v);
      gg = swapvar(g,vg,v);
      reord=true;
    }
    cg = LC( gg, v );
    dg = degree( gg, v );
    df = degree( ff, v );
    if (dg <= df) {l=LC(gg); gg = gg -LC(gg)*power(v,dg);}
    else { l = 1; }
    while ( ( dg <= df  ) && ( ff != ff.genZero()) ){
      // cout << "Start gcd..." << endl;
      test = gcd(l,LC(ff));
      // cout << "End gcd..." << endl;
      lu = l/test; lv = LC(ff)/test;
      t = power(v,df-dg) * gg * lv;
      if ( df == 0 ){ ff = ff.genZero(); }
      else { ff = ff - LC(ff)*power(v,df); }
      ff = lu*ff - t;
      df = degree( ff, v );
    }
    if ( reord ) return swapvar( ff, vg, v );
    else return ff;
  }
}

CanonicalForm
Prem( const CanonicalForm &f, const CFList &L ){
  CanonicalForm rem = f;
  CFListIterator i = L;
  for ( i.lastItem(); i.hasItem(); i-- ){
//cout << "   PREM: Prem(" << rem << "," ;
    rem = Prem( rem, i.getItem() );
//cout << i.getItem() << ")  = " << rem << endl;
  }
  if ( rem!=0 )
   return num(rem/lc(rem));
  else
   return rem;
}

CFList
Prem( const CFList &AS, const CFList &L ){
  CFList Output;
  for ( CFListIterator i=AS; i.hasItem(); i++ )
    Output = Union(CFList(Prem(i.getItem(),L)), Output);
  return Output;
}

// for characteristic sets
//////////////////////////////////
// replace the power of factors of polys in as by 1 if any
static CFList
nopower( const CanonicalForm & init ){
  CFFList sqrfreelist;// = Factorize(init);//SqrFree(init);
  CFList output;
  CanonicalForm elem;
  int count=0;
  
  for ( CFIterator j=init; j.hasTerms(); j++ )
    if (j.coeff() != 1 ) count += 1;
  //  if ( init != 1 ){
  //  cout << "nopower: f is " << init << endl;
  //  cout << "nopower: count is " << count << endl;}
  if ( count > 1 ) sqrfreelist = CFFList( CFFactor(init,1));
  else { 
    sqrfreelist = Factorize(init);
    //sqrfreelist.removeFirst();
  }
  for ( CFFListIterator i=sqrfreelist; i.hasItem(); i++ ){
    elem=i.getItem().factor();
    if ( cls(elem) > 0 ) output.append(elem);
  }
  return output;
}

// remove the content of polys in PS; add the removed content to
// Remembern.FS2 ( the set of removed factors )
CFList 
removecontent ( const CFList & PS, PremForm & Remembern ){
  CFList output;
  CanonicalForm cc,elem;

  if ( cls(PS.getFirst()) == 0 ) return PS;

  for (CFListIterator i=PS; i.hasItem(); i++){
    elem = i.getItem();
    cc = content(elem, elem.mvar());
    if ( cls(cc) > 0 ) { 
      output.append(elem/cc); 
      Remembern.FS2 = Union(CFList(cc), Remembern.FS2); 
    }
    else{ output.append(elem); }
  }
  return output;
}

// remove possible factors in Remember.FS1 from poly r
// Remember.FS2 contains all factors removed before
void 
removefactor( CanonicalForm & r , PremForm & Remembern){
  int test;
  CanonicalForm a,b,testelem;
  CFList testlist;
  int n=level(r);
  CFListIterator j ;

  for ( int J=1; J<= n ; J++ ){
    testlist.append(CanonicalForm(Variable(J)));
  }

  // remove already removed factors
  for ( j = Remembern.FS2 ; j.hasItem(); j++ ){
    testelem = j.getItem();
    while ( 1 ){
      test = divremt(r,testelem,a,b);
      if ( test && b == r.genZero() ) r = a;
      else break;
    }
  }
  
  // Let's look if we have other canditates to remove
  for ( j = testlist ; j.hasItem(); j++ ){
    testelem = j.getItem();
    if ( testelem != r ){
      while ( 1 ){
	test = divremt(r,testelem,a,b);
      test = divremt(r,testelem,a,b);
      if ( test && b == r.genZero() ) r = a;
	if ( test && b == r.genZero() ){
	    Remembern.FS2= Union(Remembern.FS2, CFList(testelem));
	      r = a;
	      if ( r == 1 ) break;
	}
	else break;
      }
    }
  }
}


// all irreducible nonconstant factors of a set of polynomials
CFList
factorps( const CFList &ps ){
  CFList qs;
  CFFList q;
  CanonicalForm elem;
  
  for ( CFListIterator i=ps; i. hasItem(); i++ ){
    q=Factorize(i.getItem());
    q.removeFirst();
    // Next can be simplified ( first (already removed) elem in q is the only constant 
    for ( CFFListIterator j=q; j.hasItem(); j++ ){
      elem = j.getItem().factor();
      if ( getNumVars(elem) > 0 )
	qs= Union(qs, CFList(num(elem/lc(elem))));
    }
  }
  return qs;
}

// the initial of poly f wrt to the order of the variables
static CanonicalForm
inital( const CanonicalForm &f ){
  CanonicalForm leadcoeff;

  if ( cls(f) == 0 ) {return f.genOne(); }
  else { 
    leadcoeff = LC(f,lvar(f));
    if ( leadcoeff != 0 ) 
      return num(leadcoeff/lc(leadcoeff));
    else return leadcoeff;
  }
}

// the set of all nonconstant factors of initals of polys in as
// CFList
// initalset(const CFList &as){
//   CanonicalForm elem;
//   CFList is, iss,iniset;

//   for ( CFListIterator i=as ; i.hasItem(); i++ ){
//     elem = inital(i.getItem());
//     if ( cls(elem) > 0 ) is.append(elem);
//   }
//   iss = factorps(is);
//   for ( CFListIterator j=iss; j.hasItem();j++ ){
//     elem = j.getItem();
//     if ( cls(elem) > 0 ) iniset.append(num(elem/lc(elem)));
//   }
//   return iniset;
// }

// the set of nonconstant initials of CS
// with certain repeated factors cancelled
CFList
initalset1(const CFList & CS){
  CFList temp;
  CFList initals;
  CanonicalForm init;

  for ( CFListIterator i = CS ; i.hasItem(); i++ ){
    initals= nopower( inital(i.getItem()) );
    //    init= inital(i.getItem());
    for ( CFListIterator j = initals; j.hasItem(); j++){
      init = j.getItem();
      if ( cls(init) > 0 )
	temp= Union(temp, CFList(init));
    }
  }
  return temp;
}

// the set of nonconstant initials of CS of those polys
// not having their cls higher than reducible
// with certain repeated factors cancelled
CFList
initalset2(const CFList & CS, const CanonicalForm & reducible){
  CFList temp;
  CFList initals;
  CanonicalForm init;
  int clsred = cls(reducible);

  for ( CFListIterator i = CS ; i.hasItem(); i++ ){
    init = i.getItem();
    if ( cls(init) < clsred ){
      initals= nopower( inital(init) );
      //    init= inital(i.getItem());
      for ( CFListIterator j = initals; j.hasItem(); j++){
	init = j.getItem();
	if ( cls(init) > 0 )
	  temp= Union(temp, CFList(init));
      }
    }
  }
  return temp;
}

//replace the power of factors of poly in CF init by 1 if any
// and return the sqrfree poly
// static CanonicalForm
// nopower1( const CanonicalForm & init ){
//   CFFList returnlist=Factorize(init);
//   CanonicalForm elem, test=init.genOne();
//   for ( CFFListIterator i= returnlist; i.hasItem(); i++){
//     elem = i.getItem().factor();
//     if ( cls(elem)>0 ) test *= elem;
//   }
//   return test;
// }

// the sequence of distinct factors of f
//CF pfactor( ..... ) 

//////////////////////////////////////////
// for IrrCharSeries

#ifdef IRRCHARSERIESDEBUG
#  define DEBUGOUTPUT
#else
#  undef DEBUGOUTPUT
#endif
#include "debug.h"
// examine the irreducibility of as for IrrCharSeries
int
irreducible( const CFList & AS){
// AS is given by AS = { A1, A2, .. Ar }, d_i = degree(Ai)

  DEBOUTMSG(cout, rcsid);
// 1) we test: if d_i > 1, d_j =1 for all j<>i, then AS is irreducible.
  bool deg1=1;
  for ( CFListIterator i = AS ; i.hasItem(); i++ ){
    if ( degree(i.getItem()) > 1 ){
      if ( deg1 ) deg1=0;
      else return 0; // found 2nd poly with deg > 1
    }
  }
  return 1;
}


// select an item from PS for irras
CFList
select( const ListCFList & PS){

  return PS.getFirst();
}

// divide list ppi in elems having length <= and > length
void
select( const ListCFList & ppi, int length, ListCFList & ppi1, ListCFList & ppi2){
  CFList elem;
  for ( ListCFListIterator i=ppi ; i.hasItem(); i++ ){
    elem = i.getItem();
    if ( ! elem.isEmpty() )
      if ( length <= elem.length() ){ ppi2.append(elem); }
      else { ppi1.append(elem); }
  }
}


//////////////////////////////////////////////////////////////
// help-functions for sets

// is f in F ?
static bool
member( const CanonicalForm &f, const CFList &F){

  for ( CFListIterator i=F; i.hasItem(); i++ )
    if ( i.getItem() == f ) return 1;
  return 0;
}

// are list A and B the same?
bool
same( const CFList &A, const CFList &B ){
  CFListIterator i;

  for (i = A; i.hasItem(); i++)
    if (! member(i.getItem(), B) )  return 0;
  for (i = B; i.hasItem(); i++)
    if (! member(i.getItem(), A) )  return 0;
  return 1;
}


// is List cs contained in List of lists pi?
bool
member( const CFList & cs, const ListCFList & pi ){
  ListCFListIterator i;
  CFList elem;

  for ( i=pi; i.hasItem(); i++){
    elem = i.getItem();
    if ( same(cs,elem) ) return 1;
  }
  return 0;
}

// is PS a subset of CS ?
bool
subset( const CFList &PS, const CFList &CS ){

  //  cout << "subset: called with: " << PS << "   " << CS << endl;
  for ( CFListIterator i=PS; i.hasItem(); i++ )
    if ( ! member(i.getItem(), CS) ) { 
      //      cout << "subset: " << i.getItem() << "  is not a member of " << CS << endl; 
      return 0;
    }
  return 1;
}

// Union of two List of Lists
ListCFList
MyUnion( const ListCFList & a, const ListCFList &b ){
  ListCFList output;
  ListCFListIterator i;
  CFList elem;
  
  for ( i = a ; i.hasItem(); i++ ){
    elem=i.getItem();
    // if ( ! member(elem,output) ){
    if ( (! elem.isEmpty()) && ( ! member(elem,output)) ){
      output.append(elem);
    }
  }
  
  for ( i = b ; i.hasItem(); i++ ){
    elem=i.getItem();
    // if ( ! member(elem,output) ){
    if ( (! elem.isEmpty()) && ( ! member(elem,output)) ){
      output.append(elem);
    }
  }
  return output;
}

//if list b is member of the list of lists remove b and return the rest
ListCFList
MyDifference( const ListCFList & a, const CFList &b){
  ListCFList output;
  ListCFListIterator i;
  CFList elem;

  for ( i = a ; i.hasItem(); i++ ){
    elem=i.getItem();
    if ( (! elem.isEmpty()) && ( ! same(elem,b)) ){
      output.append(elem);
    }
  }
return output;
}

// remove all elements of b from list of lists a and return the rest
ListCFList
Minus( const ListCFList & a, const ListCFList & b){
  ListCFList output=a;

  for ( ListCFListIterator i=b; i.hasItem(); i++ )
    output = MyDifference(output, i.getItem() );

  return output;
}

/*
$Log: not supported by cvs2svn $
Revision 1.3  1997/04/25 22:55:00  michael
Version for libfac-0.2.1

Revision 1.2  1996/12/13 05:53:42  michael
fixed a typo in nopower

*/
