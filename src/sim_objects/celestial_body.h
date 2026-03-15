#pragma once
#include "../CONSTANTS.h"
#include "SimObject.h"
#include <SFML/Graphics.hpp>
#include <random>
#include <vector>
#include <string_view>
#include "../Life.h"

class Space;

class CelestialBody : public SimObject {
private:
	std::string name = generate_name();

	//PHYSICAL
	BodyType planetType;

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

	//STELLAR EVOLUTION
	double fuel = 0.0;                         // hydrogen fuel reserve
	double age = 0.0;                          // accumulated simulation time
	StellarSubType subType = SUBTYPE_NONE;     // pulsar/magnetar variant
	double subTypeTimer = 0.0;                 // countdown for subtype phase
	bool isEvolved = false;                    // true = type set by evolution, not mass ladder

	//FOR DISINTEGRATION AND IGNORING
	double disintegrate_grace_end_time = 0;
	std::vector<int> ignore_ids;

	//GRAPHICS
	mutable sf::CircleShape circle;
	int randBrightness;
	sf::VertexArray light;

public:
	friend class Space;

	struct GoldilockInfo {
		double min_rad;
		double max_rad;
	};

	// Constructors
	explicit CelestialBody(double m = 6.0, double xx = 0.0, double yy = 0.0, double xvv = 0.0, double yvv = 0.0);

	// Getters
	[[nodiscard]] BodyType getType() const noexcept { return planetType; }
	[[nodiscard]] static std::string getTypeString(BodyType type) noexcept;
	[[nodiscard]] std::string getDisplayName() const noexcept;
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
	[[nodiscard]] double getFuel() const noexcept { return fuel; }
	[[nodiscard]] double maxFuel() const noexcept;
	[[nodiscard]] double fuelFraction() const noexcept;
	[[nodiscard]] double getAge() const noexcept { return age; }
	[[nodiscard]] StellarSubType getSubType() const noexcept { return subType; }
	[[nodiscard]] bool getIsEvolved() const noexcept { return isEvolved; }

	// Setters
	void setName(const std::string& n) noexcept { name = n; }
	void setStrongestAttractorIdRef(int id) noexcept { strongestAttractorId = id; }
	void markForRemoval() noexcept { marked_for_removal = true; }
	void setStrongestAttractorStrength(double strength) noexcept { strongestAttractorStrength = strength; }
	void setMass(double m) noexcept override { SimObject::setMass(m); }
	void setAtmosphere(double a) noexcept { atmoCur = a; }
	void setAtmospherePotensial(double a) noexcept { atmoPot = a; }
	void setLifeLevel(lType level) noexcept { life.setLifeLevel(level); }
	void setFuel(double f) noexcept { fuel = f; }
	void setIsEvolved(bool evolved) noexcept { isEvolved = evolved; }
	void setSubType(StellarSubType st) noexcept { subType = st; }
	void setType(BodyType t) noexcept { planetType = t; }

	// Type classification helpers
	[[nodiscard]] bool isMainSequenceStar() const noexcept { return planetType == STAR; }
	[[nodiscard]] bool isAnyStarType() const noexcept;
	[[nodiscard]] bool isCompactRemnant() const noexcept;
	[[nodiscard]] bool canTidallyDisrupt() const noexcept { return planetType == BLACKHOLE || planetType == NEUTRONSTAR; }
	[[nodiscard]] bool isFuelDepleted() const noexcept { return planetType == STAR && fuel <= 0.0; }
	[[nodiscard]] bool hasFuel() const noexcept { return (planetType == STAR || planetType == BROWNDWARF) && fuel > 0.0; }

	// State checks
	[[nodiscard]] bool canDisintegrate(double curr_time) const noexcept;
	[[nodiscard]] bool disintegrationGraceTimeIsActive(double curr_time) const noexcept;
	[[nodiscard]] bool disintegrationGraceTimeOver(double curr_time) const noexcept;
	[[nodiscard]] bool isIgnoring(int id) const noexcept;

	// State modifications
	void setDisintegrationGraceTime(double grace_time, double curr_time) noexcept;
	void registerIgnoredId(int id);
	void clearIgnores() noexcept { ignore_ids.clear(); }
	void becomeAbsorbedBy(CelestialBody& absorbing_planet);
	void updateRadiAndType() noexcept;
	void initializeRemnantTemperature() noexcept;
	void resetAttractorMeasure() noexcept { strongestAttractorStrength = 0; }
	void incMass(double m) noexcept;
	void collision(const CelestialBody& p);
	void giveID(int i) noexcept;
	void coolDown(int t) noexcept;
	void absorbHeat(double e, int t) noexcept;
	void updateAtmosphere(int t) noexcept;
	void colonize(int i, const sf::Color& c, std::string_view d, std::string_view cn);

	// Simulation and rendering
	void update(double timestep) override;
	void update_planet_sim(double timestep, bool heat_enabled = true, double fuelBurnRate = 1.0);
	void updateLife(int t);
	void render(sf::RenderTarget& window) const override;
	[[nodiscard]] double getDist(const CelestialBody& forcer) const noexcept;
	void draw_thermal_shine(sf::RenderTarget& window) const;
	void draw_gas_planet_atmosphere(sf::RenderTarget& window) const;
	void draw_pulsar_beams(sf::RenderTarget& window) const;
	void draw_magnetar_glow(sf::RenderTarget& window) const;
	void setColor() noexcept;

private:
	void updateMainSequenceType() noexcept;
	void updateEvolvedType() noexcept;
	void updateVisualProperties() noexcept;
	void updateRadius() noexcept;
	void initializeFuel() noexcept;
	[[nodiscard]] sf::Color getShineColor() const noexcept;

	[[nodiscard]] int modernRandomWithLimits(int min, int max) const;
	[[nodiscard]] std::string generate_name();
};

// Type alias for backward compatibility during transition
using Planet = CelestialBody;
