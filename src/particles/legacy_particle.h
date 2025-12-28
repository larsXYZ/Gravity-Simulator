#pragma once

#include "particle.h"
#include "../HeatSim.h"

class LegacyParticle : public IParticle
{	
	sf::Vector2f velocity;
	sf::Vector2f position;
	sf::Color current_color;
	double temp{ 0.0 };
	double radius;

public:
	LegacyParticle(const sf::Vector2f & position, const sf::Vector2f & velocity, double size, double removal_time, double initial_temp)
		: IParticle(removal_time), velocity(velocity), position(position), radius(size), temp(initial_temp)
	{
		update_color();
	}

	void move(double timestep) override
	{
		position += velocity * static_cast<float>(timestep);
	}

	void set_velocity(const sf::Vector2f& velocity_) override
	{
		velocity = velocity_;
	}

	sf::Vector2f get_position() const override
	{
		return position;
	}

	sf::Vector2f get_velocity() const override
	{
		return velocity;
	}

	void absorb_heat(double heat)
	{
        double mass = radius * radius * radius; 
        if (mass < 1.0) mass = 1.0; 
        temp += heat / mass;
		if (temp > MAX_TEMP) temp = MAX_TEMP;
	}

	void cool_down(double timestep)
	{
		double cooling = calculate_cooling(temp, radius, timestep);
        double mass = radius * radius * radius;
        if (mass < 1.0) mass = 1.0;
		// Slow down cooling
		temp -= (cooling / mass) / 10.0; 
        if (temp < 0) temp = 0;
        update_color();
	}

    void update_color()
    {
        sf::Color heat_col = temperature_effect(temp);
        // Mix with base color (brighter grey)
        int base = 100;
        int r = base + heat_col.r;
        int g = base + heat_col.g;
        int b = base + heat_col.b;
        
        // Increase alpha with temperature to simulate glow
        // Base alpha 100, max 255.
        // Use redness as a proxy for 'heat visible'.
        int alpha = 100 + heat_col.r / 2;

        current_color = sf::Color(
            std::min(255, r), 
            std::min(255, g), 
            std::min(255, b), 
            std::min(255, alpha)
        );
    }
    
    double get_radius() const { return radius; }
    sf::Color get_color() const { return current_color; }
    double get_temp() const { return temp; }
};