#include "Spaceship.h"

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
			flamme.setPosition(pos.x - 3 * cos(2 * PI*angle / 360), pos.y - 3 * sin(2 * PI*angle / 360));
			w.draw(flamme);

			sf::CircleShape flamme3(1.5);
			flamme3.setFillColor(sf::Color::Yellow);
			flamme3.setOrigin(4.5, 1.5);
			flamme3.setRotation(angle);
			flamme3.setPosition(pos.x - 3 * cos(2 * PI*angle / 360), pos.y - 3 * sin(2 * PI*angle / 360));
			w.draw(flamme3);

			sf::CircleShape flamme2(1);
			flamme2.setFillColor(sf::Color::Red);
			flamme2.setOrigin(1, 1);
			flamme2.setPosition(pos.x - 4 * cos(2 * PI*angle / 360), pos.y - 4 * sin(2 * PI*angle / 360));
			w.draw(flamme2);
		}
		if (isFiring == -1)
		{
			sf::CircleShape flamme(2);
			flamme.setFillColor(sf::Color::Yellow);
			flamme.setOrigin(4, 2);
			flamme.setRotation(180 + angle);
			flamme.setPosition(pos.x + 3 * cos(2 * PI*angle / 360), pos.y + 3 * sin(2 * PI*angle / 360));
			w.draw(flamme);

			sf::CircleShape flamme3(1.5);
			flamme3.setFillColor(sf::Color::Yellow);
			flamme3.setOrigin(4.5, 1.5);
			flamme3.setRotation(180 + angle);
			flamme3.setPosition(pos.x + 3 * cos(2 * PI*angle / 360), pos.y + 3 * sin(2 * PI*angle / 360));
			w.draw(flamme3);

			sf::CircleShape flamme2(1);
			flamme2.setFillColor(sf::Color::Red);
			flamme2.setOrigin(1, 1);
			flamme2.setRotation(180 + angle);
			flamme2.setPosition(pos.x + 4 * cos(2 * PI*angle / 360), pos.y + 4 * sin(2 * PI*angle / 360));
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
