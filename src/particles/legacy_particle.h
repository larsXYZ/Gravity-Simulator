#pragma once

#include "particle.h"
#include "src/CONSTANTS"

class LegacyParticles : public IParticle
{	
	sf::Vector2f velocity;
	sf::CircleShape indicator;
	bool remove_me{ false };

public:
	LegacyParticles(const sf::Vector2f & position, double size, const sf::Vector2f & velocity)
		: velocity(velocity)
	{
		indicator.setFillColor(sf::Color(200, 200, 200, 100));
		indicator.setRadius(size);
		indicator.setOrigin(size, size);
		indicator.setPosition(position);
	}

	void render(sf::RenderWindow& window) override
	{
		window.draw(indicator);
	}

	void update_based_on_planets(const std::vector<Planet>& planets, int timestep, double curr_time) override
	{
		for (const auto & planet : planets)
		{
			//CHECK IF PLANET IS MASSIVE ENOUGH FOR US TO CARE
			if (planet.getmass() < DUST_MIN_PHYSICS_SIZE)
				return;

			//MATH
			const auto curr_pos = indicator.getPosition();
			const auto distanceSquared = (planet.getx() - curr_pos.x) * (planet.getx() - curr_pos.x)
										+ (planet.gety() - curr_pos.y) * (planet.gety() - curr_pos.y);
			double angle = atan2(planet.gety() - curr_pos.y, planet.getx() - curr_pos.x);

			const auto acceleration = G * planet.getmass() / (distanceSquared);

			//DOING THE BUSINESS
			if (distanceSquared > planet.getRad() * planet.getRad())
			{
				velocity = static_cast<float>(timestep) * sf::Vector2f(acceleration * cos(angle), 
																		acceleration * sin(angle));
			}
			else if (planet.getRad() > COLLISION_SIZE_MULTIPLIER * MINIMUMBREAKUPSIZE &&
				!planet.disintegrationGraceTimeIsActive(curr_time))
			{
				remove_me = true;
			}
		}

		
	}

	bool killMe()
	{
		return destroyme;
	}
};