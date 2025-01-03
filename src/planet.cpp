#include "planet.h"

#include <sstream>

//GET FUNCTIONS

double Planet::getx() const
{
	return x;
}

double Planet::gety() const
{
	return y;
}

double Planet::getxv() const
{
	return xv;
}

double Planet::getyv() const
{
	return yv;
}

double Planet::getmass() const
{
	return mass;
}

double Planet::getDist(const Planet& forcer) const
{
	return sqrt(
		(forcer.getx() - getx()) * (forcer.getx() - getx()) + (forcer.gety() - gety()) * (forcer.gety() - gety()));
}

double Planet::getRad() const
{
	return radi;
}

pType Planet::getType() const
{
	return planetType;
}

std::string Planet::getTypeString(pType type)
{
	switch (type)
	{
	case ROCKY: 
		return "Rocky";
	case TERRESTIAL:
		return "Terrestial";
	case GASGIANT:
		return "Gas giant";
	case SMALLSTAR:
		return "Small star";
	case STAR:
		return "Star";
	case BIGSTAR:
		return "Big star";
	case BLACKHOLE:
		return "Black hole";
	}
	return "Unknown";
}

int Planet::getId() const
{
	return id;
}

int Planet::getStrongestAttractorId() const
{
	return ID_strongest_attractor;
}

int Planet::getStrongestAttractorIdRef() const
{
	return ID_strongest_attractor;
}

void Planet::setStrongestAttractorIdRef(int id)
{
	ID_strongest_attractor = id;
}

std::string Planet::getName() const
{
	return name;
}

void Planet::giveID(int i)
{
	id = i;
	life.giveId(i);
}

bool Planet::emitsHeat() const
{
	switch (getType())
	{
	case SMALLSTAR:
	case STAR:
	case BIGSTAR:
	case BLACKHOLE:
		return true;
	case ROCKY:
	case TERRESTIAL:
	case GASGIANT:
	default:
		return false;
	}
}

std::string Planet::getFlavorTextLife() const
{
	switch (static_cast<int>(getLife().getTypeEnum()))
	{
	case (0):
		{
			return "Lifeless planet. Either the conditions\nfor life are not met or life has yet to evolve.";
		}
	case (1):
		{
			return
				"Organisms that consist of one cell. The first form of life.\nOften lives in fluids in, on or under the surface.";
		}
	case (2):
		{
			return
				"Aggregate from either cell division or individuals cells\ncoming togheter. The next step in the evolution of life.";
		}
	case (3):
		{
			return
				"Enormous numbers of cells work togheter to\nsupport a sizable lifeform. These can often be found\nroaming the surface of the planet.";
		}
	case (4):
		{
			return
				"The organisms have developed intelligence and are banding\ntogheter in groups. Often using simple technology.";
		}
	case (5):
		{
			return
				"The organisms are now the dominant species on the planet.\nThey have created advanced technology and culture.";
		}
	case (6):
		{
			return
				"The organisms technology has enabled them to spread to other planets.\nOnly a truly devestating event can end their civilization now.";
		}
	case (7):
		{
			return
				"An outpost made by the organisms. With time it will\ngrow to a fully capable part of the civilization.";
		}
	default:
		{
			return "Do not look into the void.";
		}
	}
}

class StarColorInterpolator
{
	struct TempColorPair
	{
		double temperature;
		sf::Color color;
	};

	std::vector<TempColorPair> temperatures_and_colors;

public:

	StarColorInterpolator()
	{
		temperatures_and_colors.push_back({ 1600.0, sf::Color(255, 118, 0) });
		temperatures_and_colors.push_back({ 3000.0, sf::Color(255, 162, 60) });
		temperatures_and_colors.push_back({ 3700.0, sf::Color(255, 180, 107) });
		temperatures_and_colors.push_back({ 4500.0, sf::Color(255, 206, 146) });
		temperatures_and_colors.push_back({ 5500.0, sf::Color(255, 219, 186) });
		temperatures_and_colors.push_back({ 6500.0, sf::Color(255, 238, 222) });
		temperatures_and_colors.push_back({ 7200.0, sf::Color(255, 249, 251) });
		temperatures_and_colors.push_back({ 8000.0, sf::Color(240, 241, 255) });
		temperatures_and_colors.push_back({ 9000.0, sf::Color(227, 233, 255) });
		temperatures_and_colors.push_back({ 10000.0, sf::Color(214, 225, 255) });
		temperatures_and_colors.push_back({ 11000.0, sf::Color(207, 218, 255) });
		temperatures_and_colors.push_back({ 12000.0, sf::Color(200, 213, 255) });
		temperatures_and_colors.push_back({ 13000.0, sf::Color(191, 211, 255) });
	}

	sf::Color interpolate(TempColorPair a, TempColorPair b, double temp) const
	{
		const auto dist_to_a = std::abs(a.temperature - temp);
		const auto dist_to_b = std::abs(b.temperature - temp);
		const auto range = std::abs(a.temperature - b.temperature);

		const auto a_ = 1.0 - dist_to_a / range;
		const auto b_ = 1.0 - dist_to_b / range;

		return sf::Color(
			a.color.r * a_ + b.color.r * b_,
			a.color.g * a_ + b.color.g * b_,
			a.color.b * a_ + b.color.b * b_,
			a.color.a * a_ + b.color.a * b_
		);
	}

	sf::Color getStarColor(double temperature) const
	{
		if (temperature <= temperatures_and_colors.front().temperature)
			return temperatures_and_colors.front().color;
		if (temperature >= temperatures_and_colors.back().temperature)
			return temperatures_and_colors.back().color;

		auto lower = temperatures_and_colors.begin();
		auto higher = temperatures_and_colors.end();
		for (auto tempcol = temperatures_and_colors.begin();
			tempcol != temperatures_and_colors.end();
			++tempcol)
		{
			const auto point_temp = tempcol->temperature;
			if (point_temp < temperature)
			{
				lower = tempcol;
				higher = std::next(tempcol);
			}
			else
				break;
		}

		return interpolate(*lower, *higher, temperature);
	}
};

sf::Color Planet::getStarCol() const
{
	const static StarColorInterpolator interpolator;
	return interpolator.getStarColor(temperature);	
}

//SIMULATION FUNCTIONS

void Planet::updateTemp()
{
	temperature = temp();
}

double Planet::temp() const
{
	return tEnergy / (mass * tCapacity);
}

double Planet::getTemp() const
{
	return temperature;
}

void Planet::setTemp(double t)
{
	tEnergy = mass * t * tCapacity;
}

double Planet::fusionEnergy() const
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
	default:
		return 0;
	}
}

double Planet::thermalEnergy() const
{
	return tEnergy;
}

void Planet::coolDown(int t)
{
	tEnergy -= t * (SBconst * (radi * radi * temp()) - fusionEnergy());
}

void Planet::absorbHeat(double e, int t)
{
	tEnergy += (e * (1 + greenHouseEffectMult * atmoCur));
}

double Planet::giveThermalEnergy(int t) const
{
	return t * (SBconst * (radi * radi * temp()));
}

void Planet::increaseThermalEnergy(double e)
{
	tEnergy += e;
}

void Planet::update_planet_sim(double timestep)
{
	coolDown(timestep);
	setColor();
	updateTemp();
	updateAtmosphere(timestep);
	updateLife(timestep);
}

bool Planet::canDisintegrate(double curr_time) const
{
	if (getType() == BLACKHOLE)
		return false;

	if (getmass() < MINIMUMBREAKUPSIZE)
		return false;

	if (!ignore_ids.empty())
		return false;

	return true;
}

void Planet::setDisintegrationGraceTime(double grace_time, double curr_time)
{
	disintegrate_grace_end_time = curr_time + grace_time;
}

bool Planet::disintegrationGraceTimeIsActive(double curr_time) const
{
	return !disintegrationGraceTimeOver(curr_time);
}

bool Planet::disintegrationGraceTimeOver(double curr_time) const
{
	return curr_time > disintegrate_grace_end_time;
}

void Planet::registerIgnoredId(int id)
{
	ignore_ids.push_back(id);
}

void Planet::clearIgnores()
{
	ignore_ids.clear();
}

bool Planet::isIgnoring(int id) const
{
	if (ignore_ids.empty())
		return false;

	return std::find(ignore_ids.begin(), ignore_ids.end(), id) != ignore_ids.end();
}

void Planet::becomeAbsorbedBy(Planet& absorbing_planet)
{
	markForRemoval();
	absorbing_planet.collision(*this);
	absorbing_planet.incMass(getmass());
}

void Planet::setx(double x_)
{
	x = x_;
}

void Planet::sety(double y_)
{
	y = y_;
}

void Planet::updateRadiAndType()
{
	if (mass < ROCKYLIMIT)
	{
		planetType = ROCKY;
		density = 0.5;
		circle.setOutlineThickness(0);
		circle.setPointCount(30);
	}
	else if (mass < TERRESTIALLIMIT)
	{
		planetType = TERRESTIAL;
		density = 0.5;
		circle.setPointCount(40);
	}
	else if (mass < GASGIANTLIMIT)
	{
		planetType = GASGIANT;
		density = 0.3;
		circle.setPointCount(50);
	}
	else if (mass < SMALLSTARLIMIT)
	{
		planetType = SMALLSTAR;
		density = 0.2;
		circle.setOutlineColor(sf::Color(255, 0, 0, 60));
		circle.setOutlineThickness(3);
		circle.setPointCount(90);
	}
	else if (mass < STARLIMIT)
	{
		planetType = STAR;
		density = 0.15;
		circle.setOutlineThickness(7);
		circle.setPointCount(90);
	}
	else if (mass < BIGSTARLIMIT)
	{
		planetType = BIGSTAR;
		density = 0.1;
		circle.setOutlineColor(sf::Color(150, 150, 255, 60));
		circle.setOutlineThickness(10);
		circle.setPointCount(150);
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
		radi = 2 * mass * G / (SPEED_OF_LIGHT * SPEED_OF_LIGHT);
		radi *= 3e16;
	}
	else
	{
		radi = cbrt(mass) / density;
	}
	circle.setRadius(radi);
	circle.setOrigin(radi, radi);
}

void Planet::resetAttractorMeasure()
{
	STRENGTH_strongest_attractor = 0;
}

void Planet::incMass(double m)
{
	mass += m;
	updateRadiAndType();
}

void Planet::collision(const Planet& p)
{
	xv = (mass * xv + p.mass * p.getxv()) / (mass + p.getmass());
	yv = (mass * yv + p.mass * p.getyv()) / (mass + p.getmass());

	const auto dXV = xv - p.getxv();
	const auto dYV = yv - p.getyv();

	increaseThermalEnergy(COLLISION_HEAT_MULTIPLIER * ((dXV * dXV + dYV * dYV) * p.getmass()));
}

void Planet::render_shine(sf::RenderWindow& window, sf::Color col, const double luminosity) const
{
	sf::VertexArray vertexArr(sf::TrianglesFan);
	vertexArr.append(sf::Vertex(sf::Vector2f(getx(), gety()), col));
	col.a = 0;
	const auto delta_angle = 2 * PI / static_cast<double>(LIGHT_NUMBER_OF_VERTECES);
	auto angle = 0.0;
	auto rad = luminosity;
	for (size_t nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; nr++)
	{
		sf::Vector2f pos(getx() + cos(angle) * rad, 
		                 gety() + sin(angle) * rad);
		vertexArr.append(sf::Vertex(pos, col));
		angle += delta_angle;
	}
	vertexArr.append(sf::Vertex(sf::Vector2f(getx() + rad, gety()), col));
	window.draw(vertexArr);
}

void Planet::draw_starshine(sf::RenderWindow& window) const
{
	sf::Color col = getStarCol();

	//LONG RANGE LIGHT
	col.a = 50;
	const auto long_range_luminosity = 30 * sqrt(fusionEnergy());
	render_shine(window, col, long_range_luminosity);

	//SHORT RANGE LIGHT
	col.a = 250;
	const auto short_range_luminosity = 1.5 * getRad();
	render_shine(window, col, short_range_luminosity);
}

void Planet::draw_planetshine(sf::RenderWindow& window) const
{
	/*
	 *	Caused by very hot temperatures
	 */

	sf::Color col{ sf::Color::White };
	col.a = 70;

	const auto temp = getTemp();
	auto temp_effect_by_temp = [temp]() {
		if (temp < 500.0)
			return 0.0;
		else
			return sqrt((temp - 500.0)) / 30.0;
	};

	const auto temp_effect = temp_effect_by_temp();
	if (temp_effect > 0.1)
		render_shine(window, col, temp_effect * getRad());
}

sf::Color temperature_effect(double temp)
{
	sf::Uint8 r = std::clamp(temp / 5.0, 0., 255.);
	sf::Uint8 g = std::clamp(temp / 30.0, 0., 255.);
	sf::Uint8 b = std::clamp(temp / 30.0, 0., 255.);
	return { r,g,b };
}

void Planet::draw_gas_planet_atmosphere(sf::RenderWindow& window) const
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
		auto temp_effect = temperature_effect(temperature);
		double r = atmoCol_r + atmoLinesBrightness[i] + temp_effect.r;
		double g = atmoCol_g + atmoLinesBrightness[i] + temp_effect.g;
		double b = atmoCol_b + atmoLinesBrightness[i] + temp_effect.b;

		r = std::clamp(r, 0., 255.);
		g = std::clamp(g, 0., 255.);
		b = std::clamp(b, 0., 255.);

		atmoLine.setFillColor(sf::Color(r, g, b));
		window.draw(atmoLine);
	}
}

void Planet::draw(sf::RenderWindow& window)
{
	circle.setPosition(x, y);

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

	case SMALLSTAR:
	case STAR:
	case BIGSTAR:
		window.draw(circle);
		draw_starshine(window);
		break;

	case BLACKHOLE:
		window.draw(circle);
		break;
	}
}

void Planet::setColor()
{
	switch (getType())
	{
	case ROCKY:
	case TERRESTIAL:
		{
		auto temp_effect = temperature_effect(temperature);
		const double r = 100.0 + randBrightness + temp_effect.r;
		const double g = 100.0 + randBrightness + temp_effect.g + getLife().getBmass() / 20.0;
		const double b = 100.0 + randBrightness + temp_effect.b / 30.0;
		circle.setFillColor(sf::Color(std::clamp(static_cast<int>(r), 0, 255),
			std::clamp(static_cast<int>(g), 0, 255),
			std::clamp(static_cast<int>(b), 0, 255)));
		break;
		}
	case GASGIANT:
		circle.setOutlineThickness(0);
		circle.setFillColor(sf::Color::Transparent);
		/* Handled in draw() */
		break;
	case SMALLSTAR:
	case STAR:
	case BIGSTAR:
		circle.setFillColor(getStarCol());
		circle.setOutlineColor(sf::Color(circle.getFillColor().r, circle.getFillColor().g, circle.getFillColor().b,
			20));
		break;
	}
}

double Planet::getTCap() const
{
	return tCapacity;
}

void Planet::setMass(double m)
{
	mass = m;
}

void Planet::updateAtmosphere(int t)
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

	if (temperature < 600 && temperature > 200 && atmoCur < atmoPot)
	{
		atmoCur += t * 0.05;
		if (atmoCur > atmoPot) atmoCur = atmoPot;
	}
	else
	{
		atmoCur -= t * 0.1;

		if (atmoCur < 0) atmoCur = 0;
	}

	circle.setOutlineColor(sf::Color(atmoCol_r, atmoCol_g, atmoCol_b, atmoCur * atmoAlphaMult));
	circle.setOutlineThickness(sqrt(atmoCur) * atmoThicknessMult);
}

double Planet::getCurrentAtmosphere() const
{
	return atmoCur;
}

double Planet::getAtmospherePotensial() const
{
	return atmoPot;
}

void Planet::updateLife(int t)
{
	if (planetType == ROCKY || planetType == TERRESTIAL)
	{
		supportedBiomass = 100000 / (1 + (LIFE_PREFERRED_TEMP_MULTIPLIER *
			pow((temperature - LIFE_PREFERRED_TEMP), 2) + LIFE_PREFERRED_ATMO_MULTIPLIER * pow(
				(atmoCur - LIFE_PREFERRED_ATMO), 2))) - 5000;
		if (supportedBiomass < 0) supportedBiomass = 0;

		life.update(supportedBiomass, t, radi);
	}
	else
	{
		life.kill();
	}
}

void Planet::colonize(int i, sf::Color c, std::string d)
{
	life = Life(i);
	life.giveCol(c);
	life.giveDesc(d);
}

Life Planet::getLife() const
{
	return life;
}

double Planet::getSupportedBiomass() const
{
	return supportedBiomass;
}

int Planet::modernRandomWithLimits(int min, int max) const
{
	std::random_device seeder;
	std::default_random_engine generator(seeder());
	std::uniform_int_distribution<int> uniform(min, max);
	return uniform(generator);
}

std::string convertDoubleToString(double number)
{
	std::string Result;

	std::stringstream convert;

	convert << number;

	return convert.str();
}

std::string Planet::generate_name()
{
	std::vector<std::string> navn_del_en = {
		"Jup", "Jor", "Ear", "Mar", "Ven", "Cer", "Sat", "Pl", "Nep", "Ur", "Ker", "Mer", "Jov", "Qur", "Deb", "Car",
		"Xet", "Nayt", "Erist", "Hamar", "Bjork", "Deat", "Limus", "Lant", "Hypor", "Hyper", "Tell", "It", "As", "Ka",
		"Po", "Yt", "Pertat"
	};
	std::vector<std::string> navn_del_to = {"it", "enden", "orden", "eptux", "atur", "oper", "uqtor", "axax"};
	std::vector<std::string> navn_del_tre = {"er", "us", "o", "i", "atara", "ankara", "oxos", "upol", "ol", "eq"};


	int selectorOne = modernRandomWithLimits(0, navn_del_en.size() - 1);
	int selectorTwo = modernRandomWithLimits(0, navn_del_to.size() - 1);
	int selectorThree = modernRandomWithLimits(0, navn_del_tre.size() - 1);

	std::string number = "";

	if (modernRandomWithLimits(0, 100) < 20) number = " " + convertDoubleToString(modernRandomWithLimits(100, 999));


	std::string navn = navn_del_en[selectorOne] + navn_del_to[selectorTwo] + navn_del_tre[selectorThree] + number;

	return navn;
}

void Planet::setxv(double v)
{
	xv = v;
}

void Planet::setyv(double v)
{
	yv = v;
}

Planet::GoldilockInfo Planet::getGoldilockInfo() const
{
	const auto goldilock_inner_rad = (tempConstTwo * getRad() * getRad() * getTemp()) / inner_goldi_temp;
	const auto goldilock_outer_rad = (tempConstTwo * getRad() * getRad() * getTemp()) / outer_goldi_temp;
	return { goldilock_inner_rad, goldilock_outer_rad };
}
