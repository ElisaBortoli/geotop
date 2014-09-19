#ifndef STANDARD_NAMES_H
#define STANDARD_NAMES_H

#include <vector>
#include <string>

namespace geotop
{
    namespace input
    {
        /**
         * @brief Output variables known
         */
        enum Variable {
            SOIL_TEMP = 0, //Soil temperature
            SOIL_WATER_CONTENT, //Water content in soil [mm]
            SOIL_ICE_CONTENT, //Ice content in soil depth
            SOIL_WATER_PRESSURE, //Liquid water pressure in soil depth
            SOIL_TOTAL_PRESSURE, //Total water and ice pressure in soil depth
            UNKNOWN_VAR
        };


        typedef struct {

            std::string longstring;
            std::string shortstring;
            std::string unitstring;

        } stringpair_s;


        void fillStandardNames();

        std::string getLongString(Variable v);
        std::string getShortString(Variable v);
        std::string getUnitString(Variable v);

    }
}

#endif
