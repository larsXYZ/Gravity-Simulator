#pragma once


#include "SFML/Graphics/RenderWindow.hpp"

class IParticleContainer
{
public:
	~IParticleContainer() = default;
	virtual void update(const std::vector<Planet> & planets, const Bound &bound, double timestep, double curr_time, bool gravity_enabled, bool heat_enabled) = 0;
	virtual void render_all(sf::RenderWindow &w) = 0;
	virtual void add_particle(const sf::Vector2f& position, const sf::Vector2f& velocity, double size, double removal_time, double initial_temp) = 0;
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

	void update(const std::vector<Planet>& planets, const Bound& bound, double timestep, double curr_time, bool gravity_enabled, bool heat_enabled) override
	{
		next_dec_simulation_target();

		auto& target_particles = particles[current_dec_simulation_target];

		#pragma omp parallel for if(target_particles.size() > 500)
		for (int i = 0; i < (int)target_particles.size(); ++i)
		{
			auto& particle = target_particles[i];

			for (const auto& planet : planets)
			{
				if (planet.getMass() < DUST_MIN_PHYSICS_SIZE)
					continue;

				const auto curr_pos = particle.get_position();

				const auto dx = planet.getx() - curr_pos.x;
				const auto dy = planet.gety() - curr_pos.y;
				const auto distanceSquared = dx * dx + dy * dy;

				double dist = 1.0;
				bool dist_calculated = false;

				if (heat_enabled && planet.emitsHeat())
				{
					dist = std::max(static_cast<double>(std::sqrt(distanceSquared)), 1.0);
					dist_calculated = true;

					double emitted = planet.giveThermalEnergy(timestep * decimation_factor);
					double heat = calculate_heating(particle.get_radius(), emitted, dist);
					particle.absorb_heat(heat);
				}

				if (gravity_enabled)
				{
					double real_dist;
					if (dist_calculated)
						real_dist = dist;
					else
						real_dist = std::sqrt(distanceSquared);

					if (real_dist < 0.1) real_dist = 0.1;

					// F = G * M * m / r^2
					// a = F / m = G * M / r^2
					// ax = a * (dx / r) = G * M * dx / r^3
					
					const double r3 = real_dist * real_dist * real_dist;
					const double A_div_r3 = (G * planet.getMass()) / r3;

					const auto acceleration = sf::Vector2f(
						static_cast<float>(A_div_r3 * dx),
						static_cast<float>(A_div_r3 * dy)
					);

					const auto dv = static_cast<float>(timestep) * static_cast<float>(decimation_factor) * acceleration;
					particle.set_velocity(particle.get_velocity() + dv);
				}
			}

			particle.cool_down(timestep * decimation_factor);
		}

		for (auto& particle_vector : particles)
		{
			std::erase_if(particle_vector, [&](auto& particle) {
				if (particle.to_be_removed(curr_time)) return true;

				particle.move(timestep);

				if (bound.isActive() && bound.isOutside(particle.get_position()))
					return true;

				for (const auto& planet : planets)
				{
					if (planet.getMass() < DUST_MIN_PHYSICS_SIZE)
						continue;

					const float dx = planet.getx() - particle.get_position().x;
					const float dy = planet.gety() - particle.get_position().y;
					const float distanceSquared = dx * dx + dy * dy;

					if (distanceSquared <= planet.getRadius() * planet.getRadius() &&
						!planet.disintegrationGraceTimeIsActive(curr_time))
					{
						return true;
					}
				}
				return false;
			});
		}
	}

	void render_all(sf::RenderWindow& window) override
	{
		for (auto& particle_vector : particles)
			for (auto& particle : particle_vector)
				particle.render(window);
	}

	void add_particle(const sf::Vector2f& position, const sf::Vector2f& velocity, double size, double removal_time, double initial_temp) override
	{
		auto smallest_vector = std::min_element(particles.begin(),
			particles.end(), [](const auto& vec1, const auto& vec2) {return vec1.size() < vec2.size(); });

		smallest_vector->push_back(LegacyParticle(
			position,
			velocity,
			size,
			removal_time,
			initial_temp
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
