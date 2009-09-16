#ifndef SHORTFL_H
#define SHORTFL_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/*
* ABSTRACT
*/
/* $Id: shortfl.h,v 1.4 2009-09-16 12:26:27 Singular Exp $ */
#include "structs.h"

BOOLEAN nrGreaterZero (number k);
number  nrMult        (number a, number b);
number  nrInit        (int i);
int     nrInt         (number &n, const ring r);
number  nrAdd         (number a, number b);
number  nrSub         (number a, number b);
void    nrPower       (number a, int i, number * result);
BOOLEAN nrIsZero      (number a);
BOOLEAN nrIsOne       (number a);
BOOLEAN nrIsMOne      (number a);
number  nrDiv         (number a, number b);
number  nrNeg         (number c);
number  nrInvers      (number c);
BOOLEAN nrGreater     (number a, number b);
BOOLEAN nrEqual       (number a, number b);
void    nrWrite       (number &a);
const char *  nrRead  (const char *s, number *a);
int     nrGetChar();
#ifdef LDEBUG
BOOLEAN nrDBTest(number a, const char *f, const int l);
#endif

nMapFunc nrSetMap(ring src, ring dst);

float   nrFloat(number n);
number nrMapQ(number from);
#endif

