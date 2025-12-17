#pragma once
#include <SFML/Graphics.hpp>
#include "sim_objects/planet.h"
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
	double mass = 1;
	int isFiring;
	bool isLanded;
	int planetID;
	double angleHold;
	sf::Vector2f posHold;
	bool exist;
	int timeAtGround;
	double maxCollisionSpeed = 0.5;

	sf::RectangleShape ship;

    // Gun
    std::vector<Projectile> projectiles;
    int shoot_cooldown{ 0 };
    double charge_level{ 0.0 };
    bool is_charging{ false };

    // Tug
    bool tug_active{ false };
    int tug_target_id{ -1 };

public:

	SpaceShip();
	SpaceShip(sf::Vector2f p);

	int move(int timeStep);
	bool pullofGravity(Planet forcer, SpaceShip &ship, int timeStep);
	sf::Vector2f getpos();
	sf::Vector2f getvel();
	void reset(sf::Vector2f p);
	int getPlanetID();
	bool getLandedState();
	void setLandedstate(bool state);
	void destroy();
	double getMaxCollisionSpeed();
	bool isExist();
	float getAngle();
	
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
    void updateTug(Space& space, double dt);
    void renderTug(sf::RenderWindow& window, Space& space);
};