#pragma once

#include <string>

namespace StringConstants {

    // Resolution strings
    inline const std::string RES_2560x1440 = "2560 x 1440";
    inline const std::string RES_1920x1080 = "1920 x 1080";
    inline const std::string RES_1366x768 = "1366 x 768";
    inline const std::string RES_CUSTOM = "CUSTOM";

    // Window mode strings
    inline const std::string MODE_FULLSCREEN = "FULLSCREEN";
    inline const std::string MODE_WINDOWED = "WINDOWED";

    // Planet type strings
    inline const std::string PLANET_ROCKY = "Rocky";
    inline const std::string PLANET_TERRESTRIAL = "Terrestrial";
    inline const std::string PLANET_GAS_GIANT = "Gas Giant";
    inline const std::string PLANET_BROWN_DWARF = "Brown Dwarf";
    inline const std::string PLANET_STAR = "Star";
    inline const std::string PLANET_RED_GIANT = "Red Giant";
    inline const std::string PLANET_RED_SUPERGIANT = "Red Supergiant";
    inline const std::string PLANET_WHITE_DWARF = "White Dwarf";
    inline const std::string PLANET_NEUTRON_STAR = "Neutron Star";
    inline const std::string PLANET_BLACK_HOLE = "Black Hole";

    // Life level strings
    inline const std::string LIFE_LIFELESS = "Lifeless";
    inline const std::string LIFE_UNICELLULAR = "Unicellular";
    inline const std::string LIFE_MULTICELLULAR_S = "Multicellular (S)";
    inline const std::string LIFE_MULTICELLULAR_C = "Multicellular (C)";
    inline const std::string LIFE_TRIBAL = "Tribal";
    inline const std::string LIFE_GLOBAL = "Global";
    inline const std::string LIFE_INTERPLANETARY = "Interplanetary";
    inline const std::string LIFE_COLONY = "Colony";

    // Command line argument strings
    inline const std::string ARG_OBJECTS_LONG = "--objects";
    inline const std::string ARG_OBJECTS_SHORT_N = "-n";
    inline const std::string ARG_OBJECTS_SHORT_O = "-o";
    inline const std::string ARG_ITERATIONS_LONG = "--iterations";
    inline const std::string ARG_ITERATIONS_SHORT = "-i";
}