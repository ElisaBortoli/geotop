#include <gtest/gtest.h>
#include <meteoio_wrapper.h>

#include <iostream>
#include <vector>

// Data Reading
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

TEST(Meteoio, print_MeteoStations_fixed_day){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);

  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::Date d1(2012,01,20,13,00,TZ);
  mio::IOUtils::convertString(d1,"2012-01-20T13:00:00", TZ);

  std::vector<mio::MeteoData> vecMeteo;
  
  iomanager.getMeteoData(d1, vecMeteo);

  std::cout << vecMeteo.size()
	    << " stations with an average sampling rate of "
	    << iomanager.getAvgSamplingRate()
	    << " or 1 point every " << 1./(iomanager.getAvgSamplingRate()*60.+1e-12)
	    << " minutes" << std::endl;
  std::cout << std::endl;
  
  for (unsigned int i=0; i<vecMeteo.size(); i++) {
    std::cout << "---------- Station: " << (i+1)
	      << " / " << vecMeteo.size() << std::endl;
    std::cout << vecMeteo[i].toString() << std::endl;
  }
  std::cout << std::endl;
}

TEST(Meteoio, print_MeteoStations_fixed_period){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);

  mio::Date d1,d2;
  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::IOUtils::convertString(d1, cfg.get("START_DATE", "Dates"), TZ);
  mio::IOUtils::convertString(d2, cfg.get("END_DATE", "Dates"), TZ);
  // Suggested by Mathias Bavay:
  // const mio::Date startdate = cfg.get("START_DATE", "Dates", TZ);
  
  double Tstep;
  cfg.getValue("TSTEP", "Dates",Tstep);
  Tstep /= 24.; // every hour
 
  std::vector<mio::MeteoData> vecMeteo;

  for(; d1<=d2; d1+=Tstep) {
    iomanager.getMeteoData(d1, vecMeteo);
    std::cout << vecMeteo[0].toString() << std::endl;
    std::cout << "------------------------------------" << std::endl;
  }
  
  std::cout << std::endl;
}
