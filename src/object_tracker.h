#pragma once

#include "SFML/Graphics/View.hpp"

class Space;


class ObjectTracker
{
	int target_id{ -1 };
public:
	bool is_active() const;
	void deactivate();
	void activate(int new_target_id);
	void update(Space& space, sf::View& view);
};