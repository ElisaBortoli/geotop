#include <gtest/gtest.h>
#include <meteoio_wrapper.h>


TEST(Meteoio, wrapper){
  MeteoioWrapper MW {};
  EXPECT_EQ(MW.a, std::size_t{5});
 
}

TEST(Meteoio, read_cfgfile){
  MeteoioWrapper MW {};
  MW.which_cfgfile();
}

TEST(Meteoio, type_cfgfile){
  MeteoioWrapper MW {};
  MW.print_types();
}


TEST(Meteoio, read_DEM){
  MeteoioWrapper MW {};
  mio::Config cfg(MW.cfgfile);
  mio::IOManager iomanager(cfg); 
  mio::DEMObject dem; 
  iomanager.readDEM(dem);
  std::cerr << "DEM information: \n";
  std::cerr << "\tmin=" << dem.grid2D.getMin()
	    << " max=" << dem.grid2D.getMax()
	    << " mean=" << dem.grid2D.getMean() << std::endl;
}

