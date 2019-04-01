#ifndef _GEOTOP_METEOIO_H
#define _GEOTOP_METEOIO_H

#include <meteoio/MeteoIO.h>
#include <string>
#include <typeinfo>

class MeteoioWrapper {
 private:
  
 public:
  int a;
  std::string wd = "/home/elisa/Scrivania/MHPC/geotop_3.0/tests/3D/small_example/";
  std::string cfgfile = wd + "io_it.ini";

  void which_cfgfile(){
    std::cout << "cfgfile = " << cfgfile << std::endl;
  }

  void print_types(){
    std::cout << "type(wd) = " << typeid(wd).name() << std::endl;
    std::cout << "type(cfgfile) = " << typeid(cfgfile).name() << std::endl;
  }

    
  // mio::Config cfg(cfgfile); => perchè non va? (1)
  
 MeteoioWrapper():
  a{5}{}
  
};


#endif // _GEOTOP_METEOIO_H
