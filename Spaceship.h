#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "planet.h"
#include "Effect.h"
#include <vector>

class SpaceShip
{
private:
	sf::Vector2f pos;
	sf::Vector2f speed;
	double acc;
	float angle;
	double mass = 1;
	int isFiring;
	bool isLanded;
	int planetID;
	double angleHold;
	sf::Vector2f posHold;
	bool exist;
	int timeAtGround;
	double maxCollisionSpeed = 0.5;

	sf::RectangleShape ship;

public:

	SpaceShip();
	SpaceShip(sf::Vector2f p);

	int move(int tidsskritt);
	bool pullofGravity(Planet forcer, SpaceShip &ship, int tidsskritt);
	sf::Vector2f getpos();
	sf::Vector2f getvel();
	void reset(sf::Vector2f p);
	int getPlanetID();
	bool getLandedState();
	void setLandedstate(bool state);
	void destroy();
	double getMaxCollisionSpeed();
	bool isExist();
	float getAngle();
	
	void draw(sf::RenderWindow &w, int midlx, int midly);
};