
/* STATEMENT:

 Geotop MODELS THE ENERGY AND WATER FLUXES AT THE LAND SURFACE
 Geotop 1.225-15 - 20 Jun 2013

 Copyright (c), 2013 - Stefano Endrizzi

 This file is part of Geotop 1.225-15

 Geotop 1.225-15  is a free software and is distributed under GNU General Public License v. 3.0 <http://www.gnu.org/licenses/>
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

 Geotop 1.225-15  is distributed as a free software in the hope to create and support a community of developers and users that constructively interact.
 If you just use the code, please give feedback to the authors and the community.
 Any way you use the model, may be the most trivial one, is significantly helpful for the future development of the Geotop model. Any feedback will be highly appreciated.

 If you have satisfactorily used the code, please acknowledge the authors.

 */

#include <tensor.h>
#include "turtle.h"
#include "rw_maps.h"
#include "import_ascii.h"
#include "write_ascii.h"
#include "extensions.h"
#include "t_datamanipulation.h"
/* #include "tensor3D.h" */


/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

Matrix<short> * copyshort_doublematrix(Matrix<double> *M)
{
    Matrix<short> *S;
    long r, c;

    S = new Matrix<short>{M->nrh,M->nch};
    for (r=1; r<=M->nrh; r++)
    {
        for (c=1; c<=M->nch; c++)
        {
            (*S)(r,c)=(short)(*M)(r,c);
        }
    }

    return (S);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

Matrix<long> *copylong_doublematrix(Matrix<double> *M)
{
    Matrix<long> *L;
    long r, c;

    L= new Matrix<long>{M->nrh,M->nch};
    for (r=1; r<=M->nrh; r++)
    {
        for (c=1; c<=M->nch; c++)
        {
            (*L)(r,c)=(long)(*M)(r,c);
        }
    }

    return (L);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

Matrix<double>* copydouble_longmatrix(Matrix<long> *L)
{
    Matrix<double> *M;
    long r, c;

    M = new Matrix<double>{L->nrh,L->nch};
    for (r=1; r<=L->nrh; r++)
    {
        for (c=1; c<=L->nch; c++)
        {
            (*M)(r,c)=(double)(*L)(r,c);
        }
    }

    return (M);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

Matrix<double> *copydoublematrix_const(double c0, Matrix<double> *Mref, double NOVALUE)
{
    Matrix<double> *M;
    long r, c;

    M = new Matrix<double>{Mref->nrh,Mref->nch};

    for (r=1; r<=M->nrh; r++)
    {
        for (c=1; c<=M->nch; c++)
        {
            if ((*Mref)(r,c)==NOVALUE)
            {
                (*M)(r,c)=NOVALUE;
            }
            else
            {
                (*M)(r,c)=c0;
            }
        }
    }
    return (M);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_suffix(char *suffix, long i, short start)
{
    short m=0, c=0, d=0, u=0;

    if (i>=0 && i<=9)
    {
        m = 0;
        c = 0;
        d = 0;
        u = i;
    }
    else if (i<=99)
    {
        m = 0;
        c = 0;
        d = (short)(i/10.0);
        u = i-10.0*d;
    }
    else if (i<=999)
    {
        m = 0;
        c = (short)(i/100.0);
        d = (short)((i-100.0*c)/10.0);
        u = i-100.0*c-10*d;
    }
    else if (i<=9999)
    {
        m = (short)(i/1000.0);
        c = (short)((i-1000.0*m)/100.0);
        d = (short)((i-1000.0*m-100.0*c)/10.0);
        u = i-1000*m-100.0*c-10*d;
    }
    else
    {
        t_error("Number too high");
    }

    m += 48;
    c += 48;
    d += 48;
    u += 48;

    suffix[start] = m;
    suffix[start+1] = c;
    suffix[start+2] = d;
    suffix[start+3] = u;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

char *namefile_i(char *name, long i)
{
    char SSSS[ ]= {"SSSS"};
    char *name_out;
    char *temp;

    write_suffix(SSSS, i, 0);

    temp=join_strings(name,SSSS);
    name_out=join_strings(temp,textfile);
    free(temp);

    return (name_out);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

char *namefile_i_we(char *name, long i)
{
    char SSSS[ ]= {"LSSSS"};
    char *name_out;

    write_suffix(SSSS, i, 1);

    name_out=join_strings(name,SSSS);

    return (name_out);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

char *namefile_i_we2(char *name, long i)
{
    char SSSS[ ]= {"SSSS"};
    char *name_out;

    write_suffix(SSSS, i, 0);

    name_out=join_strings(name,SSSS);

    return (name_out);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

short existing_file(char *name)
{
/**
 * gives
 * 1: if the file exists (fluidturtle)
 * 2: if the file exists (grassascii)
 * 3: if the file exists (esriascii),
 * 0: if the file doesn't exist
 */
    short a=0;
    FILE *f;
    //char *ft;
    char *esri,*grass;

    //ft=join_strings(name,ascii_ft);
    esri=join_strings(name,ascii_esri);
    grass=join_strings(name,ascii_grass);

    //if( (f=fopen(ft,"r"))!=nullptr ){
    //  a=1;
    //  fclose(f);
    //}else
    if ( (f=fopen(grass,"r"))!=nullptr )
    {
        a=2;
        fclose(f);
    }
    else if ( (f=fopen(esri,"r"))!=nullptr )
    {
        a=3;
        fclose(f);
    }

    //free(ft);
    free(esri);
    free(grass);

    return (a);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

short existing_file_wext(char *name, char *extension)
{
/**
 * gives
 * 1: if the file exists
 * 0: if the file doesn't exist
 */
    short a=0;
    FILE *f;
    char *temp;

    temp=join_strings(name,extension);

    if ( (f=fopen(temp,"r"))!=nullptr )
    {
        a=1;
        fclose(f);
    }

    free(temp);

    return (a);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

short existing_file_woext(char *name)
{
/**
 * gives
 * 1: if the file exists
 * 0: if the file doesn't exist
 */

    short a=0;
    FILE *f;

    if ( (f=fopen(name,"r"))!=nullptr )
    {
        a=1;
        fclose(f);
    }

    return (a);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

Matrix<double>* read_map(short a, char *filename, Matrix<double> *Mref,
                         T_INIT *UVref, double no_value)
{
/**
 * a=0 does not use Mref, UVref output
 * a=1 does not check novalues, Mref e UVref input
 * a=2 checks novalues, Mref e UVref input
 */
    Matrix<double> *M=nullptr;
    long r=0, c=0, nr=0, nc=0;
    double *header=nullptr, *m=nullptr;
    double Dxmap=0, Dymap=0, X0map=0, Y0map=0;

    if (a != 0 && a != 1
        && a != 2)
        t_error("Value of flag not supported in the subroutine in read_map");

    if (existing_file(filename)==1)
    {

        t_error("Fluidturtle map format not supported any more");

    }
    else
    {

        header = (double *) malloc(6*sizeof(double));

        if (existing_file(filename)==2) //grass ascii
        {
            m=read_grassascii(header, no_value, filename);
            nr=(long)header[4];
            nc=(long)header[5];
            Dxmap=(header[2]-header[3])/((long)header[5]);
            Dymap=(header[0]-header[1])/((long)header[4]);
            X0map=header[3];
            Y0map=header[1];

        }
        else if (existing_file(filename)==3)  //esri ascii
        {
            m=read_esriascii(header, no_value, filename);
            printf("%s\n",filename);
            nr=(long)header[1];
            nc=(long)header[0];
            Dxmap=header[4];
            Dymap=header[4];
            X0map=header[2];
            Y0map=header[3];

        }
        else
        {

            printf("The file %s doesn't exist\n",filename);
            t_error("Fatal error");

        }

        free(header);
        if (a==1 || a==2)
        {
            /**check header*/
            if (Dxmap != (*UVref->U)(2))
            {
                printf("Dx in %s file is different from Dx in DTM file! \n",filename);
                t_error("Inconsistent map");
            }
            if (Dymap!=(*UVref->U)(1))
            {
                printf("Dy:%f in %s file is different from Dy:%f in DTM file! \n",Dymap,
                       filename,(*UVref->U)(1));
                t_error("Inconsistent map");
            }
            if (X0map != (*UVref->U)(4))
            {
                printf("X0 in %s file is different from X0 in DTM file! \n",filename);
                t_error("Inconsistent map");
            }
            if (Y0map != (*UVref->U)(3))
            {
                printf("Y0 in %s file is different from Y0 in DTM file! \n",filename);
                t_error("Inconsistent map");
            }
            if (nr!=Mref->nrh)
            {
                printf("Number of rows in %s file (%ld) is not consistent with DTM file (%ld)! \n",
                       filename,nr,Mref->nrh);
                t_error("Inconsistent map");
            }
            if (nc!=Mref->nch)
            {
                printf("Number of columns in %s file is not consistent with DTM file! \n",
                       filename);
                t_error("Inconsistent map");
            }

        }
        else if (a==0)
        {

            UVref->U.reset(new Vector<double>{4});
            UVref->V.reset(new Vector<double>{2});
            (*UVref->U)(2) = Dxmap;
            (*UVref->U)(1) = Dymap;
            (*UVref->U)(4) = X0map;
            (*UVref->U)(3) = Y0map;

            (*UVref->V)(2) = no_value;
            if (no_value<0)
            {
                (*UVref->V)(1)=-1;
            }
            else
            {
                (*UVref->V)(1)=1;
            }
        }

        /**assign values and check novalues*/
        M = new Matrix<double>{nr,nc};
        for (r=1; r<=nr; r++)
        {
            for (c=1; c<=nc; c++)
            {
                (*M)(r,c)=m[(r-1)*nc+c-1];
                if (a==2)
                {
                    if ((*M)(r,c)==no_value && (*Mref)(r,c)!=no_value)
                    {
                        printf("ERROR:: Problem reading map %s, it has novalue where the reference maps has value: %ld %ld ref:%f %f\n",
                               filename,r,c,(*Mref)(r,c),(*M)(r,c));
                        t_error("Fatal Error (9)");
                    }
                    if ((*M)(r,c)!=no_value && (*Mref)(r,c)==no_value) (*M)(r,c)=no_value;
                }
            }
        }

        free(m);
    }

    return (M);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

std::unique_ptr<Vector<double>> read_map_vector(short type, char *namefile, Matrix<double> *mask,
                                                T_INIT *grid, double no_value, Matrix<long> *rc)
{
    std::unique_ptr<Matrix<double>> M;
    long i, n=rc->nrh;
    std::unique_ptr<Vector<double>> V{new Vector<double>{n}};

    M.reset(read_map(type, namefile, mask, grid, no_value));

    for (i=1; i<=n; i++)
    {
        (*V)(i) = (*M)((*rc)(i,1),(*rc)(i,2));
    }

    return V;
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_map(char *filename, short type, short format, Matrix<double> *M,
               T_INIT *UV, long novalue)
{
    /**
     * type=0  floating point
     * type=1  integer
     *
     * format=1 fluidturtle
     * format=2 grassascii
     * format=3 esriascii
     */

    if (format==1)
    {
        t_error("The fluidturtle format is not support any more");
    }
    else if (format==2)
    {
        write_grassascii(filename, type, M, UV, novalue);
    }
    else if (format==3)
    {
        write_esriascii(filename, type, M, UV, novalue);
    }
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_map_vector(char *filename, short type, short format,
                      Vector<double> *V, T_INIT *UV, long novalue, long **j, long nr, long nc)
{
    /**
     * type=0  floating point
     * type=1  integer
     *
     * format=1 fluidturtle
     * format=2 grassascii
     * format=3 esriascii
     */
    if (format==1)
    {
        t_error("The fluidturtle format is not support any more");
    }
    else if (format==2)
    {
        write_grassascii_vector(filename, type, V, j, nr, nc, UV, novalue);
    }
    else if (format==3)
    {
        write_esriascii_vector(filename, type, V, j, nr, nc, UV, novalue);
    }
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_tensorseries(short a, long l, long i, char *filename, short type,
                        short format, Tensor<double> *T, T_INIT *UV, long novalue)
{
/**
 * a=0 does not include "l" in the suffix
 * a=1 includes "l" in the suffix
 * l:layer
 * i:temporal step
 */
    char SSSSLLLLL[ ]= {"SSSSLLLLL"};
    char SSSS[ ]= {"SSSS"};
    char *name=nullptr;
    long r, c;
    std::unique_ptr<Matrix<double>> M;

    if (a==0)
    {
        write_suffix(SSSS, i, 0);
        name=join_strings(filename,SSSS);
    }
    else if (a==1)
    {
        write_suffix(SSSSLLLLL, i, 0);
        write_suffix(SSSSLLLLL, l, 5);
        name=join_strings(filename,SSSSLLLLL);
    }
    else
    {
        t_error("Value not admitted");
    }


    M.reset(new Matrix<double>{T->nrh,T->nch});
    for (r=1; r<=T->nrh; r++)
    {
        for (c=1; c<=T->nch; c++)
        {
            (*M)(r,c) = (*T)(l,r,c);
        }
    }

    write_map(name, type, format, M.get(), UV, novalue);

    free(name);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_tensorseries_vector(short a, long l, long i, char *filename,
                               short type, short format, Matrix<double> *T, T_INIT *UV, long novalue, long **J,
                               long nr, long nc)
{
/**
 * a=0 does not include "l" in the suffix
 * a=1 includes "l" in the suffix
 * l:layer
 * i:temporal step
 */
    char SSSSLLLLL[ ]= {"SSSSLLLLL"};
    char SSSS[ ]= {"SSSS"};
    char *name=nullptr;
    long j, npoints=T->nch;
    std::unique_ptr<Vector<double>> V;

    if (a==0)
    {
        write_suffix(SSSS, i, 0);
        name=join_strings(filename,SSSS);
    }
    else if (a==1)
    {
        write_suffix(SSSSLLLLL, i, 0);
        write_suffix(SSSSLLLLL, l, 5);
        name=join_strings(filename,SSSSLLLLL);
    }
    else
    {
        t_error("Value not admitted");
    }

    V.reset(new Vector<double>{npoints});
    for (j=1; j<=npoints; j++)
    {
        (*V)(j)=(*T)(l,j);
    }

    write_map_vector(name, type, format, V.get(), UV, novalue, J, nr, nc);

    free(name);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void rename_tensorseries(short a, long l, long i, char *filename)
{
/**
 * a=0 does not include "l" in the suffix
 * a=1 includes "l" in the suffix
 * l:layer
 * i:temporal step
 */
    char SSSSLLLLL[ ]= {"SSSSLLLLL"};
    char SSSS[ ]= {"SSSS"};
    char *name=nullptr;

    if (a==0)
    {
        write_suffix(SSSS, i, 0);
        name=join_strings(filename,SSSS);
    }
    else if (a==1)
    {
        write_suffix(SSSSLLLLL, i, 0);
        write_suffix(SSSSLLLLL, l, 5);
        name=join_strings(filename,SSSSLLLLL);
    }
    else
    {
        t_error("Value not admitted");
    }

    rename_map(name);

    free(name);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void rename_map(char *filename)
{
    char *name, *namenw, *temp;

    if (existing_file(filename) == 2)
    {
        name = join_strings(filename, ascii_grass);
        temp = join_strings(filename, ".old");
        namenw = join_strings(temp, ascii_grass);
        rename(name, namenw);
        free(namenw);
        free(name);
        free(temp);
    }
    else if (existing_file(filename) == 3)
    {
        name = join_strings(filename, ascii_esri);
        temp = join_strings(filename, ".old");
        namenw = join_strings(temp, ascii_esri);
        rename(name, namenw);
        free(namenw);
        free(name);
        free(temp);
    }
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_tensorseries2(char *suf, long l, char *filename, short type,
                         short format, DOUBLETENSOR *T, T_INIT *UV, long novalue) // maybe NOT used
{
    char LLLLL[ ]= {"LLLLL"};
    char *temp1, *temp2;
    long r, c;
    std::unique_ptr<Matrix<double>> M;

    temp1 = join_strings(LLLLL, suf);
    write_suffix(temp1, l, 1);

    M.reset(new Matrix<double>{T->nrh,T->nch});
    for (r=1; r<=T->nrh; r++)
    {
        for (c=1; c<=T->nch; c++)
        {
            (*M)(r,c) = T->co[l][r][c];
        }
    }

    temp2 = join_strings(filename, temp1);
    write_map(temp2, type, format, M.get(), UV, novalue);

    free(temp1);
    free(temp2);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_tensorseries2_vector(char *suf, long l, char *filename, short type,
                                short format, Matrix<double> *T, T_INIT *UV, long novalue, long **J, long nr,
                                long nc)
{
    char LLLLL[ ]= {"LLLLL"};
    char *temp1, *temp2;
    long i, npoints=T->nch;
    std::unique_ptr<Vector<double>> V;

    temp1 = join_strings(LLLLL, suf);
    write_suffix(temp1, l, 1);

    V.reset(new Vector<double>{npoints});

    for (i=1; i<=npoints; i++)
    {
        (*V)(i) = (*T)(l,i);
    }

    temp2 = join_strings(filename, temp1);
    write_map_vector(temp2, type, format, V.get(), UV, novalue, J, nr, nc);

    free(temp1);
    free(temp2);
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/

void write_tensorseries3_vector(char *suffix, char *filename, short type,
                                short format, Matrix<double> *T, T_INIT *UV, long novalue, long **J, long nr,
                                long nc)
{
    long l;
    for (l=T->nrl; l<=T->nrh; l++)
    {
        write_tensorseries2_vector(suffix, l, filename, type, format, T, UV, novalue, J, nr, nc);
    }
}

/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
/******************************************************************************************************************************************/
