
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

#ifndef SNOW_H
#define SNOW_H
#include "constants.h"
#include "struct.geotop.h"
#include "../libraries/ascii/extensions.h"
#include "../libraries/ascii/rw_maps.h"
#include "times.h"
#include "output.h"

#include "PBSM.h"

extern long number_novalue, number_absent;

extern T_INIT *UV;
extern char *WORKING_DIRECTORY;
extern char **files;
extern long Nl, Nr, Nc;
extern long i_sim;
extern char *FailedRunFile;

#define simpl_snow 1E-1
#define no_snow 1E-6


//Author: Stefano Endrizzi
//Date: 13 November 2005
//Contents: Snow subroutines

/*----------------------------------------------------------------------------------------------------------*/
double rho_newlyfallensnow(double u, double Tatm, double Tfreez);

/*----------------------------------------------------------------------------------------------------------*/
void snow_compactation(double Dt, long r, long c, long l, STATEVAR_3D *snow, double slope, PAR *par);

/*----------------------------------------------------------------------------------------------------------*/
void snow_layer_combination(double a, long r, long c, STATEVAR_3D *snow, double Ta, LONGVECTOR *inf, double SWEmax_layer, double SWEmax_tot, FILE *flog);

/*----------------------------------------------------------------------------------------------------------*/
double DEPTH(long r, long c, LONGMATRIX *n, DOUBLETENSOR *Dz);

/*----------------------------------------------------------------------------------------------------------*/
double get_SWE(long r, long c, LONGMATRIX *n, DOUBLETENSOR *w1, DOUBLETENSOR *w2);

/*----------------------------------------------------------------------------------------------------------*/
void snowlayer_merging(double a, long r, long c, STATEVAR_3D *snow, long l1, long l2, long lres);

/*----------------------------------------------------------------------------------------------------------*/
double internal_energy(double w_ice, double w_liq, double T);

/*----------------------------------------------------------------------------------------------------------*/
void from_internal_energy(double a, long r, long c, double h, double *w_ice, double *w_liq, double *T);

/*----------------------------------------------------------------------------------------------------------*/
void write_snow(long r, long c, long l, STATEVAR_3D *snow);

/*----------------------------------------------------------------------------------------------------------*/
void write_snow_all(long r, long c, STATEVAR_3D *snow);

/*----------------------------------------------------------------------------------------------------------*/
short set_snow_min(double a, long r, long c, STATEVAR_3D *snow, long l1, long l2, double Dmin);

/*----------------------------------------------------------------------------------------------------------*/
short set_snow_max(double a, long r, long c, STATEVAR_3D *snow, long l1, long l2, double Dmax);

/*----------------------------------------------------------------------------------------------------------*/
short set_snowice_min(double a, long r, long c, STATEVAR_1D *snow, long l1, long l2, double wicemin);

/*----------------------------------------------------------------------------------------------------------*/
void split_layers(long r, long c, STATEVAR_3D *snow, long l1);

/*----------------------------------------------------------------------------------------------------------*/
void merge_layers(double a, long r, long c, STATEVAR_3D *snow, long l1);

/*----------------------------------------------------------------------------------------------------------*/
void min_max_layer(long n, DOUBLEVECTOR *Dmin, DOUBLEVECTOR *Dmax, DOUBLEVECTOR *Dmin2, DOUBLEVECTOR *Dmax2, long linf);

/*----------------------------------------------------------------------------------------------------------*/
void initialize_snow(long r, long c, long l, STATEVAR_3D *snow);

/*----------------------------------------------------------------------------------------------------------*/
void show_Dminmax(long r, long c, double *Dmin, double *Dmax, long n);

/*----------------------------------------------------------------------------------------------------------*/
void update_snow_age(double Psnow, double Ts, double Dt, double *tsnow_nondim);

/*----------------------------------------------------------------------------------------------------------*/
double snow_albedo(double ground_alb, double snowD, double AEP, double freshsnow_alb, double C, double tsnow, double cosinc, double ( *F)(double x));

/*----------------------------------------------------------------------------------------------------------*/
double Fzen(double cosinc);

/*----------------------------------------------------------------------------------------------------------*/
double k_thermal_snow_Sturm(double density);

/*----------------------------------------------------------------------------------------------------------*/
double k_thermal_snow_Yen(double density);

/*----------------------------------------------------------------------------------------------------------*/
void non_dimensionalize_snowage(double *snowage, double Ta);

/*----------------------------------------------------------------------------------------------------------*/
void glac2snow(double a, long r, long c, STATEVAR_3D *snow, STATEVAR_3D *glac);
void snow2glac(double a, long r, long c, STATEVAR_3D *snow, STATEVAR_3D *glac);

/*----------------------------------------------------------------------------------------------------------*/
void WBsnow(double Dt, long ns, long r, long c, STATEVAR_3D *snow, double *Melt, double *RainOnSnow, PAR *par, double slope, double Rain, ENERGY *E, double Evap);

/*----------------------------------------------------------------------------------------------------------*/
void new_snow(double a, long r, long c, STATEVAR_3D *snow, double P, double Dz, double T);

/*----------------------------------------------------------------------------------------------------------*/
void WBglacier(long ns, long ng, long r, long c, STATEVAR_3D *glac, double *Melt, PAR *par, ENERGY *E, double Evap);

/*----------------------------------------------------------------------------------------------------------*/
void find_SCA(STATEVAR_3D *snow, PAR *par, double **Z, double t);

/*----------------------------------------------------------------------------------------------------------*/
double theta_snow(double a, double b, double T);

/*----------------------------------------------------------------------------------------------------------*/
double dtheta_snow(double a, double b, double T);

/*----------------------------------------------------------------------------------------------------------*/
double max_dtheta_snow(double a, double b);

/*----------------------------------------------------------------------------------------------------------*/
void allocate_and_initialize_statevar_3D(STATEVAR_3D *V, double nan, long nl, long nr, long nc);

/*----------------------------------------------------------------------------------------------------------*/
void deallocate_statevar_3D(STATEVAR_3D *V);

/*----------------------------------------------------------------------------------------------------------*/
void allocate_and_initialize_statevar_1D(STATEVAR_1D *V, double nan, long nl);

/*----------------------------------------------------------------------------------------------------------*/
void deallocate_statevar_1D(STATEVAR_1D *V);

/*----------------------------------------------------------------------------------------------------------*/
short copy_statevar_from3D_to1D(long r, long c, STATEVAR_3D *origin, STATEVAR_1D *destination);

/*----------------------------------------------------------------------------------------------------------*/
double interpolate_snow(long r, long c, double h, long max, DOUBLETENSOR *Dz, DOUBLETENSOR *Q);

/*----------------------------------------------------------------------------------------------------------*/
void copy_snowvar3D(STATEVAR_3D *from, STATEVAR_3D *to);

/*----------------------------------------------------------------------------------------------------------*/
#endif
