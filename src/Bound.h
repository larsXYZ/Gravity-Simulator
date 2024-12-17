#pragma once
#include <SFML/Graphics.hpp>

class Bound
{
	bool isActive;
	sf::CircleShape indicator;

public:

	Bound();

	sf::Vector2f getPos() const;

	void setPos(sf::Vector2f p);

	void setRad(double r);

	double getRad() const;

	void setState(bool state);

	bool getState() const;

	bool isOutside(sf::Vector2f p) const;

	void draw(sf::RenderWindow& w, double xx, double yy, double z);

};