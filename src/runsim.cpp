#include "space.h"

#include "user_functions.h"

bool is_mouse_on_widgets(const sf::RenderWindow & window, const tgui::Gui & gui)
{
    static bool started_on_gui = false;
    static bool mouse_was_pressed = false;

    bool mouse_currently_over_gui = false;
	for (const auto& gui_element : gui.getWidgets())
	{
		if (!gui_element->isVisible())
			continue;

		const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		if (gui_element->isMouseOnWidget(sf::Vector2f(mousePos.x, mousePos.y)))
        {
            mouse_currently_over_gui = true;
            break;
        }
	}

    bool mouse_pressed = sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right);
    
    if (mouse_pressed && !mouse_was_pressed)
    {
        started_on_gui = mouse_currently_over_gui;
    }
    
    if (!mouse_pressed)
    {
        started_on_gui = false;
    }

    mouse_was_pressed = mouse_pressed;

	return mouse_currently_over_gui || started_on_gui;
}

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

	//LOADING GUI
	tgui::Gui gui{ window };
	gui.setFont("sansation.ttf");
	initSetup();
	object_info.setup(*this, gui);
	gui.add(simInfo);
	gui.add(toolInfo);
	gui.add(functions);
	gui.add(newPlanetInfo);
	gui.add(objectTypeSelector);
	gui.add(massSlider);
	gui.add(timeStepLabel);
	gui.add(timeStepSlider);
	gui.add(temperatureUnitSelector);

	gui.add(debugMenu);

	gui.add(autoBound);
	autoBound->onUncheck([&]() {bound.setActiveState(false); });

	sf::Event event;
	while (window.isOpen())
	{
		window.clear(sf::Color::Black);

		timestep = paused ? 0.0 : timeStepSlider->getValue();
        is_mouse_on_gui = is_mouse_on_widgets(window, gui);
		
		FunctionContext context
		{
			.type = getSelectedFunction(functions),
			.space = *this,
			.spaceship = ship,
			.view = mainView,
			.window = window,
			.mouse_pos_window = sf::Mouse::getPosition(window),
			.mouse_pos_world = window.mapPixelToCoords(sf::Mouse::getPosition(window), mainView),
			.is_mouse_on_widgets = is_mouse_on_gui,
			.mass_slider = massSlider,
			.object_type_selector = objectTypeSelector,
			.new_object_info = newPlanetInfo,
			.bound = bound,
			.zoom = click_and_drag_handler.get_zoom()
		};

		while(window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (!object_info.is_focused())
				hotkeys(event, mainView, window);

			if (event.type == sf::Event::MouseWheelScrolled || !object_tracker.is_active())
				click_and_drag_handler.update(mainView, window, event);

			gui.handleEvent(event);

			giveFunctionEvent(context, event);
		}

		window.setView(mainView);
		
		executeFunction(context);
		flushPlanets();

		if (object_tracker.is_active())
			object_tracker.update(*this, mainView);

		updateInfoBox();

		drawPlanets(window);
		ship.draw(window);
		drawDust(window);
		drawEffects(window);
		
		if (bound.isActive())
			bound.render(window, click_and_drag_handler.get_zoom());

		if (object_info.is_active())
			object_info.render(*this, window);

		if (show_gui)
			gui.draw();

		window.display();

		if (timestep != 0.0) 
			update();

		sf::Time time = clock.getElapsedTime();

		if (iteration % FRAMERATE_CHECK_DELTAFRAME == 0) 
			fps = 1.0f / time.asSeconds();

		clock.restart().asSeconds();
	}
}
