#include "space.h"

#include "particles/particle_container.h"
#include "user_functions.h"

Space::Space(int x, int y, bool f)
	: particles(std::make_unique<DecimatedLegacyParticleContainer>())
{
	xsize = x;
	ysize = y;
	fullScreen = f;
}

int Space::addPlanet(Planet&& p)
{
	giveId(p);
	const auto id = p.getId();
	p.setTemp((p.fusionEnergy() / (p.getRad()*p.getRad()*SBconst)) + thermalEnergyAtPosition(sf::Vector2f(p.getx(), p.gety()))*tempConstTwo/SBconst);
	
	p.setColor();

	planets.push_back(std::move(p));
	return id;
}

void Space::addExplosion(sf::Vector2f p, double s, sf::Vector2f v, int l)
{
	explosions.push_back(Explosion(p, s, 0, v, l));
}

void Space::addSmoke(sf::Vector2f p,  sf::Vector2f v, double s, double lifespan)
{
	particles->add_particle(p, v, s, curr_time+lifespan);
}

sf::Vector3f Space::centerOfMass(std::vector<int> midlPList)
{
	double tMass = 0;
	double xCont = 0;
	double yCont = 0;

	for (size_t i = 0; i < midlPList.size(); i++)
	{

		Planet p = findPlanet(midlPList[i]);
		if (p.getmass() != -1)
		{
			double pMass = p.getmass();

			tMass += pMass;
			xCont += pMass*p.getx();
			yCont += pMass*p.gety();
		}

	}

	return sf::Vector3f(xCont / tMass, yCont / tMass, tMass);

}

sf::Vector2f Space::centerOfMassVelocity(std::vector<int> midlPList)
{
	double tMass = 0;
	double xCont = 0;
	double yCont = 0;

	for (size_t i = 0; i < midlPList.size(); i++)
	{

		Planet p = findPlanet(midlPList[i]);
		if (p.getmass() != -1)
		{
			double pMass = p.getmass();

			tMass += pMass;
			xCont += pMass*p.getxv();
			yCont += pMass*p.getyv();
		}

	}

	return sf::Vector2f(xCont / tMass, yCont / tMass);
}

sf::Vector2f Space::centerOfMassAll()
{
	double tMass = 0;
	double xCont = 0;
	double yCont = 0;

	for (size_t i = 0; i < planets.size(); i++)
	{

		Planet p = planets[i];
		if (p.getmass() != -1)
		{
			double pMass = p.getmass();

			tMass += pMass;
			xCont += pMass*p.getx();
			yCont += pMass*p.gety();
		}

	}

	return sf::Vector2f(xCont / tMass, yCont / tMass);
}

struct DistanceCalculationResult
{
	double dx{ 0.0 };
	double dy{ 0.0 };
	double dist{ 0.0 };
	double rad_dist{ 0.0 };
};

DistanceCalculationResult calculateDistance(const Planet & planet, const Planet & other_planet)
{
	DistanceCalculationResult result;
	result.dx = other_planet.getx() - planet.getx();
	result.dy = other_planet.gety() - planet.gety();
	result.dist = std::hypot(result.dx, result.dy);
	result.rad_dist = planet.getRad() + other_planet.getRad();
	return result;
}

struct Acceleration2D
{
	double x{ 0.0 };
	double y{ 0.0 };
};

double accumulate_acceleration(const DistanceCalculationResult & distance_info, 
						Acceleration2D & acceleration,
						const Planet & other_planet)
{
	const auto A = G * other_planet.getmass() / std::max(distance_info.dist * distance_info.dist, 0.01);
	const auto angle = atan2(distance_info.dy, distance_info.dx);

	acceleration.x += cos(angle) * A;
	acceleration.y += sin(angle) * A;

	return A;
}

bool isIgnoringOtherPlanet(const Planet & thisPlanet, const Planet & otherPlanet)
{
	return (otherPlanet.isMarkedForRemoval() ||
		thisPlanet.getId() == otherPlanet.getId() ||
		thisPlanet.isIgnoring(otherPlanet.getId()));
}

void Space::update()
{
	curr_time += timeStep;

	//SETUP & OTHER
	totalMass = 0;
	if (planets.size() > 0) iteration += 1;
	if (ship.getLandedState())
	{
		if (findPlanet(ship.getPlanetID()).getmass() == -1)
			ship.setLandedstate(false);
	}

	//SHIPCHECK
	for (size_t i = 0; i < planets.size(); i++)
	{
		if (ship.isExist() && !ship.pullofGravity(planets[i], ship, timeStep))
			addExplosion(ship.getpos(), 10, sf::Vector2f(0, 0), 10);
	}
	
	particles->update(planets, bound, timeStep, curr_time);

	for (int i = 0; i < planets.size(); i++)
	{
		Planet * thisPlanet = &planets[i];

		if (thisPlanet->isMarkedForRemoval())
			continue;

		if (thisPlanet->disintegrationGraceTimeOver(curr_time))
			thisPlanet->clearIgnores();

		thisPlanet->resetAttractorMeasure();

		Acceleration2D acc_sum_1;
		for (auto & otherPlanet : planets)
		{
			if (isIgnoringOtherPlanet(*thisPlanet, otherPlanet))
				continue;

			DistanceCalculationResult distance = calculateDistance(*thisPlanet, otherPlanet);
			const auto acceleration_magnitude = accumulate_acceleration(distance, acc_sum_1, otherPlanet);

			if (thisPlanet->canDisintegrate(curr_time) &&
				distance.dist < ROCHE_LIMIT_DIST_MULTIPLIER * distance.rad_dist &&
				thisPlanet->getmass() / otherPlanet.getmass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
			{
				const auto rad = thisPlanet->getRad();

				disintegratePlanet(*thisPlanet);
				thisPlanet = &planets[i]; /* Risking invalidation due to added planets */

				const auto& particles_by_rad = [rad]()
				{
					return static_cast<size_t>(50 * rad);
				};

				const auto n_dust_particles = std::clamp(MAX_N_DUST_PARTICLES - particles->size(),
															static_cast<size_t>(0), particles_by_rad());
				for (size_t i = 0; i < n_dust_particles; i++)
				{
					const auto scatter_pos = sf::Vector2f( thisPlanet->getx() , thisPlanet->gety()) + random_vector(rad);
					const auto scatter_vel = sf::Vector2f(thisPlanet->getxv(), thisPlanet->getyv()) + CREATEDUSTSPEEDMULT * random_vector(20.0);
					const auto lifespan = uniform_random(DUST_LIFESPAN_MIN, DUST_LIFESPAN_MAX);
					addSmoke(scatter_pos, scatter_vel,2, lifespan);
				}
				break;
			}

			if (distance.dist < distance.rad_dist)
			{
				if (thisPlanet->getmass() <= otherPlanet.getmass())
				{
					thisPlanet->becomeAbsorbedBy(otherPlanet);

					addExplosion(sf::Vector2f(thisPlanet->getx(), thisPlanet->gety()), 
								2 * thisPlanet->getRad(), 
								sf::Vector2f(thisPlanet->getxv() * 0.5, thisPlanet->getyv() * 0.5), 
								sqrt(thisPlanet->getmass()) / 2);

					break;
				}
			}

			if (otherPlanet.emitsHeat())
			{
				const auto heat = tempConstTwo * thisPlanet->getRad() * thisPlanet->getRad() * otherPlanet.giveThermalEnergy(timeStep) / distance.dist;
				thisPlanet->absorbHeat(heat, timeStep);
			}

			if (acceleration_magnitude > thisPlanet->getStrongestAttractorStrength())
			{
				thisPlanet->setStrongestAttractorStrength(acceleration_magnitude);
				thisPlanet->setStrongestAttractorIdRef(otherPlanet.getId());
			}
		}

		if (thisPlanet->isMarkedForRemoval())
			continue;
		
		/* Integrate (first part) (Leapfrog) */
		/* https://en.wikipedia.org/wiki/Leapfrog_integration */

		thisPlanet->setx(thisPlanet->getx() + thisPlanet->getxv() * timeStep + 0.5 * acc_sum_1.x * timeStep * timeStep);
		thisPlanet->sety(thisPlanet->gety() + thisPlanet->getyv() * timeStep + 0.5 * acc_sum_1.y * timeStep * timeStep);

		Acceleration2D acc_sum_2;
		for (auto& otherPlanet : planets)
		{
			if (isIgnoringOtherPlanet(*thisPlanet, otherPlanet))
				continue;

			DistanceCalculationResult distance = calculateDistance(*thisPlanet, otherPlanet);
			accumulate_acceleration(distance, acc_sum_2, otherPlanet);
		}

		/* Integrate (second part) (Leapfrog) */
		thisPlanet->setxv(thisPlanet->getxv() + 0.5 * (acc_sum_1.x + acc_sum_2.x) * timeStep);
		thisPlanet->setyv(thisPlanet->getyv() + 0.5 * (acc_sum_1.y + acc_sum_2.y) * timeStep);
	}

	std::erase_if(planets, [](const Planet & planet) { return planet.isMarkedForRemoval(); });

	//OTHER
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < planets.size(); i++)
	{
		//TEMPERATURE
		planets[i].coolDown(timeStep);
		planets[i].setColor();
		planets[i].updateTemp();

		//ATMOSPHERE
		planets[i].updateAtmosphere(timeStep);

		//LIFE
		planets[i].updateLife(timeStep);
	}

	//COLONIZATION
	for (size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getLife().willExp())
		{
			int index = findBestPlanet(i);
			if (index != -1) planets[index].colonize(planets[i].getLife().getId(), planets[i].getLife().getCol(), planets[i].getLife().getDesc());
		}
	}
	
	//CHECKING IF ANYTHING IS OUTSIDE BOUNDS
	if (bound.isActive())
	{
		//PLANETS
		for (size_t i = 0; i < planets.size(); i++)
		{
			if (bound.isOutside(sf::Vector2f(planets[i].getx(), planets[i].gety())))
				removePlanet(planets[i].getId());
		}
		
		//SHIP
		if (bound.isOutside(ship.getpos())) ship.destroy();
	}

	//PLACING BOUND AROUND MASS CENTRE IF AUTOBOUND IS ENABLED
	if (autoBound->isChecked() && iteration%BOUND_AUTO_UPDATE_RATE == 0)
	{
		bound.setPos(centerOfMassAll());
		bound.setActiveState(true);
		bound.setRad(START_RADIUS);
	}

	//CHECKING TOTAL MASS
	for (size_t i = 0; i < planets.size(); i++)
		totalMass += planets[i].getmass();
}

void Space::hotkeys(sf::Window & window, sf::View & view)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma) && timeStepSlider->getValue() > timeStepSlider->getMinimum())
		timeStepSlider->setValue(std::max(timeStepSlider->getValue() - 1, timeStepSlider->getMinimum()));

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period) && timeStepSlider->getValue() < timeStepSlider->getMaximum())
		timeStepSlider->setValue(std::min(timeStepSlider->getValue() + 1, timeStepSlider->getMaximum()));

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		exit(0);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		full_reset(view);

	setFunctionGUIFromHotkeys(functions);
}

void Space::randomPlanets(int totmass,int antall, double radius, sf::Vector2f pos)
{
	double speedmultRandom = 0.00000085 * uniform_random(120*totmass, 150*totmass);
	double angle = 0;
	double delta_angle = 2*PI / antall;
	double centermass = uniform_random(totmass / 3 , totmass / 2);
	totmass -= centermass;

	Planet centerP(centermass, pos.x, pos.y);

	addPlanet(std::move(centerP));

	for (int i = 0; i < antall; i++, angle += delta_angle)
	{
		double mass = uniform_random(0.6*totmass /antall, 1.4*totmass /antall);

		double radius2 = uniform_random(2*centerP.getRad() , radius);
		double randomelement1 = uniform_random(-speedmultRandom*0.5, speedmultRandom*0.5);
		double randomelement2 = uniform_random(-speedmultRandom*0.5, speedmultRandom*0.5);

		double x = cos(angle) * radius2;
		double y = sin(angle) * radius2;
		double xv = (speedmultRandom*cos(angle + 1.507) + randomelement1);
		double yv = (speedmultRandom*sin(angle + 1.507) + randomelement2);

		addPlanet(Planet(mass,pos.x+x,pos.y+y, xv, yv));
	}

}

void Space::removePlanet(const int id)
{
	if (Planet *planet = findPlanetPtr(id))
		planet->markForRemoval();
}

void Space::removeTrail(int ind)
{
	if (ind >= (int) trail.size() || ind < 0) return;

	auto it = trail.begin() + ind;
	*it = std::move(trail.back());
	trail.pop_back();

}

void Space::removeExplosion(int ind)
{
	if (ind >= (int) explosions.size() || ind < 0) return;

	auto it = explosions.begin() + ind;
	*it = std::move(explosions.back());
	explosions.pop_back();
}

void Space::removeSmoke(int ind)
{
	particles->clear();
}

void Space::full_reset(sf::View& view)
{
	planets.clear();
	explosions.clear();
	particles->clear();
	trail.clear();
	bound = Bound();
	
	view = sf::View();
	view.setSize(sf::Vector2f(xsize, ysize));
	view.setCenter(0, 0);
	ship.reset(sf::Vector2f(0, 0));
	iteration = 0;
	curr_time = 0.0;
	click_and_drag_handler.reset();
}

void Space::disintegratePlanet(Planet planet)
{
	const auto match = std::find_if(planets.begin(), planets.end(),
		[&planet](const auto& p) { return p.getId() == planet.getId(); });

	if (match == planets.end())
		return;

	if (match->getmass() < MINIMUMBREAKUPSIZE)
		return;

	const auto n_planets{ std::floor(planet.getmass() / MINIMUMBREAKUPSIZE) };
	const auto mass_per_planet = planet.getmass() / n_planets;

	std::vector<int> generated_ids;
	for (int n = 0; n < n_planets; n++)
	{
		Planet p(mass_per_planet, planet.getx(), planet.gety(), 
			planet.getxv(), planet.getyv());

		const auto offset_dist = uniform_random(0.0, planet.getRad() - p.getRad());
		const auto angle_offset = uniform_random(0.0, 2.0 * PI);

		p.setx(p.getx() + cos(angle_offset) * offset_dist);
		p.sety(p.gety() + sin(angle_offset) * offset_dist);

		generated_ids.push_back(addPlanet(std::move(p)));
	}

	for (const auto & id : generated_ids)
	{
		if (Planet* p = findPlanetPtr(id))
		{
			for (const auto & id2 : generated_ids)
				p->registerIgnoredId(id2);
			p->setDisintegrationGraceTime(uniform_random(MIN_DT_DISINTEGRATE_GRACE_PERIOD, MAX_DT_DISINTEGRATE_GRACE_PERIOD), curr_time);
		}
	}

	removePlanet(planet.getId());
}

void Space::explodePlanet(Planet planet)
{
	const auto match = std::find_if(planets.begin(), planets.end(), 
		[&planet](const auto& p) { return p.getId() == planet.getId(); });
	if (match == planets.end())
		return;

	if (match->getmass() < MINIMUMBREAKUPSIZE)
		return;

	double origMass = planet.getmass();
	double antall = ceil(origMass / MINIMUMBREAKUPSIZE);
	double mass = origMass / antall;
	double rad = (Planet(mass)).getRad();
	double x = planet.getx();
	double y = planet.gety();
	double xv = planet.getxv();
	double yv = planet.getyv();

	removePlanet(planet.getId());
	addExplosion(sf::Vector2f(x, y), 90, sf::Vector2f(xv, yv), 10);

	double kanter = 5;
	int added = 0;

	for (size_t i = 0; i < antall; i += kanter)
	{
		double angle = uniform_random(0.0, 2.0 * PI);
		double deltaVinkel = 2 * PI / kanter;
		double dist = 2 * rad * (1 / sin(deltaVinkel) - 1) + EXPLODE_PLANET_DISTCONST + 2 * rad + (2 * kanter - 10) * rad;
		double escVel = G * sqrt(origMass) * EXPLODE_PLANET_SPEEDMULT_OTHER / cbrt(dist + 0.1);
		kanter++;

		for (size_t q = 0; q < kanter; q++)
		{
			Planet Q(mass, x + dist * cos(angle), y + dist * sin(angle), xv + escVel * cos(angle), yv + escVel * sin(angle));
			Q.setTemp(1500);
			addPlanet(std::move(Q));
			angle += deltaVinkel;
			added++;
			if (added >= antall) return;
		}
	}
}

void Space::giveId(Planet &p)
{
	p.mark(nesteid);
	nesteid += 1;
}

Planet Space::findPlanet(double id)
{
	for(size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getId() == id)
		{
			return planets[i];
		}
	}
	return Planet(-1);
}

Planet* Space::findPlanetPtr(double id)
{
	for (auto & planet : planets)
	{
		if (planet.getId() == id)
			return &planet;
	}
	return nullptr;
}

int Space::findBestPlanet(int q)
{
	int ind = -1;
	double highest = -1;

	for(size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getType() == ROCKY || planets[i].getType() == TERRESTIAL)
		{
			int nr = planets[i].getLife().getTypeEnum();
			if (nr == 6 || nr == 7 || nr == 5 || nr == 4)
			{
			}
			else if (planets[i].getSupportedBiomass() > highest && (int) i != q)
			{
				highest = planets[i].getSupportedBiomass();
				ind = i;
			}
		}
	}

	return ind;
}

void Space::addTrail(sf::Vector2f p, int l)
{
	trail.push_back(Trail(p, l));
}

void Space::giveRings(Planet p, int inner, int outer)
{
	int antall = 0.05*outer*outer;
	double angle = 0;
	double delta_angle = 2*PI / antall;

	for (int i = 0; i < antall; i++)
	{
		double rad = ((double) uniform_random(inner*1000, outer*1000))/1000;
		double speed = sqrt(G*p.getmass() / rad);

		const auto pos = sf::Vector2f(p.getx() + cos(angle) * rad, p.gety() + sin(angle) * rad);
		const auto vel = sf::Vector2f(speed * cos(angle + PI / 2.0) + p.getxv(), speed * sin(angle + PI / 2.0));
		
		particles->add_particle(pos, vel, 1, 20000);

		angle += delta_angle;
	}
}

std::string Space::calcTemperature(double q, int e)
{
	if (e == 1)
	{
		return convertDoubleToString((int) q) + "K";
	}
	else if (e == 2)
	{
		return convertDoubleToString((int)(q - 273.15)) + "°C";
	}
	else if (e == 3)
	{
		return convertDoubleToString((int)((q - 273.15)* 1.8000 + 32.00)) + "°F";
	}
	return "-";
}

void Space::updateSpaceship()
{
	int mode = ship.move(timeStep);
	if (mode == 1)
	{
		sf::Vector2f v;
		double angl = ((double)uniform_random(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;

		sf::Vector2f p;
		p.x = ship.getpos().x - 2 * cos(angl);
		p.y = ship.getpos().y - 2 * sin(angl);

		v.x = ship.getvel().x - cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y - sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, v, uniform_random(1.3, 1.5), 400);

		angl = ((double)uniform_random(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;
		v.x = ship.getvel().x - cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y - sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, v, uniform_random(1.3, 1.5), 200);
	}
	else if (mode == -1)
	{
		sf::Vector2f v;
		double angl = ((double)uniform_random(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;

		sf::Vector2f p;
		p.x = ship.getpos().x - 2 * cos(angl);
		p.y = ship.getpos().y - 2 * sin(angl);

		v.x = ship.getvel().x + cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y + sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, v, uniform_random(1.3, 1.5), 400);

		angl = ((double)uniform_random(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;
		v.x = ship.getvel().x + cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y + sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, v, uniform_random(1.3, 1.5), 200);
	}
}

void Space::setInfo()
{
	//Mass
	size = massSlider->getValue();

	//Timestep
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))timeStepSlider->setValue(0);
	timeStep = timeStepSlider->getValue();

	/*
	//Current planet sliders
	if (findPlanet(fokusId).getmass() == -1)
	{
		currPlanetInfo->setVisible(false);
		massExistingObjectSlider->setVisible(false);
	}
	else
	{
		//Gathering current object mass slider info
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (Planet* ptr = findPlanetPtr(fokusId))
			{
				auto& planet = *ptr;

				planet.setMass(massExistingObjectSlider->getValue());
				planet.updateRadiAndType();
			}
			
		}
		else massExistingObjectSlider->setValue(findPlanet(fokusId).getmass());

		std::string typeplanet = "Black hole";
		if (findPlanet(fokusId).getmass() < ROCKYLIMIT)
		{
			typeplanet = "Rocky";
		}
		else if (findPlanet(fokusId).getmass() < TERRESTIALLIMIT)
		{
			typeplanet = "Terrestial";
		}
		else if (findPlanet(fokusId).getmass() < GASGIANTLIMIT)
		{
			typeplanet = "Gas giant";
		}
		else if (findPlanet(fokusId).getmass() < SMALLSTARLIMIT)
		{
			typeplanet = "Small star";
		}
		else if (findPlanet(fokusId).getmass() < STARLIMIT)
		{
			typeplanet = "Star";
		}
		else if (findPlanet(fokusId).getmass() < BIGSTARLIMIT)
		{
			typeplanet = "Big star";
		}

		currPlanetInfo->setText("FOCUSED OBJECT \nMass:   " + convertDoubleToString((int) findPlanet(fokusId).getmass()) + "\nType:      " + typeplanet);

		currPlanetInfo->setVisible(true);
		massExistingObjectSlider->setVisible(true);
	}
	
	//Temp unit
	if (temperatureUnitSelector->getSelectedIndex() < 2) tempEnhet = temperatureUnitSelector->getSelectedIndex() + 1;
	else tempEnhet = 3;

	//Hiding new planet
	if (functions->getSelectedItemIndex() != 0 && functions->getSelectedItemIndex() != 1 && functions->getSelectedItemIndex() != 2)
	{
		massSlider->setVisible(false);
		newPlanetInfo->setVisible(false);
	}
	else
	{
		massSlider->setVisible(true);
		newPlanetInfo->setVisible(true);

		std::string typeplanet = "Black hole";
		if (size < ROCKYLIMIT)
		{
			typeplanet = "Rocky";
		}
		else if (size < TERRESTIALLIMIT)
		{
			typeplanet = "Terrestial";
		}
		else if (size < GASGIANTLIMIT)
		{
			typeplanet = "Gas giant";
		}
		else if (size < SMALLSTARLIMIT)
		{
			typeplanet = "Small star";
		}
		else if (size < STARLIMIT)
		{
			typeplanet = "Star";
		}
		else if (size < BIGSTARLIMIT)
		{
			typeplanet = "Big star";
		}

		newPlanetInfo->setText("NEW OBJECT\nMass:   " + convertDoubleToString(size) + "\nType:      " + typeplanet);
		}

	//Displaying sim-info
	simInfo->setText("Framerate: " + convertDoubleToString(fps) + "\nFrame: " + convertDoubleToString(iteration) + "\nTimestep (,/.): " + convertDoubleToString(timeStep) + "\nTotal mass: " + convertDoubleToString(totalMass) + "\nObjects: " + convertDoubleToString(planets.size()) + "\nParticles: " + convertDoubleToString(particles->size()));
	*/
}

void Space::initSetup()
{
	simInfo->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	simInfo->setSize(160, 110);
	simInfo->setPosition(5, 5);
	simInfo->setTextSize(14);

	functions->setItemHeight(14);
	functions->getScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
	functions->setTextSize(13);
	functions->setPosition(5, simInfo->getFullSize().y + 2*UI_SEPERATION_DISTANCE);

	fillFunctionGUIDropdown(functions);
	
	functions->setSize(160, functions->getItemCount()*functions->getItemHeight()+5);

	autoBound->setPosition(170, 105 + UI_SEPERATION_DISTANCE + functions->getItemCount()*functions->getItemHeight());
	autoBound->setSize(14, 14);
	autoBound->setChecked(true);

	newPlanetInfo->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	newPlanetInfo->setSize(160, 45);
	newPlanetInfo->setPosition(5, 5 + simInfo->getFullSize().y + functions->getItemCount()*functions->getItemHeight() + UI_SEPERATION_DISTANCE * 4);
	newPlanetInfo->setTextSize(14);

	massSlider->setPosition(5, 57 + simInfo->getFullSize().y + functions->getItemCount()*functions->getItemHeight() + UI_SEPERATION_DISTANCE * 5);
	massSlider->setSize(300, 10);
	massSlider->setValue(1);
	massSlider->setMinimum(MASS_SLIDER_MIN_VALUE);
	massSlider->setMaximum(MASS_SLIDER_MAX_VALUE);

	timeStepSlider->setPosition(165, 40);
	timeStepSlider->setSize(160, 5);
	timeStepSlider->setValue(TIMESTEP_VALUE_START);
	timeStepSlider->setMinimum(0);
	timeStepSlider->setMaximum(MAX_TIMESTEP);

	temperatureUnitSelector->add("K");
	temperatureUnitSelector->add("°C");
	temperatureUnitSelector->add("°F");
	temperatureUnitSelector->select("K");
	temperatureUnitSelector->setTextSize(10);
	temperatureUnitSelector->setTabHeight(12);
	temperatureUnitSelector->setPosition(165, 50);

	currPlanetInfo->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	currPlanetInfo->setSize(145, 45);
	currPlanetInfo->setPosition(5, 75 + simInfo->getFullSize().y + functions->getItemCount()*functions->getItemHeight() + UI_SEPERATION_DISTANCE * 6);
	currPlanetInfo->setTextSize(14);

	massExistingObjectSlider->setPosition(5, 125 + simInfo->getFullSize().y + functions->getItemCount()*functions->getItemHeight() + UI_SEPERATION_DISTANCE * 7);
	massExistingObjectSlider->setSize(200, 5);
	massExistingObjectSlider->setValue(1);
	massExistingObjectSlider->setMinimum(MASS_SLIDER_MIN_VALUE);
	massExistingObjectSlider->setMaximum(MASS_SLIDER_MAX_VALUE);
}

template<typename T>
T generate_uniform(T min, T max) {
	std::random_device seeder;
	std::default_random_engine generator(seeder());

	if constexpr (std::is_integral_v<T>) {
		std::uniform_int_distribution<T> uniform(min, max);
		return uniform(generator);
	}
	else if constexpr (std::is_floating_point_v<T>) {
		std::uniform_real_distribution<T> uniform(min, max);
		return uniform(generator);
	}
}


int Space::uniform_random(int min, int max)
{
	return generate_uniform<int>(min, max);
}

double Space::uniform_random(double min, double max)
{
	return generate_uniform<double>(min, max);
}

sf::Vector2f Space::random_vector(double magn)
{
	const auto angle = generate_uniform<double>(0.0, 2.0*PI);
	const auto magnitude = generate_uniform<double>(0.0, magn);
	return sf::Vector2f
	{
		static_cast<float>(cos(angle) * magnitude),
		static_cast<float>(sin(angle) * magnitude)
	};
}

std::string Space::convertDoubleToString(double number)
{
	std::string Result;

	std::stringstream convert;

	convert << std::setprecision(5) << number;

	return convert.str();
}

double Space::convertStringToDouble(std::string string)
{
	double result;

	std::stringstream convert;

	convert << string;

	convert >> result;

	return result;
}

void Space::drawPlanets(sf::RenderWindow &window)
{
	//DRAWING PLANETS																										
	for(size_t i = 0; i < planets.size(); i++)
	{
		planets[i].draw(window);
	}
}

void Space::drawEffects(sf::RenderWindow &window)
{

	//EXPLOSIONS
	int inc = 1;
	if (timeStep == 0) inc = 0;

	for(size_t i = 0; i < explosions.size(); i++)
	{
		explosions[i].move(timeStep);

		if (explosions[i].getAge(inc) < explosions[i].levetidmax()) explosions[i].render(window);
		else
		{
			removeExplosion(i);
		}
	}
	
	//DUST
	particles->render_all(window);

	//TRAILS
	for(size_t i = 0; i < trail.size(); i++)
	{
		trail[i].move(timeStep);
		if (trail[i].getAge(0) < trail[i].levetidmax() && !trail[i].killMe()) trail[i].render(window);
		else
		{
			removeTrail(i);
		}
	}

}

void Space::drawLightEffects(sf::RenderWindow& window)
{
	for(size_t i = 0; i < planets.size(); i++)
	{
		Planet p = planets[i];
		
		if (p.getmass() >= GASGIANTLIMIT && p.getmass() < BIGSTARLIMIT)
		{
			sf::Color col = p.getStarCol();

			//LONG RANGE LIGHT
			col.a = EXPLOSION_LIGHT_START_STRENGTH;
			sf::VertexArray vertexArr(sf::TrianglesFan);
			vertexArr.append(sf::Vertex(sf::Vector2f(p.getx(), p.gety()), col));
			col.a = 0;
			double deltaAng = 2*PI / ((double)LIGHT_NUMBER_OF_VERTECES);
			double ang = 0;
			double rad = LIGHT_STRENGTH_MULTIPLIER * sqrt(p.fusionEnergy());
			for(size_t nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; nr++)
			{
				sf::Vector2f pos(p.getx() + cos(ang) * rad, p.gety() + sin(ang) * rad);
				vertexArr.append(sf::Vertex(pos, col));
				ang += deltaAng;
			}
			vertexArr.append(sf::Vertex(sf::Vector2f(p.getx() + rad, p.gety()), col));
			window.draw(vertexArr);

			//SHORT RANGE LIGHT
			col.a = LIGHT_START_STRENGTH * SHORT_LIGHT_STRENGTH_MULTIPLIER;
			sf::VertexArray vertexArr2(sf::TrianglesFan);
			vertexArr2.append(sf::Vertex(sf::Vector2f(p.getx(), p.gety()), col));
			col.a = LIGHT_END_STRENGTH;
			ang = 0;
			rad = SHORT_LIGHT_RANGE_MULTIPLIER * p.getRad();
			for(size_t nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; nr++)
			{
				sf::Vector2f pos(p.getx() + cos(ang) * rad, p.gety() + sin(ang) * rad);
				vertexArr2.append(sf::Vertex(pos, col));
				ang += deltaAng;
			}
			vertexArr2.append(sf::Vertex(sf::Vector2f(p.getx() + rad, p.gety()), col));
			window.draw(vertexArr2);
		}
	}
}

double Space::thermalEnergyAtPosition(sf::Vector2f pos)
{
	double tEnergyFromOutside = 0;

	for(size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getmass() < BIGSTARLIMIT && planets[i].getmass() >= GASGIANTLIMIT)
		{
			tEnergyFromOutside += planets[i].giveThermalEnergy(1)/ sqrt((planets[i].getx() - pos.x)*(planets[i].getx() - pos.x) + (planets[i].gety() - pos.y) * (planets[i].gety() - pos.y));
		}
	}

	return tEnergyFromOutside;
}
