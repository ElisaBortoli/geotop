#include <gtest/gtest.h>
#include <meteoio_wrapper.h>

TEST(Meteoio, which_cfgfile){
  MeteoioWrapper MW {};
  MW.which_cfgfile();
  std::cout << std::endl;
}

TEST(Meteoio, type_cfgfile){
  MeteoioWrapper MW {};
  MW.print_types();
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
  std::cout << std::endl;
}

TEST(Meteoio, print_DEM_info){
   MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg); 
  mio::DEMObject dem;

  std::cout << "DEM info" << std::endl;
  std::cout << dem.toString() << std::endl;
}

TEST(Meteoio, read_MeteoStations){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg); 

  mio::Date d1(2011,10,01,01,00,1);  
  std::vector<mio::MeteoData> vecMeteo;

  // small_example: simulated period
  // 01/10/2011 01:00
  // 26/03/2012 00:00

  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::IOUtils::convertString(d1,"2011-10-01T01:00:00", TZ);
  iomanager.getMeteoData(d1, vecMeteo);

  std::cout << vecMeteo.size()
	    << " stations with an average sampling rate of "
	    << iomanager.getAvgSamplingRate()
	    << " or 1 point every " << 1./(iomanager.getAvgSamplingRate()*60.+1e-12)
	    << " minutes" << std::endl;
  std::cout << std::endl;
}
