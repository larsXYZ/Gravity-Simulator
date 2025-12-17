#include "HeatSim.h"

sf::Color temperature_effect(double temp)
{
	sf::Uint8 r = std::clamp(temp / 5.0, 0., 255.);
	sf::Uint8 g = std::clamp(temp / 30.0, 0., 255.);
	sf::Uint8 b = std::clamp(temp / 300.0, 0., 255.);
	return { r,g,b };
}

double calculate_cooling(double temp, double radius, double timestep)
{
	return timestep * (SBconst * radius * radius * temp);
}

double calculate_heating(double radius, double source_emitted_energy, double distance)
{
	return tempConstTwo * radius * radius * source_emitted_energy / distance;
}

StarColorInterpolator::StarColorInterpolator()
{
	temperatures_and_colors.push_back({ 1600.0, sf::Color(255, 118, 0) });
	temperatures_and_colors.push_back({ 3000.0, sf::Color(255, 162, 60) });
	temperatures_and_colors.push_back({ 3700.0, sf::Color(255, 180, 107) });
	temperatures_and_colors.push_back({ 4500.0, sf::Color(255, 206, 146) });
	temperatures_and_colors.push_back({ 5500.0, sf::Color(255, 219, 186) });
	temperatures_and_colors.push_back({ 6500.0, sf::Color(255, 238, 222) });
	temperatures_and_colors.push_back({ 7200.0, sf::Color(255, 249, 251) });
	temperatures_and_colors.push_back({ 8000.0, sf::Color(240, 241, 255) });
	temperatures_and_colors.push_back({ 9000.0, sf::Color(227, 233, 255) });
	temperatures_and_colors.push_back({ 10000.0, sf::Color(214, 225, 255) });
	temperatures_and_colors.push_back({ 11000.0, sf::Color(207, 218, 255) });
	temperatures_and_colors.push_back({ 12000.0, sf::Color(200, 213, 255) });
	temperatures_and_colors.push_back({ 13000.0, sf::Color(191, 211, 255) });
}

sf::Color StarColorInterpolator::interpolate(TempColorPair a, TempColorPair b, double temp) const
{
	const auto dist_to_a = std::abs(a.temperature - temp);
	const auto dist_to_b = std::abs(b.temperature - temp);
	const auto range = std::abs(a.temperature - b.temperature);

	const auto a_ = 1.0 - dist_to_a / range;
	const auto b_ = 1.0 - dist_to_b / range;

	return sf::Color(
		a.color.r * a_ + b.color.r * b_,
		a.color.g * a_ + b.color.g * b_,
		a.color.b * a_ + b.color.b * b_,
		a.color.a * a_ + b.color.a * b_
	);
}

sf::Color StarColorInterpolator::getStarColor(double temperature) const
{
	if (temperature <= temperatures_and_colors.front().temperature)
		return temperatures_and_colors.front().color;
	if (temperature >= temperatures_and_colors.back().temperature)
		return temperatures_and_colors.back().color;

	auto lower = temperatures_and_colors.begin();
	auto higher = temperatures_and_colors.end();
	for (auto tempcol = temperatures_and_colors.begin();
		tempcol != temperatures_and_colors.end();
		++tempcol)
	{
		const auto point_temp = tempcol->temperature;
		if (point_temp < temperature)
		{
			lower = tempcol;
			higher = std::next(tempcol);
		}
		else
			break;
	}

	return interpolate(*lower, *higher, temperature);
}
