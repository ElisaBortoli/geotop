
/* Turtle_NetCdf CONTAINS FUNCTIONS TO INPORT/EXPORT FUIDTURLE STRUCT OF DATA IN NETcdf FILES AS VARIABLES
Turtle_NetCdf Version 0.9375 KMackenzie

file turtle2netcdf.c

Copyright, 2009 Stefano Endrizzi, Emanuele Cordano, Matteo Dall'Amico and Riccardo Rigon

This file is part of numerioc_solver.
	 Turtle_NetCdf is free software: you can redistribute it and/or modify
    it under the terms of the GNU  General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

     Turtle_NetCdf is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU  General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*!
 *
 * \file turtle2netcdf.c
 * \data 11 September 2009
 * \author Emanuele Cordano
 */

#include "turtle.h"
#include <netcdf.h>
#include "t_nc_utilities.h"
#include "turtle2netcdf.h"

//DEFINE/UNDEFINE this simbol in Project.Properties.C++ Build.Settings.Gcc C compiler.Defined Symbol
//to write ndetdf in standrd 3 or 4
#ifdef USE_NETCDF4
	#define NEW_EMPTY_FILE NC_CLOBBER|NC_NETCDF4
#else
	#define NEW_EMPTY_FILE NC_CLOBBER
#endif
/* file: turtle2netcdf.c
 * all C commans for NetCDF file management
 * http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c/index.html#Top
 */
/*
 * Handle errors by printing an error message and exiting with a
 * non-zero status.
 */
#define ERRCODE 2

#define ERROR_MESSAGE(e,n_function,n_ncfunction) {printf("Error in %s() function: %s",n_function,n_ncfunction); printf("\nError: %s\n", nc_strerror(e)); exit(ERRCODE);}
#define GLOBAL_ATTRIBUTE "global_attribute"

int t_nc_newemptyfile(const char *filename) {
/*!
 *
 * \param filename - (char *) name of the file to be created
 *
 * \brief This function creates a new NETCdf empty file in the  NC_CLOBBER (overwrite) mode
 * \details http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c/nc_005fcreate.html#nc_005fcreate
 *
 * \return 0 if exit is ok, otherwise an error message.
 *
 * \author Emanuele Cordano
 * \date September 2009
 *

 */

	int status;
	int ncid;

 /* Create the file. The NC_CLOBBER parameter tells netCDF to
	* overwrite this file, if it already exists.*/

	status = nc_create(filename, NEW_EMPTY_FILE, &ncid);
   if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_newemptyfile","nc_create");


 /* End define mode. This tells netCDF we are done defining
  * metadata. */
   status=nc_enddef(ncid);
   if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_newemptyfile","nc_enddef");

    status=nc_close(ncid);
    if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_newemptyfile","nc_close");


	return 0;
}

int t_nc_put_doublevector(DOUBLEVECTOR *v, const char *filename, const char *dimension){
	/*!
	 *\param v - (DOUBLEVECTOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension - (char *) nema of the dimension
	 *
	 *\brief This function write the variable contained in a doublevector within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	int status;
	int ncid;
	int dimid; /* pointer to the dimenson of the doublevector;*/
	int dvar; /* pointer to the variable of the doublevector */
	int ndim=1;
	int dim[1];
	const char *function_name="t_nc_put_doublevector";
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */
/*
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	 if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_redef");
*/

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}


	status=nc_inq_dimid(ncid,dimension,&dimid);

	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension,v->nh, &dimid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_dim");

	}
	dim[0]=dimid;
	status=nc_inq_varid(ncid,v->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,v->name,NC_DOUBLE,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,v->name,dvar);
	}

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_double(ncid,dvar,&(v->element[v->nl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_double");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}


int t_nc_put_doublematrix(DOUBLEMATRIX *m, const char *filename, const char *dimension_x, const char *dimension_y){
	/*!
	 *\param m - (DOUBLEMATRIX *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *
	 *\brief This function write the variable contained in a doublematrix within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	const char *function_name="t_nc_putdoublematrix";
	int status;
	int ncid;
	int dimid_x,dimid_y; /* pointer to the dimenson of the doublematrix;*/
	int dvar; /* pointer to the variable of the doublevector */
	int ndim=2;
	int dim[ndim];
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}
//	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");



	/* putting the dimension */

	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,m->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,m->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}


	dim[0]=dimid_y;
	dim[1]=dimid_x;

	status=nc_inq_varid(ncid,m->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,m->name,NC_DOUBLE,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,m->name,dvar);
	}



	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_double(ncid,dvar,&(m->element[m->nrl][m->ncl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_double");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}






int t_nc_put_var_textattributes(const char *filename,const char *varname, const char *attribute_name, const char *attribute_text){
	/*!
	 *
	 * \author Emanuele Cordano
	 * \date Septemner 2009
	 *\param filename (char *) - complete filename with working path
	 *\param attribute_name (char*) -  Attribute name conventions are assumed by some netCDF generic applications, e.g., �units� as the name for a string attribute that gives the units for a netCDF variable. For examples of attribute conventions see Attribute Conventions.
	 *\param attribute_text (char *) - text of the attributes
	 *
	 *
	 *\brief See http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c/nc_005fput_005fatt_005f-type.html#nc_005fput_005fatt_005f-type
	 */
	int status; /* error status */
	int ncid; /* NETCDF ID */
	int varid; /* Netcdf variable id */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_put_var_textattributes","nc_open");


	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_put_var_textattributes","nc_redef");

	if (!strcmp(varname,GLOBAL_ATTRIBUTE)) {
		varid=NC_GLOBAL; /* set glabal attribues for all variable */
	} else {
		status=nc_inq_varid (ncid,varname,&varid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_put_var_textattributes","nc_inq_varid");
	}
	/* http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c/nc_005fput_005fatt_005f-type.html#nc_005fput_005fatt_005f-type
	*/
	status=nc_put_att_text(ncid,varid,attribute_name,strlen(attribute_text),attribute_text);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_put_var_textattributes","nc_put_att_text");

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_put_var_textattributes","nc_enddef");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_put_var_textattributes","nc_close");

	return 0;

}

//EV_S
int t_nc_put_floatvector(FLOATVECTOR *v, const char *filename, const char *dimension){
	/*!
	 *\param v - (DOUBLEVECTOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension - (char *) name of the dimension
	 *
	 *\brief This function write the variable contained in a floatvector within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	int status;
	int ncid;
	int dimid; /* pointer to the dimenson of the floatvector;*/
	int dvar; /* pointer to the variable of the floatvector */
	int ndim=1;
	int dim[1];
	const char *function_name="t_nc_put_floatvector";
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */
/*
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	 if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_redef");
*/

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}


	status=nc_inq_dimid(ncid,dimension,&dimid);

	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension,v->nh, &dimid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_dim");

	}
	dim[0]=dimid;
	status=nc_inq_varid(ncid,v->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,v->name,NC_FLOAT,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,v->name,dvar);
	}

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_float(ncid,dvar,&(v->element[v->nl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_float");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

int t_nc_put_intvector(INTVECTOR *v, const char *filename, const char *dimension){
	/*!
	 *\param v - (INTVECTOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension - (char *) nema of the dimension
	 *
	 *\brief This function write the variable contained in a intvector within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	int status;
	int ncid;
	int dimid; /* pointer to the dimenson of the intvector;*/
	int dvar; /* pointer to the variable of the intvector */
	int ndim=1;
	int dim[1];
	const char *function_name="t_nc_put_intvector";
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */
/*
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	 if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_redef");
*/

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}


	status=nc_inq_dimid(ncid,dimension,&dimid);

	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension,v->nh, &dimid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_dim");

	}
	dim[0]=dimid;
	status=nc_inq_varid(ncid,v->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,v->name,NC_INT,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,v->name,dvar);
	}

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_int(ncid,dvar,&(v->element[v->nl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_int");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

int t_nc_put_longvector(LONGVECTOR *v, const char *filename, const char *dimension){
	/*!
	 *\param v - (LONGVECTOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension - (char *) nema of the dimension
	 *
	 *\brief This function write the variable contained in a longvector within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	int status;
	int ncid;
	int dimid; /* pointer to the dimenson of the longvector;*/
	int dvar; /* pointer to the variable of the longvector */
	int ndim=1;
	int dim[1];
	const char *function_name="t_nc_put_longvector";
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */
/*
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	 if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_redef");
*/

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}


	status=nc_inq_dimid(ncid,dimension,&dimid);

	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension,v->nh, &dimid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_dim");

	}
	dim[0]=dimid;
	status=nc_inq_varid(ncid,v->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,v->name,NC_LONG,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,v->name,dvar);
	}

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_long(ncid,dvar,&(v->element[v->nl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_long");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

int t_nc_put_shortvector(SHORTVECTOR *v, const char *filename, const char *dimension){
	/*!
	 *\param v - (SHORTVECTOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension - (char *) nema of the dimension
	 *
	 *\brief This function write the variable contained in a shortvector within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	int status;
	int ncid;
	int dimid; /* pointer to the dimenson of the shortvector;*/
	int dvar; /* pointer to the variable of the shortvector */
	int ndim=1;
	int dim[1];
	const char *function_name="t_nc_put_shortvector";
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */
/*
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	 if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"t_nc_adddoublevector","nc_redef");
*/

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}


	status=nc_inq_dimid(ncid,dimension,&dimid);

	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension,v->nh, &dimid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_dim");

	}
	dim[0]=dimid;
	status=nc_inq_varid(ncid,v->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,v->name,NC_SHORT,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,v->name,dvar);
	}

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_short(ncid,dvar,&(v->element[v->nl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_short");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

//CHARVECTOR TBC (unused in Geotop)

int t_nc_put_floatmatrix(FLOATMATRIX *m, const char *filename, const char *dimension_x, const char *dimension_y){
	/*!
	 *\param m - (FLOATMATRIX *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *
	 *\brief This function write the variable contained in a floatmatrix within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	const char *function_name="t_nc_putfloatmatrix";
	int status;
	int ncid;
	int dimid_x,dimid_y; /* pointer to the dimenson of the floatmatrix;*/
	int dvar; /* pointer to the variable of the floatmatrix */
	int ndim=2;
	int dim[ndim];
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}
//	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");



	/* putting the dimension */

	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,m->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,m->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}


	dim[0]=dimid_y;
	dim[1]=dimid_x;

	status=nc_inq_varid(ncid,m->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,m->name,NC_FLOAT,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,m->name,dvar);
	}



	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_float(ncid,dvar,&(m->element[m->nrl][m->ncl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_float");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

int t_nc_put_shortmatrix(SHORTMATRIX *m, const char *filename, const char *dimension_x, const char *dimension_y){
	/*!
	 *\param m - (SHORTMATRIX *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *
	 *\brief This function write the variable contained in a shortmatrix within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	const char *function_name="t_nc_putshortmatrix";
	int status;
	int ncid;
	int dimid_x,dimid_y; /* pointer to the dimenson of the shortmatrix;*/
	int dvar; /* pointer to the variable of the shortmatrix */
	int ndim=2;
	int dim[ndim];
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}
//	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");



	/* putting the dimension */

	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,m->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,m->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}


	dim[0]=dimid_y;
	dim[1]=dimid_x;

	status=nc_inq_varid(ncid,m->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,m->name,NC_SHORT,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,m->name,dvar);
	}



	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_short(ncid,dvar,&(m->element[m->nrl][m->ncl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_short");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

int t_nc_put_intmatrix(INTMATRIX *m, const char *filename, const char *dimension_x, const char *dimension_y){
	/*!
	 *\param m - (INTMATRIX *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *
	 *\brief This function write the variable contained in a intmatrix within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	const char *function_name="t_nc_putintmatrix";
	int status;
	int ncid;
	int dimid_x,dimid_y; /* pointer to the dimenson of the intmatrix;*/
	int dvar; /* pointer to the variable of the intmatrix */
	int ndim=2;
	int dim[ndim];
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}
//	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");



	/* putting the dimension */

	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,m->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,m->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}


	dim[0]=dimid_y;
	dim[1]=dimid_x;

	status=nc_inq_varid(ncid,m->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,m->name,NC_INT,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,m->name,dvar);
	}



	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_int(ncid,dvar,&(m->element[m->nrl][m->ncl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_int");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

int t_nc_put_longmatrix(LONGMATRIX *m, const char *filename, const char *dimension_x, const char *dimension_y){
	/*!
	 *\param m - (LONGMATRIX *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *
	 *\brief This function write the variable contained in a longmatrix within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	const char *function_name="t_nc_putlongmatrix";
	int status;
	int ncid;
	int dimid_x,dimid_y; /* pointer to the dimenson of the longmatrix;*/
	int dvar; /* pointer to the variable of the longmatrix */
	int ndim=2;
	int dim[ndim];
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}
//	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");



	/* putting the dimension */

	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,m->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,m->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}


	dim[0]=dimid_y;
	dim[1]=dimid_x;

	status=nc_inq_varid(ncid,m->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,m->name,NC_LONG,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,m->name,dvar);
	}



	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_long(ncid,dvar,&(m->element[m->nrl][m->ncl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_long");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

int t_nc_put_doubletensor(DOUBLETENSOR *dt, const char *filename, const char *dimension_x, const char *dimension_y, const char *dimension_z){
	/*!
	 *\param dt - (DOUBLETENSOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *\param dimension_z - (char *) name of the z dimension (number of d TBC)
	 *
	 *\brief This function write the variable contained in a doubletensor within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date September 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	const char *function_name="t_nc_put_doubletensor";
	int status;
	int ncid;
	int dimid_x,dimid_y,dimid_z; /* pointer to the dimenson of the doubletensor;*/
	int dvar; /* pointer to the variable of the doubletensor */
	int ndim=3;
	int dim[ndim];
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}
//	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");



	/* putting the dimension */

	/* dimension_z */
	status=nc_inq_dimid(ncid,dimension_z,&dimid_z);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_z,dt->ndh, &dimid_z);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim z");
	}
	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,dt->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim y");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,dt->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim x");
	}


	dim[0]=dimid_z;
	dim[1]=dimid_y;
	dim[2]=dimid_x;

	status=nc_inq_varid(ncid,dt->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,dt->name,NC_DOUBLE,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,dt->name,dvar);
	}



	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	 /* Write the pretend data to the file. Although netCDF supports
	    * reading and writing subsets of data, in this case we write all
	    * the data in one operation. */
	status=nc_put_var_double(ncid,dvar,&(dt->element[dt->nrl][dt->ncl][dt->ndl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_double");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

//EV_E

/* added by Emanuele Cordano on October 13 , 2009 */


int t_nc_put_doublematrix_vs_time(DOUBLEMATRIX *m, long k, const char *filename, const char *dimension_t,  const char *dimension_x, const char *dimension_y){
	/*!
	 *\param m - (DOUBLEMATRIX *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param k        - (long) number of the level (0 based) at which the xy map is printed
	 *\param dimension_x - (char *) name of the t dimension (number of times: UNLIMITED)
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *
	 *\brief This function writes the variable contained in a doublematrix as a map referred to a particular time step 2D + time variable within a NetCDF file
	 *
	 * \author Emanuele Cordano
	 * \date October 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.
	 *  IMPORTANT: only one NC_UNLIMITED dimension allowed
	 *  compile netcdf_lib with --enable-netcdf-4 option tu Turn on netCDF-4 features.
	 *
	 */
	const char *function_name="t_nc_putdoublematrix_vs_time";
	int status;
	int ncid;
	int dimid_t,dimid_x,dimid_y; /* pointer to the dimenson of the doublematrix;*/
	int dvar; /* pointer to the variable of the doublematrix */
	int ndim=3;
	int dim[ndim];
	size_t start[ndim],count[ndim];
	//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}
//	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");



	/* putting the dimension */
	/* dimension_t */
	//EV unlimited dimension requires other function
	status=nc_inq_dimid(ncid,dimension_t,&dimid_t);//COS^ VA MA ne definisce una sola (correggre anche per vect e tens)
	//status=nc_inq_unlimdim(ncid,&dimid_t); //BOH??
	//status=nc_def_dim(ncid,dimension_t,NC_UNLIMITED, &dimid_t); //SOLO PROVa
	if (status!=NC_NOERR) {
		//status=nc_def_dim(ncid,dimension_y,NC_UNLIMITED, &dimid_t);//EV
		status=nc_def_dim(ncid,dimension_t,NC_UNLIMITED, &dimid_t); //EV
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}

	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,m->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,m->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}

	   /* The dim array is used to pass the dimids of the dimensions of
	           the netCDF variables. Both of the netCDF variables we are
	           creating share the same four dimensions. In C, the
	           unlimited dimension must come first on the list of dimids. */
	dim[0] = dimid_t;
	dim[1] = dimid_y;
	dim[2] = dimid_x;
	//        dimids[2] = lat_dimid;
	//        dimids[3] = lon_dimid;

//	dim[0]=dimid_y;
//	dim[1]=dimid_x;

	status=nc_inq_varid(ncid,m->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,m->name,NC_DOUBLE,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,m->name,dvar);
	}



	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");
	/* These settings tell netcdf to write one timestep of data. (The
	   setting of start[0] inside the loop below tells netCDF which
	   timestep to write.) */

	count[0]=1;
	count[1]=m->nrh;
	count[2]=m->nch;
	//The indices are relative to 0, so for example,the first data value of a variable would have index (0, 0, ... , 0).
	start[0]=k;
	start[1]=0;//m->nrl;//EV
	start[2]=0;//m->ncl;//EV

	/* Write the pretend data to the file. Although netCDF supports
	 * reading and writing subsets of data, in this case we write all
	 * the data in one operation. */

	status=nc_put_vara_double(ncid,dvar,start,count,&(m->element[m->nrl][m->ncl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_vara_double");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}
/* end added by Emanuele Cordano on October 13 , 2009 */


//221009_s
int t_nc_put_doublevector_vs_time(DOUBLEVECTOR *v, long k, const char *filename, const char *dimension_t,  const char *dimension_x){
/*!
 *\param v - (DOUBLEVECTOR *) variable to be written in the NetCDF
 *\param fileneme - (char *) name of the NetCDF file
 *\param k        - (long) number of the level (0 based) at which the vector(x) is printed
 *\param dimension_x - (char *) name of the t dimension (number of times: UNLIMITED)
 *\param dimension_x - (char *) name of the x dimension
 *
 *\brief This function writes the variable contained in a vector referred to a particular time step 2D + time variable within a NetCDF file
 *
 * \author Enrico Verri
 * \date October 2009
 *
 * \return 0 if exit is ok, otherwise an error message.
 *  IMPORTANT: only one NC_UNLIMITED dimension allowed
 *  compile netcdf_lib with --enable-netcdf-4 option tu Turn on netCDF-4 features.
 *
 */
const char *function_name="t_nc_put_doublevector_vs_time";
int status;
int ncid;
int dimid_t,dimid_x; /* pointer to the dimension of the doublevector;*/
int dvar; /* pointer to the variable of the doublevector */
int ndim=2;
int dim[ndim];
size_t start[ndim],count[ndim];
//int dim_already_exists=1; /* this flag verifies the existance of the dimensions */

status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
if(status==NC_NOERR) {
	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
} else {
	status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
}

/* putting the dimension */
/* dimension_t */
//EV unlimited dimension requires other function
status=nc_inq_dimid(ncid,dimension_t,&dimid_t);
//status=nc_inq_unlimdim(ncid,&dimid_t);
if (status!=NC_NOERR) {
	status=nc_def_dim(ncid,dimension_t,NC_UNLIMITED, &dimid_t);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
}

/* dimension_x */
status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
if (status!=NC_NOERR) {
	status=nc_def_dim(ncid,dimension_x,v->nh, &dimid_x);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
}

   /* The dim array is used to pass the dimids of the dimensions of
           the netCDF variables. Both of the netCDF variables we are
           creating share the same four dimensions. In C, the
           unlimited dimension must come first on the list of dimids. */
dim[0] = dimid_t;
dim[1] = dimid_x;

status=nc_inq_varid(ncid,v->name,&dvar);
if (status!=NC_NOERR) {
	status=nc_def_var(ncid,v->name,NC_DOUBLE,ndim,dim,&dvar);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
} else {
	printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,v->name,dvar);
}

status=nc_enddef(ncid);
if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");
/* These settings tell netcdf to write one timestep of data. (The
   setting of start[0] inside the loop below tells netCDF which
   timestep to write.) */

count[0]=1;
count[1]=v->nh;
//The indices are relative to 0, so for example,the first data value of a variable would have index (0, 0, ... , 0).
start[0]=k;
start[1]=0;

/* Write the pretend data to the file. Although netCDF supports
 * reading and writing subsets of data, in this case we write all
 * the data in one operation. */

//status=nc_put_vara_double(ncid,dvar,start,count,&(m->element[m->nrl][m->ncl]));
status=nc_put_vara_double(ncid,dvar,start,count,&(v->element[v->nl]));
if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_vara_double");

status=nc_close(ncid);
if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

return 0;

}

int t_nc_put_doubletensor_vs_time(DOUBLETENSOR *t, long k, const char *filename, const char *dimension_t,  const char *dimension_x, const char *dimension_y, const char *dimension_z)
{
	/*!
	 *\param t - (DOUBLETENSOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param k        - (long) number of the level (0 based) at which the xyz map is printed
	 *\param dimension_t - (char *) name of the t dimension (number of times: UNLIMITED)
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *\param dimension_z - (char *) name of the z dimension
	 *
	 *\brief This function writes the variable contained in a doubletensor as a map referred to a particular time step 3D + time variable within a NetCDF file
	 *
	 *  IMPORTANT: only one NC_UNLIMITED dimension allowed
	 *  compile netcdf_lib with --enable-netcdf-4 option tu Turn on netCDF-4 features.
	 *
	 * \author Enrico Verri
	 * \date October 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */
	const char *function_name="t_nc_putdoubletensor_vs_time";
	int status;
	int ncid;
	int dimid_t,dimid_x,dimid_y,dimid_z ;/* pointer to the dimenson of the doubletensor;*/
	int dvar; /* pointer to the variable of the doublematrix */
	int ndim=4;
	int dim[ndim];
	size_t start[ndim],count[ndim];

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}

	/* putting the dimension */
	/* dimension_t */
	//EV unlimited dimension requires other function
	status=nc_inq_dimid(ncid,dimension_t,&dimid_t);
	//status=nc_inq_unlimdim(ncid,&dimid_t);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_t,NC_UNLIMITED, &dimid_t);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_z */
	status=nc_inq_dimid(ncid,dimension_z,&dimid_z);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_z,t->ndh, &dimid_z);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_y */
	status=nc_inq_dimid(ncid,dimension_y,&dimid_y);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_y,t->nrh, &dimid_y);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}
	/* dimension_x */
	status=nc_inq_dimid(ncid,dimension_x,&dimid_x);
	if (status!=NC_NOERR) {
		status=nc_def_dim(ncid,dimension_x,t->nch, &dimid_x);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_inq_dim");
	}

	   /* The dim array is used to pass the dimids of the dimensions of
	           the netCDF variables. Both of the netCDF variables we are
	           creating share the same four dimensions. In C, the
	           unlimited dimension must come first on the list of dimids. */
	dim[0] = dimid_t;
	dim[1] = dimid_z;
	dim[2] = dimid_y;
	dim[3] = dimid_x;

	status=nc_inq_varid(ncid,t->name,&dvar);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid,t->name,NC_DOUBLE,ndim,dim,&dvar);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,t->name,dvar);
	}


	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	/* These settings tell netcdf to write one timestep of data. (The
	   setting of start[0] inside the loop below tells netCDF which
	   timestep to write.) */
	count[0]=1;
	count[1]=t->ndh;
	count[2]=t->nrh;
	count[3]=t->nch;
	//The indices are relative to 0, so for example,the first data value of a variable would have index (0, 0, ... , 0).
	start[0]=k;
	start[1]=0;
	start[2]=0;
	start[3]=0;

	/* Write the pretend data to the file. Although netCDF supports
	 * reading and writing subsets of data, in this case we write all
	 * the data in one operation. */

	status=nc_put_vara_double(ncid,dvar,start,count,&(t->element[t->nrl][t->ncl][t->ndl]));
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_vara_double");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");
	return 0;
}
//221009_e

//201009_s
//SCALAR TYPES
//NC_BYTE
int nc_put_byte(signed char bval, const char *filename,const char *varname, const char *units, const char *description,const char *standard_name,const char *long_name)
{
	/*!
	 *
	 * \param bval (char) - value of the byte variable//
	 * \param filename (const char *) - name of the file
	 * \param varname  (const char *) - name of the variable
	 * \param units (const char *) - name of the measure units
	 * \param description (const char *) - description string
	 *
	 * \author Enrico Verri
	 * \date October 2009
	 *
	 *\save a byte value in the netcdf variable "varname".//
	 */

	const char *function_name="nc_put_byte";//
	int status;
	int ncid;
	int varid; /* pointer to the variable of the variable */
	#  define RANK_VAR 0 /* rank (number of dimensions) for the variable */

	//create or open (if exist) the selected file
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}

	/*check if variable already exists, otherwise define it*/
	status=nc_inq_varid(ncid,varname,&varid);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid, varname, NC_BYTE, RANK_VAR, 0, &varid); //
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,varname,varid);
	}

	/* assign standard cf attributes (Description,Units,standard_name and long_name) */
	status = nc_put_att_text(ncid, varid, "Description",strlen(description),description);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text description");
	status = nc_put_att_text(ncid, varid, "Units", strlen(units), units);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text units");
	status = nc_put_att_text(ncid, varid, "standard_name",strlen(standard_name),standard_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text standard_name");
	status = nc_put_att_text(ncid, varid, "long_name",strlen(long_name),long_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text long_name");

	/* leave define mode */
	status = nc_enddef (ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	//put value
	status = nc_put_var_schar(ncid, varid, &bval); //
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_schar");//

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

// NC_INT
int nc_put_int(int ival, const char *filename,const char *varname, const char *units, const char *description,const char *standard_name,const char *long_name)
{
	/*!
	 *
	 * \param ival (int) - value of the int32 variable //
	 * \param filename (const char *) - name of the file
	 * \param varname  (const char *) - name of the variable
	 * \param units (const char *) - name of the measure units
	 * \param description (const char *) - description string
	 *
	 * \author Enrico Verri
	 * \date October 2009
	 *
	 *\save a int value in the netcdf variable "varname".//
	 */

	const char *function_name="nc_put_int";//
	int status;
	int ncid;
	int varid; /* pointer to the variable of the variable */
	#  define RANK_VAR 0 /* rank (number of dimensions) for the variable */

	//create or open (if exist) the selected file
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}

	/*check if variable already exists, otherwise define it*/
	status=nc_inq_varid(ncid,varname,&varid);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid, varname, NC_INT, RANK_VAR, 0, &varid); //
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,varname,varid);
	}

	/* assign standard cf attributes (Description,Units,standard_name and long_name) */
	status = nc_put_att_text(ncid, varid, "Description",strlen(description),description);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text description");
	status = nc_put_att_text(ncid, varid, "Units", strlen(units), units);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text units");
	status = nc_put_att_text(ncid, varid, "standard_name",strlen(standard_name),standard_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text standard_name");
	status = nc_put_att_text(ncid, varid, "long_name",strlen(long_name),long_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text long_name");

	/* leave define mode */
	status = nc_enddef (ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	//put value
	status = nc_put_var_int(ncid, varid, &ival); //
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_int");//

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

// NC_SHORT
int nc_put_short(short sval, const char *filename,const char *varname, const char *units, const char *description,const char *standard_name,const char *long_name)
{
	/*!
	 *
	 * \param sval (short) - value of the short16 variable//
	 * \param filename (const char *) - name of the file
	 * \param varname  (const char *) - name of the variable
	 * \param units (const char *) - name of the measure units
	 * \param description (const char *) - description string
	 *
	 * \author Enrico Verri
	 * \date October 2009
	 *
	 *\save a short value in the netcdf variable "varname".//
	 */

	const char *function_name="nc_put_short";//
	int status;
	int ncid;
	int varid; /* pointer to the variable of the variable */
	#  define RANK_VAR 0 /* rank (number of dimensions) for the variable */

	//create or open (if exist) the selected file
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}

	/*check if variable already exists, otherwise define it*/
	status=nc_inq_varid(ncid,varname,&varid);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid, varname, NC_SHORT, RANK_VAR, 0, &varid); //
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,varname,varid);
	}

	/* assign standard cf attributes (Description,Units,standard_name and long_name) */
	status = nc_put_att_text(ncid, varid, "Description",strlen(description),description);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text description");
	status = nc_put_att_text(ncid, varid, "Units", strlen(units), units);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text units");
	status = nc_put_att_text(ncid, varid, "standard_name",strlen(standard_name),standard_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text standard_name");
	status = nc_put_att_text(ncid, varid, "long_name",strlen(long_name),long_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text long_name");

	/* leave define mode */
	status = nc_enddef (ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	//put value
	status = nc_put_var_short(ncid, varid, &sval); //
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_short");//

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

// NC_FLOAT
int nc_put_float(float fval, const char *filename,const char *varname, const char *units, const char *description,const char *standard_name,const char *long_name)
{
	/*!
	 *
	 * \param fval (float) - value of the float32 variable//
	 * \param filename (const char *) - name of the file
	 * \param varname  (const char *) - name of the variable
	 * \param units (const char *) - name of the measure units
	 * \param description (const char *) - description string
	 *
	 * \author Enrico Verri
	 * \date October 2009
	 *
	 *\save a float32 value in the netcdf variable "varname".//
	 */

	const char *function_name="nc_put_float";//
	int status;
	int ncid;
	int varid; /* pointer to the variable of the variable */
	#  define RANK_VAR 0 /* rank (number of dimensions) for the variable */

	//create or open (if exist) the selected file
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}

	/*check if variable already exists, otherwise define it*/
	status=nc_inq_varid(ncid,varname,&varid);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid, varname, NC_FLOAT, RANK_VAR, 0, &varid); //
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,varname,varid);
	}

	/* assign standard cf attributes (Description,Units,standard_name and long_name) */
	status = nc_put_att_text(ncid, varid, "Description",strlen(description),description);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text description");
	status = nc_put_att_text(ncid, varid, "Units", strlen(units), units);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text units");
	status = nc_put_att_text(ncid, varid, "standard_name",strlen(standard_name),standard_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text standard_name");
	status = nc_put_att_text(ncid, varid, "long_name",strlen(long_name),long_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text long_name");

	/* leave define mode */
	status = nc_enddef (ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	//put value
	status = nc_put_var_float(ncid, varid, &fval); //
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_float");//

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

// NC_DOUBLE
int nc_put_double(double dval, const char *filename,const char *varname, const char *units, const char *description,const char *standard_name,const char *long_name)
{
	/*!
	 *
	 * \param dval (double) - value of the double64 variable//
	 * \param filename (const char *) - name of the file
	 * \param varname  (const char *) - name of the variable
	 * \param units (const char *) - name of the measure units
	 * \param description (const char *) - description string
	 *
	 * \author Enrico Verri
	 * \date October 2009
	 *
	 *\save a double64 value in the netcdf variable "varname".//
	 */

	const char *function_name="nc_put_double";//
	int status;
	int ncid;
	int varid; /* pointer to the variable of the variable */
	#  define RANK_VAR 0 /* rank (number of dimensions) for the variable */

	//create or open (if exist) the selected file
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}

	/*check if variable already exists, otherwise define it*/
	status=nc_inq_varid(ncid,varname,&varid);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid, varname, NC_DOUBLE, RANK_VAR, 0, &varid); //
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,varname,varid);
	}

	/* assign standard cf attributes (Description,Units,standard_name and long_name) */
	status = nc_put_att_text(ncid, varid, "Description",strlen(description),description);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text description");
	status = nc_put_att_text(ncid, varid, "Units", strlen(units), units);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text units");
	status = nc_put_att_text(ncid, varid, "standard_name",strlen(standard_name),standard_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text standard_name");
	status = nc_put_att_text(ncid, varid, "long_name",strlen(long_name),long_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text long_name");

	/* leave define mode */
	status = nc_enddef (ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	//put value
	status = nc_put_var_double(ncid, varid, &dval); //
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_double");//

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}
// NC_LONG
int nc_put_long(long lval, const char *filename,const char *varname, const char *units, const char *description,const char *standard_name,const char *long_name)
{
	/*!
	 *
	 * \param lval (long) - value of the long64 variable//
	 * \param filename (const char *) - name of the file
	 * \param varname  (const char *) - name of the variable
	 * \param units (const char *) - name of the measure units
	 * \param description (const char *) - description string
	 *
	 * \author Enrico Verri
	 * \date October 2009
	 *
	 *\save a long64 value in the netcdf variable "varname".//
	 */

	const char *function_name="nc_put_long";//
	int status;
	int ncid;
	int varid; /* pointer to the variable of the variable */
	#  define RANK_VAR 0 /* rank (number of dimensions) for the variable */

	//create or open (if exist) the selected file
	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}

	/*check if variable already exists, otherwise define it*/
	status=nc_inq_varid(ncid,varname,&varid);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid, varname, NC_LONG, RANK_VAR, 0, &varid); //
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n",function_name,varname,varid);
	}

	/* assign standard cf attributes (Description,Units,standard_name and long_name) */
	status = nc_put_att_text(ncid, varid, "Description",strlen(description),description);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text description");
	status = nc_put_att_text(ncid, varid, "Units", strlen(units), units);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text units");
	status = nc_put_att_text(ncid, varid, "standard_name",strlen(standard_name),standard_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text standard_name");
	status = nc_put_att_text(ncid, varid, "long_name",strlen(long_name),long_name);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_text long_name");

	/* leave define mode */
	status = nc_enddef (ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	//put value
	status = nc_put_var_long(ncid, varid, &lval); //
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_var_long");//

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

	return 0;

}

//201009_e

/* Emanuele Cordano on October 26 October */

int t_nc_put_rotate180_y_doublematrix(DOUBLEMATRIX *m, const char *filename, const char *dimension_x, const char *dimension_y) {
	/*!
	 *\param m - (DOUBLEMATRIX *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *
     *\brief This function uses the functions rotate180_y_doublematrix() and t_nc_put_doublematrix() to write the  doublematrix within a NetCDF file
	 * \return 0 if exit is ok, otherwise an error message.
	 *
*/

	int l=0;

	char *function_name="t_nc_put_rotate180_y_doublematrix";


	l=rotate180_y_doublematrix(m);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly \n",function_name);
	l=t_nc_put_doublematrix(m,filename,dimension_x,dimension_y);
	if (l!=0) printf("Error in %s: t_nc_put_doublematrix() did not work correctly  \n",function_name);
	l=rotate180_y_doublematrix(m);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly  \n",function_name);

	return l;

}

int t_nc_put_rotate180_y_doubletensor(DOUBLETENSOR *m, const char *filename, const char *dimension_x, const char *dimension_y, const char *dimension_z) {
	/*!
	 *\param m - (DOUBLETENSOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *\param dimension_z - (char *) name of the z dimension (number of layers)
     *\brief This function uses the functions rotate180_y_doubletensor() and t_nc_put_doublematrix() to write the  doublematrix within a NetCDF file
	 * \return 0 if exit is ok, otherwise an error message.
	 *
*/

	int l=0;

	char *function_name="t_nc_put_rotate180_y_doubletensor";

	l=rotate180_y_doubletensor(m);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly \n",function_name);
	l=t_nc_put_doubletensor(m,filename,dimension_x,dimension_y,dimension_z);
	if (l!=0) printf("Error in %s: t_nc_put_doublematrix() did not work correctly \n",function_name);
	l=rotate180_y_doubletensor(m);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly \n",function_name);

	return l;

}

int t_nc_put_rotate180_y_doublematrix_vs_time(DOUBLEMATRIX *m, long k, const char *filename, const char *dimension_t,  const char *dimension_x, const char *dimension_y){

		/*!
		 *\param m - (DOUBLEMATRIX *) variable to be written in the NetCDF
		 *\param fileneme - (char *) name of the NetCDF file
		 *\param k        - (long) number of the level (0 based) at which the xy map is printed
		 *\param dimension_x - (char *) name of the t dimension (number of times: UNLIMITED)
		 *\param dimension_x - (char *) name of the x dimension (number of column)
		 *\param dimension_y - (char *) name of the y dimension (number of row)
		 *
		 *\brief This function uses the functions rotate180_y_doublematrix() and t_nc_put_doublematrix_vs_time() to write the  doublematrix within a NetCDF file
		 *
		 * \author Emanuele Cordano
		 * \date October 2009
		 *
		 * \return 0 if exit is ok, otherwise an error message.
		 *  IMPORTANT: only one NC_UNLIMITED dimension allowed
		 *  compile netcdf_lib with --enable-netcdf-4 option tu Turn on netCDF-4 features.
		 *
		 */


	int l=0;

	char *function_name="t_nc_put_rotate180_y_doublematrix_vs_time";


	l=rotate180_y_doublematrix(m);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly \n",function_name);
	l=t_nc_put_doublematrix_vs_time(m,k,filename,dimension_t,dimension_x,dimension_y);
	if (l!=0) printf("Error in %s: t_nc_put_doublematrix() did not work correctly  \n",function_name);
	l=rotate180_y_doublematrix(m);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly  \n",function_name);

	return l;

}

int t_nc_put_rotate180_y_doubletensor_vs_time(DOUBLETENSOR *t, long k, const char *filename, const char *dimension_t,  const char *dimension_x, const char *dimension_y, const char *dimension_z)
{
	/*!
	 *\param t - (DOUBLETENSOR *) variable to be written in the NetCDF
	 *\param fileneme - (char *) name of the NetCDF file
	 *\param k        - (long) number of the level (0 based) at which the xyz map is printed
	 *\param dimension_t - (char *) name of the t dimension (number of times: UNLIMITED)
	 *\param dimension_x - (char *) name of the x dimension (number of column)
	 *\param dimension_y - (char *) name of the y dimension (number of row)
	 *\param dimension_z - (char *) name of the z dimension
	 *
	 *\brief This function rotates and writes the variable contained in a doubletensor as a map referred to a particular time step 3D + time variable within a NetCDF using the functions rotate180_y_doubletensor() and t_nc_put_doubletensor_vs_time() .
	 *  IMPORTANT: only one NC_UNLIMITED dimension allowed
	 *  compile netcdf_lib with --enable-netcdf-4 option tu Turn on netCDF-4 features.
	 *
	 * \author Enrico Verri,Emanuele Cordano
	 * \date October 2009
	 *
	 * \return 0 if exit is ok, otherwise an error message.

	 *
	 */

	int l=0;

	char *function_name="t_nc_put_rotate180_y_doublematrix_vs_time";


	l=rotate180_y_doubletensor(t);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly \n",function_name);
	l=t_nc_put_doubletensor_vs_time(t,k,filename,dimension_t,dimension_x,dimension_y,dimension_z);
	if (l!=0) printf("Error in %s: t_nc_put_doublematrix() did not work correctly  \n",function_name);
	l=rotate180_y_doubletensor(t);
	if (l!=0) printf("Error in %s: rotate180_y_doublematrix() did not work correctly  \n",function_name);

	return l;


}

//271009_s
//UNUSED
/*!
 *\param fileneme - (char *) name of the NetCDF file to create
 *
 *\brief if USE_NETCDF4 is defined in a configuration with netcdf4 lib
 *\brief create a new NetCDF4 empty file.
 *\brief no action if the file already exist or USE_NETCDF4 is not defined
 *
 * \author Enrico Verri
 * \date October 2009
 *
 * \return 0 if exit is ok, otherwise an error message.

 *
 */

/*int create_empty_netcdf4_file(const char *filename){
#ifdef USE_NETCDF4
	int status;
	int ncid;
	const char *function_name="create_empty_netcdf4_file";

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if(status==NC_NOERR) {
		status=nc_redef(ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");
	} else {
		status=nc_create(filename, NEW_EMPTY_FILE, &ncid);
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_create");
	}


	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");
#endif
	return 0;

}
*/
//271009_e
void nc_add_global_attr_lat_lon_min_max(const char *filename,double long_min,double long_max,double lat_min,double lat_max){
	/*!
	 *
	 * \param filename (const char *) - name of the nc file
	 * \param long_min,long_max,lat_min,lat_max
	 * \author Enrico Verri
	 * \date October 2009
	 */
	const char *function_name="nc_add_global_attr_lat_lon_min_max";//
	int status; /* error status */
	int ncid; /* NETCDF ID */
	double *latmin,*latmax;
	double *lonmin,*lonmax;
	latmin=&lat_min;
	latmax=&lat_max;
	lonmin=&long_min;
	lonmax=&long_max;

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");


	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");

	status=nc_put_att_double(ncid,NC_GLOBAL,GLOB_ATTR_LONG_MIN,NC_DOUBLE,1,lonmin);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_double");

	status=nc_put_att_double(ncid,NC_GLOBAL,GLOB_ATTR_LONG_MAX,NC_DOUBLE,1,lonmax);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_double");

	status=nc_put_att_double(ncid,NC_GLOBAL,GLOB_ATTR_LAT_MIN,NC_DOUBLE,1,latmin);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_double");

	status=nc_put_att_double(ncid,NC_GLOBAL,GLOB_ATTR_LAT_MAX,NC_DOUBLE,1,latmax);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_double");

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

}

void nc_add_variable_attr_missing_value(const char *filename,const char *varname,double missing_value){
	/*!
	 *
	 * \param filename (const char *) - name of the nc file
	 * \param missing_value
	 * \author Enrico Verri
	 * \date November 2009
	 */
	const char *function_name="nc_add_variable_attr_missing_value";//
	int status; /* error status */
	int ncid; /* NETCDF ID */
	int varid;
	double *missingvalue=&missing_value;

	//double water_surface_missing_value[1];
	//water_surface_missing_value[0] = NaN;


	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");

	/*check if variable already exists, otherwise define it*/
	status=nc_inq_varid(ncid,varname,&varid);
	if (status!=NC_NOERR) {
		status=nc_def_var(ncid, varname, NC_DOUBLE, RANK_VAR, 0, &varid); //
		if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_def_var");
	} else {
		printf("Warning in %s (nc_inq_varid) variable %s (id: %d) already exists and will be overwritten \n","nc_add_cf_convention_missing_value",varname,varid);
	}

	status=nc_put_att_double(ncid, varid, ATTR_MISSING_VALUE, NC_DOUBLE, 1, missingvalue);
	//status=nc_put_att_double(ncid,NC_GLOBAL,GLOB_ATTR_MISSING_VALUE,NC_DOUBLE,1,missingvalue);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,"nc_add_cf_convention_missing_value","nc_put_att_double");

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

}

void nc_add_global_attr_missing_value(const char *filename,double missing_value){
	/*!
	 *
	 * \param filename (const char *) - name of the nc file
	 * \param missing_value
	 * \author Enrico Verri
	 * \date October 2009
	 */
	const char *function_name="nc_add_global_attr_missing_value";//
	int status; /* error status */
	int ncid; /* NETCDF ID */
	double *missingvalue;
	missingvalue=&missing_value;

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");

	status=nc_put_att_double(ncid,NC_GLOBAL,GLOB_ATTR_MISSING_VALUE,NC_DOUBLE,1,missingvalue);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_double");

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

}

void nc_add_global_attr_resolution(const char *filename,double map_resolution){
	/*!
	 *
	 * \param filename (const char *) - name of the nc file
	 * \param map_resolution
	 * \author Enrico Verri
	 * \date October 2009
	 */
	const char *function_name="nc_add_global_attr_resolution";//
	int status; /* error status */
	int ncid; /* NETCDF ID */
	double *mapresolution;
	mapresolution=&map_resolution;

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");

	status=nc_put_att_double(ncid,NC_GLOBAL,GLOB_ATTR_MAP_RESOLUTION,NC_DOUBLE,1,mapresolution);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_double");

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

}

void nc_add_global_attr_double(const char *filename,char *attr_name,double attr_value){
	/*!
	 *
	 * \param filename (const char *) - name of the nc file
	 * \param name and value (double) of the global attributes to insert
	 * \author Enrico Verri
	 * \date October 2009
	 */
	const char *function_name="nc_add_global_attr_double";//
	int status; /* error status */
	int ncid; /* NETCDF ID */
	double *attrvalue;
	attrvalue=&attr_value;

	status=nc_open(filename,NC_WRITE|NC_SHARE,&ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_open");

	status=nc_redef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_redef");

	status=nc_put_att_double(ncid,NC_GLOBAL,attr_name,NC_DOUBLE,1,attrvalue);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_put_att_double");

	status=nc_enddef(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_enddef");

	status=nc_close(ncid);
	if (status!=NC_NOERR) ERROR_MESSAGE(status,function_name,"nc_close");

}
