/* Copyright 1997 Michael Messollen. All rights reserved. */
////////////////////////////////////////////////////////////
// emacs edit mode for this file is -*- C++ -*-
// $Id: tmpl_inst.cc,v 1.3 1997-09-12 07:20:00 Singular Exp $
////////////////////////////////////////////////////////////


#include <templates/array.cc>
#include <templates/factor.cc>
#include <templates/list.cc>
#include <templates/functions.h>
#include <templates/matrix.cc>

#include <factory.h>

template class Factor<CanonicalForm>;
template class List<CFFactor>;
template class ListIterator<CFFactor>;
template class List<CanonicalForm>;
template class ListIterator<CanonicalForm>;
template class Array<CanonicalForm>;
template class List<MapPair>;
template class ListIterator<MapPair>;
template class Matrix<CanonicalForm>;
template class SubMatrix<CanonicalForm>;

template ostream & operator << ( ostream &, const List<CanonicalForm> & );
template ostream & operator << ( ostream &, const List<CFFactor> & );
template ostream & operator << ( ostream &, const List<MapPair> & );
template ostream & operator << ( ostream &, const Array<CanonicalForm> & );
template ostream & operator << ( ostream &, const Factor<CanonicalForm> & );
//template ostream & operator << ( ostream &, const Matrix<CanonicalForm> & );

template List<CFFactor> Union ( const List<CFFactor>&, const List<CFFactor>& );

/*
template CanonicalForm crossprod ( const Array<CanonicalForm>&, const Array<CanonicalForm>& );
template CanonicalForm prod ( const Array<CanonicalForm>& );
*/

template CanonicalForm tmax ( const CanonicalForm&, const CanonicalForm& );
template CanonicalForm tmin ( const CanonicalForm&, const CanonicalForm& );

template int tmax ( const int&, const int& );
template int tmin ( const int&, const int& );

// place here your own template stuff, not instantiated by factory
#include "tmpl_inst.h"
#include "class.cc"

template class List<int>;
template class ListIterator<int>;

template class List<IntList>;
template class ListIterator<IntList>;

template class Substitution<CanonicalForm>;
template class List<SForm>;
template class ListIterator<SForm>;

template List<CanonicalForm> Union ( const List<CanonicalForm>&, const List<CanonicalForm>& );
template List<CanonicalForm> Difference ( const List<CanonicalForm>&, const List<CanonicalForm>& );

template ostream & operator << ( ostream &, const List<int> & );
template ostream & operator << ( ostream &, const List<IntList> & );

// for charsets:
template class List<CFList>;
template class ListIterator<CFList>;
template class List<Variable>;
template class ListIterator<Variable> ;

template List<Variable> Union ( const List<Variable>&, const List<Variable>& );
template List<Variable> Difference ( const List<Variable>&, const List<Variable>& );

template ostream & operator << ( ostream &, const List<CFList> & );
template ostream & operator << ( ostream &, const List<Variable> & );
template ostream & operator << ( ostream &, const List<int> & );
template ostream & operator << ( ostream &, const Array<int> & );

template class Array<int>;
template class Array<Variable>;

// for database
#ifdef HAVE_DATABASE
template class List<CFFList>;
template class ListIterator<CFFList>;
#endif /* HAVE_DATABASE */
////////////////////////////////////////////////////////////
/*
$Log: not supported by cvs2svn $
Revision 1.2  1997/04/25 22:34:36  michael
Version for libfac-0.2.1

*/
