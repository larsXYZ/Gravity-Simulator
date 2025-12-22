#pragma once
#include "CONSTANTS.h"
#include <cmath>
#include <vector>
#include <string>
#include <SFML/Graphics/Color.hpp>
#include <random>

class Life {
private:
	int id;
	double biomass;
	lType type;
	int lifeLevel;
	bool expand;
	std::string description;
	std::string civName;
	int timer;
	sf::Color lifeColor;

public:
	// Constructors
	Life();
	explicit Life(int i);

	// Life functions
	void giveId(int i);
	void giveCol(sf::Color c);
	void giveDesc(std::string d);
	void giveCivName(std::string cn);
	void update(double supportedBM, int t, double rad);
	void kill();
	[[nodiscard]] bool willExp() const noexcept;
	void genDesc();
	void genCivName();

	// Getters
	[[nodiscard]] lType getTypeEnum() const;
	[[nodiscard]] double getBmass() const;
	[[nodiscard]] std::string getType() const;
	[[nodiscard]] int getId() const;
	[[nodiscard]] sf::Color getCol() const;
	[[nodiscard]] std::string getDesc() const;
	[[nodiscard]] std::string getCivName() const;

private:
	static int modernRandomWithLimits(int min, int max);
};
