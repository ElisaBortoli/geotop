#include <gtest/gtest.h>
#include <meteoio_wrapper.h>

#include <iostream>
#include <vector>

TEST(Meteoio, which_cfgfile){
  MeteoioWrapper MW {};
  MW.which_cfgfile();
  std::cout << std::endl;
}

TEST(Meteoio, read_DEM){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg); 
  mio::DEMObject dem;
  iomanager.readDEM(dem);

  std::cout << "DEM information:" << std::endl;
  std::cout << "\tmin=" << dem.grid2D.getMin()
	    << " max=" << dem.grid2D.getMax()
	    << " mean=" << dem.grid2D.getMean() << std::endl;
  std::cout << "\tcellsize=" << dem.cellsize << std::endl;
  std::cout << std::endl;
}

TEST(Meteoio, print_DEM_map){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg); 
  mio::DEMObject dem;
  iomanager.readDEM(dem);

  std::cout << "-------------- DEM map --------------" << std::endl;
  std::cout << dem.toString() << std::endl;
  std::cout << std::endl;
}

TEST(Meteoio, print_LANDUSE_map){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);  
  mio::Grid2DObject landuse;

  // if LANDCOVER definita nel io_it.ini
  // iomanager.readLanduse(landuse);
  // std::cout << landuse.toString() << std::endl;

  iomanager.read2DGrid(landuse, "landcover.asc");
  
  std::cout << "-------------- LANDCOVER map --------------" << std::endl;
  std::cout << landuse.toString() << std::endl;
  std::cout << std::endl;
}

TEST(Meteoio, interpolate_2D_TA){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);

  // start and end of the simulation
  mio::Date startdate,enddate;
  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::IOUtils::convertString(startdate, cfg.get("START_DATE", "Dates"), TZ);
  mio::IOUtils::convertString(enddate, cfg.get("END_DATE", "Dates"), TZ);

  // chosen date for interpolation
  mio::Date chosendate(2012,01,20,14,00, TZ);
  mio::IOUtils::convertString(chosendate,"2012-01-20T14:00:00", TZ);
  
  mio::DEMObject dem;
  iomanager.readDEM(dem);

  mio::Grid2DObject tagrid;

  // print maps at the startdate
  iomanager.getMeteoData(startdate, dem, mio::MeteoData::TA, tagrid);
  iomanager.write2DGrid(tagrid, mio::MeteoGrids::TA, startdate);

  // print maps the enddate
  iomanager.getMeteoData(enddate, dem, mio::MeteoData::TA, tagrid);
  iomanager.write2DGrid(tagrid, mio::MeteoGrids::TA, enddate);

  //performing spatial interpolations
  iomanager.getMeteoData(chosendate, dem, mio::MeteoData::TA, tagrid);
  iomanager.write2DGrid(tagrid, mio::MeteoGrids::TA, chosendate);
}

TEST(Meteoio, interpolate_2D_PSUM){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);

  // start and end of the simulation
  mio::Date startdate,enddate;
  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::IOUtils::convertString(startdate, cfg.get("START_DATE", "Dates"), TZ);
  mio::IOUtils::convertString(enddate, cfg.get("END_DATE", "Dates"), TZ);

  // chosen date for interpolation
  mio::Date chosendate(2012,01,20,14,00, TZ);
  mio::IOUtils::convertString(chosendate,"2012-01-20T14:00:00", TZ);
  
  mio::DEMObject dem;
  iomanager.readDEM(dem);

  mio::Grid2DObject psumgrid;

  // print maps at the startdate
  iomanager.getMeteoData(startdate, dem, mio::MeteoData::PSUM, psumgrid);
  iomanager.write2DGrid(psumgrid, mio::MeteoGrids::PSUM, startdate);

  // print maps the enddate
  iomanager.getMeteoData(enddate, dem, mio::MeteoData::PSUM, psumgrid);
  iomanager.write2DGrid(psumgrid, mio::MeteoGrids::PSUM, enddate);

  //performing spatial interpolations
  iomanager.getMeteoData(chosendate, dem, mio::MeteoData::PSUM, psumgrid);
  iomanager.write2DGrid(psumgrid, mio::MeteoGrids::PSUM, chosendate);
}
