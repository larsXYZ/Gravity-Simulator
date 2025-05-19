#pragma once
#include <SFML/Graphics.hpp>

class Bound
{
	bool is_active;
	sf::CircleShape indicator;

public:

	Bound();

	sf::Vector2f getPos() const;

	void setPos(sf::Vector2f p);

	void setRad(double r);

	double getRadius() const;

	void setActiveState(bool state);

	bool isActive() const;

	bool isOutside(sf::Vector2f p) const;

	void render(sf::RenderWindow& window, float zoom);

};