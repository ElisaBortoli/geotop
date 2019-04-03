#include <gtest/gtest.h>
#include <meteoio_wrapper.h>

#include <iostream>
#include <vector>

// 1D interpolation
TEST(Meteoio, interpolate_1D_TA){
 MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);
  
  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::Date d1(2012,01,20,13,30,TZ);
  mio::IOUtils::convertString(d1,"2012-01-20T13:30:00", TZ);

  std::vector<mio::MeteoData> vecMeteo;
  iomanager.getMeteoData(d1, vecMeteo);

  // print meteo data for station 1
  std::cout << vecMeteo[0].toString() << std::endl;

  // print meteo data for all meteo stations
  // for (unsigned int i=0; i<vecMeteo.size(); i++) {
  //   std::cout << "---------- Station: " << (i+1)
  // 	      << " / " << vecMeteo.size() << std::endl;
  //   std::cout << vecMeteo[i].toString() << std::endl;
  // } 
}
