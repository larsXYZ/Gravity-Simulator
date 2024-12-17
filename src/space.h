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

class Space
{
private:
	//CONSTANTS
	int xsize;
	int ysize;
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
	int iteration = 0;
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

	Space(int x, int y, bool f);

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
	void randomPlanets(int totmass, int antall,double radius, sf::Vector2f pos);
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
	void drawLightEffects(sf::RenderWindow& window);
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
	void hotkeys(sf::Window& w, sf::View& v);
	void lockToObject(sf::RenderWindow& w, sf::View& v);

	//GUI
	void initSetup();
	void setInfo();
	void printInfoPlanet(sf::RenderWindow& w, sf::View& v);

	//OTHER
	int modernRandomWithLimits(int min, int max);
	static std::string convertDoubleToString(double number);
	static double convertStringToDouble(std::string string);
	double range(double x1, double y1, double x2, double y2);
	std::string calcTemperature(double q, int e);
};
