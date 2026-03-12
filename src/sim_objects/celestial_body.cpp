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
		if (mass < 800) return "Red dwarf";
		if (mass < 1200) return "Orange dwarf";
		if (mass < 1600) return "Yellow dwarf";
		if (mass < 2200) return "White star";
		if (mass < 3000) return "Blue giant";
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

	// Initialize fuel for stars
	if (planetType == STAR)
		fuel = m * INITIAL_FUEL_PER_MASS;

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
	case TERRESTIAL: return "Terrestial";
	case GASGIANT: return "Gas giant";
	case BROWNDWARF: return "Brown dwarf";
	case STAR: return "Star";
	case REDGIANT: return "Red giant";
	case REDSUPERGIANT: return "Red supergiant";
	case WHITEDWARF: return "White dwarf";
	case NEUTRONSTAR: return "Neutron star";
	case BLACKHOLE: return "Black hole";
	default: return "Unknown";
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
	case STAR:
	case REDGIANT:
	case REDSUPERGIANT:
	case WHITEDWARF:
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

double CelestialBody::fusionEnergy() const noexcept
{
	switch (planetType)
	{
	case STAR:
		return calculateStarFusionEnergy(getMass());
	case BROWNDWARF:
		return HEAT_BROWNDWARF_MULT * getMass();
	case REDGIANT:
		return HEAT_REDGIANT_MULT * getMass();
	case REDSUPERGIANT:
		return HEAT_REDSUPERGIANT_MULT * getMass();
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

void CelestialBody::update_planet_sim(double timestep, bool heat_enabled)
{
	age += timestep;
	if (heat_enabled)
		coolDown(timestep);
	setColor();
	updateAtmosphere(timestep);
	updateLife(timestep);
}

bool CelestialBody::canDisintegrate(double curr_time) const noexcept
{
	if (getType() == BLACKHOLE || getType() == NEUTRONSTAR)
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
	// Mass ladder for non-evolved types
	if (getMass() < ROCKYLIMIT)
	{
		planetType = ROCKY;
		density = 0.5;
		circle.setOutlineThickness(0);
		circle.setPointCount(30);
	}
	else if (getMass() < TERRESTIALLIMIT)
	{
		planetType = TERRESTIAL;
		density = 0.5;
		circle.setPointCount(40);
	}
	else if (getMass() < BROWNDWARFLIMIT)
	{
		planetType = GASGIANT;
		density = 0.3;
		circle.setPointCount(50);
	}
	else if (getMass() < GASGIANTLIMIT)
	{
		planetType = BROWNDWARF;
		density = DENSITY_BROWNDWARF;
		circle.setOutlineColor(sf::Color(180, 80, 50, 60));
		circle.setOutlineThickness(2);
		circle.setPointCount(60);
	}
	else if (getMass() < STARLIMIT)
	{
		planetType = STAR;
		// Continuous density: 0.2 at low mass, 0.1 at high mass
		density = interpolate(0.2, 0.1, getMass(), GASGIANTLIMIT, STARLIMIT);
		// Continuous visual properties based on mass
		int pointCount = static_cast<int>(interpolate(90, 150, getMass(), GASGIANTLIMIT, STARLIMIT));
		circle.setPointCount(pointCount);
		double thickness = interpolate(3, 10, getMass(), GASGIANTLIMIT, STARLIMIT);
		circle.setOutlineThickness(static_cast<float>(thickness));
	}
	else
	{
		planetType = BLACKHOLE;
		density = INFINITY;
		circle.setOutlineColor(sf::Color(255, 255, 255, 255));
		circle.setFillColor(sf::Color(20, 20, 20));
		circle.setOutlineThickness(2);
		circle.setPointCount(20);
	}

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
	const bool wasNotStar = (planetType != STAR);
	setMass(getMass() + m);
	updateRadiAndType();
	// Initialize fuel if mass pushes into star range
	if (wasNotStar && planetType == STAR && fuel <= 0.0)
		fuel = getMass() * INITIAL_FUEL_PER_MASS;
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
}

void CelestialBody::draw_starshine(sf::RenderWindow& window) const
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

void CelestialBody::draw_planetshine(sf::RenderWindow& window) const
{
	/*
	 *	Caused by very hot temperatures
	 */
	draw_heat_glow(window, position, getTemp(), radius);
}

void CelestialBody::draw_gas_planet_atmosphere(sf::RenderWindow& window) const
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

void CelestialBody::render(sf::RenderWindow& window) const
{
	circle.setPosition(position);

	switch (getType())
	{
	case ROCKY:
	case TERRESTIAL:
		draw_planetshine(window);
		window.draw(circle);
		break;

	case GASGIANT:
		draw_planetshine(window);
		draw_gas_planet_atmosphere(window);
		break;

	case BROWNDWARF:
		draw_planetshine(window);
		window.draw(circle);
		break;

	case STAR:
	case REDGIANT:
	case REDSUPERGIANT:
		window.draw(circle);
		draw_starshine(window);
		break;

	case WHITEDWARF:
		window.draw(circle);
		draw_planetshine(window);
		break;

	case NEUTRONSTAR:
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
	case TERRESTIAL:
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
	case REDGIANT:
	case REDSUPERGIANT:
		circle.setFillColor(getStarCol());
		circle.setOutlineColor(sf::Color(circle.getFillColor().r, circle.getFillColor().g, circle.getFillColor().b,
			20));
		break;
	case WHITEDWARF:
		circle.setFillColor(sf::Color(220, 220, 255));
		circle.setOutlineColor(sf::Color(220, 220, 255, 40));
		break;
	case NEUTRONSTAR:
		circle.setFillColor(sf::Color(200, 200, 220));
		circle.setOutlineColor(sf::Color(200, 200, 220, 40));
		break;
	case BLACKHOLE:
		break;
	}
}

void CelestialBody::updateAtmosphere(int t) noexcept
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
	if (planetType == ROCKY || planetType == TERRESTIAL)
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