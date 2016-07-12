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

	//CONSTRUCTOR
	Bound()
	{
		isActive = true;
		omr.setPosition(sf::Vector2f(0, 0));
		omr.setOrigin(START_RADIUS, START_RADIUS);
		omr.setRadius(START_RADIUS);
		omr.setFillColor(sf::Color(0, 0, 0, 0));
		omr.setOutlineColor(sf::Color(255,0,0,50));
		omr.setPointCount(100);
	}

	//GET & SET
	sf::Vector2f getPos() { return omr.getPosition(); }
	void setPos(sf::Vector2f p) { omr.setPosition(p); }
	void setRad(double r) { omr.setRadius(r); omr.setOrigin(r,r); }
	double getRad() { return omr.getRadius(); }

	//STATE
	void setState(bool state) { isActive = state; }
	bool getState() { return isActive; }

	//CHECK IF OBJECT IS OUTSIDE BOUND
	bool isOutside(sf::Vector2f p)
	{
		if (sqrt((p.x - getPos().x)*(p.x - getPos().x) + (p.y - getPos().y)*(p.y - getPos().y)) > omr.getRadius())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//DRAW
	void draw(sf::RenderWindow& w, double xx, double yy, double z)
	{
		omr.setOutlineThickness(BOUND_THICKNESS*z);
		omr.setPosition(sf::Vector2f(getPos().x - xx, getPos().y - yy));
		w.draw(omr);
		omr.setPosition(sf::Vector2f(getPos().x + xx, getPos().y + yy));
	}

};