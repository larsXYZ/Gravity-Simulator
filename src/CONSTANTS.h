#pragma once

//STATES
enum BodyType {
	ROCKY, TERRESTRIAL, GASGIANT,
	BROWNDWARF,
	STAR,                               // single main sequence type (was SMALLSTAR/STAR/BIGSTAR)
	WHITEDWARF, NEUTRONSTAR,            // compact remnants
	BLACKHOLE
};

enum StellarSubType { SUBTYPE_NONE, PULSAR, MAGNETAR };

// Type alias for backward compatibility during transition
using pType = BodyType;
enum lType { NONE, SINGLECELL, MULTICELL_SIMPLE, MULTICELL_COMPLEX, INTELLIGENT_TRIBAL, INTELLIGENT_GLOBAL, INTELLIGENT_INTERPLANETARY, COLONY };

//MATH
double const G = 0.00667;
double const SBconst = 0.00007;
double const tempConstTwo = 0.018;
const double PI = 3.14159265359;
const double SPEED_OF_LIGHT = 299792458.;

//SIMULATION
const int FRAMERATE = 60;
const int FRAMERATE_CHECK_DELTAFRAME = 10;

//DESTRUCTION
const float CREATEDUSTSPEEDMULT = 0.003f;

//OBJECT-TYPE LIMITS
const double ROCKYLIMIT = 15;
const double TERRESTRIALLIMIT = 100;
const double BROWNDWARFLIMIT = 400;     // gas giant ends, brown dwarf begins
const double GASGIANTLIMIT = 600;       // brown dwarf ends, star begins
const double STARLIMIT = 4000;          // star ends, black hole begins

// Evolution limits
const double CHANDRASEKHAR_LIMIT = 1500; // white dwarf + mass -> Type Ia supernova
const double TOV_LIMIT = 3200;           // neutron star + mass -> black hole

//STAR HEAT MULTIPLIERS (used for interpolation across star mass range)
const double HEAT_STAR_LOW_MULT = 0.8;   // at GASGIANTLIMIT (600)
const double HEAT_STAR_HIGH_MULT = 6.0;  // at STARLIMIT (4000)

//NEUTRON STAR SUBTYPES
const double PULSAR_ROTATION_SPEED = 0.02;
const double PULSAR_BEAM_LENGTH_MULT = 20.0;
const double PULSAR_BEAM_WIDTH = 0.08;            // half-angle in radians
const double MAGNETAR_PULSE_SPEED = 0.03;
const double MAGNETAR_GLOW_SIZE_MULT = 1.5;

//INITIAL REMNANT TEMPERATURES
const double INITIAL_TEMP_WHITEDWARF = 25000.0;
const double INITIAL_TEMP_NEUTRONSTAR = 500000.0;

//EVOLVED TYPE HEAT MULTIPLIERS
const double HEAT_BROWNDWARF_MULT = 0.1;
//EVOLVED TYPE DENSITIES
const double DENSITY_BROWNDWARF = 0.25;
const double DENSITY_STAR_GIANT = 0.05;
const double DENSITY_STAR_SUPERGIANT = 0.03;
const double DENSITY_WHITEDWARF = 0.9;
const double DENSITY_NEUTRONSTAR = 1.0;

//FUEL CONSTANTS
const double INITIAL_FUEL_PER_MASS = 100.0;
const double BROWNDWARF_FUEL_FRACTION = 0.04; // deuterium is ~4% of hydrogen fuel budget
const double BASE_FUEL_BURN_RATE = 0.0025;  // GUI multiplier 1x = this rate

//STELLAR GIANT PHASE
const double GIANT_PHASE_BEGIN = 0.35;      // fuel fraction where expansion begins
const double GIANT_PHASE_END = 0.10;        // fuel fraction where expansion reaches maximum
const double GIANT_PHASE_FUSION_BOOST = 15.0; // max fusion multiplier increase at full expansion

//NEW OBJECT MASS SLIDER
const double MASS_SLIDER_MIN_VALUE = 1;
const double MASS_SLIDER_MAX_VALUE = 4000;

//PLANETTHINGS
const int maxAtmoLayer = 15;
const int minAtmoLayer = 10;
const int brightnessVariance = 8;           //THE VARIANCE OF ATMOSPHERE BRIGHTNESS IN GASGIANTS
const double greenHouseEffectMult = 0.0008;
const double maxAtmo = 600;                 //MAX ATMOSPHERE
const double atmoThicknessMult = 0.05;      //HEIGHT OF ATMOSPHERE MULTIPLIER
const double atmoAlphaMult = 0.3;           //ALPHA OF ATMOSPHERE MULTIPLIER

// SHIP
const double SHIP_GAS_EJECT_SPEED = 0.06;
const double PROJECTILE_SPEED = 1.5;
const int PROJECTILE_LIFESPAN = 5000;
const int PROJECTILE_COOLDOWN = 40;
const double PROJECTILE_DAMAGE_MASS_LIMIT = 100;
const double PROJECTILE_MAX_CHARGE = 200;
const double PROJECTILE_CHARGE_SPEED = 0.04;
const double TUG_RANGE = 400.0;
const double TUG_STRENGTH = 0.0005;
const double TUG_MASS_LIMIT = 200;
const double TUG_PREFERRED_DISTANCE = 80.0;

//DUST
const double DUST_LIFESPAN_MIN = 40000.0;
const double DUST_LIFESPAN_MAX = 80000.0;
const size_t MAX_N_DUST_PARTICLES = 10000;
const int DUST_MIN_PHYSICS_SIZE = 15;

//COLLISIONS
const int COLLISION_HEAT_MULTIPLIER = 450000;
const float FLASH_SIZE = 1.8f;

//UI
const int UI_SEPERATION_DISTANCE = 5; //SEPERATION OF UI ELEMENTS

//PHYSICS DELTA TIME
const float TIMESTEP_VALUE_START = 10.0;
const float MAX_TIMESTEP = 50.0;

//TRAIL
const double TRAILRAD = 1.2;
const int TRAILLIFE = 10000;
const int TRAILFREQ = 10;

//ADV IN ORBIT ADDER
const double ADV_ORBIT_ADDER_SELECTED_MARKER_RAD_MULT = 1.4;
const double ADV_ORBIT_ADDER_SELECTED_MARKER_THICKNESS = 2;
const double ADV_ORBIT_ADDER_CENTER_MARKER_RAD = 3;

//LIFE
const double colony_growth_rate = 0.00001;
const double interstellar_growth_rate = 0.0003;
const int colony_expand_rate = 50000;
const int interstellar_expand_rate = 20000;
const int interstellar_min_size = 1000;
const double civilization_compact_constant = 500;
const int LIFE_PREFERRED_TEMP = 298;                  //KELVIN
const int LIFE_PREFERRED_ATMO = 300;                  //KILOPASCAL
const double LIFE_PREFERRED_TEMP_MULTIPLIER = 0.005;  //HOW MUCH THE TEMPERATURE DIFFERENCE FROM THE IDEAL IMPACTS LIFE
const double LIFE_PREFERRED_ATMO_MULTIPLIER = 0.0003; //HOW MUCH THE ATMO DIFFERENCE FROM THE IDEAL IMPACTS LIFE

//GOLDI LOCK ZONE
const double inner_goldi_temp = 323.15;
const double outer_goldi_temp = 223.15;
const int goldi_strength = 25;

//LIGHT
const int LIGHT_NUMBER_OF_VERTECES = 50;

//EXPLOSIONLIGHT
const int EXPLOSION_LIGHT_START_STRENGTH = 50;
const double EXPLOSION_FLASH_SIZE = 10;

//RNDM SYSTEM GENERATOR
const double MASS_MULTIPLIER = 130;
const double NUMBER_OF_OBJECT_MULTIPLIER = 0.4;

//BOUND
const int BOUND_MIN_RAD = 200;
const double BOUND_OUTSIDE_INDICATOR_SIZE = 20;
const double BOUND_THICKNESS = 20;
const double START_RADIUS = 8000;
const int BOUND_AUTO_UPDATE_RATE = 10;

//MISSILE
const double MISSILE_START_SPEED = 0.05;
const double MISSILE_MAX_SPEED = 0.4;
const double MISSILE_ACCELERATION = 0.0005;
const double MISSILE_TURN_SPEED = 0.002;
const int MISSILE_LIFESPAN = 3000;
const int MISSILE_LAUNCH_COOLDOWN = 300;
const double MISSILE_DETECTION_RANGE = 5000;

//SHIP SHIELD
const double SHIELD_MAX_ENERGY = 100;
const double SHIELD_DRAIN_EXPLOSION = 15.0;
const double SHIELD_DRAIN_MISSILE = 50.0;
const double SHIELD_RECOVERY_RATE = 0.05;
const int SHIELD_RECOVERY_PAUSE = 3000;

//TEMPERATURE LIMITS
const double MAX_TEMP = 1000000000000.0; // 1 Trillion Kelvin

//EXPLODE PLANET
const float EXPLODE_PLANET_SPEEDMULT_OTHER = 0.05f;

//STARSHINE FADE
const double STARSHINE_FADE_LIFETIME = 250.0;

//DISINTEGRATE PLANET
const double DISINTEGRATE_PLANET_SPEEDMULT = 0.125;
const double MIN_DT_DISINTEGRATE_GRACE_PERIOD = 2000.0;
const double MAX_DT_DISINTEGRATE_GRACE_PERIOD = 15000.0;