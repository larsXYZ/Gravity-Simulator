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
#include <array>
#include <algorithm>
#include <numeric>

#include "legacy_particle.h"

class DecimatedLegacyParticleContainer : public IParticleContainer
{
	constexpr static size_t decimation_factor{ 4 };

	std::array<std::vector<LegacyParticle>, decimation_factor> particles;

	size_t current_dec_simulation_target{ 0 };
	void next_dec_simulation_target()
	{
		if (++current_dec_simulation_target >= decimation_factor)
			current_dec_simulation_target = 0;
	}

public:

	void update(const std::vector<Planet>& planets, const Bound& bound, double timestep, double curr_time) override
	{
		next_dec_simulation_target();

		for (const auto & planet : planets)
		{
			if (planet.getmass() < DUST_MIN_PHYSICS_SIZE)
				continue;

			for (auto & particle : particles[current_dec_simulation_target])
			{
				const auto curr_pos = particle.get_position();
				const auto distanceSquared = (planet.getx() - curr_pos.x) * (planet.getx() - curr_pos.x)
					+ (planet.gety() - curr_pos.y) * (planet.gety() - curr_pos.y);
				const auto angle = atan2(planet.gety() - curr_pos.y, planet.getx() - curr_pos.x);

				const auto A = G * planet.getmass() / (distanceSquared);
				const auto acceleration = sf::Vector2f(A * cos(angle),
														A * sin(angle));
				
				if (distanceSquared > planet.getRad() * planet.getRad())
				{
					const auto dv = static_cast<float>(timestep) * static_cast<float>(decimation_factor) * acceleration;
					particle.set_velocity(particle.get_velocity() + dv);
				}
				else if (planet.getRad() > COLLISION_SIZE_MULTIPLIER * MINIMUMBREAKUPSIZE &&
					!planet.disintegrationGraceTimeIsActive(curr_time))
				{
					particle.mark_for_removal();
				}
			}
		}

		for (auto& particle_vector : particles)
		{
			std::remove_if(particle_vector.begin(), particle_vector.end(),
				[](auto& p)
				{
					return p.to_be_removed();
				});

			for (auto& particle : particle_vector)
				particle.move(timestep);
		}
	}

	void render_all(sf::RenderWindow& window) override
	{
		for (auto& particle_vector : particles)
			for (auto& particle : particle_vector)
				particle.render(window);
	}

	void add_particle(const sf::Vector2f& position, const sf::Vector2f& velocity, double size, double lifespan) override
	{
		auto smallest_vector = std::min_element(particles.begin(),
			particles.end(), [](const auto& vec1, const auto& vec2) {return vec1.size() < vec2.size(); });

		smallest_vector->push_back(LegacyParticle(
			position,
			velocity,
			size
		));
	}

	void clear() override
	{
		for (auto & vector : particles)
			vector.clear();
	}

	size_t size() override
	{
		int total_size = std::accumulate(particles.begin(), particles.end(), 0, [](int sum, const auto& vec) {
			return sum + vec.size();
			});
		return total_size;
	}
};
