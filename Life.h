#pragma once
#include "CONSTANTS.h"
#include <cmath>
#include <vector>



class Life
{
private:

	int id;
	double biomass;
	lType type;
	int lifeLevel;
	bool expand;
	std::string description = "";
	int timer = 0;
	sf::Color lifeColor = sf::Color(modernRandomWithLimits(0, 255), modernRandomWithLimits(0, 255), modernRandomWithLimits(0, 255));

public:

	//CONSTRUCTORS
	Life()
	{
		biomass = 0;
		type = NONE;
		lifeLevel = 0;
		expand = false;
	}
	Life(int i)
	{
		id = i;
		biomass = 100;
		type = COLONY;
		lifeLevel = 7;
		expand = false;
	}

	//LIFE FUNCTIONS
	void giveId(int i)
	{
		id = i;
	}
	void giveCol(sf::Color c)
	{
		lifeColor = c;
	}
	void giveDesc(std::string d)
	{
		description = d;
	}
	void update(double supportedBM, int t,double rad)
	{
		expand = false;


		if (lifeLevel < 6)
		{
			//GENESIS
			if (lifeLevel == 0)
			{
				if (modernRandomWithLimits(0, 2000000) < supportedBM) lifeLevel = 1;
				return;
			}

			//LIFE GROWS/DIMINISHES AS THE SUPPORTED BIOMASS CHANGES
			if (biomass < supportedBM) biomass += t*0.0003*(supportedBM - biomass);
			else if (biomass > supportedBM) biomass += t*0.01*(supportedBM - biomass);

			//EVOLVE
			if (modernRandomWithLimits(0, 3500000 * (lifeLevel)) < (biomass - 900 * (lifeLevel)) && lifeLevel < 6)
			{
				lifeLevel++;
				if (lifeLevel == 4) genDesc();

				if (lifeLevel == 6)
				{
					expand = true;
				}
			}

			//DEVOLVE
			if (supportedBM < 450 * (lifeLevel - 1)) lifeLevel--;
			if (supportedBM == 0)
			{
				lifeLevel = 0;
				biomass = 0;
			}
		}
		else if (lifeLevel == 6)
		{
			//COUNTING
			timer += t*1;

			//CIVILIZATION GROWS ACCORDING TO HOW BIG THE PLANET IS, THEY DON'T REALLY CARE HOW THE CONDITIONS ARE... THEY'RE WICKED SMART
			if (biomass < civilization_compact_constant*rad)
			{
				double natural_growth = t*0.0003*(supportedBM - biomass);

				if (natural_growth > t*interstellar_growth_rate) biomass += natural_growth;
				else biomass += t*interstellar_growth_rate*(civilization_compact_constant*rad*rad*rad - biomass);
			}
			else if (biomass > civilization_compact_constant*rad)
			{
				biomass += t*interstellar_growth_rate*(civilization_compact_constant*rad - biomass);
			}

			//COLONIZING
			if (timer > interstellar_expand_rate)
			{
				expand = true;
				timer = 0;
			}

		}
		else if (lifeLevel == 7)
		{
			//COUNTING
			timer += t*1;

			//GROWING WITH TIME
			double natural_growth = t*0.0003*(supportedBM - biomass);
			if (natural_growth >  t*colony_growth_rate) biomass += natural_growth;
			else biomass += t*colony_growth_rate*(civilization_compact_constant*rad-biomass);


			//COLONIZING
			if (timer > colony_expand_rate)
			{
				expand = true;
				timer = 0;
			}

			//GROWS
			if (biomass > interstellar_min_size) lifeLevel = 6;
		}


		type = (lType) lifeLevel;
	}
	void kill()
	{
		biomass = 0;
		type = NONE;
	}
	bool willExp()
	{
		return expand;
	}
	void genDesc()
	{
		std::vector<std::string> startAdj = { "Flat", "Tall", "Wide", "Slimy", "Scaly", "Small", "Tiny", "Big" };
		std::vector<std::string> creature = { "fishes", "amphibians", "reptiles", "birds", "mammals", "insects", "snails", "arachnids" };
		std::vector<std::string> area = { "in caves", "deep underground", "floating in organic ballons", "on the seabed", "in self made structures", "drifting in the oceans", "on the sides of cliffs", "near active volcanoes"};
		description = startAdj[modernRandomWithLimits(0, startAdj.size() - 1)] + " " + creature[modernRandomWithLimits(0, creature.size() - 1)] + " " + area[modernRandomWithLimits(0, area.size() - 1)];

	}

	//GET FUNCTIONS
	lType getTypeEnum()
	{
		return type;
	}
	double getBmass()
	{
		return biomass;
	}
	std::string getType()
	{
		switch (type)
		{
		case NONE:
		{
			return "Lifeless";
		}
		case SINGLECELL:
		{
			return "Unicellular organisms";
		}
		case MULTICELL_SIMPLE:
		{
			return "Multicellular organisms";
		}
		case MULTICELL_COMPLEX:
		{
			return "Complex multicellular organisms";
		}
		case INTELLIGENT_TRIBAL:
		{
			return "Tribal communities";
		}
		case INTELLIGENT_GLOBAL:
		{
			return "Globalized civilization";
		}
		case INTELLIGENT_INTERPLANETARY:
		{
			return "Interplanetary civilization";
		}
		case COLONY:
		{
			return "Colony";
		}

		}
	}
	int getId()
	{
		return id;
	}
	sf::Color getCol()
	{
		return lifeColor;
	}
	std::string getDesc()
	{
		return description;
	}

	int modernRandomWithLimits(int min, int max)
	{
		std::random_device seeder;
		std::default_random_engine generator(seeder());
		std::uniform_int_distribution<int> uniform(min, max);
		return uniform(generator);
	}
};