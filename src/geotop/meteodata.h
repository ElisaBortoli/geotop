
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 2.1 release candidate  (release date: 31 december 2016)
 
 Copyright (c), 2016 - GEOtop Foundation
 
 This file is part of GEOtop 2.1 
 
 GEOtop 2.1  is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 2.1  is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */

#ifndef METEODATA2_H
#define METEODATA2_H

#include "struct.geotop.h"
#include "constants.h"
#include "times.h"
#include "meteo.h"
#include "../libraries/ascii/rw_maps.h"
#include "../libraries/ascii/tabs.h"

void time_interp_linear(double t0, double tbeg, double tend, double *out, double **data, long nlines, long ncols, long col_date, short flag, long *istart);
void time_interp_constant(double t0, double tbeg, double tend, double *out, double **data, long nlines, long ncols, long col_date, short flag, long *istart);
void time_no_interp(short flag, long *istart, double *out, double **data, long nlines, long ncols, long col_date, double tbeg);
double integrate_meas_linear_beh(short flag, double t, long i, double **data, long col, long col_date);
double integrate_meas_constant_beh(short flag, double t, long i, double **data, long col, long col_date);
long find_line_data(short flag, double t, long ibeg, double **data, long col_date, long nlines, short *a);
double time_in_JDfrom0(short flag, long i, long col, double **data);
long find_station(long metvar, long nstat, double **var);
double **read_horizon(short a, long i, std::string name, std::vector<std::string> ColDescr, long *num_lines);
short fixing_dates(long imeteo, double **data, double ST, double STstat, long nlines, long date12col, long JDfrom0col);
short fill_wind_xy(double **data, long nlines, long Wspeed, long Wdir, long Wx, long Wy, std::string HeaderWx, std::string HeaderWy);
short fill_wind_dir(double **data, long nlines, long Wspeed, long Wdir, long Wx, long Wy, std::string HeaderWSpeed, std::string HeaderWdir);
short fill_Tdew(long imeteo, GeoVector<double> &Z, double **data, long nlines, long RH, long Tair, long Tairdew, std::string HeaderTdew, double RHmin);
short fill_RH(long imeteo, GeoVector<double> &Z, double **data, long nlines, long RH, long Tair, long Tairdew, std::string HeaderRH);
short fill_Pint(double **data, long nlines, long Prec, long PrecInt, long JDfrom0, std::string HeaderPrecInt);
void check_times(long imeteo, double **data, long nlines, long JDfrom0);
//void rewrite_meteo_files(double **meteo, long meteolines, char **header, std::string name, short added_JD, short added_wind_xy, short added_wind_dir,
//						 short added_cloudiness, short added_Tdew, short added_RH, short added_Pint);
void rewrite_meteo_files(double **meteo, long meteolines, std::vector<std::string>, std::string name, short added_JD, short added_wind_xy, short added_wind_dir,
						 short added_cloudiness, short added_Tdew, short added_RH, short added_Pint);

#endif
