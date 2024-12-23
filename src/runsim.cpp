#include "space.h"

#include "user_functions.h"

class ClickAndDragHandler
{
	float zoom{1.f};
	bool is_dragging{false};
	sf::Vector2f last_mouse_pos_window{};
public:
	void update(sf::View & view,
				sf::RenderWindow& window,
				const sf::Event& new_event)
	{
		switch (new_event.type)
		{
		case sf::Event::MouseMoved:
			{
				if (!is_dragging)
					break;

				sf::Vector2f mouse_position_now_window {static_cast<float>(new_event.mouseMove.x),
														static_cast<float>(new_event.mouseMove.y)};

				view.move(zoom*(last_mouse_pos_window - mouse_position_now_window));
				last_mouse_pos_window = mouse_position_now_window;

				break;
			}
		case sf::Event::MouseButtonReleased:
			{
				if (!is_dragging)
					break;

				if (new_event.mouseButton.button != sf::Mouse::Button::Right)
					break;

				is_dragging = false;
				break;
			}
		case sf::Event::MouseButtonPressed:
			{
				if (is_dragging)
					break;

				if (new_event.mouseButton.button != sf::Mouse::Button::Right)
					break;

				is_dragging = true;
				last_mouse_pos_window = sf::Vector2f( (float)new_event.mouseButton.x, 
													(float)new_event.mouseButton.y );
				break;
			}
		case sf::Event::MouseWheelScrolled:
			{
				auto delta_zoom = 1.15 * new_event.mouseWheelScroll.delta / std::abs(new_event.mouseWheelScroll.delta);
				if (delta_zoom < 0)
				{
					delta_zoom = std::abs(delta_zoom);
					zoom *= delta_zoom;
					view.zoom(delta_zoom);
				}
				else if (delta_zoom > 0)
				{
					delta_zoom = std::abs(delta_zoom);
					zoom /= delta_zoom;
					view.zoom(1 / delta_zoom);
				}
				break;
			}
		}
	}
};

inline static ClickAndDragHandler click_and_drag_handler;

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
	gui.add(tempChooser);
	gui.add(currPlanetInfo);
	gui.add(massExistingObjectSlider);
	gui.add(autoBound);

	while (window.isOpen())
	{
		window.clear(sf::Color::Black);

		setInfo();

		hotkeys(window);

		while(window.pollEvent(event))
		{
			click_and_drag_handler.update(mainView, window, event);
			gui.handleEvent(event);
		}

		window.setView(mainView);

		FunctionContext context
		{
			.type = getSelectedFunction(functions),
			.space = *this,
			.spaceship = ship,
			.view = mainView,
			.window = window,
			.gui = gui,
			.mouse_pos_window = sf::Mouse::getPosition(window),
			.mouse_pos_world = window.mapPixelToCoords(sf::Mouse::getPosition(window), mainView)
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
