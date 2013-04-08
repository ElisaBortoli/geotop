
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 1.225-9 'Moab' - 24 Aug 2012
 
 Copyright (c), 2012 - Stefano Endrizzi 
 
 This file is part of GEOtop 1.225-9 'Moab'
 
 GEOtop 1.225-9 'Moab' is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 1.225-9 'Moab' is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */
#ifndef UTIL_MATH_H
#define UTIL_MATH_H
#include "../fluidturtle/turtle.h"
#include "../fluidturtle/t_utilities.h"
#include "../../geotop/constants.h"

				
short tridiag(short a, long r, long c, long nx, DOUBLEVECTOR *diag_inf, DOUBLEVECTOR *diag, DOUBLEVECTOR *diag_sup, DOUBLEVECTOR *b, DOUBLEVECTOR *e);

short tridiag2(short a, long r, long c, long nbeg, long nend, DOUBLEVECTOR *ld, DOUBLEVECTOR *d, DOUBLEVECTOR *ud, DOUBLEVECTOR *b, DOUBLEVECTOR *e);

double norm_inf(DOUBLEVECTOR *V, long nbeg, long nend);

double norm_2(DOUBLEVECTOR *V, long nbeg, long nend);

double norm_1(DOUBLEVECTOR *V, long nbeg, long nend);

void Cramer_rule(double A, double B, double C, double D, double E, double F, double *x, double *y);

double minimize_merit_function(double res0, double lambda1, double res1, double lambda2, double res2);

double product(DOUBLEVECTOR *a, DOUBLEVECTOR *b);

double adaptiveSimpsonsAux(double (*f)(double), double a, double b, double epsilon, double S, double fa, double fb, double fc, int bottom);

double adaptiveSimpsons(double (*f)(double), double a, double b, double epsilon, int maxRecursionDepth);       

double adaptiveSimpsonsAux2(double (*f)(double x, void *p), void *arg, double a, double b, double epsilon, 
							double S, double fa, double fb, double fc, int bottom);

double adaptiveSimpsons2(double (*f)(double x, void *p), void *arg, double a, double b, double epsilon, int maxRecursionDepth);

#endif
