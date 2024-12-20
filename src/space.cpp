#include "space.h"

Space::Space(int x, int y, bool f)
{
	xsize = x;
	ysize = y;
	fullScreen = f;
}

void Space::addPlanet(Planet p)
{
	giveId(p);
	p.setTemp((p.fusionEnergy() / (p.getRad()*p.getRad()*SBconst)) + thermalEnergyAtPosition(sf::Vector2f(p.getx(), p.gety()))*tempConstTwo/SBconst);
	
	p.setColor();
	pListe.push_back(p);
}

void Space::addExplosion(sf::Vector2f p, double s, sf::Vector2f v, int l)
{
	expListe.push_back(Explosion(p, s, 0, v, l));
}

void Space::addSmoke(sf::Vector2f p, double s, sf::Vector2f v, int l)
{
	smkListe.push_back(Smoke(p, s, 0, v, l));
}

void Space::printPListe()
{
	if (pListe.size() > 0)
	{
		for (size_t i = 0; i < pListe.size(); i++)
		{
			std::cout << "Element: " << i << " // ";
			pListe[i].printInfoShort();
		}
	}
	else
	{
		std::cout << "Listen er tom!" << std::endl;
	}

	std::cout << std::endl;
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

	for (size_t i = 0; i < pListe.size(); i++)
	{

		Planet p = pListe[i];
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

void accumulate_acceleration(const DistanceCalculationResult & distance_info, 
						Acceleration2D & acceleration,
						const Planet & other_planet)
{
	const auto F = G * other_planet.getmass() / std::max(distance_info.dist * distance_info.dist, 0.01);
	const auto angle = atan2(distance_info.dy, distance_info.dx);

	acceleration.x += cos(angle) * F;
	acceleration.y += sin(angle) * F;
}

void Space::update()
{
	//SETUP & OTHER
	totalMass = 0;
	if (pListe.size() > 0) iteration += 1;
	if (ship.getLandedState())
	{
		if (findPlanet(ship.getPlanetID()).getmass() == -1) ship.setLandedstate(false);
	}
	bool updateSMK = false;
	if (iteration % SMK_ACCURACY == 0) updateSMK = true;

	//SHIPCHECK
	for (size_t i = 0; i < pListe.size(); i++)
	{
		if (ship.isExist() && !ship.pullofGravity(pListe[i], ship, timeStep))
		{
			addExplosion(ship.getpos(), 10, sf::Vector2f(0, 0), 10);
		}
	}

	//SMOKE MOVE
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < pListe.size(); i++)
	{
		if (pListe[i].getmass() > MINIMUMBREAKUPSIZE && updateSMK) GravitySmoke(pListe[i], timeStep);
	}


	for (auto & thisPlanet : pListe)
	{
		if (thisPlanet.isMarkedForRemoval())
			continue;

		Acceleration2D acc_sum_1;
		for (auto & otherPlanet : pListe)
		{
			if (otherPlanet.isMarkedForRemoval() ||
				thisPlanet.getId() == otherPlanet.getId())
				continue;

			DistanceCalculationResult distance = calculateDistance(thisPlanet, otherPlanet);
			accumulate_acceleration(distance, acc_sum_1, otherPlanet);

			if (thisPlanet.getType() != BLACKHOLE &&
				distance.dist < ROCHE_LIMIT_DIST_MULTIPLIER * distance.rad_dist &&
				thisPlanet.getmass() > MINIMUMBREAKUPSIZE &&
				thisPlanet.getmass() / otherPlanet.getmass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
			{
				explodePlanet(thisPlanet);
				break;
			}

			if (distance.dist < distance.rad_dist)
			{
				if (thisPlanet.getmass() <= otherPlanet.getmass())
				{
					removePlanet(thisPlanet.getId());
					if (MAX_N_DUST_PARTICLES > smkListe.size() + PARTICULES_PER_COLLISION)
						for (size_t i = 0; i < PARTICULES_PER_COLLISION; i++)
							addSmoke(sf::Vector2f(thisPlanet.getx(), thisPlanet.gety()), 10, sf::Vector2f(thisPlanet.getxv() + CREATEDUSTSPEEDMULT * modernRandomWithLimits(-4, 4), thisPlanet.getyv() + CREATEDUSTSPEEDMULT * modernRandomWithLimits(-4, 4)), DUSTLEVETID);
					addExplosion(sf::Vector2f(thisPlanet.getx(), thisPlanet.gety()), 2 * thisPlanet.getRad(), sf::Vector2f(thisPlanet.getxv() * 0.5, thisPlanet.getyv() * 0.5), sqrt(thisPlanet.getmass()) / 2);
					otherPlanet.collision(thisPlanet);
					otherPlanet.incMass(thisPlanet.getmass());
					break;
				}
			}
		}
		
		/* Integrate (first part) (Leapfrog) */
		/* https://en.wikipedia.org/wiki/Leapfrog_integration */

		thisPlanet.setx(thisPlanet.getx() + thisPlanet.getxv() * timeStep + 0.5 * acc_sum_1.x * timeStep * timeStep);
		thisPlanet.sety(thisPlanet.gety() + thisPlanet.getyv() * timeStep + 0.5 * acc_sum_1.y * timeStep * timeStep);

		Acceleration2D acc_sum_2;
		for (auto& otherPlanet : pListe)
		{
			if (otherPlanet.isMarkedForRemoval() ||
				thisPlanet.getId() == otherPlanet.getId())
				continue;

			DistanceCalculationResult distance = calculateDistance(thisPlanet, otherPlanet);
			accumulate_acceleration(distance, acc_sum_2, otherPlanet);
		}

		/* Integrate (second part) (Leapfrog) */
		thisPlanet.setxv(thisPlanet.getxv() + 0.5 * (acc_sum_1.x + acc_sum_2.x) * timeStep);
		thisPlanet.setyv(thisPlanet.getyv() + 0.5 * (acc_sum_1.y + acc_sum_2.y) * timeStep);
	}

	std::erase_if(pListe, [](const Planet & planet) { return planet.isMarkedForRemoval(); });

	//OTHER
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < pListe.size(); i++)
	{
		//TEMPERATURE
		pListe[i].coolDOWN(timeStep);
		pListe[i].setColor();
		pListe[i].updateTemp();

		//ATMOSPHERE
		pListe[i].updateAtmo(timeStep);

		//LIFE
		pListe[i].updateLife(timeStep);
	}

	//COLONIZATION
	for (size_t i = 0; i < pListe.size(); i++)
	{
		if (pListe[i].getLife().willExp())
		{
			int index = findBestPlanet(i);
			if (index != -1) pListe[index].colonize(pListe[i].getLife().getId(), pListe[i].getLife().getCol(), pListe[i].getLife().getDesc());
		}
	}
	
	//CHECKING IF ANYTHING IS OUTSIDE BOUNDS
	if (bound.getState())
	{
		//PLANETS
		for (size_t i = 0; i < pListe.size(); i++)
		{
			if (bound.isOutside(sf::Vector2f(pListe[i].getx(), pListe[i].gety())))
				removePlanet(pListe[i].getId());
		}

		//DUST
		for (size_t i = 0; i < smkListe.size(); i++)
		{
			if (bound.isOutside(smkListe[i].getpos())) removeSmoke(i);
		}

		//SHIP
		if (bound.isOutside(ship.getpos())) ship.destroy();

	}

	//PLACING BOUND AROUND MASS CENTRE IF AUTOBOUND IS ENABLED
	if (autoBound->isChecked() && iteration%BOUND_AUTO_UPDATE_RATE == 0)
	{
		bound.setPos(centerOfMassAll());
		bound.setState(true);
		bound.setRad(START_RADIUS);
	}

	//CHECKING TOTAL MASS
	for (size_t i = 0; i < pListe.size(); i++)
		totalMass += pListe[i].getmass();
}

void Space::hotkeys(sf::Window& w, sf::View& v)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma) && timeStepSlider->getValue() > timeStepSlider->getMinimum())
	{
		timeStepSlider->setValue(timeStepSlider->getValue() - 1);
		if (timeStepSlider->getValue() < timeStepSlider->getMinimum()) timeStepSlider->setValue(timeStepSlider->getMinimum());
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period) && timeStepSlider->getValue() < timeStepSlider->getMaximum())
	{
		timeStepSlider->setValue(timeStepSlider->getValue() + 1);
		if (timeStepSlider->getValue() > timeStepSlider->getMaximum()) timeStepSlider->setValue(timeStepSlider->getMaximum());
	}

	const static std::map<sf::Keyboard::Key, std::string> hotkey_map
	{
		{sf::Keyboard::F, "Object (F)"},
		{sf::Keyboard::O, "Object in orbit (O)"},
		{sf::Keyboard::S, "Adv Object in orbit (S)"},
		{sf::Keyboard::D, "Remove object (D)"},
		{sf::Keyboard::C, "Explode object (C)"},
		{sf::Keyboard::G, "Random system (G)"},
		{sf::Keyboard::Q, "Rings (Q)"},
		{sf::Keyboard::E, "Spawn ship (E)"},
		{sf::Keyboard::I, "Info (I)"},
		{sf::Keyboard::T, "Follow object (T)"},
		{sf::Keyboard::B, "Bound (B)"}
	};

	for (const auto& [key, action] : hotkey_map)
	{
		if (sf::Keyboard::isKeyPressed(key))
			functions->setSelectedItem(action);
	}
}

std::vector<Planet> Space::getPListe()
{
	return pListe;
}

void Space::randomPlanets(int totmass,int antall, double radius, sf::Vector2f pos)
{
	double speedmultRandom = 0.00000085 * modernRandomWithLimits(120*totmass, 150*totmass);
	double angle = 0;
	double delta_angle = 2*PI / antall;
	double centermass = modernRandomWithLimits(totmass / 3 , totmass / 2);
	totmass -= centermass;

	Planet centerP(centermass, pos.x, pos.y);

	addPlanet(centerP);

	for (int i = 0; i < antall; i++, angle += delta_angle)
	{
		double mass = modernRandomWithLimits(0.6*totmass /antall, 1.4*totmass /antall);

		double radius2 = modernRandomWithLimits(2*centerP.getRad() , radius);
		double randomelement1 = modernRandomWithLimits(-speedmultRandom*0.5, speedmultRandom*0.5);
		double randomelement2 = modernRandomWithLimits(-speedmultRandom*0.5, speedmultRandom*0.5);

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
	if (ind >= (int) trlListe.size() || ind < 0) return;

	auto it = trlListe.begin() + ind;
	*it = std::move(trlListe.back());
	trlListe.pop_back();

}

void Space::removeExplosion(int ind)
{
	if (ind >= (int) expListe.size() || ind < 0) return;

	auto it = expListe.begin() + ind;
	*it = std::move(expListe.back());
	expListe.pop_back();
}

void Space::removeSmoke(int ind)
{
	if (ind >= (int) smkListe.size() || ind < 0) return;

	std::vector<Smoke> midl;

	auto it = smkListe.begin() + ind;
	*it = std::move(smkListe.back());
	smkListe.pop_back();
}

void Space::clear(sf::View& v, sf::Window& w)
{
	(void) w;

	pListe.clear();
	expListe.clear();
	smkListe.clear();
	trlListe.clear();
	bound = Bound();

	xtrans = 0;
	ytrans = 0;
	xmidltrans = 0;
	ymidltrans = 0;
	zoom = 1;
	v = sf::View();
	v.setSize(sf::Vector2f(xsize, ysize));
	v.setCenter(0, 0);
	ship.reset(sf::Vector2f(0, 0));
	iteration = 0;
	fokusId = -1;
}

void Space::explodePlanet(Planet& planet)
{
	const auto match = std::find_if(pListe.begin(), pListe.end(), 
		[&planet](const auto& p) { return p.getId() == planet.getId(); });
	if (match == pListe.end())
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
		double angle = ((double)modernRandomWithLimits(0, 200 * PI)) / 100;
		double deltaVinkel = 2 * PI / kanter;
		double dist = 2 * rad * (1 / sin(deltaVinkel) - 1) + EXPLODE_PLANET_DISTCONST + 2 * rad + (2 * kanter - 10) * rad;
		double escVel = G * sqrt(origMass) * EXPLODE_PLANET_SPEEDMULT_OTHER / cbrt(dist + 0.1);
		kanter++;

		for (size_t q = 0; q < kanter; q++)
		{
			Planet Q(mass, x + dist * cos(angle), y + dist * sin(angle), xv + escVel * cos(angle), yv + escVel * sin(angle));
			Q.setTemp(1500);
			addPlanet(Q);
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
	for(size_t i = 0; i < pListe.size(); i++)
	{
		if (pListe[i].getId() == id)
		{
			return pListe[i];
		}
	}
	return Planet(-1);
}

Planet* Space::findPlanetPtr(double id)
{
	for (auto & planet : pListe)
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

	for(size_t i = 0; i < pListe.size(); i++)
	{
		if (pListe[i].getType() == ROCKY || pListe[i].getType() == TERRESTIAL)
		{
			int nr = pListe[i].getLife().getTypeEnum();
			if (nr == 6 || nr == 7 || nr == 5 || nr == 4)
			{
			}
			else if (pListe[i].getSupportedBiomass() > highest && (int) i != q)
			{
				highest = pListe[i].getSupportedBiomass();
				ind = i;
			}
		}
	}

	return ind;
}

void Space::addTrail(sf::Vector2f p, int l)
{
	trlListe.push_back(Trail(p, l));
}

void Space::GravitySmoke(Planet& forcer, int t)
{
	for (auto& smoke : smkListe)
	{
		smoke.pullOfGravity(forcer, t);
	}
}

void Space::giveRings(Planet p, int inner, int outer)
{
	int antall = 0.05*outer*outer;
	double angle = 0;
	double delta_angle = 2*PI / antall;

	for (int i = 0; i < antall; i++)
	{
		double rad = ((double) modernRandomWithLimits(inner*1000, outer*1000))/1000;
		double hast = sqrt(G*p.getmass() / rad);

		smkListe.push_back(Smoke(sf::Vector2f(p.getx() + cos(angle)*rad, p.gety()+sin(angle)*rad), 1, 0, sf::Vector2f(hast*cos(angle + 1.507) + p.getxv(),hast*sin(angle + 1.507) + p.getyv()), 20000));
		
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
		return convertDoubleToString((int)(q - 273.15)) + "�C";
	}
	else if (e == 3)
	{
		return convertDoubleToString((int)((q - 273.15)* 1.8000 + 32.00)) + "�F";
	}
	return "-";
}

void Space::updateSpaceship()
{
	int mode = ship.move(timeStep);
	if (mode == 1)
	{
		sf::Vector2f v;
		double angl = ((double)modernRandomWithLimits(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;

		sf::Vector2f p;
		p.x = ship.getpos().x - 2 * cos(angl);
		p.y = ship.getpos().y - 2 * sin(angl);

		v.x = ship.getvel().x - cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y - sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, modernRandomWithLimits(1.3, 1.5), v, 400);

		angl = ((double)modernRandomWithLimits(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;
		v.x = ship.getvel().x - cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y - sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, modernRandomWithLimits(1.3, 1.5), v, 200);
	}
	else if (mode == -1)
	{
		sf::Vector2f v;
		double angl = ((double)modernRandomWithLimits(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;

		sf::Vector2f p;
		p.x = ship.getpos().x - 2 * cos(angl);
		p.y = ship.getpos().y - 2 * sin(angl);

		v.x = ship.getvel().x + cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y + sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, modernRandomWithLimits(1.3, 1.5), v, 400);

		angl = ((double)modernRandomWithLimits(-50, 50)) / 150 + 2 * PI*ship.getAngle() / 360;
		v.x = ship.getvel().x + cos(angl)*SHIP_GAS_EJECT_SPEED;
		v.y = ship.getvel().y + sin(angl)*SHIP_GAS_EJECT_SPEED;
		addSmoke(p, modernRandomWithLimits(1.3, 1.5), v, 200);
	}
}

void Space::setInfo()
{
	//Mass
	size = massSlider->getValue();

	//Timestep
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))timeStepSlider->setValue(0);
	timeStep = timeStepSlider->getValue();
	
	//Current planet sliders
	if (findPlanet(fokusId).getmass() == -1)
	{
		currPlanetInfo->setVisible(false);
		massExistingObjectSlider->setVisible(false);
	}
	else
	{
		//Gathering current object mass slider info
		if (mouseOnMassSliderSelected && sf::Mouse::isButtonPressed(sf::Mouse::Left))
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
	if (tempChooser->getSelectedIndex() < 2) tempEnhet = tempChooser->getSelectedIndex() + 1;
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
	simInfo->setText("Framerate: " + convertDoubleToString(fps) + "\nFrame: " + convertDoubleToString(iteration) + "\nTimestep (,/.): " + convertDoubleToString(timeStep) + "\nTotal mass: " + convertDoubleToString(totalMass) + "\nObjects: " + convertDoubleToString(pListe.size()) + "\nParticles: " + convertDoubleToString(smkListe.size()) + "\nZoom: " + convertDoubleToString(1 / zoom));

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
	functions->addItem("Object (F)");
	functions->addItem("Object in orbit (O)");
	functions->addItem("Adv Object in orbit (S)");
	functions->addItem("Remove object (D)");
	functions->addItem("Explode object (C)");
	functions->addItem("Random system (G)");
	functions->addItem("Rings (Q)");
	functions->addItem("Spawn ship (E)");
	functions->addItem("Info (I)");
	functions->addItem("Follow object (T)");
	functions->addItem("Bound (B)");
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

	tempChooser->add("K");
	tempChooser->add("�C");
	tempChooser->add("�F");
	tempChooser->select("K");
	tempChooser->setTextSize(10);
	tempChooser->setTabHeight(12);
	tempChooser->setPosition(165, 50);

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

void Space::drawPlanetInfo(sf::RenderWindow& w, sf::View& v)
{
	(void) v;

	//PRINTING INFO TO WINDOW
	if (findPlanet(fokusId).getmass() != -1)
	{
		//GATHERING INFO
		Planet fokusP = findPlanet(fokusId);
		sf::Vector2f pos(fokusP.getx(), fokusP.gety());
		Planet fokusP_Parent = findPlanet(fokusP.getStrongestAttractorId());
		std::string typeplanet;

		//DETERMINING OBJECT TYPE
		switch (fokusP.getType())
		{
		case ROCKY: typeplanet = "Rocky Planet"; break;
		case TERRESTIAL: typeplanet = "Terrestial Planet"; break;
		case GASGIANT: typeplanet = "Gasgiant"; break;
		case SMALLSTAR: typeplanet = "Red Dwarf"; break;
		case STAR: typeplanet = "Yellow Dwarf"; break;
		case BIGSTAR: typeplanet = "Blue Giant"; break;
		case BLACKHOLE: typeplanet = "Black Hole"; break;
		default: typeplanet = "????????"; break;
		}

		//INFOVECTOR
		sf::Vertex l[] =
		{
			sf::Vertex(sf::Vector2f(pos.x + 1.5*fokusP.getRad() - xmidltrans, pos.y + 1.5*fokusP.getRad() - ymidltrans),sf::Color::Cyan),
			sf::Vertex(sf::Vector2f(pos.x - xmidltrans, pos.y - ymidltrans), sf::Color::Cyan)
		};
		w.draw(l, 2, sf::Lines);

		//VELOCITY VECTOR
		sf::Vertex v[] =
		{
			sf::Vertex(sf::Vector2f(pos.x + 400 * fokusP.getxv() - xmidltrans, pos.y + 400 * fokusP.getyv() - ymidltrans),sf::Color::Red),
			sf::Vertex(sf::Vector2f(pos.x - xmidltrans, pos.y - ymidltrans), sf::Color::Red)
		};
		w.draw(v, 2, sf::Lines);

		//DRAWING TRAILS
		if (iteration % TRAILFREQ == 0) addTrail(pos, TRAILLIFE);

		//POINTING TO STRONGEST GRAVITY SOURCE
		if (pListe.size() > 1 && fokusP_Parent.getmass() != -1)
		{
			sf::Vertex g[] =
			{
				sf::Vertex(sf::Vector2f(fokusP_Parent.getx() - xmidltrans, fokusP_Parent.gety() - ymidltrans),sf::Color::Yellow),
				sf::Vertex(sf::Vector2f(pos.x - xmidltrans, pos.y - ymidltrans), sf::Color::Yellow)
			};
			w.draw(g, 2, sf::Lines);
		}

		//DRAWING ROCHE LIMIT IF IT EXISTS
		if (fokusP_Parent.getmass() != -1 && fokusP.getmass() > MINIMUMBREAKUPSIZE && fokusP.getmass() / fokusP_Parent.getmass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
		{
			double rocheRad = ROCHE_LIMIT_DIST_MULTIPLIER*(fokusP_Parent.getRad() + fokusP.getRad());

			sf::CircleShape omr(rocheRad);
			omr.setPosition(sf::Vector2f(fokusP_Parent.getx() - xmidltrans, fokusP_Parent.gety() - ymidltrans));
			omr.setOrigin(rocheRad, rocheRad);
			omr.setFillColor(sf::Color(0, 0, 0, 0));
			omr.setOutlineColor(sf::Color(255, 140, 0));
			omr.setOutlineThickness(1 * zoom);
			w.draw(omr);
		}

		//CENTER OF MASS
		sf::CircleShape midtpunkt(2);
		midtpunkt.setOrigin(2, 2);
		midtpunkt.setFillColor(sf::Color::Yellow);

		double avst = sqrt((fokusP_Parent.getx() - pos.x)*(fokusP_Parent.getx() - pos.x) + (fokusP_Parent.gety() - pos.y)*(fokusP_Parent.gety() - pos.y));
		avst = avst*(fokusP_Parent.getmass()) / (fokusP.getmass() + fokusP_Parent.getmass());
		double angleb = atan2(fokusP_Parent.gety() - fokusP.gety(), fokusP_Parent.getx() - fokusP.getx());

		midtpunkt.setPosition(fokusP.getx() + avst*cos(angleb) - xmidltrans, fokusP.gety() + avst*sin(angleb) - ymidltrans);
		w.draw(midtpunkt);

		//GOLDILOCK-ZONE
		if (fokusP.getType() == SMALLSTAR || fokusP.getType() == STAR || fokusP.getType() == BIGSTAR)
		{
			double goldilock_inner_rad = (tempConstTwo * fokusP.getRad() * fokusP.getRad() * fokusP.temp()) / inner_goldi_temp;
			double goldilock_outer_rad = (tempConstTwo * fokusP.getRad() * fokusP.getRad() * fokusP.temp()) / outer_goldi_temp;

			sf::CircleShape g(goldilock_inner_rad);
			g.setPointCount(60);
			g.setPosition(sf::Vector2f(fokusP.getx()-xmidltrans, fokusP.gety() - ymidltrans));
			g.setOrigin(goldilock_inner_rad, goldilock_inner_rad);
			g.setOutlineThickness(goldilock_outer_rad - goldilock_inner_rad);
			g.setFillColor(sf::Color(0, 0, 0, 0));
			g.setOutlineColor(sf::Color(0, 200, 0, goldi_strength));
			w.draw(g);
		}
		if ((fokusP_Parent.getType() == SMALLSTAR || fokusP_Parent.getType() == STAR || fokusP_Parent.getType() == BIGSTAR) && pListe.size() > 1)
		{
			double goldilock_inner_rad = (tempConstTwo * fokusP_Parent.getRad() * fokusP_Parent.getRad() * fokusP_Parent.temp()) / inner_goldi_temp;
			double goldilock_outer_rad = (tempConstTwo * fokusP_Parent.getRad() * fokusP_Parent.getRad() * fokusP_Parent.temp()) / outer_goldi_temp;

			sf::CircleShape g(goldilock_inner_rad);
			g.setPointCount(60);
			g.setPosition(sf::Vector2f(fokusP_Parent.getx() - xmidltrans, fokusP_Parent.gety() - ymidltrans));
			g.setOrigin(goldilock_inner_rad, goldilock_inner_rad);
			g.setOutlineThickness(goldilock_outer_rad - goldilock_inner_rad);
			g.setFillColor(sf::Color(0, 0, 0, 0));
			g.setOutlineColor(sf::Color(0, 200, 0, goldi_strength));
			w.draw(g);
		}

		//DRAWING LINES TO OTHER PLANET WITH THE SAME SPECIES
		if (fokusP.getLife().getTypeEnum() >= 6) {
			for(size_t i = 0; i < pListe.size(); i++)
			{
				if (pListe[i].getLife().getId() == fokusP.getLife().getId())
				{
					sf::Vertex q[] =
					{
						sf::Vertex(sf::Vector2f(pListe[i].getx() - xmidltrans, pListe[i].gety() - ymidltrans),fokusP.getLife().getCol()),
						sf::Vertex(sf::Vector2f(pos.x - xmidltrans, pos.y - ymidltrans),fokusP.getLife().getCol())
					};
					w.draw(q, 2, sf::Lines);

					sf::CircleShape omr(pListe[i].getRad() + 5);
					omr.setPosition(sf::Vector2f(pListe[i].getx() - xmidltrans, pListe[i].gety() - ymidltrans));
					omr.setOrigin(pListe[i].getRad() + 5, pListe[i].getRad() + 5);
					omr.setFillColor(sf::Color(0, 0, 0, 0));
					omr.setOutlineColor(pListe[i].getLife().getCol());
					omr.setOutlineThickness(1 * zoom);
					w.draw(omr);
				}
			}
		}

		//FINDING GREENHOUSE EFFECT
		double dTherEnergy = fokusP.thermalEnergy() - fokusP.thermalEnergy() / (1 + greenHouseEffectMult*fokusP.getAtmoCur());
		double dTemp = dTherEnergy / (fokusP.getmass()*fokusP.getTCap());
		std::string dTempString;
		if (tempEnhet == 1) dTempString = calcTemperature(dTemp, tempEnhet);
		else dTempString = calcTemperature(dTemp + 273.15,tempEnhet);

		//FIXING TEXT
		text2.setPosition(pos.x + 1.5*fokusP.getRad() - xmidltrans, pos.y + 1.5*fokusP.getRad() - ymidltrans);
		text2.setScale(0.5*zoom, 0.5*zoom);
		text2.setString(fokusP.getName() + "\nType: " + typeplanet + "\nRadius: " + convertDoubleToString(fokusP.getRad()) + "\nMass: " + convertDoubleToString(fokusP.getmass()) + "\nSpeed: " + convertDoubleToString(sqrt(fokusP.getxv()*fokusP.getxv() + fokusP.getyv()*fokusP.getyv())));
		if (pListe.size() > 1) text2.setString(text2.getString() + "\nDistance: " + convertDoubleToString(sqrt((findPlanet(fokusP.getStrongestAttractorId()).getx() - pos.x)*(findPlanet(fokusP.getStrongestAttractorId()).getx() - pos.x) + (findPlanet(fokusP.getStrongestAttractorId()).gety() - pos.y)*(findPlanet(fokusP.getStrongestAttractorId()).gety() - pos.y))));
		text2.setString(text2.getString() + "\nTemp: " + calcTemperature(fokusP.temp(), tempEnhet));
		if (fokusP.getType() == TERRESTIAL)
		{
			text2.setString(text2.getString() + "\n\nAtmo: " + convertDoubleToString((int)fokusP.getAtmoCur()) + " / " + convertDoubleToString((int)fokusP.getAtmoPot()) + "kPa \nGreenhouse Effect: " + dTempString);
			if (fokusP.getLife().getTypeEnum() == 0) text2.setString(text2.getString() + "\n\n" + fokusP.getFlavorTextLife());
		}
		if (fokusP.getLife().getTypeEnum() != 0)
		{
			text2.setString(text2.getString() + "\n\nBiomass: " + convertDoubleToString((int)fokusP.getLife().getBmass()) + "MT");
			if (fokusP.getLife().getTypeEnum() > 3) text2.setString(text2.getString() + "\n" + fokusP.getLife().getDesc() + " (" + fokusP.getLife().getType() + ")\n" + fokusP.getFlavorTextLife());
			else text2.setString(text2.getString() + "\n" + fokusP.getLife().getType() + "\n" + fokusP.getFlavorTextLife());
		}
		text2.setColor(sf::Color(255, 255, 255));
		drawtext2 = true;
	}
}

//OTHER

int Space::modernRandomWithLimits(int min, int max)
{
	std::random_device seeder;
	std::default_random_engine generator(seeder());
	std::uniform_int_distribution<int> uniform(min, max);
	return uniform(generator);
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
	for(size_t i = 0; i < pListe.size(); i++)
	{
		pListe[i].draw(window, xmidltrans, ymidltrans);
	}
}

void Space::drawEffects(sf::RenderWindow &window)
{

	//EXPLOSIONS
	int inc = 1;
	if (timeStep == 0) inc = 0;

	for(size_t i = 0; i < expListe.size(); i++)
	{
		expListe[i].move(timeStep);

		if (expListe[i].getAge(inc) < expListe[i].levetidmax()) expListe[i].print(window, xmidltrans, ymidltrans);
		else
		{
			removeExplosion(i);
		}
	}
	
	//DUST
	for(int i = 0; i < smkListe.size(); i++)
	{
		smkListe[i].move(timeStep);
		if (smkListe[i].getAge(timeStep) < smkListe[i].levetidmax() && !smkListe[i].killMe())
		{
			smkListe[i].print(window, xmidltrans, ymidltrans);
		}
		else
		{
			removeSmoke(i);
		}
	}
	//TRAILS
	for(size_t i = 0; i < trlListe.size(); i++)
	{
		trlListe[i].move(timeStep);
		if (trlListe[i].getAge(0) < trlListe[i].levetidmax() && !trlListe[i].killMe()) trlListe[i].print(window, xmidltrans, ymidltrans);
		else
		{
			removeTrail(i);
		}
	}

}

void Space::drawLightEffects(sf::RenderWindow& window)
{
	for(size_t i = 0; i < pListe.size(); i++)
	{
		Planet p = pListe[i];
		
		if (p.getmass() >= GASGIANTLIMIT && p.getmass() < BIGSTARLIMIT)
		{
			sf::Color col = p.getStarCol();

			//LONG RANGE LIGHT
			col.a = EXPLOSION_LIGHT_START_STRENGTH;
			sf::VertexArray vertexArr(sf::TrianglesFan);
			vertexArr.append(sf::Vertex(sf::Vector2f(p.getx() - xmidltrans, p.gety() - ymidltrans), col));
			col.a = 0;
			double deltaAng = 2*PI / ((double)LIGHT_NUMBER_OF_VERTECES);
			double ang = 0;
			double rad = LIGHT_STRENGTH_MULTIPLIER * sqrt(p.fusionEnergy());
			for(size_t nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; nr++)
			{
				sf::Vector2f pos(p.getx() - xmidltrans + cos(ang) * rad, p.gety() - ymidltrans + sin(ang) * rad);
				vertexArr.append(sf::Vertex(pos, col));
				ang += deltaAng;
			}
			vertexArr.append(sf::Vertex(sf::Vector2f(p.getx() - xmidltrans + rad, p.gety() - ymidltrans), col));
			window.draw(vertexArr);

			//SHORT RANGE LIGHT
			col.a = LIGHT_START_STRENGTH * SHORT_LIGHT_STRENGTH_MULTIPLIER;
			sf::VertexArray vertexArr2(sf::TrianglesFan);
			vertexArr2.append(sf::Vertex(sf::Vector2f(p.getx() - xmidltrans, p.gety() - ymidltrans), col));
			col.a = LIGHT_END_STRENGTH;
			ang = 0;
			rad = SHORT_LIGHT_RANGE_MULTIPLIER * p.getRad();
			for(size_t nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; nr++)
			{
				sf::Vector2f pos(p.getx() - xmidltrans + cos(ang) * rad, p.gety() - ymidltrans + sin(ang) * rad);
				vertexArr2.append(sf::Vertex(pos, col));
				ang += deltaAng;
			}
			vertexArr2.append(sf::Vertex(sf::Vector2f(p.getx() - xmidltrans + rad, p.gety() - ymidltrans), col));
			window.draw(vertexArr2);
		}
	}
}

void Space::lockToObject(sf::RenderWindow& w, sf::View& v)
{
	//FINDING NEW OBJECT TO FOCUS ON
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && functions->getSelectedItem() == "Follow object (T)" && !mouseOnWidgets)
	{
		sf::Vector2i localPosition(w.mapPixelToCoords(sf::Mouse::getPosition(w), v));

		//SEARCH
		for(size_t i = 0; i < pListe.size(); i++)
		{
			const auto dist = std::hypot(localPosition.x - pListe[i].getx(), localPosition.y - pListe[i].gety());
			if (dist < pListe[i].getRad())
			{
				lockToObjectId = pListe[i].getId();
				return;
			}
		}

		//CHECKING IF WE PRESSED ON THE SPACESHIP (NOT USED FOR ANYTHING)
		const auto dist_to_ship = std::hypot(localPosition.x - ship.getpos().x, localPosition.y - ship.getpos().y);
		if (dist_to_ship < 4.5)
		{
			lockToObjectId = -1;
			return;
		}

		//IN CASE WE DON'T FIND AN OBJECT
		lockToObjectId = -2;
	}
}

double Space::thermalEnergyAtPosition(sf::Vector2f pos)
{
	double tEnergyFromOutside = 0;

	for(size_t i = 0; i < pListe.size(); i++)
	{
		if (pListe[i].getmass() < BIGSTARLIMIT && pListe[i].getmass() >= GASGIANTLIMIT)
		{
			tEnergyFromOutside += pListe[i].giveTEnergy(1)/ sqrt((pListe[i].getx() - pos.x)*(pListe[i].getx() - pos.x) + (pListe[i].gety() - pos.y) * (pListe[i].gety() - pos.y));
		}
	}

	return tEnergyFromOutside;
}
