/* emacs edit mode for this file is -*- C++ -*- */
/* $Id: int_pp.cc,v 1.4 1997-09-09 09:03:35 schmidt Exp $ */

#include <config.h>

#include "assert.h"

#include "cf_defs.h"
#include "int_pp.h"
#include "canonicalform.h"
#include "cf_factory.h"

MP_INT InternalPrimePower::primepow;
MP_INT InternalPrimePower::primepowhalf;
int InternalPrimePower::prime;
int InternalPrimePower::exp;
int InternalPrimePower::initialized = InternalPrimePower::initialize();


InternalPrimePower::InternalPrimePower()
{
    mpz_init( &thempi );
}

InternalPrimePower::InternalPrimePower( const int i )
{
    mpz_init_set_si( &thempi, i );
    if ( mpz_cmp_si( &thempi, 0 ) < 0 ) {
	mpz_neg( &thempi, &thempi );
	mpz_mod( &thempi, &thempi, &primepow );
	mpz_sub( &thempi, &primepow, &thempi );
    }
    else
	mpz_mod( &thempi, &thempi, &primepow );
}

InternalPrimePower::InternalPrimePower( const MP_INT & mpi ) : thempi( mpi ) {}

InternalPrimePower::InternalPrimePower( const char * str )
{
    mpz_init_set_str( &thempi, str, 10 );
    if ( mpz_cmp_si( &thempi, 0 ) < 0 ) {
	mpz_neg( &thempi, &thempi );
	mpz_mod( &thempi, &thempi, &primepow );
	mpz_sub( &thempi, &primepow, &thempi );
    }
    else
	mpz_mod( &thempi, &thempi, &primepow );
}

InternalPrimePower::~InternalPrimePower()
{
    mpz_clear( &thempi );
}

InternalCF* InternalPrimePower::deepCopyObject() const
{
    MP_INT dummy;
    mpz_init_set( &dummy, &thempi );
    return new InternalPrimePower( dummy );
}

InternalCF * InternalPrimePower::normalize_myself()
{
    ASSERT( getRefCount() == 1, "illegal operation" );
    if ( mpz_cmp_si( &thempi, 0 ) < 0 ) {
	mpz_neg( &thempi, &thempi );
	mpz_mod( &thempi, &thempi, &primepow );
	mpz_sub( &thempi, &primepow, &thempi );
    }
    else
	mpz_mod( &thempi, &thempi, &primepow );
    return this;
}

int InternalPrimePower::initialize()
{
    mpz_init_set_si( &primepow, 3 );
    mpz_init_set_si( &primepowhalf, 1 );
    prime = 3;
    exp = 1;
    return 1;
}

void
InternalPrimePower::setPrimePower( int p, int k )
{
    ASSERT( p > 1 && k > 0, "illegal prime power" );
    if ( p != prime || k != exp ) {
	mpz_set_si( &primepow, p );
	mpz_pow_ui( &primepow, &primepow, (unsigned int)k );
	mpz_div_ui( &primepowhalf, &primepow, 2 );
	prime = p;
	exp = k;
    }
}

int
InternalPrimePower::getp()
{
    return prime;
}

int
InternalPrimePower::getk()
{
    return exp;
}

#ifndef NOSTREAMIO
void InternalPrimePower::print( ostream & os, char * c )
{
    if ( *c == '*' && mpz_cmp_si( &thempi, 1 ) == 0 )
	os << c+1;
    else if ( *c == '*' && mpz_cmp_si( &thempi, -1 ) == 0 )
	os << '-' << c+1;
    else {
	char * str = new char[mpz_sizeinbase( &thempi, 10 ) + 2];
	str = mpz_get_str( str, 10, &thempi );
	os << str << c;
	delete [] str;
    }
}
#endif /* NOSTREAMIO */

bool InternalPrimePower::isZero() const
{
    return mpz_cmp_si( &thempi, 0 ) == 0;
}

bool InternalPrimePower::isOne() const
{
    return mpz_cmp_si( &thempi, 1 ) == 0;
}

bool InternalPrimePower::is_imm() const
{
    return false;
}

InternalCF* InternalPrimePower::genZero()
{
    if ( isZero() )
	return copyObject();
    else
	return new InternalPrimePower();
}

InternalCF* InternalPrimePower::genOne()
{
    if ( isOne() )
	return copyObject();
    else
	return new InternalPrimePower();
}

InternalCF* InternalPrimePower::neg()
{
    if ( getRefCount() > 1 ) {
	decRefCount();
	MP_INT dummy;
	mpz_init( &dummy );
	mpz_sub( &dummy, &primepow, &thempi );
	return new InternalPrimePower( dummy );
    }
    else {
	mpz_sub( &thempi, &primepow, &thempi );
	return this;
    }
}


int InternalPrimePower::comparesame( InternalCF * c )
{
    return mpz_cmp( &thempi, &MPI( c ) );
}

InternalCF* InternalPrimePower::addsame( InternalCF * c )
{
    if ( getRefCount() > 1 ) {
	decRefCount();
	MP_INT dummy;
	mpz_init( &dummy );
	mpz_add( &dummy, &thempi, &MPI( c ) );
	if ( mpz_cmp( &dummy, &primepow ) >= 0 )
	    mpz_sub( &dummy, &dummy, &primepow );
	return new InternalPrimePower( dummy );
    }
    else {
	mpz_add( &thempi, &thempi, &MPI( c ) );
	if ( mpz_cmp( &thempi, &primepow ) >= 0 )
	    mpz_sub( &thempi, &thempi, &primepow );
	return this;
    }
}

InternalCF* InternalPrimePower::subsame( InternalCF * c )
{
    if ( getRefCount() > 1 ) {
	decRefCount();
	MP_INT dummy;
	mpz_init( &dummy );
	mpz_sub( &dummy, &thempi, &MPI( c ) );
	if ( mpz_cmp_si( &dummy, 0 ) < 0 )
	    mpz_add( &dummy, &dummy, &primepow );
	return new InternalPrimePower( dummy );
    }
    else {
	mpz_sub( &thempi, &thempi, &MPI( c ) );
	if ( mpz_cmp_si( &thempi, 0 ) < 0 )
	    mpz_add( &thempi, &thempi, &primepow );
	return this;
    }
}

InternalCF* InternalPrimePower::mulsame( InternalCF * c )
{
    if ( getRefCount() > 1 ) {
	decRefCount();
	MP_INT dummy;
	mpz_init( &dummy );
	mpz_mul( &dummy, &thempi, &MPI( c ) );
	mpz_mod( &dummy, &dummy, &primepow );
	return new InternalPrimePower( dummy );
    }
    else {
	mpz_mul( &thempi, &thempi, &MPI( c ) );
	mpz_mod( &thempi, &thempi, &primepow );
	return this;
    }
}

InternalCF* InternalPrimePower::dividesame( InternalCF * c )
{
    return divsame( c );
}

InternalCF* InternalPrimePower::divsame( InternalCF * c )
{
    if ( c == this ) {
	if ( deleteObject() ) delete this;
	return CFFactory::basic( 1 );
    }
    if ( getRefCount() > 1 ) {
	decRefCount();
	MP_INT dummy, a, b;
	mpz_init( &dummy ); mpz_init( &a ); mpz_init( &b );
	mpz_gcdext( &dummy, &a, &b, &primepow, &MPI( c ) );
	ASSERT( mpz_cmp_si( &dummy, 1 ) == 0, "illegal inversion" );
	mpz_clear( &dummy ); mpz_clear( &a );
	if ( mpz_cmp_si( &b, 0 ) < 0 )
	    mpz_add( &b, &b, &primepow );
	mpz_mul( &b, &b, &thempi );
	mpz_mod( &b, &b, &primepow );
	return new InternalPrimePower( b );
    }
    else {
	MP_INT dummy, a, b;
	mpz_init( &dummy ); mpz_init( &a ); mpz_init( &b );
	mpz_gcdext( &dummy, &a, &b, &primepow, &MPI( c ) );
	ASSERT( mpz_cmp_si( &dummy, 1 ) == 0, "illegal inversion" );
	if ( mpz_cmp_si( &b, 0 ) < 0 )
	    mpz_add( &b, &b, &primepow );
	mpz_mul( &thempi, &b, &thempi );
	mpz_mod( &thempi, &thempi, &primepow );
	mpz_clear( &dummy ); mpz_clear( &a ); mpz_clear( &b );
	return this;
    }
}

InternalCF* InternalPrimePower::modulosame( InternalCF * c )
{
    if ( deleteObject() ) delete this;
    return CFFactory::basic( 0 );
}

InternalCF* InternalPrimePower::modsame( InternalCF * c )
{
    if ( deleteObject() ) delete this;
    return CFFactory::basic( 0 );
}

void InternalPrimePower::divremsame( InternalCF * c, InternalCF*& quot, InternalCF*& rem )
{
    if ( c == this ) {
	quot = CFFactory::basic( 1 );
	rem = CFFactory::basic( 0 );
    }
    else {
	MP_INT dummy, a, b;
	mpz_init( &dummy ); mpz_init( &a ); mpz_init( &b );
	mpz_gcdext( &dummy, &a, &b, &primepow, &MPI( c ) );
	ASSERT( mpz_cmp_si( &dummy, 1 ) == 0, "illegal inversion" );
	mpz_clear( &dummy ); mpz_clear( &a );
	if ( mpz_cmp_si( &b, 0 ) < 0 )
	    mpz_add( &b, &b, &primepow );
	mpz_mul( &b, &b, &thempi );
	mpz_mod( &b, &b, &primepow );
	quot = new InternalPrimePower( b );
	rem = CFFactory::basic( 0 );
    }
}

bool InternalPrimePower::divremsamet( InternalCF* c, InternalCF*& quot, InternalCF*& rem )
{
    divremsame( c, quot, rem );
    return true;
}

int InternalPrimePower::comparecoeff( InternalCF* c )
{
    ASSERT( 0, "this function should never be called" );
    return 0;
}

InternalCF* InternalPrimePower::addcoeff( InternalCF* c )
{
    ASSERT( 0, "this function should never be called" );
    return this;
}

InternalCF* InternalPrimePower::subcoeff( InternalCF* c, bool negate )
{
    ASSERT( 0, "this function should never be called" );
    return this;
}

InternalCF* InternalPrimePower::mulcoeff( InternalCF* c )
{
    ASSERT( 0, "this function should never be called" );
    return this;
}

InternalCF* InternalPrimePower::dividecoeff( InternalCF* c, bool invert )
{
    ASSERT( 0, "this function should never be called" );
    return this;
}

InternalCF* InternalPrimePower::divcoeff( InternalCF* c, bool invert )
{
    ASSERT( 0, "this function should never be called" );
    return this;
}

InternalCF* InternalPrimePower::modcoeff( InternalCF* c, bool invert )
{
    ASSERT( 0, "this function should never be called" );
    return this;
}

InternalCF* InternalPrimePower::modulocoeff( InternalCF* c, bool invert )
{
    ASSERT( 0, "this function should never be called" );
    return this;
}

void InternalPrimePower::divremcoeff( InternalCF* c, InternalCF*& quot, InternalCF*& rem, bool invert )
{
    ASSERT( 0, "this function should never be called" );
}

bool InternalPrimePower::divremcoefft( InternalCF* c, InternalCF*& quot, InternalCF*& rem, bool invert )
{
    ASSERT( 0, "this function should never be called" );
    return true;
}

int InternalPrimePower::intval() const
{
  return (int)mpz_get_si( &thempi );
}

int InternalPrimePower::intmod( int p ) const
{
  return (int)mpz_mmod_ui( 0, &thempi, (unsigned long)p );
}

int InternalPrimePower::sign ( ) const
{
    return mpz_cmp_si( &thempi, 0 );
}
