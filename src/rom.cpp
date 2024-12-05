#include "rom.h"

Rom::Rom(int x, int y, bool f)
{
	xstorrelse = x;
	ystorrelse = y;
	fullScreen = f;
}

void Rom::addPlanet(Planet p)
{
	giveId(p);
	p.setTemp((p.fusionEnergy() / (p.getRad()*p.getRad()*SBconst)) + getTherEnergyAtPos(sf::Vector2f(p.getx(), p.gety()))*tempConstTwo/SBconst);
	
	p.setColor();
	pListe.push_back(p);
}

void Rom::addExplosion(sf::Vector2f p, double s, sf::Vector2f v, int l)
{
	expListe.push_back(Explosion(p, s, 0, v, l));
}

void Rom::addSmoke(sf::Vector2f p, double s, sf::Vector2f v, int l)
{
	smkListe.push_back(Smoke(p, s, 0, v, l));
}

void Rom::printPListe()
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

sf::Vector3f Rom::centerOfMass(std::vector<int> midlPList)
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

sf::Vector2f Rom::centerOfMassVelocity(std::vector<int> midlPList)
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

sf::Vector2f Rom::centerOfMassAll()
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

void Rom::update()
{
	//SETUP & OTHER
	totalMass = 0;
	if (pListe.size() > 0) iterasjon += 1;
	if (ship.getLandedState())
	{
		if (findPlanet(ship.getPlanetID()).getmass() == -1) ship.setLandedstate(false);
	}
	bool updateSMK = false;
	if (iterasjon % SMK_ACCURACY == 0) updateSMK = true;

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

	//GRAVITY, COLLISIONS, ROCHE, AND TEMP
	for (size_t i = 0; i < pListe.size(); i++)
	{
		for (size_t p = 0; p < pListe.size(); p++)
		{
			if (p != i)
			{
				double dx = pListe[p].getx() - pListe[i].getx();
				double dy = pListe[p].gety() - pListe[i].gety();
				double dist = sqrt(dx*dx + dy*dy);
				double radDist = pListe[i].getRad() + pListe[p].getRad();

				//TEMP
				if (p != i && pListe[p].getType() != ROCKY && pListe[p].getType() != TERRESTIAL && pListe[p].getType() != GASGIANT)
				{
					pListe[i].heatUP(tempConstTwo*pListe[i].getRad()*pListe[i].getRad()*pListe[p].giveTEnergy(timeStep) / (dist), timeStep);
				}

				//GRAVITY, COLLISIONS AND ROCHE LIMIT
				if (pListe[i].getPlanetType() != BLACKHOLE && dist < ROCHE_LIMIT_DIST_MULTIPLIER*radDist && pListe[i].getmass() > MINIMUMBREAKUPSIZE && pListe[i].getmass() / pListe[p].getmass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
				{
					explodePlanetOld(i);
					break;
				}
				else if (dist < radDist)
				{
					if (pListe[i].getmass() >= pListe[p].getmass())
					{
						if (MAXANTALLDUST > smkListe.size() + PARTICULES_PER_COLLISION) for (size_t i = 0; i < PARTICULES_PER_COLLISION; i++) addSmoke(sf::Vector2f(pListe[p].getx(), pListe[p].gety()), 10, sf::Vector2f(pListe[p].getxv() + CREATEDUSTSPEEDMULT*modernRandomWithLimits(-4, 4), pListe[p].getyv() + CREATEDUSTSPEEDMULT*modernRandomWithLimits(-4, 4)), DUSTLEVETID);
						addExplosion(sf::Vector2f(pListe[p].getx(), pListe[p].gety()), 2 * pListe[p].getRad(), sf::Vector2f(pListe[p].getxv()*0.5, pListe[p].getyv()*0.5), sqrt(pListe[i].getmass()) / 2);
						pListe[i].kollisjon(pListe[p]);
						pListe[i].incMass(pListe[p].getmass());
						removePlanet(p);
					}
					break;
				}
				else
				{
					double aks = 0;
					double angle = atan2(dy, dx);

					if (dist != 0) aks = G * pListe[p].getmass() / (dist*dist);

					if (aks > pListe[i].strAttr())
					{
						pListe[i].strAttr() = aks;
						pListe[i].getStrongestAttractorIdRef() = pListe[p].getId();
					}

					pListe[i].getxv() += cos(angle) * aks * timeStep;
					pListe[i].getyv() += sin(angle) * aks * timeStep;
				}
			}
		}
	}

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

	//MOVE
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < pListe.size(); i++)
	{
		pListe[i].move(timeStep);
		pListe[i].resetAttractorMeasure();
	}

	//CHECKING IF ANYTHING IS OUTSIDE BOUNDS
	if (bound.getState())
	{
		//PLANETS
		for (size_t i = 0; i < pListe.size(); i++)
		{
			if (bound.isOutside(sf::Vector2f(pListe[i].getx(), pListe[i].gety()))) removePlanet(i);
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
	if (autoBound->isChecked() && iterasjon%BOUND_AUTO_UPDATE_RATE == 0)
	{
		bound.setPos(centerOfMassAll());
		bound.setState(true);
		bound.setRad(START_RADIUS);
	}

	//CHECKING TOTAL MASS
	for (size_t i = 0; i < pListe.size(); i++) totalMass += pListe[i].getmass();

}

std::vector<Planet> Rom::getPListe()
{
	return pListe;
}

void Rom::randomPlaneter(int totmass,int antall, double radius, sf::Vector2f pos)
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

void Rom::removePlanet(int ind)
{
	if (ind >= (int) pListe.size() || ind < 0) return;

	auto it = pListe.begin() + ind;
	*it = std::move(pListe.back());
	pListe.pop_back();
}

void Rom::removeTrail(int ind)
{
	if (ind >= (int) trlListe.size() || ind < 0) return;

	auto it = trlListe.begin() + ind;
	*it = std::move(trlListe.back());
	trlListe.pop_back();

}

void Rom::removeExplosion(int ind)
{
	if (ind >= (int) expListe.size() || ind < 0) return;

	auto it = expListe.begin() + ind;
	*it = std::move(expListe.back());
	expListe.pop_back();
}

void Rom::removeSmoke(int ind)
{
	if (ind >= (int) smkListe.size() || ind < 0) return;

	std::vector<Smoke> midl;

	auto it = smkListe.begin() + ind;
	*it = std::move(smkListe.back());
	smkListe.pop_back();
}

void Rom::clear(sf::View& v, sf::Window& w)
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
	v.setSize(sf::Vector2f(xstorrelse, ystorrelse));
	v.setCenter(0, 0);
	ship.reset(sf::Vector2f(0, 0));
	iterasjon = 0;
	fokusId = -1;
}

void Rom::explodePlanet(int ind)
{
	if (pListe.size() > 0 && pListe[ind].getmass() > MINIMUMBREAKUPSIZE)
	{
		double origMass = pListe[ind].getmass();
		double antall = ceil(origMass / MINIMUMBREAKUPSIZE);
		double mass = origMass / antall;
		double rad = (Planet(mass)).getRad();
		double x = pListe[ind].getx();
		double y = pListe[ind].gety();
		double xv = pListe[ind].getxv();
		double yv = pListe[ind].getyv();

		removePlanet(ind);
		addExplosion(sf::Vector2f(x, y), 90, sf::Vector2f(xv, yv), 10);

		double kanter = 5;
		int added = 0;


		for (size_t i = 0; i < antall; i += kanter)
		{
			double angle = ((double) modernRandomWithLimits(0,200*PI))/100;
			double deltaVinkel = 2 * PI / kanter;
			double dist = 2 * rad * (1 / sin(deltaVinkel) - 1) + EXPLODE_PLANET_DISTCONST + 2 * rad + (2 * kanter - 10)*rad;
			double escVel = G*sqrt(origMass) * EXPLODE_PLANET_SPEEDMULT_OTHER / cbrt(dist+0.1);
			kanter++;

			for (size_t q = 0; q < kanter; q++)
			{
				Planet Q(mass, x + dist  * cos(angle), y + dist * sin(angle), xv + escVel * cos(angle), yv + escVel * sin(angle));
				Q.setTemp(1500);
				addPlanet(Q);
				angle += deltaVinkel;
				added++;
				if (added >= antall) return;
			}

		}
		
	}
}

void Rom::explodePlanetOld(int ind)
{
	if (pListe.size() > 0)
	{
		double antall = modernRandomWithLimits(4, 8);
		double masse = pListe[ind].getmass();
		int x = pListe[ind].getx();
		double y = pListe[ind].gety();
		double radius = pListe[ind].getRad();

		double angle = 0;

		for(size_t i = 0; i < antall; i++)
		{

			Planet Q(masse / antall, x + 2 * cbrt(radius)*cos(angle), y + 2 * cbrt(radius)*sin(angle), pListe[ind].getxv() + EXPLODE_PLANET_SPEEDMULT* sqrt(masse) *cos(angle), pListe[ind].getyv() + EXPLODE_PLANET_SPEEDMULT * sqrt(masse) *sin(angle));
			Q.setTemp(1500);
			angle += 2 * PI / antall;
			addPlanet(Q);
		}
		removePlanet(ind);
		if (MAXANTALLDUST > smkListe.size() + PARTICULES_PER_EXPLOSION) for(size_t i = 0; i < PARTICULES_PER_EXPLOSION; i++) addSmoke(sf::Vector2f(x, y), 10, sf::Vector2f(pListe[ind].getxv() + CREATEDUSTSPEEDMULT*(((double)modernRandomWithLimits(-400, 400)) / 100), pListe[ind].getyv() + CREATEDUSTSPEEDMULT*(((double)modernRandomWithLimits(-400, 400)) / 100)), DUSTLEVETID);

	}
}

double Rom::range(double x1, double y1, double x2, double y2)
{
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

void Rom::giveId(Planet &p)
{
	p.mark(nesteid);
	nesteid += 1;
}

Planet Rom::findPlanet(double id)
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

Planet& Rom::findPlanetRef(double id)
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

int Rom::findBestPlanet(int q)
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

void Rom::addTrail(sf::Vector2f p, int l)
{
	trlListe.push_back(Trail(p, l));
}

void Rom::GravitySmoke(Planet& forcer, int t)
{
	for (auto& smoke : smkListe)
	{
		smoke.pullOfGravity(forcer, t);
	}
}

void Rom::giveRings(Planet p, int inner, int outer)
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

std::string Rom::calcTemperature(double q, int e)
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

void Rom::romskipHandling()
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

void Rom::setInfo()
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
			findPlanetRef(fokusId).setMass(massExistingObjectSlider->getValue());
			findPlanetRef(fokusId).updateRadiAndType();
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
	simInfo->setText("Framerate: " + convertDoubleToString(fps) + "\nFrame: " + convertDoubleToString(iterasjon) + "\nTimestep (,/.): " + convertDoubleToString(timeStep) + "\nTotal mass: " + convertDoubleToString(totalMass) + "\nObjects: " + convertDoubleToString(pListe.size()) + "\nParticles: " + convertDoubleToString(smkListe.size()) + "\nZoom (Z/X): " + convertDoubleToString(1 / zoom));

}

void Rom::initSetup()
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
	timeStepSlider->setMinimum(-TIMESTEP_VALUE_RANGE);
	timeStepSlider->setMaximum(TIMESTEP_VALUE_RANGE);

	tempChooser->add("K");
	tempChooser->add("°C");
	tempChooser->add("°F");
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

void Rom::printInfoPlanet(sf::RenderWindow& w, sf::View& v)
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
		if (iterasjon % TRAILFREQ == 0) addTrail(pos, TRAILLIFE);

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

void Rom::PlanetSkjermPrint(sf::RenderWindow &window)
{
	//DRAWING PLANETS																										
	for(size_t i = 0; i < pListe.size(); i++)
	{
		pListe[i].draw(window, xmidltrans, ymidltrans);
	}
}

void Rom::effectSkjermPrint(sf::RenderWindow &window)
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

void Rom::lightSkjermPrint(sf::RenderWindow& window)
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

void Rom::lockToObject(sf::RenderWindow& w, sf::View& v)
{
	//FINDING NEW OBJECT TO FOCUS ON
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && functions->getSelectedItem() == "Follow object (T)" && !mouseOnWidgets)
	{
		sf::Vector2i localPosition(w.mapPixelToCoords(sf::Mouse::getPosition(w), v));

		//SEARCH
		for(size_t i = 0; i < pListe.size(); i++)
		{
			if (range(localPosition.x, localPosition.y, pListe[i].getx(), pListe[i].gety()) < pListe[i].getRad())
			{
				lockToObjectId = pListe[i].getId();
				return;
			}
		}

		//CHECKING IF WE PRESSED ON THE SPACESHIP (NOT USED FOR ANYTHING)
		if (range(localPosition.x, localPosition.y, ship.getpos().x, ship.getpos().y) < 4.5)
		{
			lockToObjectId = -1;
			return;
		}

		//IN CASE WE DON'T FIND AN OBJECT
		lockToObjectId = -2;
	}
}

double Rom::getTherEnergyAtPos(sf::Vector2f pos)
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
