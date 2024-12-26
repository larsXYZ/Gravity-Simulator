#include "user_functions.h"

#include "space.h"


FunctionType getSelectedFunction(tgui::ListBox::Ptr listbox)
{
	const auto selected_label = listbox->getSelectedItem();

	const auto match = std::find_if(function_info.begin(), function_info.end(),
		[&selected_label](auto function)
		{
			return function.label == selected_label;
		});

	if (match == function_info.end())
		return FunctionType::NO_FUNCTION;

	return match->type;
}

void fillFunctionGUIDropdown(tgui::ListBox::Ptr listbox)
{
	for (const auto& function : function_info)
		listbox->addItem(function.label);
	listbox->setSelectedItemByIndex(0);
}

void setFunctionGUIFromHotkeys(tgui::ListBox::Ptr listbox)
{
	for (const auto& function : function_info)
	{
		if (sf::Keyboard::isKeyPressed(function.hotkey))
			listbox->setSelectedItem(function.label);
	}
}

void fillTextBox(tgui::TextArea::Ptr textbox, double mass)
{
	Planet temp(mass);
	textbox->setText("NEW OBJECT\nMass:   " + std::to_string(static_cast<int>(mass))
							+ "\nType:      " + Planet::getTypeString(temp.getType()));
}

class IUserFunction
{
	virtual void reset() {}
public:
	virtual ~IUserFunction() = default;
	virtual void on_selection(FunctionContext& context)
	{
		context.mass_slider->setVisible(false);
		context.new_object_info->setVisible(false);
	};
	virtual void execute(FunctionContext& context) {}
	virtual void handle_event(FunctionContext& context, sf::Event event) {}
	virtual void on_deselection(FunctionContext& context)
	{
		reset();
	};
};

class NewObjectFunction : public IUserFunction
{
	bool mouseToggle{ false };
	sf::Vector2f start_pos;
public:
	void on_selection(FunctionContext& context) override
	{
		context.mass_slider->setVisible(true);
		context.new_object_info->setVisible(true);
	}

	void execute(FunctionContext& context) override
	{
		fillTextBox(context.new_object_info, context.mass_slider->getValue());

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !context.is_mouse_on_widgets)
		{
			if (!mouseToggle)
			{
				mouseToggle = true;
				start_pos = context.mouse_pos_world;
			}
			if (mouseToggle)
			{
				Planet R(context.mass_slider->getValue());
				sf::CircleShape tempCircle(R.getRad());
				tempCircle.setOrigin(R.getRad(), R.getRad());
				tempCircle.setPosition(start_pos);
				tempCircle.setFillColor(sf::Color(255, 0, 0, 100));
				context.window.draw(tempCircle);
				
				const auto to_now = context.mouse_pos_world - start_pos;
				sf::Vertex line[] =
				{
					sf::Vertex(start_pos,sf::Color::Red),
					sf::Vertex(start_pos - to_now, sf::Color::Red)
				};

				context.window.draw(line, 2, sf::Lines);
			}
		}
		else if (mouseToggle)
		{
			mouseToggle = false;
			const auto to_now = context.mouse_pos_world - start_pos;
			const auto speed_multiplier{ 0.002 };
			Planet R(context.mass_slider->getValue(), start_pos.x, start_pos.y, -speed_multiplier * to_now.x, -speed_multiplier * to_now.y);
			context.space.addPlanet(std::move(R));
		}
	}
};

class NewObjectInOrbitFunction : public IUserFunction
{
	enum class InOrbitFunctionState
	{
		INACTIVE,
		PARENT_FOUND,
	} state{InOrbitFunctionState::INACTIVE};
	int target_planet_id{ -1 };

	void reset() override
	{
		state = InOrbitFunctionState::INACTIVE;
		target_planet_id = -1;
	}
public:
	void on_selection(FunctionContext& context) override
	{
		context.mass_slider->setVisible(true);
		context.new_object_info->setVisible(true);
	}

	void execute(FunctionContext& context) override
	{
		fillTextBox(context.new_object_info, context.mass_slider->getValue());

		switch (state)
		{
		case InOrbitFunctionState::INACTIVE:
			{
				if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) || context.is_mouse_on_widgets)
					break;

				for (const auto & planet : context.space.planets)
				{
					const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
														planet.gety() - context.mouse_pos_world.y);
					if (dist < planet.getRad())
					{
						target_planet_id = planet.getId();
						state = InOrbitFunctionState::PARENT_FOUND;
						break;
					}
				}

				break;
			}
		case InOrbitFunctionState::PARENT_FOUND:
			{
				Planet* target = context.space.findPlanetPtr(target_planet_id);
				if (!target)
				{
					reset();
					break;
				}

				const auto rad = std::hypot(context.mouse_pos_world.x - target->getx(),
													context.mouse_pos_world.y - target->gety());

				Planet temp_planet(context.mass_slider->getValue());
				
				if (target->getType() == SMALLSTAR 
					|| target->getType() == STAR 
					|| target->getType() == BIGSTAR)
				{
					const auto goldilock_info = target->getGoldilockInfo();

					sf::CircleShape goldilock_visualizer(goldilock_info.min_rad);
					goldilock_visualizer.setPointCount(60);
					goldilock_visualizer.setPosition(sf::Vector2f(target->getx(), target->gety()));
					goldilock_visualizer.setOrigin(goldilock_info.min_rad, goldilock_info.min_rad);
					goldilock_visualizer.setOutlineThickness(goldilock_info.max_rad - goldilock_info.min_rad);
					goldilock_visualizer.setFillColor(sf::Color(0, 0, 0, 0));
					goldilock_visualizer.setOutlineColor(sf::Color(0, 200, 0, goldi_strength));
					context.window.draw(goldilock_visualizer);
				}
				
				sf::CircleShape orbit_visualizer(rad);
				orbit_visualizer.setPosition(sf::Vector2f(target->getx(), target->gety()));
				orbit_visualizer.setRadius(rad);
				orbit_visualizer.setOrigin(rad, rad);
				orbit_visualizer.setFillColor(sf::Color(0, 0, 0, 0));
				orbit_visualizer.setOutlineColor(sf::Color::Red);
				orbit_visualizer.setOutlineThickness(context.zoom);
				context.window.draw(orbit_visualizer);

				//DRAWING MASS CENTER
				sf::CircleShape center_point(2);
				center_point.setOrigin(2, 2);
				center_point.setFillColor(sf::Color(255, 0, 0));
				
				double dist = std::hypot(context.mouse_pos_world.x - target->getx(), context.mouse_pos_world.y - target->gety());
				dist = dist * (temp_planet.getmass()) / (temp_planet.getmass() + target->getmass());
				double angleb = atan2(context.mouse_pos_world.y - target->gety(), context.mouse_pos_world.x - target->getx());

				center_point.setPosition(target->getx() + dist * cos(angleb), target->gety() + dist * sin(angleb));
				context.window.draw(center_point);

				//DRAWING ROCHE LIMIT
				if (context.mass_slider->getValue() > MINIMUMBREAKUPSIZE 
					&& context.mass_slider->getValue() / target->getmass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
				{
					double rocheRad = ROCHE_LIMIT_DIST_MULTIPLIER * (temp_planet.getRad() + target->getRad());

					sf::CircleShape viz(rocheRad);
					viz.setPosition(sf::Vector2f(target->getx(), target->gety()));
					viz.setOrigin(rocheRad, rocheRad);
					viz.setFillColor(sf::Color(0, 0, 0, 0));
					viz.setOutlineColor(sf::Color(255, 140, 0));
					viz.setOutlineThickness(context.zoom);
					context.window.draw(viz);

				}

				//DRAWING PLANET
				sf::CircleShape bump(temp_planet.getRad());
				bump.setOrigin(temp_planet.getRad(), temp_planet.getRad());
				bump.setFillColor(sf::Color::Red);
				bump.setPosition(context.mouse_pos_world);
				context.window.draw(bump);

				if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{

					const auto speed = sqrt(G * (target->getmass() + context.mass_slider->getValue()) / rad);
					const auto angle = atan2(context.mouse_pos_world.y - target->gety(), context.mouse_pos_world.x - target->getx());

					//To keep total momentum change = 0
					const auto normalizing_speed = context.mass_slider->getValue() * speed / (context.mass_slider->getValue() + target->getmass());

					target->setxv(target->getxv() - normalizing_speed * cos(angle + PI / 2.0));
					target->setyv(target->getyv() - normalizing_speed * sin(angle + PI / 2.0));

					context.space.addPlanet(Planet(context.mass_slider->getValue(), 
						target->getx() + rad * cos(angle), 
						target->gety() + rad * sin(angle),
						target->getxv() + speed * cos(angle + PI / 2.0),
						target->getyv() + speed * sin(angle + PI / 2.0)));

					reset();
				}
				break;
			}
		}
	}
};

class RemoveObjectFunction : public IUserFunction
{
public:
	void execute(FunctionContext& context) override
	{
		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) || context.is_mouse_on_widgets)
			return;

		for (auto& planet : context.space.planets)
		{
			const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
				planet.gety() - context.mouse_pos_world.y);
			if (dist < planet.getRad())
			{
				planet.markForRemoval();
				return;
			}
		}
	}
};

class SpawnShipFunction : public IUserFunction
{
public:
	void execute(FunctionContext& context) override
	{
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !context.is_mouse_on_widgets)
			context.spaceship.reset(context.mouse_pos_world);
	}
};

class AddRingsFunction : public IUserFunction
{
	enum class AddRingsFunctionState
	{
		INACTIVE,
		PARENT_FOUND,
		INNER_CREATED
	} state{AddRingsFunctionState::INACTIVE};

	int target_planet_id{ -1 };
	double inner_rad{ -1.0 };

	void reset() override
	{
		state = AddRingsFunctionState::INACTIVE;
		target_planet_id = -1;
		inner_rad = -1.0;
	}

public:
	void execute(FunctionContext& context) override
	{
		switch (state)
		{
		case AddRingsFunctionState::INACTIVE:
		{
			if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) || context.is_mouse_on_widgets)
				return;

			for (const auto& planet : context.space.planets)
			{
				const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y);
				if (dist < planet.getRad())
				{
					target_planet_id = planet.getId();
					state = AddRingsFunctionState::PARENT_FOUND;
					break;
				}
			}

			break;
		}
		case AddRingsFunctionState::PARENT_FOUND:
			{
				Planet* parent = context.space.findPlanetPtr(target_planet_id);
				if (!parent)
				{
					reset();
					break;
				}

				const auto rad = std::hypot(context.mouse_pos_world.x - parent->getx(),
													context.mouse_pos_world.y - parent->gety());

				if (rad < parent->getRad())
				{
					if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
						reset();
					break;
				}

				sf::CircleShape indicator(rad);
				indicator.setPosition(sf::Vector2f(parent->getx(), parent->gety()));
				indicator.setOrigin(rad, rad);
				indicator.setFillColor(sf::Color(0, 0, 0, 0));
				indicator.setOutlineColor(sf::Color::Red);
				indicator.setOutlineThickness(context.zoom);
				context.window.draw(indicator);

				if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					state = AddRingsFunctionState::INNER_CREATED;
					inner_rad = rad;
				}
				break;
			}
		case AddRingsFunctionState::INNER_CREATED:
			{
				Planet* parent = context.space.findPlanetPtr(target_planet_id);
				if (!parent)
				{
					reset();
					break;
				}

				const auto rad = std::hypot(context.mouse_pos_world.x - parent->getx(),
					context.mouse_pos_world.y - parent->gety());

				if (rad < inner_rad)
				{
					if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
						reset();
					break;
				}

				sf::CircleShape indicator(inner_rad);
				indicator.setPosition(sf::Vector2f(parent->getx(), parent->gety()));
				indicator.setOrigin(inner_rad, inner_rad);
				indicator.setFillColor(sf::Color(0, 0, 0, 0));
				indicator.setOutlineColor(sf::Color::Red);
				indicator.setOutlineThickness(context.zoom);
				context.window.draw(indicator);

				indicator.setRadius(rad);
				indicator.setOrigin(rad, rad);
				context.window.draw(indicator);

				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					context.space.giveRings(*parent, inner_rad, rad);
					reset();
				}
				break;
			}
		}
	}
};

class RandomSystemFunction : public IUserFunction
{
	sf::Font font;

	enum class RandomSystemState
	{
		INACTIVE,
		LOCATION_FOUND
	} state{RandomSystemState::INACTIVE};

	sf::Vector2f location{};

	void reset()
	{
		state = RandomSystemState::INACTIVE;
		location = {};
	}
public:
	RandomSystemFunction()
	{
		font.loadFromFile("sansation.ttf");
	}

	void execute(FunctionContext& context) override
	{
		switch (state)
		{
		case RandomSystemState::INACTIVE:
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !context.is_mouse_on_widgets)
				{
					location = context.mouse_pos_world;
					state = RandomSystemState::LOCATION_FOUND;
				}
				break;
			}
		case RandomSystemState::LOCATION_FOUND:
			{
			const auto rad = std::hypot(context.mouse_pos_world.x - location.x,
				context.mouse_pos_world.y - location.y);

			sf::CircleShape indicator(rad);
			indicator.setPosition(location);
			indicator.setOrigin(rad, rad);
			indicator.setFillColor(sf::Color(0, 0, 0, 0));
			indicator.setOutlineColor(sf::Color::Red);
			indicator.setOutlineThickness(context.zoom);
			indicator.setPointCount(100);
			context.window.draw(indicator);

			sf::Vertex line[] =
			{
				sf::Vertex(location,sf::Color::Red),
				sf::Vertex(context.mouse_pos_world, sf::Color::Red)
			};

			sf::Text t;
			t.setString("Planets: " + std::to_string((int)((NUMBER_OF_OBJECT_MULTIPLIER * rad) + 1))
				+ "\nMass: ca " + std::to_string(MASS_MULTIPLIER * cbrt(rad))
				+ "\nRadius: " + std::to_string(rad));
			t.setPosition(context.mouse_pos_world.x + 10, context.mouse_pos_world.y);
			t.setColor(sf::Color::Red);
			t.setFont(font);
			t.setCharacterSize(10);

			context.window.draw(line, 2, sf::Lines);
			context.window.draw(t);

			if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && !context.is_mouse_on_widgets)
			{
				context.space.randomPlanets(MASS_MULTIPLIER * cbrt(rad), NUMBER_OF_OBJECT_MULTIPLIER * rad, rad, location);
				reset();
			}
			break;
			}
		}
	}
};

class TrackObjectFunction : public IUserFunction
{
public:

	void handle_event(FunctionContext& context, sf::Event event) override
	{
		if (event.type != sf::Event::EventType::MouseButtonReleased)
			return;

		if (event.mouseButton.button == sf::Mouse::Left && !context.is_mouse_on_widgets)
		{
			for (const auto& planet : context.space.planets)
			{
				const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y);
				if (dist >= planet.getRad())
					continue;
				context.space.object_tracker.activate(planet.getId());
				return;
			}
			context.space.object_tracker.deactivate();
		}
	}
};

class ShowObjectInfoFunction : public IUserFunction
{
public:
	void execute(FunctionContext& context) override
	{
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !context.is_mouse_on_widgets)
		{
			for (const auto& planet : context.space.planets)
			{
				const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y);
				if (dist >= planet.getRad())
					continue;
				context.space.object_info.activate(planet.getId());
				return;
			}
			context.space.object_info.deactivate();
		}
	}
};

class AdvancedInOrbitFunction : public IUserFunction
{
	std::vector<int> object_ids;
public:

	void on_selection(FunctionContext& context) override
	{
		context.mass_slider->setVisible(true);
		context.new_object_info->setVisible(true);
	}

	void reset() override
	{
		object_ids.clear();
	}

	void handle_event(FunctionContext& context, sf::Event event) override
	{
		if (event.type != sf::Event::EventType::MouseButtonReleased)
			return;

		if (event.mouseButton.button == sf::Mouse::Left && !context.is_mouse_on_widgets)
		{
			for (const auto& planet : context.space.planets)
			{
				if (std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y) < planet.getRad())
				{
					auto match = std::find(object_ids.begin(), object_ids.end(), planet.getId());
					if (match == object_ids.end())
						object_ids.push_back(planet.getId());
					else
						object_ids.erase(match);
				}
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
			{
				const auto massCenterInfoVector(context.space.centerOfMass(object_ids));
				const auto massCenterVelocity = context.space.centerOfMassVelocity(object_ids);
				const auto rad = std::hypot(context.mouse_pos_world.x - massCenterInfoVector.x,
					context.mouse_pos_world.y - massCenterInfoVector.y);
				const auto speed = sqrt(G * (massCenterInfoVector.z + context.mass_slider->getValue()) / rad);
				const auto angle = atan2(context.mouse_pos_world.y - massCenterInfoVector.y,
					context.mouse_pos_world.x - massCenterInfoVector.x);
				const auto adjust_speed = context.mass_slider->getValue() * speed / (context.mass_slider->getValue() + massCenterInfoVector.z);
				context.space.addPlanet(Planet(context.mass_slider->getValue(),
					massCenterInfoVector.x + rad * cos(angle),
					massCenterInfoVector.y + rad * sin(angle),
					(massCenterVelocity.x + speed * cos(angle + PI / 2.0) - adjust_speed * cos(angle + PI / 2.0)),
					(massCenterVelocity.y + speed * sin(angle + PI / 2.0) - adjust_speed * sin(angle + PI / 2.0))));

				for (const auto id : object_ids)
				{
					if (Planet* planet = context.space.findPlanetPtr(id))
					{
						planet->setxv(planet->getxv() - adjust_speed * cos(angle + PI / 2.0));
						planet->setyv(planet->getyv() - adjust_speed * sin(angle + PI / 2.0));
					}
				}

				reset();
			}
		}
	}

	void execute(FunctionContext& context) override
	{
		std::erase_if(object_ids,
			[&context](auto id)
			{
				return !context.space.findPlanetPtr(id);
			});

		if (object_ids.empty())
			return;

		for (const auto id : object_ids)
		{
			Planet* planet = context.space.findPlanetPtr(id);
			
			sf::CircleShape indicator(planet->getRad() + 10);
			indicator.setPosition(planet->getx(), planet->gety());
			indicator.setOrigin(indicator.getRadius(), indicator.getRadius());
			indicator.setFillColor(sf::Color(0, 0, 0, 0));
			indicator.setOutlineColor(sf::Color::Red);
			indicator.setOutlineThickness(5*context.zoom);
			context.window.draw(indicator);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
		{
			const auto massCenterInfoVector(context.space.centerOfMass(object_ids));
			const auto rad = std::hypot(context.mouse_pos_world.x - massCenterInfoVector.x,
				context.mouse_pos_world.y - massCenterInfoVector.y);

			//DRAWING NEW PLANET AND ORBIT
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				//DRAWING ORBIT
				sf::CircleShape indicator(rad);
				indicator.setPosition(sf::Vector2f(massCenterInfoVector.x, massCenterInfoVector.y));
				indicator.setOrigin(rad, rad);
				indicator.setFillColor(sf::Color(0, 0, 0, 0));
				indicator.setOutlineColor(sf::Color::Red);
				indicator.setOutlineThickness(context.zoom);
				context.window.draw(indicator);

				//DRAWING PLANET
				Planet temp(context.mass_slider->getValue());
				sf::CircleShape bump(temp.getRad());
				bump.setOrigin(temp.getRad(), temp.getRad());
				bump.setFillColor(sf::Color::Red);
				bump.setPosition(context.mouse_pos_world);
				context.window.draw(bump);
			}
		}
	}
};

class ExplodeObjectFunction : public IUserFunction
{
public:
	void handle_event(FunctionContext& context, sf::Event event) override
	{
		if (event.type != sf::Event::EventType::MouseButtonReleased)
			return;

		if (event.mouseButton.button == sf::Mouse::Left && !context.is_mouse_on_widgets)
		{
			for (const auto planet : context.space.planets)
			{
				if (std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y) < planet.getRad())
				{
					context.space.explodePlanet(planet);
					return;
				}
			}
		}
	}
};

class BoundControlFunction : public IUserFunction
{
	bool has_center{ false };
public:
	void execute(FunctionContext& context) override
	{
		if (context.space.auto_bound_active())
		{
			reset();
			return;
		}

		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (has_center && context.bound.getRad() > BOUND_MIN_RAD)
			{
				context.bound.setActiveState(true);
				reset();
			}
				
			return;
		}

		if (!has_center)
		{
			has_center = true;
			context.bound.setPos(context.mouse_pos_world);
			context.bound.setActiveState(false);
		}

		double rad = std::hypot(context.mouse_pos_world.x - context.bound.getPos().x,
								context.mouse_pos_world.y - context.bound.getPos().y);
		context.bound.setRad(rad);

		if (context.bound.getRad() > BOUND_MIN_RAD)
			context.bound.render(context.window);

	}

	void reset()
	{
		has_center = false;
	}
};

class ExecutionerContainer
{
	FunctionType prev_type;
	std::weak_ptr<IUserFunction> prev_function;
	std::map<FunctionType, std::shared_ptr<IUserFunction>> executioners;
public:
	ExecutionerContainer()
	{
		executioners[FunctionType::NEW_OBJECT] = std::make_shared<NewObjectFunction>();
		executioners[FunctionType::OBJECT_IN_ORBIT] = std::make_shared<NewObjectInOrbitFunction>();
		executioners[FunctionType::REMOVE_OBJECT] = std::make_shared<RemoveObjectFunction>();
		executioners[FunctionType::SPAWN_SHIP] = std::make_shared<SpawnShipFunction>();
		executioners[FunctionType::ADD_RINGS] = std::make_shared<AddRingsFunction>();
		executioners[FunctionType::RANDOM_SYSTEM] = std::make_shared<RandomSystemFunction>();
		executioners[FunctionType::FOLLOW_OBJECT] = std::make_shared<TrackObjectFunction>();
		executioners[FunctionType::SHOW_INFO] = std::make_shared<ShowObjectInfoFunction>();
		executioners[FunctionType::ADVANCED_OBJECT_IN_ORBIT] = std::make_shared<AdvancedInOrbitFunction>();
		executioners[FunctionType::EXPLODE_OBJECT] = std::make_shared<ExplodeObjectFunction>();
		executioners[FunctionType::ADD_BOUND] = std::make_shared<BoundControlFunction>();
	}
	void execute(FunctionContext & context)
	{
		auto match = executioners.find(context.type);
		if (match == executioners.end())
		{
			if (auto prev = prev_function.lock())
				prev->on_deselection(context);

			prev_function.reset();

			return;
		}

		if (context.type != prev_type)
		{
			if (auto prev = prev_function.lock())
				prev->on_deselection(context);

			match->second->on_selection(context);
		}

		match->second->execute(context);

		prev_function = match->second;
		prev_type = context.type;
	}

	void handle_event(FunctionContext& context, sf::Event event)
	{
		auto match = executioners.find(context.type);
		if (match == executioners.end())
			return;

		match->second->handle_event(context, event);
	}
};


ExecutionerContainer executioners;

void executeFunction(FunctionContext& context)
{
	executioners.execute(context);
}

void giveFunctionEvent(FunctionContext& context, sf::Event event)
{
	executioners.handle_event(context, event);
}
