/*

 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 2.1 - 31 December 2016

 Copyright (c), 2016 - GEOtop Foundation

 This file is part of GEOtop 2.1

 GEOtop 2.1  is a free software and is distributed under GNU General Public
 License v. 3.0 <http://www.gnu.org/licenses/> WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE

 GEOtop 2.1 is distributed as a free software in the hope to create and support
 a community of developers and users that constructively interact. If you just
 use the code, please give feedback to GEOtop Foundation and the community. Any
 way you use the model, may be the most trivial one, is significantly helpful
 for the future development of the GEOtop model. Any feedback will be highly
 appreciated.

 */

#include "meteoio_plugin.h"
#include "struct.geotop.h"
/**
 * @brief Initialization for the MeteoIO plugin.
 * - Reading the DEM (necessary for several spatial interpolations algorithms)
 *
 * @param iomanager Reference to IOManager object that will be connected to
 * static iomanager variable io
 */
void meteoio_init(mio::IOManager &iomanager) // (1)
{
    mio::DEMObject dem;
    iomanager.readDEM(dem);
 //   dem.setUpdatePpt(mio::DEMObject::SLOPE);

//    std::cerr << "DEM information: \n";
//    std::cerr << "\tmin=" << dem.grid2D.getMin() << " max=" << dem.grid2D.getMax() << " mean=" << dem.grid2D.getMean() << "\n";
//    std::cerr << "\tmin slope=" << dem.min_slope << " max slope=" << dem.max_slope << std::endl << std::endl;
}
// -----------------------------------------------------------------------------------------------------------------
//void meteoio_readDEM(Matrix<double>* matrix) // (3)
//{
//    // copy DEM to topo struct
//    matrix.resize(dem.getNy() + 1, dem.getNx() + 1);
//
//    geotop::common::Variables::UV->V.resize(2 + 1);
//    geotop::common::Variables::UV->U.resize(4 + 1);
//    geotop::common::Variables::UV->V[1] = -1.0;
//    geotop::common::Variables::UV->V[2] = geotop::input::gDoubleNoValue;  // GEOtop nodata -9999.0
//
//    geotop::common::Variables::UV->U[1] = dem.cellsize;
//    geotop::common::Variables::UV->U[2] = dem.cellsize;
//    geotop::common::Variables::UV->U[3] = dem.llcorner.getNorthing();
//    geotop::common::Variables::UV->U[4] = dem.llcorner.getEasting();
//
//    copyGridToMatrix(dem, matrix);
//}