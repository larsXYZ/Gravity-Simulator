#pragma once
#include <SFML/Graphics.hpp>
#include "sim_objects/planet.h"
#include "user_functions.h"
#include <deque>

class Space;

struct Projectile {
    sf::Vector2f pos;
    sf::Vector2f vel;
    int life;
    double power;
    std::deque<sf::Vector2f> trail;
};

class SpaceShip
{
	sf::Vector2f pos;
	sf::Vector2f speed;
	double acc;
	float angle;
    float angular_velocity{ 0.0f };
	double mass = 1;
	int isFiring;
	bool exist;
	double maxCollisionSpeed = 0.5;

	sf::RectangleShape ship; // Will be replaced by custom drawing

    // Energy cannon
    std::vector<Projectile> projectiles;
    int shoot_cooldown{ 0 };
    double charge_level{ 0.0 };
    bool is_charging{ false };

    // Tug
    bool tug_active{ false };
    int tug_target_id{ -1 };
    double tug_rest_length{ 0.0 };
    float tug_activity_score{ 0.0f };

    struct GrappleHook {
        sf::Vector2f pos;
        sf::Vector2f vel;
        bool flying{ false };
    } grapple;

    // Shield
    float shield_active_timer{ 0.0f };
    const float shield_radius{ 30.0f };
    double shield_energy{ SHIELD_MAX_ENERGY };
    int shield_recovery_pause_timer{ 0 };

    // Trajectory
    bool trajectory_active{ false };
    PredictionResult last_prediction;

    friend class Space;

public:
    enum class Tool { ENERGY_CANNON, GRAPPLE, TRAJECTORY };

private:
    Tool current_tool{ Tool::ENERGY_CANNON };
    bool space_key_prev{ false };

public:

	SpaceShip();
	SpaceShip(sf::Vector2f p);

	int move(int timeStep);
    void checkShield(Space& space, double dt);
    void handleInput(Space& space, double dt);
    void switchTool(Space& space);
    std::string getToolName() const;

	bool pullofGravity(Planet forcer, SpaceShip &ship, int timeStep, bool gravity_enabled);
	sf::Vector2f getpos();
	sf::Vector2f getvel();
	void reset(sf::Vector2f p);
    // Removed landing getters/setters
	void destroy();
	double getMaxCollisionSpeed();
    double getShieldEnergy() const { return shield_energy; }
	bool isExist();
	float getAngle();
    void missileHit(Space& space);
	
	void draw(sf::RenderWindow &w);

    // New Features
    void startCharge();
    void updateCharge(double dt);
    void shoot(Space& space);
    void updateProjectiles(double dt, Space& space);
    void renderProjectiles(sf::RenderWindow& window);
    void renderCharge(sf::RenderWindow& window);
    void checkProjectileCollisions(Space& space, double dt);

    void toggleTug(Space& space);
    void shootGrapple();
    void updateGrapple(double dt, Space& space);
    void updateTug(Space& space, double dt);
    void renderTug(sf::RenderWindow& window, Space& space);

    void updateTrajectory(Space& space);
    void renderTrajectory(sf::RenderWindow& window, float zoom);
};