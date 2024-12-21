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

double Planet::getId() const
{
	return id;
}

double Planet::getG() const
{
	return G;
}

void Planet::mark(double i)
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

//SIMULATION FUNCTIONS

void Planet::updateTemp()
{
	temperature = temp();
}

bool Planet::canDisintegrate(double curr_time)
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

	incTEnergy(COLLISION_HEAT_MULTIPLIER * ((dXV * dXV + dYV * dYV) * p.getmass()));
}

void Planet::draw(sf::RenderWindow& w, double xx, double yy)
{
	circle.setPosition(x - xx, y - yy);
	if (planetType != GASGIANT)
	{
		w.draw(circle);
	}
	else
	{
		w.draw(circle);
		for (size_t i = 0; i < atmoLinesBrightness.size(); i++)
		{
			sf::CircleShape atmoLine;

			//SETTING FEATURES
			int midlLines = (numAtmoLines - 1);
			atmoLine.setRadius(
				circle.getRadius() - i * i * i * circle.getRadius() / (midlLines * midlLines * midlLines));
			atmoLine.setOrigin(atmoLine.getRadius(), atmoLine.getRadius());
			atmoLine.setPosition(circle.getPosition());
			atmoLine.setOutlineThickness(0);

			//FINDING COLOR
			double r = atmoCol_r + atmoLinesBrightness[i] + temperature / 10;
			double g = atmoCol_g + atmoLinesBrightness[i] - temperature / 15;
			double b = atmoCol_b + atmoLinesBrightness[i] - temperature / 15;

			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;

			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;

			atmoLine.setFillColor(sf::Color(r, g, b));
			w.draw(atmoLine);
		}
	}
}

void Planet::setColor()
{
	if (planetType != ROCKY && planetType != TERRESTIAL && planetType != BLACKHOLE && planetType != GASGIANT)
	{
		circle.setFillColor(getStarCol());

		circle.setOutlineColor(sf::Color(circle.getFillColor().r, circle.getFillColor().g, circle.getFillColor().b,
		                                 20));
	}
	else if (planetType == ROCKY || planetType == TERRESTIAL)
	{
		double r = 100 + randBrightness + temperature / 10;
		double g = 100 + randBrightness - temperature / 15 + getLife().getBmass() / 20;
		double b = 100 + randBrightness - temperature / 15;

		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;

		circle.setFillColor(sf::Color(r, g, b));
	}
	else if (planetType == GASGIANT)
	{
		double r = atmoCol_r + temperature / 10;
		double g = atmoCol_g - temperature / 15;
		double b = atmoCol_b - temperature / 15;

		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;

		circle.setFillColor(sf::Color(r, g, b));
	}
}

std::string convertDoubleToString(double number)
{
	std::string Result;

	std::stringstream convert;

	convert << number;

	return convert.str();
}

std::string Planet::genName()
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
