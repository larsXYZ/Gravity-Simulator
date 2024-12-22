#include "SFML/Graphics/RenderWindow.hpp"

class IParticleContainer
{
public:
	void init();
	void update(double timestep);
	void render_all(sf::RenderWindow &w);
};