
/* STATEMENT:

GEO_TOP MODELS THE ENERGY AND WATER FLUXES AT LAND SURFACE
GEOtop-Version 0.9375-Subversion MacLavagna

Copyright, 2008 Stefano Endrizzi, Emanuele Cordano, Riccardo Rigon, Matteo Dall'Amico

 LICENSE:

 This file is part of GEOtop 0.9375 MacLavagna.
 GEOtop is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.*/


#include "struct.geotop.09375.h"
#include "liston.h"
#include "input.09375.h"
#include "geomorphology.0875.h"
#include "pedo.funct.h"
#include "write_dem.h"
#include "t_datamanipulation.h"
#include "geo_statistic.09375.h"
#include "t_random.h"
#include "networks.h"
#include "t_utilities.h"
#include "constant.h"
#include "keywords_file.h"

#include "dtm_resolution.h"
#include "rw_maps.h"
#include "extensions.h"
#include "tabs.h"
#include "key.palette.h"

extern T_INIT *UV;
extern char *WORKING_DIRECTORY;
extern STRINGBIN *files;
extern long Nl, Nr, Nc;
extern double NoV;

/*--------------------------------------------------------------------------------------------------*/
//! Subroutine which reads input data, performs  geomporphological analisys and allocates data I_METEO_CLOUDCOVER
void get_all_input(int argc, char *argv[], TOPO *top, SOIL *sl, LAND *land, METEO *met, WATER *wat, CHANNEL *cnet,
					PAR *par, ENERGY *egy, SNOW *snow, GLACIER *glac, TIMES *times, LISTON *liston)

{

 /*counters of layers(l),rows(r),columns(c) and other things(i) (internal variables):*/
 long l,r,c,i,j,n_soiltypes,index,ncols;
 /*auxiliary internal variables:*/
 static short r_DD[11]={0,0,-1,-1,-1,0,1,1,1,-9999,0};/*differential of number-pixel for rows and*/
 static short c_DD[11]={0,1,1,0,-1,-1,-1,0,1,-9999,0};/*columns, depending on Drainage Directions*/
 FILE *f;
 /*auxiliary internal vectors-variables:*/
 DOUBLEMATRIX *SNOW0, *GLACIER0, *M;
 DOUBLEVECTOR *v;
 //Checking variables
 short iland=0, sy;
 double glac0, theta;
 INIT_TOOLS *IT;

 /* Declaration of variables to be read in ___geotop.init & ___geotop.inpts
  * \author Emanuele Cordano
  * \date 13 dec 2008
  * */
 char *keyfile, *inptsfile;
 KEYWORDS *keywords_file;
 short ik;
 FILE *fkey,*finpts;


 /* End Declaration of variables to be read in ___geotop.init & ___geotop.inpts*/

 IT=(INIT_TOOLS *)malloc(sizeof(INIT_TOOLS));
 if(!IT) t_error("IT was not allocated");

/****************************************************************************************************/
/*! Reading of program control par from geo_top.inpts file									*/
/*  (if it is in WORKING_DIRECTORY), otherwise from prompt        									*/
/****************************************************************************************************/
printf("\nGEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE\n");
printf("\nVersion 0.9375 MacLavagna\n");
printf("\nCopyright, 2008 Stefano Endrizzi, Emanuele Cordano, Riccardo Rigon, Matteo Dall'Amico and others following the footsteps of the FREE SOFTWARE FOUNDATION\n");

if(!argv[1]){
	WORKING_DIRECTORY=get_workingdirectory();
}else{
	WORKING_DIRECTORY=argv[1];
}

printf("\nWORKING DIRECTORY: %s",WORKING_DIRECTORY);
/* READ FILENAMES with KeyPalette */
keyfile=join_strings(WORKING_DIRECTORY,"___geotop.init");
inptsfile=join_strings(WORKING_DIRECTORY,"___geotop.inpts");
/* GET KEYWORD FOR EACH FILES */
fkey=t_fopen(keyfile,"r");
ik=read_index(fkey,PRINT); /* read file keywords string */
keywords_file=read_keywords(fkey,PRINT); /* read  keywords for each file */
t_fclose(fkey);
write_keywords(keywords_file);



/* GET FILENAMES */
finpts=t_fopen(inptsfile,"r");
ik=read_index(finpts,PRINT);
files=join_path_to_stringbin(WORKING_DIRECTORY,read_names(finpts,keywords_file->names,NOFILE_NAME,PRINT),"/");


write_read_filenames(files,keywords_file->comments);

t_fclose(finpts);

free_keywords(keywords_file); /* the keywords_file is no more recalled and than it is freed */
/* END READ FILENAMES with KeyPalette */
/* READ INPUT parameters contained on I_CONTROL_PARMETERS */
read_inpts_par(par, times,files->element[I_CONTROL_PARAMETERS]+1, ".txt", "1:");

//files=read_filenames(WORKING_DIRECTORY, "__geotop", ".inpts", "1:");
if(files->index->nh!=nfiles){
	for(i=1;i<=files->index->nh;i++){
		printf("i:%ld file:%s\n",i,files->co[i]+1);
	}
	printf("files given:%ld files required%ld\n",files->index->nh,nfiles);
	t_error("Wrong number of files in .inpts file");
}
for(i=1;i<=nfiles;i++){
	printf("File n:%ld %s\n",i,files->co[i]+1);
}
files->co[ferr]=join_strings(files->co[ferr]+1,textfile)-1;
f=t_fopen(files->co[ferr]+1,"w");
fprintf(f,"Files of the errors in the simulation\n");
t_fclose(f);


/****************************************************************************************************/
/*! Reading of the Input files:                                                                      */
/****************************************************************************************************/
read_parameterfile(files->co[fpar]+1, par, liston, IT);

read_soil_parameters(files->co[fspar]+1, &n_soiltypes, sl);	//it also defines Nl

if(par->point_sim!=1){ /* distributed simulation */
	read_inputmaps(top, land, sl, par);
	read_optionsfile_distr(files->co[fopt]+1, par, times, top->Z0);
}else{
	read_optionsfile_point(files->co[fopt]+1, par, top, land, sl, IT);
}


Nr=top->Z0->nrh;
Nc=top->Z0->nch;
NoV=UV->V->co[2];

//set INIZIAL TIME
times->time=0.0;



/****************************************************************************************************/
//Reading of RAIN data file,	METEO data file 	and CLOUD data file

//METEO DATA
met->st=(METEO_STATIONS *)malloc(sizeof(METEO_STATIONS));
if(!met->st) t_error("meteo_stations was not allocated");
init_meteo_stations(IT->met, met->st);

met->column=alloc_long2(IT->met->nrh);
met->data=(double ***)malloc(IT->met->nrh*sizeof(double**));
met->horizon=(double ***)malloc(IT->met->nrh*sizeof(double**));
met->var=alloc2(met->st->Z->nh,nmet);	//meteo variables for the current instant


for(i=1;i<=IT->met->nrh;i++){
	f=t_fopen(namefile_i(files->co[fmet]+1, i),"r");
	met->column[i-1]=alloc_long1(nmet);
	ReadMeteoHeader(f, IT->met_col_names, met->st->offset->co[i], &ncols, met->column[i-1]);
	met->data[i-1]=read_datameteo(f, met->st->offset->co[i], ncols, UV->V->co[2]);
	met->horizon[i-1]=read_horizon(files->co[fhor]+1, i);
	t_fclose(f);
}


/*double kkk;
f=t_fopen(join_strings(WORKING_DIRECTORY,"___datameteo.txt"),"w");
c=0;
for(i=1;i<=dim2(met->data[0]);i++){
	for(r=0;r<=0;r++){
		if(fmod(i,2)==0){
			if(r!=1){
				kkk=0.5*(met->data[0][i-1][r]+met->data[0][i][r]);
			}else{
				kkk=fmin(0.5*(met->data[0][i-1][r]+met->data[0][i][r]),0.5*(met->data[0][i-1][r]+met->data[0][i][r]+360.0));
				if(kkk>360){
					printf("%f %f\n",met->data[0][i-1][r]+met->data[0][i][r]);
				}
			}
			fprintf(f,"%f",kkk);
			if(r==0)c++;
			if(r==0)printf("%ld\n",c);
			if(r<0)fprintf(f,",");
			if(r==0)fprintf(f,"\n");
		}
	}
}
t_fclose(f);*/

/*double kkk;
f=t_fopen(join_strings(WORKING_DIRECTORY,"___datameteo.txt"),"w");
c=0;
for(i=1;i<=dim2(met->data[0]);i++){
	for(r=0;r<=4;r++){

		if(r!=1){
			kkk=0.5*(met->data[0][i-1][r]+met->data[0][i][r]);
		}else{
			kkk=fmin(0.5*(met->data[0][i-1][r]+met->data[0][i][r]),0.5*(met->data[0][i-1][r]+met->data[0][i][r]+360.0));
		}

		fprintf(f,"%f",kkk);
		if(r==4)c++;
		if(r==4)printf("-%ld\n",c);
		if(r<4)fprintf(f,",");
		if(r==4)fprintf(f,"\n");
	}

	for(r=0;r<=4;r++){

		fprintf(f,"%f",met->data[0][i][r]);
		if(r==4)c++;
		if(r==4)printf("%ld\n",c);
		if(r<4)fprintf(f,",");
		if(r==4)fprintf(f,"\n");
	}
}

t_fclose(f);*/

//FIND A STATION WITH SHORTWAVE RADIATION DATA
met->nstsrad=0;
do{
	met->nstsrad++;
}while(met->nstsrad<met->st->Z->nh && ( met->column[met->nstsrad-1][iSW]!=-1 || (met->column[met->nstsrad-1][iSWb]!=-1 && met->column[met->nstsrad-1][iSWd]!=-1) ));
if(met->column[met->nstsrad-1][iSW]==-1){
	printf("WARNING: NO shortwave radiation measurements available\n");
}else{
	printf("Shortwave radiation measurements from station %ld\n",met->nstsrad);
}

//FIND A STATION WITH CLOUD DATA
met->nstcloud=0;
do{
	met->nstcloud++;
}while(met->nstcloud<met->st->Z->nh && met->column[met->nstcloud-1][iC]!=-1);
if(met->column[met->nstcloud-1][iC]==-1){
	printf("WARNING: NO cloudiness measurements available\n");
}else{
	printf("Cloudiness measurements from station %ld\n",met->nstcloud);
}

//FIND A STATION WITH LONGWAVE RADIATION DATA
met->nstlrad=0;
do{
	met->nstlrad++;
}while(met->nstlrad<met->st->Z->nh && met->column[met->nstlrad-1][iLWi]!=-1);
if(met->column[met->nstlrad-1][iLWi]==-1){
	printf("WARNING: NO longwave radiation measurements available\n");
}else{
	printf("Longwave radiation measurements from station %ld\n",met->nstlrad);
}



/****************************************************************************************************/
/*! Completing the several time-indipendent input variables with the data of input-files:           */
/****************************************************************************************************/

/****************************************************************************************************/
// Completing of "land" (of the type LAND):

//Initialize matrix of shadow
land->shadow=new_shortmatrix(top->Z0->nrh,top->Z0->nch);
initialize_shortmatrix(land->shadow,1);/* initialized as if it was always NOT in shadow*/

//Check that there aren't cell with an undefined land use value
for(r=1;r<=top->Z0->nrh;r++){
	for(c=1;c<=top->Z0->nch;c++){
		if (top->Z0->co[r][c]!=UV->V->co[2]){
			for(i=1;i<=IT->land_classes->nch;i++){
				if (land->use->co[r][c]==IT->land_classes->co[1][i]) iland=1;
			}
			if(iland==0){
				printf("Cell r=%ld c=%ld has an undefined land class value (=%ld)\n",r,c,land->use->co[r][c]);
				t_error("Fatal Error");
			}else{
				iland=0;
			}
		}
	}
}


par->n_landuses=0;
for(i=1;i<=IT->land_classes->nch;i++){
	if(par->n_landuses<(long)IT->land_classes->co[1][i]) par->n_landuses=(long)IT->land_classes->co[1][i];
}
printf("\nNumber of land use types: %ld\n",par->n_landuses);

//properties for each land use
land->ty=new_doublematrix(par->n_landuses,nlandprop);
initialize_doublematrix(land->ty,0.0);

land->LAI=new_doublevector(par->n_landuses);

for(i=1;i<=par->n_landuses;i++){

	if(i==(long)IT->land_classes->co[1][i]){
		for(j=1;j<=nlandprop;j++){
			land->ty->co[i][j]=IT->land_classes->co[j+1][i];
		}
		//z0 (convert in m)
		land->ty->co[i][jz0]*=0.001;
		land->ty->co[i][jd0]*=0.001;
		land->ty->co[i][jhc]*=0.001;
	}

	land->LAI->co[i]=0.5*land->ty->co[i][jLAIw]+0.5*land->ty->co[i][jLAIs];
}


if(par->output_albedo>0){
	land->albedo=new_doublematrix(land->use->nrh,land->use->nch);
	initialize_doublematrix(land->albedo,0.0);
}

land->clax=new_longvector(IT->land_classes->nch);
land->cont=new_longmatrix(IT->land_classes->nch,2);
initialize_longmatrix(land->cont,0);
for(i=1;i<=IT->land_classes->nch;i++){
	land->clax->co[i]=IT->land_classes->co[1][i];
	for(r=1;r<=land->use->nrh;r++){
		for(c=1;c<=land->use->nch;c++){
			if (land->use->co[r][c]!=(UV->V->co[2])){
				if(land->use->co[r][c]==land->clax->co[i]) land->cont->co[i][1]+=1;
			}
		}
	}
}


/****************************************************************************************************/
// Completing of "top" (of the type TOPO):

if(par->point_sim==0){
	top->dz_dx=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	top->dz_dy=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	initialize_doublematrix(top->dz_dx,0.0);
	initialize_doublematrix(top->dz_dy,0.0);
	for(r=1;r<=top->Z0->nrh;r++){
		for(c=1;c<=top->Z0->nch-1;c++){
			if(top->Z0->co[r][c]!=UV->V->co[2]){
				if(top->Z0->co[r][c+1]!=UV->V->co[2]){
					top->dz_dx->co[r][c]=(top->Z0dp->co[r][c]-top->Z0dp->co[r][c+1])/UV->U->co[1];
				}else{
					top->dz_dx->co[r][c]=0.0;
				}
			}
		}
	}
	for(c=1;c<=top->Z0->nch;c++){
		for(r=1;r<=top->Z0->nrh-1;r++){
			if (top->Z0->co[r][c]!=UV->V->co[2]){
				if (top->Z0->co[r+1][c]!=UV->V->co[2]){
					top->dz_dy->co[r][c]=(top->Z0dp->co[r][c]-top->Z0dp->co[r+1][c])/UV->U->co[2];
				}else{
					top->dz_dy->co[r][c]=0.0;
				}
			}
		}
	}
}

/*It is found the total number of pixel which are not novalues:*/
par->total_pixel=0;
for(r=1;r<=top->Z0->nrh;r++){
    for(c=1;c<=top->Z0->nch;c++){
       if (top->Z0->co[r][c]!=UV->V->co[2]) par->total_pixel++;
    }
}
f=fopen(files->co[ferr]+1,"a");
fprintf(f,"Valid pixels: %ld\n",par->total_pixel);
fprintf(f,"Novalue pixels: %ld\n",(top->Z0->nrh*top->Z0->nch-par->total_pixel));
fprintf(f,"Basin area: %12.2f mq\n",(double)par->total_pixel*UV->U->co[1]*UV->U->co[2]);
for(r=1;r<=land->use->nrh;r++){
    for(c=1;c<=land->use->nch;c++){
       if(top->pixel_distance->co[r][c]>0.0 && top->pixel_distance->co[r][c]<UV->U->co[2]){
          fprintf(f,"There is an outlet in the pixel [%ld,%ld], according to the pixel distance!\n",r,c);
		  top->DD->co[r][c]=0;
		  top->pixel_type->co[r][c]=0;
       }
    }
}
fclose(f);


/*! Modification of Dranaige Direction "top->DD" pixels lake (11), sea (12) or novalue (9) are put to the same value (0) */
for(r=1;r<=top->Z0->nrh;r++){
   for(c=1;c<=top->Z0->nch;c++){
      if (top->Z0->co[r][c]!=UV->V->co[2]){
         //if (top->pixel_type->co[r][c]==11 || top->pixel_type->co[r][c]==12) top->DD->co[r][c]=0; /*lake or sea*/
         if (top->DD->co[r][c]==9 || top->DD->co[r][c]>12) top->DD->co[r][c]=0; /*novalue or errors*/
      }else{
         top->DD->co[r][c]=UV->V->co[2];
      }
   }
}
/*! Channels pixels are put to the value (10) and other changes */
for(r=1;r<=top->Z0->nrh;r++){
   for(c=1;c<=top->Z0->nch;c++){
      if (top->pixel_distance->co[r][c]>0.0 && top->pixel_distance->co[r][c]<UV->U->co[2]){
         top->pixel_distance->co[r][c]=UV->U->co[2]/2.0;
         if(top->pixel_type->co[r+r_DD[top->DD->co[r][c]]][c+c_DD[top->DD->co[r][c]]]==9){
            if(top->pixel_type->co[r][c]==10){
               top->DD->co[r][c]=10;
            }else{
               top->DD->co[r][c]=0;
            }
         }
      }
   }
}


/* Creating of the matrix of slope to calculate the surface velocity of the channel incoming flow ("top->i_ch"):*/
 top->i_ch=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 channel_lateral_slope(top->Z0dp,top->DD,UV,top->i_ch);

/****************************************************************************************************/
/*! Filling up of the struct "channel" (of the type CHANNEL):                                        */

/*The number of channel-pixel are counted:*/
i=0;
for(r=1;r<=top->Z0->nrh;r++){
   for(c=1;c<=top->Z0->nch;c++){
      if (top->pixel_type->co[r][c]==10) i++;
   }
}
f=fopen(files->co[ferr]+1,"a");
fprintf(f,"Channel pixels: %ld\n",i);
fclose(f);

/* Creation of the vectors with the position of the channel-pixels ("cnet->r" and "cnet->c"):*/
cnet->r=new_longvector(i);
cnet->c=new_longvector(i);
i=0;
for(r=1;r<=top->Z0->nrh;r++){
   for(c=1;c<=top->Z0->nch;c++){
      if (top->pixel_type->co[r][c]==10){
         i++;
         cnet->r->co[i]=r;
         cnet->c->co[i]=c;
      }
   }
}

/* Initialization of the matrix with the sub/superficial flows which goes in a channel-cell ("cnet->q_sup"):*/
cnet->Q=new_doublevector(cnet->r->nh);

/* Extraction of the vector of channel-pixels distances ("cnet->s0") from the matrix of the distances of
   each pixel from the outlet ("top->pixel_distance")*/
cnet->s0=new_doublevector(cnet->r->nh);
for(i=1;i<=cnet->r->nh;i++){
   cnet->s0->co[i]=top->pixel_distance->co[cnet->r->co[i]][cnet->c->co[i]];
   if (cnet->s0->co[i]<=0.0){
		f=fopen(files->co[ferr]+1,"a");
		fprintf(f,"Warning: negative distance from outlet of the channel pixel %ld, %ld\n",cnet->r->co[i],cnet->c->co[i]);
		fclose(f);
   }
}
if(par->print==1){
   f=fopen(files->co[ferr]+1,"a");
   for(i=1;i<=cnet->r->nh;i++){
      fprintf(f,"cnet->s0->co[i] = %f con i = %ld r = %ld c = %ld\n",cnet->s0->co[i],i,cnet->r->co[i],cnet->c->co[i]);
   }
   fclose(f);
}

/* Creation of the matrix with the coeficent to spread the channel-flow for each channel-pixel ("cnet->fraction_spread"):*/

cnet->fraction_spread=De_Saint_Venant(cnet->s0,IT->u0,IT->D,par->Dt/(double)par->nDt_water);
if(par->print==1){
   f=fopen(join_strings(WORKING_DIRECTORY,"ii_fraction_spread.txt"),"a");
   fprintf(f,"/* Fraction spread calculated with De Saint Venant subroutine */ \n \n");
   for(r=1;r<=cnet->fraction_spread->nrh;r++){
      for(c=1;c<=cnet->fraction_spread->nch;c++){
         fprintf(f,"r=%4ld c=%4ld fraction_spread=%f ",r,c,cnet->fraction_spread->co[r][c]);
      }
      fprintf(f,"\n");
   }
   fclose(f);
}

/* Initialization of the vector with channel-flow (derived from q_sup) for each virtual channel-pixel with the
   same distance from outlet ("cnet->Q_sup_s"); note: vector's dimension is the number of virtual stretches
   of channel (starting from outlet):*/
cnet->Q_sup_s=new_doublevector(cnet->fraction_spread->nch);
initialize_doublevector(cnet->Q_sup_s,0.0);

cnet->Q_sup_spread=new_doublevector(cnet->fraction_spread->nch);

/* Initialization of the vector with channel-flow (derived from q_sub) for each virtual channel-pixel with the
   same distance from outlet ("cnet->Q_sub_s"); note: vector's dimension is the number of virtual stretches
   of channel (starting from outlet):*/
cnet->Q_sub_s=new_doublevector(cnet->fraction_spread->nch);
initialize_doublevector(cnet->Q_sub_s,0.0);

cnet->Q_sub_spread=new_doublevector(cnet->fraction_spread->nch);

if(par->recover==1){
	if(existing_file_text(files->co[rQch]+1)==1){
		f=t_fopen(join_strings(files->co[rQch]+1,textfile),"r");
		index=read_index(f,PRINT);
		M=read_doublematrix(f,"a",PRINT);
		t_fclose(f);
		if(M->nrh!=cnet->fraction_spread->nch) t_error("The number of channel-pixels is not the same as the previous simulation");
		for(i=1;i<=M->nrh;i++){
			cnet->Q_sup_s->co[i]=M->co[i][1];
			cnet->Q_sub_s->co[i]=M->co[i][2];
		}
		free_doublematrix(M);
	}
}



/****************************************************************************************************/
/*! Completing of the initialization of SOIL structure                               */
/****************************************************************************************************/

/****************************************************************************************************/
/*! Initialization of the sl temperature tensor "T" [C]: */

/****************************************************************************************************/
/*! Completing of "sl" (of the type SOIL):                                                    */

sl->thice=new_doubletensor(Nl,Nr,Nc);
initialize_doubletensor(sl->thice,0.0);

sl->Jinf=new_doublematrix(land->use->nrh,land->use->nch);
initialize_doublematrix(sl->Jinf,0.0);

sl->J=new_doubletensor(Nl,Nr,Nc);
initialize_doubletensor(sl->J,0.0);

sl->P=new_doubletensor(Nl,Nr,Nc);
initialize_doubletensor(sl->P,0.0);

sl->T=new_doubletensor(Nl,Nr,Nc);
initialize_doubletensor(sl->T,0.0);

for(r=1;r<=Nr;r++){
	for(c=1;c<=Nc;c++){
		if(top->Z0->co[r][c]!=NoV){
			for(i=1;i<=n_soiltypes;i++){
				if(sl->type->co[r][c]==i){
					sy=sl->type->co[r][c];
					for(l=1;l<=Nl;l++){
						sl->P->co[l][r][c]=sl->pa->co[sy][jpsi][l];
						sl->T->co[l][r][c]=sl->pa->co[sy][jT][l];

						if(sl->T->co[l][r][c]<=Tfreezing && sl->pa->co[sy][jsf][l]==1){
							theta=teta_psi(sl->P->co[l][r][c], 0.0, sl->pa->co[sy][jsat][l], sl->pa->co[sy][jres][l], sl->pa->co[sy][ja][l], sl->pa->co[sy][jns][l],
									1-1/sl->pa->co[sy][jns][l], par->psimin, par->Esoil);
							//Theta_ice=Theta(without freezing) - Theta_unfrozen(in equilibrium with T)
							sl->thice->co[l][r][c]=theta - teta_psi(Psif2(sl->T->co[l][r][c]), 0.0, sl->pa->co[sy][jsat][l],
									sl->pa->co[sy][jres][l], sl->pa->co[sy][ja][l], sl->pa->co[sy][jns][l], 1-1/sl->pa->co[sy][jns][l], par->psimin, par->Esoil);
							//if Theta(without freezing)<Theta_unfrozen(in equilibrium with T) Theta_ice is set at 0
							if(sl->thice->co[l][r][c]<0) sl->thice->co[l][r][c]=0.0;
							//Psi is updated taking into account the freezing
							theta-=sl->thice->co[l][r][c];
							sl->P->co[l][r][c]=psi_teta(theta, sl->thice->co[l][r][c], sl->pa->co[sy][jsat][l], sl->pa->co[sy][jres][l], sl->pa->co[sy][ja][l],
									sl->pa->co[sy][jns][l], 1-1/sl->pa->co[sy][jns][l], par->psimin, par->Esoil);

						}
					}
				}
			}
		}else{

			for(l=1;l<=Nl;l++){
				sl->P->co[l][r][c]=NoV;
				sl->T->co[l][r][c]=NoV;
				sl->thice->co[l][r][c]=NoV;
			}

		}
	}
}


if(par->recover==1){
	for(l=1;l<=Nl;l++){
		assign_recovered(namefile_i_we(files->co[riceg]+1, l), sl->thice->co[l], par, top->Z0, IT->Z);
		assign_recovered(namefile_i_we(files->co[rpsi]+1, l), sl->P->co[l], par, top->Z0, IT->Z);
		assign_recovered(namefile_i_we(files->co[rTg]+1, l), sl->T->co[l], par, top->Z0, IT->Z);
	}
}

/****************************************************************************************************/
/*! Initialization of the struct "egy" (of the type ENERGY):*/

 egy->Hgrid=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 initialize_doublematrix(egy->Hgrid,0.0);
 egy->Tsgrid=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 initialize_doublematrix(egy->Tsgrid,0.0);
 if(par->micromet2==1) egy->SWin=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 if(par->micromet3==1) egy->LWin=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 if(par->output_Rn>0){
	egy->Rn_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->Rn_min=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->Rn_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->LW_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->LW_min=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->LW_in=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->LW_out=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->SW=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->SW_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 }
 if(par->output_ET>0){
	egy->ET_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->ET_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->ET_min=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 }
 if(par->output_H>0){
	egy->H_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->H_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->H_min=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 }
 if(par->output_G>0){
	egy->G_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->G_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->G_min=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->G_snowsoil=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 }
 if(par->output_Ts>0){
	egy->Ts_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->Ts_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->Ts_min=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 }
 if(par->output_Rswdown>0){
	egy->Rswdown_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->Rswdown_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->Rswbeam=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 }

 egy->nDt_shadow=new_longmatrix(top->Z0->nrh,top->Z0->nch);
 egy->nDt_sun=new_longmatrix(top->Z0->nrh,top->Z0->nch);
 initialize_longmatrix(egy->nDt_shadow,0);
 initialize_longmatrix(egy->nDt_sun,0);

 if(par->output_meteo>0){
	egy->Ta_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->Ta_max=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->distr_stat==1)egy->Ta_min=new_doublematrix(top->Z0->nrh,top->Z0->nch);
 }

 for(r=1;r<=top->Z0->nrh;r++){
	for(c=1;c<=top->Z0->nch;c++){
		if(top->Z0->co[r][c]!=UV->V->co[2]){

			if(par->output_Rn>0){
				egy->Rn_mean->co[r][c]=0.0;
				if(par->distr_stat==1)egy->Rn_max->co[r][c]=-9.0E99;
				if(par->distr_stat==1)egy->Rn_min->co[r][c]=+9.0E99;
				if(par->distr_stat==1)egy->LW_min->co[r][c]=+9.0E99;
				if(par->distr_stat==1)egy->LW_max->co[r][c]=-9.0E99;
				egy->LW_in->co[r][c]=0.0;
				egy->LW_out->co[r][c]=0.0;
				egy->SW->co[r][c]=0.0;
				if(par->distr_stat==1)egy->SW_max->co[r][c]=-9.0E99;
			}
			if(par->output_ET>0){
				egy->ET_mean->co[r][c]=0.0;
				if(par->distr_stat==1)egy->ET_max->co[r][c]=-9.0E99;
				if(par->distr_stat==1)egy->ET_min->co[r][c]=+9.0E99;
			}
			if(par->output_H>0){
				egy->H_mean->co[r][c]=0.0;
				if(par->distr_stat==1)egy->H_max->co[r][c]=-9.0E99;
				if(par->distr_stat==1)egy->H_min->co[r][c]=+9.0E99;
			}
			if(par->output_G>0){
				egy->G_mean->co[r][c]=0.0;
				if(par->distr_stat==1)egy->G_max->co[r][c]=-9.0E99;
				if(par->distr_stat==1)egy->G_min->co[r][c]=+9.0E99;
				egy->G_snowsoil->co[r][c]=0.0;
			}
			if(par->output_Ts>0){
				egy->Ts_mean->co[r][c]=0.0;
				if(par->distr_stat==1)egy->Ts_max->co[r][c]=-9.0E99;
				if(par->distr_stat==1)egy->Ts_min->co[r][c]=+9.0E99;
			}
			if(par->output_Rswdown>0){
				egy->Rswdown_mean->co[r][c]=0.0;
				if(par->distr_stat==1)egy->Rswdown_max->co[r][c]=-9.0E99;
				egy->Rswbeam->co[r][c]=0.0;
			}
			if(par->output_meteo>0){
				egy->Ta_mean->co[r][c]=0.0;
				if(par->distr_stat==1)egy->Ta_max->co[r][c]=-9.0E99;
				if(par->distr_stat==1)egy->Ta_min->co[r][c]=+9.0E99;
			}

		}else{

			if(par->output_Rn>0){
				egy->Rn_mean->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->Rn_max->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->Rn_min->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->LW_min->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->LW_max->co[r][c]=UV->V->co[2];
				egy->LW_in->co[r][c]=UV->V->co[2];
				egy->LW_out->co[r][c]=UV->V->co[2];
				egy->SW->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->SW_max->co[r][c]=UV->V->co[2];
			}
			if(par->output_ET>0){
				egy->ET_mean->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->ET_max->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->ET_min->co[r][c]=UV->V->co[2];
			}
			if(par->output_H>0){
				egy->H_mean->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->H_max->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->H_min->co[r][c]=UV->V->co[2];
			}
			if(par->output_G>0){
				egy->G_mean->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->G_max->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->G_min->co[r][c]=UV->V->co[2];
				egy->G_snowsoil->co[r][c]=UV->V->co[2];
			}
			if(par->output_Ts>0){
				egy->Ts_mean->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->Ts_max->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->Ts_min->co[r][c]=UV->V->co[2];
			}
			if(par->output_Rswdown>0){
				egy->Rswdown_mean->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->Rswdown_max->co[r][c]=UV->V->co[2];
				egy->Rswbeam->co[r][c]=UV->V->co[2];
			}
			if(par->output_meteo>0){
				egy->Ta_mean->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->Ta_max->co[r][c]=UV->V->co[2];
				if(par->distr_stat==1)egy->Ta_min->co[r][c]=UV->V->co[2];
			}
		}
	}
 }

 egy->out1=new_doublematrix(58,par->chkpt->nrh);
 initialize_doublematrix(egy->out1,0.0);

 egy->out2=new_doublevector(11);
 initialize_doublevector(egy->out2,0.0);

 if(par->ES_num>0){
	egy->out3=new_doublematrix(23,par->ES_num);
	initialize_doublematrix(egy->out3,0.0);
 }

 if(par->JD_plots->co[1]!=0){
	egy->Hplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->LEplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->SWinplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->SWoutplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->LWinplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->LWoutplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	egy->Tsplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	for(r=1;r<=top->Z0->nrh;r++){
		for(c=1;c<=top->Z0->nch;c++){
			if(top->Z0->co[r][c]==UV->V->co[2]){
				egy->Hplot->co[r][c]=UV->V->co[2];
				egy->LEplot->co[r][c]=UV->V->co[2];
				egy->SWinplot->co[r][c]=UV->V->co[2];
				egy->SWoutplot->co[r][c]=UV->V->co[2];
				egy->LWinplot->co[r][c]=UV->V->co[2];
				egy->LWoutplot->co[r][c]=UV->V->co[2];
				egy->Tsplot->co[r][c]=UV->V->co[2];
			}else{
				egy->Hplot->co[r][c]=0.0;
				egy->LEplot->co[r][c]=0.0;
				egy->SWinplot->co[r][c]=0.0;
				egy->SWoutplot->co[r][c]=0.0;
				egy->LWinplot->co[r][c]=0.0;
				egy->LWoutplot->co[r][c]=0.0;
				egy->Tsplot->co[r][c]=0.0;
			}
		}
	}
}

egy->VSFA=1.0;
egy->HSFA=0.0;
if(par->recover==1){
	f=t_fopen(join_strings(files->co[rSFA]+1,textfile),"r");
	index=read_index(f,PRINT);
	v=read_doublearray(f,PRINT);
	t_fclose(f);
	egy->VSFA=v->co[1];
	egy->HSFA=v->co[2];
	free_doublevector(v);
}

/****************************************************************************************************/
/*! Completing of the struct "water" (of the type WATER) with the initializations of the remanent
    matrices (wat->rain, wat->Pn, wat->wt):        */

/* Initialization of wat->Pn (liquid precipitation that reaches the sl surface in mm):*/
wat->Pn=new_doublematrix(land->use->nrh,land->use->nch);
initialize_doublematrix(wat->Pn,0.0);

/* Initialization of wat->wt: (liquid precipitation intercepted by vegetation in mm):*/
wat->wt=new_doublematrix(land->use->nrh,land->use->nch);

/* Initialization of wat->h_sup: (height of water over the sl-surface not infiltrated in mm):*/
wat->h_sup=new_doublematrix(land->use->nrh,land->use->nch);
wat->q_sup=new_doublematrix(land->use->nrh,land->use->nch);

/* Initialization of wat->total (total precipitation (rain+snow) precipitation):*/
wat->total=new_doublematrix(land->use->nrh,land->use->nch);
initialize_doublematrix(wat->total,0.0);

wat->Psnow=new_doublematrix(land->use->nrh,land->use->nch);

/* Initialization of the matrices with the output of total precipitation and interception:*/
if(par->output_P>0){
	wat->PrTOT_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	wat->PrSNW_mean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
}

if(par->output_h_sup>0) wat->hsupav=new_doublematrix(top->Z0->nrh,top->Z0->nch);

for(r=1;r<=top->Z0->nrh;r++){
	for(c=1;c<=top->Z0->nch;c++){
		if(top->Z0->co[r][c]!=UV->V->co[2]){
			wat->wt->co[r][c]=0.0;
			wat->h_sup->co[r][c]=0.0;
			if(par->output_P>0){
				wat->PrTOT_mean->co[r][c]=0.0;
				wat->PrSNW_mean->co[r][c]=0.0;
			}
			if(par->output_h_sup>0) wat->hsupav->co[r][c]=0.0;
		}else{
			wat->wt->co[r][c]=UV->V->co[2];
			wat->h_sup->co[r][c]=UV->V->co[2];
			if(par->output_P>0){
				wat->PrTOT_mean->co[r][c]=UV->V->co[2];
				wat->PrSNW_mean->co[r][c]=UV->V->co[2];
			}
			if(par->output_h_sup>0) wat->hsupav->co[r][c]=UV->V->co[2];
		}
	}
}

if(par->recover==1){
	assign_recovered(files->co[rhsup]+1, wat->h_sup->co, par, top->Z0, IT->Z);
	assign_recovered(files->co[rwt]+1, wat->wt->co, par, top->Z0, IT->Z);
}


/* Initialization of vector "wat->output" with basin's means and pixel's values for the output:*/
wat->out1=new_doublematrix(28,par->chkpt->nrh);
initialize_doublematrix(wat->out1,0.0);
wat->out2=new_doublevector(8);
initialize_doublevector(wat->out2,0.0);
wat->outfluxes=new_doublematrix(6,IT->land_classes->nch);
initialize_doublematrix(wat->outfluxes,0.0);


/* Creation of Kriging weights to model the rainfall distribution ("wat->weights_Kriging"):*/
wat->weights_Kriging=new_doublematrix((land->use->nrh*land->use->nch), met->st->Z->nh);
initialize_doublematrix(wat->weights_Kriging, 0.999999);
/* Call of the function ordi_kriging in geo_statistic.c:
	Output: 1) matrix with kriging weights (DOUBLEMATRIX [NR*NC,number of station] - wat->weights_Kriging)
	Input:  2) matrix with the coordinates of rain stations (par->rain_stations)
			3) matrix with elevations (top->Z0)
			4) vector with pixel dimension (UV->U)
			5) vector with turtle file header (UV->V)
			6) integral spatial scale (scala_integr)
			7) spatial variance (varianza) */
ordi_kriging2(wat->weights_Kriging, met->st->E, met->st->N, top->Z0, UV, par->integr_scale_rain, par->variance_rain);
//doublematrix_dem3(wat->weights_Kriging,UV->U,UV->V,"ii_kriging_weights.txt","Doublematrix of krigingweights",par->print);




/****************************************************************************************************/
/*! Initialization of the struct "snow" (of the type SNOW):*/

/***************************************************************************************************/
/*! Optional reading of initial real snow thickness map in the whole basin ("SNOW0"):    */
if (par->state_snow==1){
	SNOW0=read_map(2, files->co[fsn0]+1, top->Z0, UV);
}else{
	SNOW0=copydoublematrix_const(IT->Dsnow0, top->Z0, UV->V->co[2]);
}

/*! Optional reading of snow age in the whole basin     */
if (par->state_snow_age==1){
	snow->age=read_map(2, files->co[fsnag0]+1, top->Z0, UV);
}else{
    snow->age=copydoublematrix_const(IT->agesnow0, top->Z0, UV->V->co[2]);
}

if(par->JD_plots->co[1]!=0){
	snow->Dplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	for(r=1;r<=top->Z0->nrh;r++){
		for(c=1;c<=top->Z0->nch;c++){
			if(top->Z0->co[r][c]==UV->V->co[2]){
				snow->Dplot->co[r][c]=UV->V->co[2];
			}else{
				snow->Dplot->co[r][c]=0.0;
			}
		}
	}
}

snow->CR1=new_doublevector(par->snowlayer_max);
snow->CR2=new_doublevector(par->snowlayer_max);
snow->CR3=new_doublevector(par->snowlayer_max);
snow->CR1m=new_doublevector(par->snowlayer_max);
snow->CR2m=new_doublevector(par->snowlayer_max);
snow->CR3m=new_doublevector(par->snowlayer_max);
initialize_doublevector(snow->CR1m,0.0);
initialize_doublevector(snow->CR2m,0.0);
initialize_doublevector(snow->CR3m,0.0);

snow->type=new_shortmatrix(land->use->nrh,land->use->nch);
initialize_shortmatrix(snow->type,0);

snow->lnum=new_longmatrix(land->use->nrh,land->use->nch);
initialize_longmatrix(snow->lnum,0);

snow->Dzl=new_doubletensor(par->snowlayer_max,land->use->nrh,land->use->nch);   /*in mm*/
initialize_doubletensor(snow->Dzl,0.0);

snow->w_liq=new_doubletensor(par->snowlayer_max,land->use->nrh,land->use->nch);
initialize_doubletensor(snow->w_liq,0.0);

snow->w_ice=new_doubletensor(par->snowlayer_max,land->use->nrh,land->use->nch);
initialize_doubletensor(snow->w_ice,0.0);

snow->T=new_doubletensor(par->snowlayer_max,land->use->nrh,land->use->nch);
initialize_doubletensor(snow->T,-99.0);

snow->rho_newsnow=new_doublematrix(land->use->nrh,land->use->nch);
initialize_doublematrix(snow->rho_newsnow,UV->V->co[2]);

if(par->snowtrans==1){
	snow->Wsalt=new_doublematrix(land->use->nrh,land->use->nch);
	snow->Wsusp=new_doublematrix(land->use->nrh,land->use->nch);
	snow->Wsubl=new_doublematrix(land->use->nrh,land->use->nch);
	snow->Wsubgrid=new_doublematrix(land->use->nrh,land->use->nch);
	snow->ListonSWE=new_doublematrix(land->use->nrh,land->use->nch);
	snow->softSWE=new_doublematrix(land->use->nrh,land->use->nch);
	initialize_doublematrix(snow->softSWE,UV->V->co[2]);
	snow->softSWE1=new_doublematrix(land->use->nrh,land->use->nch);
	if(par->output_snow>0){
		snow->Wtot_cum=new_doublematrix(land->use->nrh,land->use->nch);
		snow->Wsalt_cum=new_doublematrix(land->use->nrh,land->use->nch);
		snow->Wsusp_cum=new_doublematrix(land->use->nrh,land->use->nch);
		snow->Wsubl_cum=new_doublematrix(land->use->nrh,land->use->nch);
		snow->Wsubgrid_cum=new_doublematrix(land->use->nrh,land->use->nch);
		for(r=1;r<=top->Z0->nrh;r++){
			for(c=1;c<=top->Z0->nch;c++){
				if(top->Z0->co[r][c]==UV->V->co[2]){
					snow->Wtot_cum->co[r][c]=UV->V->co[2];
					snow->Wsalt_cum->co[r][c]=UV->V->co[2];
					snow->Wsusp_cum->co[r][c]=UV->V->co[2];
					snow->Wsubl_cum->co[r][c]=UV->V->co[2];
					snow->Wsubgrid_cum->co[r][c]=UV->V->co[2];
				}else{
					snow->Wtot_cum->co[r][c]=0.0;
					snow->Wsalt_cum->co[r][c]=0.0;
					snow->Wsusp_cum->co[r][c]=0.0;
					snow->Wsubl_cum->co[r][c]=0.0;
					snow->Wsubgrid_cum->co[r][c]=0.0;
				}
			}
		}
	}
}
snow->out_bs=new_doublematrix(10,par->chkpt->nrh);
initialize_doublematrix(snow->out_bs,0.0);
snow->evap=new_doublevector(par->chkpt->nrh);
initialize_doublevector(snow->evap,0.0);
snow->subl=new_doublevector(par->chkpt->nrh);
initialize_doublevector(snow->subl,0.0);
snow->melted=new_doublevector(par->chkpt->nrh);
initialize_doublevector(snow->melted,0.0);

if(par->output_balancesn>0){
	snow->MELTED=new_doublematrix(Nr,Nc);
	initialize_doublematrix(snow->MELTED,NoV);
	snow->SUBL=new_doublematrix(Nr,Nc);
	initialize_doublematrix(snow->SUBL,NoV);
	snow->t_snow=new_doublematrix(Nr,Nc);
	initialize_doublematrix(snow->t_snow,NoV);
	snow->totav_snow=new_doublematrix(Nr,Nc);
	initialize_doublematrix(snow->totav_snow,NoV);
}

if(par->output_snow>0){
	snow->max=new_doublematrix(Nr,Nc);
	initialize_doublematrix(snow->max,NoV);
	snow->average=new_doublematrix(Nr,Nc);
	initialize_doublematrix(snow->average,NoV);
}


for(r=1;r<=Nr;r++){
	for(c=1;c<=Nc;c++){

		if(top->Z0->co[r][c]!=UV->V->co[2]){

			if(SNOW0->co[r][c]<0){
				t_error("Error: negative snow data");
			}else if(SNOW0->co[r][c]<0.00001){
				SNOW0->co[r][c]=0.0;
				snow->type->co[r][c]=0;
				snow->age->co[r][c]=0.0;
			}else if(SNOW0->co[r][c]<par->Dmin->co[1]){
				snow->lnum->co[r][c]=1;
				snow->Dzl->co[1][r][c]=SNOW0->co[r][c];
				snow->type->co[r][c]=1;
				snow->age->co[r][c]*=86400.0;	//now in [s]
			}else{
				//just a first guess
				snow->type->co[r][c]=2;
				snow->lnum->co[r][c]=par->snowlayer_max;
				for(l=1;l<=par->snowlayer_max;l++){
					snow->Dzl->co[l][r][c]=SNOW0->co[r][c]/par->snowlayer_max;
				}
				snow->age->co[r][c]*=86400.0;	//now in [s]
			}

			if(snow->lnum->co[r][c]>0){
				for(l=1;l<=snow->lnum->co[r][c];l++){
					snow->w_ice->co[l][r][c]=IT->rhosnow0*0.001*snow->Dzl->co[l][r][c];
					snow->T->co[l][r][c]=-1.0;
				}
			}

			if(par->output_snow>0){
				snow->max->co[r][c]=0.0;
				snow->average->co[r][c]=0.0;
			}

			if(par->output_balancesn>0){
				snow->MELTED->co[r][c]=0.0;
				snow->SUBL->co[r][c]=0.0;
				snow->t_snow->co[r][c]=0.0;
				snow->totav_snow->co[r][c]=0.0;
			}
		}
	}
}


if(par->recover==1){

	for(r=1;r<=Nr;r++){
		for(c=1;c<=Nc;c++){
			snow->type->co[r][c]=2;
		}
	}

	assign_recovered_long(files->co[rns]+1, snow->lnum->co, par, top->Z0, IT->Z);
	assign_recovered(files->co[rsnag]+1, snow->age->co, par, top->Z0, IT->Z);

    for(l=1;l<=par->snowlayer_max;l++){
		if(l<=3){
			assign_recovered(namefile_i_we(files->co[rDzs]+1, l), snow->Dzl->co[l], par, top->Z0, IT->Z);
			assign_recovered(namefile_i_we(files->co[rwls]+1, l), snow->w_liq->co[l], par, top->Z0, IT->Z);
			assign_recovered(namefile_i_we(files->co[rwis]+1, l), snow->w_ice->co[l], par, top->Z0, IT->Z);
			assign_recovered(namefile_i_we(files->co[rTs]+1, l), snow->T->co[l], par, top->Z0, IT->Z);
		}else{
			for(r=1;r<=Nr;r++){
				for(c=1;c<=Nc;c++){

					snow->Dzl->co[l][r][c]=0.0;
					snow->w_ice->co[l][r][c]=0.0;
					snow->w_liq->co[l][r][c]=0.0;
					snow->T->co[l][r][c]=-99.0;

				}
			}

		}
	}
}


/****************************************************************************************************/
/*! Initialization of the struct "glac" (of the type GLACIER):*/

/***************************************************************************************************/

/*! Optional reading of glacier depth in the whole basin ("GLACIER0"):    */
if(par->glaclayer_max>0){
	if(par->state_glac==1){
		GLACIER0=read_map(2, files->co[fgl0]+1, top->Z0, UV);
	}else{
		GLACIER0=copydoublematrix_const(IT->Dglac0, top->Z0, UV->V->co[2]);
		for(r=1;r<=top->Z0->nrh;r++){
			for(c=1;c<=top->Z0->nch;c++){
				if(top->Z0->co[r][c]==UV->V->co[2]){
					GLACIER0->co[r][c]=(double)UV->V->co[2];
				}else{
					GLACIER0->co[r][c]=(double)IT->Dglac0;
				}
			}
		}
	}

}else{
	//check
	if(IT->Dglac0>0){
		f=fopen(files->co[ferr]+1,"a");
		printf("\nWARNING: You have chosen 0 glacier layers in block 10 in the parameter file, but you assigned a value of the glacier depth. The latter will be ignored.\n");
		fprintf(f,"\nWARNING: You have chosen 0 glacier layers in block 10 in the parameter file, but you assigned a value of the glacier depth. The latter will be ignored.\n");
		fclose(f);
	}
	if(par->state_glac==1){
		f=fopen(files->co[ferr]+1,"a");
		printf("\nWARNING: You have chosen 0 glacier layers in block 10 in the parameter file, but you assigned a distributed glacier depth. The latter will be ignored.\n");
		fprintf(f,"\nWARNING: You have chosen 0 glacier layers in block 10 in the parameter file, but you assigned a distributed glacier depth. The latter will be ignored.\n");
		fclose(f);
	}

}

glac->evap=new_doublevector(par->chkpt->nrh);
glac->subl=new_doublevector(par->chkpt->nrh);
glac->melted=new_doublevector(par->chkpt->nrh);
initialize_doublevector(glac->evap,0.0);
initialize_doublevector(glac->subl,0.0);
initialize_doublevector(glac->melted,0.0);

//If the max number of glacier layers is greater than 1, the matrices (or tensors) lnum, Dzl. w_liq, w_ice, T and print matrices are defined, according to the respective flags
if(par->glaclayer_max>0){

	glac->lnum=new_longmatrix(land->use->nrh,land->use->nch);
	initialize_longmatrix(glac->lnum,0);

	glac->Dzl=new_doubletensor(par->glaclayer_max,land->use->nrh,land->use->nch);   /*in mm*/
	initialize_doubletensor(glac->Dzl,0.0);

	glac->w_liq=new_doubletensor(par->glaclayer_max,land->use->nrh,land->use->nch);
	initialize_doubletensor(glac->w_liq,0.0);

	glac->w_ice=new_doubletensor(par->glaclayer_max,land->use->nrh,land->use->nch);
	initialize_doubletensor(glac->w_ice,0.0);

	glac->T=new_doubletensor(par->glaclayer_max,land->use->nrh,land->use->nch);
	initialize_doubletensor(glac->T,-99.0);

	if(par->output_balancegl>0){
		glac->MELTED=new_doublematrix(land->use->nrh,land->use->nch);
		initialize_doublematrix(glac->MELTED,NoV);
		glac->SUBL=new_doublematrix(land->use->nrh,land->use->nch);
		initialize_doublematrix(glac->SUBL,NoV);
	}

	for(r=1;r<=land->use->nrh;r++){
		for(c=1;c<=land->use->nch;c++){
			if(top->Z0->co[r][c]!=UV->V->co[2]){

				if(GLACIER0->co[r][c]<0){
					t_error("Error: negative glacier data");

				}else if(GLACIER0->co[r][c]<10){
					GLACIER0->co[r][c]=0.0;
					glac->lnum->co[r][c]=0;
					for(l=1;l<=par->glaclayer_max;l++){
						glac->Dzl->co[l][r][c]=0.0;
						glac->w_ice->co[l][r][c]=0.0;
						GLACIER0->co[r][c]=0.0;
					}

				}else{
					//just a first guess
					glac->lnum->co[r][c]=par->glaclayer_max;
					for(l=1;l<=par->glaclayer_max;l++){
						glac->Dzl->co[l][r][c]=GLACIER0->co[r][c]/par->glaclayer_max;
						glac->w_ice->co[l][r][c]=IT->rhoglac0*0.001*glac->Dzl->co[l][r][c];
						//maximum liquid content considered
						glac->w_liq->co[l][r][c]=par->Sr_glac*( 1.0 - (glac->w_ice->co[l][r][c]/(rho_i*1.0E-3*glac->Dzl->co[l][r][c])) )*rho_w*1.0E-3*glac->Dzl->co[l][r][c];
						glac->T->co[l][r][c]=IT->Tglac0;
					}
				}

				if(par->output_balancegl>0){
					glac->MELTED->co[r][c]=0.0;
					glac->SUBL->co[r][c]=0.0;
				}
			}
		}
	}

	if(par->recover==1){

		assign_recovered_long(files->co[rni]+1, glac->lnum->co, par, top->Z0, IT->Z);

		for(l=1;l<=par->glaclayer_max;l++){

			assign_recovered(namefile_i_we(files->co[rDzi]+1, l),  glac->Dzl->co[l], par, top->Z0, IT->Z);
			assign_recovered(namefile_i_we(files->co[rwli]+1, l),  glac->w_liq->co[l], par, top->Z0, IT->Z);
			assign_recovered(namefile_i_we(files->co[rwii]+1, l),  glac->w_ice->co[l], par, top->Z0, IT->Z);
			assign_recovered(namefile_i_we(files->co[rTi]+1, l),  glac->T->co[l], par, top->Z0, IT->Z);

		}
	}
}




//*ALTIMETRIC RANKS******************************************************
if(par->ES_num>0){
	top->ES_pixel=new_longvector(par->ES_num);
	initialize_longvector(top->ES_pixel,0);
	top->ES_aspect=new_doublevector(par->ES_num);
	initialize_doublevector(top->ES_aspect,0.0);
	top->ES_slope=new_doublevector(par->ES_num);
	initialize_doublevector(top->ES_slope,0.0);
	top->Zmin=10000.0;
	top->Zmax=-1000.0;
	for(r=1;r<=top->Z0->nrh;r++){
		for(c=1;c<=top->Z0->nch;c++){
			if(top->Z0->co[r][c]!=UV->V->co[2]){
				if(par->glaclayer_max>0){
					glac0=GLACIER0->co[r][c];
				}else{
					glac0=0.0;
				}
				if(glac0>=par->glac_thr){
					if(top->Z0->co[r][c]<top->Zmin) top->Zmin=top->Z0->co[r][c];
					if(top->Z0->co[r][c]>top->Zmax) top->Zmax=top->Z0->co[r][c];
				}
			}
		}
	}

	for(i=1;i<=par->ES_num;i++){
		for(r=1;r<=top->Z0->nrh;r++){
			for(c=1;c<=top->Z0->nch;c++){
				if(top->Z0->co[r][c]!=UV->V->co[2]){
					if(par->glaclayer_max>0){
						glac0=GLACIER0->co[r][c];
					}else{
						glac0=0.0;
					}
					if(glac0>=par->glac_thr){
						if( (top->Z0->co[r][c]>=top->Zmin+(i-1)*(top->Zmax-top->Zmin)/(double)par->ES_num &&
							top->Z0->co[r][c]<top->Zmin+i*(top->Zmax-top->Zmin)/(double)par->ES_num) ||
							(top->Z0->co[r][c]==top->Zmin+i*(top->Zmax-top->Zmin)/(double)par->ES_num &&
							i==par->ES_num) ){
							top->ES_pixel->co[i]+=1;
							top->ES_aspect->co[i]+=top->aspect->co[r][c]*180/Pi;
							top->ES_slope->co[i]+=top->slopes->co[r][c]*180/Pi;
						}
					}
				}
			}
		}
	}
	for(i=1;i<=par->ES_num;i++){
		top->ES_aspect->co[i]/=(double)top->ES_pixel->co[i];
		top->ES_slope->co[i]/=(double)top->ES_pixel->co[i];
	}
}


/****************************************************************************************************/
/*! Filling up of the struct "met" (of the type METEO):                                     */

met->Tgrid=new_doublematrix(top->Z0->nrh,top->Z0->nch);
initialize_doublematrix(met->Tgrid, 0.0);

met->Pgrid=new_doublematrix(top->Z0->nrh,top->Z0->nch);

if(par->micromet1==1){
	met->Vgrid=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	met->Vdir=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	met->RHgrid=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	if(par->output_meteo>0){
		met->Vspdmean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
		met->Vdirmean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
		met->RHmean=new_doublematrix(top->Z0->nrh,top->Z0->nch);
		for(r=1;r<=top->Z0->nrh;r++){
			for(c=1;c<=top->Z0->nch;c++){
				if(top->Z0->co[r][c]==UV->V->co[2]){
					met->Vspdmean->co[r][c]=UV->V->co[2];
					met->Vdirmean->co[r][c]=UV->V->co[2];
					met->RHmean->co[r][c]=UV->V->co[2];
				}else{
					met->Vspdmean->co[r][c]=0.0;
					met->Vdirmean->co[r][c]=0.0;
					met->RHmean->co[r][c]=0.0;
				}
			}
		}
	}
}

//vector used for instanatneous data for each station
met->LT=(float*)malloc(met->st->Z->nh*sizeof(float));
met->Lrh=(float*)malloc(met->st->Z->nh*sizeof(float));
met->Lws=(float*)malloc(met->st->Z->nh*sizeof(float));
met->Lwd=(float*)malloc(met->st->Z->nh*sizeof(float));
met->LP=(float*)malloc(met->st->Z->nh*sizeof(float));

//plot output
if(par->JD_plots->co[1]!=0){
	met->Taplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	for(r=1;r<=top->Z0->nrh;r++){
		for(c=1;c<=top->Z0->nch;c++){
			if(top->Z0->co[r][c]==UV->V->co[2]){
				met->Taplot->co[r][c]=UV->V->co[2];
			}else{
				met->Taplot->co[r][c]=0.0;
			}
		}
	}
	if(par->micromet1==1){
		met->Vspdplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
		met->Vdirplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
		met->RHplot=new_doublematrix(top->Z0->nrh,top->Z0->nch);
		for(r=1;r<=top->Z0->nrh;r++){
			for(c=1;c<=top->Z0->nch;c++){
				if(top->Z0->co[r][c]==UV->V->co[2]){
					met->Vspdplot->co[r][c]=UV->V->co[2];
					met->Vdirplot->co[r][c]=UV->V->co[2];
					met->RHplot->co[r][c]=UV->V->co[2];
				}else{
					met->Vspdplot->co[r][c]=0.0;
					met->Vdirplot->co[r][c]=0.0;
					met->RHplot->co[r][c]=0.0;
				}
			}
		}
	}
}


/****************************************************************************************************/

//LISTON's submodels
if(par->micromet1==1 || par->micromet2==1 || par->micromet3==1 || par->snowtrans==1 || (par->point_sim==1 && par->recover==1)){
	if(par->point_sim==1){
		M=new_doublematrix(IT->Z->nrh,IT->Z->nch);
		initialize_doublematrix(M,IT->Dsnow0);
		initialize_liston(land->ty, IT->LU, IT->Z, par, met->st, M, IT->rhosnow0, liston);
		free_doublematrix(M);
		free_shortmatrix(IT->LU);
		free_doublematrix(IT->Z);
	}else{
		initialize_liston(land->ty, land->use, top->Z0, par, met->st, SNOW0, IT->rhosnow0, liston);
	}
}

f=t_fopen(join_strings(files->co[fHpatch]+1,textfile),"w");
fprintf(f,"DATE,JD,perc.SFA,perc.SCA,H_SCA,H_SFA,MAX_H_SFA,MIN_H_SFA,RSM_H_SFA,TS_MEAN_SFA,TS_MIN_SFA,TS_MAX_SFA,TA_MEAN,Frac(Tsoil>Ta),Hvmean,Hvmin,Hvmax,Hvrsm,Tsvmean\n");
t_fclose(f);

/****************************************************************************************************/

/*Free the struct allocated in this subroutine:*/
free_doublematrix(SNOW0);
if(par->glaclayer_max>0) free_doublematrix(GLACIER0);
if(par->point_sim==0) free_doublematrix(top->Z0dp);

free_doublematrix(IT->land_classes);
free_doublematrix(IT->met);
free(IT->met_col_names);
free(IT);

}
/*--------------------------------------------------------------------------------------------------*/







/****************************************************************************************************/
/* Subroutine to find the the coefficents to diffuse the channel-flows ("fraction_spread" matrix):  */
/****************************************************************************************************/
DOUBLEMATRIX *De_Saint_Venant(DOUBLEVECTOR *s0,double u0,double D,double Dt)
{
 DOUBLEMATRIX *fraction_spread;    /*the returned output matrix*/
 double cumulative_fraction_spread;/*the cumulative sum of the fractions for a virtual distance*/
 double s0max;                     /*the distance of the farthest channel-pixel [m]*/
 long smax;   /*the maximum number of Dx=u0*Dt from the farthest virtual channel-stretch to the outlet*/
 long ch;     /*counter of the channel-pixels*/
 long s;      /*counter of the virtual channel-stretch; s=1 is the channel-stretch next to outlet*/
 FILE *f;

 /* Finding out the distance of the farthest channel-pixel: */
 s0max=0.0;
 for(ch=1;ch<=s0->nh;ch++){
    if (s0max<s0->co[ch]) s0max=s0->co[ch];
 }

 f=fopen(files->co[ferr]+1,"a");

 /* Finding out the distance of the farthest virtual channel-stretch; it has to be enough to allow an
    adeguate diffusion (to do this it is fixed a minimum fraction_spread=0.00005):*/
 smax=(long)(s0max/(u0*Dt))+1;
 do{ smax++; }while(((s0max*Dt)/pow((4.0*Pi*D*pow(smax*Dt,3.0)),0.5)*exp(-u0*pow(s0max-smax*u0*Dt,2.0)
                                                                          /(4.0*D*smax*u0*Dt)))>0.00001);

 fprintf(f,"s0max(farthest channel distance)= %f",s0max);
 fprintf(f,"\nsmax(number channel-pixels)=%ld",smax);

 /* Initialization of fraction_spread matrix:*/
 fraction_spread=new_doublematrix(s0->nh,smax);

 /* Filling up of fraction_spread matrix:*/
 for(ch=1;ch<=fraction_spread->nrh;ch++){
    cumulative_fraction_spread=0.0;
    for(s=1;s<=fraction_spread->nch;s++){
       /*now is used the following formula:
         DV(x0,x)=(x0*V*Dt)/(4*3.14*D*(x/u)^3)^0.5*exp(-(u*(x0-x)^2)/(4*D*x))
         where:
         DV = water volume fraction = fraction_spread
         V  = unitay volume (dirac) = 1
         x0 = distance from outlet of the letting of water into the channel = s0 [m]
         x  = distance from outlet at which DV is calculated = s*u0*Dt [m]
         D  = mean coefficent of dispersion = D [m^2/s]
         u  = mean speed of water in the channel = u0 [m/s] */
		if (s0->co[ch]<=0.0) t_error ("Negative distance of a channel pixel from outlet!");
		fraction_spread->co[ch][s]=(s0->co[ch]*Dt)/pow((4.0*Pi*D*pow(s*Dt,3.0)),0.5)*exp(-u0
                                        *pow(s0->co[ch]-s*u0*Dt,2.0)/(4.0*D*s*u0*Dt));


		if (fraction_spread->co[ch][s]<0.0) fraction_spread->co[ch][s]=0.0;
		if (fraction_spread->co[ch][s]>0.9) fraction_spread->co[ch][s]=0.9;

		cumulative_fraction_spread+=fraction_spread->co[ch][s];
	}

	if(cumulative_fraction_spread==0){
		fprintf(f,"\nWARNING: channel flow description not correct, reduce Dt or u0\n");
		printf("\nWARNING: channel flow description not correct, reduce Dt or u0\n");
	}

	for(s=1;s<=fraction_spread->nch;s++){
		fraction_spread->co[ch][s]*=(1.0/cumulative_fraction_spread);		//correction
	}
 }

 fclose(f);

 return fraction_spread;
}
/*--------------------------------------------------------------------------------------------------*/











/****************************************************************************************************/
/* year_avg_temp: calcola la temprarura media e l'escursione annuale dell'aria                   */
/* Input:	data_meteo: matrice dati meteorolgici                                                   */
/*  		ndays [gg]: intervallo in cui viene mediato il dato di temperatura per  */
/*	    	calcolare l'escursione media annuale                                                    */
/* Output:	T: temperatura media annuale                                                 */
/*		    DT: escursione annuale di temperatura                                       */
/****************************************************************************************************/
void year_avg_temp(DOUBLEVECTOR *Tdata, double ndays, double *T, double *DT, double Dt)

{
long n,i,j,k;
double temp,temp_max,temp_min;

/* calcolo il numero di elementi in ndays */
n=floor(ndays*86400/Dt);

/* calcolo DT */
for(i=n/2+1; i<Tdata->nh-n/2; i++){
	temp=0;
	k=0;
	for(j=i-n/2;j<=i+n/2;j++){
		k+=1;
		temp+=Tdata->co[j];
	}
	temp/=(double)k;
	if(i==n/2+1){
		temp_min=temp;
		temp_max=temp;
	}
	if(temp>temp_max) temp_max=temp;
	if(temp<temp_min) temp_min=temp;
}
*DT=temp_max-temp_min;

/* calcolo T */
*T=0;
for(i=1;i<=Tdata->nh;i++){
	*T+=Tdata->co[i];
}
*T/=(double)Tdata->nh;

}

/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/

long row(double N, long nrows, long i, T_INIT *UV)
{

	long cont=0;

	if(N<UV->U->co[3] || N>UV->U->co[3]+nrows*UV->U->co[1]){
		printf("North coordinate %f in point #%ld out of region",N,i);
		t_error("Fatal error");
	}
	do{
		cont+=1;
	}while(UV->U->co[3]+(nrows-cont)*UV->U->co[1]>N);

	return(cont);
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

long col(double E, long ncols, long i, T_INIT *UV)
{

	long cont=0;

	if(E<UV->U->co[4] || E>UV->U->co[4]+ncols*UV->U->co[2]){
		printf("East coordinate %f in point #%ld out of region",E,i);
		t_error("Fatal error");
	}
	do{
		cont+=1;
	}while(UV->U->co[4]+cont*UV->U->co[2]<E);

	return(cont);
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------


void read_inputmaps(TOPO *top, LAND *land, SOIL *sl, PAR *par){

long r, c, rtot, ctot;
DOUBLEMATRIX *M, *Q, *curv;
SHORTMATRIX *P;
LONGMATRIX *ca;
T_INIT *UV2;
FILE *f;

//reading LAND USE AND TOPOGRAPHY
M=new_doublematrix(1,1);
top->Z0=read_map(0, files->co[fdem]+1, M, UV); //topography
free_doublematrix(M);

if(existing_file(files->co[flu]+1)>0){ //landuse (reading or write default values)
	M=read_map(1, files->co[flu]+1, top->Z0, UV);
	//Check borders
	for(r=1;r<=M->nrh;r++){
		M->co[r][1]=UV->V->co[2];
		M->co[r][M->nch]=UV->V->co[2];
	}
	for(c=1;c<=M->nch;c++){
		M->co[1][c]=UV->V->co[2];
		M->co[M->nrh][c]=UV->V->co[2];
	}
	//Land use is the official mask
	land->use=copyshort_doublematrix(M);
	for(r=1;r<=M->nrh;r++){
		for(c=1;c<=M->nch;c++){
			if(M->co[r][c]==UV->V->co[2]){
				top->Z0->co[r][c]=UV->V->co[2];
			}else{
				if(top->Z0->co[r][c]==UV->V->co[2]){
					printf("ERROR Land use mask include DTM novalue pixels");
					printf("\nr:%ld c:%ld Z:%f landuse:%f\n",r,c,top->Z0->co[r][c],M->co[r][c]);
					M->co[r][c]=UV->V->co[2];
					printf("LANDUSE set at novalue where DTM is not available\n");
					stop_execution();
					//t_error("Land use mask include DTM novalue pixels");
				}
			}
		}
	}
}else{  //writes default value (1)
	//Check DTM border
	for(r=1;r<=top->Z0->nrh;r++){
		top->Z0->co[r][1]=UV->V->co[2];
		top->Z0->co[r][top->Z0->nch]=UV->V->co[2];
	}
	for(c=1;c<=top->Z0->nch;c++){
		top->Z0->co[1][c]=UV->V->co[2];
		top->Z0->co[top->Z0->nrh][c]=UV->V->co[2];
	}
	//Write land->use
	M=copydoublematrix_const(1.0, top->Z0, UV->V->co[2]);
	land->use=copyshort_doublematrix(M);
}
write_map(files->co[fdem]+1, 0, par->format_out, top->Z0, UV);
write_map(files->co[flu]+1, 1, par->format_out, M, UV);
free_doublematrix(M);

/****************************************************************************************************/
//reading SKY VIEW FACTOR
if(existing_file(files->co[fsky]+1)>0){
	top->sky=read_map(2, files->co[fsky]+1, top->Z0, UV);
}else{/*The sky view factor file "top->sky" must be calculated*/
	UV2=(T_INIT *)malloc(sizeof(T_INIT));
	if(!UV2) t_error("UV2 was not allocated");
	UV2->U=new_doublevector(4);
	UV2->V=new_doublevector(2);
	rtot=(long)(top->Z0->nrh/par->nsky);
	ctot=(long)(top->Z0->nch/par->nsky);
	if(rtot*par->nsky<top->Z0->nrh) rtot+=1;
	if(ctot*par->nsky<top->Z0->nch) ctot+=1;
	M=new_doublematrix(rtot,ctot);
	reduce_resolution(par->nsky, top->Z0, M, UV, UV2);
	P=new_shortmatrix(rtot,ctot);
	initialize_shortmatrix(P, (short)UV2->V->co[2]);
	nablaquadro_mask(M, P, UV2->U, UV2->V);
	Q=new_doublematrix(rtot,ctot);
	sky_view_factor(Q, 36, UV2, M, P);
	rtot=top->Z0->nrh-par->nsky*M->nrh;
	ctot=top->Z0->nch-par->nsky*M->nch;
	if(rtot<0) rtot=top->Z0->nrh-par->nsky*(M->nrh-1);
	if(ctot<0) ctot=top->Z0->nch-par->nsky*(M->nch-1);
	top->sky=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	amplify_resolution(par->nsky, rtot, ctot, Q, top->sky, UV2, UV);
	free_doublematrix(M);
	free_shortmatrix(P);
	free_doublematrix(Q);
	free_doublevector(UV2->U);
	free_doublevector(UV2->V);
	free(UV2);
}
write_map(files->co[fsky]+1, 0, par->format_out, top->sky, UV);

/****************************************************************************************************/
//reading DRAINAGE DIRECTIONS
M=read_map(2, files->co[fdd]+1, top->Z0, UV);
top->DD=copyshort_doublematrix(M);
for(r=1;r<=top->Z0->nrh;r++){
	for(c=1;c<=top->Z0->nch;c++){
		if (top->DD->co[r][c]>11) top->DD->co[r][c]=0;
		if (top->Z0->co[r][c]==UV->V->co[2]) top->DD->co[r][c]=9;
	}
}
write_map(files->co[fdd]+1, 1, par->format_out, M, UV);
free_doublematrix(M);

top->Z0dp=depitted(top->DD, top->Z0);

/****************************************************************************************************/
//reading SOIL MAP
if(existing_file(files->co[fsoil]+1)>0){
	M=read_map(2, files->co[fsoil]+1, top->Z0, UV);
	sl->type=copyshort_doublematrix(M);

}else{//default value (99)
	M=copydoublematrix_const(1.0, top->Z0, UV->V->co[2]);
	sl->type=copyshort_doublematrix(M);
}
write_map(files->co[fsoil]+1, 1, par->format_out, M, UV);
free_doublematrix(M);


/****************************************************************************************************/
//reading CURVATURE
if(existing_file(files->co[fcurv]+1)>0){
	M=read_map(2, files->co[fcurv]+1, top->Z0, UV);
	top->curv=copyshort_doublematrix(M);

}else{//calculating
	top->curv=new_shortmatrix(top->Z0->nrh,top->Z0->nch);
	initialize_shortmatrix(top->curv,(short)UV->V->co[2]);
	nablaquadro_mask(top->Z0,top->curv,UV->U,UV->V);
	M=copydouble_shortmatrix(top->curv);
}
write_map(files->co[fcurv]+1, 1, par->format_out, M, UV);
free_doublematrix(M);

/****************************************************************************************************/
//SLOPE
top->slopes=new_doublematrix(top->Z0->nrh,top->Z0->nch);
if(existing_file(files->co[fslp]+1)>0){
	M=read_map(2, files->co[fslp]+1, top->Z0, UV);		//reads in degrees
	fmultiplydoublematrix(top->slopes, M, Pi/180.0, UV->V->co[2]);	//top->slopes in radiants
}else{
	slopes0875(top->Z0, UV->U, UV->V, top->slopes);		//calculates in radiants
	M=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	fmultiplydoublematrix(M, top->slopes, 180.0/Pi, UV->V->co[2]);	//prints in degrees
}
write_map(files->co[fslp]+1, 0, par->format_out, M, UV);
free_doublematrix(M);

/****************************************************************************************************/
//ASPECT
top->aspect=new_doublematrix(top->Z0->nrh,top->Z0->nch);
if(existing_file(files->co[fasp]+1)>0){
	M=read_map(2, files->co[fasp]+1, top->Z0, UV);
	fmultiplydoublematrix(top->aspect, M, Pi/180.0, UV->V->co[2]);
}else{
	aspect0875(top->Z0, UV->U, UV->V, top->aspect);
	M=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	fmultiplydoublematrix(M, top->aspect, 180.0/Pi, UV->V->co[2]);
}
write_map(files->co[fasp]+1, 0, par->format_out, M, UV);
free_doublematrix(M);

/****************************************************************************************************/
//GRADIENTS along drainage directions
if(existing_file(files->co[fgrad]+1)>0){
	top->i_DD=read_map(2, files->co[fgrad]+1, top->Z0, UV);
}else{/*The matrix the slope along the Drainage Direction "top->i_DD" must be calculated*/
	top->i_DD=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	initialize_doublematrix(top->i_DD,UV->V->co[2]);
	gradients(top->Z0dp,top->DD,top->i_DD,UV);
}
write_map(files->co[fgrad]+1, 0, par->format_out, top->i_DD, UV);

/****************************************************************************************************/
//Effective area of each pixel
if(existing_file(files->co[farea]+1)>0){
	top->area=read_map(2, files->co[farea]+1, top->Z0, UV);
}else{
	top->area=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	area0875(top->Z0,UV->U,UV->V,top->area);
}
write_map(files->co[farea]+1, 0, par->format_out, top->area, UV);

/****************************************************************************************************/
//Channel network (in top->pixel_type)
if(existing_file(files->co[fnet]+1)>0){
	M=read_map(2, files->co[fnet]+1, top->Z0, UV);
	top->pixel_type=copyshort_doublematrix(M);
	free_doublematrix(M);
}else{
	//reading or calculating TCA
	if(existing_file(files->co[ftca]+1)>0){
		M=read_map(2, files->co[ftca]+1, top->Z0, UV);
		ca=copylong_doublematrix(M);
	}else{
		ca=new_longmatrix(top->Z0->nrh,top->Z0->nch);
		initialize_longmatrix(ca,0);
		for(r=1;r<=top->Z0->nrh;r++){
			for(c=1;c<=top->Z0->nch;c++){
				if(top->Z0->co[r][c]==UV->V->co[2]) ca->co[r][c]=(long)UV->V->co[2];
			}
		}
		tca(top->DD,ca);
		M=copydouble_longmatrix(ca);
	}
	write_map(files->co[ftca]+1, 1, par->format_out, M, UV);
	free_doublematrix(M);
	//calculate laplacian
	curv=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	nablaquadro(top->Z0,curv,UV->U,UV->V);
	top->pixel_type=new_shortmatrix(top->Z0->nrh,top->Z0->nch);
	//copy drainage directions in top->pixel_type
	copy_shortmatrix(top->DD,top->pixel_type);
	//see geomorphologic library (overwrites pixel_type=10 for channels)
	select_hillslopes_mod(ca,top->i_DD,curv,top->pixel_type,par->channel_thres,UV->U);
	free_longmatrix(ca);
	free_doublematrix(curv);
}

/*! Creation of pixel-type matrix "top->pixel_type" (0=land,9=novalue,10=channel,11=lake,12=sea) on the
   basis of land use map (land->use) and channel network (already in top->pixel_type matrix): */
M=new_doublematrix(top->Z0->nrh,top->Z0->nch);
for(r=1;r<=top->Z0->nrh;r++){
	for(c=1;c<=top->Z0->nch;c++){
		if(top->Z0->co[r][c]!=UV->V->co[2]){
			if (top->pixel_type->co[r][c]!=10) top->pixel_type->co[r][c]=0;
			//if (land->use->co[r][c]==11/*lake*/ || land->use->co[r][c]==12/*sea*/) top->pixel_type->co[r][c]=land->use->co[r][c];
			M->co[r][c]=(double)top->pixel_type->co[r][c];
		}else{
			M->co[r][c]=UV->V->co[2];
			top->pixel_type->co[r][c]=9;
		}
	}
}
write_map(files->co[fnet]+1, 1, par->format_out, M, UV);
free_doublematrix(M);


/****************************************************************************************************/
//Outlet distances
if(existing_file(files->co[fdist]+1)>0){
	top->pixel_distance=read_map(1, files->co[fdist]+1, top->Z0, UV);
	//Check values
	for(r=1;r<=top->pixel_distance->nrh;r++){
		for(c=1;c<=top->pixel_distance->nch;c++){
			if(top->pixel_distance->co[r][c]==UV->V->co[2]){
				if(top->Z0->co[r][c]!=UV->V->co[2]){
					f=fopen(files->co[ferr]+1,"a");
					fprintf(f,"\nPixeldistance has NOVALUE where DTM does not in pixel r=%4ld c=%4ld, corrected, but inconsistently\n",r,c);
					fclose(f);
					top->pixel_distance->co[r][c]=0.0;
				}
			}
		}
	}
	//A null top->pixel_distance is changed in half pixel size
	for(r=1;r<=land->use->nrh;r++){
		for(c=1;c<=land->use->nch;c++){
			if(top->Z0->co[r][c]!=UV->V->co[2]){
				if (top->pixel_distance->co[r][c]<=0.0) top->pixel_distance->co[r][c]=UV->U->co[2]/2.0;
			}
		}
	}
}else{
	top->pixel_distance=new_doublematrix(top->Z0->nrh,top->Z0->nch);
	initialize_doublematrix(top->pixel_distance,UV->V->co[2]);
	//se geomorphologic libraries
	outletdistance(top->DD,top->pixel_distance,UV->U);
	for(r=1;r<=land->use->nrh;r++){
		for(c=1;c<=land->use->nch;c++){
			if(top->pixel_distance->co[r][c]==UV->V->co[2]){
				if(top->Z0->co[r][c]!=UV->V->co[2]){
					f=fopen(files->co[ferr]+1,"a");
					fprintf(f,"\nPixeldistance has NOVALUE where DTM does not in pixel r=%4ld c=%4ld, corrected, but inconsistently\n",r,c);
					fclose(f);
					top->pixel_distance->co[r][c]=0.0;
				}
			}
			if(top->Z0->co[r][c]!=UV->V->co[2]){
				if (top->pixel_distance->co[r][c]<=0.0) top->pixel_distance->co[r][c]=UV->U->co[2]/2.0;
			}
		}
	}
}
write_map(files->co[fdist]+1, 0, par->format_out, top->pixel_distance, UV);


}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

void read_parameterfile(char *name, PAR *par, LISTON *liston, INIT_TOOLS *itools){

	FILE *f;
	long i,index;
	DOUBLEVECTOR *v;

	f=t_fopen(join_strings(name,textfile),"r");
	index=read_index(f,PRINT);
	//1st block
	itools->land_classes=read_doublematrix(f,"a",PRINT);
	//2nd block
	v=read_doublearray(f,PRINT);
	if(v->co[1]<=0) t_error("ERROR: 0 is not admitted for nDtwater");
	par->nDt_water=(long)v->co[1];
	par->f_bound_Richards=v->co[2];	/*Parameter for the bottom boundary condition for the Richards' equation: =0 no flux, =1 free drainage*/
	par->imp=v->co[3];	/*Impedence factor for (partially) frozen sl*/
	par->psimin=v->co[4];
	par->Esoil=v->co[5];
	par->stmin=v->co[6];
	par->MaxiterVWB=(long)v->co[7];
	par->TolVWb=v->co[8];
	par->PsiInf=v->co[9];
	par->TolPsiInf=v->co[10];
	par->MaxerrVWB=v->co[11];
	par->dtminVWB=v->co[12];
	par->Dpsi=v->co[13];
	par->dtmin=v->co[14];
	itools->u0=v->co[15]; /*THE MEAN VELOCITY IN CHANNELS*/
	itools->D=v->co[16];  /*THE HYDRODYNAMICAL DISPERSION IN CHANNELS*/
	par->gamma_m=v->co[17]; /*Exponent of the law of uniform motion on the surface*/
	par->q1=v->co[18];	//q1 and q2 are calibration parameters for subsurface flow
	par->q2=v->co[19];
	free_doublevector(v);
	//3rd block
	v=read_doublearray(f,PRINT);
	par->latitude=v->co[1]*Pi/180.0;
	par->longitude=v->co[2]*Pi/180.0;
	par->Vis=v->co[3];
	par->Lozone=v->co[4];
	par->Vmin=v->co[5];		/*MINIMUM WIND VELOCITY VALUE (m/s)*/
	par->RHmin=v->co[6];		/*MINIMUM RELATIVE HUMIDITY VALUE (%)*/
	par->integr_scale_rain=v->co[7];
	par->variance_rain=v->co[8];
	liston->ifill=(int)v->co[9];
	liston->iobsint=(int)v->co[10];
	liston->dn=(float)v->co[11];
	liston->curve_len_scale=(float)v->co[12];
	liston->slopewt=(float)v->co[13];
	liston->curvewt=(float)v->co[14];
	liston->calc_subcanopy_met=(float)v->co[15];
	liston->cloud_frac_factor=(float)v->co[16];
	liston->topoflag=(float)v->co[17];
	free_doublevector(v);
	/* 4th block Snow Parameters*/
	v=read_doublearray(f,PRINT);
	itools->Dsnow0=v->co[1];/* INITIAL SNOW DEPTH (initial snow depth) [mm] valid only if state_snow==0 */
	itools->rhosnow0=v->co[2]; /*SNOW INITIAL DENSITY [kg/mc]*/
	itools->agesnow0=v->co[3];/* INITIAL SNOW AGE (in days), valid only if state_snow_age==0 */
	par->T_rain=v->co[4];   /*TEMPERATURE ABOVE WICH ALL PRECIPITAION IS RAIN [C]*/
	par->T_snow=v->co[5];   /*TEMPERATURE BELOW WICH ALL PRECIPITAION IS SNOW [C]*/
	par->aep=v->co[6];      /*AßLBEDO EXTINCTION PARAMETER [m]*/
	par->avo=v->co[7];      /*NEW SNOW VISIBLE BAND REFLECTANCE*/
	par->airo=v->co[8];     /*NEW NEAR INFRARED BAND REFLECTANCE*/
	par->Sr=v->co[9];       /*IRREDUCIBLE WATER SATURATION [-]*/
	par->epsilon_snow=v->co[10];/* SNOW LONGWAVE EMISSIVITY [-]  */
	par->z0_snow=v->co[11]*0.001;
	par->snowcorrfact=v->co[12];/* INCREASING FACTOR WHEN THE RAIN GAUGE IS SUPPOSED TO RECORD SNOW PRECIPITATION */
	par->raincorrfact=v->co[13];/* INCREASING FACTOR WHEN THE RAIN GAUGE IS SUPPOSED TO RECORD RAIN PRECIPITATION */
	par->snowlayer_max=(long)v->co[14]; /* MAXIMUM NUMBER OF SNOW LAYERS */
	par->snow_maxpor=v->co[15];
	par->drysnowdef_rate=v->co[16];
	par->snow_density_cutoff=v->co[17];
	par->wetsnowdef_rate=v->co[18];
	par->snow_viscosity=v->co[19];
	if(par->snowlayer_max<3) t_error("Maximum number of snow layers too small, it must be not smaller than 3");
	if( par->snowlayer_max!=1+2*floor(0.5*(par->snowlayer_max-1)) ) t_error("Maximum number of snow layers must be odd");
	free_doublevector(v);
	/* 5th-6th block MINIMUM and MAXIMUM SNOW LAYER THICKNESS*/
	par->Dmin=read_doublearray(f,PRINT);
	if(par->Dmin->nh<1+floor(0.5*par->snowlayer_max)) t_error("Error in assigning max and min thickness to the snow layers");
	par->Dmax=read_doublearray(f,PRINT);
	if(par->Dmax->nh<1+floor(0.5*par->snowlayer_max)) t_error("Error in assigning max and min thickness to the snow layers");
	/*if(par->Dmax->co[(long)(1+floor(0.5*par->snowlayer_max))]<1.0E10){
		printf("\nWARNING: Snow layer %ld must have infinite thickness!!! Assigned %f mm instead of %f mm\n",(long)(1+floor(0.5*par->snowlayer_max)),1.0E10,par->Dmax->co[(long)(1+floor(0.5*par->snowlayer_max))]);
		stop_execution();
		par->Dmax->co[(long)(1+floor(0.5*par->snowlayer_max))]=1.0E10;
	}*/
	//7th block
	v=read_doublearray(f,PRINT);
	liston->bc_flag=(float)v->co[1];
	liston->erosion_dist=(float)v->co[2];
	liston->tabler_dir=(float)v->co[3];
	liston->slope_adjust=(float)v->co[4];
	liston->Utau_t_const=(float)v->co[5];
	liston->Utau_t_flag=(float)v->co[6];
	liston->subgrid_flag=(float)v->co[7];
	liston->tp_scale=(float)v->co[8];
	liston->fetch=(float)v->co[9];
	liston->xmu=(float)v->co[10];
	liston->C_z=(float)v->co[11];
	liston->h_const=(float)v->co[12];
	liston->wind_min=(float)v->co[13];
	liston->Up_const=(float)v->co[14];
	liston->dz_susp=(float)v->co[15];
	liston->ztop_susp=(float)v->co[16];
	liston->fall_vel=(float)v->co[17];
	liston->Ur_const=(float)v->co[18];
	free_doublevector(v);
	//8th block
	v=read_doublearray(f,PRINT);
	itools->Dglac0=v->co[1];
	itools->rhoglac0=v->co[2];
	itools->Tglac0=v->co[3];
	par->Sr_glac=v->co[4];
	par->glaclayer_max=(long)v->co[5];
	if(par->glaclayer_max<0) t_error("Maximum number of glacier cannot be negative");
	if(par->glaclayer_max==0) printf("\nNo glacier is considered\n");
	free_doublevector(v);
	//9th block
	v=read_doublearray(f,PRINT);
	if(par->glaclayer_max>0){
		par->Dmin_glac=new_doublevector(par->glaclayer_max);
		par->Dmax_glac=new_doublevector(par->glaclayer_max);
		if(v->nh!=2*par->glaclayer_max-1) t_error("Error in assigning max and min thickness to the glacier layers");
		for(i=1;i<=par->glaclayer_max;i++){
			par->Dmin_glac->co[i]=v->co[i];
		}
		for(i=1;i<=par->glaclayer_max-1;i++){
			par->Dmax_glac->co[i]=v->co[i+par->glaclayer_max];
		}
		par->Dmax_glac->co[par->glaclayer_max]=1.0E10;
	}
	free_doublevector(v);
	//10th block
	v=read_doublearray(f,PRINT);
	par->state_turb=(short)v->co[1];
	par->state_lwrad=(short)v->co[2];
	par->monin_obukhov=(short)v->co[3];
	par->micromet1=(short)v->co[4];
	par->micromet2=(short)v->co[5];
	par->micromet3=(short)v->co[6];
	par->snowtrans=(short)v->co[7];
	if(par->snowtrans==1 && par->micromet1==0 && par->micromet2==0 && par->micromet3==0){
		par->snowtrans=0;
		printf("\nWarning: if you do not run Micromet, you can't run SnowTrans3D\n");
	}
	free_doublevector(v);
	//other par
	par->print=0;
	//11th block
	itools->met=read_doublematrix(f,"a",PRINT);
	if(itools->met->nch!=13) t_error("Error in block 22 file par");
	//12th block
	itools->met_col_names=read_stringarray(f,PRINT);

	t_fclose(f);

	/*Parameters derived, already calculated or that have to be calcultated because useful in the next subroutine:*/
	par->n_error=0;        /*Current number of error of the simulation*/
	par->max_error=10000; /*Maximum number of error for the simulation*/
	par->state_pixel=1;
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void read_optionsfile_distr(char *name, PAR *par, TIMES *times, DOUBLEMATRIX *Z0){

	FILE *f;
	long index, i;
	DOUBLEVECTOR *v;

	f=t_fopen(join_strings(name,textfile),"r");
	index=read_index(f,PRINT);

	//1. base par
	v=read_doublearray(f,PRINT);
	par->wat_balance=(short)v->co[1];
	par->en_balance=(short)v->co[2];
	par->state_snow=(short)v->co[3];
	par->state_snow_age=(short)v->co[4];
	par->state_glac=(short)v->co[5];
	par->state_px_coord=(short)v->co[6];
	par->distr_stat=(short)v->co[7];
	free_doublevector(v);

	//2. chkpt
	par->chkpt=read_doublematrix(f,"a",PRINT);

	//3. saving points
	par->saving_points=read_longarray(f,PRINT);

	//4. output par
	v=read_doublearray(f,PRINT);
	par->output_Txy=v->co[1];
	par->output_TETAxy=v->co[2];
	par->output_TETAICExy=v->co[3];
	par->output_PSIxy=v->co[4];
	par->output_snow=v->co[5];
	par->output_glac=v->co[6];
	par->output_h_sup=v->co[7];
	par->output_albedo=v->co[8];
	par->output_Rn=v->co[9];
	par->output_G=v->co[10];
	par->output_H=v->co[11];
	par->output_ET=v->co[12];
	par->output_Ts=v->co[13];
	par->output_P=v->co[14];
	par->output_Wr=v->co[15];
	par->output_balancesn=v->co[16];
	par->output_balancegl=v->co[17];
	par->output_Rswdown=v->co[18];
	par->output_meteo=v->co[19];
	free_doublevector(v);

	//5. special output
	v=read_doublearray(f,PRINT);
	if(v->co[1]!=0.0 && v->nh>1){
		times->n_plot=floor(v->co[1]*3600/par->Dt);
		par->JD_plots=new_longvector(v->nh-1);
		for(i=1;i<=v->nh-1;i++){
			par->JD_plots->co[i]=(long)v->co[i+1];
		}
	}else{
		par->JD_plots=new_longvector(1);
		par->JD_plots->co[1]=0;
	}
	free_doublevector(v);

	t_fclose(f);

	if(par->chkpt->nch!=3) t_error("Error in block 16 in parameter files: only 2 coordinates and layer number admitted");
	if(par->chkpt->nrh>9999) t_error("Error in block 16 in parameter files: no more than 9999 points allowed");

	if(par->state_pixel==1){
		par->rc=new_longmatrix(par->chkpt->nrh,2);
		for(i=1;i<=par->chkpt->nrh;i++){
			if(par->state_px_coord==1){
				par->rc->co[i][1]=row(par->chkpt->co[i][2], Z0->nrh, i, UV);
				par->rc->co[i][2]=col(par->chkpt->co[i][1], Z0->nch, i, UV);
			}else{
				par->rc->co[i][1]=(long)par->chkpt->co[i][1];
				par->rc->co[i][2]=(long)par->chkpt->co[i][2];
			}
			if(Z0->co[par->rc->co[i][1]][par->rc->co[i][2]]==UV->V->co[2]){
				printf("\nWARNING: Point #%4ld corresponds to NOVALUE pixel",i);
				stop_execution();
			}
			if((long)par->chkpt->co[i][3]<1 || (long)par->chkpt->co[i][3]>Nl){
				printf("ERROR: Check point #%4ld has invalid layer coordinate",i);
				t_error("Fatal error");
			}
		}
	}
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
void read_optionsfile_point(char *name, PAR *par, TOPO *top, LAND *land, SOIL *sl, INIT_TOOLS *IT){

	FILE *f;
	long index, i, ly, r, c;
	DOUBLEVECTOR *v;
	STRINGBIN *s;
	DOUBLEMATRIX *M, *Q, *P;
	SHORTMATRIX *curv;
	short read_dem, read_lu, read_soil, read_sl, read_as, read_sk, read_dx, read_dy;

	f=t_fopen(join_strings(name,textfile),"r");
	index=read_index(f,PRINT);

	//1. base par
	v=read_doublearray(f,PRINT);
	par->wat_balance=(short)v->co[1];
	par->en_balance=(short)v->co[2];
	par->state_px_coord=(short)v->co[3];
	ly=(short)v->co[4];
	free_doublevector(v);

	//2. chkpt
	M=read_doublematrix(f,"a",PRINT);
	if(M->nch!=10) t_error("Error in block 2 option file");

	//3. horizon name
	s=read_stringarray(f,PRINT);

	t_fclose(f);


	//4. CALCULATE TOPOGRAPHIC PROPERTIES
	//a. read dem
	read_dem=0;
	for(i=1;i<=M->nrh;i++){
		if(M->co[i][3]==-99.0 || M->co[i][6]==-99.0 || M->co[i][7]==-99.0 || M->co[i][8]==-99.0 || M->co[i][9]==-99.0 || M->co[i][10]==-99.0) read_dem=1;
	}
	if(par->micromet1==1 || par->micromet2==1 || par->micromet3==1 || par->recover==1) read_dem=1;
	if(read_dem==1){
		if(existing_file(files->co[fdem]+1)>0){
			Q=new_doublematrix(1,1);
			IT->Z=read_map(0, files->co[fdem]+1, Q, UV); //topography
			free_doublematrix(Q);
			for(r=1;r<=IT->Z->nrh;r++){
				IT->Z->co[r][1]=UV->V->co[2];
				IT->Z->co[r][IT->Z->nch]=UV->V->co[2];
			}
			for(c=1;c<=IT->Z->nch;c++){
				IT->Z->co[1][c]=UV->V->co[2];
				IT->Z->co[IT->Z->nrh][c]=UV->V->co[2];
			}
		}else{
			printf("Warning: Dem file not present\n");
			read_dem=0;
		}
	}
	if(read_dem==0 && par->state_px_coord==1) t_error("if a dem file is not used or not present, you can't set state_px_coord at 1");
	if(read_dem==0){
		par->micromet1=0;
		par->micromet2=0;
		par->micromet3=0;
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][3]==-99) M->co[i][3]=0.0;
		}
	}else{
		par->r_points=new_longvector(M->nrh);
		par->c_points=new_longvector(M->nrh);
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][3]==-99){
				if(par->state_px_coord==0){
					par->r_points->co[i]=(long)M->co[i][1];
					par->c_points->co[i]=(long)M->co[i][2];
				}else{
					par->r_points->co[i]=row(M->co[i][2], IT->Z->nrh, i, UV);
					par->c_points->co[i]=col(M->co[i][1], IT->Z->nch, i, UV);
				}
				M->co[i][3]=IT->Z->co[par->r_points->co[i]][par->c_points->co[i]];
			}
		}
	}


	//b. read land use
	read_lu=0;
	for(i=1;i<=M->nrh;i++){ if(M->co[i][4]==-99) read_lu=1; }
	if(par->micromet1==1 || par->micromet2==1 || par->micromet3==1 || par->recover==1) read_lu=1;
	if(read_lu==1){
		if(existing_file(files->co[flu]+1)>0){
			if(read_dem==0){
				Q=new_doublematrix(1,1);
				P=read_map(0, files->co[flu]+1, Q, UV);
				free_doublematrix(Q);
			}else{
				P=read_map(2, files->co[flu]+1, IT->Z, UV);
			}
		}else{
			printf("Warning: Landuse file not present,  uniforme landuse considered\n");
			P=copydoublematrix_const(1.0, IT->Z, UV->V->co[2]);
		}
	}
	if(read_lu==0){
		par->micromet1=0;
		par->micromet2=0;
		par->micromet3=0;
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][4]==-99) M->co[i][4]=1.0;
		}
	}else{
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][4]==-99){
				if(par->state_px_coord==0){
					r=(long)M->co[i][1];
					c=(long)M->co[i][2];
				}else{
					r=row(M->co[i][2], P->nrh, i, UV);
					c=col(M->co[i][1], P->nch, i, UV);
				}
				M->co[i][4]=P->co[r][c];
			}
		}
		if(par->micromet1==1 || par->micromet2==1 || par->micromet3==1 || par->recover==1) IT->LU=copyshort_doublematrix(P);
		free_doublematrix(P);
	}

	//c. read sl type
	read_soil=0;
	for(i=1;i<=M->nrh;i++){ if(M->co[i][5]==-99) read_soil=1; }
	if(read_soil==1){
		if(existing_file(files->co[fsoil]+1)>0){
			if(read_dem==0){
				Q=new_doublematrix(1,1);
				P=read_map(0, files->co[fsoil]+1, Q, UV);
				free_doublematrix(Q);
			}else{
				P=read_map(2, files->co[fsoil]+1, IT->Z, UV);
			}
		}else{
			printf("Warning: Soiltype file not present\n");
			read_soil=0;
		}
	}
	if(read_soil==0){
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][5]==-99) M->co[i][5]=1.0;
		}
	}else{
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][5]==-99){
				if(par->state_px_coord==0){
					r=(long)M->co[i][1];
					c=(long)M->co[i][2];
				}else{
					r=row(M->co[i][2], P->nrh, i, UV);
					c=col(M->co[i][1], P->nch, i, UV);
				}
				M->co[i][5]=P->co[r][c];
			}
		}
		free_doublematrix(P);
	}

	//d. read slopes
	read_sl=0;
	for(i=1;i<=M->nrh;i++){ if(M->co[i][6]==-99) read_sl=1; }
	if(read_sl==1){
		if(existing_file(files->co[fslp]+1)>0){
			if(read_dem==0){
				Q=new_doublematrix(1,1);
				P=read_map(0, files->co[fslp]+1, Q, UV);
				free_doublematrix(Q);
			}else{
				P=read_map(2, files->co[fslp]+1, IT->Z, UV);
			}
		}else{
			if(read_dem==0){
				printf("Warning: Slopes file not present\n");
				read_sl=0;
			}else{
				P=new_doublematrix(IT->Z->nrh,IT->Z->nch);
				slopes0875(IT->Z,UV->U,UV->V,P);
				for(r=1;r<=IT->Z->nrh;r++){
					for(c=1;c<=IT->Z->nch;c++){
						if(IT->Z->co[r][c]!=UV->V->co[2]){
							P->co[r][c]*=180.0/Pi;
						}
					}
				}
				write_map(files->co[fslp]+1, 0, par->format_out, P, UV);
			}
		}
	}
	if(read_sl==0){
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][6]==-99) M->co[i][6]=0.0;
		}
	}else{
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][6]==-99){
				if(par->state_px_coord==0){
					r=(long)M->co[i][1];
					c=(long)M->co[i][2];
				}else{
					r=row(M->co[i][2], P->nrh, i, UV);
					c=col(M->co[i][1], P->nch, i, UV);
				}
				M->co[i][6]=P->co[r][c];
			}
		}
		free_doublematrix(P);
	}

	//e. read aspect
	read_as=0;
	for(i=1;i<=M->nrh;i++){ if(M->co[i][7]==-99) read_as=1; }
	if(read_as==1){
		if(existing_file(files->co[fasp]+1)>0){
			if(read_dem==0){
				Q=new_doublematrix(1,1);
				P=read_map(0, files->co[fasp]+1, Q, UV);
				free_doublematrix(Q);
			}else{
				P=read_map(2, files->co[fasp]+1, IT->Z, UV);
			}
		}else{
			if(read_dem==0){
				printf("Warning: Aspect file not present\n");
				read_as=0;
			}else{
				P=new_doublematrix(IT->Z->nrh,IT->Z->nch);
				aspect0875(IT->Z,UV->U,UV->V,P);
				for(r=1;r<=IT->Z->nrh;r++){
					for(c=1;c<=IT->Z->nch;c++){
						if(IT->Z->co[r][c]!=UV->V->co[2]){
							P->co[r][c]*=180.0/Pi;
						}
					}
				}
				write_map(files->co[fasp]+1, 0, par->format_out, P, UV);
			}
		}
	}
	if(read_as==0){
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][7]==-99) M->co[i][7]=0.0;
		}
	}else{
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][7]==-99){
				if(par->state_px_coord==0){
					r=(long)M->co[i][1];
					c=(long)M->co[i][2];
				}else{
					r=row(M->co[i][2], P->nrh, i, UV);
					c=col(M->co[i][1], P->nch, i, UV);
				}
				M->co[i][7]=P->co[r][c];
			}
		}
		free_doublematrix(P);
	}

	//f. sky view factor file
	read_sk=0;
	for(i=1;i<=M->nrh;i++){ if(M->co[i][8]==-99) read_sk=1; }
	if(read_sk==1){
		if(existing_file(files->co[fsky]+1)>0){
			if(read_dem==0){
				Q=new_doublematrix(1,1);
				P=read_map(0, files->co[fsky]+1, Q, UV);
				free_doublematrix(Q);
			}else{
				P=read_map(2, files->co[fsky]+1, IT->Z, UV);
			}
		}else{
			if(read_dem==0){
				printf("Warning: Sky view factor file not present\n");
				read_sk=0;
			}else{
				P=new_doublematrix(IT->Z->nrh,IT->Z->nch);
				curv=new_shortmatrix(IT->Z->nrh,IT->Z->nch);
				nablaquadro_mask(IT->Z, curv, UV->U, UV->V);
				sky_view_factor(P, 36, UV, IT->Z, curv);
				free_shortmatrix(curv);
				write_map(files->co[fsky]+1, 0, par->format_out, P, UV);
			}
		}
	}
	if(read_sk==0){
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][8]==-99) M->co[i][8]=1.0;
		}
	}else{
		for(i=1;i<=M->nrh;i++){
			if(M->co[i][8]==-99){
				if(par->state_px_coord==0){
					r=(long)M->co[i][1];
					c=(long)M->co[i][2];
				}else{
					r=row(M->co[i][2], P->nrh, i, UV);
					c=col(M->co[i][1], P->nch, i, UV);
				}
				M->co[i][8]=P->co[r][c];
			}
		}
		free_doublematrix(P);
	}

	//g. dz/dx(east)
	read_dx=0;
	for(i=1;i<=M->nrh;i++){ if(M->co[i][9]==-99) read_dx=1; }
	if(read_dx==1){
		if(read_dem==0){
			printf("Impossible to calculate net E slope: set at 0");
			read_dx=0;
		}
	}
	for(i=1;i<=M->nrh;i++){
		if(par->state_px_coord==0){
			r=(long)M->co[i][1];
			c=(long)M->co[i][2];
		}else{
			r=row(M->co[i][2], IT->Z->nrh, i, UV);
			c=col(M->co[i][1], IT->Z->nch, i, UV);
		}
		if(read_dx==1){
			if(c>1 && c<IT->Z->nch){
				if(IT->Z->co[r][c+1]!=UV->V->co[2] && IT->Z->co[r][c-1]!=UV->V->co[2]){
					M->co[i][9]=( (IT->Z->co[r][c]-IT->Z->co[r][c+1]) - (IT->Z->co[r][c-1]-IT->Z->co[r][c])  )/UV->U->co[1];
				}
			}else{
				printf("Impossible to calculate net E slope for point %ld\n: set at 0",i);
				M->co[i][9]=0.0;
			}
		}else{
			if(M->co[i][9]==-99) M->co[i][9]=0.0;
		}
	}


	//h. dz/dy(north)
	read_dy=0;
	for(i=1;i<=M->nrh;i++){ if(M->co[i][10]==-99) read_dy=1; }
	if(read_dy==1){
		if(read_dem==0){
			printf("Impossible to calculate net S slope: set at 0");
			read_dy=0;
		}
	}
	for(i=1;i<=M->nrh;i++){
		if(par->state_px_coord==0){
			r=(long)M->co[i][1];
			c=(long)M->co[i][2];
		}else{
			r=row(M->co[i][2], IT->Z->nrh, i, UV);
			c=col(M->co[i][1], IT->Z->nch, i, UV);
		}
		if(read_dy==1){
			if(r>1 && r<IT->Z->nrh){
				if(IT->Z->co[r+1][c]!=UV->V->co[2] && IT->Z->co[r-1][c]!=UV->V->co[2]){
					M->co[i][10]=( (IT->Z->co[r][c]-IT->Z->co[r+1][c]) - (IT->Z->co[r-1][c]-IT->Z->co[r][c])  )/UV->U->co[2];
				}
			}else{
				printf("Impossible to calculate net S slope for point %ld\n: set at 0",i);
				M->co[i][10]=0.0;
			}
		}else{
			if(M->co[i][10]==-99) M->co[i][10]=0.0;
		}
	}


	//i.show results
	printf("\nPOINTS:\n");
	fprintf(f,"\nPOINTS:\n");
	f=fopen(files->co[ferr]+1,"a");
	for(r=1;r<=M->nrh;r++){
		for(c=1;c<=10;c++){
			printf("%f  ",M->co[r][c]);
			fprintf(f,"%f  ",M->co[r][c]);
		}
		printf("\n");
		fprintf(f,"\n");
	}
	fclose(f);

	//l. set UV
	if(read_dem==0 && read_lu==0 && read_soil==0 && read_sl==0 && read_as==0 && read_sk==0){
		UV=(T_INIT *)malloc(sizeof(T_INIT));
		if(!UV) t_error("UV was not allocated in input");
		UV->U=new_doublevector(4);
		UV->V=new_doublevector(2);
		UV->U->co[2]=1.0;
		UV->U->co[1]=1.0;
		UV->U->co[4]=0.0;
		UV->U->co[3]=0.0;
		UV->V->co[1]=-1;
		UV->V->co[2]=-9999.0;
	}

	//m. deallocation
	if(read_dem==1 && (par->micromet1!=1 && par->micromet2!=1 && par->micromet3!=1 && par->recover!=1)){
		free_doublematrix(IT->Z);
		free_longvector(par->r_points);
		free_longvector(par->c_points);
	}

	//5. SET CHECKPOINT
	par->chkpt=new_doublematrix(M->nrh,3);
	par->rc=new_longmatrix(M->nrh,2);
	for(i=1;i<=M->nrh;i++){
		par->chkpt->co[i][1]=M->co[i][1];
		par->chkpt->co[i][2]=M->co[i][2];
		par->chkpt->co[i][3]=(double)ly;
		par->rc->co[i][1]=1;
		par->rc->co[i][2]=i;
	}

	//6. SET PROPERTIES
	top->Z0=new_doublematrix(1,M->nrh);
	top->Z0dp=new_doublematrix(1,M->nrh);
	land->use=new_shortmatrix(1,M->nrh);
	sl->type=new_shortmatrix(1,M->nrh);
	top->slopes=new_doublematrix(1,M->nrh);
	top->aspect=new_doublematrix(1,M->nrh);
	top->sky=new_doublematrix(1,M->nrh);
	top->dz_dx=new_doublematrix(1,M->nrh);
	top->dz_dy=new_doublematrix(1,M->nrh);
	top->DD=new_shortmatrix(1,M->nrh);
	top->curv=new_shortmatrix(1,M->nrh);
	top->i_DD=new_doublematrix(1,M->nrh);
	top->area=new_doublematrix(1,M->nrh);
	top->pixel_type=new_shortmatrix(1,M->nrh);
	top->pixel_distance=new_doublematrix(1,M->nrh);
	for(i=1;i<=M->nrh;i++){
		top->Z0->co[1][i]=M->co[i][3];
		top->Z0dp->co[1][i]=0.0;
		land->use->co[1][i]=(short)M->co[i][4];
		sl->type->co[1][i]=(short)M->co[i][5];
		top->slopes->co[1][i]=M->co[i][6]*Pi/180.0;
		top->aspect->co[1][i]=M->co[i][7]*Pi/180.0;
		top->sky->co[1][i]=M->co[i][8];
		top->dz_dx->co[1][i]=M->co[i][9];
		top->dz_dy->co[1][i]=M->co[i][10];
		//these values are not used in 1d simulations
		top->DD->co[1][i]=10;	//outlet
		top->curv->co[1][i]=0;
		top->i_DD->co[1][i]=0.0;
		top->area->co[1][i]=UV->U->co[1]*UV->U->co[2];
		top->pixel_type->co[1][i]=0;
		top->pixel_distance->co[1][i]=1.0;
	}

	//7. SET PAR
	par->state_snow=0;
	par->state_snow_age=0;
	par->state_glac=0;
	par->output_Txy=0;
	par->output_TETAxy=0;
	par->output_TETAICExy=0;
	par->output_PSIxy=0;
	par->output_snow=0;
	par->output_glac=0;
	par->output_h_sup=0;
	par->output_albedo=0;
	par->output_Rn=0;
	par->output_G=0;
	par->output_H=0;
	par->output_ET=0;
	par->output_Ts=0;
	par->output_P=0;
	par->output_Wr=0;
	par->output_balancesn=0;
	par->output_balancegl=0;
	par->output_Rswdown=0;
	par->output_meteo=0;
	par->saving_points=new_longvector(1);
	initialize_longvector(par->saving_points,9999999);
	par->JD_plots=new_longvector(1);
	par->JD_plots->co[1]=0;
	par->snowtrans=0;


	//8. READ HORIZONS
	top->horizon_height=(double ****)malloc(top->Z0->nrh*sizeof(double***));
	for(r=1;r<=top->Z0->nrh;r++){
		top->horizon_height[r-1]=(double ***)malloc(top->Z0->nch*sizeof(double**));
		for(c=1;c<=top->Z0->nch;c++){
			i=c;
			top->horizon_height[r-1][c-1]=read_horizon(join_strings(WORKING_DIRECTORY,s->co[1]+1),i);
		}
	}
	free(s);

	free_doublematrix(M);

}


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
double **read_horizon(char *name, long i){

	FILE *f;
	DOUBLEMATRIX *M;
	long j, index;
	double **hor;

	f=fopen(namefile_i(name,i),"r");
	if(f!=NULL){
		fclose(f);

		f=t_fopen(namefile_i(name,i),"r");
		index=read_index(f,PRINT);
		M=read_doublematrix(f,"a",PRINT);
		t_fclose(f);

		if(M->nch>2){
			printf("Warning: in file %s columns after the 2nd will be neglected\n");
		}else if(M->nch<2){
			printf("Error: in file %s insufficient number of columns\n");
			t_error("ERROR!!");
		}
		hor=alloc2(M->nrh,2);
		for(j=1;j<=M->nch;j++){
			hor[j-1][0]=M->co[j][1];
			hor[j-1][1]=M->co[j][2];
		}

		free_doublematrix(M);

	}else{

		f=fopen(files->co[ferr]+1,"a");
		fprintf(f,"\nNo horizon file of the Meteo Station #%ld is present. In this case the horizon will be considered always not obscured, i.e. shadow=FALSE\n",i);
		fclose(f);
		printf("\nNo horizon file of the Meteo Station #%ld is present. In this case the horizon will be considered always not obscured, i.e. shadow=FALSE\n",i);

		f=t_fopen(namefile_i(name,i),"w");
		fprintf(f,"/** Horizon file for met station #%ld */\n",i);
		fprintf(f,"\n");
		fprintf(f,"1: double matrix horizon {4,2}\n");

		hor=alloc2(4,2);
		for(j=1;j<=4;j++){
			hor[j-1][0]=45.0+(j-1)*90.0;
			hor[j-1][1]=0.0;
			fprintf(f,"%f %f\n",hor[j-1][0],hor[j-1][1]);
		}
		t_fclose(f);

	}

	return(hor);

}




/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/
void init_meteo_stations(DOUBLEMATRIX *INPUTmeteo, METEO_STATIONS *st){

	long i;

	if(INPUTmeteo->nch!=13) t_error("ERROR IN INPUT OF METEO STATIONS");

	st->E=new_doublevector(INPUTmeteo->nrh);
	st->N=new_doublevector(INPUTmeteo->nrh);
	st->lat=new_doublevector(INPUTmeteo->nrh);
	st->lon=new_doublevector(INPUTmeteo->nrh);
	st->Z=new_doublevector(INPUTmeteo->nrh);
	st->sky=new_doublevector(INPUTmeteo->nrh);
	st->ST=new_doublevector(INPUTmeteo->nrh);
	st->Vheight=new_doublevector(INPUTmeteo->nrh);
	st->Theight=new_doublevector(INPUTmeteo->nrh);
	st->JD0=new_doublevector(INPUTmeteo->nrh);
	st->Y0=new_longvector(INPUTmeteo->nrh);
	st->Dt=new_doublevector(INPUTmeteo->nrh);
	st->offset=new_longvector(INPUTmeteo->nrh);

	for(i=1;i<=INPUTmeteo->nrh;i++){
		st->E->co[i]=INPUTmeteo->co[i][1];
		st->N->co[i]=INPUTmeteo->co[i][2];
		st->lat->co[i]=INPUTmeteo->co[i][3]*Pi/180.0;
		st->lon->co[i]=INPUTmeteo->co[i][4]*Pi/180.0;
		st->Z->co[i]=INPUTmeteo->co[i][5];
		st->sky->co[i]=INPUTmeteo->co[i][6];
		st->ST->co[i]=INPUTmeteo->co[i][7];
		st->Vheight->co[i]=INPUTmeteo->co[i][8];
		st->Theight->co[i]=INPUTmeteo->co[i][9];
		st->JD0->co[i]=INPUTmeteo->co[i][10];
		st->Y0->co[i]=(long)(INPUTmeteo->co[i][11]);
		st->Dt->co[i]=INPUTmeteo->co[i][12]*3600;		//seconds
		st->offset->co[i]=(long)(INPUTmeteo->co[i][13]);
	}

}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/
void ReadMeteoHeader(FILE *f, STRINGBIN *ColDescr, long offset, long *ncols, long *MeteoCont){

	char **a;
	long i,j;

	for(i=0;i<dim1l(MeteoCont);i++){
		MeteoCont[i]=-1;
	}

	a=readline_textarray(f, offset);

	*ncols=dim_vect_strings(a);

	for(j=1;j<=dim1l(MeteoCont);j++){
		for(i=1;i<=*ncols;i++){
			if(compare_strings(ColDescr->co[j]+1, a[i-1])==1 && MeteoCont[j-1]==-1){
				MeteoCont[j-1]=i-1;
				//printf("j:%ld dim:%ld ncol:%ld col:%ld\n",j,dim1l(MeteoCont),*ncols,MeteoCont[j-1]);
			}else if(compare_strings(ColDescr->co[j]+1, a[i-1])==1 && MeteoCont[j-1]!=-1){
				printf("Column '%s' is present twice\n",ColDescr->co[j]+1);
				t_error("Meteo Column name DUPLICATED!");
			}
		}
	}
	//stop_execution();

}




/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/
double **read_datameteo(FILE *f, long offset, long ncols, double ndef){

	double **a;
	long i, j;
	short end, novalueend=1;

	i=0;
	do{
		if(i==0){
			a=(double **)malloc(sizeof(double*));
		}else{
			a=(double **)realloc(a,(i+1)*sizeof(double*));
		}
		a[i]=(double *)malloc((ncols+1)*sizeof(double));
		a[i][ncols]=end_vector;
		readline_array(f, a[i], offset, ncols, ndef, &end);
		//printf("i:%ld %f\n",i,a[i][1]);
		if(end==0)i++;
	}while(end==0);


	for(j=1;j<=ncols;j++){
		if(a[i][j]!=ndef) novalueend=0;
		//printf("i:%ld %f\n",i,a[i][1]);
	}

	if(novalueend=0){
		i++;
		a=(double **)realloc(a,(i+1)*sizeof(double*));
		a[i]=(double *)malloc(sizeof(double));
	}
	a[i][0]=end_vector;

	return(a);
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/

void read_inpts_par(PAR *par, TIMES *times, char *program, char *ext, char *pos){

	DOUBLEVECTOR *V;
	double Dt_output;

	printf("\nENTERING SEVERAL CONTROL PROGRAM PAR\n");

	V=read_parameters("/", program, ext, pos);

	printf("ENTER THE INTEGRATION INTERVAL [s]: %f\n",V->co[1]);
	par->Dt=(double)V->co[1];

	printf("ENTER THE Decimal julian day at the beginning of simulation (0.0 - 365.99): %f\n",V->co[2]);
	par->JD0=(double)V->co[2];

	printf("ENTER THE YEAR at the beginning of simulation (0.0 - 365.99): %f\n",V->co[3]);
	par->year0=(long)V->co[3];

	printf("ENTER THE NUMBER OF DAYS OF SIMULATION: %f\n",V->co[4]);
	times->TH=(double)V->co[4];
	times->TH*=24; /*TH in hour*/

	printf("ENTER THE Standard time to which all the output data are referred (difference respect UMT, in hour): %f\n",V->co[5]);
	par->ST=(double)V->co[5];

	printf("ENTER THE NUMBER OF Dt AFTER WHICH THE OUTPUT FOR A SPECIFIED PIXEL ARE PRINTED: %f\n",V->co[6]);
	Dt_output=(double)V->co[6];
	if(Dt_output*3600<par->Dt) Dt_output=par->Dt/3600.0;
	times->n_pixel=(long)(Dt_output*3600/(long)par->Dt);
	times->i_pixel=0;/*counter for the output of a pixel*/

	printf("ENTER THE NUMBER OF Dt AFTER WHICH THE OUTPUT FOR THE BASIN ARE PRINTED: %f\n",V->co[7]);
	Dt_output=(double)V->co[7];
	if(Dt_output*3600<par->Dt) Dt_output=par->Dt/3600.0;
	times->n_basin=(long)(Dt_output*3600/(long)par->Dt);
	times->i_basin=0;/*counter for the output of a pixel*/

	printf(">=1 if you want to display RESULTS for altimetric stripes, it is how many altimetric stripes you want to consider (up to 99): %f\n",V->co[8]);
	par->ES_num=(short)V->co[8];
	if(par->ES_num>99 || par->ES_num<-99) t_error("Number of altimetric stripes not valid");
	if(par->ES_num<0){
		par->glac_thr=0.1;
		par->ES_num*=(-1);
	}else if(par->ES_num>0){
		par->glac_thr=-0.1;
	}

	printf("Multiplying factor decreasing the dem resolution for the calculation of the sky view factor: %f\n",V->co[9]);
	par->nsky=(short)V->co[9];
	if(par->nsky<1) t_error("Multiplying factor for sky view factor calculation must be greater than or equal to 1: %f");

	printf("Thershold for the definition of channel network (in pixel number draining): %f\n",V->co[10]);
	par->channel_thres=(double)V->co[10];

	printf("OUTPUT MAPS in fluidturtle format (=1), GRASS ASCII (=2), ESRI ASCII (=3): %f\n",V->co[11]);
	par->format_out=(short)V->co[11];

	printf("=1 for one point simulation, =0 for distributed simulation. The parameter file is different in these cases: %f\n",V->co[12]);
	par->point_sim=(short)V->co[12];

	printf("=1 if you want to recover a simulation, 0 otherwise: %f\n",V->co[13]);
	par->recover=(short)V->co[13];

	printf("\n");

	free_doublevector(V);
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/

void read_soil_parameters(char *name, long *nsoil, SOIL *sl){

	FILE *f;
	long index, i, j, k;
	DOUBLEMATRIX *M;

	f=t_fopen(join_strings(name,textfile),"r");
	index=read_index(f,PRINT);

	printf("%ld Different soil types given\n",index);

	M=read_doublematrix(f,"a",PRINT);
	Nl=M->nrh;

	sl->pa=new_doubletensor(index, nsoilprop, Nl);
	initialize_doubletensor(sl->pa,0.0);

	for(i=1;i<=index;i++){

		if(i>1){
			M=read_doublematrix(f,"a",PRINT);
			if(M->nrh!=Nl) t_error("The number of sl layers must be the same for each sl type");
		}

		if(M->nch!=nsoilprop)t_error("Wrong column number in the parameter file");

		for(j=1;j<=nsoilprop;j++){
			for(k=1;k<=M->nrh;k++){
				sl->pa->co[i][j][k]=M->co[k][j];
				if(j==jdz && i!=1){
					if(sl->pa->co[i][j][k]!=sl->pa->co[i-1][j][k]) t_error("Soil layer thicknesses must be the same for each sl type");
				}
				if(sl->pa->co[i][jlatfl][k]<0 || sl->pa->co[i][jlatfl][k]>2) t_error("Value not admitted of jlatfl in soil parameters");
				if(sl->pa->co[i][jsf][k]<0 || sl->pa->co[i][jsf][k]>1) t_error("Value not admitted of jsf in soil parameters");
				if(sl->pa->co[i][jKav][k]<0 || sl->pa->co[i][jKav][k]>1) t_error("Value not admitted of jKav in soil parameters");
			}
		}
		free_doublematrix(M);
	}

	t_fclose(f);

	*nsoil=index;

}


//***************************************************************************
//Check DD

DOUBLEMATRIX *depitted(SHORTMATRIX *DD, DOUBLEMATRIX *Z){

	static short r_DD[11]={0,0,-1,-1,-1,0,1,1,1,-9999,0}; /* differential of number-pixel for rows and*/
	static short c_DD[11]={0,1,1,0,-1,-1,-1,0,1,-9999,0}; /* columns, depending on Drainage Directions*/
	DOUBLEMATRIX *M;
	long r,c,i,cont=0;
	long maxiter=50;

	M=new_doublematrix(Z->nrh,Z->nch);
	for(r=1;r<=Z->nrh;r++){
		for(c=1;c<=Z->nch;c++){
			M->co[r][c]=Z->co[r][c];
		}
	}

	for(i=1;i<=maxiter;i++){
		for(r=1;r<=Z->nrh;r++){
			for(c=1;c<=Z->nch;c++){
				if(M->co[r][c]!=UV->V->co[2]){
					if(M->co[r][c]<M->co[r+r_DD[DD->co[r][c]]][c+c_DD[DD->co[r][c]]]){
						M->co[r+r_DD[DD->co[r][c]]][c+c_DD[DD->co[r][c]]]=M->co[r][c];
						if(i==maxiter) cont+=1;
					}
				}
			}
		}
	}

	if(cont>0) t_error("Not able to depit the dem, increase iteration number");

	return(M);

}

//***************************************************************************
void assign_recovered(char *name, double **assign, PAR *par, DOUBLEMATRIX *Z1, DOUBLEMATRIX *Z2){

	long r, c, i;
	DOUBLEMATRIX *M;

	if(par->point_sim==0){
		M=read_map(2, name, Z1, UV);
		for(r=1;r<=Nr;r++){
			for(c=1;c<=Nc;c++){
				assign[r][c]=M->co[r][c];
			}
		}
	}else{
		M=read_map(2, name, Z2, UV);
		for(i=1;i<=par->r_points->nh;i++){
			r=par->r_points->co[i];
			c=par->c_points->co[i];
			assign[1][i]=M->co[r][c];
		}
	}
	free_doublematrix(M);
}


//***************************************************************************
void assign_recovered_long(char *name, long **assign, PAR *par, DOUBLEMATRIX *Z1, DOUBLEMATRIX *Z2){

	long r, c, i;
	DOUBLEMATRIX *M;

	if(par->point_sim==0){
		M=read_map(2, name, Z1, UV);
		for(r=1;r<=Nr;r++){
			for(c=1;c<=Nc;c++){
				assign[r][c]=(long)M->co[r][c];
			}
		}
	}else{
		M=read_map(2, name, Z2, UV);
		for(i=1;i<=par->r_points->nh;i++){
			r=par->r_points->co[i];
			c=par->c_points->co[i];
			assign[1][i]=(long)M->co[r][c];
		}
	}
	free_doublematrix(M);
}
//***************************************************************************
