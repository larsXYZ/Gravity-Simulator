#include "src/planet.h"

class IParticle
{

public:

	~IParticle() = default;

	/*
	 * Render the particle in the window
	 */
	virtual void render(sf::RenderWindow& w) = 0;

	/*
	 * Update the particle by the planets
	 * Should handle two things: movement update and collision detection
	 */
	virtual void update_based_on_planets(const std::vector<Planet> & planets, 
										int timestep,
										double curr_time) = 0;

	/*
	 * Particle should be removed from the simulation
	 */
	virtual bool to_be_removed() = 0;
};
