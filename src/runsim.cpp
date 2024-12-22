#include "space.h"

void Space::runSim()
{
	sf::RenderWindow window;

	if (fullScreen)
		window.create(sf::VideoMode(xsize, ysize), "Gravity Simulator", sf::Style::Fullscreen);
	else
		window.create(sf::VideoMode(xsize, ysize), "Gravity Simulator", sf::Style::Default);

	sf::View view1;
	window.setFramerateLimit(framerate);
	view1.setSize(xsize, ysize);
	view1.setCenter(0, 0);
	window.setView(view1);
	sf::Clock clock;

	font.loadFromFile("sansation.ttf");
	text.setFont(font);
	text.setCharacterSize(14);
	text2.setFont(font);
	text.setColor(sf::Color::White);
	text2.setColor(sf::Color::White);

	//LOADING GUI
	tgui::Gui gui{ window };
	gui.setFont("sansation.ttf");
	initSetup();
	gui.add(simInfo);
	gui.add(functions);
	gui.add(newPlanetInfo);
	gui.add(massSlider);
	gui.add(timeStepSlider);
	gui.add(tempChooser);
	gui.add(currPlanetInfo);
	gui.add(massExistingObjectSlider);
	gui.add(autoBound);

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

	double ringInnerRad = 0;
	double ringOuterRad;
	int createInOrbitCounter = 0;
	int advCreateInOrbitCounter = 0;
	bool buttonPressed = false;
	bool buttonPressed2 = false;

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
			if (gui.getWidgets()[i]->isMouseOnWidget(sf::Vector2f(mousePos.x,mousePos.y)))
				mouseOnWidgets = true;
		}
		if (!buttonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseOnWidgets) { buttonPressed = true; }
		else if (buttonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left)) { buttonPressed = true, mouseOnWidgets = true;}
		else buttonPressed = false;

		mouseOnMassSliderSelected = false;
		if (massExistingObjectSlider->isMouseOnWidget(sf::Vector2f{ (float)mousePos.x,(float)mousePos.y })) mouseOnMassSliderSelected = true;
		if (!buttonPressed2 && sf::Mouse::isButtonPressed(sf::Mouse::Left) && mouseOnMassSliderSelected) { buttonPressed2 = true; }
		else if (buttonPressed2 && sf::Mouse::isButtonPressed(sf::Mouse::Left)) { buttonPressed2 = true, mouseOnMassSliderSelected = true; }
		else buttonPressed2 = false;

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
			if (window.hasFocus() && event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();
			else if (event.type == sf::Event::Closed) 
				window.close();

			//SCROLLING CHANGES MASS
			if (event.type == sf::Event::MouseWheelMoved)
			{
				auto delta_zoom = 1.15 * event.mouseWheel.delta / std::abs(event.mouseWheel.delta);
				if (delta_zoom < 0)
				{
					delta_zoom = std::abs(delta_zoom);
					zoom *= delta_zoom;
					view1.zoom(delta_zoom);
				}
				else if (delta_zoom > 0)
				{
					delta_zoom = std::abs(delta_zoom);
					zoom /= delta_zoom;
					view1.zoom(1/delta_zoom);
				}
			}

			//HIDES UI
			if (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::H))
				showGUI = !showGUI;

			//LOCK CAMERA TO OBJECT
			if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
				lockToObject(window, view1);

			//EXPLODE OBJECT FUNCTION
			if (event.type == sf::Event::MouseButtonPressed && getSelectedFunction(functions) == FunctionType::EXPLODE_OBJECT && !mouseOnWidgets)
			{
				sf::Vector2f mPos(window.mapPixelToCoords(mousePos, view1));
				for (size_t i = 0; i < planets.size(); i++)
				{
					const auto dist = std::hypot(mPos.x - planets[i].getx(), mPos.y - planets[i].gety());
					if (dist < planets[i].getRad())
						explodePlanet(planets[i]);
				}
			}

			//FOCUSING ON A NEW OBJECT
			if (event.type == sf::Event::MouseButtonPressed && getSelectedFunction(functions) == FunctionType::SHOW_INFO && !mouseOnWidgets && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				bool found = false;
				sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
				for (size_t i = 0; i < planets.size(); i++)
				{
					const auto dist = std::hypot(localPosition.x - planets[i].getx(), localPosition.y - planets[i].gety());
					if (dist < planets[i].getRad())
					{
						if (fokusId != planets[i].getId())
						{
							trail.clear();
							fokusId = planets[i].getId();
						}
						found = true;
						break;
					}
					if (!found)
					{
						fokusId = -1;
						trail.clear();
					}
				}
			}

			//ADVANCED IN ORBIT ADDER
			if (event.type == sf::Event::MouseButtonPressed && getSelectedFunction(functions) == FunctionType::ADVANCED_OBJECT_IN_ORBIT 
				&& !mouseOnWidgets && sf::Mouse::isButtonPressed(sf::Mouse::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
			{
				//SEARCHING FOR PLANET
				sf::Vector2f mPos(window.mapPixelToCoords(mousePos, view1));
				for (size_t i = 0; i < planets.size(); i++)
				{
					const auto dist = std::hypot(planets[i].getx() - mPos.x, planets[i].gety() - mPos.y);
					if (dist < planets[i].getRad())
					{

						//CHECKING IF IT ALREADY IS IN THE LIST
						bool alreadyIn = false;
						for (size_t q = 0; q < temp_planet_ids.size(); q++)
						{
							if (temp_planet_ids[q] == planets[i].getId())
							{
								auto it = temp_planet_ids.begin() + q;
								*it = std::move(temp_planet_ids.back());
								temp_planet_ids.pop_back();
								alreadyIn = true;
								break;
							}
						}

						if (!alreadyIn) temp_planet_ids.push_back(planets[i].getId());
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
			if (getSelectedFunction(functions) == FunctionType::NEW_OBJECT && !mouseOnWidgets)
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
					addPlanet(std::move(R));
				}
			}

			//ADDING PLANET IN ORBIT
			if (getSelectedFunction(functions) == FunctionType::OBJECT_IN_ORBIT && !mouseOnWidgets)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && createInOrbitCounter == 0)
				{

					for (size_t i = 0; i < planets.size(); i++)
					{
						sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));

						const auto dist = std::hypot(planets[i].getx() - localPosition.x, planets[i].gety() - localPosition.y);
						if (dist < planets[i].getRad())
						{
							planetFuncId = planets[i].getId();
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
					sf::CircleShape center_point(2);
					center_point.setOrigin(2, 2);
					Planet other_planet = findPlanet(planetFuncId);
					center_point.setFillColor(sf::Color(255, 0, 0));

					double avst = sqrt((new_mouse_pos.x - other_planet.getx())*(new_mouse_pos.x - other_planet.getx()) + (new_mouse_pos.y - other_planet.gety())*(new_mouse_pos.y - other_planet.gety()));
					avst = avst*(midlP.getmass()) / (midlP.getmass() + other_planet.getmass());
					double angleb = atan2(new_mouse_pos.y - other_planet.gety(), new_mouse_pos.x - other_planet.getx());

					center_point.setPosition(other_planet.getx() + avst*cos(angleb), other_planet.gety() + avst*sin(angleb));
					window.draw(center_point);

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

						if (Planet* ptr = findPlanetPtr(planetFuncId))
						{
							auto& planet = *ptr;
							planet.setxv(planet.getxv() - fixhast * cos(angle + 1.507));
							planet.setyv(planet.getyv() - fixhast * sin(angle + 1.507));
						}

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
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && getSelectedFunction(functions) == FunctionType::REMOVE_OBJECT && !mouseOnWidgets)
			{
				sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
				for (size_t i = 0; i < planets.size(); i++)
				{
					const auto dist = std::hypot(planets[i].getx() - localPosition.x, planets[i].gety() - localPosition.y);
					if (dist < planets[i].getRad())
					{
						removePlanet(planets[i].getId());
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
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && getSelectedFunction(functions) == FunctionType::SPAWN_SHIP && !mouseOnWidgets && lockToObjectId != -1)
			{
				ship.reset(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
			}

			//ADDING RINGS
			if (getSelectedFunction(functions) == FunctionType::ADD_RINGS && !mouseOnWidgets)
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && ringPlanetCounter == 0)
				{
					if (planets.size() > 0)
					{
						sf::Vector2i localPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view1));
						for (size_t i = 0; i < planets.size(); i++)
						{
							const auto dist = std::hypot(planets[i].getx() - localPosition.x, planets[i].gety() - localPosition.y);
							if (dist < planets[i].getRad())
							{
								planetFuncId = planets[i].getId();
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
			if (getSelectedFunction(functions) == FunctionType::RANDOM_SYSTEM && !mouseOnWidgets)
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
					randomPlanets(MASS_MULTIPLIER * cbrt(rad), NUMBER_OF_OBJECT_MULTIPLIER*rad, rad, mouse_rand_pos);
					randomToggle = false;
				}
				else
				{
					randomToggle = false;
				}
			}

			//DEALING WITH THE SPACESHIP
			updateSpaceship();

			//ADV IN ORBIT ADDER
			if (getSelectedFunction(functions) == FunctionType::ADVANCED_OBJECT_IN_ORBIT && !mouseOnWidgets && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
			{

				sf::Vector2f new_mouse_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view1);
				sf::Vector3f massCenterInfoVector(centerOfMass(temp_planet_ids));
				sf::Vector2f massCenterVelocity = centerOfMassVelocity(temp_planet_ids);
				double rad = std::hypot(new_mouse_pos.x - massCenterInfoVector.x, new_mouse_pos.y - massCenterInfoVector.y);
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
						
					for (int i = 0; i < temp_planet_ids.size(); i++)
					{
						if (Planet* ptr = findPlanetPtr(planetFuncId))
						{
							auto& planet = *ptr;
							planet.setxv(planet.getxv() - fixhast * cos(angle + 1.507));
							planet.setyv(planet.getyv() - fixhast * sin(angle + 1.507));
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
				if (getSelectedFunction(functions) == FunctionType::ADD_BOUND && !mouseOnWidgets)
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
							double midlRad = std::hypot(mPos.x - bound.getPos().x, mPos.y - bound.getPos().y);
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
		drawPlanets(window);
		drawEffects(window);
		ship.draw(window, xmidltrans, ymidltrans);
		drawLightEffects(window);
		if (getSelectedFunction(functions) != FunctionType::ADVANCED_OBJECT_IN_ORBIT)
		{
			temp_planet_ids.clear();
		}
		else
		{

			//SELECTED PLANETS
			for (size_t i = 0; i < temp_planet_ids.size(); i++)
			{
				Planet p = findPlanet(temp_planet_ids[i]);
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
			if (temp_planet_ids.size() > 1)
			{
				sf::Vector3f p = centerOfMass(temp_planet_ids);
				double rad = ADV_ORBIT_ADDER_CENTER_MARKER_RAD * zoom;
				sf::CircleShape s(rad);
				s.setOrigin(rad, rad);
				s.setPosition(p.x - xmidltrans, p.y - ymidltrans);
				s.setFillColor(sf::Color::Red);
				window.draw(s);
			}

		}
		if (bound.getState()) bound.draw(window,xmidltrans,ymidltrans, zoom);
		drawPlanetInfo(window, view1);
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

		if (timeStep != 0) 
			update();

		//FRAMERATE OUT
		sf::Time time = clock.getElapsedTime();
		if (iteration % FRAMERATE_CHECK_DELTAFRAME == 0) fps = 1.0f / time.asSeconds();
		clock.restart().asSeconds();
		}
}
