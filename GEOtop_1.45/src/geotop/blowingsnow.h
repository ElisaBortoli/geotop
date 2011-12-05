
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 1.145 'Montebello' - 8 Nov 2010
 
 Copyright (c), 2010 - Stefano Endrizzi - Geographical Institute, University of Zurich, Switzerland - stefano.endrizzi@geo.uzh.ch 
 
 This file is part of GEOtop 1.145 'Montebello'
 
 GEOtop 1.145 'Montebello' is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 1.145 'Montebello' is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */

void windtrans_snow(SNOW *snow, METEO *met, WATER *wat, LANDCOVER *land, TOPO *top, PAR *par, double t0);

void set_inhomogeneous_fetch(SNOW *snow, METEO *met, LANDCOVER *land, PAR *par, TOPO *top, short *yes);

void set_windtrans_snow(double Dt, double t, SNOW *snow, METEO *met, LANDCOVER *land, PAR *par, FILE *f);

void print_windtrans_snow(double Dt, SNOW *snow, PAR *par, METEO *met, DOUBLEMATRIX *LC);

void extend_topography(DOUBLEMATRIX *M, double novalue);

void extend_topography_row(DOUBLEMATRIX *M, double novalue);

void extend_topography_column(DOUBLEMATRIX *M, double novalue);

void find_the_nearest(long r, long c, double novalue, DOUBLEMATRIX *M, long *rr, long *cc);

void find_the_nearest_row(long r, long c, double novalue, DOUBLEMATRIX *M, long *rr, long *cc);

void find_the_nearest_column(long r, long c, double novalue, DOUBLEMATRIX *M, long *rr, long *cc);

short no_novalue(long r, long c, DOUBLEMATRIX *M, double novalue, long *rr, long *cc);

void set_no_value(DOUBLEMATRIX *M, DOUBLEMATRIX *N, double undef);

