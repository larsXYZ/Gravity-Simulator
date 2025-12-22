#include "space.h"

#include "particles/particle_container.h"
#include "user_functions.h"

Space::Space()
	: particles(std::make_unique<DecimatedLegacyParticleContainer>())
{}

int Space::addPlanet(Planet&& p)
{
	giveId(p);
	const auto id = p.getId();
	
	p.setColor();

	pending_planets.push_back(std::move(p));
	return id;
}

void Space::flushPlanets()
{
	if (pending_planets.empty())
		return;

	for (auto& p : pending_planets)
		planets.push_back(std::move(p));

	pending_planets.clear();
}

void Space::addExplosion(sf::Vector2f p, double s, sf::Vector2f v, int l)
{
	explosions.push_back(Explosion(p, s, 0, v, l));
}

void Space::addSmoke(sf::Vector2f p,  sf::Vector2f v, double s, double lifespan, double initial_temp)
{
	particles->add_particle(p, v, s, curr_time+lifespan, initial_temp);
}

sf::Vector3f Space::centerOfMass(const std::vector<int> & object_ids)
{
	auto tMass = 0.0;
	auto xCont = 0.0;
	auto yCont = 0.0;
	for (const auto id : object_ids)
	{
		if (Planet* planet = findPlanetPtr(id))
		{
			tMass += planet->getMass();
			xCont += planet->getMass() * planet->getPosition().x;
			yCont += planet->getMass() * planet->getPosition().y;
		}
	}
	return sf::Vector3f(xCont / tMass, yCont / tMass, tMass);
}

sf::Vector2f Space::centerOfMassVelocity(const std::vector<int> & object_ids)
{
	auto tMass = 0.0;
	auto xCont = 0.0;
	auto yCont = 0.0;
	for (const auto id : object_ids)
	{
		if (Planet * planet = findPlanetPtr(id))
		{
			tMass += planet->getMass();
			xCont += planet->getMass()*planet->getVelocity().x;
			yCont += planet->getMass()*planet->getVelocity().y;
		}
	}
	return sf::Vector2f(xCont / tMass, yCont / tMass);
}

int Space::get_iteration() const
{
	return iteration;
}

sf::Vector2f Space::centerOfMassAll()
{
	double tMass = 0;
	double xCont = 0;
	double yCont = 0;

	for (size_t i = 0; i < planets.size(); i++)
	{

		Planet p = planets[i];
		if (p.getMass() != -1)
		{
			double pMass = p.getMass();

			tMass += pMass;
			xCont += pMass*p.getPosition().x;
			yCont += pMass*p.getPosition().y;
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
	result.dx = other_planet.getPosition().x - planet.getPosition().x;
	result.dy = other_planet.getPosition().y - planet.getPosition().y;
	result.dist = std::hypot(result.dx, result.dy);
	result.rad_dist = planet.getRadius() + other_planet.getRadius();
	return result;
}

struct Acceleration2D
{
	float x{ 0.0 };
	float y{ 0.0 };
};

double accumulate_acceleration(const DistanceCalculationResult & distance_info, 
						Acceleration2D & acceleration,
						const Planet & other_planet)
{
	const auto A = G * other_planet.getMass() / std::max(distance_info.dist * distance_info.dist, 0.01);
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
	flushPlanets();
	curr_time += timestep;

	//SETUP & OTHER
	total_mass = 0;

	if (planets.size() > 0)
		iteration += 1;

	update_spaceship();

	particles->update(planets, bound, timestep, curr_time, gravity_enabled);

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
				thisPlanet->getMass() / otherPlanet.getMass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
			{
				const auto rad = thisPlanet->getRadius();

				disintegratePlanet(*thisPlanet);
				thisPlanet = &planets[i]; /* Risking invalidation due to added planets */

				break;
			}

			if (distance.dist < distance.rad_dist)
			{
				if (thisPlanet->getMass() <= otherPlanet.getMass())
				{
					const auto collision_pos = sf::Vector2f(thisPlanet->getPosition().x, thisPlanet->getPosition().y);
					const auto collision_vel = sf::Vector2f(thisPlanet->getVelocity().x, thisPlanet->getVelocity().y);
					const auto collision_radius = thisPlanet->getRadius();
					const auto collision_temp = thisPlanet->getTemp();

					thisPlanet->becomeAbsorbedBy(otherPlanet);

					addExplosion(collision_pos, 
								4 * collision_radius, 
								sf::Vector2f(collision_vel.x * 0.5, collision_vel.y * 0.5), 
								sqrt(otherPlanet.getMass()));

					// Particle generation
					// Non-linear relationship: fewer particles for small bodies
					const size_t n_particles_to_add = static_cast<size_t>(20 * collision_radius * std::sqrt(collision_radius));
					const auto n_dust_particles = std::clamp(MAX_N_DUST_PARTICLES - particles->size(),
						static_cast<size_t>(0), n_particles_to_add);

					for (size_t k = 0; k < n_dust_particles; k++)
					{
						const auto scatter_pos = collision_pos + random_vector(collision_radius);
						sf::Vector2f rnd_vel = random_vector(30.0);
						rnd_vel *= static_cast<float>(CREATEDUSTSPEEDMULT);
						const auto scatter_vel = collision_vel + rnd_vel;
						const auto lifespan = uniform_random(DUST_LIFESPAN_MIN, DUST_LIFESPAN_MAX);
						
						double p_temp = collision_temp;
						if (uniform_random(0, 100) < 15) p_temp *= 2.5; // 15% of particles are much hotter/glowing

						addSmoke(scatter_pos, scatter_vel, 2, lifespan, p_temp);
					}

					break;
				}
			}

			if (otherPlanet.emitsHeat())
			{
				const auto heat = tempConstTwo * thisPlanet->getRadius() * thisPlanet->getRadius() * otherPlanet.giveThermalEnergy(timestep) / std::max(distance.dist, 1.0);
				thisPlanet->absorbHeat(heat, timestep);
			}

			if (acceleration_magnitude > thisPlanet->getStrongestAttractorStrength())
			{
				thisPlanet->setStrongestAttractorStrength(acceleration_magnitude);
				thisPlanet->setStrongestAttractorIdRef(otherPlanet.getId());
			}
		}

		if (thisPlanet->isMarkedForRemoval())
			continue;
		
		if (!gravity_enabled)
		{
			acc_sum_1.x = 0.f;
			acc_sum_1.y = 0.f;
		}

		/* Integrate (first part) (Leapfrog) */
		/* https://en.wikipedia.org/wiki/Leapfrog_integration */

		thisPlanet->setPosition({ thisPlanet->getPosition().x + thisPlanet->getVelocity().x * timestep + 0.5f * acc_sum_1.x * timestep * timestep,
									thisPlanet->getPosition().y + thisPlanet->getVelocity().y * timestep + 0.5f * acc_sum_1.y * timestep * timestep });

		Acceleration2D acc_sum_2;
		for (auto& otherPlanet : planets)
		{
			if (isIgnoringOtherPlanet(*thisPlanet, otherPlanet))
				continue;

			DistanceCalculationResult distance = calculateDistance(*thisPlanet, otherPlanet);
			accumulate_acceleration(distance, acc_sum_2, otherPlanet);
		}

		if (!gravity_enabled)
		{
			acc_sum_2.x = 0.f;
			acc_sum_2.y = 0.f;
		}

		/* Integrate (second part) (Leapfrog) */
		thisPlanet->setVelocity({ thisPlanet->getVelocity().x + 0.5f * (acc_sum_1.x + acc_sum_2.x) * timestep,
									thisPlanet->getVelocity().y + 0.5f * (acc_sum_1.y + acc_sum_2.y) * timestep });
	}

	std::erase_if(planets, [](const Planet & planet) { return planet.isMarkedForRemoval(); });
	
	for (auto & planet : planets)
		planet.update_planet_sim(timestep);

	//COLONIZATION
	for (size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getLife().willExp())
		{
			int index = findBestPlanet(i);
			if (index != -1) 
				planets[index].colonize(planets[i].getLife().getId(), planets[i].getLife().getCol(), planets[i].getLife().getDesc(), planets[i].getLife().getCivName());
		}
	}
	
	//CHECKING IF ANYTHING IS OUTSIDE BOUNDS
	if (bound.isActive())
	{
		//PLANETS
		for (size_t i = 0; i < planets.size(); i++)
		{
			if (bound.isOutside(sf::Vector2f(planets[i].getPosition().x, planets[i].getPosition().y)))
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

	flushPlanets();
	total_mass = std::accumulate(planets.begin(), planets.end(), 0.0, [](auto v, const auto & p) {return v + p.getMass(); });
}

void Space::hotkeys(sf::Event event, sf::View & view, const sf::RenderWindow& window)
{
	if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::RControl)
	{
		ship.switchTool(*this);
		return;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
	{
		return;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma) && timeStepSlider->getValue() > timeStepSlider->getMinimum())
		timeStepSlider->setValue(std::max(timeStepSlider->getValue() - 1, timeStepSlider->getMinimum()));

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period) && timeStepSlider->getValue() < timeStepSlider->getMaximum())
		timeStepSlider->setValue(std::min(timeStepSlider->getValue() + 1, timeStepSlider->getMaximum()));

	setFunctionGUIFromHotkeys(functions);

	if (event.type == sf::Event::KeyReleased)
	{
		switch (event.key.code)
		{
		case sf::Keyboard::P:
			paused = !paused;
			break;
		case sf::Keyboard::Escape:
			exit(0);
		case sf::Keyboard::R:
			full_reset(view, window);
			break;
		case sf::Keyboard::F1:
			show_gui = !show_gui;
			break;
		}
	}
}

void Space::randomPlanets(int totmass,int antall, double radius, sf::Vector2f pos)
{
	double speedmultRandom = 0.00000085 * uniform_random(120*totmass, 150*totmass);
	double angle = 0;
	double delta_angle = 2*PI / antall;
	double centermass = uniform_random(totmass / 3 , totmass / 2);
	totmass -= centermass;

	Planet centerP(centermass, pos.x, pos.y);

	set_ambient_temperature(centerP);
	addPlanet(std::move(centerP));

	for (int i = 0; i < antall; i++, angle += delta_angle)
	{
		double mass = uniform_random(0.6*totmass /antall, 1.4*totmass /antall);

		radius = std::max(radius, 2 * centerP.getRadius());
		double radius2 = uniform_random(2*centerP.getRadius() , radius);
		double randomelement1 = uniform_random(-speedmultRandom*0.5, speedmultRandom*0.5);
		double randomelement2 = uniform_random(-speedmultRandom*0.5, speedmultRandom*0.5);

		double x = cos(angle) * radius2;
		double y = sin(angle) * radius2;
		double xv = (speedmultRandom*cos(angle + 1.507) + randomelement1);
		double yv = (speedmultRandom*sin(angle + 1.507) + randomelement2);

		auto new_planet = Planet(mass, pos.x + x, pos.y + y, xv, yv);
		set_ambient_temperature(new_planet);
		addPlanet(std::move(new_planet));
	}

}

bool Space::auto_bound_active() const
{
	return autoBound->isChecked();
}

void Space::set_ambient_temperature(Planet& planet)
{
	planet.setTemp((planet.fusionEnergy() / (planet.getRadius() * planet.getRadius() * SBconst))
		+ thermalEnergyAtPosition(sf::Vector2f(planet.getPosition().x, planet.getPosition().y)) * tempConstTwo / SBconst);
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

void Space::full_reset(sf::View& view, const sf::RenderWindow& window)
{
	planets.clear();
	pending_planets.clear();
	explosions.clear();
	particles->clear();
	trail.clear();
	bound = Bound();

	view.setCenter(0, 0);
	view.setSize(window.getSize().x, window.getSize().y);
	ship.destroy();
	iteration = 0;
	curr_time = 0.0;
	click_and_drag_handler.reset();
}

std::vector<int> Space::disintegratePlanet(Planet planet)
{
	const auto match = std::find_if(planets.begin(), planets.end(),
		[&planet](const auto& p) { return p.getId() == planet.getId(); });

	if (match == planets.end())
		return {};

	if (match->getMass() < MINIMUMBREAKUPSIZE)
		return {};

	const auto& particles_by_rad = [planet]()
	{
		// Non-linear relationship: fewer particles for small bodies
		// Base count scaled by radius^1.5 or similar
		double rad = planet.getRadius();
		return static_cast<size_t>(25 * rad * std::sqrt(rad));
	};

	const auto n_dust_particles = std::clamp(MAX_N_DUST_PARTICLES - particles->size(),
		static_cast<size_t>(0), particles_by_rad());
	for (size_t i = 0; i < n_dust_particles; i++)
	{
		const auto scatter_pos = sf::Vector2f(planet.getPosition().x, planet.getPosition().y) + random_vector(planet.getRadius());
		sf::Vector2f rnd_vel = random_vector(20.0);
		rnd_vel *= static_cast<float>(CREATEDUSTSPEEDMULT);
		const auto scatter_vel = sf::Vector2f(planet.getVelocity().x, planet.getVelocity().y) + rnd_vel;
		const auto lifespan = uniform_random(DUST_LIFESPAN_MIN, DUST_LIFESPAN_MAX);

		double p_temp = planet.getTemp();
		if (uniform_random(0, 100) < 15) p_temp *= 2.5; // 15% of particles are much hotter/glowing

		addSmoke(scatter_pos, scatter_vel, 2, lifespan, p_temp);
	}

	const auto n_planets{ std::floor(planet.getMass() / MINIMUMBREAKUPSIZE) };
	const auto mass_per_planet = planet.getMass() / n_planets;

	std::vector<int> generated_ids;
	for (int n = 0; n < n_planets; n++)
	{
		Planet p(mass_per_planet, planet.getPosition().x, planet.getPosition().y, 
			planet.getVelocity().x, planet.getVelocity().y);

		const float offset_dist = uniform_random(0.0, planet.getRadius() - p.getRadius());
		const float angle_offset = uniform_random(0.0, 2.0 * PI);

		p.setPosition({ p.getPosition().x + cos(angle_offset) * offset_dist, 
						p.getPosition().y + sin(angle_offset) * offset_dist });
		p.setTemp(uniform_random(3500.,6000.));

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

	return generated_ids;
}

void Space::explodePlanet(Planet planet)
{
	const float original_mass = planet.getMass();

	const sf::Vector2f original_position = { static_cast<float>(planet.getPosition().x),
										static_cast<float>(planet.getPosition().y) };

	const sf::Vector2f original_velocity = { static_cast<float>(planet.getVelocity().x),
										static_cast<float>(planet.getVelocity().y) };

	const auto lifetime = 0.5 * planet.getRadius();
	const auto size = 15.0 * planet.getRadius();

	addExplosion(original_position, size, original_velocity, lifetime);

	const auto fragment_ids = disintegratePlanet(planet);
	for (auto id : fragment_ids)
	{
		if (auto fragment = findPlanetPtr(id))
		{
			const sf::Vector2f fragment_pos = { static_cast<float>(fragment->getPosition().x),
													static_cast<float>(fragment->getPosition().y) };

			const sf::Vector2f to_fragment = (fragment_pos - original_position);

			const sf::Vector2f escape_speed = EXPLODE_PLANET_SPEEDMULT_OTHER * static_cast<float>(pow(original_mass, 0.3)) * to_fragment / std::max(std::hypot(to_fragment.x, to_fragment.y), 0.1f) *	
												static_cast<float>(uniform_random(0.85, 1.15));

			fragment->setVelocity(fragment->getVelocity() + escape_speed);
		}
	}
}

void Space::giveId(Planet &p)
{
	p.giveID(next_id);
	next_id += 1;
}

Planet Space::findPlanet(int id)
{
	for (auto& planet : planets)
	{
		if (planet.getId() == id)
			return planet;
	}
	for (auto& planet : pending_planets)
	{
		if (planet.getId() == id)
			return planet;
	}
	return Planet(-1);
}

Planet* Space::findPlanetPtr(int id)
{
	for (auto & planet : planets)
	{
		if (planet.getId() == id)
			return &planet;
	}
	for (auto& planet : pending_planets)
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

void Space::giveRings(const Planet & planet, int inner, int outer)
{
	const int particle_count = 0.05*outer*outer;
	const double delta_angle = 2.0 * PI / particle_count;
	double angle = 0;

	for (int i = 0; i < particle_count; i++)
	{
		const double rad = ((double) uniform_random(inner*1000, outer*1000))/1000;
		const double speed = sqrt(G*planet.getMass() / rad);

		const auto pos = sf::Vector2f(planet.getPosition().x + cos(angle) * rad, planet.getPosition().y + sin(angle) * rad);
		const auto vel = sf::Vector2f(speed * cos(angle + PI / 2.0) + planet.getVelocity().x, speed * sin(angle + PI / 2.0));
		
		particles->add_particle(pos, vel, 1, curr_time+2000000, 500.0);

		angle += delta_angle;
	}
}

std::string Space::temperature_info_string(double temperature_kelvin, TemperatureUnit unit)
{
	switch (unit)
	{
	case TemperatureUnit::KELVIN:
		return std::to_string(static_cast<int>(temperature_kelvin)) + "K";
	case TemperatureUnit::CELSIUS:
		return std::to_string(static_cast<int>(temperature_kelvin - 273.15)) + "°C";
	case TemperatureUnit::FAHRENHEIT:
		return std::to_string(static_cast<int>((temperature_kelvin - 273.15) * 1.8 + 32.0)) + "°F";
	default:
		return "-";
	}
}

void Space::update_spaceship()
{
	int mode = ship.move(timestep);
	if (mode == 1)
	{
        // ... (Smoke generation logic handled in move or here?) 
        // Wait, move() returns mode but doesn't spawn smoke. 
        // The original code spawned smoke here based on mode.
        // I need to keep the smoke spawning but update it to match new ship if needed.
        // For now, just removing the isLanded check.
        
		sf::Vector2f v;
		double angl = ((double)uniform_random(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;

		sf::Vector2f p;
		p.x = ship.getpos().x - 7 * cos(angl); // Adjusted for new ship length
		p.y = ship.getpos().y - 7 * sin(angl);

		v.x = ship.getvel().x - cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y - sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, v, uniform_random(1.3, 1.5), uniform_random(300.0, 500.0));

        // Extra smoke
		angl = ((double)uniform_random(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;
		v.x = ship.getvel().x - cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y - sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, v, uniform_random(1.3, 1.5), uniform_random(150.0, 250.0));
	}
	else if (mode == -1)
	{
        // Reverse thrust smoke
		sf::Vector2f v;
		double angl = ((double)uniform_random(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;

		sf::Vector2f p;
		p.x = ship.getpos().x + 5 * cos(angl);
		p.y = ship.getpos().y + 5 * sin(angl);

		v.x = ship.getvel().x + cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y + sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, v, uniform_random(1.3, 1.5), uniform_random(300.0, 500.0));
	}

	for (const auto & planet : planets)
	{
		if (ship.isExist() && !ship.pullofGravity(planet, ship, timestep, gravity_enabled))
			addExplosion(ship.getpos(), 10, sf::Vector2f(0, 0), 10);
	}

    // NEW FEATURES
    if (ship.isExist())
    {
        ship.handleInput(*this, timestep);

        ship.updateProjectiles(timestep, *this);
        ship.checkProjectileCollisions(*this, timestep);
        ship.updateGrapple(timestep, *this);
        ship.updateTug(*this, timestep);
        ship.checkShield(*this, timestep);
        ship.updateTrajectory(*this);
    }
}

void Space::updateInfoBox()
{
	simInfo->setText("Frame rate: " + std::to_string(fps) +
		"\nFrame: " + std::to_string(iteration) +
		"\nTime step: " + std::to_string(static_cast<int>(timestep)) +
		"\nTotal mass: " + std::to_string(static_cast<int>(total_mass)) +
		"\nObjects: " + std::to_string(planets.size()) +
		"\nParticles: " + std::to_string(particles->size()) +
		"\nZoom: " + std::to_string(1 / click_and_drag_handler.get_zoom()));

	if (ship.isExist())
	{
		toolInfo->setVisible(true);
		toolInfo->setText("Tool: " + ship.getToolName() + " (R-Ctrl to switch)");
	}
	else
	{
		toolInfo->setVisible(false);
	}
}

void Space::initSetup()
{
	simInfo->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	simInfo->setSize(180, 110);
	simInfo->setPosition(5, 5);
	simInfo->setTextSize(14);

	toolInfo->setPosition("2%", "95%");
	toolInfo->setTextSize(18);
	toolInfo->getRenderer()->setTextColor(sf::Color::White);
	toolInfo->setText("Tool: Energy cannon (R-Ctrl to switch)");

	functions->setItemHeight(14);
	functions->getScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
	functions->setTextSize(13);
	functions->setPosition(5, tgui::bindBottom(simInfo) + 2 * UI_SEPERATION_DISTANCE);

	fillFunctionGUIDropdown(functions);
	
	functions->setSize(180, functions->getItemCount()*functions->getItemHeight()+5);

	editObjectCheckBox->setText("");
	editObjectCheckBox->setPosition(190, 112 - 29 + functions->getItemCount() * functions->getItemHeight());
	editObjectCheckBox->setSize(14, 14);
	editObjectCheckBox->setVisible(true);

	newPlanetInfo->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	newPlanetInfo->setSize(180, 32);
	newPlanetInfo->setPosition(5, tgui::bindBottom(functions) + 2 * UI_SEPERATION_DISTANCE);
	newPlanetInfo->setTextSize(14);

	objectTypeSelector->addItem("Rocky");
	objectTypeSelector->addItem("Terrestial");
	objectTypeSelector->addItem("Gas Giant");
	objectTypeSelector->addItem("Small Star");
	objectTypeSelector->addItem("Star");
	objectTypeSelector->addItem("Big Star");
	objectTypeSelector->addItem("Black Hole");
	objectTypeSelector->setSelectedItem("Rocky");
	objectTypeSelector->setPosition(5, tgui::bindBottom(newPlanetInfo) + UI_SEPERATION_DISTANCE);
	objectTypeSelector->setSize(180, 20);
	objectTypeSelector->setVisible(true);

	objectTypeSelector->onItemSelect([this](const tgui::String& item) {
		if (item == "Rocky") {
			massSlider->setMinimum(1);
			massSlider->setMaximum(ROCKYLIMIT - 1);
		}
		else if (item == "Terrestial") {
			massSlider->setMinimum(ROCKYLIMIT);
			massSlider->setMaximum(TERRESTIALLIMIT - 1);
		}
		else if (item == "Gas Giant") {
			massSlider->setMinimum(TERRESTIALLIMIT);
			massSlider->setMaximum(GASGIANTLIMIT - 1);
		}
		else if (item == "Small Star") {
			massSlider->setMinimum(GASGIANTLIMIT);
			massSlider->setMaximum(SMALLSTARLIMIT - 1);
		}
		else if (item == "Star") {
			massSlider->setMinimum(SMALLSTARLIMIT);
			massSlider->setMaximum(STARLIMIT - 1);
		}
		else if (item == "Big Star") {
			massSlider->setMinimum(STARLIMIT);
			massSlider->setMaximum(BIGSTARLIMIT - 1);
		}
		else if (item == "Black Hole") {
			massSlider->setMinimum(BIGSTARLIMIT);
			massSlider->setMaximum(40000); // 10x Big Star
		}
		massSlider->setValue(massSlider->getMinimum());
	});

	autoBound->setPosition(190, 112 + UI_SEPERATION_DISTANCE + functions->getItemCount()*functions->getItemHeight());
	autoBound->setSize(14, 14);
	autoBound->setChecked(true);

	massSlider->setPosition(5, tgui::bindBottom(objectTypeSelector) + UI_SEPERATION_DISTANCE);
	massSlider->setSize(180, 10);
	massSlider->setValue(1);
	massSlider->setMinimum(1);
	massSlider->setMaximum(ROCKYLIMIT - 1);

	timeStepLabel->setText("Timestep");
	timeStepLabel->setTextSize(14);
	timeStepLabel->getRenderer()->setTextColor(sf::Color::White);
	timeStepLabel->setPosition("100% - 250", 5);

	timeStepSlider->setPosition(tgui::bindRight(timeStepLabel) + 10, 8);
	timeStepSlider->setSize(160, 10);
	timeStepSlider->setValue(TIMESTEP_VALUE_START);
	timeStepSlider->setMinimum(0);
	timeStepSlider->setMaximum(MAX_TIMESTEP);

	temperatureUnitSelector->add("K");
	temperatureUnitSelector->add("°C");
	temperatureUnitSelector->add("°F");
	temperatureUnitSelector->select("K");
	temperatureUnitSelector->setTextSize(10);
	temperatureUnitSelector->setTabHeight(12);
	temperatureUnitSelector->setPosition("100% - 170", tgui::bindBottom(timeStepSlider) + 10);

	// Procedurally generate a cog icon
	sf::RenderTexture rt;
	rt.create(32, 32);
	rt.clear(sf::Color::Transparent);

	sf::CircleShape body(10);
	body.setOrigin(10, 10);
	body.setPosition(16, 16);
	body.setFillColor(sf::Color(200, 200, 200));
	rt.draw(body);

	sf::RectangleShape tooth(sf::Vector2f(6, 6));
	tooth.setOrigin(3, 14);
	tooth.setPosition(16, 16);
	tooth.setFillColor(sf::Color(200, 200, 200));

	for (int i = 0; i < 8; ++i)
	{
		tooth.setRotation(i * 45.0f);
		rt.draw(tooth);
	}

	sf::CircleShape hole(4);
	hole.setOrigin(4, 4);
	hole.setPosition(16, 16);
	hole.setFillColor(sf::Color::Transparent);
	// Use blend mode to cut a hole
	rt.draw(hole, sf::BlendNone);

	rt.display();
	optionsButtonTexture = rt.getTexture();

	optionsButton->setImage(optionsButtonTexture);
	optionsButton->setSize(21, 21);
	optionsButton->setPosition("100% - 30", "100% - 30");
	optionsButton->getRenderer()->setBackgroundColor(sf::Color::Transparent);
	optionsButton->getRenderer()->setBorderColor(sf::Color::Transparent);
	optionsButton->getRenderer()->setBackgroundColorHover(sf::Color(255, 255, 255, 50));
	optionsButton->onPress([this]() { optionsMenu->setVisible(!optionsMenu->isVisible()); });

	optionsMenu->setSize(200, 120);
	optionsMenu->setPosition("50% - 100", "50% - 50");
	optionsMenu->setVisible(false);
	optionsMenu->setCloseBehavior(tgui::ChildWindow::CloseBehavior::Hide);

	gravityCheckBox->setPosition(10, 10);
	gravityCheckBox->setChecked(gravity_enabled);
	gravityCheckBox->onChange([this](bool checked) { gravity_enabled = checked; });
	optionsMenu->add(gravityCheckBox);

	renderLifeAlwaysCheckBox->setPosition(10, 40);
	renderLifeAlwaysCheckBox->setChecked(true);
	optionsMenu->add(renderLifeAlwaysCheckBox);
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
		planets[i].render(window);
	}

	if (renderLifeAlwaysCheckBox->isChecked())
	{
		std::map<int, std::vector<size_t>> civGroups;
		for (size_t i = 0; i < planets.size(); i++)
		{
			drawLifeVisuals(window, planets[i]);
			if (planets[i].getLife().getTypeEnum() >= 6)
			{
				civGroups[planets[i].getLife().getId()].push_back(i);
			}
		}

		for (auto const& [id, members] : civGroups)
		{
			if (members.size() < 2) continue;

			// Prim's algorithm for MST
			std::vector<bool> inMST(members.size(), false);
			std::vector<double> minEdge(members.size(), std::numeric_limits<double>::max());
			std::vector<int> parent(members.size(), -1);

			minEdge[0] = 0;
			for (size_t count = 0; count < members.size(); ++count)
			{
				int u = -1;
				for (size_t i = 0; i < members.size(); ++i)
				{
					if (!inMST[i] && (u == -1 || minEdge[i] < minEdge[u]))
						u = i;
				}

				if (u == -1 || minEdge[u] == std::numeric_limits<double>::max()) break;

				inMST[u] = true;
				if (parent[u] != -1)
				{
					const auto& p1 = planets[members[u]];
					const auto& p2 = planets[members[parent[u]]];
					sf::Vertex q[] =
					{
						sf::Vertex(sf::Vector2f(p1.getx(), p1.gety()), p1.getLife().getCol()),
						sf::Vertex(sf::Vector2f(p2.getx(), p2.gety()), p1.getLife().getCol())
					};
					window.draw(q, 2, sf::Lines);
				}

				for (size_t v = 0; v < members.size(); ++v)
				{
					if (!inMST[v])
					{
						double dist = planets[members[u]].getDist(planets[members[v]]);
						if (dist < minEdge[v])
						{
							minEdge[v] = dist;
							parent[v] = u;
						}
					}
				}
			}
		}
	}
}

void Space::drawLifeVisuals(sf::RenderWindow& window, const Planet& p)
{
	lType lt = p.getLife().getTypeEnum();
	if (lt < 1) return;

	sf::Vector2f pos(p.getx(), p.gety());
	float zoom = click_and_drag_handler.get_zoom();

	float indicatorRad = p.getRadius() + 5.0f;
	if (lt < 4) indicatorRad = p.getRadius() + 2.0f;

	sf::CircleShape indicator(indicatorRad);
	indicator.setPosition(pos);
	indicator.setOrigin(indicatorRad, indicatorRad);
	indicator.setFillColor(sf::Color(0, 0, 0, 0));
	indicator.setOutlineColor(p.getLife().getCol());

	float thickness = 1.0f;
	if (lt >= 4) thickness = 2.0f;
	if (lt >= 6) thickness = 3.0f;

	indicator.setOutlineThickness(thickness * zoom);
	window.draw(indicator);
}

void Space::drawCivConnections(sf::RenderWindow& window, const Planet& p, bool drawIndicatorsOnColonies)
{
	if (p.getLife().getTypeEnum() < 6) return;

	std::vector<size_t> members;
	for (size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getLife().getId() == p.getLife().getId())
		{
			members.push_back(i);
		}
	}

	if (members.size() < 2) return;

	// Prim's algorithm for MST
	std::vector<bool> inMST(members.size(), false);
	std::vector<double> minEdge(members.size(), std::numeric_limits<double>::max());
	std::vector<int> parent(members.size(), -1);

	minEdge[0] = 0;
	for (size_t count = 0; count < members.size(); ++count)
	{
		int u = -1;
		for (size_t i = 0; i < members.size(); ++i)
		{
			if (!inMST[i] && (u == -1 || minEdge[i] < minEdge[u]))
				u = i;
		}

		if (u == -1 || minEdge[u] == std::numeric_limits<double>::max()) break;

		inMST[u] = true;
		if (parent[u] != -1)
		{
			const auto& p1 = planets[members[u]];
			const auto& p2 = planets[members[parent[u]]];
			sf::Vertex q[] =
			{
				sf::Vertex(sf::Vector2f(p1.getx(), p1.gety()), p1.getLife().getCol()),
				sf::Vertex(sf::Vector2f(p2.getx(), p2.gety()), p1.getLife().getCol())
			};
			window.draw(q, 2, sf::Lines);
		}

		for (size_t v = 0; v < members.size(); ++v)
		{
			if (!inMST[v])
			{
				double dist = planets[members[u]].getDist(planets[members[v]]);
				if (dist < minEdge[v])
				{
					minEdge[v] = dist;
					parent[v] = u;
				}
			}
		}
	}

	if (drawIndicatorsOnColonies)
	{
		for (size_t idx : members)
		{
			if (&planets[idx] != &p)
				drawLifeVisuals(window, planets[idx]);
		}
	}
}

void Space::drawEffects(sf::RenderWindow &window)
{

	//EXPLOSIONS
	int inc = 1;
	if (timestep == 0) inc = 0;

	for(size_t i = 0; i < explosions.size(); i++)
	{
		explosions[i].move(timestep);

		if (explosions[i].getAge(inc) < explosions[i].levetidmax()) explosions[i].render(window);
		else
		{
			removeExplosion(i);
		}
	}
	
	//DUST (Moved to drawDust)
	//particles->render_all(window);

	//TRAILS
	for(size_t i = 0; i < trail.size(); i++)
	{
		trail[i].move(timestep);
		if (trail[i].getAge(0) < trail[i].levetidmax() && !trail[i].killMe()) trail[i].render(window);
		else
		{
			removeTrail(i);
		}
	}

    if (ship.isExist())
    {
        ship.renderProjectiles(window);
        ship.renderTug(window, *this);
        ship.renderCharge(window);
        ship.renderTrajectory(window, click_and_drag_handler.get_zoom());
    }

}

void Space::drawDust(sf::RenderWindow &window)
{
    particles->render_all(window);
}

double Space::thermalEnergyAtPosition(sf::Vector2f pos)
{
	double tEnergyFromOutside = 0;

	for(size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getMass() < BIGSTARLIMIT && planets[i].getMass() >= GASGIANTLIMIT)
		{
			double dist = sqrt((planets[i].getPosition().x - pos.x)*(planets[i].getPosition().x - pos.x) + (planets[i].getPosition().y - pos.y) * (planets[i].getPosition().y - pos.y));
			tEnergyFromOutside += planets[i].giveThermalEnergy(1)/ std::max(dist, 1.0);
		}
	}

	return tEnergyFromOutside;
}
