/* emacs edit mode for this file is -*- C++ -*- */
/* $Id: cf_eval.h,v 1.3 1997-06-19 12:27:20 schmidt Exp $ */

#ifndef INCL_CF_EVAL_H
#define INCL_CF_EVAL_H

#include <config.h>

#ifndef NOSTREAMIO
#include <iostream.h>
#endif /* NOSTREAMIO */

#include "canonicalform.h"

/*BEGINPUBLIC*/

class Evaluation
{
protected:
    CFArray values;
public:
    Evaluation() : values() {}
    Evaluation( int min, int max ) : values( min, max ) {}
    Evaluation( const Evaluation & e ) : values( e.values ) {}
    virtual ~Evaluation() {}
    Evaluation& operator= ( const Evaluation & e );
    int min() const { return values.min(); }
    int max() const { return values.max(); }
    CanonicalForm operator[] ( int i ) const { return values[i]; }
    CanonicalForm operator[] ( const Variable & v ) const { return operator[](v.level()); }
    CanonicalForm operator() ( const CanonicalForm& f ) const;
    CanonicalForm operator() ( const CanonicalForm & f, int i, int j ) const;
    virtual void nextpoint();
#ifndef NOSTREAMIO
    friend ostream& operator<< ( ostream& s, const Evaluation &e );
#endif /* NOSTREAMIO */
};

/*ENDPUBLIC*/

#endif /* ! INCL_CF_EVAL_H */
