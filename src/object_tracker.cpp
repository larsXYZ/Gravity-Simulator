#include "object_tracker.h"

#include "Space.h"
#include "Spaceship.h"

bool ObjectTracker::is_active() const
{
	return target_id != -1 || tracks_spaceship;
}

void ObjectTracker::deactivate()
{
	target_id = -1;
    tracks_spaceship = false;
}

void ObjectTracker::activate(int new_target_id)
{
	target_id = new_target_id;
    tracks_spaceship = false;
}

void ObjectTracker::activate_for_spaceship()
{
    target_id = -1;
    tracks_spaceship = true;
}

void ObjectTracker::update(Space& space, sf::View& view)
{
    if (tracks_spaceship)
    {
        if (space.ship.isExist())
            view.setCenter(space.ship.getpos());
        else
            deactivate();
        return;
    }

	Planet* target = space.findPlanetPtr(target_id);
	if (!target)
	{
		deactivate();
		return;
	}

	view.setCenter(target->getx(), target->gety());
}
