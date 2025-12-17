#include "Spaceship.h"
#include "space.h"
#include "CONSTANTS.h"
#include "HeatSim.h"
#include <iostream>

SpaceShip::SpaceShip(sf::Vector2f p)
{
	pos = p;
	speed.x = 0;
	speed.y = 0;
	angle = 0;
	acc = 0;
	isFiring = false;
	exist = true;
	timeAtGround = 0;

	isLanded = false;
	planetID = -1;

	ship.setOrigin(4,0);
	ship.setSize(sf::Vector2f(7,1));
	ship.setFillColor(sf::Color(200, 200, 200));
	ship.setRotation(angle);
}

SpaceShip::SpaceShip()
{
	pos.x = 0;
	pos.y = 0;
	speed.x = 0;
	speed.y = 0;
	angle = 0;
	acc = 0;
	isFiring = false;
	exist = true;
	timeAtGround = 0;

	isLanded = false;
	planetID = -1;

	ship.setOrigin(4,1);
	ship.setSize(sf::Vector2f(7,2));
	ship.setFillColor(sf::Color(200, 200, 200));
	ship.setRotation(angle);
}

int SpaceShip::move(int timeStep)
{
    if (shoot_cooldown > 0)
        shoot_cooldown -= timeStep;

	if (exist)
	{
		if (isLanded) timeAtGround += 1;
		else timeAtGround = 0;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			if (isLanded && timeAtGround > 60)
			{
				acc = 0.2;
				isLanded = false;
				isFiring = 1;
			}
			else if (!isLanded)
			{
				acc = 0.005;
				isFiring = 1;
			}
			else
			{
				isFiring = 0;
			}


		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !isLanded)
		{
			acc = -0.005;
			isFiring = -1;
		}
		else
		{
			acc = 0;
			isFiring = 0;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && !isLanded) angle -= 0.25 * timeStep;
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && !isLanded) angle += 0.25* timeStep;

		speed.x += acc * cos(2 * PI*angle / 360);
		speed.y += acc * sin(2 * PI*angle / 360);

		pos.x += speed.x * timeStep;
		pos.y += speed.y * timeStep;
	}

	return isFiring;
}

void SpaceShip::draw(sf::RenderWindow &w)
{
	if (exist)
	{
		sf::RectangleShape box(sf::Vector2f(3, 4));
		box.setFillColor(sf::Color(160, 160, 160));
		box.setOrigin(2, 2);
		box.setRotation(angle);
		box.setPosition(pos.x - 2 * cos(2 * PI*angle / 360), pos.y - 2 * sin(2 * PI*angle / 360));
		w.draw(box);


		ship.setPosition(pos);
		ship.setRotation(angle);

		if (isFiring == 1)
		{
			sf::CircleShape flamme(2);
			flamme.setFillColor(sf::Color::Yellow);
			flamme.setOrigin(4, 2);
			flamme.setRotation(angle);
            // Fixed position to match engine nozzle
			flamme.setPosition(pos.x - 5 * cos(2 * PI*angle / 360), pos.y - 5 * sin(2 * PI*angle / 360));
			w.draw(flamme);

			sf::CircleShape flamme3(1.5);
			flamme3.setFillColor(sf::Color::Yellow);
			flamme3.setOrigin(4.5, 1.5);
			flamme3.setRotation(angle);
			flamme3.setPosition(pos.x - 5 * cos(2 * PI*angle / 360), pos.y - 5 * sin(2 * PI*angle / 360));
			w.draw(flamme3);

			sf::CircleShape flamme2(1);
			flamme2.setFillColor(sf::Color::Red);
			flamme2.setOrigin(1, 1);
			flamme2.setPosition(pos.x - 6 * cos(2 * PI*angle / 360), pos.y - 6 * sin(2 * PI*angle / 360));
			w.draw(flamme2);
		}
		if (isFiring == -1)
		{
            // Reverse thrust effects
			sf::CircleShape flamme(2);
			flamme.setFillColor(sf::Color::Yellow);
			flamme.setOrigin(4, 2);
			flamme.setRotation(180 + angle);
			flamme.setPosition(pos.x + 5 * cos(2 * PI*angle / 360), pos.y + 5 * sin(2 * PI*angle / 360));
			w.draw(flamme);

			sf::CircleShape flamme3(1.5);
			flamme3.setFillColor(sf::Color::Yellow);
			flamme3.setOrigin(4.5, 1.5);
			flamme3.setRotation(180 + angle);
			flamme3.setPosition(pos.x + 5 * cos(2 * PI*angle / 360), pos.y + 5 * sin(2 * PI*angle / 360));
			w.draw(flamme3);

			sf::CircleShape flamme2(1);
			flamme2.setFillColor(sf::Color::Red);
			flamme2.setOrigin(1, 1);
			flamme2.setRotation(180 + angle);
			flamme2.setPosition(pos.x + 6 * cos(2 * PI*angle / 360), pos.y + 6 * sin(2 * PI*angle / 360));
			w.draw(flamme2);
		}

		w.draw(ship);
	}
}

sf::Vector2f SpaceShip::getpos()
{
	return pos;
}

sf::Vector2f SpaceShip::getvel()
{
	return speed;
}

float SpaceShip::getAngle()
{
	return angle;
}

int SpaceShip::getPlanetID()
{
	return planetID;
}

bool SpaceShip::getLandedState()
{
	return isLanded;
}

void SpaceShip::setLandedstate(bool state)
{
	isLanded = state;
}

bool SpaceShip::pullofGravity(Planet forcer, SpaceShip &ship, int timeStep)
{
	double dist = sqrt((forcer.getx() - ship.getpos().x)*(forcer.getx() - ship.getpos().x) + (forcer.gety() - ship.getpos().y) * (forcer.gety() - ship.getpos().y));

	if (!isLanded && dist < forcer.getRadius())
	{
		double speed = sqrt((forcer.getxv() - ship.getvel().x)*(forcer.getxv() - ship.getvel().x) + (forcer.getyv() - ship.getvel().y) * (forcer.getyv() - ship.getvel().y));
		if (speed > maxCollisionSpeed || forcer.getMass() > TERRESTIALLIMIT)
		{
			destroy();
			return false;
		}
		else
		{
			planetID = forcer.getId();
			isLanded = true;
			posHold.x = forcer.getx();
			posHold.y = forcer.gety();
			angleHold = atan2(forcer.gety() - ship.getpos().y, forcer.getx() - ship.getpos().x);
		}
	}
	else if (isLanded && forcer.getId() == planetID)
	{

		if (forcer.getMass() >= TERRESTIALLIMIT)
		{
			destroy();
			return false;
		}

		posHold.x = forcer.getx();
		posHold.y = forcer.gety();

		pos.x = posHold.x + (3.8 + forcer.getRadius())*cos(angleHold+PI);
		pos.y = posHold.y + (3.8 + forcer.getRadius())*sin(angleHold+ PI);
		ship.angle = 360*angleHold/(2 * PI)+180;

		speed.x = forcer.getxv();
		speed.y = forcer.getyv();
	}
	else if (!isLanded)
	{
		double angle = atan2(forcer.gety() - ship.getpos().y, forcer.getx() - ship.getpos().x);
		double xf = G * forcer.getMass() / (dist*dist) * cos(angle);
		double yf = G * forcer.getMass() / (dist*dist)* sin(angle);

		if (dist > forcer.getRadius())
		{
			speed.x += xf * timeStep / mass;
			speed.y += yf * timeStep / mass;
		}
	}

	return true;
}

void SpaceShip::reset(sf::Vector2f p)
{
	pos = p;

	speed.x = 0;
	speed.y = 0;

	isLanded = false;
	exist = true;
    projectiles.clear();
    tug_active = false;
    tug_target_id = -1;
    charge_level = 0.0;
    is_charging = false;
}

double SpaceShip::getMaxCollisionSpeed()
{
	return maxCollisionSpeed;
}

void SpaceShip::destroy()
{
	exist = false;
	isFiring = 0;
}

bool SpaceShip::isExist()
{
	return exist;
}

void SpaceShip::startCharge()
{
    if (!exist || shoot_cooldown > 0) return;
    is_charging = true;
}

void SpaceShip::updateCharge(double dt)
{
    if (is_charging)
    {
        charge_level += PROJECTILE_CHARGE_SPEED * dt;
        if (charge_level > PROJECTILE_MAX_CHARGE) charge_level = PROJECTILE_MAX_CHARGE;
    }
}

void SpaceShip::shoot(Space& space)
{
    if (!exist || !is_charging) return;

    is_charging = false;
    
    sf::Vector2f p_vel;
    double rad_angle = 2 * PI * angle / 360.0;
    
    p_vel.x = speed.x + cos(rad_angle) * PROJECTILE_SPEED;
    p_vel.y = speed.y + sin(rad_angle) * PROJECTILE_SPEED;

    // Spawn slightly in front
    sf::Vector2f p_pos = pos;
    p_pos.x += cos(rad_angle) * 10;
    p_pos.y += sin(rad_angle) * 10;

    double power = 1.0 + (charge_level / PROJECTILE_MAX_CHARGE) * 4.0; // 1x to 5x power

    // Recoil
    double recoil_force = 0.001 * power;
    speed.x -= cos(rad_angle) * recoil_force;
    speed.y -= sin(rad_angle) * recoil_force;

    projectiles.push_back({ p_pos, p_vel, PROJECTILE_LIFESPAN, power });
    shoot_cooldown = PROJECTILE_COOLDOWN;
    charge_level = 0.0;
}

void SpaceShip::updateProjectiles(double dt, Space& space)
{
    for (auto& p : projectiles)
    {
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.life -= dt;

        // Trail logic
        if (p.power > 2.0)
        {
            p.trail.push_front(p.pos);
            if (p.trail.size() > 20) p.trail.pop_back();
        }

        // Smoke emission scaling with power
        double smoke_chance = p.power * 2.0; // Higher power = more smoke
        if (space.uniform_random(0.0, 10.0) < smoke_chance)
        {
             // Randomize smoke position slightly perpendicular to velocity
             sf::Vector2f dir = p.vel / std::hypot(p.vel.x, p.vel.y);
             sf::Vector2f perp(-dir.y, dir.x);
             double offset = space.uniform_random(-2.0, 2.0);
             
             sf::Vector2f smoke_pos = p.pos + perp * (float)offset;
             sf::Vector2f smoke_vel = p.vel * 0.1f + sf::Vector2f(space.uniform_random(-0.5, 0.5), space.uniform_random(-0.5, 0.5));

             space.addSmoke(smoke_pos, smoke_vel, p.power * 0.6, space.uniform_random(150, 300));
        }
    }
    std::erase_if(projectiles, [](const Projectile& p) { return p.life <= 0; });
}

void SpaceShip::renderCharge(sf::RenderWindow& window)
{
    if (!is_charging) return;

    double rad_angle = 2 * PI * angle / 360.0;
    
    // Draw Aim Line
    sf::Vector2f end_pos;
    end_pos.x = pos.x + cos(rad_angle) * 1000.0f; // Long aiming line
    end_pos.y = pos.y + sin(rad_angle) * 1000.0f;

    sf::Vertex line[] =
    {
        sf::Vertex(pos, sf::Color(255, 255, 255, 50)),
        sf::Vertex(end_pos, sf::Color(255, 255, 255, 0))
    };
    window.draw(line, 2, sf::PrimitiveType::Lines);

    // Draw Charge Indicator (Growing sphere at nose)
    float charge_ratio = charge_level / PROJECTILE_MAX_CHARGE;
    float size = 2.0f + charge_ratio * 4.0f;
    
    sf::CircleShape charge_ball(size);
    charge_ball.setOrigin(size, size);
    charge_ball.setPosition(pos.x + cos(rad_angle) * 10, pos.y + sin(rad_angle) * 10);
    
    // Color shift from Yellow to Red/White
    sf::Uint8 r = 255;
    sf::Uint8 g = static_cast<sf::Uint8>(255 * (1.0f - charge_ratio));
    sf::Uint8 b = static_cast<sf::Uint8>(255 * charge_ratio); // Bluer/Whiter at high energy
    
    sf::Color glow_col(r, g, b);
    
    charge_ball.setFillColor(sf::Color(r, g, b, 200));
    window.draw(charge_ball);

    // Glow Effect
    render_shine(window, charge_ball.getPosition(), glow_col, size * 2.0 + charge_ratio * 10.0);
}

void SpaceShip::renderProjectiles(sf::RenderWindow& window)
{
    for (const auto& p : projectiles)
    {
        float size = 1.5f * p.power;
        sf::CircleShape shape(size);
        shape.setOrigin(size, size);
        
        sf::Color col = sf::Color::Yellow;
        if (p.power > 3.0) col = sf::Color(255, 200, 255); // High power is whitish/purple
        
        // Trail Rendering
        if (p.power > 2.0 && !p.trail.empty())
        {
            // Cone Glow behind projectile
            sf::VertexArray cone(sf::TrianglesFan);
            cone.append(sf::Vertex(p.pos, sf::Color(col.r, col.g, col.b, 100))); // Tip
            
            sf::Vector2f dir = -p.vel / std::hypot(p.vel.x, p.vel.y); // Backwards
            sf::Vector2f perp(-dir.y, dir.x);
            float coneLen = size * 10.0f;
            float coneWidth = size * 4.0f;
            
            cone.append(sf::Vertex(p.pos + dir * coneLen + perp * coneWidth, sf::Color(col.r, col.g, col.b, 0)));
            cone.append(sf::Vertex(p.pos + dir * coneLen - perp * coneWidth, sf::Color(col.r, col.g, col.b, 0)));
            
            window.draw(cone, sf::BlendAdd);

            sf::VertexArray trailLine(sf::PrimitiveType::LineStrip);
            for (size_t i = 0; i < p.trail.size(); ++i)
            {
                float alphaRatio = 1.0f - (float)i / p.trail.size();
                sf::Uint8 alpha = static_cast<sf::Uint8>(150 * alphaRatio);
                trailLine.append(sf::Vertex(p.trail[i], sf::Color(col.r, col.g, col.b, alpha)));

                // Glow for trail (every 5th point)
                if (i % 5 == 0)
                {
                    render_shine(window, p.trail[i], col, size * 1.5 * alphaRatio);
                }
            }
            // Use additive blending for glow effect
            window.draw(trailLine, sf::BlendAdd);
        }

        if (p.life < 1000)
        {
             col.a = static_cast<sf::Uint8>(255.0f * (p.life / 1000.0f));
        }
        shape.setFillColor(col);
        shape.setPosition(p.pos);
        window.draw(shape);

        // Projectile Glow
        render_shine(window, p.pos, col, size * 3.0);
    }
}

// Helper for segment-circle intersection
bool segmentIntersectsCircle(sf::Vector2f start, sf::Vector2f end, sf::Vector2f circlePos, float radius, sf::Vector2f& intersection)
{
    sf::Vector2f d = end - start;
    sf::Vector2f f = start - circlePos;
    
    float a = d.x*d.x + d.y*d.y;
    float b = 2*(f.x*d.x + f.y*d.y);
    float c = f.x*f.x + f.y*f.y - radius*radius;

    float discriminant = b*b - 4*a*c;
    if(discriminant < 0) return false;

    discriminant = sqrt(discriminant);
    float t1 = (-b - discriminant)/(2*a);
    float t2 = (-b + discriminant)/(2*a);

    if(t1 >= 0 && t1 <= 1)
    {
        intersection = start + t1*d;
        return true;
    }
    if(t2 >= 0 && t2 <= 1)
    {
        intersection = start + t2*d;
        return true;
    }
    return false;
}

void SpaceShip::checkProjectileCollisions(Space& space, double dt)
{
    for (auto it = projectiles.begin(); it != projectiles.end(); )
    {
        bool hit = false;
        
        sf::Vector2f start = it->pos;
        sf::Vector2f movement = it->vel * (float)dt;
        sf::Vector2f end = start + movement;

        // Iterating over planets to check collision
        for (const auto& planet : space.planets)
        {
            sf::Vector2f planetPos(planet.getx(), planet.gety());
            sf::Vector2f intersection;
            
            // Check broadphase (dist < radius + movement len)
            float dx = planet.getx() - start.x;
            float dy = planet.gety() - start.y;
            float distSq = dx*dx + dy*dy;
            float maxReach = planet.getRadius() + std::hypot(movement.x, movement.y);
            
            if (distSq > maxReach * maxReach) continue;

            if (segmentIntersectsCircle(start, end, planetPos, planet.getRadius(), intersection))
            {
                // Hit! Scale explosion by power
                space.addExplosion(intersection, 5 * it->power, {0,0}, 10 * it->power);
                
                // Damage based on power
                if (planet.getMass() < PROJECTILE_DAMAGE_MASS_LIMIT * it->power)
                {
                    // Destroy object
                    space.removePlanet(planet.getId());
                    // Visual pop
                    space.addExplosion(intersection, planet.getRadius()*2 * it->power, planet.getVelocity(), 20);
                }
                
                hit = true;
                break;
            }
        }
        
        if (hit)
            it = projectiles.erase(it);
        else
            ++it;
    }
}

void SpaceShip::toggleTug(Space& space)
{
    if (tug_active)
    {
        tug_active = false;
        tug_target_id = -1;
        return;
    }

    // Find target
    int best_id = -1;
    double best_dist = TUG_RANGE;

    for (const auto& planet : space.planets)
    {
        if (planet.getMass() > TUG_MASS_LIMIT) continue;

        double dx = planet.getx() - pos.x;
        double dy = planet.gety() - pos.y;
        double dist = std::hypot(dx, dy);

        if (dist < best_dist)
        {
            best_dist = dist;
            best_id = planet.getId();
        }
    }

    if (best_id != -1)
    {
        tug_active = true;
        tug_target_id = best_id;
    }
}

void SpaceShip::updateTug(Space& space, double dt)
{
    if (!tug_active) return;
    
    Planet* target = space.findPlanetPtr(tug_target_id);
    if (!target)
    {
        tug_active = false;
        return;
    }

    double dx = target->getx() - pos.x;
    double dy = target->gety() - pos.y;
    double dist = std::hypot(dx, dy);

    if (dist > TUG_RANGE * 1.5) // Break if too far
    {
        tug_active = false;
        return;
    }

    // Apply force
    // Force direction: from planet to ship (pull)
    double force = TUG_STRENGTH * (dist - TUG_PREFERRED_DISTANCE);
    
    double angle = atan2(dy, dx); // Angle from ship to planet
    
    // Pull planet towards ship
    sf::Vector2f p_vel = target->getVelocity();
    p_vel.x -= cos(angle) * force * dt / target->getMass();
    p_vel.y -= sin(angle) * force * dt / target->getMass();
    target->setVelocity(p_vel);
    
    // Note: Not applying reaction force to ship for "fun factor" (easier control), 
    // unless user wants realistic physics. "Tug feature" implies towing.
}

void SpaceShip::renderTug(sf::RenderWindow& window, Space& space)
{
    if (!tug_active) return;
     Planet* target = space.findPlanetPtr(tug_target_id);
    if (!target) return;

    sf::Vertex line[] =
    {
        sf::Vertex(pos, sf::Color(100, 255, 100)),
        sf::Vertex(target->getPosition(), sf::Color(100, 255, 100))
    };

    window.draw(line, 2, sf::PrimitiveType::Lines);
}
