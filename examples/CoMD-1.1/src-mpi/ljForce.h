/// \file
/// Computes forces for the 12-6 Lennard Jones (LJ) potential.

#ifndef _LJTYPES_H_
#define _LJTYPES_H_
#include <stdio.h>

struct BasePotentialSt;
struct BasePotentialSt* initLjPot(void);

void writeLJForce(FILE *fp, struct BasePotentialSt* pot);
struct BasePotentialSt* readLJForce(FILE *fp);

#endif

