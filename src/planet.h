#pragma once
#include <cmath>
#include <math.h>
#include "CONSTANTS.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include "Life.h"

class Planet
{
	std::string name = genName();

	//PHYSICAL
	double mass;
	double x;
	double y;
	double xv;
	double yv;
	double radi;
	double density;
	pType planetType;
	double temperature = 0;

	//TEMPERATURE
	double tCapacity = 1;
	double tEnergy;

	//ATMOSPHERE
	double atmoCur = 0;
	double atmoPot = modernRandomWithLimits(0, maxAtmo);
	int numAtmoLines;
	std::vector<int> atmoLinesBrightness;
	double atmoCol_r = modernRandomWithLimits(0, 170);
	double atmoCol_g = modernRandomWithLimits(0, 170);
	double atmoCol_b = modernRandomWithLimits(0, 170);

	//LIFE
	Life life;
	double supportedBiomass = 0;

	//OTHER
	double id;
	int ID_strongest_attractor;
	double STRENGTH_strongest_attractor = 0;
	bool marked_for_removal = false;

	//FOR DISINTEGRATION AND IGNORING
	double disintegrate_grace_end_time = 0;
	std::vector<int> ignore_ids;

	//GRAPHICS
	sf::CircleShape circle;
	int randBrightness;
	sf::VertexArray light;

public:
	//CONSTRUCTORS
	Planet()
	{
		mass = 6;
		x = 0;
		y = 0;
		xv = 0;
		yv = 0;

		tEnergy = 0;

		randBrightness = modernRandomWithLimits(-30, +30);
		updateRadiAndType();
		circle.setPosition(x, y);

		//DETERMINING NUMBER OF ATMOSPHERE LINES, FOR GASGIANT PHASE
		numAtmoLines = modernRandomWithLimits(minAtmoLayer, maxAtmoLayer);
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(
			modernRandomWithLimits(-brightnessVariance, brightnessVariance));
	}

	Planet(double m)
	{
		mass = m;
		x = 0;
		y = 0;
		xv = 0;
		yv = 0;

		tEnergy = 0;

		randBrightness = modernRandomWithLimits(-30, +30);
		updateRadiAndType();
		circle.setPosition(x, y);

		//DETERMINING NUMBER OF ATMOSPHERE LINES, FOR GASGIANT PHASE
		numAtmoLines = modernRandomWithLimits(minAtmoLayer, maxAtmoLayer);
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(
			modernRandomWithLimits(-brightnessVariance, brightnessVariance));
	}

	Planet(double m, double xx, double yy)
	{
		mass = m;
		x = xx;
		y = yy;
		xv = 0;
		yv = 0;

		tEnergy = 0;

		randBrightness = modernRandomWithLimits(-30, +30);
		updateRadiAndType();
		circle.setPosition(x, y);

		//DETERMINING NUMBER OF ATMOSPHERE LINES, FOR GASGIANT PHASE
		numAtmoLines = modernRandomWithLimits(minAtmoLayer, maxAtmoLayer);
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(
			modernRandomWithLimits(-brightnessVariance, brightnessVariance));
	}

	Planet(double m, double xx, double yy, double xvv, double yvv)
	{
		mass = m;
		x = xx;
		y = yy;
		xv = xvv;
		yv = yvv;

		tEnergy = 0;

		randBrightness = modernRandomWithLimits(-30, +30);
		updateRadiAndType();
		circle.setPosition(x, y);

		//DETERMINING NUMBER OF ATMOSPHERE LINES, FOR GASGIANT PHASE
		numAtmoLines = modernRandomWithLimits(minAtmoLayer, maxAtmoLayer);
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(
			modernRandomWithLimits(-brightnessVariance, brightnessVariance));
	}

	double getx() const;
	double gety() const;
	double getxv() const;
	double getyv() const;
	double getmass() const;
	double getRad() const;
	pType getType() const;
	double getId() const;

	int getStrongestAttractorId() const
	{
		return ID_strongest_attractor;
	}

	int getStrongestAttractorIdRef() const
	{
		return ID_strongest_attractor;
	}

	void setStrongestAttractorIdRef(int id)
	{
		ID_strongest_attractor = id;
	}

	std::string getName() const
	{
		return name;
	}

	bool emitsHeat() const;

	std::string getFlavorTextLife() const;

	sf::Color getStarCol() const;

	void markForRemoval() { marked_for_removal = true; }
	bool isMarkedForRemoval() const { return marked_for_removal; }
	double getStrongestAttractorStrength() { return STRENGTH_strongest_attractor; }
	void setStrongestAttractorStrength(double strength) { STRENGTH_strongest_attractor = strength; }

	bool canDisintegrate(double curr_time) const;
	void setDisintegrationGraceTime(double grace_time, double curr_time);
	bool disintegrationGraceTimeIsActive(double curr_time) const;
	bool disintegrationGraceTimeOver(double curr_time) const;
	void registerIgnoredId(int id);
	void clearIgnores();
	bool isIgnoring(int id) const;
	
	void becomeAbsorbedBy(Planet& absorbing_planet);
	
	//LIFE FUNCTIONS
	
	void setx(double x);
	void sety(double y);
	void updateRadiAndType();

	void resetAttractorMeasure()
	{
		STRENGTH_strongest_attractor = 0;
	}

	void draw(sf::RenderWindow& w, double xx, double yy);
	void incMass(double m);
	double getDist(const Planet& forcer) const;
	void collision(const Planet& p);
	void mark(double i);
	void updateTemp();

	//TEMPERATURE
	double temp() const
	{
		return tEnergy / (mass * tCapacity);
	}

	double getTemp() const
	{
		return temperature;
	}

	void setTemp(double t)
	{
		tEnergy = mass * t * tCapacity;
	}

	double fusionEnergy() const
	{
		switch (planetType)
		{
		case ROCKY:
			return 0;
		case TERRESTIAL:
			return 0;
		case GASGIANT:
			return 0;
		case SMALLSTAR:
			return HEAT_SMALL_STAR_MULT * mass;
		case STAR:
			return HEAT_STAR_MULT * mass;
		case BIGSTAR:
			return HEAT_BIG_STAR_MULT * mass;
		default:
			return 0;
		}
	}

	double thermalEnergy() const
	{
		return tEnergy;
	}

	void coolDOWN(int t)
	{
		tEnergy -= t * (SBconst * (radi * radi * temp()) - fusionEnergy());
	}

	void absorbHeat(double e, int t)
	{
		tEnergy += (e * (1 + greenHouseEffectMult * atmoCur));
	}

	double giveTEnergy(int t) const
	{
		return t * (SBconst * (radi * radi * temp()));
	}

	void incTEnergy(double e)
	{
		tEnergy += e;
	}

	void setColor();
	double getTCap() const { return tCapacity; };
	void setMass(double m) { mass = m; }

	//ATMOSPHERE
	void updateAtmo(int t)
	{
		if (planetType != TERRESTIAL)
		{
			if (planetType == ROCKY)
			{
				atmoCur = 0;
				return;
			}
			return;
		}

		if (temperature < 600 && temperature > 200 && atmoCur < atmoPot)
		{
			atmoCur += t * 0.05;
			if (atmoCur > atmoPot) atmoCur = atmoPot;
		}
		else
		{
			atmoCur -= t * 0.1;

			if (atmoCur < 0) atmoCur = 0;
		}

		circle.setOutlineColor(sf::Color(atmoCol_r, atmoCol_g, atmoCol_b, atmoCur * atmoAlphaMult));
		circle.setOutlineThickness(sqrt(atmoCur) * atmoThicknessMult);
	}

	double getAtmoCur() const
	{
		return atmoCur;
	}

	double getAtmoPot() const
	{
		return atmoPot;
	}

	//LIFE
	void updateLife(int t)
	{
		if (planetType == ROCKY || planetType == TERRESTIAL)
		{
			supportedBiomass = 100000 / (1 + (LIFE_PREFERRED_TEMP_MULTIPLIER *
				pow((temperature - LIFE_PREFERRED_TEMP), 2) + LIFE_PREFERRED_ATMO_MULTIPLIER * pow(
					(atmoCur - LIFE_PREFERRED_ATMO), 2))) - 5000;
			if (supportedBiomass < 0) supportedBiomass = 0;

			life.update(supportedBiomass, t, radi);
		}
		else
		{
			life.kill();
		}
	}

	void colonize(int i, sf::Color c, std::string d)
	{
		life = Life(i);
		life.giveCol(c);
		life.giveDesc(d);
	}

	Life getLife() const
	{
		return life;
	}

	double getSupportedBiomass() const
	{
		return supportedBiomass;
	}

	int modernRandomWithLimits(int min, int max) const
	{
		std::random_device seeder;
		std::default_random_engine generator(seeder());
		std::uniform_int_distribution<int> uniform(min, max);
		return uniform(generator);
	}

	std::string genName();
	
	void setxv(double v);
	void setyv(double v);
};
