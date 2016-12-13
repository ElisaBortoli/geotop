#ifndef TENSOR3D_H
#define TENSOR3D_H

#include "turtle.h"
#include "../../geotop/constants.h"
#include <vector>

double ***d3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh);
void free_d3tensor(double ***t, long nrl, long ncl, long ndl);


/*==============functions copied from utilities.h ===========*/


#define   FL     512

void stop_execution(void);
double Fmin(double a, double b);
long Fminlong(long a, long b);
double Fmax(double a, double b);
long Fmaxlong(long a, long b);

/*===============================functions copied from util_math.c========================================*/

short tridiag2(long nbeg, long nend, const GeoVector<double>& ld, const GeoVector<double>& d, const GeoVector<double>& ud, const GeoVector<double>& b, GeoVector<double>& e);

double norm_inf(const GeoVector<double>& V, long nbeg, long nend);
double norm_2(const GeoVector<double>& V, long nbeg, long nend);
void Cramer_rule(double A, double B, double C, double D, double E, double F, double *x, double *y);
double minimize_merit_function(double res0, double lambda1, double res1, double lambda2, double res2);
double adaptiveSimpsonsAux(double (*f)(double), double a, double b, double epsilon, double S, double fa, double fb, double fc, int bottom);
double adaptiveSimpsons(double (*f)(double), double a, double b, double epsilon, int maxRecursionDepth);
double adaptiveSimpsonsAux2(double (*f)(double x, void *p), void *arg, double a, double b, double epsilon,
							double S, double fa, double fb, double fc, int bottom);
double adaptiveSimpsons2(double (*f)(double x, void *p), void *arg, double a, double b, double epsilon, int maxRecursionDepth);



#endif
