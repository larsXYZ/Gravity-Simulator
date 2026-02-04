#pragma once

#include <string_view>

namespace StringConstants {

    // Resolution strings
    inline constexpr std::string_view RES_2560x1440 = "2560 x 1440";
    inline constexpr std::string_view RES_1920x1080 = "1920 x 1080";
    inline constexpr std::string_view RES_1366x768 = "1366 x 768";
    inline constexpr std::string_view RES_CUSTOM = "CUSTOM";

    // Window mode strings
    inline constexpr std::string_view MODE_FULLSCREEN = "FULLSCREEN";
    inline constexpr std::string_view MODE_WINDOWED = "WINDOWED";

    // Planet type strings
    inline constexpr std::string_view PLANET_ROCKY = "Rocky";
    inline constexpr std::string_view PLANET_TERRESTIAL = "Terrestial";
    inline constexpr std::string_view PLANET_GAS_GIANT = "Gas Giant";
    inline constexpr std::string_view PLANET_SMALL_STAR = "Small Star";
    inline constexpr std::string_view PLANET_STAR = "Star";
    inline constexpr std::string_view PLANET_BIG_STAR = "Big Star";
    inline constexpr std::string_view PLANET_BLACK_HOLE = "Black Hole";

    // Life level strings
    inline constexpr std::string_view LIFE_LIFELESS = "Lifeless";
    inline constexpr std::string_view LIFE_UNICELLULAR = "Unicellular";
    inline constexpr std::string_view LIFE_MULTICELLULAR_S = "Multicellular (S)";
    inline constexpr std::string_view LIFE_MULTICELLULAR_C = "Multicellular (C)";
    inline constexpr std::string_view LIFE_TRIBAL = "Tribal";
    inline constexpr std::string_view LIFE_GLOBAL = "Global";
    inline constexpr std::string_view LIFE_INTERPLANETARY = "Interplanetary";
    inline constexpr std::string_view LIFE_COLONY = "Colony";

    // Command line argument strings
    inline constexpr std::string_view ARG_OBJECTS_LONG = "--objects";
    inline constexpr std::string_view ARG_OBJECTS_SHORT_N = "-n";
    inline constexpr std::string_view ARG_OBJECTS_SHORT_O = "-o";
    inline constexpr std::string_view ARG_ITERATIONS_LONG = "--iterations";
    inline constexpr std::string_view ARG_ITERATIONS_SHORT = "-i";
}