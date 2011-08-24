
/* STATEMENT:
 
 GEOtop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 GEOtop 1.145 'Montebello' - 8 Nov 2010
 
 Copyright (c), 2010 - Stefano Endrizzi - Geographical Institute, University of Zurich, Switzerland - stefano.endrizzi@geo.uzh.ch 
 
 This file is part of GEOtop 1.145 'Montebello'
 
 GEOtop 1.145 'Montebello' is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
 
 GEOtop 1.145 'Montebello' is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the GEOtop model. Any feedback will be highly appreciated.
 
 If you have satisfactorily used the code, please acknowledge the authors.
 
 */

#include "struct.geotop.h"

extern long Nl, Nr, Nc;
extern long number_novalue;

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void i_lrc_cont(DOUBLEMATRIX *LC, long ***i, LONGMATRIX *lrc){
	
	long cont=0;
	long l, r, c;
	
	for(r=1; r<=Nr; r++){
		for(c=1; c<=Nc; c++){
			if((long)LC->co[r][c]!=number_novalue){
				for(l=0; l<=Nl; l++){
					cont++;
					i[l][r][c]=cont;
					lrc->co[cont][1]=l;
					lrc->co[cont][2]=r;
					lrc->co[cont][3]=c;
				}
			}
		}
	}
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void j_rc_cont(DOUBLEMATRIX *LC, long **j, LONGMATRIX *rc){
	
	long cont=0;
	long r, c;
	for(r=1;r<=Nr;r++){
		for(c=1;c<=Nc;c++){
			if((long)LC->co[r][c]!=number_novalue){
				cont++;
				j[r][c]=cont;
				rc->co[cont][1]=r;
				rc->co[cont][2]=c;
			}
		}
	}
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void lch3_cont(long n, long **ch3, LONGMATRIX *lch){
	
	long cont=0;
	long l, ch;
	
	for (ch=1; ch<=n; ch++) {
		for (l=0; l<=Nl; l++) {
			cont++;
			ch3[l][ch]=cont;
			lch->co[cont][1]=l;
			lch->co[cont][2]=ch;
		}
	}
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void cont_nonzero_values_matrix2(long *tot, long *totdiag, CHANNEL *cnet, DOUBLEMATRIX *LC, LONGMATRIX *lrc, long ***i, long n, short point){
	
	long j, jj, l, r, c;
	long cnt=0, m=0;
	long N, M;
	
	if (cnet->r->co[1] > 0) m = cnet->r->nh;
	
	N = n*(Nl+1);
	M = m*(Nl+1);
	
	for(j=1; j<=N+M; j++){
		
		if (j<=N) {
			l=lrc->co[j][1];
			r=lrc->co[j][2];
			c=lrc->co[j][3];
		}else {
			jj=j-N;
			l=cnet->lch->co[jj][1];
		}
		
		//the cell itself
		//cnt ++;		
		
		//the cell below
		if (l<Nl) cnt ++;
		
		if (j<=N) {
			if(l>0 && point!=1 && (long)LC->co[r-1][c]!=number_novalue){
				if(i[l][r-1][c]>j) cnt ++;
			}
			
			if(l>0 && point!=1 && (long)LC->co[r+1][c]!=number_novalue){
				if(i[l][r+1][c]>j) cnt ++;
			}
			
			if(l>0 && point!=1 && (long)LC->co[r][c-1]!=number_novalue){
				if(i[l][r][c-1]>j) cnt ++;
			}
			
			if(l>0 && point!=1 && (long)LC->co[r][c+1]!=number_novalue){
				if(i[l][r][c+1]>j) cnt ++;
			}
			
			if(l>0 && point!=1 && cnet->ch->co[r][c]>0) cnt++;
		}
	}
	
	*tot = cnt;
	*totdiag = N+M;
	
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void cont_nonzero_values_matrix3(LONGVECTOR *Lp, LONGVECTOR *Li, CHANNEL *cnet, DOUBLEMATRIX *LC, LONGMATRIX *lrc, long ***i, long n, short point){
	
	//Ai = line index
	//Ap = number of values for each row
	long j,jj,l,r,c;
	long cnt = 0;
	long m=0;
	long N, M;
	
	if (cnet->r->co[1] > 0) m = cnet->r->nh;
	
	N = n*(Nl+1);
	M = m*(Nl+1);
	
	for(j=1; j<=N+M; j++){
		
		if (j<=N) {
			l=lrc->co[j][1];
			r=lrc->co[j][2];
			c=lrc->co[j][3];
		}else {
			jj=j-N;
			l=cnet->lch->co[jj][1];
		}
		
		//the cell itself
		//cnt++;
		//Li->co[cnt] = j;
		
		//the cell below
		if(l<Nl){
			cnt++;
			Li->co[cnt] = j+1;
		}
		
		if (j<=N) {
			if(l>0 && point!=1 && (long)LC->co[r-1][c]!=number_novalue){
				if(i[l][r-1][c]>j){
					cnt++;
					Li->co[cnt] = i[l][r-1][c];
				}
			}
			
			if(l>0 && point!=1 && (long)LC->co[r+1][c]!=number_novalue){
				if(i[l][r+1][c]>j){
					cnt++;
					Li->co[cnt] = i[l][r+1][c];
				}
			}
			
			if(l>0 && point!=1 && (long)LC->co[r][c-1]!=number_novalue){
				if(i[l][r][c-1]>j){
					cnt++;
					Li->co[cnt] = i[l][r][c-1];
				}
			}
			
			if(l>0 && point!=1 && (long)LC->co[r][c+1]!=number_novalue){
				if(i[l][r][c+1]>j){
					cnt++;
					Li->co[cnt] = i[l][r][c+1];
				}
			}	
			
			if(l>0 && point!=1 && cnet->ch->co[r][c]>0){
				cnt++;
				Li->co[cnt] = N + cnet->ch3[l][cnet->ch->co[r][c]];
			}
			
		}
		
		Lp->co[j] = cnt;
		//printf("j:%ld cnt:%ld Li:%ld l:%ld r:%ld c:%ld ch:%ld \n",j,cnt,Li->co[cnt],l,r,c,cnet->ch->co[r][c]);
	}
	
	
}

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
		
		if(l>0 && point!=1 && (long)LC->co[r-1][c]!=number_novalue){
			if(i[l][r-1][c]>j){
				cnt++;
				Li->co[cnt] = i[l][r-1][c];
			}else if(i[l][r-1][c]<j){
				cntt++;
				Ui->co[cntt] = i[l][r-1][c];
			}
		}
		
		if(l>0 && point!=1 && (long)LC->co[r+1][c]!=number_novalue){
			if(i[l][r+1][c]>j){
				cnt++;
				Li->co[cnt] = i[l][r+1][c];
			}else if(i[l][r+1][c]<j){
				cntt++;
				Ui->co[cntt] = i[l][r+1][c];
			}		
		}
		
		if(l>0 && point!=1 && (long)LC->co[r][c-1]!=number_novalue){
			if(i[l][r][c-1]>j){
				cnt++;
				Li->co[cnt] = i[l][r][c-1];
			}else if(i[l][r][c-1]<j){
				cntt++;
				Ui->co[cntt] = i[l][r][c-1];
			}		
		}
		
		if(l>0 && point!=1 && (long)LC->co[r][c+1]!=number_novalue){
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
