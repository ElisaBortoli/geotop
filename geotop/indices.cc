
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 2.0.0 - 9 Mar 2012
 
 Copyright (c), 2012 - Stefano Endrizzi
 
 This file is part of GEOtop 2.0.0 
 
 GEOtop 2.0.0  is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 2.0.0  is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */

#include "indices.h"
#include "geotop_common.h"
#include "inputKeywords.h"

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/


  void i_lrc_cont(GeoMatrix<double>& LC, long ***i, GeoMatrix<long>& lrc){
	
	long cont=0;
	long l, r, c;
	
	for(r=1; r<=geotop::common::Variables::Nr; r++){
		for(c=1; c<=geotop::common::Variables::Nc; c++){
			if((long)LC[r][c]!=geotop::input::gDoubleNoValue){
				for(l=0; l<=geotop::common::Variables::Nl; l++){
					cont++;
					i[l][r][c]=cont;
					lrc[cont][1]=l;
					lrc[cont][2]=r;
					lrc[cont][3]=c;
				}
			}
		}
	}
#ifdef VERBOSE
	  printf("i_lrc_cont: cont: %ld\n",cont);
#endif	  
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

//void j_rc_cont(DOUBLEMATRIX *LC, long **j, LONGMATRIX *rc)
void j_rc_cont(GeoMatrix<double>& LC, long **j, GeoMatrix<long>& rc){

	long cont=0;
	long r, c;
	for(r=1;r<=geotop::common::Variables::Nr;r++){
		for(c=1;c<=geotop::common::Variables::Nc;c++){

			if((long)LC[r][c]!=geotop::input::gDoubleNoValue){
				cont++;
				j[r][c]=cont;
				rc[cont][1]=r;;
				rc[cont][2]=c;
			}
		}
	}
	
#ifdef VERBOSE 	
	printf("j_lrc_cont: cont: %ld\n",cont);
#endif 
	
}


/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

// number of parameters same as uzh version SC 08.12.2013

void lch3_cont(long **ch3, GeoMatrix<long>& lch, long Nl, long nch){
	
	long cont=0;
	long l, ch;
	
	for (ch=1; ch<=nch; ch++) {
		for (l=0; l<=geotop::common::Variables::Nl; l++) {
			cont++;
			ch3[l][ch]=cont;
			lch[cont][1]=l;
			lch[cont][2]=ch;
		}
	}
#ifdef VERBOSE 
	printf("lch3_cont: cont: %ld\n",cont);
#endif 
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

//void cont_nonzero_values_matrix2(long *tot, long *totdiag, CHANNEL *cnet, DOUBLEMATRIX *LC, LONGMATRIX *lrc, long ***i, long n){

void cont_nonzero_values_matrix2(long *tot, long *totdiag, Channel *cnet, GeoMatrix<double>& LC, GeoMatrix<long>& lrc, long ***i, long n){
	
	long j, jj, l, r, c;
	long cnt=0, m=0;
	long N, M;
	
//	if (cnet->r->co[1] > 0) m = cnet->r->nh;

	if (cnet->r[1] > 0) m = cnet->r.size()-1; //HACK: assumption that r.size() > 0
	
	N = n*(geotop::common::Variables::Nl+1);
	M = m*(geotop::common::Variables::Nl+1);
	
#ifdef VERBOSE
	printf("cont_nonzero_values_matrix2: Nl:%ld n:%ld m:%ld N:%ld M:%ld \n",geotop::common::Variables::Nl,n,m,N,M);
#endif
	
	for(j=1; j<=N+M; j++){
		
		if (j<=N) {

			l=lrc[j][1];
			r=lrc[j][2];
			c=lrc[j][3];
		}else {
			jj=j-N;
			l=cnet->lch[jj][1];
		}
		
		//the cell itself
		//cnt ++;		
		
		//the cell below
		if (l<geotop::common::Variables::Nl) cnt ++;
		
		if (j<=N) {
			if((long)LC[r-1][c]!=geotop::input::gDoubleNoValue){
				if(i[l][r-1][c]>j) cnt ++;
			}
			
			if((long)LC[r+1][c]!=geotop::input::gDoubleNoValue){
				if(i[l][r+1][c]>j) cnt ++;
			}
			
			if((long)LC[r][c-1]!=geotop::input::gDoubleNoValue){
				if(i[l][r][c-1]>j) cnt ++;
			}
			
			if((long)LC[r][c+1]!=geotop::input::gDoubleNoValue){
				if(i[l][r][c+1]>j) cnt ++;
			}
			
			if(l>0 && cnet->ch[r][c]>0) cnt++;
		}
	}
	
	*tot = cnt;
	*totdiag = N+M;
#ifdef VERBOSE
	printf(" nonzero_values_matrix2 cnt:%ld totdiag:%ld  \n",cnt,*totdiag);	
#endif
	
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/


void cont_nonzero_values_matrix3(GeoVector<long>& Lp, GeoVector<long>& Li, Channel *cnet, GeoMatrix<double>& LC, GeoMatrix<long>& lrc, long ***i, long n){

	long j,jj,l,r,c;
	long cnt = 0;
	long m=0;
	long N, M;
	
//  note: if we have channel (m) should be one element less than r.size()  
//  to double check with/without channel 
//  S.C. 24.12.2013
	
	if (cnet->r[1] > 0) m = cnet->r.size()-1;
	
	N = n*(geotop::common::Variables::Nl+1);
	M = m*(geotop::common::Variables::Nl+1);
	
#ifdef VERBOSE
	printf("cont_nonzero_values_matrix3: Nl:%ld n:%ld m:%ld N:%ld M:%ld \n",geotop::common::Variables::Nl,n,m,N,M);
#endif

	for(j=1; j<=N+M; j++){
		
		if (j<=N) {
			l=lrc[j][1];		
			r=lrc[j][2];
			c=lrc[j][3];
		}else {
			jj=j-N;
			l=cnet->lch[jj][1];
		}
		
		//	the cell itself
		//	cnt++;
		//	Li->co[cnt] = j;
		
	//	the cell below
		if(l<geotop::common::Variables::Nl){
			cnt++;
			Li[cnt] = j+1;
		}
		
		if (j<=N) 
		{
			if((long)LC[r-1][c]!=geotop::input::gDoubleNoValue)
			{
				if(i[l][r-1][c]>j)
				{
					cnt++;
					Li[cnt] = i[l][r-1][c];
				}
			}
			
			if((long)LC[r+1][c]!=geotop::input::gDoubleNoValue)
			    {
				if(i[l][r+1][c]>j)
				{
					cnt++;
				    Li[cnt] = i[l][r+1][c];
				}
			}
			
			if((long)LC[r][c-1]!=geotop::input::gDoubleNoValue)
			{
				if(i[l][r][c-1]>j){
					cnt++;
					Li[cnt] = i[l][r][c-1];
				}
			}
			
			if((long)LC[r][c+1]!=geotop::input::gDoubleNoValue)
			{
				if(i[l][r][c+1]>j){
					cnt++;
					Li[cnt] = i[l][r][c+1];
				}
			}	
			
			if(l>0 && cnet->ch[r][c]>0){
				cnt++;
				Li[cnt] = N + cnet->ch3[l][cnet->ch[r][c]];
			}
			
		}
		
		
		Lp[j] = cnt;
		
#ifdef VERBOSE
	printf(" nonzero_values_matrix3 j:%ld cnt:%ld Li:%ld l:%ld r:%ld c:%ld ch:%ld \n",j,cnt,Li[cnt],l,r,c,cnet->ch[r][c]);
#endif
	}
	
	
}

/// We should remove commented code no longer used /////
/// are there below needed ? //////////////

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/*void cont_nonzero_values_matrix4(LONGVECTOR *Lp, LONGVECTOR *Li, LONGVECTOR *Up, LONGVECTOR *Ui, DOUBLEMATRIX *LC, 
								 LONGMATRIX *lrc, long ***i, long n, short point){
	
	//Li = line index
	//Lp = number of values for each row
	//Ui = line index transposed
	//Up = number of values for each row trasnposed
	//Axt such that Ax[Axt[i]] is the transposed
	
	long j,l,r,c;
	long cnt = 0, cntt = 0;
	
	for(j=1;j<=n;j++){
		
		l=lrc->co[j][1];
		r=lrc->co[j][2];
		c=lrc->co[j][3];
		
		//the cell itself
		cnt++;
		Li->co[cnt] = j;
		
		//the cell below
		if(l<Nl){
			cnt++;
			Li->co[cnt] = j+1;
		}
		
		if(l>0 && point!=1 && (long)LC->co[r-1][c]!=geotop::input::gDoubleNoValue){
			if(i[l][r-1][c]>j){
				cnt++;
				Li->co[cnt] = i[l][r-1][c];
			}else if(i[l][r-1][c]<j){
				cntt++;
				Ui->co[cntt] = i[l][r-1][c];
			}
		}
		
		if(l>0 && point!=1 && (long)LC->co[r+1][c]!=geotop::input::gDoubleNoValue){
			if(i[l][r+1][c]>j){
				cnt++;
				Li->co[cnt] = i[l][r+1][c];
			}else if(i[l][r+1][c]<j){
				cntt++;
				Ui->co[cntt] = i[l][r+1][c];
			}		
		}
		
		if(l>0 && point!=1 && (long)LC->co[r][c-1]!=geotop::input::gDoubleNoValue){
			if(i[l][r][c-1]>j){
				cnt++;
				Li->co[cnt] = i[l][r][c-1];
			}else if(i[l][r][c-1]<j){
				cntt++;
				Ui->co[cntt] = i[l][r][c-1];
			}		
		}
		
		if(l>0 && point!=1 && (long)LC->co[r][c+1]!=geotop::input::gDoubleNoValue){
			if(i[l][r][c+1]>j){
				cnt++;
				Li->co[cnt] = i[l][r][c+1];
			}else if(i[l][r][c+1]<j){
				cntt++;
				Ui->co[cntt] = i[l][r][c+1];
			}		
		}
		
		//the cell above
		if(l>0){
			cntt++;
			Ui->co[cntt] = j-1;
		}
		
		//the cell itself
		cntt++;
		Ui->co[cntt] = j;
		
		Lp->co[j] = cnt;
		Up->co[j] = cntt;
	}
	
}*/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
