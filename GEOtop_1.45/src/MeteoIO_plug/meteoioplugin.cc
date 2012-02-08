#ifdef USE_METEOIO
#include "meteoioplugin.h"
#include "../geotop/times.h"
#include <sstream>
#include <stdlib.h>

using namespace std;
using namespace mio;
extern long i_sim;
const double TZ = 1.;
extern long number_novalue, number_absent;
DEMObject dem;
Config cfg("/Users/noori/Documents/workspace/GEOtopCPP/src/MeteoIO_plug/io_it.ini");

extern "C" DOUBLEMATRIX *meteoio_readDEM(T_INIT** UVREF) {
	DOUBLEMATRIX *myDEM = NULL;

	T_INIT* UV = (T_INIT *) malloc(sizeof(T_INIT));

	try {

		//	DEMObject dem;
		IOHandler iohandler(cfg);
		iohandler.readDEM(dem);

		myDEM = new_doublematrix(dem.nrows, dem.ncols);
		UV->V = new_doublevector(2);
		UV->U = new_doublevector(4);

		UV->V->co[1] = -1.0;
		UV->V->co[2] = number_novalue;//GEOtop nodata -9999.0

		UV->U->co[1] = dem.cellsize;
		UV->U->co[2] = dem.cellsize;
		UV->U->co[3] = dem.llcorner.getNorthing();
		UV->U->co[4] = dem.llcorner.getEasting();

		for (unsigned int ii = 0; ii < dem.nrows; ii++) {
			for (unsigned int jj = 0; jj < dem.ncols; jj++) {
				if (dem.grid2D(jj, dem.nrows - 1 - ii) == IOUtils::nodata) {
					myDEM->co[ii + 1][jj + 1] = UV->V->co[2];
				} else {
					myDEM->co[ii + 1][jj + 1] = dem.grid2D(jj, dem.nrows - 1
							- ii);
				}
			}
		}
		std::cout << "Read DEM:" << dem.nrows << "(rows) " << dem.ncols
				<< "(cols)" << std::endl;
		free((*UVREF));
		(*UVREF) = UV;
	} catch (std::exception& e) {
		std::cerr << "[E] MeteoIO: " << e.what() << std::endl;
	}

	return myDEM;
}

extern "C" DOUBLEMATRIX *meteoio_read2DGrid(T_INIT* UV, char* _filename) {
	DOUBLEMATRIX *myGrid = NULL;

	try {

		Grid2DObject gridObject;
		Config cfg("io.ini");
		IOHandler iohandler(cfg);

		std::string gridsrc = "", filename = "";
		cfg.getValue("GRID2D", "INPUT", gridsrc);

		if (gridsrc == "GRASS")
			filename = string(_filename) + ".grassasc";
		else if (gridsrc == "ARC")
			filename = string(_filename) + ".asc";

		iohandler.read2DGrid(gridObject, filename);

		if (UV->U->co[1] != gridObject.cellsize)
			throw IOException("Inconsistencies between 2D Grids read", AT);
		else if (UV->U->co[2] != gridObject.cellsize)
			throw IOException("Inconsistencies between 2D Grids read", AT);
		else if (UV->U->co[3] != gridObject.llcorner.getNorthing())
			throw IOException("Inconsistencies between 2D Grids read", AT);
		else if (UV->U->co[4] != gridObject.llcorner.getEasting())
			throw IOException("Inconsistencies between 2D Grids read", AT);

		for (unsigned int ii = 0; ii < gridObject.nrows; ii++) {
			for (unsigned int jj = 0; jj < gridObject.ncols; jj++) {
				if (gridObject.grid2D(jj, gridObject.nrows - 1 - ii)
						== IOUtils::nodata) {
					myGrid->co[ii + 1][jj + 1] = UV->V->co[2];
				} else {
					myGrid->co[ii + 1][jj + 1] = gridObject.grid2D(jj,
							gridObject.nrows - 1 - ii);
				}
			}
		}
		std::cout << "Read 2D Grid from file '" << filename << "' : "
				<< gridObject.nrows << "(rows) " << gridObject.ncols
				<< "(cols)" << std::endl;
	} catch (std::exception& e) {
		std::cerr << "[E] MeteoIO: " << e.what() << std::endl;
	}

	return myGrid;
}

extern "C" void meteoio_interpolate(T_INIT* UV, PAR* par, double currentdate,
		METEO* met, WATER* wat, long k) {
	/* 
	 This function performs the 2D interpolations by calling the respective methods within MeteoIO
	 1) Data is copied from GEOtop structs to MeteoIO objects
	 2) DEM is reread
	 3) interpolation takes place
	 4) gridded data copied back to GEOtop DOUBLEMATRIX
	 5) TA and RH values need to be converted as well as nodata values
	 */

	DEMObject dem;
	Grid2DObject tagrid, rhgrid, pgrid, vwgrid, dwgrid, hnwgrid;
	std::string coordsys = "", coordparam = "";

	double novalue = UV->V->co[2];

	Date d1;
	d1.setMatlabDate(currentdate, TZ); // GEOtop use matlab offset of julian date

	std::cout << "[MeteoIO] Time to interpolate: " << std::endl;

	std::vector<StationData> vecStation;

	try {
		cfg.getValue("COORDSYS", "Input", coordsys);
		cfg.getValue("COORDPARAM", "Input", coordparam, Config::nothrow);

		Coords coord(coordsys, coordparam);

		/*
		 for(unsigned int ii=1; ii<=met->st->Z->nh; ii++){//Build up MeteoData and StationData objects

		 //conversion of values required
		 //P=met->var[i-1][met->column[i-1][iPs]];
		 double p = met->var[ii-1][met->column[ii-1][iPs]];
		 if (p == novalue) p = IOUtils::nodata;

		 double rh = met->var[ii-1][met->column[ii-1][iRh]];
		 if (rh == novalue) {
		 rh = IOUtils::nodata;
		 } else {
		 rh = rh / 100.0; //MeteoIO needs values in [0;1]
		 }

		 double ta = met->var[ii-1][met->column[ii-1][iT]];
		 if (ta == novalue) {
		 ta = IOUtils::nodata;
		 } else {
		 ta = ta + 273.15; //MeteoIO deals with temperature in Kelvin;
		 }

		 double vw = met->var[ii-1][met->column[ii-1][iWs]];
		 if (vw == novalue) vw = IOUtils::nodata;

		 double vd = met->var[ii-1][met->column[ii-1][iWd]];
		 if (vd == novalue) vd = IOUtils::nodata;

		 double easting = met->st->E->co[ii];
		 double northing = met->st->N->co[ii];
		 double altitude = met->st->Z->co[ii];
		 coord.setXY(easting, northing, altitude);
		 StationData sd(coord);
		 vecStation.push_back(sd);

		 MeteoData md(currentdate);
		 md.ta = ta;
		 md.vw = vw;
		 md.dw = vd;
		 md.rh = rh;
		 md.p = p;

		 std::cout << "[MeteoIO] E: " << easting << std::endl;
		 std::cout << "[MeteoIO] N: " << northing << std::endl;
		 std::cout << "[MeteoIO] Altitude: " << altitude << std::endl;

		 std::cout << "[MeteoIO] P: " << p << std::endl;
		 std::cout << "[MeteoIO] RH: " << rh << std::endl;
		 std::cout << "[MeteoIO] TA: " << ta << std::endl;
		 std::cout << "[MeteoIO] VW: " << vw << std::endl;
		 std::cout << "[MeteoIO] VD: " << vd << std::endl;
		 std::cout << md.toString() << endl;

		 }
		 */
		//alternative: let MeteoIO do all the work

		double lrta = -LapseRateTair * 1E-3; // GEOtop default lapse rate of air temp.
		double lspr = -LapseRatePrec * 1E-3; // GEOtop default lapse rate of precipitation

		if (met->LRv[ilsTa] != LapseRateTair) {
			// The user has given a lapse rate: use this
			lrta = -met->LRv[ilsTa] * 1E-3;
			stringstream lapserateTA;	// Create a stringstream
			lapserateTA << lrta;		// Add number to the stream
			string s = " soft";
			cfg.addKey("TA::idw_lapse", "Interpolations2D", lapserateTA.str()
					+ s);
		}

		if (met->LRv[ilsPrec] != LapseRatePrec) {
			// The user has given a lapse rate: use this
			lspr = -met->LRv[ilsPrec] * 1E-3;
			stringstream lapserateHNW;	// Create a stringstream
			lapserateHNW << lspr;		// Add number to the stream
			string s = " soft";
			cfg.addKey("HNW::idw_lapse", "Interpolations2D", lapserateHNW.str()
					+ s);
		}

		IOManager io(cfg);

		//read DEM
		io.readDEM(dem);

		std::vector<std::vector<MeteoData> > vecMeteos;
		std::vector<MeteoData> meteo; 					// Intermediate storage for storing data sets for 1 timestep
		int numOfStations = io.getMeteoData(d1, meteo); // Read the meteo data for the given timestep
		vecMeteos.insert(vecMeteos.begin(), meteo.size(), std::vector<MeteoData>()); // Allocation for the vectors
		double rain, snow, hnw,ta;

		std::cout << " raincorrfact "<<par->raincorrfact <<" rain "<<rain <<" snowcorrfact "<< par->snowcorrfact <<" snow "<< snow << std::endl;
		for(int i=0;i<numOfStations;i++){
           	hnw = meteo[i](MeteoData::HNW);
           	ta = meteo[i](MeteoData::TA)- 273.15; 							// MeteoIO deals with temperature in Kelvin;
        	if (hnw > 0 && hnw!= IOUtils::nodata && ta!=IOUtils::nodata ) { // check for non-zero precipitation
        		part_snow(hnw, &rain, &snow, ta, par->T_rain, par->T_snow); // perform prepossessing on HNW
        		std::cout <<i<< " Mhnw " << hnw <<" MTa "<<ta<<" IR "<<par->raincorrfact * rain + par->snowcorrfact * snow << std::endl;
        		meteo[i](MeteoData::HNW)= par->raincorrfact * rain + par->snowcorrfact * snow; // rain correction factor and snow correction factor
        		vecMeteos.at(i).push_back(meteo[i]); // fill the data into the vector of vectors
           		}
        	}

		// Bypass the internal reading of MeteoData and to performed processing and interpolation on the data of the given vector
		io.push_meteo_data(IOManager::filtered,d1,d1,vecMeteos);

		// PRECIPITATION
		// TODO: correct the precipitation at each station for the raincorrfact and the snowcorrfact
//		   double prec, rain, snow;
//		   for(n=1;n<=met->st->Z->nh;n++){
//					if((long)met->var[n-1][Pcode]!=number_novalue && (long)met->var[n-1][Pcode]!=number_absent){// check if exists prec. value
//					prec = met->var[n-1][Pcode];// precipitation of the meteo station n
//					part_snow(prec, &rain, &snow, met->var[n-1][Tcode], par->T_rain, par->T_snow);
//					met->var[n-1][Pcode] = par->raincorrfact * rain + par->snowcorrfact * snow;
//					//hnwgrid.grid2D(r, c) = par->raincorrfact * rain + par->snowcorrfact * snow;
//					}
//				}
//		    part_snow(prec, &rain, &snow, tagrid.grid2D(r, c), Train, Tsnow);
//

		cout <<"Time stamp " << d1.toString(d1.DIN) << std::endl;

		io.interpolate(d1, dem, MeteoData::TA, tagrid);
		io.interpolate(d1, dem, MeteoData::RH, rhgrid);
		io.interpolate(d1, dem, MeteoData::P, pgrid);
		io.interpolate(d1, dem, MeteoData::VW, vwgrid);
	  //  io.interpolate(d1, dem, MeteoData::DW, dwgrid);
		io.interpolate(d1, dem, MeteoData::HNW, hnwgrid);

		//convert values accordingly, necessary for TA , RH and P
		//changeRHgrid(rhgrid);// TODO: check whether GEOtop wants RH from 0 to 100 or from 0 to 1
		changeTAgrid(tagrid);
		io.write2DGrid(hnwgrid, "hnw_c.asc");
		changePgrid(pgrid);

	} catch (std::exception& e) {
		std::cerr << "[E] MeteoIO: " << e.what() << std::endl;
	}

	std::cout << "[MeteoIO] Start copying data to GEOtop format: " << std::endl;
	if (par->point_sim == 1) {
		// if point-wise simulation
		// Now copy all that data to the appropriate grids
		copyGridToTensorPointWise(novalue, tagrid, met->Tgrid,k ,par->chkpt);
		copyGridToTensorPointWise(novalue, rhgrid, met->RHgrid,k, par->chkpt);
		copyGridToTensorPointWise(novalue, pgrid, met->Pgrid,k, par->chkpt);
		copyGridToTensorPointWise(novalue, dwgrid, met->Vdir, k,par->chkpt);
		copyGridToTensorPointWise(novalue, vwgrid, met->Vgrid, k, par->chkpt);
		copyGridToTensorPointWise(novalue, hnwgrid, wat->PrecTot, k, par->chkpt);

	} else {
		// if distributed simulation
		//Now copy all that data to the appropriate grids
		copyGridToTensor(novalue, tagrid, met->Tgrid,k);
		copyGridToTensor(novalue, rhgrid, met->RHgrid,k);
		copyGridToTensor(novalue, pgrid, met->Pgrid,k);
		copyGridToTensor(novalue, vwgrid, met->Vgrid,k);
		copyGridToTensor(novalue, dwgrid, met->Vdir,k);
		copyGridToTensor(novalue, hnwgrid, wat->PrecTot,k);
	}
}

void copyGridToMatrix(const double& novalue, const Grid2DObject& gridObject,
		DOUBLEMATRIX* myGrid) {

	for (unsigned int ii = 0; ii < gridObject.nrows; ii++) {
		for (unsigned int jj = 0; jj < gridObject.ncols; jj++) {
			if (gridObject.grid2D(jj, gridObject.nrows - 1 - ii)
					== IOUtils::nodata) {
				myGrid->co[ii + 1][jj + 1] = novalue;
			} else {
				myGrid->co[ii + 1][jj + 1] = gridObject.grid2D(jj,
						gridObject.nrows - 1 - ii);
			}
		}
	}
}

void copyGridToTensor(const double& novalue, const Grid2DObject& gridObject,
		DOUBLETENSOR* myGrid, long indx) {

	for (unsigned int ii = 0; ii < gridObject.nrows; ii++) {
		for (unsigned int jj = 0; jj < gridObject.ncols; jj++) {
			if (gridObject.grid2D(jj, gridObject.nrows - 1 - ii)
					== IOUtils::nodata) {
				myGrid->co[indx][ii + 1][jj + 1] = novalue;
			} else {
				myGrid->co[indx][ii + 1][jj + 1] = gridObject.grid2D(jj,
						gridObject.nrows - 1 - ii);
			}
		}
	}
}

void copyGridToMatrixPointWise(const double& novalue,
		const Grid2DObject& gridObject, DOUBLEMATRIX* myGrid,
		DOUBLEMATRIX *coordinates) {
	std::string coordin = "", coordinparam = "";
	cfg.getValue("COORDSYS", "Input", coordin);
	cfg.getValue("COORDPARAM", "Input", coordinparam, Config::nothrow);
	Coords point(coordin, coordinparam);
	point.copyProj(gridObject.llcorner);

	double eastX, northY, pointValue;

	for (int i = 1; i <= coordinates->nrh; i++) {
		eastX = coordinates->co[i][ptX];
		northY = coordinates->co[i][ptY];
		point.setXY(eastX, northY, IOUtils::nodata);
		gridObject.gridify(point);
		pointValue = gridObject(point.getGridI(), point.getGridJ());
		myGrid->co[1][i] = pointValue;
	}
}

void copyGridToTensorPointWise(const double& novalue,
		const Grid2DObject& gridObject, DOUBLETENSOR* myGrid,long indx,
		DOUBLEMATRIX *coordinates) {
	std::string coordin = "", coordinparam = "";
	cfg.getValue("COORDSYS", "Input", coordin);
	cfg.getValue("COORDPARAM", "Input", coordinparam, Config::nothrow);
	Coords point(coordin, coordinparam);
	point.copyProj(gridObject.llcorner);

	double eastX, northY, pointValue;

	for (int i = coordinates->nrl; i <= coordinates->nrh; i++) {
		eastX = coordinates->co[i][ptX];
		northY = coordinates->co[i][ptY];
		point.setXY(eastX, northY, IOUtils::nodata);
		gridObject.gridify(point);
		pointValue = gridObject(point.getGridI(), point.getGridJ());
		// One row because in point wise simulations the matrix is formed by one row and n columns where n is the number of checkpoints
		myGrid->co[indx][1][i] = pointValue;
		}
}


void changeRHgrid(Grid2DObject& g2d) {
	for (unsigned int ii = 0; ii < g2d.ncols; ii++) {
		for (unsigned int jj = 0; jj < g2d.nrows; jj++) {
			if (g2d.grid2D(ii, jj) != IOUtils::nodata)
				g2d.grid2D(ii, jj) *= 100.0;
		}
	}
}


void changeTAgrid(Grid2DObject& g2d) {
	for (unsigned int ii = 0; ii < g2d.ncols; ii++) {
		for (unsigned int jj = 0; jj < g2d.nrows; jj++) {
			if (g2d.grid2D(ii, jj) != IOUtils::nodata)
				g2d.grid2D(ii, jj) -= 273.15; //back to celsius
		}
	}
}

void changePgrid(Grid2DObject& g2d) {
	for (unsigned int ii = 0; ii < g2d.ncols; ii++) {
		for (unsigned int jj = 0; jj < g2d.nrows; jj++) {
			if (g2d.grid2D(ii, jj) != IOUtils::nodata)
				g2d.grid2D(ii, jj) /= 100.0;
		}
	}
}


extern "C" double ***meteoio_readMeteoData(long*** column,
		METEO_STATIONS *stations, double novalue, long nrOfVariables, PAR *par,
		TIMES *times) {
	long ncols = nrOfVariables; //the total number of meteo variables used in GEOtop (should stay fixed)

	//Date d1 holds the beginning of this simulation, d2 the end date of the simulation 
	//d1=times->time;
	//d2=times->time+par->Dt;
	Date d1 = par->init_date->co[i_sim];
	Date d2 = par->end_date->co[i_sim];
	//	Date d1((int)par->year0, 1, 1, 0, 0);
	//	d1 += par->JD0;
	//	d1 += times->time/86400; //times->time is in seconds, conversion to julian by devision
	//
	//	Date d2((int)par->year0, 1, 1, 0, 0);
	//	d2 += par->JD0;
	//	d2 += times->TH/24;      //the end of the simulation

	//Construction a BufferIOHandler and reading the meteo data through meteoio as configured in io.ini
	Config cfg("io.ini");
	IOManager io(cfg);

	std::vector<std::vector<MeteoData> > vecMeteo; //the dimension of this vector will be nrOfStations
	std::vector<std::vector<StationData> > vecStation;//the dimension of this vector will be nrOfStations

	cout << "[I] MeteoIO: Fetching all data between " << d1.toString(Date::ISO)
			<< " and " << d2.toString(Date::ISO) << endl;

	/*
	 * In the following section the BufferedIOHandler is used to request data for descrete times
	 * MeteoIO will deal with accumulation, linear interpolation, filtering and cleaning of the data
	 * During the loop a discrete amount of time will be added to the loop variable (one hour)
	 */
	for (Date currentDate = d1; currentDate <= d2; currentDate += Date(1.0
			/ 24.0)) {
		cout << "[I] MeteoIO: Getting data for " << currentDate.toString(
				Date::ISO) << endl;
		std::vector<MeteoData> vecM; //dimension: nrOfStations
		std::vector<StationData> vecS; //dimension: nrOfStations

		io.getMeteoData(currentDate, vecM); //reading meteo data and meta datafor currentDate

		//the data needs to be appended to the collection of all read meteo and meta data:
		for (unsigned int jj = 0; jj < vecM.size(); jj++) {
			if (currentDate == d1) {
				vecMeteo.push_back(std::vector<MeteoData>());
				vecStation.push_back(std::vector<StationData>());
			}
			vecMeteo.at(jj).push_back(vecM.at(jj)); //append meteo data to vector
			vecStation.at(jj).push_back(vecS.at(jj)); //append meta data to vector
		}
	}
	cout << "[I] MeteoIO: " << "Finished getting meteo and station data"
			<< endl;
	//print out all data if configured by user
	try {
		string tmp;
		cfg.getValue("METEO", "OUTPUT", tmp);
		io.writeMeteoData(vecMeteo);
	} catch (std::exception& e) {
	} //Do nothing if not configured or error happens

	//Deal with meta data, that is allocate met->st and fill with data
	std::vector<StationData> vecMyStation;
	io.getStationData(d1, vecMyStation);

	initializeMetaData(vecMyStation, d1, novalue, par, stations);
	cout << "[I] MeteoIO: Amount of stations: " << vecMeteo.size() << endl;

	//resize the column matrix - it will hold information about what column in the data matrix holds what parameter
	*column = (long**) realloc(*column, vecStation.size() * sizeof(long*));

	double ***data = (double ***) malloc(vecMeteo.size() * sizeof(double**));
	short novalueend = 1;

	for (unsigned int jj = 0; jj < vecStation.size(); jj++) { //for each station
		//{Iprec, WindS, WindDir, RelHum, AirT, AirP, SWglobal, SWdirect, SWdiffuse, TauCloud, Cloud, LWin, SWnet, Tsup}
		//iPt ,iWs ,iWdir , iWsx , iWsy, iRh ,iT ,iTdew ,iPs,iSW ,iSWb ,iSWd,itauC ,iC,iLWi,iSWn
		(*column)[jj] = (long*) malloc((ncols + 1) * sizeof(long));
		//(*column)[jj][ncols] = end_vector_long;
		for (int ff = 0; ff < nrOfVariables; ff++) {
			//(*column)[jj][ff] = ff;
			(*column)[jj][ff] = -1;
		}

		unsigned int ii = 0;
		while ((*column)[jj][ii] != 999999) {//the geotop way of ending a vector
			//cout << "Station " << jj << "  " << ii << ": " << (*column)[jj][ii] << endl;
			ii++;
		}

		data[jj] = (double **) malloc(vecMeteo[jj].size() * sizeof(double*));

		unsigned int ll = 0;
		for (ll = 0; ll < vecMeteo[jj].size(); ll++) {
			data[jj][ll] = (double *) malloc((ncols + 1) * sizeof(double));

			//Put data in the correct cell

			data[jj][ll][0] = vecMeteo[jj][ll].HNW;
			data[jj][ll][1] = vecMeteo[jj][ll].VW;
			data[jj][ll][2] = vecMeteo[jj][ll].DW;
			data[jj][ll][3] = vecMeteo[jj][ll].RH * 100.00; //MeteoIO deals with RH in values [0;1]
			data[jj][ll][4] = vecMeteo[jj][ll].TA - 273.15; //MeteoIO deals with temperature in Kelvin
			data[jj][ll][5] = vecMeteo[jj][ll].P;
			data[jj][ll][6] = vecMeteo[jj][ll].ISWR;
			data[jj][ll][7] = novalue;
			data[jj][ll][8] = novalue;
			data[jj][ll][9] = novalue;
			data[jj][ll][10] = novalue;
			data[jj][ll][11] = novalue;
			data[jj][ll][12] = novalue;
			data[jj][ll][13] = novalue;
			//data[jj][ll][ncols] = end_vector;

			for (int gg = 0; gg < nrOfVariables; gg++) {
				if (data[jj][ll][gg] == IOUtils::nodata) {
					data[jj][ll][gg] = novalue;
				} else if (data[jj][ll][gg] != novalue) {
					(*column)[jj][gg] = gg; //HACK!
				}
			}

			//Comment this in if you dont want to see the data that was retrieved
			cout << "MeteoData [" << ll + 1 << "/" << vecMeteo[jj].size()
					<< "]:" << endl;
			//	std::cout << vecMeteo[jj][ll].toString() << std::endl;

		}

		for (int ff = 1; ff <= ncols; ff++) {
			if (ll > 0)
				if (data[jj][ll - 1][ff] != novalue)
					novalueend = 0;
		}

		//		if(novalueend==0){
		//			data[jj]=(double **)realloc(data[jj],(vecMeteo[jj].size()+1)*sizeof(double*));
		//			data[jj][vecMeteo[jj].size()] = (double *)malloc(sizeof(double));
		//			//data[jj][vecMeteo[jj].size()][0]=end_vector;
		//		} else {
		//			if (vecMeteo[jj].size()>0)
		//				data[jj][vecMeteo[jj].size()-1][0]=end_vector;
		//		}
	}

	cout << "[I] MeteoIO NOVAL used          : " << novalue << endl;
	cout << "[I] MeteoIO #of meteo parameters: " << nrOfVariables << endl;

	//Testing access to the whole tensor
	for (unsigned int ii = 0; ii < vecStation.size(); ii++) {
		double d = 0.0;
		for (unsigned int ll = 0; ll < vecMeteo[ii].size(); ll++) {
			for (int kk = 0; kk < ncols; kk++) {
				//printf("%f ", data[ii][ll][kk]);
				d = data[ii][ll][kk];
			}
			//printf("\n");
		}
	}

	return data; //return pointer to heap allocated memory
}

void initializeMetaData(const std::vector<StationData>& vecStation,
		const Date& startDate, const double& novalue, PAR *par,
		METEO_STATIONS *stations) {
	unsigned int nrOfStations = vecStation.size();

	//Initialize the station data: set beginning of data
	int year, month, day, hour, minute;
	startDate.getDate(year, month, day, hour, minute);
	//	Date JD_start = startDate - Date((int)par->year0, 1, 1, 0, 0); // this is the actual elapsed simulation time

	//init station struct met->st
	stations->E = new_doublevector(nrOfStations); // East coordinate [m] of the meteo station
	stations->N = new_doublevector(nrOfStations); // North coordinate [m] of the meteo station
	stations->lat = new_doublevector(nrOfStations);// Latitude [rad] of the meteo station
	stations->lon = new_doublevector(nrOfStations);// Longitude [rad] of the meteo station
	stations->Z = new_doublevector(nrOfStations); // Elevation [m] of the meteo station
	stations->sky = new_doublevector(nrOfStations);// Sky-view-factor [-] of the meteo station
	stations->ST = new_doublevector(nrOfStations); // Standard time minus UTM [hours] of the meteo station
	stations->Vheight = new_doublevector(nrOfStations); // Wind velocity measurement height [m] (a.g.l.)
	stations->Theight = new_doublevector(nrOfStations); // Air temperature measurement height [m] (a.g.l.)
	//	stations->JD0=new_doublevector(nrOfStations);// Decimal Julian Day of the first data
	//	stations->Y0=new_longvector(nrOfStations);	// Year of the first data
	//	stations->Dt=new_doublevector(nrOfStations);	// Dt of sampling of the data [sec]
	//	stations->offset=new_longvector(nrOfStations);// offset column

	for (unsigned int ii = 1; ii <= nrOfStations; ii++) { //HACK
		std::cout << "[I] MeteoIO station " << ii << ":\n"
				<< vecStation[ii - 1] << std::endl;

		stations->E->co[ii] = vecStation[ii - 1].position.getEasting();
		stations->N->co[ii] = vecStation[ii - 1].position.getNorthing();
		stations->lat->co[ii] = vecStation[ii - 1].position.getLat() * PI
				/ 180.0;// from deg to [rad]
		stations->lon->co[ii] = vecStation[ii - 1].position.getLon() * PI
				/ 180.0;// from deg to [rad]
		stations->Z->co[ii] = vecStation[ii - 1].position.getAltitude();
		stations->sky->co[ii] = novalue;
		stations->ST->co[ii] = 0;
		stations->Vheight->co[ii] = 8.0;
		stations->Theight->co[ii] = 8.0;
		//		stations->JD0->co[ii]     = JD_start.getJulianDate();
		//		stations->Y0->co[ii]      = year;
		//		stations->Dt->co[ii]      = 3600.0; //in seconds
		//		stations->offset->co[ii]  = 1;

		//cout << "JD beginning of data: " << stations->JD0->co[ii] << endl;
		//cout << "Y0 beginning of data: " << stations->Y0->co[ii] << endl;
		//cout << "Dt beginning of data: " << stations->Dt->co[ii] << endl;
	}
}

DOUBLEMATRIX * doubletens_to_doublemat(DOUBLETENSOR * input,
		DOUBLEMATRIX * output, long indx) {
	/*
	 * this functions transforms a doubletensor to a doublematrix
	 */
	long r, c;
	for (r = input->nrl; r <= input->nrh; r++) {
		for (c = input->ncl; c <= input->nch; c++) {
			output->co[r][c] = input->co[indx][r][c];
		}
	}
	return output;
}

DOUBLETENSOR * doublemat_to_doubletens(DOUBLEMATRIX * input,
		DOUBLETENSOR * output, long indx) {
	/*
	 * this functions transforms a doublematrix to a doubletensor
	 * \param: indx (long) is the index of the doubletensor in which to copy the matrix
	 */
	long r, c;
	for (r = input->nrl; r <= input->nrh; r++) {
		for (c = input->ncl; c <= input->nch; c++) {
			output->co[indx][r][c] = input->co[r][c];
		}
	}
	return output;
}
#endif
