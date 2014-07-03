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

/**
 *@file output_new.cc
 *@author Gianfranco Gallizia
 *@date 07/2014
 *@brief GEOtop output post-processing routines
 */

#include "output_new.h"
#include "inputKeywords.h"
#include "output_file.h"
#include <algorithm>
#include <math.h>

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
        std::vector<OutputFilesVector>* instants;
        std::vector<OutputFilesVector>* cumulates;
        std::vector<OutputFilesVector>* averages;
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

static OutputFilesStore ofs;
static bool compareByPeriod(geotop::input::OutputFile a, geotop::input::OutputFile b)
{
    return (a.getPeriod() < b.getPeriod());
}

void output_file_preproc()
{
    boost::shared_ptr< std::vector<geotop::input::OutputFile> > output_files;
    geotop::input::OutputFile of;
    std::vector<OutputFilesVector>* lInstants = new std::vector<OutputFilesVector>();
    std::vector<OutputFilesVector>* lCumulates = new std::vector<OutputFilesVector>();
    std::vector<OutputFilesVector>* lAverages = new std::vector<OutputFilesVector>();
    double lPeriod, lTmpDouble;
    size_t i = 0;
    size_t j = 0;
    size_t sz;

    lInstants->push_back(OutputFilesVector(lPeriod));
    lCumulates->push_back(OutputFilesVector(lPeriod));
    lAverages->push_back(OutputFilesVector(lPeriod));

    //Get all output files from geotop.inpts OUTPUT_SEC
    boost::shared_ptr<geotop::input::ConfigStore> lConfigStore =
        geotop::input::ConfigStoreSingletonFactory::getInstance();

    if (lConfigStore->case_sensitive_get("OUTPUT_SEC", output_files) == false)
        return; //TODO: handle errors

    sz = output_files->size();

    if (sz == 0) //No output files defined
        return;

    //Sort them by period from shorter to longer
    std::sort(output_files->begin(), output_files->end(), compareByPeriod);

    //Initial period
    lPeriod = (output_files->at(0)).getPeriod();

    lTmpVector = new std::vector<geotop::input::OutputFile>();

    //For each output file defined in geotop.inpts
    while (i < sz)
    {
        of = output_files->at(i);
        lTmpDouble = of.getPeriod();

        if (fabs(lTmpDouble - lPeriod) < 1.E-6) //lTmpDouble == lPeriod
        {
            switch(of.getIntegrationType())
            {
                case INS:
                    (lInstants->at(j)).append(of);
                    break;
                case CUM:
                    (lCumulates->at(j)).append(of);
                    break;
                case AVG:
                    (lAverages->at(j)).append(of);
                    break;
            }

            i++;
        }
        else
        {
            lPeriod = lTmpDouble;

            lInstants->push_back(OutputFilesVector(lPeriod));
            lCumulates->push_back(OutputFilesVector(lPeriod));
            lAverages->push_back(OutputFilesVector(lPeriod));

            j++;
        }
    }

    //TODO: prune the vectors

    ofs.instants = lInstants;
    ofs.cumulates = lCumulates;
    ofs.averages = lAverages;
}

void write_output_new(AllData* A)
{
    double time_from_start = A->I->time;
}

