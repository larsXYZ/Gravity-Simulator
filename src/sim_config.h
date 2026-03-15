#pragma once

#include "CONSTANTS.h"

struct SimConfig
{
    bool paused{ false };
    bool gravity_enabled{ true };
    bool heat_enabled{ true };
    bool show_gui{ true };
    bool bloom_enabled{ false };
    bool autobound{ false };
    bool render_life_always{ false };
    float timestep_slider_value{ TIMESTEP_VALUE_START };
    double fuel_burn_rate{ 1.0 };
};
