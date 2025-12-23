#pragma once

#include "particle.h"
#include "../HeatSim.h"

class LegacyParticle : public IParticle
{	
	sf::Vector2f velocity;
	sf::CircleShape indicator;
	double temp{ 0.0 };
	double radius;

public:
	LegacyParticle(const sf::Vector2f & position, const sf::Vector2f & velocity, double size, double removal_time, double initial_temp)
		: IParticle(removal_time), velocity(velocity), radius(size), temp(initial_temp)
	{
		indicator.setFillColor(sf::Color(100, 100, 100, 100));
		indicator.setRadius(size);
		indicator.setOrigin(size, size);
		indicator.setPosition(position);
		update_color();
	}

	void render(sf::RenderWindow& window) const override
	{
		draw_heat_glow(window, indicator.getPosition(), temp, radius);
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

	void absorb_heat(double heat)
	{
		// Assuming unit mass/capacity relationship similar to planets, 
        // or effectively treating 'temp' as energy density for visual purposes.
        // If we want strict physics: dQ = m * c * dT. 
        // For dust, let's say heat increases temp directly proportional to mass?
        // But we don't store mass. 
        // Let's assume the heat passed in is ALREADY scaled by mass or we just add it to a "temp-like" energy.
        // Planet::absorbHeat does: tEnergy += heat. And tEnergy = mass * temp * cap.
        // So dT = heat / (mass * cap).
        // Let's assume mass ~ radius^3 (volume) and density=1.
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

        indicator.setFillColor(sf::Color(
            std::min(255, r), 
            std::min(255, g), 
            std::min(255, b), 
            std::min(255, alpha)
        ));
    }
    
    double get_radius() const { return radius; }
};