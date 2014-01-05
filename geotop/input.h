
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
    
#ifndef INPUT_H
#define INPUT_H
#include "struct.geotop.h"
//#include "../libraries/geomorphology/geomorphology.0875.h"
//#include "../libraries/geomorphology/geomorphology.h"
#include "pedo.funct.h"
//#include "../libraries/geomorphology/networks.h"
#include "constants.h"
//#include "../libraries/geomorphology/dtm_resolution.h"
#include "../libraries/ascii/rw_maps.h"
#include "../libraries/ascii/tabs.h"
#include "snow.h"
#include "vegetation.h"
#include "output.h"
#include "times.h"
#include "clouds.h"
#include "meteo.h"
#include "meteodata.h"
#include "channels.h"
#include "indices.h"
#include "recovering.h"
#include "keywords.h"
#ifdef USE_NETCDF
//#include "../gt_utilities/gt_utilities.h"
#include "../netCDF/read_command_line.h"
#endif
#include <iostream>
#include <string>
#include <vector>
#include <meteoio/MeteoIO.h>

typedef struct __INIT_TOOLS__
{
	double swe0;
	double Tsnow0;
	double agesnow0;
    double rhosnow0;
	double rhoglac0;
	double Dglac0;
	double Tglac0;
	std::vector<std::string> met_col_names;
	std::vector<std::string> soil_col_names;
	std::vector<std::string> horizon_col_names;
	std::vector<std::string> point_col_names;
    std::vector<std::string> lapserates_col_names;
	std::vector<std::string> meteostations_col_names;
//TODO: remove LU
//	DOUBLEMATRIX *LU;
	GeoMatrix<double> LU;

//	DOUBLEMATRIX *bed;
	GeoMatrix<double> bed;
//	DOUBLETENSOR *pa_bed;
	GeoTensor<double> pa_bed ;
//	DOUBLEVECTOR *init_water_table_depth;
	GeoVector<double> init_water_table_depth;

} InitTools;


//void get_all_input(long argc, char *argv[], TOPO *top, SOIL *sl, LAND *land, METEO *met, WATER *wat, CHANNEL *cnet,
//			    PAR *par, Energy *egy, SNOW *snow, GLACIER *glac, TIMES *times, mio::IOManager& iomanager);
  void get_all_input(long argc, char *argv[], Topo *top, Soil *sl, Land *land, Meteo *met, Water *wat, Channel *cnet,
		  Par *par, Energy *egy, Snow *snow, Glacier *glac, Times *times, mio::IOManager& iomanager);

//void read_inputmaps(TOPO *top, LAND *land, SOIL *sl, PAR *par, FILE *flog, mio::IOManager& iomanager);
  void read_inputmaps(Topo *top, Land *land, Soil *sl, Par *par, FILE *flog, mio::IOManager& iomanager);

//void read_optionsfile_point(PAR *par, TOPO *top, LAND *land, SOIL *sl, TIMES *times, INIT_TOOLS *IT, FILE *flog);

//void read_optionsfile_point(PAR *par, TOPO *top, Land *land, SOIL *sl, TIMES *times, INIT_TOOLS *IT, FILE *flog);
  void read_optionsfile_point(Par *par, Topo *top, Land *land, Soil *sl, Times *times, InitTools *IT, FILE *flog);

//void set_bedrock(SOIL *sl, CHANNEL *cnet, PAR *par, TOPO *top, DOUBLEMATRIX *LC, FILE *flog);
  void set_bedrock(Soil *sl, Channel *cnet, Par *par, Topo *top, GeoMatrix<double>& LC, FILE *flog);

//DOUBLETENSOR *find_Z_of_any_layer(DOUBLEMATRIX *Zsurface, DOUBLEMATRIX *slope, DOUBLEMATRIX *LC, SOIL *sl, short point);
  GeoTensor<double> find_Z_of_any_layer(GeoMatrix<double>& Zsurface, GeoMatrix<double>& slope, GeoMatrix<double>& LC, Soil *sl, short point);

short file_exists(short key, FILE *flog);

double peat_thickness(double dist_from_channel);

//void initialize_soil_state(SOIL_STATE *S, long n, long nl);
void initialize_soil_state(SoilState *S, long n, long nl);

//void copy_soil_state(SOIL_STATE *from, SOIL_STATE *to);
void copy_soil_state(SoilState *from, SoilState *to);

//void initialize_veg_state(STATE_VEG *V, long n);
void initialize_veg_state(StateVeg *V, long n);

//void copy_veg_state(STATE_VEG *from, STATE_VEG *to);
void copy_veg_state(StateVeg *from, StateVeg *to);


#endif
