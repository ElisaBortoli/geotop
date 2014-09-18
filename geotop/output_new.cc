/**
 * @file output_new.cc
 * @author Gianfranco Gallizia
 * @copyright (C) 2014 eXact lab srl
 * @date 07/2014
 * @brief GEOtop output post-processing routines
 */

#include "output_new.h"
#include "constants.h"
#include "inputKeywords.h"
#include "output_file.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "times.h"
#include "geotop_common.h"
#include "global_logger.h"
#include "../gt_utilities/path_utils.h"

#ifdef METEOIO_OUTPUT
#define __MATHOPTIM_H__
#include <meteoio/MeteoIO.h>
#endif

/*==============================================================================
   Constants
 ==============================================================================*/

static const double epsilon = 1.E-6;

/*==============================================================================
   Classes
 ==============================================================================*/

/**
 * @internal 
 * @brief Holds a vector of output files
 */
class OutputFilesVector
{
    public:
        OutputFilesVector(double period);
        virtual ~OutputFilesVector();
        double period() { return mPeriod; };
        void append(geotop::input::OutputFile of);
        geotop::input::OutputFile at(size_t index);
        bool empty() { return (mVector->size() == 0); }
        size_t size() { return mVector->size(); }
    private:
        double mPeriod;
        std::vector<geotop::input::OutputFile>* mVector;
} ;

OutputFilesVector::OutputFilesVector(double period)
{
    mPeriod = period;
    mVector = new std::vector<geotop::input::OutputFile>();
}

OutputFilesVector::~OutputFilesVector()
{
    delete mVector;
}

void OutputFilesVector::append(geotop::input::OutputFile of)
{
    mVector->push_back(of);
}

geotop::input::OutputFile OutputFilesVector::at(size_t index)
{
    return mVector->at(index);
}

/**
 * @internal 
 * @brief Holds the informations about the output files
 */
class OutputFilesStore
{
    public:
        OutputFilesStore();
        virtual ~OutputFilesStore();
        std::vector<OutputFilesVector*>* instants;
        std::vector<OutputFilesVector*>* cumulates;
        std::vector<OutputFilesVector*>* averages;
} ;

OutputFilesStore::OutputFilesStore()
{
    instants = NULL;
    cumulates = NULL;
    averages = NULL;
}

OutputFilesStore::~OutputFilesStore()
{
    if (instants != NULL)
    {
        delete instants;
        instants = NULL ;
    }

    if (cumulates != NULL)
    {
        delete cumulates;
        cumulates = NULL ;
    }

    if (averages != NULL)
    {
        delete averages;
        averages = NULL;
    }
}

/*==============================================================================
   Static variables
 ==============================================================================*/

static OutputFilesStore ofs;
#ifdef METEOIO_OUTPUT
static mio::Config cfg;
#endif

/*==============================================================================
   Static functions prototypes
 ==============================================================================*/

/**
 * @internal
 * @brief gets the supervector olding the values for an output variable
 * @param A global data storage pointer
 * @param what variable to fetch
 * @return a GeoMatrix with one row per layer, each row holding the values for that layer
 */
static GeoMatrix<double>* getSupervectorVariableM(AllData* A, geotop::input::Variable what);

//Same as above but for 2D-only variables
static GeoVector<double>* getSupervectorVariableV(AllData* A, geotop::input::Variable what);

/*==============================================================================
   Static functions
 ==============================================================================*/

static bool compareByPeriod(geotop::input::OutputFile a, geotop::input::OutputFile b)
{
    return (a.getPeriod() < b.getPeriod());
}

/**
 * @internal
 * @brief extracts a single row from a GeoMatrix and puts it into a new GeoVector
 * @param[in] M the matrix
 * @param[in] layer the layer's index
 * @return a newly created GeoVector holding the data of that layer.
 *  WARNING: you have to explicitly call delete on that GeoVector.
 */
static GeoVector<double>* extractGeoVector(GeoMatrix<double>* M, long layer)
{
    GeoVector<double>* V = new GeoVector<double>(M->getCols());

    for (size_t i = 0, s = V->size(); i < s; i++)
    {
        (*V)[i] = (*M)[layer][i];
    }

    return V;
}

/**
 * @internal
 * @brief Gets a single layer's supervector
 * @param[in] layer the layer's index
 * param[in] of pointer to an OutputFile specification object
 * param[in] A pointer to the AllData structure
 * @return a newly created GeoVector holding the requested data.
 * WARNING; the caller has to explicitly delete that GeoVector.
 */
static GeoVector<double>* getSupervectorLayer(long layer, geotop::input::OutputFile* of, AllData* A)
{
    GeoVector<double>* output = NULL;

    GeoMatrix<double>* M = getSupervectorVariableM(A, of->getVariable());

    if (M != NULL)
    {
        output = extractGeoVector(M, layer);
    }
    else
    {
        GeoVector<double>* T = getSupervectorVariableV(A, of->getVariable());

        if (T != NULL)
        {
            output = new GeoVector<double>(T->size());
            for (size_t i=0, s = T->size(); i < s; i++)
            {
                output[i] = T->at(i);
            }
        }
    }

    return output;
}

static GeoVector<double>* initTempValuesVector(size_t count)
{
    GeoVector<double>* output = new GeoVector<double>(count, 0.);

    return output;
}

static GeoMatrix<double>* initTempValuesMatrix(size_t rows, size_t cols)
{
    GeoMatrix<double>* output = new GeoMatrix<double>(rows,
                                                      cols,
                                                      0.);

    return output;
}

static GeoTensor<double>* initTempValuesTensor(size_t layers, size_t rows, size_t cols)
{
    GeoTensor<double>* output = new GeoTensor<double>(layers,
                                                      rows,
                                                      cols,
                                                      0.);

    return output;
}

static void initTemporaryValues(geotop::input::OutputFile& of, AllData* A)
{
    switch(of.getDimension())
    {
        case geotop::input::D1Dp:
            break;
        case geotop::input::D1Ds:
            {
                size_t count;

                switch(of.getVariable())
                {
                    //Soil variables
                    case geotop::input::SOIL_TEMP:
                    case geotop::input::SOIL_WATER_CONTENT:
                        count = geotop::common::Variables::Nl;
                        break;
                    default:
                        count = 0;
                }

                of.values = geotop::input::TemporaryValues(initTempValuesVector(count));
            }
            break;
        case geotop::input::D2D:
            {
                size_t count;

                switch (of.getVariable())
                {
                    //Soil variables
                    case geotop::input::SOIL_TEMP:
                    case geotop::input::SOIL_WATER_CONTENT:
                        count = A->P->total_pixel + 1;
                        break;
                    default:
                        count = 0;
                        break;
                }

                of.values = geotop::input::TemporaryValues(initTempValuesVector(count));
            }
            break;
        case geotop::input::D3D:
            {
                size_t rows, cols;

                switch (of.getVariable())
                {
                    //Soil variables
                    case geotop::input::SOIL_TEMP:
                    case geotop::input::SOIL_WATER_CONTENT:
                        rows = geotop::common::Variables::Nl + 1;
                        cols = A->P->total_pixel + 1;
                        of.values = geotop::input::TemporaryValues(initTempValuesMatrix(rows, cols));
                        break;
                    default:
                        rows = 0;
                        cols = 0;
                        of.values = geotop::input::TemporaryValues(initTempValuesMatrix(rows, cols));
                        break;
                }

            }
            break;
        default:
            break;

    }
}

static void zeroFill(geotop::input::OutputFile* of)
{

    switch(of->getDimension())
    {
        case geotop::input::D1Dp:
            break;
        case geotop::input::D1Ds:
        case geotop::input::D2D:
            {
                GeoVector<double>* V = of->values.getValuesV();
                V->resize(V->size(), 0.);
            }
            break;
        case geotop::input::D3D:
            {
                GeoMatrix<double>* M = of->values.getValuesM();
                M->reset(0.);
            }
            break;
        default:
            break;
    }
}

static void refreshTemporaryValuesV(geotop::input::OutputFile* f, AllData* A, long layer)
{

    GeoMatrix<double>* M = getSupervectorVariableM(A, f->getVariable());

    if (M != NULL)
    {
        GeoVector<double>* V = f->values.getValuesV();

        assert(V->size() == M->getCols());

        for (size_t i = 0, s = V->size(); i < s; i++)
        {
            (*V)[i] = (*M)[layer][i];
        }
    }
    else
    {
        GeoVector<double>* V = getSupervectorVariableV(A, f->getVariable());

        if (V != NULL)
        {
            GeoVector<double>* TV = f->values.getValuesV();

            assert(TV->size() == V->size());

            for (size_t i = 0, s = TV->size(); i < s; i++)
            {
                (*TV)[i] = V->at(i);
            }

        }
        else
        {
            std::string vname = geotop::input::OutputFile::var2str(f->getVariable());

            geotop::logger::GlobalLogger* lg =
                geotop::logger::GlobalLogger::getInstance();
            lg->logsf(geotop::logger::CRITICAL,
                      "Unable to retrieve data for variable '%s'. Aborting.",
                      vname.c_str());
            exit(1);
        }
    }

}


static void refreshTemporaryValuesM(geotop::input::OutputFile* f, AllData* A)
{
    GeoMatrix<double>* M = getSupervectorVariableM(A, f->getVariable());
    GeoMatrix<double>* N = f->values.getValuesM();

    assert(M != NULL && N != NULL);
    
    for (size_t l = 0, sr = N->getRows(); l < sr; l++)
    {
        for (size_t i = 0, sc = N->getCols(); i < sc; i++)
            (*N)[l][i] = (*M)[l][i];
    }
}

static double getPointValue(AllData* A, geotop::input::Variable what, long layer, long row, long col)
{
    long i;
    bool found = false;
    double output = geotop::input::gDoubleNoValue;

    //Search for the requested point
    for (i = 1; i <= A->P->total_pixel; i++)
    {
        if ((row == A->T->rc_cont[i][1])  && (col == A->T->rc_cont[i][2]))
        {
            found = true;
            break;
        }
    }

    if (found)
    {

        GeoMatrix<double>* var = getSupervectorVariableM(A, what);

        if (var != NULL)
        {
            output = (*var)[layer][i];
        }
        else
        {
            GeoVector<double>* var = getSupervectorVariableV(A, what);

            if (var != NULL)
            {
                output = (*var)[i];
            }
        }

    }

    return output;

}

#ifndef METEOIO_OUTPUT
static void printLayer(std::string filename, GeoVector<double>* V, AllData* A)
{
    long r,c;
    FILE* fp = NULL;
    GeoMatrix<double> M(geotop::common::Variables::Nr+1,
                        geotop::common::Variables::Nc+1,
                        geotop::input::gDoubleNoValue);

    //Cycle through the supervector and populate the matrix
    for (size_t i = 1; i < V->size(); i++)
    {
        r = A->T->rc_cont[i][1];
        c = A->T->rc_cont[i][2];

        M[r][c] = V->at(i);
    }

    fp = fopen (filename.c_str(), "w");

    if (fp == NULL)
    {
        geotop::logger::GlobalLogger* lg =
            geotop::logger::GlobalLogger::getInstance();
        lg->logsf(geotop::logger::CRITICAL,
                  "Unable to open file %s for writing. Aborting.",
                  filename.c_str());
        exit(1);
    }

    //Header
    fprintf(fp, "ncols         %u\n", M.getCols()-1);
    fprintf(fp, "nrows         %u\n", M.getRows()-1);
    fprintf(fp, "xllcorner     %f\n", geotop::common::Variables::UV->U[4]);
    fprintf(fp, "yllcorner     %f\n", geotop::common::Variables::UV->U[3]);
    fprintf(fp, "cellsize      %f\n", geotop::common::Variables::UV->U[1]);
    fprintf(fp, "NODATA_value  %.3f\n", geotop::input::gDoubleNoValue);

    //Data
    for (r = 1; r < M.getRows(); r++)
    {
        for (c = 1; c < M.getCols(); c++)
            fprintf(fp, "%.3f ", M[r][c]);

        fseek(fp, -1, SEEK_CUR);
        fprintf(fp,"\n");
    }

    fclose(fp);
}
#else
static void printLayer(std::string filename, GeoVector<double>* V, AllData* A)
{

    assert(geotop::common::Variables::Nc > 0);
    size_t ncols = geotop::common::Variables::Nc;

    assert(geotop::common::Variables::Nr > 0);
    size_t nrows = geotop::common::Variables::Nr;

    //The output matrix
    GeoMatrix<double> M(ncols, nrows, mio::IOUtils::nodata);

    //HACK: GEOTop most of the times uses 1..n indexes instead of 0..n-1
    //so we cut out one element even if this can lead to data loss
    for (size_t i = 1, s = V->size(); i < s; i++)
    {
        long r = A->T->rc_cont[i][1];
        long c = A->T->rc_cont[i][2];

        M[c - 1][nrows - r] = V->at(i);
    }

    mio::IOManager iomanager(cfg);

    //Required by write2DGrid
    mio::Grid2DObject g2d;

    //Setting the coordinates
	g2d.llcorner.setXY(geotop::common::Variables::UV->U[4],geotop::common::Variables::UV->U[3], 0);

    //Setting the grid (rows, columns, cellsize, lower left corner coordinates and grid data)
    g2d.set(M.getRows(), M.getCols(), geotop::common::Variables::UV->U[1], g2d.llcorner, M);

    iomanager.write2DGrid(g2d, filename);
}
#endif

static void printTableHeader(std::string filename, size_t numlayers, bool printunit = false, std::string unit = "")
{
    FILE* fp;

    geotop::logger::GlobalLogger* lg =
        geotop::logger::GlobalLogger::getInstance();

    switch (gt_fileExists(filename.c_str()))
    {
        case 0:
            //File doesn't exists
            break;
        case 1:
            //File exists
            lg->logsf(geotop::logger::WARNING,
                      "File '%s' already exists and it will be overwritten.",
                      filename.c_str());
            break;
        case 2:
            //File is a directory
            lg->logsf(geotop::logger::CRITICAL,
                      "Unable to open file '%s' for writing: the file is a directory. Aborting.",
                      filename.c_str());
            exit(1);
            break;
        case 3:
            //File is a special file
            lg->logsf(geotop::logger::CRITICAL,
                      "Unable to open file '%s' for writing: wrong type. Aborting.",
                      filename.c_str());
            exit(1);
            break;
        default:
            //Unspecified error
            lg->logsf(geotop::logger::CRITICAL,
                      "Unable to open file '%s' for writing: an error occurred while trying to find file's type. Aborting.",
                      filename.c_str());
            exit(1);
            break;
    }

    //Create the file
    fp = fopen (filename.c_str(), "w");

    if (fp == NULL)
    {
        lg->logsf(geotop::logger::CRITICAL,
                  "Unable to open file '%s' for writing. Aborting.",
                  filename.c_str());
        exit(1);
    }

    //Write the header
    fprintf(fp, "\"Time[s]\",");

    for (size_t i = 1; i <= numlayers; i++)
    {
        if (printunit)
        {
            fprintf(fp, "\"Layer %zu [%s]\",", i, unit.c_str());
        }
        else
        {
            fprintf(fp, "\"Layer %zu\",", i);
        }
    }

    //Erase last ',' and place a newline instead
    fseek(fp, -1, SEEK_CUR);
    fprintf(fp,"\n");

    fclose(fp);
}

static void printTableRow(std::string filename, GeoVector<double>* row, size_t seconds_from_start, bool skipfirst = true)
{
    FILE* fp;

    size_t i = 0, last = row->size();   

    geotop::logger::GlobalLogger* lg =
        geotop::logger::GlobalLogger::getInstance();

    switch (gt_fileExists(filename.c_str()))
    {
        case 0:
            //filename doesn't exists
            lg->logsf(geotop::logger::CRITICAL,
                      "'%s' doesn't exists. Aborting.",
                      filename.c_str());
            exit(1);
            break;
        case 1:
            //filename exists and it's a regular file: proceed
            break;
        case 2:
            //filename points to a directory
            lg->logsf(geotop::logger::CRITICAL,
                      "'%s' is a directory. Aborting.",
                      filename.c_str());
            exit(1);
            break;
        case 3:
            //filename points to a special file (device)
            lg->logsf(geotop::logger::CRITICAL,
                      "'%s' is not valid. Aborting",
                      filename.c_str());
            exit(1);
            break;
        default:
            //An error occurred
            lg->logsf(geotop::logger::CRITICAL,
                      "Unable to retrieve informations about '%s'. Aborting.",
                      filename.c_str());
            exit(1);
            break;
    }

    fp = fopen(filename.c_str(), "a");

    if (fp == NULL)
    {
        lg->logsf(geotop::logger::CRITICAL,
                  "Unable to open file '%s' for writing. Aborting.",
                  filename.c_str());
        exit(1);
    }

    //Print time field
    fprintf(fp, "%zu,", seconds_from_start);

    //Scan row elements
    if (last > 0)
    {
        if (last > 1)
        {
            //Set starting index
            i = skipfirst ? 1 : 0;

            while(i < last - 1)
            {
                fprintf(fp, "%f,", row->at(i));
                i++;
            }
        }

        fprintf(fp, "%f\n", row->at(i));

    }

    fclose(fp);

}

static GeoVector<double> getCurrentSpatialMeans(geotop::input::OutputFile* f, AllData* A)
{
    GeoVector<double> output;

    //Fetch TemporaryValues Vector
    GeoVector<double>* TV = f->values.getValuesV();
    size_t layers = TV->size();

    output.resize(layers);

    //For all layers
    for (size_t i = 1; i <= layers; i++)
    {
        double mean = 0.;

        //Get layer's data
        GeoVector<double>* V = getSupervectorLayer(i, f, A);
        size_t s = V->size();

        //Compute mean of all layer's points
        for (size_t j = 1; j < s; j++)
            mean += V->at(j);

        mean /= (s - 1);

        //Store the data in the temporary values array
        output[i - 1] = mean;

        //Delete temporary GeoVector
        delete V;
    }

    return output;
}

/**
 * @brief Prints a variable to the corrisponding file(s)
 * @param[in] A global data storage pointer
 * @param[in] f output file definition
 */
static void printInstant(AllData* A, geotop::input::OutputFile* f)
{
    if (f->getVariable() != geotop::input::UNKNOWN_VAR)
    {
        double lJDate = A->I->time; //seconds passed since the beginning of the simulation

        lJDate /= GTConst::secinday; //seconds to days
        lJDate += A->P->init_date;
        lJDate = convert_JDfrom0_dateeur12(lJDate);

        switch(f->getDimension())
        {
            case geotop::input::D1Dp:
                break;
            case geotop::input::D1Ds:
                {
                    GeoVector<double> TV = getCurrentSpatialMeans(f, A);

                    //Print a new row in the table
                    printTableRow(f->getFilePath(), &TV, (size_t)A->I->time, false);
                }
                break;
            case geotop::input::D2D:
                {
                    long l = f->getLayer();

                    if (l >= 0)
                    {
                        refreshTemporaryValuesV(f, A, l);
                        std::string filename = f->getFilePath(lJDate, l);
                        printLayer(filename, f->values.getValuesV(), A);
                    }
                    else
                    {
                        //Invalid layer number
                        geotop::logger::GlobalLogger* lg =
                            geotop::logger::GlobalLogger::getInstance();

                        lg->logsf(geotop::logger::ERROR,
                                  "Invalid layer number (%ld) for line:\n%s",
                                  l, (f->toString()).c_str());
                    }
                }
                break;
            case geotop::input::D3D:
                {
                    refreshTemporaryValuesM(f, A);
                    
                    //For each layer
                    for (long l = 1; l <= geotop::common::Variables::Nl; l++)
                    {
                        std::string filename = f->getFilePath(lJDate, l);

                        //Extract layer's supervector
                        GeoVector<double>* V = extractGeoVector(f->values.getValuesM(), l);

                        printLayer(filename, V, A);

                        delete V; //Deallocate temporary vector V
                    }
                }
                break;
            default:
                {
                    geotop::logger::GlobalLogger* lg =
                        geotop::logger::GlobalLogger::getInstance();

                    lg->log("Unable to find the output dimension. Check your geotop.inpts file.",
                            geotop::logger::WARNING);
                }
                break;
        }
    }
}

/**
 * @brief Adds current values to temporary values
 * @param[in] A GEOTop data structure
 * @param[in] of OutputFile to refresh
 */
static void refreshCumulates(AllData* A, geotop::input::OutputFile* of)
{
   
    switch(of->getDimension())
    {
        case geotop::input::D1Dp:
            break;
        case geotop::input::D1Ds:
            break;
        case geotop::input::D2D:
            {
                //Get temporary value vector
                GeoVector<double>* TV = of->values.getValuesV();
                //Get instant supervector copy
                GeoVector<double>* IV = getSupervectorLayer(of->getLayer(), of, A);

                assert(IV != NULL);

                //Add supervector to temporary values
                for (size_t i = 0, s = TV->size(); i < s; i++)
                    (*TV)[i] = TV->at(i) + IV->at(i);

                delete IV; //deallocate instant supervector
            }
            break;
        case geotop::input::D3D:
            {
                //Get temporary value matrix
                GeoMatrix<double>* TM = of->values.getValuesM();
                //Get instant supervector matrix
                GeoMatrix<double>* M = getSupervectorVariableM(A, of->getVariable());

                assert(M != NULL);

                //For each layer add supervector to temp
                for (size_t l = 0, nl = TM->getRows(); l < nl; l++)
                {
                    for (size_t i = 0, s = TM->getCols(); i < s; i++)
                        (*TM)[l][i] = (*TM)[l][i] + (*M)[l][i];
                }
            }
            break;
        default:
            {
                geotop::logger::GlobalLogger* lg =
                    geotop::logger::GlobalLogger::getInstance();

                lg->log("Unable to find the output dimension. Check your geotop.inpts file.",
                        geotop::logger::WARNING);
            }
            break;
    }

}

static void printCumulates(AllData* A, geotop::input::OutputFile* of)
{
    double lJDate = A->I->time; //seconds passed since the beginning of the simulation

    lJDate /= GTConst::secinday; //seconds to days
    lJDate += A->P->init_date;
    lJDate = convert_JDfrom0_dateeur12(lJDate);

    std::string filename;
   
    switch(of->getDimension())
    {
        case geotop::input::D1Dp:
            break;
        case geotop::input::D1Ds:
            break;
        case geotop::input::D2D:
            {
                long l = of->getLayer();

                if (l >= 0)
                {
                    GeoVector<double>* V = of->values.getValuesV(); 
                    std::string filename = of->getFilePath(lJDate, l);
                    printLayer(filename, V, A);
                }
                else
                {
                    //Invalid layer number
                    geotop::logger::GlobalLogger* lg =
                        geotop::logger::GlobalLogger::getInstance();

                    lg->logsf(geotop::logger::ERROR,
                              "Invalid layer number (%ld) for line:\n%s",
                              l, (of->toString()).c_str());
                }
            }
            break;
        case geotop::input::D3D:
            {
                //For each layer
                for (long l = 1; l <= geotop::common::Variables::Nl; l++)
                {
                    std::string filename = of->getFilePath(lJDate, l);

                    //Extract layer's supervector
                    GeoVector<double>* V = extractGeoVector(of->values.getValuesM(), l);

                    printLayer(filename, V, A);

                    delete V; //Deallocate temporary vector V
                }
            }
            break;
        default:
            {
                geotop::logger::GlobalLogger* lg =
                    geotop::logger::GlobalLogger::getInstance();

                lg->log("Unable to find the output dimension. Check your geotop.inpts file.",
                        geotop::logger::WARNING);
            }
            break;
    }

}

static void printAverages(AllData* A, geotop::input::OutputFile* of)
{
    double lJDate = A->I->time; //seconds passed since the beginning of the simulation

    lJDate /= GTConst::secinday; //seconds to days
    lJDate += A->P->init_date;
    lJDate = convert_JDfrom0_dateeur12(lJDate);

    std::string filename;
   
    switch(of->getDimension())
    {
        case geotop::input::D1Dp:
            break;
        case geotop::input::D1Ds:
            break;
        case geotop::input::D2D:
            {
                long l = of->getLayer();

                if (l >= 0)
                {
                    //Get cumulates vector
                    GeoVector<double>* V = of->values.getValuesV();

                    //Get number of additions
                    size_t count = of->values.getCount();
                    
                    //Divide each member of cumulates vector by count to get the mean value
                    for (size_t i = 0, s = V->size(); i < s; i++)
                    {
                        (*V)[i] = V->at(i) / (double)count;
                    }

                    std::string filename = of->getFilePath(lJDate, l);
                    printLayer(filename, of->values.getValuesV(), A);
                }
                else
                {
                    //Invalid layer number
                    geotop::logger::GlobalLogger* lg =
                        geotop::logger::GlobalLogger::getInstance();

                    lg->logsf(geotop::logger::ERROR,
                              "Invalid layer number (%ld) for line:\n%s",
                              l, (of->toString()).c_str());
                }
            }
            break;
        case geotop::input::D3D:
            {
            }
            break;
        default:
            {
                geotop::logger::GlobalLogger* lg =
                    geotop::logger::GlobalLogger::getInstance();

                lg->log("Unable to find the output dimension. Check your geotop.inpts file.",
                        geotop::logger::WARNING);
            }
            break;
    }

}

/*==============================================================================
   Public functions
 ==============================================================================*/

#ifdef METEOIO_OUTPUT
void output_file_preproc(AllData* A, mio::Config& mioConfig)
#else
void output_file_preproc(AllData* A)
#endif
{
    boost::shared_ptr< std::vector<geotop::input::OutputFile> > output_files;
    std::vector<OutputFilesVector*>* lInstants = new std::vector<OutputFilesVector*>();
    std::vector<OutputFilesVector*>* lCumulates = new std::vector<OutputFilesVector*>();
    std::vector<OutputFilesVector*>* lAverages = new std::vector<OutputFilesVector*>();
    long lPeriod, lTmpLong;
    size_t i = 0;
    size_t j = 0;
    size_t sz;

#ifdef METEOIO_OUTPUT
    cfg = mioConfig;
#endif

    //Get Global Logger instance
    geotop::logger::GlobalLogger* lg = geotop::logger::GlobalLogger::getInstance();

    //Get all output files from geotop.inpts OUTPUT_SEC
    boost::shared_ptr<geotop::input::ConfigStore> lConfigStore =
        geotop::input::ConfigStoreSingletonFactory::getInstance();

    if (lConfigStore->case_sensitive_get("OUTPUT_SEC", output_files) == false)
        return; //No Output section defined

    sz = output_files->size();

    if (sz == 0)
        return; //No output files defined

    //Sort them by period from shorter to longer
    std::sort(output_files->begin(), output_files->end(), compareByPeriod);

    //Initial period
    lPeriod = (output_files->at(0)).getPeriod();

    //Check if initial period is less than TimeStep
    if (lPeriod < A->P->Dt)
    {
        lg->logsf(geotop::logger::CRITICAL,
                  "The minimum integration period (%d) is shorter than the time step (%.0f).\nPlease increase the integration period at least to %.0f or more in your geotop.inpts file in the line:\n%s",
                  lPeriod, A->P->Dt, A->P->Dt, ((output_files->at(0)).toString()).c_str());
        exit(1);
    }

    lInstants->push_back(new OutputFilesVector(lPeriod));
    lCumulates->push_back(new OutputFilesVector(lPeriod));
    lAverages->push_back(new OutputFilesVector(lPeriod));

    //For each output file defined in geotop.inpts
    while (i < sz)
    {
        geotop::input::OutputFile of = output_files->at(i);
        lTmpLong = of.getPeriod();
        std::string prefix = of.getPrefix();

        //If the user set a prefix directory for OutputFile of
        if (prefix != "")
        {
            //Test if prefix directory exists
            switch (gt_fileExists(prefix.c_str()))
            {
                case 0:
                    //Prefix directory doesn't exist
                    //Attempt to create it
                    if (gt_makeDirectory(prefix.c_str()) == 0)
                    {
                        //An error occurred
                        lg->logsf(geotop::logger::CRITICAL,
                                  "Unable to create directory '%s'. Aborting.",
                                  prefix.c_str());
                        exit(1);
                    }
                    break;
                case 2:
                    //Prefix directory exists
                    break;
                case 1:
                    //Pass through
                case 3:
                    //Prefix exists but it's a file
                    lg->logsf(geotop::logger::CRITICAL,
                              "File '%s' already exists and it's not a directory. Aborting",
                              prefix.c_str());
                    exit(1);
                    break;
                default:
                    //An error occurred
                    lg->logsf(geotop::logger::CRITICAL,
                              "An unknown error occurred while trying to access to '%s'. Aborting.",
                              prefix.c_str());
                    exit(1);
                    break;
            }
        }

        if (lTmpLong == lPeriod)
        {
            //Ignore unknown variables
            if (of.getVariable() != geotop::input::UNKNOWN_VAR)
            {
                initTemporaryValues(of, A);

                switch(of.getIntegrationType())
                {
                    case geotop::input::INS:
                        (lInstants->at(j))->append(of);
                        break;
                    case geotop::input::CUM:
                        (lCumulates->at(j))->append(of);
                        break;
                    case geotop::input::AVG:
                        (lAverages->at(j))->append(of);
                        break;
                    default:
                        break;
                }
            }

            if (of.getDimension() == geotop::input::D1Ds)
            {
                //TODO: handle different layer numbers for non-soil tensors
                printTableHeader(of.getFilePath(), (size_t)geotop::common::Variables::Nl);
            }

            i++;
        }
        else
        {
            lPeriod = lTmpLong;

            lInstants->push_back(new OutputFilesVector(lPeriod));
            lCumulates->push_back(new OutputFilesVector(lPeriod));
            lAverages->push_back(new OutputFilesVector(lPeriod));

            j++;
        }
    }

    //Pruning
    
    bool doneI = false;
    bool doneC = false;
    bool doneA = false;

    std::vector<OutputFilesVector*>::iterator itI = lInstants->end();
    std::vector<OutputFilesVector*>::iterator itC = lCumulates->end();
    std::vector<OutputFilesVector*>::iterator itA = lAverages->end();

    while (!doneI || !doneC || !doneA)
    {
        if (lInstants->empty())
            doneI = true;
        if (lCumulates->empty())
            doneC = true;
        if (lAverages->empty())
            doneA = true;

        if (!doneI)
        {
            if (itI != lInstants->begin())
                --itI;

            if ((*itI)->empty() == true)
            {
                itI = lInstants->erase(itI);
            }
            else if (itI == lInstants->begin())
                doneI = true;
        }

        if (!doneC)
        {
            if (itC != lCumulates->begin())
                --itC;

            if ((*itC)->empty() == true)
            {
                itC = lCumulates->erase(itC);
            }
            else if (itC == lCumulates->begin())
                doneC = true;
        }

        if (!doneA)
        {
            if (itA != lAverages->begin())
                --itA;

            if ((*itA)->empty() == true)
            {
                itA = lAverages->erase(itA);
            }
            else if (itA == lAverages->begin())
                doneA = true;
        }
    }

    if (lInstants->empty() == false)
        ofs.instants = lInstants;
    if (lCumulates->empty() == false)
        ofs.cumulates = lCumulates;
    if (lAverages->empty() == false)
        ofs.averages = lAverages;
    
}

void write_output_new(AllData* A)
{
    double lJDate = A->I->time; //seconds passed since the beginning of the simulation
    OutputFilesVector* ofv = NULL;
    size_t i, j;

    lJDate /= GTConst::secinday;
    lJDate += A->P->init_date;

    if (ofs.instants != NULL)
    {
        for (i = 0; i < ofs.instants->size(); i++)
        {
            ofv = ofs.instants->at(i);

            if (fabs(fmod(A->I->time, ofv->period())) < epsilon)
            {
                for (j = 0; j < ofv->size(); j++)
                {
                    geotop::input::OutputFile f = ofv->at(j);
                    printInstant(A, &f);
                }
            }
        }
    }

    if (ofs.cumulates != NULL)
    {
        for (i = 0; i < ofs.cumulates->size(); i++)
        {
            ofv = ofs.cumulates->at(i);

            for (j = 0; j < ofv->size(); j++)
            {
                geotop::input::OutputFile f = ofv->at(j);
                refreshCumulates(A, &f);
                if (fabs(fmod(A->I->time, ofv->period())) < epsilon)
                {
                    printCumulates(A, &f);
                    zeroFill(&f);
                }
            }
        }
    }

    if (ofs.averages != NULL)
    {
        for (i = 0; i < ofs.averages->size(); i++)
        {
            ofv = ofs.averages->at(i);

            for (j = 0; j < ofv->size(); j++)
            {
                geotop::input::OutputFile f = ofv->at(j);
                refreshCumulates(A, &f);
                f.values.incCount();
                if (fabs(fmod(A->I->time, ofv->period())) < epsilon)
                {
                    printAverages(A, &f);
                    zeroFill(&f);
                }
            }
        }
    }

}

static void deallocate_helper(geotop::input::OutputFile* of)
{
    int wiv = of->values.whatIsValid();
    switch (wiv)
    {
        case 0:
            break;
        case 1:
            {
                GeoVector<double>* V = of->values.getValuesV();
                delete V;
            }
            break;
        case 2:
            {
                GeoMatrix<double>* M = of->values.getValuesM();
                delete M;
            }
            break;
        case 3:
            {
                GeoTensor<double>* T = of->values.getValuesT();
                delete T;
            }
            break;
        default:
            //This code shouldn't be reached, if it is then VERY BAD things happened
            geotop::logger::GlobalLogger* lg =
                geotop::logger::GlobalLogger::getInstance();
            lg->logsf(geotop::logger::CRITICAL,
                      "Invalid value for TemporaryValue.whatIsValid: %d. Aborting.",
                      wiv);
            exit(666);
            break;
    }
}

void deallocate_output_new()
{
    size_t i,j;
    OutputFilesVector* ofv = NULL;

    if (ofs.instants != NULL)
    {
        for (i = 0; i < ofs.instants->size(); i++)
        {
            ofv = ofs.instants->at(i);

            for (j = 0; j < ofv->size(); j++)
            {
                geotop::input::OutputFile of = ofv->at(j);
                deallocate_helper(&of);
            }
        }
    }

    if (ofs.cumulates != NULL)
    {
        for (i = 0; i < ofs.cumulates->size(); i++)
        {
            ofv = ofs.cumulates->at(i);

            for (j = 0; j < ofv->size(); j++)
            {
                geotop::input::OutputFile of = ofv->at(j);
                deallocate_helper(&of);
            }
        }
    }

    if (ofs.averages != NULL)
    {
        for (i = 0; i < ofs.averages->size(); i++)
        {
            ofv = ofs.averages->at(i);

            for (j = 0; j < ofv->size(); j++)
            {
                geotop::input::OutputFile of = ofv->at(j);
                deallocate_helper(&of);
            }
        }
    }
}

//Implementation moved here because this function will grow considerably
static GeoMatrix<double>* getSupervectorVariableM(AllData* A, geotop::input::Variable what)
{
    GeoMatrix<double>* var = NULL;

    switch(what)
    {
        case geotop::input::SOIL_TEMP:
            var = &(A->S->SS->T);
            break;
        case geotop::input::SOIL_WATER_CONTENT:
            var = &(A->S->th);
            break;
        default:
            break;
    }

    if (var != NULL && var->getCols() < A->P->total_pixel)
        return NULL;

    return var;

}

static GeoVector<double>* getSupervectorVariableV(AllData* A, geotop::input::Variable what)
{
    GeoVector<double>* var = NULL;

    switch(what)
    {
        default:
            break;
    }

    if (var != NULL && var->size() < (size_t)A->P->total_pixel)
        return NULL;

    return var;

}

