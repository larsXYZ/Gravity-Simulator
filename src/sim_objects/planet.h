#pragma once
#include "../CONSTANTS.h"
#include "SimObject.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <string_view>
#include "../Life.h"

class Planet : public SimObject {
private:
	std::string name = generate_name();

	//PHYSICAL
	pType planetType;

		//ATMOSPHERE
	double atmoCur = 0;
	double atmoPot = modernRandomWithLimits(0, (int)maxAtmo);
	int numAtmoLines;
	std::vector<int> atmoLinesBrightness;
	sf::Color atmoColor = sf::Color(modernRandomWithLimits(0, 170), modernRandomWithLimits(0, 170), modernRandomWithLimits(0, 170));

	//LIFE
	Life life;
	double supportedBiomass = 0;

	//OTHER
	int id;
	int strongestAttractorId = -1;
	double strongestAttractorStrength = 0;
	bool marked_for_removal = false;

	//FOR DISINTEGRATION AND IGNORING
	double disintegrate_grace_end_time = 0;
	std::vector<int> ignore_ids;

	//GRAPHICS
	mutable sf::CircleShape circle;
	int randBrightness;
	sf::VertexArray light;

public:
	struct GoldilockInfo {
		double min_rad;
		double max_rad;
	};

	// Constructors
	explicit Planet(double m = 6.0, double xx = 0.0, double yy = 0.0, double xvv = 0.0, double yvv = 0.0);

	// Getters
	[[nodiscard]] pType getType() const noexcept { return planetType; }
	[[nodiscard]] static std::string getTypeString(pType type) noexcept;
	[[nodiscard]] int getId() const noexcept override { return id; }
	[[nodiscard]] int getStrongestAttractorId() const noexcept { return strongestAttractorId; }
	[[nodiscard]] int getStrongestAttractorIdRef() const noexcept { return strongestAttractorId; }
	[[nodiscard]] const std::string& getName() const noexcept;
	[[nodiscard]] bool emitsHeat() const noexcept;
	[[nodiscard]] std::string getFlavorTextLife() const;
	[[nodiscard]] sf::Color getStarCol() const noexcept;
	[[nodiscard]] bool isMarkedForRemoval() const noexcept { return marked_for_removal; }
	[[nodiscard]] double getStrongestAttractorStrength() const noexcept { return strongestAttractorStrength; }
	[[nodiscard]] double fusionEnergy() const noexcept;
	[[nodiscard]] double thermalEnergy() const noexcept;
	[[nodiscard]] double giveThermalEnergy(int t) const noexcept;
	[[nodiscard]] double getCurrentAtmosphere() const noexcept { return atmoCur; }
	[[nodiscard]] double getAtmospherePotensial() const noexcept { return atmoPot; }
	[[nodiscard]] const Life& getLife() const noexcept { return life; }
	[[nodiscard]] double getSupportedBiomass() const noexcept { return supportedBiomass; }
	[[nodiscard]] GoldilockInfo getGoldilockInfo() const noexcept;

	// Setters
	void setName(const std::string& n) noexcept { name = n; }
	void setStrongestAttractorIdRef(int id) noexcept { strongestAttractorId = id; }
	void markForRemoval() noexcept { marked_for_removal = true; }
	void setStrongestAttractorStrength(double strength) noexcept { strongestAttractorStrength = strength; }
	void setMass(double m) noexcept override { SimObject::setMass(m); }
	void setAtmosphere(double a) noexcept { atmoCur = a; }
	void setAtmospherePotensial(double a) noexcept { atmoPot = a; }

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
	void resetAttractorMeasure() noexcept { strongestAttractorStrength = 0; }
	void incMass(double m) noexcept;
	void collision(const Planet& p);
	void giveID(int i) noexcept;
	void coolDown(int t) noexcept;
	void absorbHeat(double e, int t) noexcept;
	void updateAtmosphere(int t) noexcept;
	void colonize(int i, const sf::Color& c, std::string_view d, std::string_view cn);

	// Simulation and rendering
	void update(double timestep) override;
	void update_planet_sim(double timestep);
	void updateLife(int t);
	void render(sf::RenderWindow& window) const override;
	[[nodiscard]] double getDist(const Planet& forcer) const noexcept;
	void draw_starshine(sf::RenderWindow& window) const;
	void draw_planetshine(sf::RenderWindow& window) const;
	void draw_gas_planet_atmosphere(sf::RenderWindow& window) const;
	void setColor() noexcept;

private:
	[[nodiscard]] int modernRandomWithLimits(int min, int max) const;
	[[nodiscard]] std::string generate_name();
};
