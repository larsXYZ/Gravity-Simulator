
class IParticle
{

	bool remove_me{ false };

public:

	~IParticle() = default;

	/*
	 * Render the particle in the window
	 */
	virtual void render(sf::RenderWindow& w) const = 0;

	/*
	 *	Update position of particle
	 */
	virtual void move(double timestep) = 0;

	/*
	 * Set particle velocity
	 */
	virtual void set_velocity(const sf::Vector2f& velocity) = 0;

	/*
	 * Return the particle position
	 */
	virtual sf::Vector2f get_position() const = 0;

	/*
	 * Return the particle velocity
	 */
	virtual sf::Vector2f get_velocity() const = 0;

	/*
	 * For removing particles
	 */
	void mark_for_removal() { remove_me = true; }
	bool to_be_removed() const { return remove_me; }
};
