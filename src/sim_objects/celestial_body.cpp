#include "celestial_body.h"
#include "../HeatSim.h"
#include "../roche_limit.h"

#include <sstream>

namespace {
	// Linear interpolation helper
	double interpolate(double low_val, double high_val, double mass, double low_mass, double high_mass)
	{
		double t = std::clamp((mass - low_mass) / (high_mass - low_mass), 0.0, 1.0);
		return low_val + t * (high_val - low_val);
	}

	// Spectral class display name for STAR type based on mass
	std::string getStarSpectralName(double mass)
	{
		const double range = STARLIMIT - GASGIANTLIMIT;
		const double t = (mass - GASGIANTLIMIT) / range; // 0.0 to 1.0
		if (t < 1.0 / 6.0) return "Red dwarf";
		if (t < 2.0 / 6.0) return "Orange dwarf";
		if (t < 3.0 / 6.0) return "Yellow dwarf";
		if (t < 4.0 / 6.0) return "White star";
		if (t < 5.0 / 6.0) return "Blue giant";
		return "Blue supergiant";
	}

	// Continuous fusion energy for STAR type
	double calculateStarFusionEnergy(double mass)
	{
		double mult = interpolate(HEAT_STAR_LOW_MULT, HEAT_STAR_HIGH_MULT, mass, GASGIANTLIMIT, STARLIMIT);
		return mult * mass;
	}
}

CelestialBody::CelestialBody(double m, double xx, double yy, double xvv, double yvv)
	: SimObject({static_cast<float>(xx), static_cast<float>(yy)}, {static_cast<float>(xvv), static_cast<float>(yvv)})
{
	setMass(m);

	randBrightness = modernRandomWithLimits(-30, +30);
	updateRadiAndType();
	circle.setPosition(position);

	initializeFuel();

	//DETERMINING NUMBER OF ATMOSPHERE LINES, FOR GASGIANT PHASE
	numAtmoLines = modernRandomWithLimits(minAtmoLayer, maxAtmoLayer);
	for (int i = 0; i < numAtmoLines; i++) atmoLinesBrightness.push_back(
		modernRandomWithLimits(-brightnessVariance, brightnessVariance));
}

double CelestialBody::getDist(const CelestialBody& forcer) const noexcept
{
	const auto& otherPos = forcer.getPosition();
	return sqrt(
		(otherPos.x - position.x) * (otherPos.x - position.x) + (otherPos.y - position.y) * (otherPos.y - position.y));
}

std::string CelestialBody::getTypeString(BodyType type) noexcept
{
	switch (type)
	{
	case ROCKY: return "Rocky";
	case TERRESTRIAL: return "Terrestrial";
	case GASGIANT: return "Gas giant";
	case BROWNDWARF: return "Brown dwarf";
	case STAR: return "Star";
	case WHITEDWARF: return "White dwarf";
	case NEUTRONSTAR: return "Neutron star";
	case BLACKHOLE: return "Black hole";
	default: return "Unknown";
	}
}

std::string CelestialBody::getDisplayName() const noexcept
{
	switch (planetType)
	{
	case ROCKY:
		if (getMass() < ROCKYLIMIT * 0.4) return "Asteroid";
		return "Dwarf planet";
	case TERRESTRIAL:
		if (getMass() > (ROCKYLIMIT + TERRESTRIALLIMIT) / 2.0) return "Super-Earth";
		return "Earth-like";
	case GASGIANT:
		if (getMass() > TERRESTRIALLIMIT + (BROWNDWARFLIMIT - TERRESTRIALLIMIT) * 0.66) return "Super-Jupiter";
		if (getMass() > TERRESTRIALLIMIT + (BROWNDWARFLIMIT - TERRESTRIALLIMIT) * 0.33) return "Jupiter-like";
		return "Neptune-like";
	case BROWNDWARF:
		return "Brown dwarf";
	case STAR:
		if (fuelFraction() < GIANT_PHASE_BEGIN)
		{
			if (getMass() > (GASGIANTLIMIT + STARLIMIT) / 2.0) return "Red supergiant";
			return "Red giant";
		}
		return getStarSpectralName(getMass());
	case WHITEDWARF:
		return "White dwarf";
	case NEUTRONSTAR:
		if (subType == PULSAR) return "Pulsar";
		if (subType == MAGNETAR) return "Magnetar";
		return "Neutron star";
	case BLACKHOLE:
		if (getMass() > STARLIMIT * 2.5) return "Supermassive black hole";
		return "Black hole";
	default:
		return "Unknown";
	}
}

const std::string& CelestialBody::getName() const noexcept
{
	return name;
}

void CelestialBody::giveID(int i) noexcept
{
	id = i;
	life.giveId(i);
}

bool CelestialBody::emitsHeat() const noexcept
{
	switch (getType())
	{
	case BROWNDWARF:
	case STAR:
	case WHITEDWARF:
	case NEUTRONSTAR:
	case BLACKHOLE:
		return true;
	default:
		return false;
	}
}

bool CelestialBody::isAnyStarType() const noexcept
{
	switch (planetType)
	{
	case STAR:
	case BROWNDWARF:
		return true;
	default:
		return false;
	}
}

bool CelestialBody::isCompactRemnant() const noexcept
{
	switch (planetType)
	{
	case WHITEDWARF:
	case NEUTRONSTAR:
	case BLACKHOLE:
		return true;
	default:
		return false;
	}
}

std::string CelestialBody::getFlavorTextLife() const
{
	std::string baseText = "";
	if (life.getTypeEnum() >= 4 && !life.getCivName().empty())
	{
		baseText = "The " + life.getCivName() + ".\n";
		if (!life.getDesc().empty())
			baseText += life.getDesc() + ". ";
	}

	switch (static_cast<int>(life.getTypeEnum()))
	{
	case 0:
		return "Lifeless planet. Either the conditions for life are not met or life has yet to evolve.";
	case 1:
		return "Organisms that consist of one cell. The first form of life. Often lives in fluids in, on, or under the surface.";
	case 2:
		return "Aggregate from either cell division or individuals cells coming together. The next step in the evolution of life.";
	case 3:
		return "Enormous numbers of cells work together to support a sizable lifeform. These can often be found roaming the surface of the planet.";
	case 4:
		return baseText + "The organisms have developed intelligence and are banding together in groups. Often using simple technology.";
	case 5:
		return baseText + "The organisms are now the dominant species on the planet. They have created advanced technology and culture.";
	case 6:
		return baseText + "The organisms technology has enabled them to spread to other planets. Only a truly devestating event can end their civilization now.";
	case 7:
		return baseText + "An outpost made by the organisms. With time it will grow into a fully capable part of the civilization.";
	default:
		return "Do not look into the void.";
	}
}

sf::Color CelestialBody::getStarCol() const noexcept
{
	const static StarColorInterpolator interpolator;
	return interpolator.getStarColor(getTemp());	
}

double CelestialBody::maxFuel() const noexcept
{
	if (planetType == STAR)
		return getMass() * INITIAL_FUEL_PER_MASS;
	if (planetType == BROWNDWARF)
		return getMass() * INITIAL_FUEL_PER_MASS * BROWNDWARF_FUEL_FRACTION;
	return 0.0;
}

double CelestialBody::fuelFraction() const noexcept
{
	const double mf = maxFuel();
	return (mf > 0.0) ? fuel / mf : 1.0;
}

void CelestialBody::initializeFuel() noexcept
{
	if (fuel <= 0.0)
		fuel = maxFuel();
}

double CelestialBody::fusionEnergy() const noexcept
{
	switch (planetType)
	{
	case STAR:
		{
		double base = calculateStarFusionEnergy(getMass());
		double ff = fuelFraction();
		if (ff < GIANT_PHASE_BEGIN)
		{
			double t = std::clamp((GIANT_PHASE_BEGIN - ff) / (GIANT_PHASE_BEGIN - GIANT_PHASE_END), 0.0, 1.0);
			double boost = 1.0 + t * t * GIANT_PHASE_FUSION_BOOST;
			return base * boost;
		}
		return base;
		}
	case BROWNDWARF:
		return HEAT_BROWNDWARF_MULT * getMass();
	default:
		return 0;
	}
}

double CelestialBody::thermalEnergy() const noexcept
{
	return tEnergy;
}

void CelestialBody::coolDown(int t) noexcept
{
	// Thermal radiation loss (Stefan-Boltzmann law)
	tEnergy -= calculate_cooling(getTemp(), radius, t);
	
	// Add energy from fusion
	tEnergy += t * fusionEnergy();
	clampTemperature();
}

void CelestialBody::absorbHeat(double e, int t) noexcept
{
	tEnergy += (e * (1 + greenHouseEffectMult * atmoCur));
	clampTemperature();
}

double CelestialBody::giveThermalEnergy(int t) const noexcept
{
	return t * (SBconst * (radius * radius * getTemp()));
}

void CelestialBody::update(double timestep)
{
	update_planet_sim(timestep, true);
}

void CelestialBody::update_planet_sim(double timestep, bool heat_enabled, double fuelBurnRate)
{
	age += timestep;

	// Burn fusion fuel
	if (hasFuel())
	{
		fuel -= timestep * fusionEnergy() * BASE_FUEL_BURN_RATE * fuelBurnRate;
		if (fuel < 0.0)
			fuel = 0.0;
		updateVisualProperties();
		updateRadius();
	}

	// Brown dwarf fuel depleted — cools into a gas giant
	if (planetType == BROWNDWARF && fuel <= 0.0)
	{
		planetType = GASGIANT;
		isEvolved = true;
		updateVisualProperties();
		updateRadius();
	}

	if (heat_enabled)
		coolDown(timestep);
	setColor();
	updateAtmosphere(timestep);
	updateLife(timestep);
}

bool CelestialBody::canDisintegrate(double curr_time) const noexcept
{
	if (canTidallyDisrupt())
		return false;

	if (!RocheLimit::hasMinimumBreakupSize(getMass()))
		return false;

	if (!ignore_ids.empty())
		return false;

	return true;
}

void CelestialBody::setDisintegrationGraceTime(double grace_time, double curr_time) noexcept
{
	disintegrate_grace_end_time = curr_time + grace_time;
}

bool CelestialBody::disintegrationGraceTimeIsActive(double curr_time) const noexcept
{
	return curr_time < disintegrate_grace_end_time;
}

bool CelestialBody::disintegrationGraceTimeOver(double curr_time) const noexcept
{
	return curr_time >= disintegrate_grace_end_time;
}

void CelestialBody::registerIgnoredId(int ignored_id)
{
	if (std::find(ignore_ids.begin(), ignore_ids.end(), ignored_id) == ignore_ids.end()) {
		ignore_ids.push_back(ignored_id);
	}
}

bool CelestialBody::isIgnoring(int check_id) const noexcept
{
	return std::find(ignore_ids.begin(), ignore_ids.end(), check_id) != ignore_ids.end();
}

void CelestialBody::becomeAbsorbedBy(CelestialBody& absorbing_planet)
{
	markForRemoval();
	absorbing_planet.collision(*this);
	absorbing_planet.incMass(getMass());
}

void CelestialBody::updateRadiAndType() noexcept
{
	if (isEvolved)
		updateEvolvedType();
	else
		updateMainSequenceType();
	updateVisualProperties();
	updateRadius();
}

void CelestialBody::initializeRemnantTemperature() noexcept
{
	if (planetType == WHITEDWARF && getTemp() < 1.0)
		setTemp(INITIAL_TEMP_WHITEDWARF);
	else if (planetType == NEUTRONSTAR && getTemp() < 1.0)
		setTemp(INITIAL_TEMP_NEUTRONSTAR);
}

void CelestialBody::updateMainSequenceType() noexcept
{
	if (getMass() < ROCKYLIMIT)
		planetType = ROCKY;
	else if (getMass() < TERRESTRIALLIMIT)
		planetType = TERRESTRIAL;
	else if (getMass() < BROWNDWARFLIMIT)
		planetType = GASGIANT;
	else if (getMass() < GASGIANTLIMIT)
		planetType = BROWNDWARF;
	else
		planetType = STAR;
}

void CelestialBody::updateEvolvedType() noexcept
{
	switch (planetType)
	{
	case WHITEDWARF:
		// Chandrasekhar limit handled in Space::update() where explosion can be spawned
		break;
	case NEUTRONSTAR:
		if (getMass() > TOV_LIMIT)
		{
			planetType = BLACKHOLE;
			isEvolved = false;
		}
		break;
	case BROWNDWARF:
	case GASGIANT:
		if (getMass() >= GASGIANTLIMIT)
		{
			planetType = STAR;
			isEvolved = false;
			initializeFuel();
		}
		break;
	case BLACKHOLE:
		// Black holes are forever
		isEvolved = false;
		break;
	default:
		// If somehow an evolved object has a non-evolved type, fall back to mass ladder
		isEvolved = false;
		updateMainSequenceType();
		break;
	}
}

void CelestialBody::updateVisualProperties() noexcept
{
	switch (planetType)
	{
	case ROCKY:
		density = 0.5;
		circle.setOutlineThickness(0);
		circle.setPointCount(30);
		break;
	case TERRESTRIAL:
		density = 0.5;
		circle.setPointCount(40);
		break;
	case GASGIANT:
		density = 0.3;
		circle.setPointCount(50);
		break;
	case BROWNDWARF:
		density = DENSITY_BROWNDWARF;
		circle.setOutlineColor(sf::Color(180, 80, 50, 60));
		circle.setOutlineThickness(2);
		circle.setPointCount(60);
		break;
	case STAR:
		{
		double baseDensity = interpolate(0.2, 0.1, getMass(), GASGIANTLIMIT, STARLIMIT);
		double ff = fuelFraction();
		if (ff < GIANT_PHASE_BEGIN)
		{
			// Star expands as fuel depletes — quadratic curve for gradual start
			double t = std::clamp((GIANT_PHASE_BEGIN - ff) / (GIANT_PHASE_BEGIN - GIANT_PHASE_END), 0.0, 1.0);
			double expansion = t * t; // slow start, accelerates toward end
			double giantDensity = interpolate(DENSITY_STAR_GIANT, DENSITY_STAR_SUPERGIANT, getMass(), GASGIANTLIMIT, STARLIMIT);
			baseDensity = baseDensity + expansion * (giantDensity - baseDensity);
		}
		density = baseDensity;
		circle.setPointCount(static_cast<int>(interpolate(90, 150, getMass(), GASGIANTLIMIT, STARLIMIT)));
		circle.setOutlineThickness(static_cast<float>(interpolate(3, 10, getMass(), GASGIANTLIMIT, STARLIMIT)));
		break;
		}
	case WHITEDWARF:
		density = DENSITY_WHITEDWARF;
		circle.setOutlineColor(sf::Color(220, 220, 255, 40));
		circle.setOutlineThickness(1);
		circle.setPointCount(30);
		break;
	case NEUTRONSTAR:
		density = DENSITY_NEUTRONSTAR;
		circle.setOutlineColor(sf::Color(200, 200, 220, 40));
		circle.setOutlineThickness(1);
		circle.setPointCount(20);
		break;
	case BLACKHOLE:
		density = INFINITY;
		circle.setOutlineColor(sf::Color(255, 255, 255, 255));
		circle.setFillColor(sf::Color(20, 20, 20));
		circle.setOutlineThickness(2);
		circle.setPointCount(20);
		break;
	}
}

void CelestialBody::updateRadius() noexcept
{
	if (planetType == BLACKHOLE)
	{
		// Schwarzschild radius
		radius = 2 * getMass() * G / (SPEED_OF_LIGHT * SPEED_OF_LIGHT);
		radius *= 3e16;
	}
	else
	{
		radius = cbrt(getMass()) / density;
	}
	circle.setRadius(radius);
	circle.setOrigin(radius, radius);
}

void CelestialBody::incMass(double m) noexcept
{
	const auto prevType = planetType;
	setMass(getMass() + m);
	updateRadiAndType();
	if (prevType != planetType)
		initializeFuel();
}

void CelestialBody::collision(const CelestialBody& p)
{
	// Conservation of momentum
	velocity.x = (getMass() * velocity.x + p.getMass() * p.getVelocity().x) / (getMass() + p.getMass());
	velocity.y = (getMass() * velocity.y + p.getMass() * p.getVelocity().y) / (getMass() + p.getMass());

	// Calculate kinetic energy converted to heat
	const auto dXV = velocity.x - p.getVelocity().x;
	const auto dYV = velocity.y - p.getVelocity().y;
	increaseThermalEnergy(COLLISION_HEAT_MULTIPLIER * ((dXV * dXV + dYV * dYV) * p.getMass()));

	// Transfer thermal energy from the colliding planet
	// We use mass ratio to determine how much thermal energy is transferred
	const double mass_ratio = p.getMass() / (getMass() + p.getMass());
	increaseThermalEnergy(p.thermalEnergy() * mass_ratio);

	// Conservation of fuel
	fuel += p.fuel;
}

void CelestialBody::draw_starshine(sf::RenderTarget& window) const
{
	sf::Color col = getStarCol();

	//LONG RANGE LIGHT
	col.a = 50;
	const auto long_range_luminosity = 30 * sqrt(fusionEnergy());
	render_shine(window, position, col, long_range_luminosity);

	//SHORT RANGE LIGHT
	col.a = 250;
	const auto short_range_luminosity = 1.5 * getRadius();
	render_shine(window, position, col, short_range_luminosity);
}

void CelestialBody::draw_planetshine(sf::RenderTarget& window) const
{
	/*
	 *	Caused by very hot temperatures
	 */
	draw_heat_glow(window, position, getTemp(), radius);
}

void CelestialBody::draw_gas_planet_atmosphere(sf::RenderTarget& window) const
{
	for (size_t i = 0; i < atmoLinesBrightness.size(); i++)
	{
		sf::CircleShape atmoLine;

		//SETTING FEATURES
		int temp_lines = (numAtmoLines - 1);
		atmoLine.setRadius(
			circle.getRadius() - i * i * i * circle.getRadius() / (temp_lines * temp_lines * temp_lines));
		atmoLine.setOrigin(atmoLine.getRadius(), atmoLine.getRadius());
		atmoLine.setPosition(circle.getPosition());
		atmoLine.setOutlineThickness(0);

		//FINDING COLOR
		auto temp_effect = temperature_effect(getTemp());
		double r = atmoColor.r + atmoLinesBrightness[i] + temp_effect.r;
		double g = atmoColor.g + atmoLinesBrightness[i] + temp_effect.g;
		double b = atmoColor.b + atmoLinesBrightness[i] + temp_effect.b;

		r = std::clamp(r, 0., 255.);
		g = std::clamp(g, 0., 255.);
		b = std::clamp(b, 0., 255.);

		atmoLine.setFillColor(sf::Color(r, g, b));
		window.draw(atmoLine);
	}
}

void CelestialBody::draw_white_dwarf_glow(sf::RenderTarget& window) const
{
	// Intense compact blue-white core glow
	sf::Color coreCol(220, 220, 255, 120);
	render_shine(window, position, coreCol, radius * 6.0);

	// Tight bright inner glow
	sf::Color innerCol(240, 240, 255, 200);
	render_shine(window, position, innerCol, radius * 2.5);
}

void CelestialBody::draw_neutron_star_glow(sf::RenderTarget& window) const
{
	if (subType == PULSAR)
	{
		// Dimmer ambient glow for pulsars
		sf::Color outerCol(60, 140, 255, 25);
		render_shine(window, position, outerCol, radius * 18.0);

		sf::Color midCol(80, 180, 255, 70);
		render_shine(window, position, midCol, radius * 7.0);

		sf::Color innerCol(100, 220, 255, 140);
		render_shine(window, position, innerCol, radius * 3.0);

		sf::Color coreCol(140, 240, 255, 240);
		render_shine(window, position, coreCol, radius * 1.8);

		draw_pulsar_beams(window);
	}
	else if (subType == MAGNETAR)
	{
		draw_magnetar_glow(window);
	}
	else
	{
		// Normal neutron star — original glow
		sf::Color outerCol(80, 120, 255, 35);
		render_shine(window, position, outerCol, radius * 22.0);

		sf::Color midCol(100, 160, 255, 90);
		render_shine(window, position, midCol, radius * 9.0);

		sf::Color innerCol(140, 200, 255, 160);
		render_shine(window, position, innerCol, radius * 4.0);

		sf::Color coreCol(180, 230, 255, 250);
		render_shine(window, position, coreCol, radius * 2.0);
	}
}

void CelestialBody::draw_pulsar_beams(sf::RenderTarget& window) const
{
	double angle = std::fmod(age * PULSAR_ROTATION_SPEED, 2.0 * PI);
	double beamLen = radius * PULSAR_BEAM_LENGTH_MULT;

	for (int beam = 0; beam < 2; beam++)
	{
		double beamAngle = angle + beam * PI;

		sf::VertexArray tri(sf::Triangles, 3);

		// Center vertex (bright)
		tri[0].position = position;
		tri[0].color = sf::Color(100, 240, 255, 100);

		// Two outer vertices (transparent)
		double leftAngle = beamAngle - PULSAR_BEAM_WIDTH;
		double rightAngle = beamAngle + PULSAR_BEAM_WIDTH;

		tri[1].position = sf::Vector2f(
			position.x + std::cos(leftAngle) * beamLen,
			position.y + std::sin(leftAngle) * beamLen);
		tri[1].color = sf::Color(100, 240, 255, 0);

		tri[2].position = sf::Vector2f(
			position.x + std::cos(rightAngle) * beamLen,
			position.y + std::sin(rightAngle) * beamLen);
		tri[2].color = sf::Color(100, 240, 255, 0);

		window.draw(tri);
	}
}

void CelestialBody::draw_magnetar_glow(sf::RenderTarget& window) const
{
	double pulse = 0.7 + 0.3 * std::sin(age * MAGNETAR_PULSE_SPEED);
	auto pulseAlpha = [pulse](int base) -> sf::Uint8 {
		return static_cast<sf::Uint8>(std::min(255.0, base * pulse));
	};

	// Wide purple halo — pulsating
	sf::Color outerCol(120, 60, 255, pulseAlpha(45));
	render_shine(window, position, outerCol, radius * 22.0 * MAGNETAR_GLOW_SIZE_MULT);

	// Mid-range magenta glow
	sf::Color midCol(160, 80, 255, pulseAlpha(100));
	render_shine(window, position, midCol, radius * 10.0 * MAGNETAR_GLOW_SIZE_MULT);

	// Intense inner glow
	sf::Color innerCol(200, 140, 255, pulseAlpha(180));
	render_shine(window, position, innerCol, radius * 4.5);

	// Searing bright core
	sf::Color coreCol(220, 180, 255, pulseAlpha(250));
	render_shine(window, position, coreCol, radius * 2.0);
}

void CelestialBody::render(sf::RenderTarget& window) const
{
	circle.setPosition(position);

	switch (getType())
	{
	case ROCKY:
	case TERRESTRIAL:
		draw_planetshine(window);
		window.draw(circle);
		break;

	case GASGIANT:
		draw_planetshine(window);
		draw_gas_planet_atmosphere(window);
		break;

	case BROWNDWARF:
	case STAR:
		window.draw(circle);
		draw_starshine(window);
		break;

	case WHITEDWARF:
		draw_white_dwarf_glow(window);
		window.draw(circle);
		break;

	case NEUTRONSTAR:
		draw_neutron_star_glow(window);
		window.draw(circle);
		break;

	case BLACKHOLE:
		window.draw(circle);
		break;
	}
}

void CelestialBody::setColor() noexcept
{
	switch (getType())
	{
	case ROCKY:
	case TERRESTRIAL:
		{
		auto temp_effect = temperature_effect(getTemp());
		const double r = 100.0 + randBrightness + temp_effect.r;
		const double g = 100.0 + randBrightness + temp_effect.g + getLife().getBmass() / 20.0;
		const double b = 100.0 + randBrightness + temp_effect.b;
		circle.setFillColor(sf::Color(std::clamp(static_cast<int>(r), 0, 255),
			std::clamp(static_cast<int>(g), 0, 255),
			std::clamp(static_cast<int>(b), 0, 255)));
		break;
		}
	case GASGIANT:
		circle.setOutlineThickness(0);
		circle.setFillColor(sf::Color::Transparent);
		break;
	case BROWNDWARF:
		circle.setFillColor(sf::Color(180, 80, 50));
		circle.setOutlineColor(sf::Color(180, 80, 50, 30));
		break;
	case STAR:
		circle.setFillColor(getStarCol());
		circle.setOutlineColor(sf::Color(circle.getFillColor().r, circle.getFillColor().g, circle.getFillColor().b,
			20));
		break;
	case WHITEDWARF:
		circle.setFillColor(sf::Color(220, 220, 255));
		circle.setOutlineColor(sf::Color(220, 220, 255, 40));
		break;
	case NEUTRONSTAR:
		if (subType == PULSAR) {
			circle.setFillColor(sf::Color(140, 230, 255));
			circle.setOutlineColor(sf::Color(100, 210, 255, 50));
		} else if (subType == MAGNETAR) {
			circle.setFillColor(sf::Color(200, 160, 255));
			circle.setOutlineColor(sf::Color(180, 120, 255, 50));
		} else {
			circle.setFillColor(sf::Color(160, 200, 255));
			circle.setOutlineColor(sf::Color(140, 180, 255, 50));
		}
		break;
	case BLACKHOLE:
		break;
	}
}

void CelestialBody::updateAtmosphere(int t) noexcept
{
	if (planetType != TERRESTRIAL)
	{
		if (planetType == ROCKY)
		{
			atmoCur = 0;
			return;
		}
		return;
	}

	if (getTemp() < 600 && getTemp() > 200 && atmoCur < atmoPot)
	{
		atmoCur += t * 0.05;
		if (atmoCur > atmoPot) atmoCur = atmoPot;
	}
	else
	{
		atmoCur -= t * 0.1;

		if (atmoCur < 0) atmoCur = 0;
	}

	circle.setOutlineColor(sf::Color(atmoColor.r, atmoColor.g, atmoColor.b, atmoCur * atmoAlphaMult));
	circle.setOutlineThickness(sqrt(atmoCur) * atmoThicknessMult);
}

void CelestialBody::updateLife(int t)
{
	if (planetType == ROCKY || planetType == TERRESTRIAL)
	{
		supportedBiomass = 100000 / (1 + (LIFE_PREFERRED_TEMP_MULTIPLIER *
			pow((getTemp() - LIFE_PREFERRED_TEMP), 2) + LIFE_PREFERRED_ATMO_MULTIPLIER * pow(
				(atmoCur - LIFE_PREFERRED_ATMO), 2))) - 5000;
		if (supportedBiomass < 0) supportedBiomass = 0;

		life.update(supportedBiomass, t, radius);
	}
	else
	{
		life.kill();
	}
}

void CelestialBody::colonize(int i, const sf::Color& c, std::string_view d, std::string_view cn)
{
	life = Life(i);
	life.giveCol(c);
	life.giveDesc(std::string(d));
	life.giveCivName(std::string(cn));
}

int CelestialBody::modernRandomWithLimits(int min, int max) const
{
	std::random_device seeder;
	std::default_random_engine generator(seeder());
	std::uniform_int_distribution<int> uniform(min, max);
	return uniform(generator);
}

std::string convertDoubleToString(double number)
{
	return std::to_string(static_cast<int>(number));
}

[[nodiscard]] std::string CelestialBody::generate_name()
{
	std::vector<std::string> name_first_part = {
		"Jup", "Jor", "Ear", "Mar", "Ven", "Cer", "Sat", "Pl", "Nep", "Ur", "Ker", "Mer", "Jov", "Qur", "Deb", "Car",
		"Xet", "Nayt", "Erist", "Hamar", "Bjork", "Deat", "Limus", "Lant", "Hypor", "Hyper", "Tell", "It", "As", "Ka",
		"Po", "Yt", "Pertat"
	};
	std::vector<std::string> name_second_part = {"it", "enden", "orden", "eptux", "atur", "oper", "uqtor", "axax"};
	std::vector<std::string> name_third_part = {"er", "us", "o", "i", "atara", "ankara", "oxos", "upol", "ol", "eq"};


	int selectorOne = modernRandomWithLimits(0, static_cast<int>(name_first_part.size()) - 1);
	int selectorTwo = modernRandomWithLimits(0, static_cast<int>(name_second_part.size()) - 1);
	int selectorThree = modernRandomWithLimits(0, static_cast<int>(name_third_part.size()) - 1);

	std::string number = "";

	if (modernRandomWithLimits(0, 100) < 20) number = " " + convertDoubleToString(modernRandomWithLimits(100, 999));


	std::string name = name_first_part[selectorOne] + name_second_part[selectorTwo] + name_third_part[selectorThree] + number;

	return name;
}

CelestialBody::GoldilockInfo CelestialBody::getGoldilockInfo() const noexcept
{
	const auto goldilock_inner_rad = (tempConstTwo * getRadius() * getRadius() * getTemp()) / inner_goldi_temp;
	const auto goldilock_outer_rad = (tempConstTwo * getRadius() * getRadius() * getTemp()) / outer_goldi_temp;
	return { goldilock_inner_rad, goldilock_outer_rad };
}