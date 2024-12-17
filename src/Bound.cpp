#include "Bound.h"

#include "CONSTANTS.h"

Bound::Bound()
{
	isActive = true;
	omr.setPosition(sf::Vector2f(0, 0));
	omr.setOrigin(START_RADIUS, START_RADIUS);
	omr.setRadius(START_RADIUS);
	omr.setFillColor(sf::Color(0, 0, 0, 0));
	omr.setOutlineColor(sf::Color(255, 0, 0, 50));
	omr.setPointCount(100);
}

sf::Vector2f Bound::getPos() const
{
	return omr.getPosition();
}

void Bound::setPos(sf::Vector2f p)
{
	omr.setPosition(p);
}

void Bound::setRad(double r)
{ 
	omr.setRadius(r);
	omr.setOrigin(r, r);
}

double Bound::getRad() const
{
	return omr.getRadius();
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
	return sqrt((p.x - getPos().x) * (p.x - getPos().x) + (p.y - getPos().y) * (p.y - getPos().y)) > omr.getRadius();
}

void Bound::draw(sf::RenderWindow& w, double xx, double yy, double z)
{
	omr.setOutlineThickness(BOUND_THICKNESS * z);
	omr.setPosition(sf::Vector2f(getPos().x - xx, getPos().y - yy));
	w.draw(omr);
	omr.setPosition(sf::Vector2f(getPos().x + xx, getPos().y + yy));
}