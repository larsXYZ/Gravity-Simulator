#pragma once

#include <vector>
#include <random>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "planet.h"
#include "Spaceship.h"
#include "Effect.h"
#include "CONSTANTS.h"
#include "Bound.h"
#include "click_and_drag.h"
#include "object_tracker.h"
#include "particles/particle_container.h"



class Space
{
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
	int xs;
	int ys;
	int next_id = 0;
	double timeStep = TIMESTEP_VALUE_START;
	double curr_time = 0;
	int iteration = 0;
	bool fullScreen;
	double size = 1;
	int tempEnhet = 1;
	int fps = 0;
	bool showGUI = true;

	sf::Text text;
	sf::Text text2;
	sf::Font font;
	sf::Event event;

	//OBJECTS IN THE SIMULATION
	SpaceShip ship;
	std::vector<Planet> planets;
	std::unique_ptr<IParticleContainer> particles;
	std::vector<int> temp_planet_ids;
	std::vector<Explosion> explosions;
	std::vector<Trail> trail;
	Bound bound;

	//GUI
	tgui::TextArea::Ptr simInfo = std::make_shared<tgui::TextArea>();
	tgui::TextArea::Ptr newPlanetInfo = std::make_shared<tgui::TextArea>();
	tgui::TextArea::Ptr currPlanetInfo = std::make_shared<tgui::TextArea>();
	tgui::ListBox::Ptr functions = std::make_shared<tgui::ListBox>();
	tgui::Tabs::Ptr temperatureUnitSelector = std::make_shared<tgui::Tabs>();
	tgui::Slider::Ptr massSlider = std::make_shared<tgui::Slider>();
	tgui::Slider::Ptr timeStepSlider = std::make_shared<tgui::Slider>();
	tgui::Slider::Ptr massExistingObjectSlider = std::make_shared<tgui::Slider>();
	tgui::CheckBox::Ptr autoBound = std::make_shared<tgui::CheckBox>();
	ClickAndDragHandler click_and_drag_handler;
	ObjectTracker object_tracker;

public:

	explicit Space(int x, int y, bool f);

	//PLANET FUNCTIONS
	int addPlanet(Planet&& p);
	void removePlanet(const int id);
	void removeExplosion(int ind);
	void removeSmoke(int ind);
	void removeTrail(int ind);
	void full_reset(sf::View& view);

	void disintegratePlanet(Planet planet);	/* No reference due to addition of new planets possibly invalidating references */
	void explodePlanet(Planet planet);	/* No reference due to addition of new planets possibly invalidating references */

	void randomPlanets(int totmass, int antall, double radius, sf::Vector2f pos);
	void addExplosion(sf::Vector2f p, double s, sf::Vector2f v, int l);
	void addSmoke(sf::Vector2f p, sf::Vector2f v, double s, double lifespan);
	void addTrail(sf::Vector2f p, int l);
	void giveRings(const Planet & planet, int inner, int outer);
	
	//SIMULATION FUNCTIONS
	void update();
	void runSim();
	void drawPlanets(sf::RenderWindow &window);
	void drawEffects(sf::RenderWindow & window);
	void drawLightEffects(sf::RenderWindow& window);
	void giveId(Planet &p);
	Planet findPlanet(int id);
	Planet* findPlanetPtr(int id);
	int findBestPlanet(int q);
	void updateSpaceship();
	double thermalEnergyAtPosition(sf::Vector2f pos);
	sf::Vector3f centerOfMass(std::vector<int> midlPList);
	sf::Vector2f centerOfMassAll();
	sf::Vector2f centerOfMassVelocity(std::vector<int> midlPList);

	//USERFUNCTIONS
	void hotkeys(sf::Window & window, sf::View & view);

	//GUI
	void initSetup();
	void setInfo();

	//OTHER
	int uniform_random(int min, int max);
	double uniform_random(double min, double max);
	sf::Vector2f random_vector(double magn);
	static double convertStringToDouble(std::string string);
	std::string calcTemperature(double q, int e);

	friend class NewObjectInOrbitFunction;
	friend class RemoveObjectFunction;
	friend class AddRingsFunction;
	friend class TrackObjectFunction;
};
