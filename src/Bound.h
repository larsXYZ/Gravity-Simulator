#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>

class Bound
{
private:

	bool isActive;
	sf::CircleShape omr;

public:

	Bound();

	sf::Vector2f getPos();

	void setPos(sf::Vector2f p);

	void setRad(double r);

	double getRad();

	void setState(bool state);

	bool getState();

	bool isOutside(sf::Vector2f p);

	void draw(sf::RenderWindow& w, double xx, double yy, double z);

};