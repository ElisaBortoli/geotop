#ifndef _GEOTOP_METEOIO_H
#define _GEOTOP_METEOIO_H

#include <meteoio/MeteoIO.h>
#include <string>

class MeteoioWrapper {
  
 public:
  std::string wd = "/home/elisa/Scrivania/MHPC/geotop_3.0/tests/3D/small_example/";
  std::string cfgfile = wd + "io_it.ini";

  void which_cfgfile(){
    std::cout << "cfgfile = " << cfgfile << std::endl;
  }

  // mio::Config cfg(cfgfile); => perchè non va? (1)
  
  MeteoioWrapper(){};
  
};


#endif // _GEOTOP_METEOIO_H
