#include "Life.h"

Life::Life() : 
    biomass(0),
    type(NONE),
    lifeLevel(0),
    expand(false),
    description(""),
    civName(""),
    timer(0),
    lifeColor(sf::Color(modernRandomWithLimits(0, 255), 
                       modernRandomWithLimits(0, 255), 
                       modernRandomWithLimits(0, 255)))
{
}

Life::Life(int i) :
    id(i),
    biomass(100),
    type(COLONY),
    lifeLevel(7),
    expand(false),
    description(""),
    civName(""),
    timer(0),
    lifeColor(sf::Color(modernRandomWithLimits(0, 255), 
                       modernRandomWithLimits(0, 255), 
                       modernRandomWithLimits(0, 255)))
{
}

void Life::giveId(int i) {
    id = i;
}

void Life::giveCol(sf::Color c) {
    lifeColor = c;
}

void Life::giveDesc(std::string d) {
    description = d;
}

void Life::giveCivName(std::string cn) {
    civName = cn;
}

void Life::setLifeLevel(lType level) {
    if (level < 0) level = lType::NONE;
    if (level > 7) level = lType::COLONY;
    
    lifeLevel = level;
    type = static_cast<lType>(lifeLevel);
    
    // Ensure some biomass for the level
    double minBiomass = 0;
    if (level == 1) minBiomass = 100;
    else if (level == 2) minBiomass = 500;
    else if (level == 3) minBiomass = 1000;
    else if (level == 4) minBiomass = 2000;
    else if (level == 5) minBiomass = 5000;
    else if (level == 6) minBiomass = 10000;
    else if (level == 7) minBiomass = 100; // Colony starts small

    if (biomass < minBiomass) biomass = minBiomass;

    if (lifeLevel >= 4) {
        if (description.empty()) genDesc();
        if (civName.empty()) genCivName();
    }

    if (lifeLevel == 0) {
        biomass = 0;
        description = "";
        civName = "";
    }
}

void Life::update(double supportedBM, int t, double rad) {
    expand = false;

    if (lifeLevel < 6) {
        // GENESIS
        if (lifeLevel == 0) {
            if (modernRandomWithLimits(0, 2000000) < supportedBM) lifeLevel = 1;
            return;
        }

        // LIFE GROWS/DIMINISHES AS THE SUPPORTED BIOMASS CHANGES
        if (biomass < supportedBM) {
            biomass += t * 0.0003 * (supportedBM - biomass);
        } else if (biomass > supportedBM) {
            biomass += t * 0.01 * (supportedBM - biomass);
        }

        // EVOLVE
        if (modernRandomWithLimits(0, 3500000 * lifeLevel) < (biomass - 900 * lifeLevel) && lifeLevel < 6) {
            lifeLevel++;
            if (lifeLevel == 4) {
                genDesc();
                genCivName();
            }
            if (lifeLevel == 6) {
                expand = true;
            }
        }

        // DEVOLVE
        if (supportedBM < 450 * (lifeLevel - 1)) lifeLevel--;
        if (supportedBM == 0) {
            lifeLevel = 0;
            biomass = 0;
        }
    } else if (lifeLevel == 6) {
        // COUNTING
        timer += t;

        // CIVILIZATION GROWS ACCORDING TO HOW BIG THE PLANET IS
        if (biomass < civilization_compact_constant * rad) {
            double natural_growth = t * 0.0003 * (supportedBM - biomass);
            if (natural_growth > t * interstellar_growth_rate) {
                biomass += natural_growth;
            } else {
                biomass += t * interstellar_growth_rate * (civilization_compact_constant * rad * rad * rad - biomass);
            }
        } else if (biomass > civilization_compact_constant * rad) {
            biomass += t * interstellar_growth_rate * (civilization_compact_constant * rad - biomass);
        }

        // COLONIZING
        if (timer > interstellar_expand_rate) {
            expand = true;
            timer = 0;
        }
    } else if (lifeLevel == 7) {
        // COUNTING
        timer += t;

        // GROWING WITH TIME
        double natural_growth = t * 0.0003 * (supportedBM - biomass);
        if (natural_growth > t * colony_growth_rate) {
            biomass += natural_growth;
        } else {
            biomass += t * colony_growth_rate * (civilization_compact_constant * rad - biomass);
        }

        // COLONIZING
        if (timer > colony_expand_rate) {
            expand = true;
            timer = 0;
        }

        // GROWS
        if (biomass > interstellar_min_size) lifeLevel = 6;
    }

    type = static_cast<lType>(lifeLevel);
}

void Life::kill() {
    biomass = 0;
    type = NONE;
    lifeLevel = 0;
    description = "";
    civName = "";
}

bool Life::willExp() const noexcept {
    return expand;
}

void Life::genDesc() {
    static const std::vector<std::string> startAdj = { 
        "Flat", "Tall", "Wide", "Slimy", "Scaly", "Small", "Tiny", "Big", "Bioluminescent",
        "Spiky", "Armored", "Gelatinous", "Transparent", "Hairy", "Glowing", "Soft",
        "Metallic", "Rubbery", "Feathered", "Crystalline", "Gaseous", "Multi-limbed"
    };
    static const std::vector<std::string> creature = { 
        "fishes", "amphibians", "reptiles", "birds", "mammals", "insects", "snails", 
        "arachnids", "crustaceans", "worms", "fungi-like organisms", "mollusks",
        "cephalopods", "slugs", "polyps", "jellies", "tentacled beasts", "hexapods"
    };
    static const std::vector<std::string> area = { 
        "in caves", "deep underground", "floating in organic balloons", "on the seabed", 
        "in self made structures", "drifting in the oceans", "on the sides of cliffs", 
        "near active volcanoes", "in the upper atmosphere", "within toxic swamps",
        "under thick ice sheets", "on floating islands", "inside geothermal vents",
        "within giant crystalline forests", "across endless desert dunes",
        "in radioactive craters", "attached to massive fungal towers",
        "in high-gravity plains"
    };

    description = startAdj[modernRandomWithLimits(0, (int)startAdj.size() - 1)] + " " +
                 creature[modernRandomWithLimits(0, (int)creature.size() - 1)] + " " +
                 area[modernRandomWithLimits(0, (int)area.size() - 1)];
}

void Life::genCivName() {
    static const std::vector<std::string> part1 = { 
        "Gloo", "Ble", "Kri", "Zor", "Pla", "Xi", "Vora", "Sali", "Grom", "Trak", 
        "Mora", "Siv", "Ocr", "Ura", "Zet", "Kla", "Vex", "Dra", "Loo", "Trel"
    };
    static const std::vector<std::string> part2 = { 
        "bi", "to", "a", "i", "e", "u", "o", "ra", "si", "lo", "va", "ne", "mu", "ka"
    };
    static const std::vector<std::string> part3 = { 
        "ans", "oids", "ish", "ites", "ons", "erons", "alians", "ians", "ids", "ods"
    };

    civName = part1[modernRandomWithLimits(0, (int)part1.size() - 1)] + 
              part2[modernRandomWithLimits(0, (int)part2.size() - 1)] + 
              part3[modernRandomWithLimits(0, (int)part3.size() - 1)];
}

lType Life::getTypeEnum() const {
    return type;
}

double Life::getBmass() const {
    return biomass;
}

std::string Life::getType() const {
    switch (type) {
        case NONE:
            return "Lifeless";
        case SINGLECELL:
            return "Unicellular organisms";
        case MULTICELL_SIMPLE:
            return "Multicellular organisms";
        case MULTICELL_COMPLEX:
            return "Complex multicellular organisms";
        case INTELLIGENT_TRIBAL:
            return "Tribal communities";
        case INTELLIGENT_GLOBAL:
            return "Globalized civilization";
        case INTELLIGENT_INTERPLANETARY:
            return "Interplanetary civilization";
        case COLONY:
            return "Colony";
        default:
            return "Kraken";
    }
}

int Life::getId() const {
    return id;
}

sf::Color Life::getCol() const {
    return lifeColor;
}

std::string Life::getDesc() const {
    return description;
}

std::string Life::getCivName() const {
    return civName;
}

int Life::modernRandomWithLimits(int min, int max) {
    static std::random_device seeder;
    static std::default_random_engine generator(seeder());
    std::uniform_int_distribution<int> uniform(min, max);
    return uniform(generator);
} 