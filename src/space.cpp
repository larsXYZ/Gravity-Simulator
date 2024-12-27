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

sf::Vector3f Space::centerOfMass(const std::vector<int> & object_ids)
{
	auto tMass = 0.0;
	auto xCont = 0.0;
	auto yCont = 0.0;
	for (const auto id : object_ids)
	{
		if (Planet* planet = findPlanetPtr(id))
		{
			tMass += planet->getmass();
			xCont += planet->getmass() * planet->getx();
			yCont += planet->getmass() * planet->gety();
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
			tMass += planet->getmass();
			xCont += planet->getmass()*planet->getxv();
			yCont += planet->getmass()*planet->getyv();
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
	curr_time += timestep;

	//SETUP & OTHER
	total_mass = 0;

	if (planets.size() > 0)
		iteration += 1;

	update_spaceship();

	particles->update(planets, bound, timestep, curr_time);

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
				const auto heat = tempConstTwo * thisPlanet->getRad() * thisPlanet->getRad() * otherPlanet.giveThermalEnergy(timestep) / distance.dist;
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
		
		/* Integrate (first part) (Leapfrog) */
		/* https://en.wikipedia.org/wiki/Leapfrog_integration */

		thisPlanet->setx(thisPlanet->getx() + thisPlanet->getxv() * timestep + 0.5 * acc_sum_1.x * timestep * timestep);
		thisPlanet->sety(thisPlanet->gety() + thisPlanet->getyv() * timestep + 0.5 * acc_sum_1.y * timestep * timestep);

		Acceleration2D acc_sum_2;
		for (auto& otherPlanet : planets)
		{
			if (isIgnoringOtherPlanet(*thisPlanet, otherPlanet))
				continue;

			DistanceCalculationResult distance = calculateDistance(*thisPlanet, otherPlanet);
			accumulate_acceleration(distance, acc_sum_2, otherPlanet);
		}

		/* Integrate (second part) (Leapfrog) */
		thisPlanet->setxv(thisPlanet->getxv() + 0.5 * (acc_sum_1.x + acc_sum_2.x) * timestep);
		thisPlanet->setyv(thisPlanet->getyv() + 0.5 * (acc_sum_1.y + acc_sum_2.y) * timestep);
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
				planets[index].colonize(planets[i].getLife().getId(), planets[i].getLife().getCol(), planets[i].getLife().getDesc());
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

	total_mass = std::accumulate(planets.begin(), planets.end(), 0.0, [](auto v, const auto & p) {return v + p.getmass(); });
}

void Space::hotkeys(sf::Event event, sf::View & view, const sf::RenderWindow& window)
{
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

		radius = std::max(radius, 2 * centerP.getRad());
		double radius2 = uniform_random(2*centerP.getRad() , radius);
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
	planet.setTemp((planet.fusionEnergy() / (planet.getRad() * planet.getRad() * SBconst))
		+ thermalEnergyAtPosition(sf::Vector2f(planet.getx(), planet.gety())) * tempConstTwo / SBconst);
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
	explosions.clear();
	particles->clear();
	trail.clear();
	bound = Bound();

	view.setCenter(0, 0);
	view.setSize(window.getSize().x, window.getSize().y);
	ship.reset(sf::Vector2f(0, 0));
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

	if (match->getmass() < MINIMUMBREAKUPSIZE)
		return {};

	const auto& particles_by_rad = [planet]()
	{
		return static_cast<size_t>(50 * planet.getRad());
	};

	const auto n_dust_particles = std::clamp(MAX_N_DUST_PARTICLES - particles->size(),
		static_cast<size_t>(0), particles_by_rad());
	for (size_t i = 0; i < n_dust_particles; i++)
	{
		const auto scatter_pos = sf::Vector2f(planet.getx(), planet.gety()) + random_vector(planet.getRad());
		const auto scatter_vel = sf::Vector2f(planet.getxv(), planet.getyv()) + CREATEDUSTSPEEDMULT * random_vector(20.0);
		const auto lifespan = uniform_random(DUST_LIFESPAN_MIN, DUST_LIFESPAN_MAX);
		addSmoke(scatter_pos, scatter_vel, 2, lifespan);
	}

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
	const float original_mass = planet.getmass();

	const sf::Vector2f original_position = { static_cast<float>(planet.getx()),
										static_cast<float>(planet.gety()) };

	const sf::Vector2f original_velocity = { static_cast<float>(planet.getxv()),
										static_cast<float>(planet.getyv()) };

	const auto lifetime = 0.1 * planet.getRad();
	const auto size = 5.0 * planet.getRad();

	addExplosion(original_position, size, original_velocity, lifetime);

	const auto fragment_ids = disintegratePlanet(planet);
	for (auto id : fragment_ids)
	{
		if (auto fragment = findPlanetPtr(id))
		{
			const sf::Vector2f fragment_pos = { static_cast<float>(fragment->getx()),
													static_cast<float>(fragment->gety()) };

			const sf::Vector2f to_fragment = (fragment_pos - original_position);

			const sf::Vector2f escape_speed = EXPLODE_PLANET_SPEEDMULT_OTHER * sqrt(original_mass) * to_fragment / std::max(std::hypot(to_fragment.x, to_fragment.y), 0.1f) *	
												static_cast<float>(uniform_random(0.85, 1.15));

			fragment->setxv(fragment->getxv() + escape_speed.x);
			fragment->setyv(fragment->getyv() + escape_speed.y);
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
	for(size_t i = 0; i < planets.size(); i++)
	{
		if (planets[i].getId() == id)
		{
			return planets[i];
		}
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
		const double speed = sqrt(G*planet.getmass() / rad);

		const auto pos = sf::Vector2f(planet.getx() + cos(angle) * rad, planet.gety() + sin(angle) * rad);
		const auto vel = sf::Vector2f(speed * cos(angle + PI / 2.0) + planet.getxv(), speed * sin(angle + PI / 2.0));
		
		particles->add_particle(pos, vel, 1, curr_time+2000000);

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
	if (ship.getLandedState())
	{
		if (findPlanet(ship.getPlanetID()).getmass() == -1)
			ship.setLandedstate(false);
	}

	int mode = ship.move(timestep);
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

	for (const auto & planet : planets)
	{
		if (ship.isExist() && !ship.pullofGravity(planet, ship, timestep))
			addExplosion(ship.getpos(), 10, sf::Vector2f(0, 0), 10);
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
		planets[i].draw(window);
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
	
	//DUST
	particles->render_all(window);

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
