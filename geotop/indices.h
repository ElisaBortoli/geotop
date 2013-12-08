
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 1.225 'Moab' - 9 Mar 2012
 
 Copyright (c), 2012 - Stefano Endrizzi 
 
 This file is part of GEOtop 1.225 'Moab'
 
 GEOtop 1.225 'Moab' is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 1.225 'Moab' is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */
#ifndef INDICES_H
#define INDICES_H

#include "struct.geotop.h"

extern long Nl, Nr, Nc;
extern long number_novalue;

void i_lrc_cont(DOUBLEMATRIX *LC, long ***i, LONGMATRIX *lrc);
void i_lrc_cont(GeoMatrix<double>& LC, long ***i, GeoMatrix<long>& lrc);

void j_rc_cont(DOUBLEMATRIX *LC, long **j, LONGMATRIX *rc);
void j_rc_cont(GeoMatrix<double>& LC, long **j, GeoMatrix<long>& rc);

//void lch3_cont(long n, long **ch3, LONGMATRIX *lch);
void lch3_cont(long **ch3, GeoMatrix<long>& lch, long Nl, long nch);

//void cont_nonzero_values_matrix2(long *tot, long *totdiag, CHANNEL *cnet, DOUBLEMATRIX *LC, LONGMATRIX *lrc, long ***i, long n);
void cont_nonzero_values_matrix2(long *tot, long *totdiag, Channel *cnet, GeoMatrix<double>& LC, GeoMatrix<long>& lrc, long ***i, long n);

//void cont_nonzero_values_matrix3(LONGVECTOR *Lp, LONGVECTOR *Li, CHANNEL *cnet, DOUBLEMATRIX *LC, LONGMATRIX *lrc, long ***i, long n);
void cont_nonzero_values_matrix3(GeoVector<long>& Lp, GeoVector<long>& Li, Channel *cnet, GeoMatrix<double>& LC, GeoMatrix<long>& lrc, long ***i, long n);

#endif
