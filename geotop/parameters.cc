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

#include "parameters.h"
#include "constants.h"
#include <iomanip>
#include <inputKeywords.h>
#include "geotop_common.h"

using namespace std;
using namespace boost::assign ;

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/

/** @brief get a double parameter from the configuration file and return it, applying default value.
  * @param[in] pConfigStore a pointer to a previously initialized configuration store
  * @param[in] pName the keyword of the parameter to read
  * @param[in] pDefaultValue a default value to be applied if the value of the parameter is geotop::input::gDoubleNoValue
  * @param[in] pAllowNoValue if true the returning of no value is permitted, by default is false
  * @return the parameter value. If the keyword does not exists or if the return value is
  *     geotop::input::gDoubleNoValue (and pAllowNoValue == false)
  *     the function will cause an exit from the program by calling the t_error function
  */
static double getDoubleValueWithDefault(const boost::shared_ptr<geotop::input::ConfigStore> pConfigStore, const std::string pName, const double pDefaultValue, const bool pAllowNoValue = false){
    
	double lValue;
    bool lGetResult = pConfigStore->get(pName, lValue) ;
    
    if(not lGetResult) {
        t_error(std::string("Fatal Error: unable to get parameter: ") + pName);
    }
    
    if(lValue == geotop::input::gDoubleNoValue) {
        lValue = pDefaultValue ;
    }
    
    if(lValue == geotop::input::gDoubleNoValue && not pAllowNoValue) {
        t_error(std::string("Fatal Error: value not assigned: ") + pName);
    }
    
    return lValue ;
}

/** @brief get a double vector parameter from the configuration file and return a modified version (extend the size, apply default values, etc.)
  * @param[in] pConfigStore a pointer to a previously initialized configuration store
  * @param[in] pName the keyword of the parameter to read
  * @param[in] pDefaultValue a default value to be applied if the value of the parameter is geotop::input::gDoubleNoValue
  * @param[in] pUsePrevElement if true, use the element repeat the last element instead of the default value, use the default value only
  *     if the latest value is geotop::input::gDoubleNoValue. If false use the default value for each element.
  * @param[in] pLength the expected length of the output vector
  * @param[in] pAllowNoValue if true the returning of a vector containing no value is permitted, by default is false
  * @return the parameter with the array of double. If the keyword does not exists or if the return value is
  *     geotop::input::gDoubleNoValue (and pAllowNoValue == false)
  *     the function will cause an exit from the program by calling the t_error function
  */
static std::vector<double> getDoubleVectorValueWithDefault(const boost::shared_ptr<geotop::input::ConfigStore> pConfigStore, const std::string pName, const double pDefaultValue, const bool pUsePrevElement, const size_t pLength, const bool pAllowNoValue = false){

    std::vector<double> lValue;
    bool lGetResult = pConfigStore->get(pName, lValue) ;
    
    size_t lDesiredLength = pLength ;
    if ( pLength == 0 ) {
        lDesiredLength = lValue.size() ;
    }
    
    if(not lGetResult) {
        t_error(std::string("Fatal Error: unable to get parameter: ") + pName);
    }

    size_t lLength = lValue.size() ;
    
    for (size_t i = 0; i < lDesiredLength ; i++) {
        double lElementValue = geotop::input::gDoubleNoValue ;
        
        if(i < lLength) {
            lElementValue = lValue[i] ;
        } else {
            lValue.push_back( geotop::input::gDoubleNoValue ) ;
        }

        if ( lElementValue == geotop::input::gDoubleNoValue ) {
            if( i > 0 ) {
                if(pUsePrevElement)
                    lElementValue = lValue[i-1] ;
                else
                    lElementValue = pDefaultValue ;
            } else {
                lElementValue = pDefaultValue ;
            }
        }

        if ( not pAllowNoValue && lElementValue == geotop::input::gDoubleNoValue ) {
            t_error(std::string("Fatal Error: array value not assigned: ") + pName);
        } else {
            lValue[i] = lElementValue ;
        }
    }
    
    return lValue ;
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/

/** @brief return a list with requested parameters
  * @param[in] pConfigStore a pointer to a previously initialized configuration store
  * @param[in] pKeys a list with the requested key
  * @return the array with the requested parameters
  */
static std::vector<std::string> getStringValues(const boost::shared_ptr<geotop::input::ConfigStore> pConfigStore, const std::vector<std::string> &pKeys){
	std::vector<std::string> lVector;
    
	for (size_t i=0; i<pKeys.size(); i++) {
        std::string lValue;
        bool lGetResult = pConfigStore->get(pKeys[i], lValue) ;
        if(lGetResult == false){
            t_error(std::string("Fatal Error: unable to get parameter: ") + pKeys[i]);
        }
        lVector.push_back(lValue);
	}
    
	return(lVector);
    
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/

short read_inpts_par(Par *par, Land *land, Times *times, Soil *sl, Meteo *met, InitTools *itools, std::string filename, FILE *flog){

    std::vector<std::string> string_param;

    std::string temp;
    std::string path_rec_files;
	
    boost::shared_ptr<geotop::input::ConfigStore> lConfigStore = geotop::input::ConfigStoreSingletonFactory::getInstance() ;

	//assign parameter
	assign_numeric_parameters(par, land, times, sl, met, itools, flog);

	//assign parameter
    std::vector<std::string> lKeys ;
    lKeys += "HeaderDateDDMMYYYYhhmmMeteo", "HeaderJulianDayfrom0Meteo", "HeaderIPrec",
        "HeaderPrec", "HeaderWindVelocity", "HeaderWindDirection", "HeaderWindX",
        "HeaderWindY", "HeaderRH", "HeaderAirTemp", "HeaderDewTemp", "HeaderSWglobal",
        "HeaderSWdirect", "HeaderSWdiffuse", "HeaderCloudSWTransmissivity", "HeaderCloudFactor",
        "HeaderLWin", "HeaderSWnet", "HeaderSurfaceTemperature" ;
	itools->met_col_names = getStringValues(lConfigStore, lKeys) ;
    
    lKeys.clear() ;
    lKeys += "HeaderSoilDz", "HeaderSoilInitPres", "HeaderSoilInitTemp", "HeaderNormalHydrConductivity",
        "HeaderLateralHydrConductivity", "HeaderThetaRes", "HeaderWiltingPoint", "HeaderFieldCapacity",
        "HeaderThetaSat", "HeaderAlpha", "HeaderN", "HeaderV", "HeaderKthSoilSolids",
        "HeaderCthSoilSolids", "HeaderSpecificStorativity" ;
	itools->soil_col_names = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "HeaderHorizonAngle", "HeaderHorizonHeight" ;
	itools->horizon_col_names = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "TimeStepsFile", "SoilParFile", "MeteoFile", "MeteoStationsListFile",
    "LapseRateFile", "HorizonMeteoStationFile", "PointFile", "HorizonPointFile",
    "TimeDependentVegetationParameterFile", "TimeDependentIncomingDischargeFile", "DemFile", "LandCoverMapFile",
    "SoilMapFile", "DaysDelayMapFile", "SkyViewFactorMapFile", "SlopeMapFile",
    "RiverNetwork", "AspectMapFile", "CurvaturesMapFile", "BedrockDepthMapFile",
    "InitWaterTableDepthMapFile", "InitSnowDepthMapFile", "InitSWEMapFile", "InitSnowAgeMapFile",
    "InitGlacierDepthMapFile", "DischargeFile", "BasinOutputFile", "BasinOutputFileWriteEnd",
    "PointOutputFile", "PointOutputFileWriteEnd", "SoilTempProfileFile", "SoilTempProfileFileWriteEnd",
    "SoilAveragedTempProfileFile", "SoilAveragedTempProfileFileWriteEnd", "SoilLiqWaterPressProfileFile", "SoilLiqWaterPressProfileFileWriteEnd",
    "SoilTotWaterPressProfileFile", "SoilTotWaterPressProfileFileWriteEnd", "SoilLiqContentProfileFile", "SoilLiqContentProfileFileWriteEnd",
    "SoilAveragedLiqContentProfileFile", "SoilAveragedLiqContentProfileFileWriteEnd", "SoilIceContentProfileFile", "SoilIceContentProfileFileWriteEnd",
    "SoilAveragedIceContentProfileFile", "SoilAveragedIceContentProfileFileWriteEnd", "SoilSaturationRatioProfileFile", "SnowTempProfileFile",
    "SnowLiqContentProfileFile", "SnowIceContentProfileFile", "SnowDepthLayersFile", "SnowTempProfileFileWriteEnd",
    "SnowLiqContentProfileFileWriteEnd", "SnowIceContentProfileFileWriteEnd", "SnowDepthLayersFileWriteEnd", "GlacierProfileFile",
    "GlacierProfileFileWriteEnd", "SnowCoveredAreaFile", "RunSoilAveragedTemperatureFile", "RunSoilAveragedTotalSoilMoistureFile",
    "RunSoilAveragedInternalEnergyFile", "RunSoilAveragedSnowWaterEquivalentFile", "RunSoilMaximumTemperatureFile", "RunSoilMinimumTemperatureFile",
    "RunSoilMaximumTotalSoilMoistureFile", "RunSoilMinimumTotalSoilMoistureFile", "SoilTempTensorFile", "FirstSoilLayerTempMapFile",
    "SoilAveragedTempTensorFile", "FirstSoilLayerAveragedTempMapFile", "SoilLiqContentTensorFile", "FirstSoilLayerLiqContentMapFile",
    "SoilAveragedLiqContentTensorFile", "SoilIceContentTensorFile", "FirstSoilLayerIceContentMapFile", "SoilAveragedIceContentTensorFile",
    "LandSurfaceWaterDepthMapFile", "ChannelSurfaceWaterDepthMapFile", "NetRadiationMapFile", "InLongwaveRadiationMapFile",
    "NetLongwaveRadiationMapFile", "NetShortwaveRadiationMapFile", "InShortwaveRadiationMapFile", "DirectInShortwaveRadiationMapFile",
    "ShadowFractionTimeMapFile", "SurfaceHeatFluxMapFile", "SurfaceSensibleHeatFluxMapFile", "SurfaceLatentHeatFluxMapFile",
    "SurfaceTempMapFile", "PrecipitationMapFile", "CanopyInterceptedWaterMapFile", "SoilLiqWaterPressTensorFile",
    "SoilTotWaterPressTensorFile", "SnowDepthMapFile", "GlacierDepthMapFile", "SnowMeltedMapFile",
    "SnowSublMapFile", "GlacierMeltedMapFile", "GlacierSublimatedMapFile", "AirTempMapFile",
    "WindSpeedMapFile", "WindDirMapFile", "RelHumMapFile", "SWEMapFile",
    "GlacierWaterEqMapFile", "SnowDurationMapFile", "ThawedSoilDepthMapFile", "ThawedSoilDepthFromAboveMapFile",
    "WaterTableDepthMapFile", "WaterTableDepthFromAboveMapFile", "NetPrecipitationMapFile", "EvapotranspirationFromSoilMapFile",
    "SpecificPlotSurfaceHeatFluxMapFile", "SpecificPlotTotalSensibleHeatFluxMapFile", "SpecificPlotTotalLatentHeatFluxMapFile", "SpecificPlotSurfaceSensibleHeatFluxMapFile",
    "SpecificPlotSurfaceLatentHeatFluxMapFile", "SpecificPlotVegSensibleHeatFluxMapFile", "SpecificPlotVegLatentHeatFluxMapFile", "SpecificPlotIncomingShortwaveRadMapFile",
    "SpecificPlotNetSurfaceShortwaveRadMapFile", "SpecificPlotNetVegShortwaveRadMapFile", "SpecificPlotIncomingLongwaveRadMapFile", "SpecificPlotNetSurfaceLongwaveRadMapFile",
    "SpecificPlotNetVegLongwaveRadMapFile", "SpecificPlotCanopyAirTempMapFile", "SpecificPlotSurfaceTempMapFile", "SpecificPlotVegTempMapFile",
    "SpecificPlotAboveVegAirTempMapFile", "SpecificPlotWindSpeedMapFile", "SpecificPlotWindDirMapFile", "SpecificPlotRelHumMapFile",
    "SpecificPlotSnowDepthMapFile", "SpecificPlotSurfaceWaterContentMapFile", "RecoverSoilWatPres", "RecoverSoilIceCont",
    "RecoverSoilTemp", "RecoverSnowLayerThick", "RecoverSnowLiqMass", "RecoverSnowIceMass",
    "RecoverSnowTemp", "RecoverGlacierLayerThick", "RecoverGlacierLiqMass", "RecoverGlacierIceMass",
    "RecoverGlacierTemp", "RecoverSnowLayerNumber", "RecoverGlacierLayerNumber", "RecoverNonDimensionalSnowAge",
    "RecoverLiqWaterOnCanopy", "RecoverSnowOnCanopy", "RecoverVegTemp", "RecoverSoilWatPresChannel",
    "RecoverSoilIceContChannel", "RecoverSoilTempChannel", "RecoverRunSoilAveragedTemperatureFile", "RecoverRunSoilAveragedTotalSoilMoistureFile",
    "RecoverRunSoilAveragedInternalEnergyFile", "RecoverRunSoilAveragedSnowWaterEquivalentFile", "RecoverRunSoilMaximumTemperatureFile", "RecoverRunSoilMinimumTemperatureFile",
    "RecoverRunSoilMaximumTotalSoilMoistureFile", "RecoverRunSoilMinimumTotalSoilMoistureFile", "RecoverTime", "SuccessfulRecoveryFile" ;
	geotop::common::Variables::files = getStringValues(lConfigStore, lKeys) ;
    
    lKeys.clear() ;
    lKeys += "HeaderDatePoint", "HeaderJulianDayFromYear0Point", "HeaderTimeFromStartPoint", "HeaderPeriodPoint",
    "HeaderRunPoint", "HeaderIDPointPoint", "HeaderPsnowPoint", "HeaderPrainPoint",
    "HeaderPsnowNetPoint", "HeaderPrainNetPoint", "HeaderPrainOnSnowPoint", "HeaderWindSpeedPoint",
    "HeaderWindDirPoint", "HeaderRHPoint", "HeaderAirPressPoint", "HeaderAirTempPoint",
    "HeaderTDewPoint", "HeaderTsurfPoint", "HeaderTvegPoint", "HeaderTCanopyAirPoint",
    "HeaderSurfaceEBPoint", "HeaderSoilHeatFluxPoint", "HeaderSWinPoint", "HeaderSWbeamPoint",
    "HeaderSWdiffPoint", "HeaderLWinPoint", "HeaderLWinMinPoint", "HeaderLWinMaxPoint",
    "HeaderSWNetPoint", "HeaderLWNetPoint", "HeaderHPoint", "HeaderLEPoint",
    "HeaderCanopyFractionPoint", "HeaderLSAIPoint", "Headerz0vegPoint", "Headerd0vegPoint",
    "HeaderEstoredCanopyPoint", "HeaderSWvPoint", "HeaderLWvPoint", "HeaderHvPoint",
    "HeaderLEvPoint", "HeaderHgUnvegPoint", "HeaderLEgUnvegPoint", "HeaderHgVegPoint",
    "HeaderLEgVegPoint", "HeaderEvapSurfacePoint", "HeaderTraspCanopyPoint", "HeaderWaterOnCanopyPoint",
    "HeaderSnowOnCanopyPoint", "HeaderQVegPoint", "HeaderQSurfPoint", "HeaderQAirPoint",
    "HeaderQCanopyAirPoint", "HeaderLObukhovPoint", "HeaderLObukhovCanopyPoint", "HeaderWindSpeedTopCanopyPoint",
    "HeaderDecayKCanopyPoint", "HeaderSWupPoint", "HeaderLWupPoint", "HeaderHupPoint",
    "HeaderLEupPoint", "HeaderSnowDepthPoint", "HeaderSWEPoint", "HeaderSnowDensityPoint",
    "HeaderSnowTempPoint", "HeaderSnowMeltedPoint", "HeaderSnowSublPoint", "HeaderSWEBlownPoint",
    "HeaderSWESublBlownPoint", "HeaderGlacDepthPoint", "HeaderGWEPoint", "HeaderGlacDensityPoint",
    "HeaderGlacTempPoint", "HeaderGlacMeltedPoint", "HeaderGlacSublPoint", "HeaderLowestThawedSoilDepthPoint",
    "HeaderHighestThawedSoilDepthPoint", "HeaderLowestWaterTableDepthPoint", "HeaderHighestWaterTableDepthPoint" ;
	geotop::common::Variables::hpnt = getStringValues(lConfigStore, lKeys) ;
    
    lKeys.clear() ;
    lKeys += "HeaderDateBasin", "HeaderJulianDayFromYear0Basin", "HeaderTimeFromStartBasin", "HeaderPeriodBasin",
    "HeaderRunBasin", "HeaderPRainNetBasin", "HeaderPSnowNetBasin", "HeaderPRainBasin",
    "HeaderPSnowBasin", "HeaderPNetBasin", "HeaderAirTempBasin", "HeaderTSurfBasin",
    "HeaderTvegBasin", "HeaderEvapSurfaceBasin", "HeaderTraspCanopyBasin", "HeaderLEBasin",
    "HeaderHBasin", "HeaderSWNetBasin", "HeaderLWNetBasin", "HeaderLEvBasin",
    "HeaderHvBasin", "HeaderSWvBasin", "HeaderLWvBasin", "HeaderSWinBasin",
    "HeaderLWinBasin", "HeaderMeanTimeStep", "HeaderTimeStepAverage" ;
	geotop::common::Variables::hbsn = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "HeaderDateSnow", "HeaderJulianDayFromYear0Snow", "HeaderTimeFromStartSnow", "HeaderPeriodSnow",
    "HeaderRunSnow", "HeaderIDPointSnow", "HeaderTempSnow", "HeaderIceContentSnow",
    "HeaderWatContentSnow", "HeaderDepthSnow" ;
	geotop::common::Variables::hsnw = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "HeaderDateGlac", "HeaderJulianDayFromYear0Glac", "HeaderTimeFromStartGlac", "HeaderPeriodGlac",
    "HeaderRunGlac", "HeaderIDPointGlac", "HeaderTempGlac", "HeaderIceContentGlac",
    "HeaderWatContentGlac", "HeaderDepthGlac" ;
	geotop::common::Variables::hglc = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "HeaderDateSoil", "HeaderJulianDayFromYear0Soil", "HeaderTimeFromStartSoil", "HeaderPeriodSoil",
    "HeaderRunSoil", "HeaderIDPointSoil" ;
	geotop::common::Variables::hsl = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "HeaderPointID", "HeaderCoordinatePointX", "HeaderCoordinatePointY", "HeaderPointElevation",
    "HeaderPointLandCoverType", "HeaderPointSoilType", "HeaderPointSlope", "HeaderPointAspect",
    "HeaderPointSkyViewFactor", "HeaderPointCurvatureNorthSouthDirection", "HeaderPointCurvatureWestEastDirection", "HeaderPointCurvatureNorthwestSoutheastDirection",
    "HeaderPointCurvatureNortheastSouthwestDirection", "HeaderPointDepthFreeSurface", "HeaderPointHorizon", "HeaderPointMaxSWE",
    "HeaderPointLatitude", "HeaderPointLongitude", "HeaderPointBedrockDepth" ;
	itools->point_col_names = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "HeaderDateDDMMYYYYhhmmLapseRates", "HeaderLapseRateTemp", "HeaderLapseRateDewTemp",
    "HeaderLapseRatePrec" ;
	itools->lapserates_col_names = getStringValues(lConfigStore, lKeys) ;

    lKeys.clear() ;
    lKeys += "HeaderIDMeteoStation", "HeaderMeteoStationCoordinateX", "HeaderMeteoStationCoordinateY", "HeaderMeteoStationLatitude",
    "HeaderMeteoStationLongitude", "HeaderMeteoStationElevation", "HeaderMeteoStationSkyViewFactor", "HeaderMeteoStationStandardTime" ;
	itools->meteostations_col_names = getStringValues(lConfigStore, lKeys) ;
	
    lKeys.clear() ;
    lKeys += "SuccessfulRunFile" ;
    std::vector<std::string> lValues =  getStringValues(lConfigStore, lKeys) ;
	temp = lValues[0] ;
	if (geotop::input::gStringNoValue == temp) {
		temp = "_SUCCESSFUL_RUN";
	}
	geotop::common::Variables::SuccessfulRunFile = geotop::common::Variables::WORKING_DIRECTORY + temp ;

    lKeys.clear() ;
    lKeys += "FailedRunFile" ;
    lValues =  getStringValues(lConfigStore, lKeys) ;
	temp = lValues[0] ;
	if (geotop::input::gStringNoValue == temp) {
		temp = "_FAILED_RUN";
	}	
	geotop::common::Variables::FailedRunFile = geotop::common::Variables::WORKING_DIRECTORY + temp;
	
    lKeys.clear() ;
    lKeys += "SubfolderRecoveryFiles" ;
    lValues =  getStringValues(lConfigStore, lKeys) ;
	path_rec_files = lValues[0] ; //path of recovery files

	//replace none value with some default values
	
	//Horizon
	for (size_t j=0; j<2; j++) {
		if(itools->horizon_col_names[j] == geotop::input::gStringNoValue){
			if (j==0) {
                itools->horizon_col_names[j] = "AngleFromNorthClockwise";
			}else
                if (j==1) {  itools->horizon_col_names[j] = "HorizonHeight";
			}
		}
	}
	
	//HeaderPointFile
	for (size_t j=0; j<otot; j++) {
		if(geotop::common::Variables::hpnt[j] == geotop::input::gStringNoValue ){
			if(j==odate12){ geotop::common::Variables::hpnt[j] = "Date12[DDMMYYYYhhmm]" ;
			}else if(j==oJDfrom0){ geotop::common::Variables::hpnt[j] = "JulianDayFromYear0[days]" ;
			}else if(j==odaysfromstart){ geotop::common::Variables::hpnt[j] = "TimeFromStart[days]" ;
			}else if(j==operiod){ geotop::common::Variables::hpnt[j] = "Simulation_Period" ;
			}else if(j==orun){ geotop::common::Variables::hpnt[j] = "Run" ;
			}else if(j==opoint){ geotop::common::Variables::hpnt[j] = "IDpoint" ;
			}else if(j==osnowover){geotop::common::Variables::hpnt[j] = "Psnow_over_canopy[mm]" ;     	
			}else if(j==orainover){geotop::common::Variables::hpnt[j] = "Prain_over_canopy[mm]" ; 		
			}else if(j==oprecsnow){geotop::common::Variables::hpnt[j] = "Psnow_under_canopy[mm]" ; 
			}else if(j==oprecrain){geotop::common::Variables::hpnt[j] = "Prain_under_canopy[mm]" ; 		
			}else if(j==orainonsnow){geotop::common::Variables::hpnt[j] = "Prain_rain_on_snow[mm]" ;
			}else if(j==oV){geotop::common::Variables::hpnt[j] = "Wind_speed[m/s]" ;      	   
			}else if(j==oVdir){geotop::common::Variables::hpnt[j] = "Wind_direction[deg]" ;  
			}else if(j==oRH){geotop::common::Variables::hpnt[j] = "Relative_Humidity[-]" ;    
			}else if(j==oPa){geotop::common::Variables::hpnt[j] = "Pressure[mbar]" ;    
			}else if(j==oTa){geotop::common::Variables::hpnt[j] = "Tair[C]" ;    
			}else if(j==oTdew){geotop::common::Variables::hpnt[j] = "Tdew[C]" ;  
			}else if(j==oTg){geotop::common::Variables::hpnt[j] = "Tsurface[C]" ;    
			}else if(j==oTv){geotop::common::Variables::hpnt[j] = "Tvegetation[C]" ;    
			}else if(j==oTs){geotop::common::Variables::hpnt[j] = "Tcanopyair[C]" ;    
			}else if(j==oEB){geotop::common::Variables::hpnt[j] = "Surface_Energy_balance[W/m2]" ;    
			}else if(j==oG){geotop::common::Variables::hpnt[j] = "Soil_heat_flux[W/m2]" ;     
			}else if(j==oSWin){geotop::common::Variables::hpnt[j] = "SWin[W/m2]" ;  
			}else if(j==oSWb){geotop::common::Variables::hpnt[j] = "SWbeam[W/m2]" ;   
			}else if(j==oSWd){geotop::common::Variables::hpnt[j] = "SWdiff[W/m2]" ;  
			}else if(j==oLWin){geotop::common::Variables::hpnt[j] = "LWin[W/m2]" ; 
			}else if(j==ominLWin){geotop::common::Variables::hpnt[j] = "LWin_min[W/m2]" ; 
			}else if(j==omaxLWin){geotop::common::Variables::hpnt[j] = "LWin_max[W/m2]" ;
			}else if(j==oSW){geotop::common::Variables::hpnt[j] = "SWnet[W/m2]" ;     
			}else if(j==oLW){geotop::common::Variables::hpnt[j] = "LWnet[W/m2]" ;     
			}else if(j==oH){geotop::common::Variables::hpnt[j] = "H[W/m2]" ;      
			}else if(j==oLE){geotop::common::Variables::hpnt[j] = "LE[W/m2]" ;     
			}else if(j==ofc){geotop::common::Variables::hpnt[j] = "Canopy_fraction[-]" ;     
			}else if(j==oLSAI){geotop::common::Variables::hpnt[j] = "LSAI[m2/m2]" ;   
			}else if(j==oz0v){geotop::common::Variables::hpnt[j] = "z0veg[m]" ;    
			}else if(j==od0v){geotop::common::Variables::hpnt[j] = "d0veg[m]" ;    
			}else if(j==oEcan){geotop::common::Variables::hpnt[j] = "Estored_canopy[W/m2]" ;   
			}else if(j==oSWv){geotop::common::Variables::hpnt[j] = "SWv[W/m2]" ;    
			}else if(j==oLWv){geotop::common::Variables::hpnt[j] = "LWv[W/m2]" ;    
			}else if(j==oHv){geotop::common::Variables::hpnt[j] = "Hv[W/m2]" ;     
			}else if(j==oLEv){geotop::common::Variables::hpnt[j] = "LEv[W/m2]" ;    
			}else if(j==oHg0){geotop::common::Variables::hpnt[j] = "Hg_unveg[W/m2]" ;    
			}else if(j==oLEg0){geotop::common::Variables::hpnt[j] = "LEg_unveg[W/m2]" ;   
			}else if(j==oHg1){geotop::common::Variables::hpnt[j] = "Hg_veg[W/m2]" ;    
			}else if(j==oLEg1){geotop::common::Variables::hpnt[j] = "LEg_veg[W/m2]" ;   
			}else if(j==oevapsur){geotop::common::Variables::hpnt[j] = "Evap_surface[mm]" ;  
			}else if(j==otrasp){geotop::common::Variables::hpnt[j] = "Trasp_canopy[mm]" ;    
			}else if(j==owcan_rain){geotop::common::Variables::hpnt[j] = "Water_on_canopy[mm]" ;
			}else if(j==owcan_snow){geotop::common::Variables::hpnt[j] = "Snow_on_canopy[mm]" ;
			}else if(j==oQv){geotop::common::Variables::hpnt[j] = "Qvegetation[-]" ;   
			}else if(j==oQg){geotop::common::Variables::hpnt[j] = "Qsurface[-]" ;   
			}else if(j==oQa){geotop::common::Variables::hpnt[j] = "Qair[-]" ;   
			}else if(j==oQs){geotop::common::Variables::hpnt[j] = "Qcanopyair[-]" ;   
			}else if(j==oLobuk){geotop::common::Variables::hpnt[j] = "LObukhov[m]" ;
			}else if(j==oLobukcan){geotop::common::Variables::hpnt[j] = "LObukhovcanopy[m]" ;
			}else if(j==outop){geotop::common::Variables::hpnt[j] = "Wind_speed_top_canopy[m/s]" ;    
			}else if(j==odecay){geotop::common::Variables::hpnt[j] = "Decay_of_K_in_canopy[-]" ;   
			}else if(j==oSWup){geotop::common::Variables::hpnt[j] = "SWup[W/m2]" ;   
			}else if(j==oLWup){geotop::common::Variables::hpnt[j] = "LWup[W/m2]" ;   
			}else if(j==oHup){geotop::common::Variables::hpnt[j] = "Hup[W/m2]" ;    
			}else if(j==oLEup){geotop::common::Variables::hpnt[j] = "LEup[W/m2]" ;   
			}else if(j==osnowdepth){geotop::common::Variables::hpnt[j] = "snow_depth[mm]" ; 
			}else if(j==oSWE){geotop::common::Variables::hpnt[j] = "snow_water_equivalent[mm]" ; 
			}else if(j==osnowdens){geotop::common::Variables::hpnt[j] = "snow_density[kg/m3]" ; 
			}else if(j==osnowT){geotop::common::Variables::hpnt[j] = "snow_temperature[C]" ; 	
			}else if(j==omrsnow){geotop::common::Variables::hpnt[j] = "snow_melted[mm]" ; 
			}else if(j==osrsnow){geotop::common::Variables::hpnt[j] = "snow_subl[mm]" ; 
			}else if(j==oblowingsnowtrans){geotop::common::Variables::hpnt[j] = "snow_blown_away[mm]" ; 
			}else if(j==oblowingsnowsubl){geotop::common::Variables::hpnt[j] = "snow_subl_while_blown[mm]" ;			
			}else if(j==oglacdepth){geotop::common::Variables::hpnt[j] = "glac_depth[mm]" ; 
			}else if(j==oGWE){geotop::common::Variables::hpnt[j] = "glac_water_equivalent[mm]" ; 
			}else if(j==oglacdens){geotop::common::Variables::hpnt[j] = "glac_density[kg/m3]" ; 
			}else if(j==oglacT){geotop::common::Variables::hpnt[j] = "glac_temperature[C]" ; 
			}else if(j==omrglac){geotop::common::Variables::hpnt[j] = "glac_melted[mm]" ; 
			}else if(j==osrglac){geotop::common::Variables::hpnt[j] = "glac_subl[mm]" ; 
			}else if(j==othawedup){geotop::common::Variables::hpnt[j] = "lowest_thawed_soil_depth[mm]" ; 
			}else if(j==othaweddw){geotop::common::Variables::hpnt[j] = "highest_thawed_soil_depth[mm]" ; 
			}else if(j==owtableup){geotop::common::Variables::hpnt[j] = "lowest_water_table_depth[mm]" ; 
			}else if(j==owtabledw){geotop::common::Variables::hpnt[j] = "highest_water_table_depth[mm]" ; 
			} else {
                geotop::common::Variables::hpnt[j] = geotop::input::gStringNoValue ;
            }
		}
	}
	
	//HeaderBasinFile
	for (size_t j=0; j<ootot; j++) {
		if(geotop::common::Variables::hbsn[j] == geotop::input::gStringNoValue){
			if(j==oodate12){geotop::common::Variables::hbsn[j] = "Date12[DDMMYYYYhhmm]" ;
			}else if(j==ooJDfrom0){geotop::common::Variables::hbsn[j] = "JulianDayFromYear0[days]" ;   		
			}else if(j==oodaysfromstart){geotop::common::Variables::hbsn[j] = "TimeFromStart[days]" ;    
			}else if(j==ooperiod){geotop::common::Variables::hbsn[j] = "Simulation_Period" ;
			}else if(j==oorun){geotop::common::Variables::hbsn[j] = "Run" ;	
			}else if(j==ooprecrain){geotop::common::Variables::hbsn[j] = "Prain_below_canopy[mm]" ;     	
			}else if(j==ooprecsnow){geotop::common::Variables::hbsn[j] = "Psnow_below_canopy[mm]" ;     	
			}else if(j==oorainover){geotop::common::Variables::hbsn[j] = "Prain_above_canopy[mm]" ;     	
			}else if(j==oosnowover){geotop::common::Variables::hbsn[j] = "Prain_above_canopy[mm]" ; 
			}else if(j==oopnet){geotop::common::Variables::hbsn[j] = "Pnet[mm]" ; 			
			}else if(j==ooTa){geotop::common::Variables::hbsn[j] = "Tair[C]" ;     	
			}else if(j==ooTg){geotop::common::Variables::hbsn[j] = "Tsurface[C]" ;     	
			}else if(j==ooTv){geotop::common::Variables::hbsn[j] = "Tvegetation[C]" ;     	
			}else if(j==ooevapsur){geotop::common::Variables::hbsn[j] = "Evap_surface[mm]" ;     	
			}else if(j==ootrasp){geotop::common::Variables::hbsn[j] = "Transpiration_canopy[mm]" ;     	
			}else if(j==ooLE){geotop::common::Variables::hbsn[j] = "LE[W/m2]" ;     	
			}else if(j==ooH){geotop::common::Variables::hbsn[j] = "H[W/m2]" ;     	
			}else if(j==ooSW){geotop::common::Variables::hbsn[j] = "SW[W/m2]" ;     	
			}else if(j==ooLW){geotop::common::Variables::hbsn[j] = "LW[W/m2]" ;     	
			}else if(j==ooLEv){geotop::common::Variables::hbsn[j] = "LEv[W/m2]" ;     	
			}else if(j==ooHv){geotop::common::Variables::hbsn[j] = "Hv[W/m2]" ;     	
			}else if(j==ooSWv){geotop::common::Variables::hbsn[j] = "SWv[W/m2]" ;     	
			}else if(j==ooLWv){geotop::common::Variables::hbsn[j] = "LWv[W/m2]" ;     	
			}else if(j==ooSWin){geotop::common::Variables::hbsn[j] = "SWin[W/m2]" ;     	
			}else if(j==ooLWin){geotop::common::Variables::hbsn[j] = "LWin[W/m2]" ;     	
			}else if(j==oomasserror){geotop::common::Variables::hbsn[j] = "Mass_balance_error[mm]" ;     	
			}else if(j==ootimestep){geotop::common::Variables::hbsn[j] = "Mean_Time_Step[s]" ;     					
			}else {
                geotop::common::Variables::hbsn[j] = geotop::input::gStringNoValue;
            }
		}
	}
	
	//HeaderSnowFile
	for (size_t j=0; j<10; j++) {
		if(geotop::common::Variables::hsnw[j] == geotop::input::gStringNoValue){
			if(j==0){
                geotop::common::Variables::hsnw[j] = "Date12[DDMMYYYYhhmm]" ;
			}else if(j==1){
                geotop::common::Variables::hsnw[j] = "JulianDayFromYear0[days]" ;
			}else if(j==2){
                geotop::common::Variables::hsnw[j] = "TimeFromStart[days]" ;
			}else if(j==3){
                geotop::common::Variables::hsnw[j] = "Simulation_Period" ;
			}else if(j==4){
                geotop::common::Variables::hsnw[j] = "Run" ;
			}else if(j==5){
                geotop::common::Variables::hsnw[j] = "IDpoint" ;
			} else {
                geotop::common::Variables::hsnw[j] = geotop::input::gStringNoValue;
            }
		}
	}
	
	//HeaderGlacierFile
	for (size_t j=0; j<10; j++) {
		if(geotop::common::Variables::hglc[j] == geotop::input::gStringNoValue){
			if(j==0){ geotop::common::Variables::hglc[j] = "Date12[DDMMYYYYhhmm]" ;
			}else if(j==1){ geotop::common::Variables::hglc[j] = "JulianDayFromYear0[days]" ;
			}else if(j==2){ geotop::common::Variables::hglc[j] = "TimeFromStart[days]" ;
			}else if(j==3){ geotop::common::Variables::hglc[j] = "Simulation_Period" ;
			}else if(j==4){ geotop::common::Variables::hglc[j] = "Run" ;
			}else if(j==5){ geotop::common::Variables::hglc[j] = "IDpoint" ;
			}else if(j==6){ geotop::common::Variables::hglc[j] = "Temperature[C]" ;
			}else if(j==7){ geotop::common::Variables::hglc[j] = "wice[kg/m2]" ;
			}else if(j==8){ geotop::common::Variables::hglc[j] = "wliq[kg/m2]" ;
			}else if(j==9){ geotop::common::Variables::hglc[j] = "Dz[mm]" ;
			} else {
                geotop::common::Variables::hglc[j] = geotop::input::gStringNoValue ;
            }
		}
	}
	
	//HeaderGlacierFile
	for (size_t j=0; j<6; j++) {
		if(geotop::common::Variables::hsl[j] == geotop::input::gStringNoValue){
			if(j==0){ geotop::common::Variables::hsl[j] = "Date12[DDMMYYYYhhmm]" ;
			}else if(j==1){ geotop::common::Variables::hsl[j] = "JulianDayFromYear0[days]" ;
			}else if(j==2){ geotop::common::Variables::hsl[j] = "TimeFromStart[days]" ;
			}else if(j==3){ geotop::common::Variables::hsl[j] = "Simulation_Period" ;
			}else if(j==4){ geotop::common::Variables::hsl[j] = "Run" ;
			}else if(j==5){ geotop::common::Variables::hsl[j] = "IDpoint" ;
			} else {
                geotop::common::Variables::hsl[j] = geotop::input::gStringNoValue;
            }
		}	
	}	
	
	//Recovery Files
	for (size_t j=rpsi; j<=rsux; j++) {
		if(geotop::common::Variables::files[j] == geotop::input::gStringNoValue){
			if (j==rpsi) {geotop::common::Variables::files[j] = "SoilPressure";
			}else if (j==riceg) {geotop::common::Variables::files[j] = "SoilIceContent";
			}else if (j==rTg) {geotop::common::Variables::files[j] = "SoilTemperature";
			}else if (j==rDzs) {geotop::common::Variables::files[j] = "SnowThickness";
			}else if (j==rwls) {geotop::common::Variables::files[j] = "SnowLiqWaterContent";
			}else if (j==rwis) {geotop::common::Variables::files[j] = "SnowIceContent";
			}else if (j==rTs) {geotop::common::Variables::files[j] = "SnowTemperature";
			}else if (j==rDzi) {geotop::common::Variables::files[j] = "GlacThickness";
			}else if (j==rwli) {geotop::common::Variables::files[j] = "GlacLiqWaterContent";
			}else if (j==rwii) {geotop::common::Variables::files[j] = "GlacIceContent";
			}else if (j==rTi) {geotop::common::Variables::files[j] = "GlacTemperature";
			}else if (j==rns) {geotop::common::Variables::files[j] = "SnowLayersNumber";
			}else if (j==rni) {geotop::common::Variables::files[j] = "GlacLayersNumber";
			}else if (j==rsnag) {geotop::common::Variables::files[j] = "SnowAge";
			}else if (j==rwcrn) {geotop::common::Variables::files[j] = "RainOnCanopy";
			}else if (j==rwcsn) {geotop::common::Variables::files[j] = "SnowOnCanopy";
			}else if (j==rTv) {geotop::common::Variables::files[j] = "VegTemperature";
			}else if (j==rpsich) {geotop::common::Variables::files[j] = "SoilChannelPressure";
			}else if (j==ricegch) {geotop::common::Variables::files[j] = "SoilChannelIceContent";
			}else if (j==rTgch) {geotop::common::Variables::files[j] = "SoilChannelTemperature";
			}else if (j==rTrun) {geotop::common::Variables::files[j] = "RunMeanSoilTemperature";
			}else if (j==rwrun) {geotop::common::Variables::files[j] = "RunMeanSoilTotWater";
			}else if (j==rdUrun) {geotop::common::Variables::files[j] = "RunSoilInternalEnergy";
			}else if (j==rSWErun) {geotop::common::Variables::files[j] = "RunMeanSWE";
			}else if (j==rTmaxrun) {geotop::common::Variables::files[j] = "RunMaxSoilTemperature";
			}else if (j==rTminrun) {geotop::common::Variables::files[j] = "RunMinSoilTemperature";
			}else if (j==rwmaxrun) {geotop::common::Variables::files[j] = "RunMaxSoilTotWater";
			}else if (j==rwminrun) {geotop::common::Variables::files[j] = "RunMinSoilTotWater";
			}else if (j==rtime) {geotop::common::Variables::files[j] = "RecoveryTime";
			}else if (j==rsux) {geotop::common::Variables::files[j] = "SuccessfulRecovery";
			} else {
                geotop::common::Variables::files[j] = geotop::input::gStringNoValue ;
            }
		}
	}

	//add path to recovery files
	if(path_rec_files != geotop::input::gStringNoValue){
		temp = path_rec_files;
		path_rec_files = temp + std::string("/");
		for (size_t i=rpsi; i<=rsux; i++) {
			temp =geotop::common::Variables::files[i];
			geotop::common::Variables::files[i] = path_rec_files + temp;
		}
	}

	//add working path to the file name
	for (size_t i=0; i<nfiles; i++) {
		if (geotop::common::Variables::files[i] != geotop::input::gStringNoValue) {
			temp =geotop::common::Variables::files[i];
			geotop::common::Variables::files[i] = geotop::common::Variables::WORKING_DIRECTORY + std::string(temp);
		}
	}

	
	return 1;
	
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/
void assign_numeric_parameters(Par *par, Land *land, Times *times, Soil *sl, Meteo *met, InitTools *itools, FILE *flog)
{
	short occurring;
    size_t k, n, m, nsoillayers, nmeteo_stations, npoints;
	double a;
    double minDt=1.E99;

    std::vector<double> lDoubleTempVector ;

	fprintf(flog,"\n");
	
	par->print=0;

    boost::shared_ptr<geotop::input::ConfigStore> lConfigStore = geotop::input::ConfigStoreSingletonFactory::getInstance() ;

    n = (long)GTConst::max_cols_time_steps_file + 1;
    std::vector<double> lTimeStepEnergyAndWater = getDoubleVectorValueWithDefault(lConfigStore, "TimeStepEnergyAndWater", geotop::input::gDoubleNoValue, false, (long)GTConst::max_cols_time_steps_file, true);
    times->Dt_vector=(double *)malloc(n*sizeof(double));
    times->Dt_vector[0] = 0.;//it is the space for the date in case of time variable time step
    for (size_t i=1; i<n; i++) {
        times->Dt_vector[i] = lTimeStepEnergyAndWater[i-1] ;
        if((long)times->Dt_vector[i] != geotop::input::gDoubleNoValue){
            if (times->Dt_vector[i] < minDt) minDt = times->Dt_vector[i];
        }
    }

	//init date
        std::vector<double> lInitDateDDMMYYYYhhmm = getDoubleVectorValueWithDefault(lConfigStore, "InitDateDDMMYYYYhhmm", geotop::input::gDoubleNoValue, false, 0, false);
    par->init_date.resize(lInitDateDDMMYYYYhhmm.size() + 1 , 0);
	for (size_t i=1; i<par->init_date.size(); i++) {
		par->init_date[i] = lInitDateDDMMYYYYhhmm[i-1];
		par->init_date[i] = convert_dateeur12_JDfrom0(par->init_date[i]);
	}
	
	//simulation time
    par->simulation_hours = getDoubleValueWithDefault(lConfigStore, "SimulationHours", geotop::input::gDoubleNoValue, false) ;

	//end date
    std::vector<double> lEndDateDDMMYYYYhhmm = getDoubleVectorValueWithDefault(lConfigStore, "EndDateDDMMYYYYhhmm", geotop::input::gDoubleNoValue, false, 0, false);
    
    //	par->end_date = new_doublevector(num_param_components[cod]);
    par->end_date.resize(lEndDateDDMMYYYYhhmm.size() + 1, 0) ;
	if (par->end_date.size() != par->init_date.size()){
		fprintf(flog, "Error:: End date has a number of components different from Init Date");
		printf("Error:: End date has a number of components different from Init Date");
		t_error("Fatal Error! Geotop is closed. See failing report.");
	}
    
	for (size_t i=1; i<par->end_date.size(); i++) {
		par->end_date[i] = lEndDateDDMMYYYYhhmm[i-1];
		if ((long)par->end_date[i] == geotop::input::gDoubleNoValue){
			par->end_date[i] = par->init_date[i] + par->simulation_hours/24.;
		}else {
			par->end_date[i] = convert_dateeur12_JDfrom0(par->end_date[i]);
		}
	}

    par->run_times.resize(par->init_date.size() + 1, 0);

    std::vector<double> lNumSimulationTimes = getDoubleVectorValueWithDefault(lConfigStore, "NumSimulationTimes", 1., true, par->init_date.size(), false);
    size_t lNumSimulationTimesSize = lNumSimulationTimes.size() ;
	for (size_t i=1; i<par->run_times.size(); i++) {
		par->run_times[i] = lNumSimulationTimes[i-1] ;
	}

	par->ST = getDoubleValueWithDefault(lConfigStore, "StandardTimeSimulation", 0., false) ;

	par->Dtplot_discharge.resize(par->init_date.size() + 1, 0);

    std::vector<double> lDtPlotDischarge = getDoubleVectorValueWithDefault(lConfigStore, "TimeStepEnergyAndWater", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->Dtplot_discharge.size(); i++) {
		par->Dtplot_discharge[i] = lDtPlotDischarge[i-1] ;
	}
    
	par->plot_discharge_with_Dt_integration.resize(par->init_date.size() + 1, 0);
	par->state_discharge = 0;
	for (size_t i=1; i<par->init_date.size(); i++) {
		par->Dtplot_discharge[i] *= 3600.;
		if(par->Dtplot_discharge[i] > 1.E-5 && par->Dtplot_discharge[i] <= minDt){
			par->plot_discharge_with_Dt_integration[i]=1;
		}else{
			par->plot_discharge_with_Dt_integration[i]=0;
		}
		if(par->Dtplot_discharge[i] > 1.E-5) par->state_discharge = 1;
	}
	
	par->Dtplot_point.resize(par->init_date.size() + 1, 0);

    std::vector<double> lDtPlotPoint = getDoubleVectorValueWithDefault(lConfigStore, "DtPlotPoint", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->Dtplot_point.size(); i++) {
		par->Dtplot_point[i] = lDtPlotPoint[i-1];
	}

	par->plot_point_with_Dt_integration.resize(par->init_date.size() + 1, 0);
	par->state_pixel = 0;
	for (size_t i=1; i<par->init_date.size(); i++) {
		par->Dtplot_point[i] *= 3600.;
		if(par->Dtplot_point[i] > 1.E-5 && par->Dtplot_point[i] <= minDt){
			par->plot_point_with_Dt_integration[i]=1;
		}else{
			par->plot_point_with_Dt_integration[i]=0;
		}
		if(par->Dtplot_point[i] > 1.E-5) par->state_pixel = 1;
	}

    par->Dtplot_basin.resize(par->init_date.size() + 1, 0);

    std::vector<double> lDtPlotBasin = getDoubleVectorValueWithDefault(lConfigStore, "DtPlotBasin", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->Dtplot_basin.size(); i++) {
		par->Dtplot_basin[i] = lDtPlotBasin[i-1];
	}
    
	par->plot_basin_with_Dt_integration.resize(par->init_date.size() + 1, 0);
	par->state_basin = 0;
	for (size_t i=1; i<par->init_date.size(); i++) {
		par->Dtplot_basin[i] *= 3600.;
		if(par->Dtplot_basin[i] > 1.E-5 && par->Dtplot_basin[i] <= minDt){
			par->plot_basin_with_Dt_integration[i]=1;
		}else{
			par->plot_basin_with_Dt_integration[i]=0;
		}
		if(par->Dtplot_basin[i] > 1.E-5) par->state_basin = 1;
	}

    par->lowpass = (long)getDoubleValueWithDefault(lConfigStore, "NumLowPassFilterOnDemForAll", 0., false) ;
	par->lowpass_curvatures = (long)getDoubleValueWithDefault(lConfigStore, "NumLowPassFilterOnDemForCurv", 0., false) ;
	par->sky = (short)getDoubleValueWithDefault(lConfigStore, "FlagSkyViewFactor", 0., false) ;
	par->format_out = (short)getDoubleValueWithDefault(lConfigStore, "FormatOutputMaps", 3., false) ;
	par->point_sim = (short)getDoubleValueWithDefault(lConfigStore, "PointSim", 0., false) ;
	par->recover = (short)getDoubleValueWithDefault(lConfigStore, "RecoverSim", 0., false) ;
    
    //land cover types
	par->n_landuses = (long)getDoubleValueWithDefault(lConfigStore, "NumLandCoverTypes", 0., false) ;
	
	land->ty.resize(par->n_landuses + 1, nlandprop + 1, 0);

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilRoughness", 10., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jz0] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "ThresSnowSoilRough", land->ty[1][jz0], true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jz0thressoil] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "VegHeight", 1000., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jHveg] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "ThresSnowVegUp", land->ty[1][jHveg], true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jz0thresveg] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "ThresSnowVegDown", land->ty[1][jz0thresveg], true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jz0thresveg2] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "LSAI", 1., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jLSAI] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "CanopyFraction", 0., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jcf] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "DecayCoeffCanopy", 2.5, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jdecay0] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "VegSnowBurying", 1., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jexpveg] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "RootDepth", 300., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jroot] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MinStomatalRes", 60., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jrs] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "VegReflectVis", 0.2, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jvR_vis] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "VegReflNIR", 0.2, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jvR_nir] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "VegTransVis", 0.2, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jvT_vis] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "VegTransNIR", 0.2, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jvT_nir] = lDoubleTempVector[i-1];
    }
    
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "LeafAngles", 0., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jvCh] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "CanDensSurface", 2., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jcd] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilAlbVisDry", 0.2, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][ja_vis_dry] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilAlbNIRDry", land->ty[1][ja_vis_dry], true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][ja_nir_dry] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilAlbVisWet", land->ty[1][ja_vis_dry], true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][ja_vis_sat] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilAlbNIRWet", land->ty[1][ja_nir_dry], true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][ja_nir_sat] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilEmissiv", 0.96, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jemg] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SurFlowResLand", 0.5, true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jcm] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "RoughElemXUnitArea", 0., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jN] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "RoughElemDiam", 50., true, par->n_landuses, false) ;
	for (size_t i=1; i<land->ty.getNx(); i++) {
        land->ty[i][jdv] = lDoubleTempVector[i-1];
    }


	//former block 2
    
	par->imp = getDoubleValueWithDefault(lConfigStore, "FrozenSoilHydrCondReduction", geotop::input::gDoubleNoValue, false) ;
    par->free_drainage_bottom = getDoubleValueWithDefault(lConfigStore, "FreeDrainageAtBottom", geotop::input::gDoubleNoValue, false) ;
    par->free_drainage_lateral = getDoubleValueWithDefault(lConfigStore, "FreeDrainageAtLateralBorder", geotop::input::gDoubleNoValue, false) ;
    par->TolVWb = getDoubleValueWithDefault(lConfigStore, "RichardTol", geotop::input::gDoubleNoValue, false) ;

	par->RelTolVWb = GTConst::RelativeErrorRichards;
	par->MaxErrWb = 1.E99;

    par->MaxiterTol = (long) getDoubleValueWithDefault(lConfigStore, "RichardMaxIter", geotop::input::gDoubleNoValue, false) ;
	par->TolCG =  getDoubleValueWithDefault(lConfigStore, "RichardInitForc", geotop::input::gDoubleNoValue, false) ;
    par->min_lambda_wat =  getDoubleValueWithDefault(lConfigStore, "MinLambdaWater", geotop::input::gDoubleNoValue, false) ;
    par->max_times_min_lambda_wat = (long)getDoubleValueWithDefault(lConfigStore, "MaxTimesMinLambdaWater", geotop::input::gDoubleNoValue, false) ;
    par->exit_lambda_min_wat = (short)getDoubleValueWithDefault(lConfigStore, "ExitMinLambdaWater", geotop::input::gDoubleNoValue, false) ;
    par->min_Dt = getDoubleValueWithDefault(lConfigStore, "MinTimeStep", geotop::input::gDoubleNoValue, false) ;
    par->gamma_m = getDoubleValueWithDefault(lConfigStore, "SurFlowResExp", geotop::input::gDoubleNoValue, false) ;
    par->thres_hsup_1 = getDoubleValueWithDefault(lConfigStore, "ThresWaterDepthLandInf", geotop::input::gDoubleNoValue, false) ;
    par->thres_hsup_2 = getDoubleValueWithDefault(lConfigStore, "ThresWaterDepthLandSup", geotop::input::gDoubleNoValue, false) ;
    par->Ks_channel = getDoubleValueWithDefault(lConfigStore, "SurFlowResChannel", geotop::input::gDoubleNoValue, false) ;
    par->thres_hchannel = getDoubleValueWithDefault(lConfigStore, "ThresWaterDepthChannel", geotop::input::gDoubleNoValue, false) ;
    par->w_dx = getDoubleValueWithDefault(lConfigStore, "RatioChannelWidthPixelWidth", geotop::input::gDoubleNoValue, false) ;
    par->depr_channel = getDoubleValueWithDefault(lConfigStore, "ChannelDepression", geotop::input::gDoubleNoValue, false) ;
    par->max_courant_land = getDoubleValueWithDefault(lConfigStore, "MaxCourantSupFlowLand", geotop::input::gDoubleNoValue, false) ;
    par->max_courant_channel = getDoubleValueWithDefault(lConfigStore, "MaxCourantSupFlowChannel", geotop::input::gDoubleNoValue, false) ;
    par->min_hsup_land = getDoubleValueWithDefault(lConfigStore, "MinSupWaterDepthLand", geotop::input::gDoubleNoValue, false) ;
    par->min_hsup_channel = getDoubleValueWithDefault(lConfigStore, "MinSupWaterDepthChannel", geotop::input::gDoubleNoValue, false) ;
    par->min_dhsup_land_channel_in = getDoubleValueWithDefault(lConfigStore, "MinDiffSupWaterDepthLandChannel", geotop::input::gDoubleNoValue, false) ;
	par->dtmin_sup = getDoubleValueWithDefault(lConfigStore, "MinTimeStepSupFlow", geotop::input::gDoubleNoValue, false) ;

    //former block 3
    par->latitude = getDoubleValueWithDefault(lConfigStore, "Latitude", geotop::input::gDoubleNoValue, false) ;
    par->longitude = getDoubleValueWithDefault(lConfigStore, "Longitude", geotop::input::gDoubleNoValue, false) ;
    par->Vmin = getDoubleValueWithDefault(lConfigStore, "Vmin", geotop::input::gDoubleNoValue, false) ;
    par->RHmin = getDoubleValueWithDefault(lConfigStore, "RHmin", geotop::input::gDoubleNoValue, false) ;
    par->alpha_snow = getDoubleValueWithDefault(lConfigStore, "AlphaSnow", geotop::input::gDoubleNoValue, false) ;
    par->nsurface = (long)getDoubleValueWithDefault(lConfigStore, "HighestNodeCorrespondsToLayer", geotop::input::gDoubleNoValue, false) ;
    par->tol_energy = getDoubleValueWithDefault(lConfigStore, "HeatEqTol", geotop::input::gDoubleNoValue, false) ;
    par->maxiter_energy = (long)getDoubleValueWithDefault(lConfigStore, "HeatEqMaxIter", geotop::input::gDoubleNoValue, false) ;
    par->min_lambda_en = getDoubleValueWithDefault(lConfigStore, "MinLambdaEnergy", geotop::input::gDoubleNoValue, false) ;
    par->max_times_min_lambda_en = (long)getDoubleValueWithDefault(lConfigStore, "MaxTimesMinLambdaEnergy", geotop::input::gDoubleNoValue, false) ;
    par->exit_lambda_min_en = (short)getDoubleValueWithDefault(lConfigStore, "ExitMinLambdaEnergy", geotop::input::gDoubleNoValue, false) ;
    par->dem_rotation = getDoubleValueWithDefault(lConfigStore, "DEMRotationAngle", geotop::input::gDoubleNoValue, false) ;
    par->maxiter_canopy = (long)getDoubleValueWithDefault(lConfigStore, "CanopyMaxIter", geotop::input::gDoubleNoValue, false) ;
    par->maxiter_Businger = (long)getDoubleValueWithDefault(lConfigStore, "BusingerMaxIter", geotop::input::gDoubleNoValue, false) ;
    par->maxiter_Ts = (long)getDoubleValueWithDefault(lConfigStore, "TsMaxIter", geotop::input::gDoubleNoValue, false) ;
    par->maxiter_Loc = (long)getDoubleValueWithDefault(lConfigStore, "LocMaxIter", geotop::input::gDoubleNoValue, false) ;
    par->stabcorr_incanopy = (short)getDoubleValueWithDefault(lConfigStore, "CanopyStabCorrection", geotop::input::gDoubleNoValue, false) ;
    par->iobsint = (short)getDoubleValueWithDefault(lConfigStore, "Iobsint", geotop::input::gDoubleNoValue, false) ;
    par->dn = getDoubleValueWithDefault(lConfigStore, "Dn", geotop::input::gDoubleNoValue, false) ;
    par->slopewt = getDoubleValueWithDefault(lConfigStore, "SlopeWeight", geotop::input::gDoubleNoValue, false) ;
    par->curvewt = getDoubleValueWithDefault(lConfigStore, "CurvatureWeight", geotop::input::gDoubleNoValue, false) ;
    par->slopewtD = getDoubleValueWithDefault(lConfigStore, "SlopeWeightD", geotop::input::gDoubleNoValue, false) ;
    par->curvewtD = getDoubleValueWithDefault(lConfigStore, "CurvatureWeightD", geotop::input::gDoubleNoValue, false) ;
    par->slopewtI = getDoubleValueWithDefault(lConfigStore, "SlopeWeightI", geotop::input::gDoubleNoValue, false) ;
    par->curvewtI = getDoubleValueWithDefault(lConfigStore, "CurvatureWeightI", geotop::input::gDoubleNoValue, false) ;
    par->Zboundary = getDoubleValueWithDefault(lConfigStore, "ZeroTempAmplitDepth", geotop::input::gDoubleNoValue, false) ;
    par->Tboundary = getDoubleValueWithDefault(lConfigStore, "ZeroTempAmplitTemp", geotop::input::gDoubleNoValue, false) ;
	par->Fboundary = getDoubleValueWithDefault(lConfigStore, "BottomBoundaryHeatFlux", geotop::input::gDoubleNoValue, false) ;

    //former block 4
    itools->swe0 = getDoubleValueWithDefault(lConfigStore, "InitSWE", geotop::input::gDoubleNoValue, false) ; ;
    itools->rhosnow0 = getDoubleValueWithDefault(lConfigStore, "InitSnowDensity", geotop::input::gDoubleNoValue, false) ; ;
    itools->Tsnow0 = getDoubleValueWithDefault(lConfigStore, "InitSnowTemp", geotop::input::gDoubleNoValue, false) ; ;
    itools->agesnow0 = getDoubleValueWithDefault(lConfigStore, "InitSnowAge", geotop::input::gDoubleNoValue, false) ; ;
    par->T_rain = getDoubleValueWithDefault(lConfigStore, "ThresTempRain", geotop::input::gDoubleNoValue, false) ; ;
    par->T_snow = getDoubleValueWithDefault(lConfigStore, "ThresTempSnow", geotop::input::gDoubleNoValue, false) ; ;
    par->dew = (short)getDoubleValueWithDefault(lConfigStore, "DewTempOrNormTemp", geotop::input::gDoubleNoValue, false) ; ;
    par->aep = getDoubleValueWithDefault(lConfigStore, "AlbExtParSnow", geotop::input::gDoubleNoValue, false) ; ;
    par->avo = getDoubleValueWithDefault(lConfigStore, "FreshSnowReflVis", geotop::input::gDoubleNoValue, false) ; ;
    par->airo = getDoubleValueWithDefault(lConfigStore, "FreshSnowReflNIR", geotop::input::gDoubleNoValue, false) ; ;
    par->Sr = getDoubleValueWithDefault(lConfigStore, "IrriducibleWatSatSnow", geotop::input::gDoubleNoValue, false) ; ;
    par->epsilon_snow = getDoubleValueWithDefault(lConfigStore, "SnowEmissiv", geotop::input::gDoubleNoValue, false) ; ;
    par->z0_snow = 0.001*getDoubleValueWithDefault(lConfigStore, "SnowRoughness", geotop::input::gDoubleNoValue, false) ; ;
    par->snowcorrfact = getDoubleValueWithDefault(lConfigStore, "SnowCorrFactor", geotop::input::gDoubleNoValue, false) ; ;
    par->raincorrfact = getDoubleValueWithDefault(lConfigStore, "RainCorrFactor", geotop::input::gDoubleNoValue, false) ; ;
    par->snow_maxpor = getDoubleValueWithDefault(lConfigStore, "MaxSnowPorosity", geotop::input::gDoubleNoValue, false) ; ;
	par->drysnowdef_rate = getDoubleValueWithDefault(lConfigStore, "DrySnowDefRate", geotop::input::gDoubleNoValue, false) ; ;
    par->snow_density_cutoff = getDoubleValueWithDefault(lConfigStore, "SnowDensityCutoff", geotop::input::gDoubleNoValue, false) ; ;
    par->wetsnowdef_rate = getDoubleValueWithDefault(lConfigStore, "WetSnowDefRate", geotop::input::gDoubleNoValue, false) ; ;
    par->snow_viscosity = getDoubleValueWithDefault(lConfigStore, "SnowViscosity", geotop::input::gDoubleNoValue, false) ; ;
    par->fetch_up = getDoubleValueWithDefault(lConfigStore, "FetchUp", geotop::input::gDoubleNoValue, false) ; ;
    par->fetch_down = getDoubleValueWithDefault(lConfigStore, "FetchDown", geotop::input::gDoubleNoValue, false) ; ;
    par->Wice_PBSM = getDoubleValueWithDefault(lConfigStore, "BlowingSnowSoftLayerIceContent", geotop::input::gDoubleNoValue, false) ; ;
    par->Dt_PBSM = getDoubleValueWithDefault(lConfigStore, "TimeStepBlowingSnow", geotop::input::gDoubleNoValue, false) ; ;
    par->snow_smin = getDoubleValueWithDefault(lConfigStore, "SnowSMIN", geotop::input::gDoubleNoValue, false) ; ;
    par->snow_smax = getDoubleValueWithDefault(lConfigStore, "SnowSMAX", geotop::input::gDoubleNoValue, false) ; ;
	par->snow_curv = getDoubleValueWithDefault(lConfigStore, "SnowCURV", geotop::input::gDoubleNoValue, false) ; ;
	
	//former blocks 5/6
	par->max_weq_snow = getDoubleValueWithDefault(lConfigStore, "MaxWaterEqSnowLayerContent", geotop::input::gDoubleNoValue, false) ; ;
	par->max_snow_layers = (long)getDoubleValueWithDefault(lConfigStore, "MaxSnowLayersMiddle", geotop::input::gDoubleNoValue, false) ; ;

	n = par->max_snow_layers ;
	if(n < 1){
		fprintf(flog,"Error:: MaxSnowLayersMiddle must be 1 or larger\n");
		printf("Error:: MaxSnowLayersMiddle must be 1 or larger\n");
		t_error("Fatal Error! Geotop is closed.");
	}

	par->SWE_bottom = getDoubleValueWithDefault(lConfigStore, "SWEbottom", geotop::input::gDoubleNoValue, false) ;
	par->SWE_top = getDoubleValueWithDefault(lConfigStore, "SWEtop", geotop::input::gDoubleNoValue, false) ;
    
	par->max_snow_layers = (long)floor(par->SWE_bottom/par->max_weq_snow) + (long)floor(par->SWE_top/par->max_weq_snow) + n;
	par->inf_snow_layers.resize(n + 1, 0);
	fprintf(flog,"Max snow layer number: %ld, of which %.0f at the bottom, %ld in the middle, and %.0f at the top.\n",par->max_snow_layers,floor(par->SWE_bottom/par->max_weq_snow),n,floor(par->SWE_top/par->max_weq_snow));
	fprintf(flog,"Infinite Snow layer numbers are numbers: ");
	for (size_t i=1; i<=n; i++) {
		par->inf_snow_layers[i] = (long)floor(par->SWE_bottom/par->max_weq_snow) + i;
		fprintf(flog, "%ld ",par->inf_snow_layers[i]);
	}
	fprintf(flog,"\n");
	
	//former block 7
    itools->Dglac0 = getDoubleValueWithDefault(lConfigStore, "InitGlacierDepth", geotop::input::gDoubleNoValue, false) ;
    itools->rhoglac0 = getDoubleValueWithDefault(lConfigStore, "InitGlacierDensity", geotop::input::gDoubleNoValue, false) ;
    itools->Tglac0 = getDoubleValueWithDefault(lConfigStore, "InitGlacierTemp", geotop::input::gDoubleNoValue, false) ;
    par->Sr_glac = getDoubleValueWithDefault(lConfigStore, "IrriducibleWatSatGlacier", geotop::input::gDoubleNoValue, false) ;

    //former block 8
    par->max_weq_glac = getDoubleValueWithDefault(lConfigStore, "MaxWaterEqGlacLayerContent", geotop::input::gDoubleNoValue, false) ;
    n = (long)getDoubleValueWithDefault(lConfigStore, "MaxGlacLayersMiddle", geotop::input::gDoubleNoValue, false) ;

    par->GWE_bottom = getDoubleValueWithDefault(lConfigStore, "GWEbottom", geotop::input::gDoubleNoValue, false) ;
	par->GWE_top = getDoubleValueWithDefault(lConfigStore, "GWEtop", geotop::input::gDoubleNoValue, false) ;

	if(n < 1 && (par->GWE_bottom > 0 || par->GWE_top > 0)){
		fprintf(flog,"Error:: MaxGlacLayersMiddle must be 1 or larger\n");
		printf("Error:: MaxGlacLayersMiddle must be 1 or larger\n");
		t_error("Fatal Error! Geotop is closed.");	
	}

	par->max_glac_layers = (long)floor(par->GWE_bottom/par->max_weq_glac) + (long)floor(par->GWE_top/par->max_weq_glac) + n;
	par->inf_glac_layers.resize(n + 1, 0);
	fprintf(flog,"Max glac layer number: %ld, of which %.0f at the bottom, %ld in the middle, and %.0f at the top.\n",par->max_glac_layers,floor(par->GWE_bottom/par->max_weq_glac),n,floor(par->GWE_top/par->max_weq_glac));
	fprintf(flog,"Infinite Glac layer numbers are numbers: ");
	for (size_t i=1; i<=n; i++) {
		par->inf_glac_layers[i] = (long)floor(par->GWE_bottom/par->max_weq_glac) + i;
		fprintf(flog, "%ld ",par->inf_glac_layers[i]);
	}
	fprintf(flog,"\n");

    par->state_turb = 1;
    
	//former block 9
    par->state_lwrad = (short)getDoubleValueWithDefault(lConfigStore, "LWinParameterization", geotop::input::gDoubleNoValue, false) ;
    par->monin_obukhov = (short)getDoubleValueWithDefault(lConfigStore, "MoninObukhov", geotop::input::gDoubleNoValue, false) ;
    par->surroundings = (short)getDoubleValueWithDefault(lConfigStore, "Surroundings", geotop::input::gDoubleNoValue, false) ;

    //distributed option file
    par->wat_balance = (short)getDoubleValueWithDefault(lConfigStore, "WaterBalance", geotop::input::gDoubleNoValue, false) ;
    par->en_balance = (short)getDoubleValueWithDefault(lConfigStore, "EnergyBalance", geotop::input::gDoubleNoValue, false) ;
    par->blowing_snow = (short)getDoubleValueWithDefault(lConfigStore, "BlowingSnow", geotop::input::gDoubleNoValue, false) ;
	par->Wmin_BS = getDoubleValueWithDefault(lConfigStore, "MinIceContentForBlowingSnow", geotop::input::gDoubleNoValue, false) ;

    std::vector<std::string> lKeywordString ;
    lKeywordString += "PointID", "CoordinatePointX", "CoordinatePointY" ;
	if (par->point_sim == 1) {
        lKeywordString += "PointElevation", "PointLandCoverType", "PointSoilType",
        "PointSlope", "PointAspect", "PointSkyViewFactor", "PointCurvatureNorthSouthDirection", "PointCurvatureWestEastDirection",
        "PointCurvatureNorthwestSoutheastDirection", "PointCurvatureNortheastSouthwestDirection",
        "PointDepthFreeSurface", "PointHorizon", "PointMaxSWE", "PointLatitude", "PointLongitude" ;
	}

    npoints = 0 ;
    for (size_t j=1; j<par->chkpt.getCols(); j++) {
        lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, lKeywordString[j-1], geotop::input::gDoubleNoValue, false, 0, true) ;
		if (npoints < lDoubleTempVector.size()) {
            npoints = lDoubleTempVector.size();
        }
        
	}
    par->chkpt.resize(npoints + 1, lKeywordString.size() + 1, 0);

    for (size_t j=1; j<par->chkpt.getCols(); j++) {
        lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, lKeywordString[j-1], geotop::input::gDoubleNoValue, false, 0, true) ;
        for (size_t i=1; i<par->chkpt.getRows(); i++) {
			par->chkpt[i][j] = lDoubleTempVector[i-1] ;
		}
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SavingPoints", geotop::input::gDoubleNoValue, false, 0, false) ;
	par->saving_points.resize(lDoubleTempVector.size() + 1, 0);
	for (size_t i=1; i<par->saving_points.size(); i++) {
		par->saving_points[i] = lDoubleTempVector[i-1];
	}
			
	par->output_soil.resize(par->init_date.size() + 1, 0);
	par->output_snow.resize(par->init_date.size() + 1, 0);
	par->output_glac.resize(par->init_date.size() + 1, 0);
	par->output_surfenergy.resize(par->init_date.size() + 1, 0);
	par->output_vegetation.resize(par->init_date.size() + 1, 0);
	par->output_meteo.resize(par->init_date.size() + 1, 0);

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "OutputSoilMaps", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->output_soil.size(); i++) {
		par->output_soil[i] = lDoubleTempVector[i-1] ;
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "OutputSnowMaps", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->output_snow.size(); i++) {
		par->output_snow[i] = lDoubleTempVector[i-1] ;
	}
    
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "OutputGlacierMaps", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->output_glac.size(); i++) {
		par->output_glac[i] = lDoubleTempVector[i-1] ;
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "OutputSurfEBALMaps", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->output_surfenergy.size(); i++) {
		par->output_surfenergy[i] = lDoubleTempVector[i-1] ;
	}
	
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "OutputVegetationMaps", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->output_vegetation.size(); i++) {
		par->output_vegetation[i] = lDoubleTempVector[i-1] ;
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "OutputMeteoMaps", 0., true, par->init_date.size(), false) ;
	for (size_t i=1; i<par->output_meteo.size(); i++) {
		par->output_meteo[i] = lDoubleTempVector[i-1] ;
	}
    
	par->output_soil_bin = 0;
	par->output_snow_bin = 0;
	par->output_glac_bin = 0;
	par->output_surfenergy_bin = 0;
	par->output_meteo_bin = 0;

	for (size_t i=1; i<par->init_date.size(); i++) {
		if (par->output_soil[i] > 0) par->output_soil_bin = 1;		
		if (par->output_snow[i] > 0) par->output_snow_bin = 1;
		if (par->output_glac[i] > 0) par->output_glac_bin = 1;
		if (par->output_surfenergy[i] > 0) par->output_surfenergy_bin = 1;
		if (par->output_meteo[i] > 0) par->output_meteo_bin = 1;
	}

    std::vector<double> lSpecialPlotBegin = getDoubleVectorValueWithDefault(lConfigStore, "SpecialPlotBegin", geotop::input::gDoubleNoValue, false, 0, false) ;
    std::vector<double> lSpecialPlotEnd = getDoubleVectorValueWithDefault(lConfigStore, "SpecialPlotEnd", geotop::input::gDoubleNoValue, false, 0, false) ;

    if(lSpecialPlotBegin.size() != lSpecialPlotEnd.size()){
		fprintf(flog, "Error:: Number of components of parameters SpecialPlotBegin and SpecialPlotEnd must be equal\n");
		printf("Error:: Number of components of parameters SpecialPlotBegin and SpecialPlotEnd must be equal\n");
		t_error("Fatal Error! Geotop is closed. See failing report.");
	}

    times->JD_plots.resize(lSpecialPlotBegin.size() + lSpecialPlotEnd.size() + 1, 0) ;

    for (size_t i=1; i<(size_t)(times->JD_plots.size()/2.); i++) {
		times->JD_plots[2*i-1] = lSpecialPlotBegin[i-1];
		times->JD_plots[2*i  ] = lSpecialPlotEnd[i-1];
	}
	if (times->JD_plots.size() == 3 && times->JD_plots[1] < 1.E-5 && times->JD_plots[2] < 1.E-5) {
        //		free_doublevector(times->JD_plots);
		times->JD_plots.resize(1 + 1, 0) ;
	}
	if (times->JD_plots.size() > 2) {
		for (size_t i=1; i<times->JD_plots.size(); i++) {
            
			times->JD_plots[i] = convert_dateeur12_JDfrom0(times->JD_plots[i]);
		}
	}

	//initial condition on the water pressure
	par->nsoiltypes = (long)getDoubleValueWithDefault(lConfigStore, "SoilLayerTypes", geotop::input::gDoubleNoValue, false) ; ;
    if (par->nsoiltypes < 1)
        par->nsoiltypes = 1;

	itools->init_water_table_depth.resize(par->nsoiltypes + 1, 0);

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "InitWaterTableDepth", 0., true, par->nsoiltypes, false) ;
	for (size_t i=1; i< itools->init_water_table_depth.size(); i++) {
		itools->init_water_table_depth[i] = lDoubleTempVector[i-1] ;
	}

    //soil properties and discretization
	par->soil_type_land_default = (long)getDoubleValueWithDefault(lConfigStore, "DefaultSoilTypeLand", geotop::input::gDoubleNoValue, false) ;

	if(par->soil_type_land_default<1 || par->soil_type_land_default>par->nsoiltypes){
		fprintf(flog, "Error:: Soil_type_land_default lower than 0 or higher than soil types numbers");
		printf("Error:: Soil_type_land_default lower than 0 or higher than soil types numbers");
		t_error("Fatal Error! Geotop is closed. See failing report.");	
	}

	par->soil_type_chan_default = (long)getDoubleValueWithDefault(lConfigStore, "DefaultSoilTypeChannel", geotop::input::gDoubleNoValue, false) ;

	if(par->soil_type_chan_default<1 || par->soil_type_chan_default>par->nsoiltypes){
		fprintf(flog, "Error:  Soil_type_chan_default lower than 0 or higher than soil types numbers");
		printf("Error:  Soil_type_chan_default lower than 0 or higher than soil types numbers");
		t_error("Fatal Error! Geotop is closed. See failing report.");	
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilLayerThicknesses", geotop::input::gDoubleNoValue, true, 0, true) ;
    a = lDoubleTempVector[0] ;
    
	//there is a specific layer discretization
	if ( (long)a != geotop::input::gDoubleNoValue && lDoubleTempVector.size() > 1) {
		
		nsoillayers = lDoubleTempVector.size();
		sl->pa.resize(par->nsoiltypes + 1, nsoilprop + 1, nsoillayers + 1) ;
		
        lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilLayerThicknesses", a, true, nsoillayers, true) ;
		for (size_t i=1; i<sl->pa.getCh(); i++) {
			sl->pa[1][jdz][i] = lDoubleTempVector[i-1] ;
		}
        
	}else {
		
		if((long)a==geotop::input::gDoubleNoValue) a=100.;
		nsoillayers = (long)getDoubleValueWithDefault(lConfigStore, "SoilLayerNumber", geotop::input::gDoubleNoValue, false) ;
		
        sl->pa.resize(par->nsoiltypes + 1, nsoilprop + 1, nsoillayers + 1);
        
		for (size_t i=1; i<sl->pa.getCh(); i++) {
			sl->pa[1][jdz][i] = a;
		}
        
	}
	
	//first layer
	size_t lStartIndex = 1;
    sl->pa[1][jpsi][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "InitSoilPressure", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jT][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "InitSoilTemp", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jKn][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "NormalHydrConductivity", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jKl][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "LateralHydrConductivity", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jres][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "ThetaRes", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jwp][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "WiltingPoint", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jfc][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "FieldCapacity", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jsat][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "ThetaSat", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][ja][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "AlphaVanGenuchten", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jns][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "NVanGenuchten", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jv][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "VMualem", geotop::input::gDoubleNoValue, false, 0, false)[lStartIndex-1] ;
	sl->pa[1][jkt][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "ThermalConductivitySoilSolids", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jct][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "ThermalCapacitySoilSolids", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;
	sl->pa[1][jss][lStartIndex] = getDoubleVectorValueWithDefault(lConfigStore, "SpecificStorativity", geotop::input::gDoubleNoValue, false, 0, true)[lStartIndex-1] ;

    //other layers
    lKeywordString.clear() ;
    lKeywordString += "InitSoilPressure", "InitSoilTemp", "NormalHydrConductivity", "LateralHydrConductivity",
        "ThetaRes", "WiltingPoint", "FieldCapacity", "ThetaSat", "AlphaVanGenuchten", "NVanGenuchten",
        "VMualem", "ThermalConductivitySoilSolids", "ThermalCapacitySoilSolids", "SpecificStorativity" ;

    for (size_t j=2; j<sl->pa.getRh(); j++) {
        lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, lKeywordString[j-2], sl->pa(1,j,1), true, sl->pa.getCh(), true) ;
        for (size_t i=2; i<sl->pa.getCh(); i++) {
            sl->pa(1,j,i) = lDoubleTempVector[i-1];
		}
	}

	//field capacity (-0.333 bar) and wilting point (-15 bar)
	for (size_t i=1; i<sl->pa.getCh(); i++){
		if( (long)sl->pa(1,jfc,i) == geotop::input::gDoubleNoValue){
			sl->pa[1][jfc][i] = teta_psi( (-1./3.)*1.E5/GTConst::GRAVITY, 0., sl->pa(1,jsat,i), sl->pa(1,jres,i), sl->pa(1,ja,i),
										 sl->pa(1,jns,i), 1.-1./sl->pa(1,jns,i), GTConst::PsiMin, sl->pa(1,jss,i));
		}
		
		if( (long)sl->pa(1,jwp,i) == geotop::input::gDoubleNoValue){
			sl->pa(1,jwp,i) = teta_psi( -15.*1.E5/GTConst::GRAVITY, 0., sl->pa(1,jsat,i), sl->pa(1,jres,i), sl->pa(1,ja,i),
                                       sl->pa(1,jns,i), 1.-1./sl->pa(1,jns,i), GTConst::PsiMin, sl->pa(1,jss,i));
		}
	}
	
	//other soil types
    for (k=2; k<=par->nsoiltypes; k++) {
        for (size_t i=1; i<sl->pa.getCh(); i++) {
            for (size_t j=1; j<sl->pa.getRh(); j++) {
                sl->pa(k,j,i) = sl->pa(1,j,i);
            }
        }
    }
	
	//use water table for water pressure
    for (k=1; k<=par->nsoiltypes; k++) {
        occurring = 0;//check if psi initial has at least one novalue
        /*for (i=1; i<=sl->pa->nch; i++) {
         if ( (long)sl->pa->co[k][jpsi][i] == geotop::input::gDoubleNoValue) occurring = 1;*/
        for (size_t i=1; i<sl->pa.getCh(); i++) {
            if ( (long)sl->pa(k,jpsi,i) == geotop::input::gDoubleNoValue) occurring = 1;
        }
        //      if (occurring == 0) sl->init_water_table_depth->co[k] = geotop::input::gDoubleNoValue;
        if (occurring == 0) sl->init_water_table_depth[k] = geotop::input::gDoubleNoValue;
    }

	itools->pa_bed.resize(1 + 1, nsoilprop + 1, nsoillayers + 1);
	for (size_t i=1; i<=nsoillayers; i++) {
		itools->pa_bed(1,jdz,i) = sl->pa(1,jdz,i);
	}
    
    //other layers
    lKeywordString.clear() ;
    lKeywordString += "InitSoilPressureBedrock", "InitSoilTempBedrock", "NormalHydrConductivityBedrock",
        "LateralHydrConductivityBedrock", "ThetaResBedrock", "WiltingPointBedrock",
        "FieldCapacityBedrock", "ThetaSatBedrock", "AlphaVanGenuchtenBedrock", "NVanGenuchtenBedrock",
        "VMualemBedrock", "ThermalConductivitySoilSolidsBedrock", "ThermalCapacitySoilSolidsBedrock",
        "SpecificStorativityBedrock" ;

    for (size_t j=1; j<itools->pa_bed.getCh(); j++) {
        lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, lKeywordString[j-1], geotop::input::gDoubleNoValue, true, nsoillayers, true) ;
        for (size_t i=1; i<itools->pa_bed.getCh(); i++) {
			if(j != jdz)
                itools->pa_bed(1,j,i) = lDoubleTempVector[i-1] ;
		}
	}
    
	//field capacity (-0.333 bar) and wilting point (-15 bar)
	for (size_t i=1; i<sl->pa.getCh(); i++){
		if( (long)itools->pa_bed(1,jsat,i) != geotop::input::gDoubleNoValue && (long)itools->pa_bed(1,jres,i) != geotop::input::gDoubleNoValue &&
			(long)itools->pa_bed(1,ja,i) != geotop::input::gDoubleNoValue && (long)itools->pa_bed(1,jns,i) != geotop::input::gDoubleNoValue &&
			(long)itools->pa_bed(1,jss,i) ) {
			if ( (long)itools->pa_bed(1,jfc,i) == geotop::input::gDoubleNoValue) {
				itools->pa_bed(1,jfc,i) = teta_psi( (-1./3.)*1.E5/GTConst::GRAVITY, 0., itools->pa_bed[1][jsat][i], itools->pa_bed(1,jres,i), itools->pa_bed(1,ja,i),
										 itools->pa_bed(1,jns,i), 1.-1./itools->pa_bed(1,jns,i), GTConst::PsiMin, itools->pa_bed(1,jss,i));
			}
			if ( (long)itools->pa_bed(1,jwp,i) == geotop::input::gDoubleNoValue) {
				itools->pa_bed(1,jwp,i) = teta_psi( -15.*1.E5/GTConst::GRAVITY, 0., itools->pa_bed(1,jsat,i), itools->pa_bed(1,jres,i), itools->pa_bed(1,ja,i),
										 itools->pa_bed(1,jns,i), 1.-1./itools->pa_bed(1,jns,i), GTConst::PsiMin, itools->pa_bed(1,jss,i));
			}
		}	
	}

	//meteo stations
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationsID", geotop::input::gDoubleNoValue, false, 0, true) ;
	met->imeteo_stations.resize(lDoubleTempVector.size() + 1);
    met->imeteo_stations[1] = lDoubleTempVector[0] ;
	if ( met->imeteo_stations[1] != geotop::input::gDoubleNoValue ) {
		for (size_t i=2; i < met->imeteo_stations.size() ; i++) {
            double lValue = lDoubleTempVector[i-1];
            if (lValue == geotop::input::gDoubleNoValue)
                lValue = 0. ;
			met->imeteo_stations[i] = (long)lValue;
		}
		nmeteo_stations = lDoubleTempVector.size();
	}else {
		nmeteo_stations = (long) getDoubleValueWithDefault(lConfigStore, "NumberOfMeteoStations", geotop::input::gDoubleNoValue, false);
	}

    //	met->st=(METEO_STATIONS *)malloc(sizeof(METEO_STATIONS));
    met->st = new MeteoStations();
    size_t lMeteoStationContainerSize = nmeteo_stations+1 ;
    if(!met->st) t_error("meteo_stations was not allocated");
    met->st->E.resize(lMeteoStationContainerSize);
    met->st->N.resize(lMeteoStationContainerSize);
    met->st->lat.resize(lMeteoStationContainerSize);
    met->st->lon.resize(lMeteoStationContainerSize);
    met->st->Z.resize(lMeteoStationContainerSize);
    met->st->sky.resize(lMeteoStationContainerSize);
    met->st->ST.resize(lMeteoStationContainerSize);
    met->st->Vheight.resize(lMeteoStationContainerSize);
    met->st->Theight.resize(lMeteoStationContainerSize);
    
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationCoordinateX", geotop::input::gDoubleNoValue, true, nmeteo_stations, false);
    for(size_t i=1 ; i < lMeteoStationContainerSize ; i++) {
        met->st->E[i] = lDoubleTempVector[i-1];
    }
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationCoordinateY", geotop::input::gDoubleNoValue, true, nmeteo_stations, false);
    for(size_t i=1 ; i < lMeteoStationContainerSize ; i++) {
        met->st->N[i] = lDoubleTempVector[i-1];
    }
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationLatitude", par->latitude, true, nmeteo_stations, false);
    for(size_t i=1 ; i < lMeteoStationContainerSize ; i++) {
        met->st->lat[i] = lDoubleTempVector[i-1];
    }
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationLongitude", par->longitude, true, nmeteo_stations, false);
    for(size_t i=1 ; i < lMeteoStationContainerSize ; i++) {
        met->st->lon[i] = lDoubleTempVector[i-1];
    }
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationElevation", 0., true, nmeteo_stations, false);
    for(size_t i=1 ; i < lMeteoStationContainerSize ; i++) {
        met->st->Z[i] = lDoubleTempVector[i-1];
    }
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationSkyViewFactor", 1., true, nmeteo_stations, false);
    for(size_t i=1 ; i < lMeteoStationContainerSize ; i++) {
        met->st->sky[i] = lDoubleTempVector[i-1];
    }
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationStandardTime", par->ST, true, nmeteo_stations, false);
    for(size_t i=1 ; i < lMeteoStationContainerSize ; i++) {
        met->st->ST[i] = lDoubleTempVector[i-1];
    }

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationWindVelocitySensorHeight", geotop::input::gDoubleNoValue, true, nmeteo_stations, false) ;
	for (size_t i=1; i<lMeteoStationContainerSize; i++) {
		met->st->Vheight[i] = lDoubleTempVector[i-1];
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "MeteoStationTemperatureSensorHeight", geotop::input::gDoubleNoValue, true, nmeteo_stations, false) ;
	for (size_t i=1; i<lMeteoStationContainerSize; i++) {
		met->st->Theight[i] = lDoubleTempVector[i-1];
	}

	//lapse rates (cyclic)
	n = (long)nlstot;
	met->LRcnc = (long*)malloc(n*sizeof(long));
	met->LRcnc[ilsDate12] = 1;
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "LapseRateTemp", geotop::input::gDoubleNoValue, false, 0, false) ;
	met->LRcnc[ilsTa] = lDoubleTempVector.size() ;
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "LapseRateDewTemp", geotop::input::gDoubleNoValue, false, 0, true) ;
	met->LRcnc[ilsTdew] = lDoubleTempVector.size() ;
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "LapseRatePrec", geotop::input::gDoubleNoValue, false, 0, false) ;
	met->LRcnc[ilsPrec] = lDoubleTempVector.size() ;
	met->LRc = (double**)malloc(n*sizeof(double*));

    size_t lColumnIndexJ ;
    std::vector<double> lLapseRateTemp = getDoubleVectorValueWithDefault(lConfigStore, "LapseRateTemp", geotop::input::gDoubleNoValue, false, 0, true) ;
    std::vector<double> lLapseRateDewTemp = getDoubleVectorValueWithDefault(lConfigStore, "LapseRateDewTemp", geotop::input::gDoubleNoValue, false, 0, true) ;
    std::vector<double> lLapseRatePrec = getDoubleVectorValueWithDefault(lConfigStore, "LapseRatePrec", geotop::input::gDoubleNoValue, false, 0, true) ;
	for (size_t i=0; i<nlstot; i++) {
		met->LRc[i] = (double*)malloc(met->LRcnc[i]*sizeof(double));
		for (lColumnIndexJ=0; lColumnIndexJ<met->LRcnc[i]; lColumnIndexJ++) {
			if(i==ilsDate12)
                met->LRc[i][lColumnIndexJ] = 0.;
			if(i==ilsTa) {
                if (lColumnIndexJ <= lLapseRateTemp.size())
                    met->LRc[i][lColumnIndexJ] = lLapseRateTemp[lColumnIndexJ];
                else
                    met->LRc[i][lColumnIndexJ] = geotop::input::gDoubleNoValue ;
            }
			if(i==ilsTdew) {
                if (lColumnIndexJ <= lLapseRateDewTemp.size())
                    met->LRc[i][lColumnIndexJ] = lLapseRateDewTemp[lColumnIndexJ];
                else
                    met->LRc[i][lColumnIndexJ] = geotop::input::gDoubleNoValue ;
            }
			if(i==ilsPrec) {
                if (lColumnIndexJ <= lLapseRatePrec.size())
                    met->LRc[i][lColumnIndexJ] = lLapseRatePrec[lColumnIndexJ];
                else
                    met->LRc[i][lColumnIndexJ] = geotop::input::gDoubleNoValue ;
            }
		}
	}

	par->MinIncrFactWithElev =  getDoubleValueWithDefault(lConfigStore, "MinPrecIncreaseFactorWithElev", geotop::input::gDoubleNoValue, false) ;
	par->MaxIncrFactWithElev =  getDoubleValueWithDefault(lConfigStore, "MaxPrecDecreaseFactorWithElev", geotop::input::gDoubleNoValue, false) ;
	
	//output point column
	n = (long)otot;
	geotop::common::Variables::opnt = (long*)malloc(n*sizeof(long));
	geotop::common::Variables::ipnt = (short*)malloc(n*sizeof(short));

    par->all_point = (short)getDoubleValueWithDefault(lConfigStore, "PointAll", geotop::input::gDoubleNoValue, false) ;

	if (par->all_point == 1) {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::ipnt[i] = 1;
			geotop::common::Variables::opnt[i] = i;
		}
		
		geotop::common::Variables::nopnt = n;
		
	}else {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::ipnt[i] = 0;
			geotop::common::Variables::opnt[i] = -1;
		}

        lKeywordString.clear() ;
        lKeywordString += "DatePoint", "JulianDayFromYear0Point", "TimeFromStartPoint",
        "PeriodPoint", "RunPoint", "IDPointPoint", "PsnowPoint",
        "PrainPoint", "PsnowNetPoint", "PrainNetPoint", "PrainOnSnowPoint",
        "WindSpeedPoint", "WindDirPoint", "RHPoint", "AirPressPoint",
        "AirTempPoint", "TDewPoint", "TsurfPoint", "TvegPoint",
        "TCanopyAirPoint", "SurfaceEBPoint", "SoilHeatFluxPoint", "SWinPoint",
        "SWbeamPoint", "SWdiffPoint", "LWinPoint", "LWinMinPoint",
        "LWinMaxPoint", "SWNetPoint", "LWNetPoint", "HPoint",
        "LEPoint", "CanopyFractionPoint", "LSAIPoint", "z0vegPoint",
        "d0vegPoint", "EstoredCanopyPoint", "SWvPoint", "LWvPoint",
        "HvPoint", "LEvPoint", "HgUnvegPoint", "LEgUnvegPoint",
        "HgVegPoint", "LEgVegPoint", "EvapSurfacePoint", "TraspCanopyPoint",
        "WaterOnCanopyPoint", "SnowOnCanopyPoint", "QVegPoint", "QSurfPoint",
        "QAirPoint", "QCanopyAirPoint", "LObukhovPoint", "LObukhovCanopyPoint",
        "WindSpeedTopCanopyPoint", "DecayKCanopyPoint", "SWupPoint", "LWupPoint",
        "HupPoint", "LEupPoint", "SnowDepthPoint", "SWEPoint",
        "SnowDensityPoint", "SnowTempPoint", "SnowMeltedPoint", "SnowSublPoint",
        "SWEBlownPoint", "SWESublBlownPoint", "GlacDepthPoint", "GWEPoint",
        "GlacDensityPoint", "GlacTempPoint", "GlacMeltedPoint", "GlacSublPoint",
        "LowestThawedSoilDepthPoint", "HighestThawedSoilDepthPoint", "LowestWaterTableDepthPoint", "HighestWaterTableDepthPoint" ;

		for (size_t i=0; i<n; i++) {
			lColumnIndexJ = (long)getDoubleValueWithDefault(lConfigStore, lKeywordString[i], geotop::input::gDoubleNoValue, false) ;
			if (lColumnIndexJ>=1 && lColumnIndexJ<=n){
				geotop::common::Variables::opnt[lColumnIndexJ-1] = i;
				geotop::common::Variables::ipnt[i] = 1;
			}
		}
		geotop::common::Variables::nopnt = 0;
		for (size_t i=0; i<n; i++) {
			if(geotop::common::Variables::opnt[i] > 0) geotop::common::Variables::nopnt = i+1;
		}
	}
	
	//output basin column
	n = (long)ootot;
	geotop::common::Variables::obsn = (long*)malloc(n*sizeof(long));
	geotop::common::Variables::ibsn = (short*)malloc(n*sizeof(short));
	
    par->all_basin = (short)getDoubleValueWithDefault(lConfigStore, "BasinAll", geotop::input::gDoubleNoValue, false) ;

	if (par->all_basin == 1) {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::ibsn[i] = 1;
			geotop::common::Variables::obsn[i] = i;
		}
		
		geotop::common::Variables::nobsn = (long)ootot;
		
	}else {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::ibsn[i] = 0;
			geotop::common::Variables::obsn[i] = -1;
		}

        lKeywordString.clear() ;
        lKeywordString += "DateBasin", "JulianDayFromYear0Basin", "TimeFromStartBasin",
            "PeriodBasin", "RunBasin", "PRainNetBasin", "PSnowNetBasin", "PRainBasin",
            "PSnowBasin", "PNetBasin", "AirTempBasin", "TSurfBasin", "TvegBasin",
            "EvapSurfaceBasin", "TraspCanopyBasin", "LEBasin", "HBasin", "SWNetBasin",
            "LWNetBasin", "LEvBasin", "HvBasin", "SWvBasin", "LWvBasin",
            "SWinBasin", "LWinBasin", "MassErrorBasin", "MeanTimeStep" ;

		for (size_t i=0; i<n; i++) {
			lColumnIndexJ = (long)getDoubleValueWithDefault(lConfigStore, lKeywordString[i], geotop::input::gDoubleNoValue, false) ;
			if (lColumnIndexJ>=1 && lColumnIndexJ<=n){
				geotop::common::Variables::obsn[lColumnIndexJ-1] = i;
				geotop::common::Variables::ibsn[i] = 1;
			}
		}

		geotop::common::Variables::nobsn = 0;
		for (size_t i=0; i<n; i++) {
			if(geotop::common::Variables::obsn[i] > 0) geotop::common::Variables::nobsn = i+1;
		}
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SoilPlotDepths", geotop::input::gDoubleNoValue, false, 0, true) ;
    size_t lSoilPlotDepthsSize = lDoubleTempVector.size() + 1;
    par->soil_plot_depths.resize(lSoilPlotDepthsSize);
	for (size_t i=1; i<lSoilPlotDepthsSize; i++) {
		par->soil_plot_depths[i] = lDoubleTempVector[i-1] ;
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SnowPlotDepths", geotop::input::gDoubleNoValue, false, 0, true) ;
    lSoilPlotDepthsSize = lDoubleTempVector.size() + 1;
    par->snow_plot_depths.resize(lSoilPlotDepthsSize);
	for (size_t i=1; i<lSoilPlotDepthsSize; i++) {
		par->snow_plot_depths[i] = lDoubleTempVector[i-1] ;
	}

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "GlacPlotDepths", geotop::input::gDoubleNoValue, false, 0, true) ;
    lSoilPlotDepthsSize = lDoubleTempVector.size() + 1;
    par->glac_plot_depths.resize(lSoilPlotDepthsSize);
	for (size_t i=1; i<lSoilPlotDepthsSize; i++) {
		par->glac_plot_depths[i] = lDoubleTempVector[i-1] ;
	}

	//output snow column
	if ((long)par->snow_plot_depths[1] != geotop::input::gDoubleNoValue) {
		m = par->snow_plot_depths.size();
	}else {
		m = par->max_snow_layers;
	}
	n = 6;
	
	geotop::common::Variables::osnw = (long*)malloc(n*sizeof(long));

    par->all_snow = (short)getDoubleValueWithDefault(lConfigStore, "SnowAll", geotop::input::gDoubleNoValue, false) ;

	if (par->all_snow == 1) {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::osnw[i] = i;
		}
		
		geotop::common::Variables::nosnw = n;
		
	}else {
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::osnw[i] = -1;
		}

        lKeywordString.clear() ;
        lKeywordString += "DateSnow", "JulianDayFromYear0Snow", "TimeFromStartSnow",
            "PeriodSnow", "RunSnow", "IDPointSnow" ;

		for (size_t i=0; i<6; i++) {
			lColumnIndexJ = (long)getDoubleValueWithDefault(lConfigStore, lKeywordString[i], geotop::input::gDoubleNoValue, false) ;
			if (lColumnIndexJ>=1 && lColumnIndexJ<=n)
                geotop::common::Variables::osnw[lColumnIndexJ-1] = i;
		}

		geotop::common::Variables::nosnw = 0;
		for (size_t i=0; i<n; i++) {
			if(geotop::common::Variables::osnw[i] > 0) geotop::common::Variables::nosnw = i+1;
		}
	}
		
	
	//output glacier column
	if ((long)par->glac_plot_depths[1] != geotop::input::gDoubleNoValue) {
		m = par->glac_plot_depths.size();
	}else {
		m = par->max_glac_layers;
	}
	n = 6 + 3*m + 1*par->max_glac_layers;
	geotop::common::Variables::oglc = (long*)malloc(n*sizeof(long));

    par->all_glac = (short)getDoubleValueWithDefault(lConfigStore, "GlacAll", geotop::input::gDoubleNoValue, false) ;

	if (par->all_glac == 1) {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::oglc[i] = i;
		}
		
		geotop::common::Variables::noglc = n;
		
	}else {
				
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::oglc[i] = -1;
		}

        lKeywordString.clear() ;
        lKeywordString += "SnowAll", "DateGlac", "JulianDayFromYear0Glac", "TimeFromStartGlac", "PeriodGlac", "RunGlac" ;
        for(size_t i = 0 ; i < lKeywordString.size() ; i++)
        {
            lColumnIndexJ = getDoubleValueWithDefault(lConfigStore, "SnowAll", geotop::input::gDoubleNoValue, false) ;
            if (lColumnIndexJ>=1 && lColumnIndexJ<=n) geotop::common::Variables::oglc[lColumnIndexJ-1] = i;
        }

        lKeywordString.clear() ;
        lKeywordString += "IDPointGlac", "TempGlac", "IceContentGlac" ;
        for(size_t i = 0 ; i < lKeywordString.size() ; i++)
        {
            lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, lKeywordString[i], -1, false, 0, false) ;
			for (k=0; k<m; k++) {
				lColumnIndexJ = (size_t)lDoubleTempVector[k];
				if (lColumnIndexJ>=1 && lColumnIndexJ<=n) geotop::common::Variables::oglc[lColumnIndexJ-1] = (i)*m + k + 6;
			}
        }

        lKeywordString.clear() ;
        lKeywordString += "WatContentGlac" ;
        for(size_t i = 0 ; i < lKeywordString.size() ; i++)
        {
            lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, lKeywordString[i], -1, false, 0, false) ;
			for (k=0; k<par->max_glac_layers; k++) {
				lColumnIndexJ = (size_t)lDoubleTempVector[k];
				if (lColumnIndexJ>=1 && lColumnIndexJ<=n) geotop::common::Variables::oglc[lColumnIndexJ-1] = (i-9)*par->max_glac_layers + k + 6 + 3*m;
			}
        }

		geotop::common::Variables::noglc = 0;
		for (size_t i=0; i<n; i++) {
			if(geotop::common::Variables::oglc[i] > 0) geotop::common::Variables::noglc = i+1;
		}
	}
	
	//output soil column
	n = 6;
	geotop::common::Variables::osl = (long*)malloc(n*sizeof(long));

    par->all_soil = (short)getDoubleValueWithDefault(lConfigStore, "SoilAll", geotop::input::gDoubleNoValue, false) ;

	if (par->all_soil == 1) {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::osl[i] = i;
		}
		
		geotop::common::Variables::nosl = n;
		
	}else {
		
		for (size_t i=0; i<n; i++) {
			geotop::common::Variables::osl[i] = -1;
		}
        lKeywordString.clear() ;
        lKeywordString += "DateSoil", "JulianDayFromYear0Soil", "TimeFromStartSoil",
            "PeriodSoil", "RunSoil", "IDPointSoil" ;

		for (size_t i=0; i<n; i++) {
			lColumnIndexJ = (long)getDoubleValueWithDefault(lConfigStore, lKeywordString[i], geotop::input::gDoubleNoValue, false) ;
			if (lColumnIndexJ>=1 && lColumnIndexJ<=n) geotop::common::Variables::osl[lColumnIndexJ-1] = i;
		}

		geotop::common::Variables::nosl = 0;
		for (size_t i=0; i<n; i++) {
			if(geotop::common::Variables::osl[i] > 0) geotop::common::Variables::nosl = i+1;
		}
	}

	par->ric_cloud = (short)getDoubleValueWithDefault(lConfigStore, "RicalculateCloudiness", geotop::input::gDoubleNoValue, false) ;
	par->vap_as_RH = (short)getDoubleValueWithDefault(lConfigStore, "DewTemperatureAsRH", geotop::input::gDoubleNoValue, false) ;
	par->vap_as_Td = (short)getDoubleValueWithDefault(lConfigStore, "RHAsDewTemperature", geotop::input::gDoubleNoValue, false) ;
	par->ndivdaycloud = (long)getDoubleValueWithDefault(lConfigStore, "NumberDayIntervalsToCalculateCloudiness", geotop::input::gDoubleNoValue, false) ;
	par->cast_shadow = (short)getDoubleValueWithDefault(lConfigStore, "CalculateCastShadow", geotop::input::gDoubleNoValue, false) ;
	par->wind_as_dir = (short)getDoubleValueWithDefault(lConfigStore, "WindAsSpeedAndDirection", geotop::input::gDoubleNoValue, false) ;
	par->wind_as_xy = (short)getDoubleValueWithDefault(lConfigStore, "WindAsWindXAndWindY", geotop::input::gDoubleNoValue, false) ;
	par->snow_aging_vis = getDoubleValueWithDefault(lConfigStore, "SnowAgingCoeffVis", geotop::input::gDoubleNoValue, false) ;
	par->snow_aging_nir = getDoubleValueWithDefault(lConfigStore, "SnowAgingCoeffNIR", geotop::input::gDoubleNoValue, false) ;
	par->DepthFreeSurface = getDoubleValueWithDefault(lConfigStore, "DepthFreeSurfaceAtTheBoundary", geotop::input::gDoubleNoValue, false) ;
	par->prec_as_intensity = getDoubleValueWithDefault(lConfigStore, "PrecAsIntensity", geotop::input::gDoubleNoValue, false) ;

    par->linear_interpolation_meteo.resize(nmeteo_stations + 1);
    
    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "LinearInterpolation", 0., true, nmeteo_stations, false) ;
	for (size_t i=1; i<par->linear_interpolation_meteo.size(); i++) {
		par->linear_interpolation_meteo[i] = lDoubleTempVector[i-1] ;
	}

    par->output_vertical_distances = (short)getDoubleValueWithDefault(lConfigStore, "OutputDepthsVertical", geotop::input::gDoubleNoValue, false) ;
	if(par->point_sim != 1){
		if(par->output_vertical_distances == 1){
			printf("Only for point simulations the parameter OutputDepthsVertical can be assigned to 1, layers are defined vertically\n");
			fprintf(flog, "Only for point simulations the parameter OutputDepthsVertical can be assigned to 1, layers are defined vertically\n");
		}
	}

	par->upwindblowingsnow = (short)getDoubleValueWithDefault(lConfigStore, "UpwindBorderBlowingSnow", geotop::input::gDoubleNoValue, false) ;
    
	par->UpdateK = (short)getDoubleValueWithDefault(lConfigStore, "UpdateHydraulicConductivity", geotop::input::gDoubleNoValue, false) ;

	par->ContRecovery = getDoubleValueWithDefault(lConfigStore, "ContinuousRecovery", geotop::input::gDoubleNoValue, false) ;
	par->flag1D = (short)getDoubleValueWithDefault(lConfigStore, "ActualOrProjectedArea", geotop::input::gDoubleNoValue, false) ;
	
	par->k_to_ksat = getDoubleValueWithDefault(lConfigStore, "MinRatioKactualToKSat", geotop::input::gDoubleNoValue, false) ;
	par->RunIfAnOldRunIsPresent = (short)getDoubleValueWithDefault(lConfigStore, "RunIfAnOldRunIsPresent", geotop::input::gDoubleNoValue, false) ;
	
	par->max_courant_land_channel = getDoubleValueWithDefault(lConfigStore, "MaxCourantSupFlowChannelLand", geotop::input::gDoubleNoValue, false) ;
	par->min_dhsup_land_channel_out = getDoubleValueWithDefault(lConfigStore, "MinDiffSupWaterDepthChannelLand", geotop::input::gDoubleNoValue, false) ;

	par->Nl_spinup.resize(par->end_date.size() + 1);

    lDoubleTempVector = getDoubleVectorValueWithDefault(lConfigStore, "SpinUpLayerBottom", 0., true, par->end_date.size(), false) ;
	for (size_t i=1; i<par->Nl_spinup.size(); i++) {
		par->Nl_spinup[i] = lDoubleTempVector[i-1] ;
	}
	if(par->Nl_spinup[1]<10000. && par->point_sim!=1){
		printf("You can use SpinUpLayerBottom only if PointSim is set to 1\n");
		fprintf(flog,"You can use SpinUpLayerBottom only if PointSim is set to 1\n");
		t_error("Not possible to continue");
	}

	par->newperiodinit = (short)getDoubleValueWithDefault(lConfigStore, "InitInNewPeriods", geotop::input::gDoubleNoValue, false) ;

	if(par->newperiodinit != 0 && par->point_sim != 1){
		printf("You can use InitInNewPeriods only if PointSim is set to 1\n");
		fprintf(flog,"You can use InitInNewPeriods only if PointSim is set to 1\n");
		t_error("Not possible to continue");
	}

	par->k1 = getDoubleValueWithDefault(lConfigStore, "KonzelmannA", geotop::input::gDoubleNoValue, false) ;
	par->k2 = getDoubleValueWithDefault(lConfigStore, "KonzelmannB", geotop::input::gDoubleNoValue, false) ;
	par->Lozone = getDoubleValueWithDefault(lConfigStore, "Lozone", geotop::input::gDoubleNoValue, false) ;
	par->alpha_iqbal = getDoubleValueWithDefault(lConfigStore, "AngstromAlpha", geotop::input::gDoubleNoValue, false) ;
	par->beta_iqbal = getDoubleValueWithDefault(lConfigStore, "AngstromBeta", geotop::input::gDoubleNoValue, false) ;
    
	par->albedoSWin = (short)getDoubleValueWithDefault(lConfigStore, "ConsiderAlbedoInSWin", geotop::input::gDoubleNoValue, false) ;
	
	par->micro = (short)getDoubleValueWithDefault(lConfigStore, "ConsiderMicrometeorology", geotop::input::gDoubleNoValue, false) ;
	par->EB = getDoubleValueWithDefault(lConfigStore, "SurfaceEnergyFlux", geotop::input::gDoubleNoValue, true) ;
	par->Cair = getDoubleValueWithDefault(lConfigStore, "ConvectiveHeatTransferCoefficient", geotop::input::gDoubleNoValue, true) ;
	par->Tsup = getDoubleValueWithDefault(lConfigStore, "SurfaceTemperature", geotop::input::gDoubleNoValue, true) ;
    
	par->Tair_default = getDoubleValueWithDefault(lConfigStore, "BaseAirTemperature", geotop::input::gDoubleNoValue, false) ;
	par->RH_default = getDoubleValueWithDefault(lConfigStore, "BaseRelativeHumidity", geotop::input::gDoubleNoValue, false)/100.;
	par->V_default = getDoubleValueWithDefault(lConfigStore, "BaseWindSpeed", geotop::input::gDoubleNoValue, false) ;
	par->Vdir_default = getDoubleValueWithDefault(lConfigStore, "BaseWindDirection", geotop::input::gDoubleNoValue, false) ;
	par->IPrec_default = getDoubleValueWithDefault(lConfigStore, "BaseIPrec", geotop::input::gDoubleNoValue, false) ;

	par->soil_type_bedr_default = (long)getDoubleValueWithDefault(lConfigStore, "DefaultSoilTypeBedrock", geotop::input::gDoubleNoValue, false) ;

    if(par->soil_type_bedr_default<1 || par->soil_type_bedr_default>par->nsoiltypes){
		fprintf(flog, "Error:  soil_type_bedr_default lower than 0 or higher than soil types numbers");
		printf("Error:  soil_type_bedr_default lower than 0 or higher than soil types numbers");
		t_error("Fatal Error! Geotop is closed. See failing report.");
	}

	par->minP_torestore_A = getDoubleValueWithDefault(lConfigStore, "MinPrecToRestoreFreshSnowAlbedo", geotop::input::gDoubleNoValue, false) ;
	par->snow_conductivity = (short)getDoubleValueWithDefault(lConfigStore, "SnowThermalConductivityPar", geotop::input::gDoubleNoValue, false) ;
	par->snow_wind_compaction_1D = (short)getDoubleValueWithDefault(lConfigStore, "WindCompaction1D", geotop::input::gDoubleNoValue, false) ;

	if(par->snow_wind_compaction_1D == 1) par->blowing_snow = 1;

	par->DDchannel = (short)getDoubleValueWithDefault(lConfigStore, "DDChannel", geotop::input::gDoubleNoValue, false) ;
	par->DDland = (short)getDoubleValueWithDefault(lConfigStore, "DDLand", geotop::input::gDoubleNoValue, false) ;
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/

short read_soil_parameters(std::string name, InitTools *IT, Soil *sl, long bed, FILE *flog){
	
	short ok;
	long i, j, k, n, nlines, nlinesprev;
    std::string temp;
	double **soildata;
    GeoTensor<double> old_sl_par;
    FILE *f ;
	
	//look if there is at least 1 soil file
i = 0;
	ok = 0;
	nlinesprev = -1;
	
	do{
		
		temp = namefile_i_we2(name, i+1);
		
		if (mio::IOUtils::fileExists(string(temp) + string(textfile))) {
			ok = 1;
			temp = namefile_i(name, i+1);
			nlines = count_lines(temp, 33, 44);
            
			if (nlinesprev >= 0 && nlines != nlinesprev){
				f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
				fprintf(f,"Error:: The file %s with soil paramaters has a number of layers %ld, which different from the numbers %ld of the other soil parameter files\n",temp.c_str(), nlines, nlinesprev);
				fprintf(f,"In GEOtop it is only possible to have the same number of layers in any soil parameter files\n");
				fclose(f);
				t_error("Fatal Error! GEOtop is closed. See failing report.");
			}
			nlinesprev = nlines;
		}else {
			if (i==0 && name != geotop::input::gStringNoValue) {
				f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
				fprintf(f,"Error:: Soil file %s not existing.\n", name.c_str());
				fclose(f);
				t_error("Fatal Error! GEOtop is closed. See failing report.");	
			}
		}
			
		i++;
		
	} while(ok == 0 && i < sl->pa.getDh()-1);
	
	if (ok == 1){
		
		//save sl->pa in a new doubletensor and deallocate
		old_sl_par.resize(sl->pa.getDh() + 1, sl->pa.getRh() + 1, sl->pa.getCh() + 1);
		for (i=1; i<sl->pa.getDh(); i++) {
			for (n=1; n<sl->pa.getRh(); n++) {
				for (j=1; j<sl->pa.getCh(); j++) {
					old_sl_par[i][n][j] = sl->pa(i,n,j);
				}
			}
		}
	//	free_doubletensor(sl->pa);

		//reallocate
		sl->pa.resize(old_sl_par.getDh(), old_sl_par.getRh(), nlines+1);

		for (i=1; i<sl->pa.getDh(); i++) {
			
			//read files
			temp = namefile_i_we2(name, i);
			
			if (mio::IOUtils::fileExists(string(temp) + string(textfile))) {
				temp = namefile_i(name, i);
				soildata = read_txt_matrix(temp, 33, 44, IT->soil_col_names, nsoilprop, &nlines, flog);
			}else {
				soildata = (double**)malloc(nlines*sizeof(double*));
				for (j=0; j<nlines; j++) {
					k = (long)nsoilprop;
					soildata[j] = (double*)malloc(k*sizeof(double));
					for (n=0; n<k; n++) {
						soildata[j][n] = (double)geotop::input::gDoubleAbsent;
					}
				}
			}
			
			//assign soildata to soil->pa
			for (n=1; n<=nsoilprop; n++) {
				for (j=1; j<sl->pa.getCh(); j++) { //j is the layer index
					sl->pa[i][n][j] = soildata[j-1][n-1];
				}
			}
			 
			//deallocate soildata
			for (j=0; j<nlines; j++) {
				free(soildata[j]);
			}
			free(soildata);
			
			//fix layer thickness
			n = jdz;
			for (j=1; j<sl->pa.getCh(); j++) { //j is the layer index
				if ((long)sl->pa[i][n][j] != geotop::input::gDoubleNoValue && (long)sl->pa[i][n][j] != geotop::input::gDoubleAbsent) {
					if ( i > 1 && fabs( sl->pa[i][n][j] - sl->pa[i-1][n][j] ) > 1.E-5 )  {
						f = fopen(geotop::common::Variables::FailedRunFile.c_str(), "w");
						fprintf(f,"Error:: For soil type %ld it has been given a set of soil layer thicknesses different from the other ones.\n",i);
						fprintf(f,"In Geotop it is only possible to have the soil layer discretization in any soil parameter files.\n");
						fclose(f);
						t_error("Fatal Error! Geotop is closed. See failing report.");	
					}
				} else if (i == 1) {
					if (j < old_sl_par.getCh()) {
							sl->pa(i,n,j) = old_sl_par(i,n,j);
					} else {
							sl->pa(i,n,j) = sl->pa(i,n,j-1);
					}
				} else {
					sl->pa(i,n,j) = sl->pa(i-1,n,j);
				}
			}
			
			//all other variables
            for (n=1; n<=nsoilprop; n++) {
                if (n != jdz) {
                    //for (j=1; j<=sl->pa->nch; j++) { //j is the layer index
                    for (j=1; j<sl->pa.getCh(); j++) { //j is the layer index
                        if ((long)sl->pa[i][n][j] == geotop::input::gDoubleNoValue || (long)sl->pa[i][n][j] == geotop::input::gDoubleAbsent) {
                            //if (j <= old_sl_par->nch) {
                            if (j < old_sl_par.getCh()) {
                                sl->pa[i][n][j] = old_sl_par[i][n][j];
                            }else {
                                sl->pa[i][n][j] = sl->pa[i][n][j-1];
                            }
                        }
                    }
                }
            }
			
			//field capacity and wilting point
			for (j=1; j<sl->pa.getCh(); j++){
				if( (long)sl->pa[i][jfc][j] == geotop::input::gDoubleNoValue){
					sl->pa[i][jfc][j] = teta_psi( (-1./3.)*1.E5/GTConst::GRAVITY, 0., sl->pa[i][jsat][j], sl->pa[i][jres][j], sl->pa[i][ja][j],
													 sl->pa[i][jns][j], 1.-1./sl->pa[i][jns][j], GTConst::PsiMin, sl->pa[i][jss][j]);
				}
				
				if( (long)sl->pa[i][jwp][j] == geotop::input::gDoubleNoValue){
					sl->pa[i][jwp][j] = teta_psi( -15.*1.E5/GTConst::GRAVITY, 0., sl->pa[i][jsat][j], sl->pa[i][jres][j], sl->pa[i][ja][j],
													 sl->pa[i][jns][j], 1.-1./sl->pa[i][jns][j], GTConst::PsiMin, sl->pa[i][jss][j]);
				}
			}
			
			//pressure
            ok = 1;
            for (j=1; j<sl->pa.getCh(); j++){
                if( (long)sl->pa[i][jpsi][j] == geotop::input::gDoubleNoValue) ok = 0;
            }


            if (ok == 1 )
            {
                if (IT->init_water_table_depth.size() <= i ) {

			IT->init_water_table_depth.resize(IT->init_water_table_depth.size() + 1);
                }
                IT->init_water_table_depth[i] = geotop::input::gDoubleNoValue;
            }
        }

		//free_doubletensor(old_sl_par);
			
	}
	
	//write on the screen the soil paramater
    
    std::vector<string> lSoilParameters ;
    lSoilParameters += "HeaderSoilDz", "HeaderSoilInitPres", "HeaderSoilInitTemp", "HeaderNormalHydrConductivity",
    "HeaderLateralHydrConductivity", "HeaderThetaRes", "HeaderWiltingPoint", "HeaderFieldCapacity",
    "HeaderThetaSat", "HeaderAlpha", "HeaderN", "HeaderV",
    "HeaderKthSoilSolids", "HeaderCthSoilSolids", "HeaderSpecificStorativity" ;
	fprintf(flog,"\n");
	k = (long)nmet;
	fprintf(flog,"Soil Layers: %ud\n",sl->pa.getCh()-1);
	for (i=1; i<sl->pa.getDh(); i++) {
		fprintf(flog,"-> Soil Type: %ld\n",i);
		for (n=1; n <= lSoilParameters.size(); n++) {
			fprintf(flog,"%s: ",lSoilParameters[n-1].c_str());
			for (j=1; j<sl->pa.getCh(); j++) {
				fprintf(flog,"%f(%.2e)",sl->pa[i][n][j],sl->pa[i][n][j]);
				if (j<sl->pa.getCh()-1) fprintf(flog,", ");
			}
			fprintf(flog,"\n");
		}
	}
	
	//bedrock
	old_sl_par.resize(1 + 1, IT->pa_bed.getRh() + 1, IT->pa_bed.getCh() + 1);
	for (n=1; n<sl->pa_bed.getRh(); n++) {
		for (j=1; j<sl->pa_bed.getCh(); j++) {
			old_sl_par[1][n][j] = IT->pa_bed[1][n][j];
		}
	}
	//free_doubletensor(sl->pa_bed);
	
	IT->pa_bed.resize(sl->pa.getDh(), sl->pa.getRh(), sl->pa.getCh());
	for (i=1; i<IT->pa_bed.getDh(); i++) {
		for (n=1; n<IT->pa_bed.getRh(); n++) {
			if (n == jdz) {
				for (j=1; j<IT->pa_bed.getCh(); j++) {
					IT->pa_bed(i,n,j) = sl->pa(1,n,j);
				}
			}else {
				for (j=1; j<IT->pa_bed.getCh(); j++) {
					if (j < old_sl_par.getCh()) {
						IT->pa_bed(i,n,j) = old_sl_par(1,n,j);
					}else {
						IT->pa_bed(i,n,j) = sl->pa_bed(i,n,j-1);
					}
				}
				for (j=1; j<IT->pa_bed.getCh(); j++) {
					if ( (long)IT->pa_bed(i,n,j) == geotop::input::gDoubleNoValue ) IT->pa_bed(i,n,j) = sl->pa(bed,n,j);
				}
			}
		}		
	}
	//free_doubletensor(old_sl_par);
	
	fprintf(flog,"\n");
	k = (long)nmet;
	fprintf(flog,"Soil Bedrock Layers: %ud\n",sl->pa.getCh()-1);
	for (i=1; i<IT->pa_bed.getDh(); i++) {
		fprintf(flog,"-> Soil Type: %ld\n",i);
		for (n=1; n<=lSoilParameters.size(); n++) {
			fprintf(flog,"%s: ",lSoilParameters[n-1].c_str());
			for (j=1; j<sl->pa.getCh(); j++) {
				fprintf(flog,"%f(%.2e)",IT->pa_bed[i][n][j],IT->pa_bed[i][n][j]);
				if(j < sl->pa.getCh()-1) fprintf(flog,", ");
			}
			fprintf(flog,"\n");
		}
	}
	fprintf(flog,"\n");
	
	return 1;
	
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/		

short read_point_file(std::string name, std::vector<std::string> key_header, Par *par, FILE *flog){

	GeoMatrix<double>  chkpt2;
	double **points;
	long nlines, n, j;
      std::string temp;

	if (mio::IOUtils::fileExists(string(name) + string(textfile))) {
		temp = name + std::string(textfile);
		printf("%s\n",temp.c_str());
		points = read_txt_matrix(temp, 34, 44, key_header, par->chkpt.getCols()-1, &nlines, flog);
				
		chkpt2.resize(par->chkpt.getRows() + 1, par->chkpt.getCols() + 1);

		chkpt2=par->chkpt;
	//	free_doublematrix(par->chkpt);
		
		par->chkpt.resize(nlines+1, chkpt2.getCols());
		for (n=1; n<=nlines; n++) {
			for (j=1; j< chkpt2.getCols(); j++) {
				par->chkpt[n][j] = points[n-1][j-1];
				if ( (long)par->chkpt[n][j] == geotop::input::gDoubleNoValue || (long)par->chkpt[n][j] == geotop::input::gDoubleAbsent ) {
					if ( n < chkpt2.getRows() ) {
						par->chkpt[n][j] = chkpt2[n][j];
					}else {
						par->chkpt[n][j] = chkpt2[chkpt2.getRows()-1][j];
					}
				}
			}
			
			if ( par->point_sim != 1 ){
				if( (long)par->chkpt[n][ptX] == geotop::input::gDoubleNoValue || (long)par->chkpt[n][ptY] == geotop::input::gDoubleNoValue ) {
					par->state_pixel = 0;
				}
			}
			
			free(points[n-1]);
		}
				
	//	free_doublematrix(chkpt2);
		free(points);
	}
	
	if (par->point_sim != 1){
		for (n=1; n< par->chkpt.getRows(); n++) {
			if ( (long)par->chkpt[n][ptX] == geotop::input::gDoubleNoValue || (long)par->chkpt[n][ptY] == geotop::input::gDoubleNoValue) {
				fprintf(flog,"Warning: The points to plot specific results are not completely specified\n");
				fprintf(flog,"Output for single point output is deactivated.\n");
				printf("Warning: The points to plot specific results are not completely specified\n");
				printf("Output for single point output is deactivated.\n");
				par->state_pixel = 0;
			}
		}
	}
	
	
	return 1;
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/		
	
//short read_meteostations_file(LONGVECTOR *i, METEO_STATIONS *S, char *name, char **key_header, FILE *flog){
short read_meteostations_file(const GeoVector<long>& i, MeteoStations *S, std::string name, std::vector<std::string> key_header, FILE *flog){
	double **M;
	long nlines, n, j, k;
    std::string temp;
		
	if (mio::IOUtils::fileExists(name + string(textfile))) {
		temp = name + textfile ;
		M = read_txt_matrix(temp, 33, 44, key_header, 8, &nlines, flog);
				
		for (j=1; j<i.size(); j++) {
			for (n=1; n<=nlines; n++) {
				if ((long)M[n-1][0] == i[j]) {
					for (k=1; k<8; k++) {
						if ((long)M[n-1][k] != geotop::input::gDoubleNoValue && (long)M[n-1][k] != geotop::input::gDoubleAbsent) {
							if (k==1) {
								S->E[j] = M[n-1][k];
							}else if (k==2) {
								S->N[j] = M[n-1][k];
							}else if (k==3) {
								S->lat[j] = M[n-1][k];
							}else if (k==4) {
								S->lon[j] = M[n-1][k];
							}else if (k==5) {
								S->Z[j] = M[n-1][k];
							}else if (k==6) {
								S->sky[j] = M[n-1][k];
							}else if (k==7) {
								S->ST[j] = M[n-1][k];
							}
						}
					}
				}					
			}
		}
		
		for (n=1; n<=nlines; n++) {
			free(M[n-1]);
		}
		
		free(M);
		
		return 1;
	}else {
		return 0;
	}
}

/***********************************************************/
/***********************************************************/
/***********************************************************/
/***********************************************************/		
