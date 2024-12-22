#pragma once


#include "SFML/Graphics/RenderWindow.hpp"

class IParticleContainer
{
public:
	~IParticleContainer() = default;
	virtual void update(const std::vector<Planet> & planets, const Bound &bound, double timestep, double curr_time) = 0;
	virtual void render_all(sf::RenderWindow &w) = 0;
	virtual void add_particle(const sf::Vector2f& position, const sf::Vector2f& velocity, double size, double lifespan) = 0;
	virtual void clear() = 0;
	virtual size_t size() = 0;
};





#include <vector>

#include "legacy_particle.h"

class DecimatedLegacyParticleContainer : public IParticleContainer
{
	std::vector<LegacyParticle> particles;

public:

	void update(const std::vector<Planet>& planets, const Bound& bound, double timestep, double curr_time) override
	{
		for (const auto & planet : planets)
		{
			if (planet.getmass() < DUST_MIN_PHYSICS_SIZE)
				continue;

			for (auto & particle : particles)
			{
				const auto curr_pos = particle.get_position();
				const auto distanceSquared = (planet.getx() - curr_pos.x) * (planet.getx() - curr_pos.x)
					+ (planet.gety() - curr_pos.y) * (planet.gety() - curr_pos.y);
				const auto angle = atan2(planet.gety() - curr_pos.y, planet.getx() - curr_pos.x);

				const auto acceleration = G * planet.getmass() / (distanceSquared);
				
				if (distanceSquared > planet.getRad() * planet.getRad())
				{
					const auto dv = static_cast<float>(timestep) * sf::Vector2f(acceleration * cos(angle),
						acceleration * sin(angle));

					particle.set_velocity(particle.get_velocity() + dv);
				}
				else if (planet.getRad() > COLLISION_SIZE_MULTIPLIER * MINIMUMBREAKUPSIZE &&
					!planet.disintegrationGraceTimeIsActive(curr_time))
				{
					particle.mark_for_removal();
				}
			}
		}

		for (auto& particle : particles)
			particle.move(timestep);

		std::remove_if(particles.begin(), particles.end(), 
			[](auto& p)
			{
				return p.to_be_removed();
			});
	}

	void render_all(sf::RenderWindow& window) override
	{
		for (const auto& particle : particles)
			particle.render(window);
	}

	void add_particle(const sf::Vector2f& position, const sf::Vector2f& velocity, double size, double lifespan) override
	{
		particles.push_back(LegacyParticle(
			position,
			velocity,
			size
		));
	}

	void clear() override
	{
		particles.clear();
	}

	size_t size() override
	{
		return particles.size();
	}
};
