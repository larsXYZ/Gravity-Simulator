#pragma once
#include <iostream>
#include <vector>
#include "planet.h"
#include <random>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <TGUI/TGUI.hpp>
#include <SFML/System.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <string>
#include <sstream>
#include <algorithm>
#include "Spaceship.h"
#include "Effect.h"
#include "CONSTANTS.h"
#include "Bound.h"
#include <omp.h>
#include <fstream>
#include <iomanip>

class Rom
{
private:
	//CONSTANTS
	int xstorrelse;
	int ystorrelse;
	const static int framerate = FRAMERATE;
	double createPlanetSpeedmult = CREATEPLANETSPEEDMULT;
	double maxSize = MAXSIZEPLANETCREATOR;
	double minSize = MINSIZEPLANETCREATOR;
	double deltaMasse = DELTAMASSE;
	double timeStepInc = TIDSKRITTINC;


	//WINDOW
	double totalMass = 0;
	int xtrans = 0;
	int ytrans = 0;
	int xmidltrans = 0;
	int ymidltrans = 0;
	double zoom = 1;
	int xs;
	int ys;
	int nesteid = 0;
	double timeStep = TIMESTEP_VALUE_START;
	int iterasjon = 0;
	bool fullScreen;
	double size = 1;
	int tempEnhet = 1;
	int fps = 0;
	bool showGUI = true;
	sf::Vector2i mousePos = sf::Vector2i(0, 0);

	sf::Text text;
	sf::Text text2;
	sf::Font font;
	sf::Event event;

	//TOGGLES AND OTHER THINGS
	bool randomizeToggle = true;
	bool mouseToggle = false;
	bool transToggle = true;
	bool eksplosjonToggle = true;
	bool updatetoggle = true;

	bool drawtext2 = false;
	bool showText = true;
	int fokusId = -1;
	int lockToObjectId = -2;
	bool mouseOnWidgets = false;
	bool mouseOnMassSliderSelected = false;

	//OBJECTS IN THE SIMULATION
	SpaceShip ship;
	std::vector<Planet> pListe;
	std::vector<int> midlPListe;
	std::vector<Explosion> expListe;
	std::vector<Smoke> smkListe;
	std::vector<Trail> trlListe;
	Bound bound;

	//GUI
	tgui::TextArea::Ptr simInfo = std::make_shared<tgui::TextArea>();
	tgui::TextArea::Ptr newPlanetInfo = std::make_shared<tgui::TextArea>();
	tgui::TextArea::Ptr currPlanetInfo = std::make_shared<tgui::TextArea>();
	tgui::ListBox::Ptr functions = std::make_shared<tgui::ListBox>();
	tgui::Tabs::Ptr tempChooser = std::make_shared<tgui::Tabs>();
	tgui::Slider::Ptr massSlider = std::make_shared<tgui::Slider>();
	tgui::Slider::Ptr timeStepSlider = std::make_shared<tgui::Slider>();
	tgui::Slider::Ptr massExistingObjectSlider = std::make_shared<tgui::Slider>();
	tgui::CheckBox::Ptr autoBound = std::make_shared<tgui::CheckBox>();

public:

	Rom(int x, int y, bool f);

	//PLANET FUNCTIONS
	void addPlanet(Planet p);
	void printPListe();
	std::vector<Planet> getPListe();
	void removePlanet(int index);
	void removeExplosion(int ind);
	void removeSmoke(int ind);
	void removeTrail(int ind);
	void clear(sf::View& v, sf::Window& w);
	void explodePlanet(int ind);
	void explodePlanetOld(int ind);
	void randomPlaneter(int totmass, int antall,double radius, sf::Vector2f pos);
	void addExplosion(sf::Vector2f p, double s, sf::Vector2f v, int l);
	void addSmoke(sf::Vector2f p, double s, sf::Vector2f v, int l);
	void addTrail(sf::Vector2f p, int l);
	void GravitySmoke(Planet& forcer, int t);
	void giveRings(Planet p, int inner, int outer);
	
	//SIMULATION FUNCTIONS
	void update();
	void runSim();
	void PlanetSkjermPrint(sf::RenderWindow &window);
	void effectSkjermPrint(sf::RenderWindow & window);
	void lightSkjermPrint(sf::RenderWindow& window);
	void giveId(Planet &p);
	Planet findPlanet(double id);
	Planet& findPlanetRef(double id);
	int findBestPlanet(int q);
	void romskipHandling();
	double getTherEnergyAtPos(sf::Vector2f pos);
	sf::Vector3f centerOfMass(std::vector<int> midlPList);
	sf::Vector2f centerOfMassAll();
	sf::Vector2f centerOfMassVelocity(std::vector<int> midlPList);

	//USERFUNCTIONS
	void hotkeys(sf::Window& w, sf::View& v)
	{
		(void) w;

		//ZOOM
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && zoom < 15)
		{
			zoom = zoom*1.05;
			v.zoom(1.05);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) && zoom > 0.03)
		{
			zoom = zoom / 1.05;
			v.zoom(1 / 1.05);
		}

		//TIMESTEP
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma) && timeStepSlider->getValue()>timeStepSlider->getMinimum())
		{
			timeStepSlider->setValue(timeStepSlider->getValue() - 1);
			if (timeStepSlider->getValue() < timeStepSlider->getMinimum()) timeStepSlider->setValue(timeStepSlider->getMinimum());
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period) && timeStepSlider->getValue()<timeStepSlider->getMaximum())
		{
			timeStepSlider->setValue(timeStepSlider->getValue() + 1);
			if (timeStepSlider->getValue() > timeStepSlider->getMaximum()) timeStepSlider->setValue(timeStepSlider->getMaximum());
		}

		//FUNCTIONS
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
		{
			functions->setSelectedItem("Object (F)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
		{
			functions->setSelectedItem("Object in orbit (O)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			functions->setSelectedItem("Adv Object in orbit (S)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			functions->setSelectedItem("Remove object (D)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
		{
			functions->setSelectedItem("Explode object (C)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
		{
			functions->setSelectedItem("Random system (G)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		{
			functions->setSelectedItem("Rings (Q)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			functions->setSelectedItem("Spawn ship (E)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::I))
		{
			functions->setSelectedItem("Info (I)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::T))
		{
			functions->setSelectedItem("Follow object (T)");
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
		{
			functions->setSelectedItem("Bound (B)");
		}
	}
	void lockToObject(sf::RenderWindow& w, sf::View& v);

	//GUI
	void initSetup();
	void setInfo();
	void printInfoPlanet(sf::RenderWindow& w, sf::View& v);

	//OTHER
	int modernRandomWithLimits(int min, int max)
	{
		std::random_device seeder;
		std::default_random_engine generator(seeder());
		std::uniform_int_distribution<int> uniform(min, max);
		return uniform(generator);
	}
	static std::string convertDoubleToString(double number)
	{
		std::string Result;

		std::stringstream convert;

		convert << std::setprecision(5) << number;

		return convert.str();
	}
	static double convertStringToDouble(std::string string)
	{
		double result;

		std::stringstream convert;

		convert << string;

		convert >> result;

		return result;
	}
	double range(double x1, double y1, double x2, double y2);
	std::string calcTemperature(double q, int e);
};
