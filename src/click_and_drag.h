#pragma once

#include <SFML/Graphics.hpp>

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

	void reset()
	{
		zoom = 1.f;
		is_dragging = false;
		last_mouse_pos_window = {};
	}
};
