#pragma once

#include "particle.h"

class LegacyParticle : public IParticle
{	
	sf::Vector2f velocity;
	sf::CircleShape indicator;

public:
	LegacyParticle(const sf::Vector2f & position, const sf::Vector2f & velocity, double size, double removal_time)
		: IParticle(removal_time), velocity(velocity)
	{
		indicator.setFillColor(sf::Color(200, 200, 200, 100));
		indicator.setRadius(size);
		indicator.setOrigin(size, size);
		indicator.setPosition(position);
	}

	void render(sf::RenderWindow& window) const override
	{
		window.draw(indicator);
	}

	void move(double timestep) override
	{
		indicator.move(velocity * static_cast<float>(timestep));
	}

	void set_velocity(const sf::Vector2f& velocity_) override
	{
		velocity = velocity_;
	}

	sf::Vector2f get_position() const override
	{
		return indicator.getPosition();
	}

	sf::Vector2f get_velocity() const override
	{
		return velocity;
	}
};