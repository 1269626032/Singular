// emacs edit mode for this file is -*- C++ -*-
// $Id: singext.cc,v 1.1 1997-04-15 09:37:55 schmidt Exp $

/*
$Log: not supported by cvs2svn $
Revision 1.0  1996/05/17 10:59:47  stobbe
Initial revision

*/

#include <config.h>

#include "assert.h"

#include "cf_defs.h"
#include "singext.h"
#include "int_cf.h"
#include "int_int.h"
#include "int_rat.h"
#include "imm.h"
#include "cf_factory.h"

MP_INT
gmp_numerator ( const CanonicalForm & f )
{
    InternalCF * ff = f.getval();
    ASSERT( ! is_imm( ff ), "illegal type" );
    MP_INT result;
    if ( ff->levelcoeff() == IntegerDomain ) {
	mpz_init_set( &result, &(InternalInteger::MPI( ff )) );
	ff->deleteObject();
    }
    else  if ( ff->levelcoeff() == RationalDomain ) {
	mpz_init_set( &result, &(InternalRational::MPQNUM( ff )) );
	ff->deleteObject();
    }
    else {
	ASSERT( 0, "illegal type" );
    }
    return result;
}

MP_INT
gmp_denominator ( const CanonicalForm & f )
{
    InternalCF * ff = f.getval();
    ASSERT( ! is_imm( ff ), "illegal type" );
    MP_INT result;
    if ( ff->levelcoeff() == IntegerDomain ) {
	mpz_init_set_si( &result, 1 );
	ff->deleteObject();
    }
    else  if ( ff->levelcoeff() == RationalDomain ) {
	mpz_init_set( &result, &(InternalRational::MPQDEN( ff )) );
	ff->deleteObject();
    }
    else {
	ASSERT( 0, "illegal type" );
    }
    return result;
}

CanonicalForm
make_cf ( const MP_INT & n )
{
    return CanonicalForm( CFFactory::basic( n ) );
}

CanonicalForm
make_cf ( const MP_INT & n, const MP_INT & d, bool normalize )
{
    return CanonicalForm( CFFactory::rational( n, d, normalize ) );
}
