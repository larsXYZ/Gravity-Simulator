#pragma once

#include <vector>
#include <random>
#include <string>
#include <map>
#include <limits>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "sim_objects/celestial_body.h"
#include "sim_config.h"
#include "Spaceship.h"
#include "Effect.h"
#include "CONSTANTS.h"
#include "Bound.h"
#include "click_and_drag.h"
#include "object_info.h"
#include "object_tracker.h"
#include "particles/particle_container.h"
#include "BloomEffect.h"

enum class TemperatureUnit
{
	KELVIN,
	CELSIUS,
	FAHRENHEIT
};

struct Missile {
	sf::Vector2f pos;
	sf::Vector2f vel;
	double current_speed;
	int life;
	int owner_id;
	sf::Color color;
};

class Space
{
	double total_mass{0.0};
	int next_id{0};
	float timestep{ TIMESTEP_VALUE_START };
	double curr_time{ 0.0 };
	int iteration{0};
	int fps{ 0 };
    bool is_mouse_on_gui{ false };
	
	SpaceShip ship;
	std::vector<Planet> planets;
	std::vector<Planet> pending_planets;
	std::unique_ptr<IParticleContainer> particles;
	std::vector<Explosion> explosions;
	std::vector<StarshineFade> starshine_fades;
	std::vector<Trail> trail;
	std::vector<Missile> missiles;
	Bound bound;
	
	struct HotPlanet
	{
		float x, y;
		double mass;
		double radius;
		int id;
		BodyType type;
		double strongestAttractorMag;
		int strongestAttractorId;
		double accumulatedHeat;
		double thermalEnergyOutput;
		double g_mass;
		double radius_sq;
	};
	std::vector<HotPlanet> hot_planets;
	std::vector<sf::Vector2f> accelerations;

	struct CollisionEvent {
		int planetA_idx;
		int planetB_idx;
	};

	struct RocheEvent {
		int planet_idx;
	};

	tgui::TextArea::Ptr simInfo = std::make_shared<tgui::TextArea>();
	tgui::Label::Ptr toolInfo = std::make_shared<tgui::Label>();
	tgui::TextArea::Ptr newPlanetInfo = std::make_shared<tgui::TextArea>();
	tgui::ListBox::Ptr functions = std::make_shared<tgui::ListBox>();
	tgui::Tabs::Ptr temperatureUnitSelector = std::make_shared<tgui::Tabs>();
	tgui::Label::Ptr timeStepLabel = std::make_shared<tgui::Label>();
	tgui::ComboBox::Ptr objectTypeSelector = std::make_shared<tgui::ComboBox>();
	tgui::Slider::Ptr massSlider = std::make_shared<tgui::Slider>();
	tgui::Slider::Ptr timeStepSlider = std::make_shared<tgui::Slider>();
	tgui::CheckBox::Ptr autoBound = tgui::CheckBox::create("Auto Bound");

	tgui::ChildWindow::Ptr quitDialog = tgui::ChildWindow::create("Quit?");

	tgui::ChildWindow::Ptr optionsMenu = tgui::ChildWindow::create("Options Menu");
	tgui::BitmapButton::Ptr optionsButton = tgui::BitmapButton::create();
	sf::Texture optionsButtonTexture;
	tgui::CheckBox::Ptr gravityCheckBox = tgui::CheckBox::create("Gravity Enabled");
	tgui::CheckBox::Ptr heatCheckBox = tgui::CheckBox::create("Heat Enabled");
	tgui::CheckBox::Ptr renderLifeAlwaysCheckBox = tgui::CheckBox::create("Always Render Life");
	tgui::CheckBox::Ptr bloomCheckBox = tgui::CheckBox::create("Bloom");
	tgui::Slider::Ptr fuelBurnSlider = tgui::Slider::create();
	tgui::Label::Ptr fuelBurnLabel = tgui::Label::create();

	BloomEffect bloom;

	ClickAndDragHandler click_and_drag_handler;
	ObjectTracker object_tracker;
	ObjectInfo object_info;

	void renderMST(sf::RenderTarget& window, const std::vector<size_t>& members);

public:

	explicit Space();
	
	int addPlanet(Planet&& p);
	void flushPlanets();
	void removePlanet(const int id);
	void removeExplosion(int ind);
	void removeStarshineFade(int ind);
	void removeSmoke(int ind);
	void removeTrail(int ind);
	void full_reset(sf::View& view, const sf::RenderWindow & window);

	std::vector<int> disintegratePlanet(Planet planet);	/* No reference due to addition of new planets possibly invalidating references */
	std::vector<int> explodePlanet(Planet planet, CelestialBody* remnant = nullptr);	/* No reference due to addition of new planets possibly invalidating references */
	void checkChandrasekharLimit();

	void randomPlanets(int totmass, int antall, double radius, sf::Vector2f pos);
	void addExplosion(sf::Vector2f p, double s, sf::Vector2f v, int l);
	void addStarshineFade(sf::Vector2f p, sf::Vector2f v, sf::Color col, double lr_lum, double sr_lum, int l);
	void addParticle(sf::Vector2f p, sf::Vector2f v, double s, double lifespan, double initial_temp = 2000.0);
	void addTrail(sf::Vector2f p, int l);
	void giveRings(const Planet & planet, int inner, int outer);
	
	//SIMULATION FUNCTIONS
	void update();
	void runSim(sf::Vector2i window_size, bool fullscreen, int udp_port = 0);
	void drawPlanets(sf::RenderTarget &window);
	void drawLifeVisuals(sf::RenderTarget& window, const Planet& p);
	void drawCivConnections(sf::RenderTarget& window, const Planet& p, bool drawIndicatorsOnColonies = false);
	void drawEffects(sf::RenderTarget & window);
	void drawMissiles(sf::RenderTarget& window);
	void drawDust(sf::RenderTarget &window);
	void giveId(Planet &p);
	Planet findPlanet(int id);
	Planet* findPlanetPtr(int id);
	int findBestPlanetByRef(const Planet& query_planet);
	void update_spaceship();
	double thermalEnergyAtPosition(sf::Vector2f pos);
	sf::Vector3f centerOfMass(const std::vector<int> & object_ids);
	sf::Vector2f centerOfMassVelocity(const std::vector<int> & object_ids);
	sf::Vector2f centerOfMassAll();
	int get_iteration() const;
	bool auto_bound_active() const;
	const std::vector<Planet>& getPlanets() const { return planets; }
	void syncConfigToWidgets();

	SimConfig config;

	void set_ambient_temperature(Planet& planet);
	
	tgui::BitmapButton::Ptr editObjectButton = std::make_shared<tgui::BitmapButton>();
	sf::Texture editButtonTexture;
	bool editPanelOpen = false;

	void hotkeys(sf::Event event, sf::View & view, const sf::RenderWindow & window);
	
	void initSetup();
	void updateInfoBox();
	
	int uniform_random(int min, int max);
	double uniform_random(double min, double max);
	sf::Vector2f random_vector(double magn);
	static double convertStringToDouble(std::string string);
	static std::string temperature_info_string(double temperature_kelvin, TemperatureUnit unit);

	friend class ObjectInfo;
	friend class NewObjectFunction;
	friend class NewObjectInOrbitFunction;
	friend class RemoveObjectFunction;
	friend class AddRingsFunction;
	friend class TrackObjectFunction;
	friend class ShowObjectInfoFunction;
	friend class AdvancedInOrbitFunction;
	friend class ExplodeObjectFunction;
	friend class SpaceShip;
	friend class ObjectTracker;
};
