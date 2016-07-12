#pragma once
#include <cmath>
#include <math.h>
#include <iostream>
#include "CONSTANTS.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <random>
#include <sstream>
#include <vector>
#include "Life.h"

class Planet
{
private:

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
	bool killState = false;

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
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(modernRandomWithLimits(-brightnessVariance, brightnessVariance));
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
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(modernRandomWithLimits(-brightnessVariance, brightnessVariance));
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
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(modernRandomWithLimits(-brightnessVariance, brightnessVariance));
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
		for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(modernRandomWithLimits(-brightnessVariance, brightnessVariance));
	}


	//GET FUNCTIONS
	double getx();
	double gety();
	double& getxv();
	double& getyv();
	double getmass();
	void printInfo();
	void printInfoShort();
	double getRad();
	pType getType();
	double getId();
	double getG();
	int getStrongestAttractorId()
	{
		return ID_strongest_attractor;
	}
	int& getStrongestAttractorIdRef()
	{
		return ID_strongest_attractor;
	}
	std::string getName()
	{
		return name;
	}
	std::string getFlavorTextLife();
	sf::Color getStarCol()
	{
		if (temperature < 1600) return (sf::Color(255, 38, 00));
		else if (temperature < 2400) return (sf::Color(255, 118, 00));
		else if (temperature < 3000) return (sf::Color(255, 162, 60));
		else if (temperature < 3700) return (sf::Color(255, 180, 107));
		else if (temperature < 4500) return (sf::Color(255, 206, 146));
		else if (temperature < 5500) return (sf::Color(255, 219, 186));
		else if (temperature < 6500) return (sf::Color(255, 238, 222));
		else if (temperature < 7200) return (sf::Color(255, 249, 251));
		else if (temperature < 8000) return (sf::Color(240, 241, 255));
		else if (temperature < 9000) return (sf::Color(227, 233, 255));
		else if (temperature < 10000) return (sf::Color(214, 225, 255));
		else if (temperature < 11000) return (sf::Color(207, 218, 255));
		else if (temperature < 12000) return (sf::Color(200, 213, 255));
		else if (temperature < 13000) return (sf::Color(191, 211, 255));
		else return (sf::Color(186, 208, 255));
	}
	bool& killStateRef() { return killState; }
	double& strAttr() { return STRENGTH_strongest_attractor; }

	//LIFE FUNCTIONS

	void updateVel(Planet forcer, double timeStep);
	void move(double timeStep);
	void updateRadiAndType();
	void resetAttractorMeasure()
	{
		STRENGTH_strongest_attractor = 0;
	}
	void draw(sf::RenderWindow &w, double xx , double yy);
	void incMass(double m);
	double getDist(Planet forcer);
	void kollisjon(Planet p);
	void mark(double i);
	void updateTemp();

	//TEMPERATURE
	double temp()
	{
		return tEnergy / (mass*tCapacity);
	}
	double getTemp()
	{
		return temperature;
	}
	void setTemp(double t)
	{
		tEnergy = mass*t*tCapacity;
	}
	double fusionEnergy()
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
		}
	}
	double thermalEnergy()
	{
		return tEnergy;
	}
	void coolDOWN(int t)
	{
		tEnergy -= t*(SBconst*(radi*radi*temp()) - fusionEnergy());
	}
	void heatUP(double e, int t)
	{
		tEnergy += (e*(1+greenHouseEffectMult*atmoCur));
	}
	double giveTEnergy(int t)
	{
		return t*(SBconst*(radi*radi*temp()));
	}
	void incTEnergy(double e)
	{
		tEnergy += e;
	}
	void setColor();
	double getTCap() { return tCapacity; };
	void setMass(double m) { mass = m; }

	//ATMOSPHERE
	void updateAtmo(int t)
	{
		if (planetType != TERRESTIAL)
		{
			if (planetType == ROCKY)
			{
				atmoCur == 0;
				return;
			}
			else
			{
				return;
			}
		}

		if (temperature < 600 &&temperature > 200 && atmoCur < atmoPot)
		{
			atmoCur += t*0.05;
			if (atmoCur > atmoPot) atmoCur = atmoPot;
		}
		else
		{
			atmoCur -= t*0.1;

			if (atmoCur < 0) atmoCur = 0;
		}

		circle.setOutlineColor(sf::Color(atmoCol_r, atmoCol_g, atmoCol_b, atmoCur*atmoAlphaMult));
		circle.setOutlineThickness(sqrt(atmoCur)*atmoThicknessMult);

	}
	double getAtmoCur()
	{
		return atmoCur;
	}
	double getAtmoPot()
	{
		return atmoPot;
	}

	//LIFE
	void updateLife(int t)
	{
		if (planetType == ROCKY || planetType == TERRESTIAL)
		{
			supportedBiomass = 100000 / (1 + (LIFE_PREFERRED_TEMP_MULTIPLIER*pow((temperature - LIFE_PREFERRED_TEMP),2) + LIFE_PREFERRED_ATMO_MULTIPLIER*pow((atmoCur - LIFE_PREFERRED_ATMO),2))) - 5000;
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

	Life getLife()
	{
		return life;
	}
	double getSupportedBiomass()
	{
		return supportedBiomass;
	}

	int modernRandomWithLimits(int min, int max)
	{
		std::random_device seeder;
		std::default_random_engine generator(seeder());
		std::uniform_int_distribution<int> uniform(min, max);
		return uniform(generator);
	}
	std::string genName();

};
