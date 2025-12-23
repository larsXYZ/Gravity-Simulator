#pragma once

#include "SFML/Graphics/View.hpp"

class Space;


class ObjectTracker
{
	int target_id{ -1 };
    bool tracks_spaceship{ false };
public:
	bool is_active() const;
	void deactivate();
	void activate(int new_target_id);
    void activate_for_spaceship();
	void update(Space& space, sf::View& view);
};