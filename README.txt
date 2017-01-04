GEOtop
======

GEOtop is a distributed model of the mass and energy balance of the
hydrological cycle, which is applicable to simulations in continuum in
small catchments. GEOtop deals with the effects of topography on the
interaction between energy balance and hydrological cycle with peculiar
solutions.

GEOtop is distributed under the GNU General Public License version 3.
A copy of the license text can be found in the COPYING file.

You can find more informations about GEOtop on the following website

                www.geotop.org 

where the model is briefly described and links to papers and other useful
websites have been collected.

Installation
============

To install GEOtop you need the following libraries and tools:

- CMake 2.8 or later (optional but recommended: ccmake ncurses ui)
- Boost 1.49 or later (filesystem, system, iostreams, regex,
  program_options, unit_test_framework and spirit_classic)
- MeteoIO 2.5.1
  (http://models.slf.ch/p/meteoio/downloads/get/MeteoIO-2.5.1-src.tar.gz)
  (always use tagged relases, avoid using trunk)

GEOtop is known to run under the following Operating Systems:

    - Mac OS X 10.8 or later
    - CentOS 6.5 or later
    - Debian 7
    - Ubuntu 14.04 LTS

GEOtop is known to work under following compilers:
    - GCC
    - Intel compiler 16.0

GEOtop has NOT been tested on Microsoft Windows either with Cygwin or
MinGW compiler. This configuration therefore is NOT supported: you are
on your own.

Compilation options
===================

 Cmake Option                     Default
 BUILD_STATIC                     OFF
 CMAKE_BUILD_TYPE                 RELEASE
 CMAKE_INSTALL_PREFIX             /usr/local
 ENABLE_INTERNAL_METEODISTR       ON
 METEOIO_OUTPUT                   OFF
 METEOIO_PATH                     
 VERBOSE                          OFF
 VERY_VERBOSE                     OFF
 
BUILD_STATIC: If set to ON will build GEOtop as a single static binary.

CMAKE_BUILD_TYPE: Can be RELEASE or DEBUG. The latter sets up GEOtop
logging to high verbosity and enables GDB support.

CMAKE_INSTALL_PREFIX: Sets the "make install" target directory.

ENABLE_INTERNAL_METEODISTR: If set to ON GEOtop will NOT use MeteoIO for
data interpolation.

METEOIO_OUTPUT: If set to ON GEOtop will use MeteoIO to output ARC
files.

METEOIO_PATH: Path to MeteoIO libraries

VERBOSE: increases logging verbosity.

VERY_VERBOSE: increases even more the logging verbosity
