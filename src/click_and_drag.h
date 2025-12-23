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
				const sf::Event& new_event,
				bool zoom_towards_mouse = true)
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
				if (new_event.mouseWheelScroll.delta == 0)
					break;

				float delta_zoom_factor = (new_event.mouseWheelScroll.delta > 0) ? (1.f / 1.15f) : 1.15f;

				if (zoom_towards_mouse)
				{
					sf::Vector2i mouse_pos{ new_event.mouseWheelScroll.x, new_event.mouseWheelScroll.y };
					sf::Vector2f before_zoom = window.mapPixelToCoords(mouse_pos, view);

					view.zoom(delta_zoom_factor);
					zoom *= delta_zoom_factor;

					sf::Vector2f after_zoom = window.mapPixelToCoords(mouse_pos, view);
					view.move(before_zoom - after_zoom);
				}
				else
				{
					view.zoom(delta_zoom_factor);
					zoom *= delta_zoom_factor;
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

	float get_zoom() const
	{
		return zoom;
	}
};
