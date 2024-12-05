#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <random>
#include "planet.h"
#include "CONSTANTS.h"

class Effect
{
private:
	sf::Vector2f pos;
	sf::Vector2f vel;
	double size;

	int timeExist = 0;
	sf::Color col;
	int id;
	int levetid;

public:

	Effect(sf::Vector2f p, double s, int i, sf::Vector2f v, int l);

	void setpos(sf::Vector2f p);
	void setsize(double s);
	void setcol(sf::Color c);
	sf::Vector2f getpos();
	double getsize();
	sf::Color getcol();
	virtual void print(sf::RenderWindow &w, int xm, int ym) = 0;
	void setID(int i);
	int getID();
	int getAge(double t);
	void move(int t);
	int levetidmax();
	void setVel(sf::Vector2f a);

	int modernRandomWithLimits(int min, int max)
	{
		std::random_device seeder;
		std::default_random_engine generator(seeder());
		std::uniform_int_distribution<int> uniform(min, max);
		return uniform(generator);
	}
};

class Explosion : public Effect
{
private:

public:
	Explosion(sf::Vector2f p, double s, int i, sf::Vector2f v, int l) : Effect(p, s, i, v, l)
	{
		setcol(sf::Color::Yellow);
	}

	void print(sf::RenderWindow &w, int xm, int ym)
	{


		//LIGHT
		if (getAge(0) == 1 || getAge(0) == 2)
		{

			sf::Color col = sf::Color(255, 255, 150);
			col.a = EXPLOSION_LIGHT_START_STRENGTH;
			sf::VertexArray vertexArr(sf::TrianglesFan);
			vertexArr.append(sf::Vertex(sf::Vector2f(getpos().x - xm, getpos().y - ym), col));
			col.a = 0;

			double deltaAng = 2*PI / ((double)LIGHT_NUMBER_OF_VERTECES);
			double ang = 0;
			double rad = getsize()*getsize()*EXPLOSION_FLASH_SIZE;

			//REDUCES SIZE IF SECOND FRAME
			if (getAge(0) == 2) rad /= 2;

			//DRAWS VERTEXES
			for (int nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; nr++)
			{
				sf::Vector2f pos(getpos().x - xm + cos(ang) * rad, getpos().y - ym + sin(ang) * rad);
				vertexArr.append(sf::Vertex(pos, col));
				ang += deltaAng;
			}
			vertexArr.append(sf::Vertex(sf::Vector2f(getpos().x + rad - xm, getpos().y - ym), col));
			w.draw(vertexArr);
		}


		double rad = getsize() * getAge(0) / levetidmax();
		sf::CircleShape eksplosjon(rad);

		eksplosjon.setFillColor(getcol());
		eksplosjon.setPosition(getpos().x - xm, getpos().y - ym);
		eksplosjon.setOrigin(rad, rad);

		w.draw(eksplosjon);
	}
};

class Smoke : public Effect
{
private:
	bool destroyme = false;
	double xpos[10];
	double ypos[10];
	double rad =  EXPLOSION_SIZE + RANDOMPERCENTAGESIZE/10*modernRandomWithLimits(-10*EXPLOSION_SIZE, 10*EXPLOSION_SIZE);
	sf::CircleShape royk;

public:
	Smoke(sf::Vector2f p, double s, int i, sf::Vector2f v, int l) : Effect(p, s, i, v, l + modernRandomWithLimits(-0.3*l, 0.3*l))
	{

		setcol(sf::Color(200,200,200,100));
		royk.setRadius(rad);
		royk.setOrigin(rad, rad);
		royk.setFillColor(getcol());
#if PARTICLES_PER_SMOKE != 0
		for (int i = 0; i < PARTICLES_PER_SMOKE; i++)
		{
			xpos[i] = (double) modernRandomWithLimits(-EXPLOSION_SIZE, EXPLOSION_SIZE) / PARTICLES_PER_SMOKE;
			ypos[i] = (double) modernRandomWithLimits(-EXPLOSION_SIZE, EXPLOSION_SIZE) / PARTICLES_PER_SMOKE;
		}
#endif
	}

	void print(sf::RenderWindow &w, int xm, int ym)
	{
		royk.setPosition(getpos().x - xm, getpos().y - ym);
		w.draw(royk);

		if (PARTICLES_PER_SMOKE > 0)
		{
			sf::CircleShape p(0.5);
			for (int i = 0; i < PARTICLES_PER_SMOKE; i++)
			{
				p.setPosition(sf::Vector2f(getpos().x + xpos[i], getpos().y + ypos[i]));
				w.draw(p);
			}
		}

	}
	void pullOfGravity(Planet forcer, int timeStep)
	{

		//CHECK IF PLANET IS MASSIVE ENOUGH FOR US TO CARE
		if (forcer.getmass() < DUST_MIN_PHYSICS_SIZE) return;


		//MATH
		double distanceSquared = (forcer.getx() - getpos().x)*(forcer.getx() - getpos().x) + (forcer.gety() - getpos().y) * (forcer.gety() - getpos().y);
		double angle = atan2(forcer.gety() - getpos().y, forcer.getx() - getpos().x);
		double f = forcer.getG() * forcer.getmass() / (distanceSquared);
		f *= SMK_ACCURACY*timeStep;

		//DOING THE BUSINESS
		if (distanceSquared > forcer.getRad()*forcer.getRad())
		{
			setVel(sf::Vector2f(f*cos(angle), f*sin(angle)));
		}
		else if(forcer.getRad() > COLLISION_SIZE_MULTIPLIER*MINIMUMBREAKUPSIZE)
		{
			destroyme = true;
		}
	}
	
	bool killMe()
	{
		return destroyme;
	}
};

class Trail : public Effect
{
private:
	sf::CircleShape royk;
	bool destroyme = false;

public:
	Trail(sf::Vector2f p,  int l) : Effect(p,0,0,sf::Vector2f(0,0),l)
	{
		setcol(sf::Color(0, 255, 255,130));
		royk.setRadius(TRAILRAD);
		royk.setFillColor(getcol());
		royk.setOrigin(TRAILRAD, TRAILRAD);
	}
	void print(sf::RenderWindow &w, int xm, int ym)
	{

		royk.setPosition(getpos().x - xm, getpos().y - ym);

		w.draw(royk);
	}
	bool killMe()
	{
		return destroyme;
	}
};
