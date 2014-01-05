
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 1.225 'Moab' - 9 Mar 2012
 
 Copyright (c), 2012 - Stefano Endrizzi
 
 This file is part of GEOtop 1.225 'Moab'
 
 GEOtop 1.225 'Moab' is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 1.225 'Moab' is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H
#include "struct.geotop.h"
//#include "input.h"
#include "constants.h"
#include "../libraries/ascii/tabs.h"
//#include "../libraries/ascii/extensions.h"
#include "../libraries/ascii/rw_maps.h"
#include "times.h"
#include "pedo.funct.h"
#include "input.h"
#include <meteoio/MeteoIO.h>
#include <string>

short read_inpts_par(Par *par, Land *land, Times *times, Soil *sl, Meteo *met, InitTools *itools, std::string filename, FILE *flog) ;

#ifdef STAGED_FOR_REMOVING
void assign_numeric_parameters(Par *par, Land *land, Times *times, Soil *sl, Meteo *met, InitTools *itools, double **num_param, long *num_param_components, std::string keyword[], FILE *flog) ;
#else
void assign_numeric_parameters(Par *par, Land *land, Times *times, Soil *sl, Meteo *met, InitTools *itools, FILE *flog) ;
#endif

std::vector<std::string> assign_string_parameter(FILE *f, long beg, long end, std::vector<std::string> string_param, std::string keyword[]);

#ifdef STAGED_FOR_REMOVING
double assignation_number(FILE *f, long i, long j, std::string keyword[], double **num_param, long *num_param_components, double default_value, short code_error);
#endif

#ifdef STAGED_FOR_REMOVING
    std::string assignation_string(FILE *f, long i, std::string keyword[], std::vector<std::string> string_param);
#endif

short read_soil_parameters(std::string name, InitTools *IT, Soil *sl, long bed, FILE *flog);

short read_point_file(std::string name, std::vector<std::string> key_header, Par *par, FILE *flog);

short read_meteostations_file(const GeoVector<long>& i, MeteoStations *S, std::string name, std::vector<std::string> key_header, FILE *flog);

#endif
