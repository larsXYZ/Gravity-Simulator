#include "Effect.h"

Effect::Effect(sf::Vector2f p, double s, int i, sf::Vector2f v,int l)
{
	(void) i;

	timeExist = 0;
	setpos(p);
	setsize(s);
	vel = v;
	levetid = l;
}

void Effect::setpos(sf::Vector2f p)
{
	pos = p;
}

void Effect::setsize(double s)
{
	size = s;
}

void Effect::setcol(sf::Color c)
{
	col = c;
}

sf::Vector2f Effect::getpos() const
{
	return pos;
}

double Effect::getsize() const
{
	return size;
}

sf::Color Effect::getcol() const
{
	return col;
}

void Effect::setID(int i)
{
	id = i;
}

int Effect::getID() const
{
	return id;
}

int Effect::getAge(double t)
{

	timeExist += t;

	return timeExist;
}

void Effect::move(int t)
{
	setpos(sf::Vector2f(getpos().x + vel.x*t, getpos().y + vel.y*t));
}

int Effect::maxLifeTime() const
{
	return levetid;
}

void Effect::setVel(sf::Vector2f a)
{
	vel.x += a.x;
	vel.y += a.y;
}
