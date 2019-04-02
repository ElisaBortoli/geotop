#include <gtest/gtest.h>
#include <meteoio_wrapper.h>
#include <vector>

TEST(Meteoio, print_MeteoStations_fixed_day){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg);

  const double TZ = cfg.get("TIME_ZONE", "Input");
  mio::Date d1(2011,10,01,01,00,TZ);
  mio::IOUtils::convertString(d1,"2011-10-01T01:00:00", TZ);

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

// FILTERING NOT WORKING (METEOIO RUN TIME ERROR)
// [ProcessingBlock.cc:182] InvalidArgumentException:
// Wrong number of arguments for filter/processing element "MIN_MAX"
// TEST(Meteoio, print_MeteoStations_fixed_period_filteringRH){
//   MeteoioWrapper MW {};
//   mio::Config cfg(MW.cfgfile);
//   mio::IOManager iomanager(cfg);

//   mio::Date d1,d2;
//   const double TZ = cfg.get("TIME_ZONE", "Input");
//   mio::IOUtils::convertString(d1, cfg.get("START_DATE", "Dates"), TZ);
//   mio::IOUtils::convertString(d2, cfg.get("END_DATE", "Dates"), TZ);
  
//   double Tstep;
//   cfg.getValue("TSTEP", "Dates",Tstep);
//   Tstep /= 24.; // every hour
 
//   std::vector<mio::MeteoData> vecMeteo;

//   for(; d1<=d2; d1+=Tstep) {
//     iomanager.getMeteoData(d1, vecMeteo);
//     std::cout << vecMeteo[0].toString() << std::endl;
//     std::cout << "------------------------------------" << std::endl;
//   }
  
//   std::cout << std::endl;
// }


