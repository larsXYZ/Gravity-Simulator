#pragma once

#include <string>
#include <vector>

#include "SFML/Window/Keyboard.hpp"
#include "TGUI/Widgets/ListBox.hpp"

enum class FunctionType
{
	NEW_OBJECT,
	OBJECT_IN_ORBIT,
	ADVANCED_OBJECT_IN_ORBIT,
	REMOVE_OBJECT,
	EXPLODE_OBJECT,
	RESET_SIMULATION,
	SPAWN_SHIP,
	SHOW_INFO,
	ADD_RINGS,
	RANDOM_SYSTEM,
	FOLLOW_OBJECT,
	ADD_BOUND,
	NO_FUNCTION
};

struct Function
{
	sf::Keyboard::Key hotkey;
	std::string label;
	FunctionType type;
};

const std::vector<Function> function_info
{
	{sf::Keyboard::N, "New Object (N)", FunctionType::NEW_OBJECT},
	{sf::Keyboard::O, "New Object in orbit (O)", FunctionType::OBJECT_IN_ORBIT},
	{sf::Keyboard::S, "Adv Object in orbit (S)", FunctionType::ADVANCED_OBJECT_IN_ORBIT},
	{sf::Keyboard::D, "Remove object (D)", FunctionType::REMOVE_OBJECT},
	{sf::Keyboard::C, "Explode object (C)", FunctionType::EXPLODE_OBJECT},
	{sf::Keyboard::G, "Random system(G)", FunctionType::RANDOM_SYSTEM},
	{sf::Keyboard::E, "Spawn ship (E)", FunctionType::SPAWN_SHIP},
	{sf::Keyboard::Q, "Rings (Q)", FunctionType::ADD_RINGS},
	{sf::Keyboard::I, "Info (I)", FunctionType::SHOW_INFO},
	{sf::Keyboard::F, "Follow object (F)", FunctionType::FOLLOW_OBJECT},
	{sf::Keyboard::B, "Bound (B)", FunctionType::ADD_BOUND}
};

FunctionType getSelectedFunction(tgui::ListBox::Ptr listbox);

void fillFunctionGUIDropdown(tgui::ListBox::Ptr listbox);

void setFunctionGUIFromHotkeys(tgui::ListBox::Ptr listbox);