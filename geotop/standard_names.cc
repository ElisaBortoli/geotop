#include "standard_names.h"


namespace geotop
{
    namespace input
    {

        static std::vector<stringpair_s> StandardNames = std::vector<stringpair_s>((size_t)UNKNOWN_VAR);

        static void fillStandardName(size_t index, const std::string& longstring, const std::string& shortstring, const std::string& unitstring)
        {
            StandardNames[index].longstring = longstring;
            StandardNames[index].shortstring = shortstring;
            StandardNames[index].unitstring = unitstring;
        }
        
        void fillStandardNames()
        {
            static bool init = false;

            //Avoid multiple initializations
            if (not init)
            {
                // SOIL TENSOR VARIABLES
                fillStandardName(SOIL_TEMP, "SoilTemperature", "SoilT", "C");
                fillStandardName(SOIL_WATER_CONTENT, "SoilWaterContent", "SoilTh", "mm");
                fillStandardName(SOIL_ICE_CONTENT, "SoilIceContent", "SoilThI", "mm");
                fillStandardName(SOIL_WATER_PRESSURE, "SoilWaterPressure", "SoilP", "m");
                fillStandardName(SOIL_TOTAL_PRESSURE, "SoilTotalPressure", "SoilTotP", "m");
                // fillStandardName(SOIL_ET, "SoilEvapotranspiration", "SoilET", "-");
                // SOIL MAP VARIABLES
                fillStandardName(SOIL_CAN_RAIN, "SoilCanopyInterceptedRain", "SoilCanRain", "mm");
                fillStandardName(SOIL_CAN_SNOW, "SoilCanopyInterceptedSnow", "SoilCanSnow", "mm");
                // SNOW MAP VARIABLES
                fillStandardName(SNOW_AGE, "SnowAge", "SnowA", "s");
                // fillStandardName(SNOW_DEPTH, "SnowDepth", "SnowHS", "mm");
                fillStandardName(SNOW_HN, "SnowHeightFallen", "SnowHN", "mm");
                fillStandardName(SNOW_MELTED, "SnowMelted", "SnowM", "-");
                fillStandardName(SNOW_SUBL, "SnowSublimation", "SnowSub", "-");
                fillStandardName(SNOW_DURATION, "SnowDuration", "SnowDur", "-");
                // fillStandardName(SNOW_CA, "SnowCoveredArea", "SnowCA", "-");
                // WATER MAPS VARIABLES
                fillStandardName(PREC_TOTAL, "PrecipitationTotal", "PrecTot", "mm");
                fillStandardName(PREC_LIQ, "PrecipitationNet", "PrecNet", "mm");
                fillStandardName(PREC_SNOW, "PrecipitationSnow", "PrecSnow", "mm");
                // ENERGY MAP VARIABLES
                fillStandardName(ENER_LWin, "EnergyIncomingLongWave", "EnerInLW", "W/m2");
                fillStandardName(ENER_SW, "EnergyShortWave", "EnerSW", "W/m2");
                fillStandardName(ENER_LW, "EnergyLongWave", "EnerLW", "W/m2");
                fillStandardName(ENER_LE, "EnergySurfaceLatentHeat", "EnerLE", "W/m2");
                fillStandardName(ENER_H, "EnergySurfaceSensibleHeat", "EnerH", "W/m2");
                fillStandardName(ENER_G, "EnergySurfaceHeat", "EnerG", "W/m2");
                fillStandardName(ENER_Ts, "EnergySurfaceTemperature", "EnerTs", "W/m2");
                fillStandardName(ENER_SWin, "EnergyIncomingShortWave", "EnerInSW", "W/m2");
                fillStandardName(ENER_SWinb, "EnergyDirectIncomingShortWave", "EnerDISW", "W/m2");
                // GLACIER MAP VARIABLES
                fillStandardName(GLAC_MELT, "GlacierMelted", "GlacM", "-");
                fillStandardName(GLAC_SUBL, "GlacierSublimation", "GlacS", "-");
                // METEO MAPS VARIABLES
                fillStandardName(METEO_AIRTEMP, "MeteoAirTemperature", "MeteoAT", "-");
                fillStandardName(METEO_WSPEED, "MeteoWindSpeed", "MeteoWS", "-");
                fillStandardName(VECTOR_TEST, "VectorTest", "VT", "-");
                init = true;
            }
        }

        std::string getLongString(Variable v)
        {
            return StandardNames[v].longstring;
        }
        
        std::string getShortString(Variable v)
        {
            return StandardNames[v].shortstring;
        }
        
        std::string getUnitString(Variable v)
        {
            return StandardNames[v].unitstring;
        }
    }
}

