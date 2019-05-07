#include "meteoio_plugin.h"

mio::IOManager *io;
mio::IOManager *io_prepocessing;
mio::DEMObject dem;

void meteoio_init(mio::IOManager &iomanager) // (1)
{
    io = &iomanager;  // pointer to the iomanager instantiated in geotop.cc
    io_prepocessing = nullptr;

    std::string cfgfile = "io_it_extra.ini";
    if (mio::IOUtils::fileExists(cfgfile))
    {
        mio::Config cfg(cfgfile);
        io_prepocessing = new mio::IOManager(cfg);
    }

    try
    {
        io->readDEM(dem);
        std::cout << dem.toString() << std::endl;

    }
    catch (std::exception &e) { std::cerr << "[ERROR] MeteoIO: " << e.what() << std::endl; }
}

