#include "object_tracker.h"

#include "Space.h"

bool ObjectTracker::is_active() const
{
	return target_id != -1;
}

void ObjectTracker::deactivate()
{
	target_id = -1;
}

void ObjectTracker::activate(int new_target_id)
{
	target_id = new_target_id;
}

void ObjectTracker::update(Space& space, sf::View& view)
{
	Planet* target = space.findPlanetPtr(target_id);
	if (!target)
	{
		deactivate();
		return;
	}

	view.setCenter(target->getx(), target->gety());
}
