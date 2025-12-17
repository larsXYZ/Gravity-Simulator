#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include "CONSTANTS.h"

// Returns the color offset for a given temperature (for rocky planets/dust)
sf::Color temperature_effect(double temp);

// Calculates energy lost due to cooling (Stefan-Boltzmann law variant)
double calculate_cooling(double temp, double radius, double timestep);

// Calculates energy absorbed from a source (Inverse distance law variant)
double calculate_heating(double radius, double source_emitted_energy, double distance);

// Renders a glow/shine effect at a given position
void render_shine(sf::RenderWindow& window, sf::Vector2f position, const sf::Color& col, double luminosity);

// Implementation of the "planet-like" heat glow effect
void draw_heat_glow(sf::RenderWindow& window, sf::Vector2f position, double temp, double radius);

class StarColorInterpolator
{
	struct TempColorPair
	{
		double temperature;
		sf::Color color;
	};

	std::vector<TempColorPair> temperatures_and_colors;

public:

	StarColorInterpolator();

	sf::Color interpolate(TempColorPair a, TempColorPair b, double temp) const;

	sf::Color getStarColor(double temperature) const;
};
