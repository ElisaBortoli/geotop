
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 1.225-9 'Moab' - 24 Aug 2012
 
 Copyright (c), 2012 - Stefano Endrizzi 
 
 This file is part of GEOtop 1.225-9 'Moab'
 
 GEOtop 1.225-9 'Moab' is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 1.225-9 'Moab' is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */
#ifndef WRITE_ASCII_H
#define WRITE_ASCII_H
#include "../fluidturtle/turtle.h"
#include "extensions.h"


void write_grassascii(char *name, short type, DOUBLEMATRIX *DTM, T_INIT *UV, long novalue);

void write_grassascii_vector(char *name, short type, DOUBLEVECTOR *DTM, long **j, long nr, long nc, T_INIT *UV, long novalue);

void write_esriascii(char *name, short type, DOUBLEMATRIX *DTM, T_INIT *UV, long novalue);

void write_esriascii_vector(char *name, short type, DOUBLEVECTOR *DTM, long **j, long nr, long nc, T_INIT *UV, long novalue);

#endif
