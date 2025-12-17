#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include "sim_objects/planet.h"
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
	virtual void render(sf::RenderWindow &w) = 0;
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

	void render(sf::RenderWindow &w)
	{
		//LIGHT - Flash
		if (getAge(0) <= 2)
		{
			sf::Color col = sf::Color(255, 255, 200);
			col.a = EXPLOSION_LIGHT_START_STRENGTH;
			sf::VertexArray vertexArr(sf::TrianglesFan);
			vertexArr.append(sf::Vertex(getpos(), col));
			col.a = 0;

			double deltaAng = 2*PI / ((double)LIGHT_NUMBER_OF_VERTECES);
			double ang = 0;
			double rad = getsize()*getsize()*EXPLOSION_FLASH_SIZE;

			if (getAge(0) == 2) rad /= 2;

			for (int nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; nr++)
			{
				sf::Vector2f pos(getpos().x + cos(ang) * rad, getpos().y + sin(ang) * rad);
				vertexArr.append(sf::Vertex(pos, col));
				ang += deltaAng;
			}
			vertexArr.append(sf::Vertex(sf::Vector2f(getpos().x + rad, getpos().y), col));
			w.draw(vertexArr);
		}

        float life_ratio = (float)getAge(0) / levetidmax();
        float expansion = getsize() * life_ratio;

		// Main Body - Layered
        // Inner (White/Hot)
		sf::CircleShape core(expansion * 0.5f);
		core.setFillColor(sf::Color(255, 255, 220, static_cast<sf::Uint8>(255 * (1.0f - life_ratio))));
		core.setPosition(getpos());
		core.setOrigin(expansion * 0.5f, expansion * 0.5f);
		w.draw(core);

        // Middle (Yellow/Orange)
		sf::CircleShape mid(expansion * 0.8f);
		mid.setFillColor(sf::Color(255, 150, 0, static_cast<sf::Uint8>(150 * (1.0f - life_ratio))));
		mid.setPosition(getpos());
		mid.setOrigin(expansion * 0.8f, expansion * 0.8f);
		w.draw(mid);

        // Shockwave Ring
        sf::CircleShape shockwave(expansion * 1.2f);
        shockwave.setFillColor(sf::Color::Transparent);
        shockwave.setOutlineColor(sf::Color(200, 200, 255, static_cast<sf::Uint8>(100 * (1.0f - life_ratio))));
        shockwave.setOutlineThickness(2.0f);
        shockwave.setPosition(getpos());
        shockwave.setOrigin(expansion * 1.2f, expansion * 1.2f);
        w.draw(shockwave);
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
	void render(sf::RenderWindow &w)
	{

		royk.setPosition(getpos().x, getpos().y);

		w.draw(royk);
	}
	bool killMe()
	{
		return destroyme;
	}
};
