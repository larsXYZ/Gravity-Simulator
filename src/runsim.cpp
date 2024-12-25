#include "space.h"

#include "user_functions.h"

void Space::runSim(sf::Vector2i window_size, bool fullscreen)
{
	sf::RenderWindow window;

	window.create(sf::VideoMode(window_size.x, window_size.y), "Gravity Simulator",
		fullscreen ? sf::Style::Fullscreen : sf::Style::Default);

	sf::View mainView;
	window.setFramerateLimit(FRAMERATE);
	mainView.setSize(window_size.x, window_size.y);
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
	gui.add(autoBound);

	while (window.isOpen())
	{
		window.clear(sf::Color::Black);

		timestep = timeStepSlider->getValue();

		while(window.pollEvent(event))
		{
			hotkeys(window, mainView);

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

		updateInfoBox();

		drawPlanets(window);
		drawEffects(window);
		ship.draw(window);
		drawLightEffects(window);
		
		if (bound.isActive())
			bound.render(window);

		if (object_info.is_active())
			object_info.render(*this, window);

		if (showGUI)
			gui.draw();

		window.display();

		if (timestep != 0) 
			update();

		sf::Time time = clock.getElapsedTime();

		if (iteration % FRAMERATE_CHECK_DELTAFRAME == 0) 
			fps = 1.0f / time.asSeconds();

		clock.restart().asSeconds();
	}
}
