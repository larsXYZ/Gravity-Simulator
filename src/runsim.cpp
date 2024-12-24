#include "space.h"

#include "user_functions.h"

void Space::runSim()
{
	sf::RenderWindow window;

	window.create(sf::VideoMode(xsize, ysize), "Gravity Simulator", 
		fullScreen ? sf::Style::Fullscreen : sf::Style::Default);

	sf::View mainView;
	window.setFramerateLimit(framerate);
	mainView.setSize(xsize, ysize);
	mainView.setCenter(0, 0);
	window.setView(mainView);
	sf::Clock clock;

	font.loadFromFile("sansation.ttf");
	text.setFont(font);
	text.setCharacterSize(14);
	text.setColor(sf::Color::White);

	//LOADING GUI
	tgui::Gui gui{ window };
	gui.setFont("sansation.ttf");
	initSetup();
	gui.add(simInfo);
	gui.add(functions);
	gui.add(newPlanetInfo);
	gui.add(massSlider);
	gui.add(timeStepSlider);
	gui.add(temperatureUnitSelector);
	gui.add(currPlanetInfo);
	gui.add(massExistingObjectSlider);
	gui.add(autoBound);

	while (window.isOpen())
	{
		window.clear(sf::Color::Black);

		setInfo();

		hotkeys(window, mainView);

		while(window.pollEvent(event))
		{
			if (!object_tracker.is_active())
				click_and_drag_handler.update(mainView, window, event);

			gui.handleEvent(event);
		}

		if (object_tracker.is_active())
		{
			object_tracker.update(*this, mainView);
			window.setView(mainView);
		}

		window.setView(mainView);

		FunctionContext context
		{
			.type = getSelectedFunction(functions),
			.space = *this,
			.spaceship = ship,
			.view = mainView,
			.window = window,
			.mouse_pos_window = sf::Mouse::getPosition(window),
			.mouse_pos_world = window.mapPixelToCoords(sf::Mouse::getPosition(window), mainView),
			.mass_slider = massSlider,
			.new_object_info = newPlanetInfo
		};
		executeFunction(context);


		drawPlanets(window);
		drawEffects(window);
		ship.draw(window);
		drawLightEffects(window);
		
		if (bound.isActive())
			bound.render(window);

		if (showGUI)
			gui.draw();

		window.display();

		if (timeStep != 0) 
			update();

		sf::Time time = clock.getElapsedTime();

		if (iteration % FRAMERATE_CHECK_DELTAFRAME == 0) 
			fps = 1.0f / time.asSeconds();

		clock.restart().asSeconds();
		}
}
