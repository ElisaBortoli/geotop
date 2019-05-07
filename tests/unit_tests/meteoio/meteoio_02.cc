#include <gtest/gtest.h>
#include <meteoio_wrapper.h>

#include <iostream>
#include <vector>

std::string cfgfile = "/home/elisa/Scrivania/MHPC/geotop_3.0/tests/3D/small_example/io_it.ini";

// Data Filtering
TEST(Meteoio, print_MeteoStations_fixed_period_TA_filtered){
  MeteoioWrapper MW {cfgfile};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);

  mio::Date d1,d2;
  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::IOUtils::convertString(d1, cfg.get("START_DATE", "Dates"), TZ);
  mio::IOUtils::convertString(d2, cfg.get("END_DATE", "Dates"), TZ);
  
  double Tstep;
  cfg.getValue("TSTEP", "Dates",Tstep);
  Tstep /= 24.; // every hour
 
  std::vector<mio::MeteoData> vecMeteo;

  // print meteo data for station 1 (TA supposely filtered)
  for(; d1<=d2; d1+=Tstep) {
    iomanager.getMeteoData(d1, vecMeteo);
    std::cout << vecMeteo[0].toString() << std::endl;
    std::cout << "------------------------------------" << std::endl;
  }
  
  std::cout << std::endl;
}

// FILTERING NOT WORKING (METEOIO RUN TIME ERROR)
// [ProcessingBlock.cc:182] InvalidArgumentException:
// Wrong number of arguments for filter/processing element "MIN_MAX"
