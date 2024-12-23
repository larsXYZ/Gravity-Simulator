#include "Bound.h"

#include "CONSTANTS.h"

Bound::Bound()
{
	isActive = true;
	indicator.setPosition(sf::Vector2f(0, 0));
	indicator.setOrigin(START_RADIUS, START_RADIUS);
	indicator.setRadius(START_RADIUS);
	indicator.setFillColor(sf::Color(0, 0, 0, 0));
	indicator.setOutlineColor(sf::Color(255, 0, 0, 50));
	indicator.setPointCount(100);
}

sf::Vector2f Bound::getPos() const
{
	return indicator.getPosition();
}

void Bound::setPos(sf::Vector2f p)
{
	indicator.setPosition(p);
}

void Bound::setRad(double r)
{ 
	indicator.setRadius(r);
	indicator.setOrigin(r, r);
}

double Bound::getRad() const
{
	return indicator.getRadius();
}

void Bound::setState(bool state)
{ 
	isActive = state;
}
bool Bound::getState() const
{
	return isActive;
}

bool Bound::isOutside(sf::Vector2f p) const
{
	return std::hypot((p.x - getPos().x), (p.y - getPos().y)) > indicator.getRadius();
}

void Bound::render(sf::RenderWindow& w)
{
	indicator.setPosition(getPos());
	w.draw(indicator);
}