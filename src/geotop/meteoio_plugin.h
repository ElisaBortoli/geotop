#ifndef __METEOIOPLUGIN_H__
#define __METEOIOPLUGIN_H__

#include "../geotop/constants.h"
#include <meteoio/MeteoIO.h>
#include <iostream>
#include <vector>
#include "matrix.h"

void copyGridToMatrix(mio::Grid2DObject& gridObject, Matrix<double>* matrix); // (14) copy from MeteoIO to GEOtop

void meteoio_copyDEM(mio::DEMObject& dem, Matrix<double>* matrix); // (3)


#endif
