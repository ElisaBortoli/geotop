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
}
//void meteoio_readDEM(Matrix<double>* matrix)
//{
//    // copy DEM to topo struct
//    matrix.reset{dem.getNy() + 1, dem.getNx() + 1};
//
//    UV->V.reset(2 + 1);
//    geotop::common::Variables::UV->U.reset(4 + 1);
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

// -----------------------------------------------------------------------------------------------------------------
//void meteoio_readDEM(Matrix<double>* matrix) // (3)
//{
//    // copy DEM to topo struct
//    matrix.reset(dem.getNy() + 1, dem.getNx() + 1);
//
//    geotop::common::Variables::UV->V.reset(2 + 1);
//    geotop::common::Variables::UV->U.reset(4 + 1);
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

int fill_GTmeteostations_meta(const double &JDE, mio::IOManager &iomanager, METEO *met)
{

    mio::Config cfg = iomanager.getConfig();

    std::string tz = cfg.get("TIME_ZONE", "Input");

   double d_tz = atof(tz.c_str()); // from string to float
  //  mio::Date d1(JDE + mio::Date::Matlab_offset, d_tz); => GEOtop 2.1 !!!
    mio::Date d1;
    mio::IOUtils::convertString(d1,"2012-01-20T18:00:00", d_tz);

    std::vector<mio::MeteoData> vec_meteo;
    iomanager.getMeteoData(d1, vec_meteo);

    std::cerr << vec_meteo.size() << " stations with an average sampling rate of " << iomanager.getAvgSamplingRate()
              << " or 1 point every " << 1./(iomanager.getAvgSamplingRate()*60.+1e-12) << " minutes\n";

    for (unsigned int ii=0; ii < vec_meteo.size(); ii++) {
        std::cerr << "---------- Station: " << (ii+1) << " / " << vec_meteo.size() << std::endl;
        std::cerr << vec_meteo[ii].toString() << std::endl;
    }


//    size_t lMeteoStationContainerSize = vec_meteo.size() + 1;
//
//    met->st->E.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->N.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->lat.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->lon.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->Z.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->sky.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->ST.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->Vheight.reset(new Vector<double>{lMeteoStationContainerSize});
//    met->st->Theight.reset(new Vector<double>{lMeteoStationContainerSize});
//
//    for (size_t i = 1; i < lMeteoStationContainerSize; i++)
//    {
//        mio::MeteoData &tmpmeteo = vec_meteo[i - 1];
//
//        met->st->E->co[i] = tmpmeteo.meta.position.getEasting();
//        met->st->N->co[i] = tmpmeteo.meta.position.getNorthing();
//        met->st->lat->co[i] = tmpmeteo.meta.position.getLat();
//        met->st->lon->co[i] = tmpmeteo.meta.position.getLon();
//        met->st->Z->co[i] = tmpmeteo.meta.position.getAltitude();
//
//        met->st->ST->co[i] = d_tz;
//        met->st->Vheight->co[i] = 2;
//        met->st->Theight->co[i] = 5;
//    }


    return 1;
}