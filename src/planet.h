#pragma once
#include "CONSTANTS.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <string_view>
#include "Life.h"

class Planet {
private:
	std::string name = generate_name();

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
	int id;
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
	struct GoldilockInfo {
		double min_rad;
		double max_rad;
	};

	// Constructors
	Planet();
	Planet(double m);
	Planet(double m, double xx, double yy);
	Planet(double m, double xx, double yy, double xvv, double yvv);

	// Getters
	[[nodiscard]] double getx() const noexcept { return x; }
	[[nodiscard]] double gety() const noexcept { return y; }
	[[nodiscard]] double getxv() const noexcept { return xv; }
	[[nodiscard]] double getyv() const noexcept { return yv; }
	[[nodiscard]] double getmass() const noexcept { return mass; }
	[[nodiscard]] double getRad() const noexcept { return radi; }
	[[nodiscard]] pType getType() const noexcept { return planetType; }
	[[nodiscard]] static std::string getTypeString(pType type) noexcept;
	[[nodiscard]] int getId() const noexcept { return id; }
	[[nodiscard]] int getStrongestAttractorId() const noexcept;
	[[nodiscard]] int getStrongestAttractorIdRef() const noexcept;
	[[nodiscard]] const std::string& getName() const noexcept;
	[[nodiscard]] bool emitsHeat() const noexcept;
	[[nodiscard]] std::string getFlavorTextLife() const;
	[[nodiscard]] sf::Color getStarCol() const noexcept;
	[[nodiscard]] bool isMarkedForRemoval() const noexcept { return marked_for_removal; }
	[[nodiscard]] double getStrongestAttractorStrength() const noexcept { return STRENGTH_strongest_attractor; }
	[[nodiscard]] double temp() const noexcept { return temperature; }
	[[nodiscard]] double getTemp() const noexcept { return temperature; }
	[[nodiscard]] double fusionEnergy() const noexcept;
	[[nodiscard]] double thermalEnergy() const noexcept;
	[[nodiscard]] double giveThermalEnergy(int t) const noexcept;
	[[nodiscard]] double getTCap() const noexcept { return tCapacity; }
	[[nodiscard]] double getCurrentAtmosphere() const noexcept { return atmoCur; }
	[[nodiscard]] double getAtmospherePotensial() const noexcept { return atmoPot; }
	[[nodiscard]] const Life& getLife() const noexcept { return life; }
	[[nodiscard]] double getSupportedBiomass() const noexcept { return supportedBiomass; }
	[[nodiscard]] GoldilockInfo getGoldilockInfo() const noexcept;

	// Setters
	void setStrongestAttractorIdRef(int id) noexcept;
	void markForRemoval() noexcept { marked_for_removal = true; }
	void setStrongestAttractorStrength(double strength) noexcept { STRENGTH_strongest_attractor = strength; }
	void setx(double new_x) noexcept { x = new_x; }
	void sety(double new_y) noexcept { y = new_y; }
	void setTemp(double t) noexcept;
	void setMass(double m) noexcept;
	void setxv(double v) noexcept { xv = v; }
	void setyv(double v) noexcept { yv = v; }

	// State checks
	[[nodiscard]] bool canDisintegrate(double curr_time) const noexcept;
	[[nodiscard]] bool disintegrationGraceTimeIsActive(double curr_time) const noexcept;
	[[nodiscard]] bool disintegrationGraceTimeOver(double curr_time) const noexcept;
	[[nodiscard]] bool isIgnoring(int id) const noexcept;

	// State modifications
	void setDisintegrationGraceTime(double grace_time, double curr_time) noexcept;
	void registerIgnoredId(int id);
	void clearIgnores() noexcept { ignore_ids.clear(); }
	void becomeAbsorbedBy(Planet& absorbing_planet);
	void updateRadiAndType() noexcept;
	void resetAttractorMeasure() noexcept { STRENGTH_strongest_attractor = 0; }
	void incMass(double m) noexcept;
	void collision(const Planet& p);
	void giveID(int i) noexcept;
	void updateTemp() noexcept;
	void coolDown(int t) noexcept;
	void absorbHeat(double e, int t) noexcept;
	void increaseThermalEnergy(double e) noexcept { tEnergy += e; }
	void updateAtmosphere(int t) noexcept;
	void colonize(int i, const sf::Color& c, std::string_view d);

	// Simulation and rendering
	void update_planet_sim(double timestep) noexcept;
	void updateLife(int t);
	void draw(sf::RenderWindow& window);
	[[nodiscard]] double getDist(const Planet& forcer) const noexcept;
	void render_shine(sf::RenderWindow& window, const sf::Color& col, double luminosity) const;
	void draw_starshine(sf::RenderWindow& window) const;
	void draw_planetshine(sf::RenderWindow& window) const;
	void draw_gas_planet_atmosphere(sf::RenderWindow& window) const;
	void setColor() noexcept;

private:
	[[nodiscard]] int modernRandomWithLimits(int min, int max) const;
	[[nodiscard]] std::string generate_name();
};
