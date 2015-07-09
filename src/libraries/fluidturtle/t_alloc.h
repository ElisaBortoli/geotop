#ifndef T_ALLOC_H
#define T_ALLOC_H

#include "turtle.h"

float *vector(long nl,long nh);
int *ivector(long nl, long nh);
long  *lvector(long nl, long nh) ;
short  *svector(long nl, long nh) ;
double *dvector(long nl, long nh) ;
char *cvector(long nl, long nh);
float **matrix(long nrl, long nrh, long ncl,long nch);
int **imatrix(long nrl, long nrh, long ncl,long nch);
short **smatrix(long nrl, long nrh, long ncl,long nch);
long **lmatrix(long nrl, long nrh, long ncl,long nch);
double **dmatrix(long nrl, long nrh, long ncl,long nch);

/**

Name:  new_
Version: 1.0

Description:
Allocate vectors, matrixes and bind. Strings length is not known apriori.
Thus, the allocation program is one with the reading routine one finds in
t_io.h.

Authors & Date: Paolo D'Odorico, Riccardo Rigon, 1996-1997

Inputs: the number of elements in the  vector, the number of rows and columns matrixes,
 a LONGVECTOR for bins.

Return: a pointer to the allocated structure

*/

SHORTVECTOR *new_shortvector(long);
INTVECTOR *new_intvector(long);
FLOATVECTOR *new_floatvector(long);
LONGVECTOR *new_longvector(long);
DOUBLEVECTOR *new_doublevector(long);
DOUBLEVECTOR *new_doublevector0(long);
CHARVECTOR *new_charvector(long);
SHORTMATRIX *new_shortmatrix( long,long);
INTMATRIX *new_intmatrix( long,long);
FLOATMATRIX *new_floatmatrix( long,long);
LONGMATRIX *new_longmatrix( long,long);
DOUBLEMATRIX *new_doublematrix( long,long);
DOUBLEMATRIX *new_doublematrix0_(long ,long );
DOUBLEMATRIX *new_doublematrix_0(long ,long );

/**

Name: free_

Description:
This section includes the deallocation routines
for the vector, matrixes and bins


Version: 1.0

Authors  & date: Paolo D'Odorico, Riccardo Rigon 1996-1997
FILE: LIBRARIES/BASICS/t_alloc.h, LIBRARIES/BASICS/alloc.c
Inputs: the name of the structure to be deallocated

*/

void free_shortvector( SHORTVECTOR *);
void free_intvector( INTVECTOR *);
void free_longvector( LONGVECTOR *);
void free_floatvector( FLOATVECTOR *);
void free_doublevector( DOUBLEVECTOR *);
void free_charvector( CHARVECTOR *);

void free_shortmatrix( SHORTMATRIX *);
void free_intmatrix( INTMATRIX *);
void free_longmatrix( LONGMATRIX *);
void free_floatmatrix( FLOATMATRIX *);
void free_doublematrix( DOUBLEMATRIX *);

void free_svector(short* v, long nl);
void free_ivector(int* v, long nl);
void free_vector(float * v, long nl);
void free_lvector(long * v, long nl);
void free_dvector(double * v, long nl);
void free_cvector(char * v, long nl);
void free_smatrix(short **m,long nrl,long ncl);
void free_imatrix(int **m,long nrl,long ncl);
void free_matrix(float **m,long nrl,long ncl);
void free_lmatrix(long **m,long nrl,long ncl);
void free_dmatrix(double **m,long nrl,long ncl);
void free_charmatrix(char **m,long nrl,long ncl);


double ***d3tensor( long nrl, long nrh, long ncl, long nch, long ndl, long ndh);
DOUBLETENSOR *new_doubletensor(long nrh,long nch,long ndh);
DOUBLETENSOR *new_doubletensor0(long ndh,long nrh,long nch);

void free_d3tensor(double ***t, long nrl, long ncl, long ndl);
void free_doubletensor( DOUBLETENSOR *m);

double **dmatrix(long nrl, long nrh, long ncl,long nch);
double *dvector(long nl, long nh);
#endif
