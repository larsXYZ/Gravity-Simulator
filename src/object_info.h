#pragma once

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"

class Space;


class ObjectInfo
{
	int target_id{ -1 };
	sf::Font font;
	sf::Text text;
public:
	ObjectInfo();

	bool is_active() const;
	void deactivate();
	void activate(int new_target_id);
	void render(Space& space, sf::RenderWindow & window);
};