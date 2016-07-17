#include "rom.h"

void Rom::runSim()
{
	sf::RenderWindow window;

	//LAGER VINDU
	if (fullScreen)
	{
		window.create(sf::VideoMode(xstorrelse, ystorrelse), "Gravity Simulator", sf::Style::Fullscreen);
	}
	else
	{
		window.create(sf::VideoMode(xstorrelse, ystorrelse), "Gravity Simulator", sf::Style::Default);
	}

	sf::View view1;
	window.setFramerateLimit(framerate);
	view1.setSize(xstorrelse, ystorrelse);
	view1.setCenter(0, 0);
	window.setView(view1);
	sf::Clock clock;

	//LOADING FONT
	if (!font.loadFromFile("sansation.ttf"))
	{
		std::cout << "FONT-LOADING ERROR" << std::endl;
	}
	text.setFont(font);
	text.setCharacterSize(14);
	text2.setFont(font);
	text.setColor(sf::Color::White);
	text2.setColor(sf::Color::White);

	//LOADING GUI
	tgui::Gui gui{ window };
	gui.setFont("sansation.ttf");
	initSetup();
	gui.add(simInfo), gui.add(functions), gui.add(newPlanetInfo), gui.add(massSlider), gui.add(timeStepSlider), gui.add(tempChooser), gui.add(currPlanetInfo), gui.add(massExistingObjectSlider), gui.add(autoBound);

	//PREPARING EVENTS AND OTHER THINGS	
	int xx = 0;
	int yy = 0;
	xtrans = 0;
	ytrans = 0;
	xmidltrans = 0;
	ymidltrans = 0;

	//TOGGLES AND OTHER THINGS
	bool randomToggle = false;
	bool randomToggle2 = false;

	int ringPlanetCounter = 0;
	int planetFuncId = 0;

	sf::Vector2f mouse_rand_pos;
	sf::Vector2f mouse_ring_pos;

	double ringInnerRad = 0;
	double ringOuterRad;
	int createInOrbitCounter = 0;
	int advCreateInOrbitCounter = 0;
	bool harTrykket = false;
	bool harTrykket2 = false;

	int boundCounter = 0;

	
	while (window.isOpen())
	{
		//CLEARING WINDOW AND GETTING MOUSEPOS
		window.clear(sf::Color(10, 10, 10));
		mousePos = sf::Mouse::getPosition(window);

		//CHECKING IF MOUSE IS ON WIDGETS OR CURRENT OBJECT MASS-SLIDER
		mouseOnWidgets = false;
		for (size_t i = 0; i < gui.getWidgets().size(); i++)
		{
			if (gui.getWidgets()[i]->mouseOnWidget(mousePos.x,mousePos.y)) mouseOnWidgets = true;
		}
		if (!harTrykket && sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseOnWidgets) { harTrykket = true; }
		else if (harTrykket && sf::Mouse::isButtonPressed(sf::Mouse::Left)) { harTrykket = true, mouseOnWidgets = true;}
		else harTrykket = false;

		mouseOnMassSliderSelected = false;
		if (massExistingObjectSlider->mouseOnWidget(mousePos.x,mousePos.y)) mouseOnMassSliderSelected = true;
		if (!harTrykket2 && sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseOnMassSliderSelected) { harTrykket2 = true; }
		else if (harTrykket2 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) { harTrykket2 = true, mouseOnMassSliderSelected = true; }
		else harTrykket2 = false;

		//HOTKEYS
		hotkeys(window, view1);
		window.setView(view1);

		//GUI
		gui.handleEvent(event);
		setInfo();

		//EVENTS
		while (window.pollEvent(event))
			{
				//CLOSING WINDOW
				if (window.hasFocus())
				{
					if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();
				}
				else if (event.type == sf::Event::Closed) window.close();

				//SCROLLING CHANGES MASS
				if (event.type == sf::Event::MouseWheelMoved)
				{
					massSlider->setValue(massSlider->getValue() + 5*pow(event.mouseWheel.delta,3));
				}

				//HIDES UI
				if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::H))
				{
					if(showGUI == true) showGUI = false;
					else showGUI = true;
				}

				//LOCK CAMERA TO OBJECT
				if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					lockToObject(window, view1);
				}

				//EXPLODE OBJECT FUNCTION
				if (event.type == sf::Event::MouseButtonPressed && functions->getSelectedItem() == "Explode object (C)" && !mouseOnWidgets)
				{
					sf::Vector2f mPos(window.mapPixelToCoords(mousePos, view1));
					for (size_t i = 0; i < pListe.size(); i++)
					{
						if (range(mPos.x, mPos.y, pListe[i].getx(), pListe[i].gety()) < pListe[i].getRad())
						{
							explodePlanet(i);
						}
					}
				}

				//FOCUSING ON A NEW OBJECT
				if (event.type == sf::Event::MouseButtonPressed && functions->getSelectedItem() == "Info (I)" && !mouseOnWidgets && sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					bool fantP = false;
					sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
					for (size_t i = 0; i < pListe.size(); i++)
					{
						if (range(localPosition.x, localPosition.y, pListe[i].getx(), pListe[i].gety()) < pListe[i].getRad())
						{
							if (fokusId != pListe[i].getId())
							{
								trlListe.clear();
								fokusId = pListe[i].getId();
							}
							fantP = true;
							break;
						}
						if (!fantP)
						{
							fokusId = -1;
							trlListe.clear();
						}
					}

				}

				//ADVANCED IN ORBIT ADDER
				if (event.type == sf::Event::MouseButtonPressed && functions->getSelectedItem() == "Adv Object in orbit (S)" && !mouseOnWidgets && sf::Mouse::isButtonPressed(sf::Mouse::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
				{
					//SEARCHING FOR PLANET
					sf::Vector2f mPos(window.mapPixelToCoords(mousePos, view1));
					for (size_t i = 0; i < pListe.size(); i++)
					{
						if (range(pListe[i].getx(), pListe[i].gety(), mPos.x, mPos.y) < pListe[i].getRad())
						{

							//CHECKING IF IT ALREADY IS IN THE LIST
							bool alreadyIn = false;
							for (size_t q = 0; q < midlPListe.size(); q++)
							{
								if (midlPListe[q] == pListe[i].getId())
								{
									auto it = midlPListe.begin() + q;
									*it = std::move(midlPListe.back());
									midlPListe.pop_back();
									alreadyIn = true;
									break;
								}

							}

							if (!alreadyIn) midlPListe.push_back(pListe[i].getId());
							break;

						}
					}
				}

			}


		//FUNCTIONS
		if (window.hasFocus())
			{

				//MOVING WINDOW
				if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && transToggle && lockToObjectId == -2)
				{
					sf::Vector2i midl = sf::Mouse::getPosition(window);
					sf::Vector2i localPosition(window.mapPixelToCoords(midl, view1));
					xs = localPosition.x;
					ys = localPosition.y;
					transToggle = false;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !transToggle && lockToObjectId == -2)
				{

					sf::Vector2i midl = sf::Mouse::getPosition(window);
					sf::Vector2i localPosition(window.mapPixelToCoords(midl, view1));
					xmidltrans = -localPosition.x + xs;
					ymidltrans = -localPosition.y + ys;

				}
				else if (!transToggle)
				{
					xtrans += xmidltrans;
					ytrans += ymidltrans;

					transToggle = true;
					view1.move(xmidltrans, ymidltrans);
					xmidltrans = 0;
					ymidltrans = 0;
					window.setView(view1);
				}

				//TRACKING OBJECT
				if (findPlanet(lockToObjectId).getmass() == -1 && lockToObjectId >= 0) lockToObjectId = -2;
				if (!ship.isExist() && lockToObjectId == -1) lockToObjectId = -2;
				if (lockToObjectId >= 0)
				{
					view1.setCenter(findPlanet(lockToObjectId).getx(), findPlanet(lockToObjectId).gety()), window.setView(view1);
					xtrans = findPlanet(lockToObjectId).getx();
					ytrans = findPlanet(lockToObjectId).gety();
					xmidltrans = ymidltrans = 0;
				}
				if (lockToObjectId == -1)
				{
					view1.setCenter(ship.getpos()), window.setView(view1);
					xtrans = ship.getpos().x;
					ytrans = ship.getpos().y;
					xmidltrans = ymidltrans = 0;
				}

				//ADDING PLANET
				if (functions->getSelectedItem() == "Object (F)" && !mouseOnWidgets)
				{
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						if (!mouseToggle)
						{
							mouseToggle = true;

							sf::Vector2i midl = sf::Mouse::getPosition(window);
							midl.x -= xtrans / zoom;
							midl.y -= ytrans / zoom;
							sf::Vector2i localPosition(window.mapPixelToCoords(midl, view1));
							xx = localPosition.x;
							yy = localPosition.y;
						}
						if (mouseToggle)
						{
							sf::Vector2i midl = sf::Mouse::getPosition(window);
							midl.x -= xtrans / zoom;
							midl.y -= ytrans / zoom;

							sf::Vector2i localPosition(window.mapPixelToCoords(midl, view1));


							//PLANET CIRCLE
							Planet R(size, (xx + xtrans), (yy + ytrans));
							sf::CircleShape midlCircle(R.getRad());
							midlCircle.setOrigin(R.getRad(), R.getRad());
							midlCircle.setPosition(sf::Vector2f(R.getx(), R.gety()));
							midlCircle.setFillColor(sf::Color(255,0,0,100));
							window.draw(midlCircle);

							//VELOCITY VECTOR
							sf::Vertex line[] =
							{
								sf::Vertex(sf::Vector2f(xx + xtrans, yy + ytrans),sf::Color::Red),
								sf::Vertex(sf::Vector2f((2 * xx - localPosition.x + xtrans), 2 * yy - localPosition.y + ytrans), sf::Color::Red)
							};

							//INFO TEXT
							sf::Text t2;
							t2.setScale(zoom, zoom);
							t2.setString("Mass: " + convertDoubleToString(size) + "\nSpeed: " + convertDoubleToString(createPlanetSpeedmult* sqrt((xx - localPosition.x)* (xx - localPosition.x) + (yy - localPosition.y)* (yy - localPosition.y))));
							t2.setPosition(xx + xtrans + 10, yy + ytrans);
							t2.setColor(sf::Color::Red);
							t2.setFont(font);
							t2.setCharacterSize(10);
							window.draw(t2);

							window.draw(line, 2, sf::Lines);
						}
					}
					else if (mouseToggle)
					{
						mouseToggle = false;
						sf::Vector2i midl = sf::Mouse::getPosition(window);
						midl.x -= xtrans / zoom;
						midl.y -= ytrans / zoom;
						sf::Vector2i localPos(window.mapPixelToCoords(midl, view1));
						Planet R(size, (xx + xtrans), (yy + ytrans), createPlanetSpeedmult* (xx - localPos.x), createPlanetSpeedmult*(yy - localPos.y));
						addPlanet(R);
					}
				}

				//ADDING PLANET IN ORBIT
				if (functions->getSelectedItem() == "Object in orbit (O)" && !mouseOnWidgets)
				{
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && createInOrbitCounter == 0)
					{

						for (size_t i = 0; i < pListe.size(); i++)
						{
							sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));

							if (range(pListe[i].getx(), pListe[i].gety(), localPosition.x, localPosition.y) < pListe[i].getRad())
							{
								planetFuncId = pListe[i].getId();
								createInOrbitCounter = 1;
								break;
							}
						}
					}
					else if (createInOrbitCounter == 1)
					{
						sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						double rad = sqrt((new_mouse_pos.x - findPlanet(planetFuncId).getx())*(new_mouse_pos.x - findPlanet(planetFuncId).getx()) + (new_mouse_pos.y - findPlanet(planetFuncId).gety())*(new_mouse_pos.y - findPlanet(planetFuncId).gety()));
						Planet midlP(size);

						//GOLDILOCK-ZONE
						if (findPlanet(planetFuncId).getType() == SMALLSTAR || findPlanet(planetFuncId).getType() == STAR || findPlanet(planetFuncId).getType() == BIGSTAR)
						{
							double goldilock_inner_rad = (tempConstTwo * findPlanet(planetFuncId).getRad() * findPlanet(planetFuncId).getRad() * findPlanet(planetFuncId).getTemp()) / inner_goldi_temp;
							double goldilock_outer_rad = (tempConstTwo * findPlanet(planetFuncId).getRad() * findPlanet(planetFuncId).getRad() * findPlanet(planetFuncId).getTemp()) / outer_goldi_temp;

							sf::CircleShape g(goldilock_inner_rad);
							g.setPointCount(60);
							g.setPosition(sf::Vector2f(findPlanet(planetFuncId).getx(), findPlanet(planetFuncId).gety()));
							g.setOrigin(goldilock_inner_rad, goldilock_inner_rad);
							g.setOutlineThickness(goldilock_outer_rad - goldilock_inner_rad);
							g.setFillColor(sf::Color(0, 0, 0, 0));
							g.setOutlineColor(sf::Color(0, 200, 0, goldi_strength));
							window.draw(g);
						}

						//DRAWING ORBIT
						sf::CircleShape omr(rad);
						omr.setPosition(sf::Vector2f(findPlanet(planetFuncId).getx(), findPlanet(planetFuncId).gety()));
						omr.setOrigin(rad, rad);
						omr.setFillColor(sf::Color(0, 0, 0, 0));
						omr.setOutlineColor(sf::Color::Red);
						omr.setOutlineThickness(1 * zoom);
						window.draw(omr);

						//DRAWING MASS CENTER
						sf::CircleShape midtpunkt(2);
						midtpunkt.setOrigin(2, 2);
						Planet annen = findPlanet(planetFuncId);
						midtpunkt.setFillColor(sf::Color(255, 0, 0));

						double avst = sqrt((new_mouse_pos.x - annen.getx())*(new_mouse_pos.x - annen.getx()) + (new_mouse_pos.y - annen.gety())*(new_mouse_pos.y - annen.gety()));
						avst = avst*(midlP.getmass()) / (midlP.getmass() + annen.getmass());
						double angleb = atan2(new_mouse_pos.y - annen.gety(), new_mouse_pos.x - annen.getx());

						midtpunkt.setPosition(annen.getx() + avst*cos(angleb), annen.gety() + avst*sin(angleb));
						window.draw(midtpunkt);

						//DRAWING ROCHE LIMIT
						if (size > MINIMUMBREAKUPSIZE && size / findPlanet(planetFuncId).getmass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
						{
							double rocheRad = ROCHE_LIMIT_DIST_MULTIPLIER*(midlP.getRad() + findPlanet(planetFuncId).getRad());

							sf::CircleShape omr(rocheRad);
							omr.setPosition(sf::Vector2f(findPlanet(planetFuncId).getx(), findPlanet(planetFuncId).gety()));
							omr.setOrigin(rocheRad, rocheRad);
							omr.setFillColor(sf::Color(0, 0, 0, 0));
							omr.setOutlineColor(sf::Color(255, 140, 0));
							omr.setOutlineThickness(1 * zoom);
							window.draw(omr);

						}

						//DRAWING PLANET
						sf::CircleShape bump(midlP.getRad());
						bump.setOrigin(midlP.getRad(), midlP.getRad());
						bump.setFillColor(sf::Color::Red);
						bump.setPosition(new_mouse_pos);
						window.draw(bump);


						if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
						{

							double hast = sqrt(G*(findPlanet(planetFuncId).getmass() + size) / rad);
							double angle = atan2(new_mouse_pos.y - findPlanet(planetFuncId).gety(), new_mouse_pos.x - findPlanet(planetFuncId).getx());
							double fixhast = size*hast / (size + findPlanet(planetFuncId).getmass());

							findPlanetRef(planetFuncId).getxv() -= fixhast*cos(angle + 1.507);
							findPlanetRef(planetFuncId).getyv() -= fixhast*sin(angle + 1.507);

							addPlanet(Planet(size, findPlanet(planetFuncId).getx() + rad*cos(angle), findPlanet(planetFuncId).gety() + rad*sin(angle), (findPlanet(planetFuncId).getxv() + hast*cos(angle + 1.507)), (findPlanet(planetFuncId).getyv() + hast*sin(angle + 1.507))));
							createInOrbitCounter = 0;
						}


					}
					else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::F))
					{
						createInOrbitCounter = 0;
					}
				}

				//REMOVING PLANET
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && functions->getSelectedItem() == "Remove object (D)" && !mouseOnWidgets)
				{
					sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
					for (size_t i = 0; i < pListe.size(); i++)
					{
						if (range(pListe[i].getx(), pListe[i].gety(), localPosition.x, localPosition.y) < pListe[i].getRad())
						{

							addExplosion(sf::Vector2f(pListe[i].getx(), pListe[i].gety()), 2 * pListe[i].getRad(), sf::Vector2f(pListe[i].getxv(), pListe[i].getyv()), pListe[i].getmass() / 2);
							removePlanet(i);
							break;
						}
					}
				}

				//RESETTING SIMULATION
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
				{
					clear(view1,window);
					window.setView(view1);
				}

				//SPAWNING SHIP
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && functions->getSelectedItem() == "Spawn ship (E)" && !mouseOnWidgets && lockToObjectId != -1)
				{
					ship.reset(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
				}

				//ADDING RINGS
				if (functions->getSelectedItem() == "Rings (Q)" && !mouseOnWidgets)
				{
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && ringPlanetCounter == 0)
					{
						if (pListe.size() > 0)
						{
							sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
							for (size_t i = 0; i < pListe.size(); i++)
							{
								if (range(pListe[i].getx(), pListe[i].gety(), localPosition.x, localPosition.y) < pListe[i].getRad())
								{
									planetFuncId = pListe[i].getId();
									ringPlanetCounter = 1;
									break;
								}
							}
						}
					}
					else if (ringPlanetCounter == 1 && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						ringPlanetCounter = 2;
					}
					else if (ringPlanetCounter == 2 && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						double rad = sqrt((new_mouse_pos.x - findPlanet(planetFuncId).getx())*(new_mouse_pos.x - findPlanet(planetFuncId).getx()) + (new_mouse_pos.y - findPlanet(planetFuncId).gety())*(new_mouse_pos.y - findPlanet(planetFuncId).gety()));

						sf::CircleShape omr(rad);
						omr.setPosition(sf::Vector2f(findPlanet(planetFuncId).getx(), findPlanet(planetFuncId).gety()));
						omr.setOrigin(rad, rad);
						omr.setFillColor(sf::Color(0, 0, 0, 0));
						omr.setOutlineColor(sf::Color::Red);
						omr.setOutlineThickness(1 * zoom);
						window.draw(omr);
					}
					else if (ringPlanetCounter == 2 && sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						double rad = sqrt((new_mouse_pos.x - findPlanet(planetFuncId).getx())*(new_mouse_pos.x - findPlanet(planetFuncId).getx()) + (new_mouse_pos.y - findPlanet(planetFuncId).gety())*(new_mouse_pos.y - findPlanet(planetFuncId).gety()));

						ringInnerRad = rad;
						ringPlanetCounter = 3;
					}
					else if (ringPlanetCounter == 3 && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						ringPlanetCounter = 4;
					}
					else if ((ringPlanetCounter == 4 && !sf::Mouse::isButtonPressed(sf::Mouse::Left)))
					{
						sf::CircleShape omr(ringInnerRad);
						omr.setPosition(sf::Vector2f(findPlanet(planetFuncId).getx(), findPlanet(planetFuncId).gety()));
						omr.setOrigin(ringInnerRad, ringInnerRad);
						omr.setFillColor(sf::Color(0, 0, 0, 0));
						omr.setOutlineColor(sf::Color::Red);
						omr.setOutlineThickness(1 * zoom);
						window.draw(omr);

						sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						double rad2 = sqrt((new_mouse_pos.x - findPlanet(planetFuncId).getx())*(new_mouse_pos.x - findPlanet(planetFuncId).getx()) + (new_mouse_pos.y - findPlanet(planetFuncId).gety())*(new_mouse_pos.y - findPlanet(planetFuncId).gety()));

						sf::CircleShape omr2(rad2);
						omr2.setPosition(sf::Vector2f(findPlanet(planetFuncId).getx(), findPlanet(planetFuncId).gety()));
						omr2.setOrigin(rad2, rad2);
						omr2.setFillColor(sf::Color(0, 0, 0, 0));
						omr2.setOutlineColor(sf::Color::Red);
						omr2.setOutlineThickness(1 * zoom);
						window.draw(omr2);
					}
					else if (ringPlanetCounter == 4 && sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						ringOuterRad = sqrt((new_mouse_pos.x - findPlanet(planetFuncId).getx())*(new_mouse_pos.x - findPlanet(planetFuncId).getx()) + (new_mouse_pos.y - findPlanet(planetFuncId).gety())*(new_mouse_pos.y - findPlanet(planetFuncId).gety()));
						giveRings(findPlanet(planetFuncId), ringInnerRad, ringOuterRad);
						ringPlanetCounter = 0;
					}
				}
				else
				{
					ringPlanetCounter = 0;
				}

				//RANDOM GENERATED SYSTEM
				if (functions->getSelectedItem() == "Random system (G)" && !mouseOnWidgets)
				{
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !randomToggle && !randomToggle2)
					{
						mouse_rand_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						randomToggle = true;
						randomToggle2 = true;
					}
					else if (randomToggle && sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						double rad = sqrt((new_mouse_pos.x - mouse_rand_pos.x)*(new_mouse_pos.x - mouse_rand_pos.x) + (new_mouse_pos.y - mouse_rand_pos.y)*(new_mouse_pos.y - mouse_rand_pos.y));

						sf::CircleShape omr(rad);
						omr.setPosition(mouse_rand_pos);
						omr.setOrigin(rad, rad);
						omr.setFillColor(sf::Color(0, 0, 0, 0));
						omr.setOutlineColor(sf::Color::Red);
						omr.setOutlineThickness(1 * zoom);
						omr.setPointCount(100);
						window.draw(omr);

						sf::Vertex line[] =
						{
							sf::Vertex(mouse_rand_pos,sf::Color::Red),
							sf::Vertex(new_mouse_pos, sf::Color::Red)
						};

						sf::Text t;
						t.setScale(zoom, zoom);
						t.setString("Planets: " + convertDoubleToString((int)((NUMBER_OF_OBJECT_MULTIPLIER*rad) + 1)) + "\nMass: ca " + convertDoubleToString(MASS_MULTIPLIER * cbrt(rad)) + "\nRadius: " + convertDoubleToString(rad));
						t.setPosition(mouse_rand_pos.x + rad + 10, mouse_rand_pos.y);
						t.setColor(sf::Color::Red);
						t.setFont(font);
						t.setCharacterSize(10);

						window.draw(line, 2, sf::Lines);
						window.draw(t);
						randomToggle2 = false;
					}
					else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && randomToggle && !randomToggle2)
					{
						sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
						double rad = sqrt((new_mouse_pos.x - mouse_rand_pos.x)*(new_mouse_pos.x - mouse_rand_pos.x) + (new_mouse_pos.y - mouse_rand_pos.y)*(new_mouse_pos.y - mouse_rand_pos.y));
						randomPlaneter(MASS_MULTIPLIER * cbrt(rad), NUMBER_OF_OBJECT_MULTIPLIER*rad, rad, mouse_rand_pos);
						randomToggle = false;
					}
					else
					{
						randomToggle = false;
					}
				}

				//DEALING WITH THE SPACESHIP
				romskipHandling();

				//ADV IN ORBIT ADDER
				if (functions->getSelectedItem() == "Adv Object in orbit (S)" && !mouseOnWidgets && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
				{

					sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
					sf::Vector3f massCenterInfoVector(centerOfMass(midlPListe));
					sf::Vector2f massCenterVelocity = centerOfMassVelocity(midlPListe);
					double rad = range(new_mouse_pos.x, new_mouse_pos.y, massCenterInfoVector.x, massCenterInfoVector.y);
					Planet midlP(size);

					//DRAWING NEW PLANET AND ORBIT
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						//DRAWING ORBIT
						sf::CircleShape omr(rad);
						omr.setPosition(sf::Vector2f(massCenterInfoVector.x, massCenterInfoVector.y));
						omr.setOrigin(rad, rad);
						omr.setFillColor(sf::Color(0, 0, 0, 0));
						omr.setOutlineColor(sf::Color::Red);
						omr.setOutlineThickness(1 * zoom);
						window.draw(omr);

						//DRAWING PLANET
						sf::CircleShape bump(midlP.getRad());
						bump.setOrigin(midlP.getRad(), midlP.getRad());
						bump.setFillColor(sf::Color::Red);
						bump.setPosition(new_mouse_pos);
						window.draw(bump);

						advCreateInOrbitCounter = 1;
					}

					//ADDING NEW PLANET
					if (advCreateInOrbitCounter == 1 && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{

						double hast = sqrt(G*(massCenterInfoVector.z + size) / rad);
						double angle = atan2(new_mouse_pos.y - massCenterInfoVector.y, new_mouse_pos.x - massCenterInfoVector.x);
						double fixhast = size*hast / (size + massCenterInfoVector.z);
						addPlanet(Planet(size, massCenterInfoVector.x + rad*cos(angle), massCenterInfoVector.y + rad*sin(angle), (massCenterVelocity.x + hast*cos(angle + 1.507)- fixhast*cos(angle + 1.507)), (massCenterVelocity.y + hast*sin(angle + 1.507)- fixhast*sin(angle + 1.507))));
						
						for (int i = 0; i < midlPListe.size(); i++)
						{
							if (findPlanet(midlPListe[i]).getmass() != -1)
							{
								findPlanetRef(midlPListe[i]).getxv() -= fixhast*cos(angle + 1.507);
								findPlanetRef(midlPListe[i]).getyv() -= fixhast*sin(angle + 1.507);
							}
						}

						advCreateInOrbitCounter = 0;
					}

				}
				else
				{
					advCreateInOrbitCounter = 0;
				}

				//BOUNDS
				if (!autoBound->isChecked())
				{
					if (functions->getSelectedItem() == "Bound (B)" && !mouseOnWidgets)
					{
						if (bound.getState() && sf::Mouse::isButtonPressed(sf::Mouse::Left))
						{
							bound.setState(false);
							boundCounter = -1;
						}
						else
						{
							if (boundCounter == 0 && sf::Mouse::isButtonPressed(sf::Mouse::Left))
							{
								sf::Vector2f mPos(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
								bound.setPos(mPos);
								boundCounter++;
							}
							else if (boundCounter == 1 && sf::Mouse::isButtonPressed(sf::Mouse::Left))
							{
								sf::Vector2f mPos(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
								double midlRad = range(mPos.x, mPos.y, bound.getPos().x, bound.getPos().y);
								bound.setRad(midlRad);
								if (bound.getRad() > BOUND_MIN_RAD) bound.draw(window, xmidltrans, ymidltrans, zoom);
							}
							else if (boundCounter == 1)
							{
								if (bound.getRad() > BOUND_MIN_RAD) bound.setState(true);
								boundCounter = 0;
							}
							else boundCounter = 0;
						}
					}
					else
					{
						boundCounter = 0;
					}
				}
			}
		
		//PRINTING TO WINDOW
		PlanetSkjermPrint(window);
		effectSkjermPrint(window);
		ship.draw(window, xmidltrans, ymidltrans);
		lightSkjermPrint(window);
		if (functions->getSelectedItem() != "Adv Object in orbit (S)")
		{
			midlPListe.clear();
		}
		else
		{

			//SELECTED PLANETS
			for (size_t i = 0; i < midlPListe.size(); i++)
			{
				Planet p = findPlanet(midlPListe[i]);
				if (p.getmass() != -1)
				{
					sf::Vector2f pos(p.getx() - xmidltrans, p.gety() - ymidltrans);
					double rad = ADV_ORBIT_ADDER_SELECTED_MARKER_RAD_MULT*p.getRad();
					sf::CircleShape circ(rad);
					circ.setPosition(pos);
					circ.setOrigin(rad, rad);
					circ.setOutlineColor(sf::Color::Red);
					circ.setOutlineThickness(ADV_ORBIT_ADDER_SELECTED_MARKER_THICKNESS*zoom);
					circ.setFillColor(sf::Color(0, 0, 0, 0));
					window.draw(circ);
				}
			}

			//CENTER OF MASS
			if (midlPListe.size() > 1)
			{
				sf::Vector3f p = centerOfMass(midlPListe);
				double rad = ADV_ORBIT_ADDER_CENTER_MARKER_RAD * zoom;
				sf::CircleShape s(rad);
				s.setOrigin(rad, rad);
				s.setPosition(p.x - xmidltrans, p.y - ymidltrans);
				s.setFillColor(sf::Color::Red);
				window.draw(s);
			}

		}
		if (bound.getState()) bound.draw(window,xmidltrans,ymidltrans, zoom);
		printInfoPlanet(window, view1);
		if (drawtext2 && findPlanet(fokusId).getmass() != -1) window.draw(text2);
		if (showGUI) gui.draw();


		//DRAWING WARNING IF MOUSE OUTSIDE BOUNDS
		if (!mouseOnWidgets && bound.getState() && bound.isOutside(sf::Vector2f(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1))))
		{
			sf::Vector2f mousePos(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));

			sf::Vertex v[] =
			{
				sf::Vertex(sf::Vector2f(mousePos.x - BOUND_OUTSIDE_INDICATOR_SIZE * zoom, mousePos.y + BOUND_OUTSIDE_INDICATOR_SIZE * zoom),sf::Color::Red),
				sf::Vertex(sf::Vector2f(mousePos.x + BOUND_OUTSIDE_INDICATOR_SIZE * zoom, mousePos.y - BOUND_OUTSIDE_INDICATOR_SIZE * zoom),sf::Color::Red)
			};
			window.draw(v, 2, sf::Lines);

			sf::Vertex q[] =
			{
				sf::Vertex(sf::Vector2f(mousePos.x + BOUND_OUTSIDE_INDICATOR_SIZE * zoom, mousePos.y + BOUND_OUTSIDE_INDICATOR_SIZE * zoom),sf::Color::Red),
				sf::Vertex(sf::Vector2f(mousePos.x - BOUND_OUTSIDE_INDICATOR_SIZE * zoom, mousePos.y - BOUND_OUTSIDE_INDICATOR_SIZE * zoom),sf::Color::Red)
			};
			window.draw(q, 2, sf::Lines);
		}

		window.setView(view1);
		window.display();

		if (timeStep != 0) update();

		//FRAMERATE OUT
		sf::Time time = clock.getElapsedTime();
		if (iterasjon % FRAMERATE_CHECK_DELTAFRAME == 0) fps = 1.0f / time.asSeconds();
		clock.restart().asSeconds();
		}
}
