#include "space.h"

#include "user_functions.h"
#include "udp_server.h"
#include <memory>

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

void Space::runSim(sf::Vector2i window_size, bool fullscreen, int udp_port)
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
	gui.add(editObjectButton);
	gui.add(newPlanetInfo);
	gui.add(objectTypeSelector);
	gui.add(massSlider);
	gui.add(timeStepLabel);
	gui.add(timeStepSlider);
	gui.add(temperatureUnitSelector);

	gui.add(optionsMenu);
	gui.add(optionsButton);
	gui.add(quitDialog);


	// Initialize bloom effect
	bloom.init(window_size.x, window_size.y);

	// UDP command server
	std::unique_ptr<UdpCommandServer> udpServer;
	if (udp_port > 0)
	{
		udpServer = std::make_unique<UdpCommandServer>(static_cast<unsigned short>(udp_port));
		if (!udpServer->start())
			udpServer.reset();
	}

	sf::Event event;
	while (window.isOpen())
	{
		timestep = config.paused ? 0.0 : timeStepSlider->getValue();

		// Check if mouse is inside the window bounds
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		bool mouse_in_window = mousePos.x >= 0 && mousePos.y >= 0
			&& mousePos.x < static_cast<int>(window.getSize().x)
			&& mousePos.y < static_cast<int>(window.getSize().y);

        is_mouse_on_gui = !mouse_in_window || is_mouse_on_widgets(window, gui);

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

			if (event.type == sf::Event::Resized)
				bloom.resize(event.size.width, event.size.height);

			if (!object_info.is_focused(window))
				hotkeys(event, mainView, window);

			if (event.type == sf::Event::MouseWheelScrolled || !object_tracker.is_active())
				click_and_drag_handler.update(mainView, window, event, !object_tracker.is_active());

			gui.handleEvent(event);

			giveFunctionEvent(context, event);
		}

		window.setView(mainView);

		flushPlanets();

		if (udpServer && udpServer->processCommands(*this, mainView, window))
			syncConfigToWidgets();

		if (object_tracker.is_active())
			object_tracker.update(*this, mainView);

		updateInfoBox();
		editObjectButton->setVisible(object_info.is_active());

		// Draw scene to bloom render target or directly to window
		bool useBloom = bloom.isAvailable() && config.bloom_enabled;
		sf::RenderTarget& target = useBloom ?
			static_cast<sf::RenderTarget&>(bloom.getSceneTarget()) :
			static_cast<sf::RenderTarget&>(window);

		target.clear(sf::Color::Black);
		target.setView(mainView);

		drawPlanets(target);
		ship.draw(target);
		drawDust(target);
		drawEffects(target);

		if (bound.isActive())
			bound.render(target, click_and_drag_handler.get_zoom());

		// Apply bloom post-processing
		if (useBloom)
		{
			window.clear(sf::Color::Black);
			bloom.apply(window);
		}

		// Draw black hole discs on top of bloom so they stay dark
		window.setView(mainView);
		drawBlackHoleDiscs(window);

		// Draw overlays directly to window (not bloomed)
		executeFunction(context);

		if (object_info.is_active())
			object_info.render(*this, window);

		if (config.show_gui)
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
