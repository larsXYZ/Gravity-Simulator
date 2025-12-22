#include "user_functions.h"

#include "space.h"
#include <algorithm>


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

namespace PredictionConfig
{
	constexpr int PREDICTION_LENGTH = 200;
	constexpr float PREDICTION_STEP_SIZE = 50.0f;
}

namespace {
struct DistanceCalculationResult
{
	double dx{ 0.0 };
	double dy{ 0.0 };
	double dist{ 0.0 };
	double rad_dist{ 0.0 };
};

DistanceCalculationResult calculateDistance(const Planet & planet, const Planet & other_planet)
{
	DistanceCalculationResult result;
	result.dx = other_planet.getPosition().x - planet.getPosition().x;
	result.dy = other_planet.getPosition().y - planet.getPosition().y;
	result.dist = std::hypot(result.dx, result.dy);
	result.rad_dist = planet.getRadius() + other_planet.getRadius();
	return result;
}

struct Acceleration2D
{
	float x{ 0.0 };
	float y{ 0.0 };
};

double accumulate_acceleration(const DistanceCalculationResult & distance_info, 
						Acceleration2D & acceleration,
						const Planet & other_planet)
{
	const auto A = G * other_planet.getMass() / std::max(distance_info.dist * distance_info.dist, 0.01);
	const auto angle = atan2(distance_info.dy, distance_info.dx);

	acceleration.x += cos(angle) * A;
	acceleration.y += sin(angle) * A;

	return A;
}

enum class PredictionEndReason { MaxSteps, Collision, Disintegration };

struct PredictionResult {
	std::vector<sf::Vertex> path;
	PredictionEndReason reason{ PredictionEndReason::MaxSteps };
	sf::Vector2f endPoint;
	sf::Vector2f endVelocity;
};

struct PhysicsBody
{
	sf::Vector2f position;
	sf::Vector2f velocity;
	double mass;
	double radius;
};

DistanceCalculationResult calculateDistance(const PhysicsBody& planet, const PhysicsBody& other_planet)
{
	DistanceCalculationResult result;
	result.dx = other_planet.position.x - planet.position.x;
	result.dy = other_planet.position.y - planet.position.y;
	result.dist = std::hypot(result.dx, result.dy);
	result.rad_dist = planet.radius + other_planet.radius;
	return result;
}

double accumulate_acceleration(const DistanceCalculationResult& distance_info,
	Acceleration2D& acceleration,
	const PhysicsBody& other_planet)
{
	const auto A = G * other_planet.mass / std::max(distance_info.dist * distance_info.dist, 0.01);
	const auto angle = atan2(distance_info.dy, distance_info.dx);

	acceleration.x += cos(angle) * A;
	acceleration.y += sin(angle) * A;

	return A;
}

}

PredictionResult predict_trajectory(const std::vector<Planet>& planets_orig, const Planet& subject)
{
	std::vector<PhysicsBody> planets;
	planets.reserve(planets_orig.size() + 1);
	for (const auto& p : planets_orig)
		planets.push_back({ p.getPosition(), p.getVelocity(), p.getMass(), p.getRadius() });
	planets.push_back({ subject.getPosition(), subject.getVelocity(), subject.getMass(), subject.getRadius() });

	const size_t subject_index = planets.size() - 1;
	PredictionResult result;
	result.path.reserve(PredictionConfig::PREDICTION_LENGTH);

	const float dt = PredictionConfig::PREDICTION_STEP_SIZE;

	std::vector<Acceleration2D> acc_1(planets.size());
	std::vector<Acceleration2D> acc_2(planets.size());

	for (int i = 0; i < PredictionConfig::PREDICTION_LENGTH; i++)
	{
		std::fill(acc_1.begin(), acc_1.end(), Acceleration2D{ 0,0 });

		// 1. Calculate first acceleration
		for (size_t j = 0; j < planets.size(); j++)
		{
			for (size_t k = 0; k < planets.size(); k++)
			{
				if (j == k) continue;
				auto dist = calculateDistance(planets[j], planets[k]);
				accumulate_acceleration(dist, acc_1[j], planets[k]);
			}
		}

		// 2. First integration step (Position)
		for (size_t j = 0; j < planets.size(); j++)
		{
			planets[j].position.x += planets[j].velocity.x * dt + 0.5f * acc_1[j].x * dt * dt;
			planets[j].position.y += planets[j].velocity.y * dt + 0.5f * acc_1[j].y * dt * dt;
		}

		// CHECK COLLISIONS / ROCHE HERE (using new positions)
		PhysicsBody& pSub = planets.back();
		bool collision = false;
		bool disintegration = false;

		for (size_t k = 0; k < planets.size() - 1; ++k) // Check against all others
		{
			auto distRes = calculateDistance(pSub, planets[k]); 

			// Roche
			if (pSub.mass >= MINIMUMBREAKUPSIZE &&
				distRes.dist < ROCHE_LIMIT_DIST_MULTIPLIER * distRes.rad_dist &&
				pSub.mass / planets[k].mass < ROCHE_LIMIT_SIZE_DIFFERENCE)
			{
				disintegration = true;
				break;
			}

			// Collision
			if (distRes.dist < distRes.rad_dist)
			{
				collision = true;
				break;
			}
		}

		result.path.emplace_back(pSub.position, sf::Color::Red);

		if (collision) {
			result.reason = PredictionEndReason::Collision;
			result.endPoint = pSub.position;
			break;
		}
		if (disintegration) {
			result.reason = PredictionEndReason::Disintegration;
			result.endPoint = pSub.position;
			result.endVelocity = pSub.velocity;
			break;
		}

		std::fill(acc_2.begin(), acc_2.end(), Acceleration2D{ 0,0 });

		// 3. Calculate second acceleration (at new position)
		for (size_t j = 0; j < planets.size(); j++)
		{
			for (size_t k = 0; k < planets.size(); k++)
			{
				if (j == k) continue;
				auto dist = calculateDistance(planets[j], planets[k]);
				accumulate_acceleration(dist, acc_2[j], planets[k]);
			}
		}

		// 4. Second integration step (Velocity)
		for (size_t j = 0; j < planets.size(); j++)
		{
			planets[j].velocity.x += 0.5f * (acc_1[j].x + acc_2[j].x) * dt;
			planets[j].velocity.y += 0.5f * (acc_1[j].y + acc_2[j].y) * dt;
		}
	}
	
	if (result.reason == PredictionEndReason::MaxSteps && !result.path.empty())
	{
		int fadeLen = std::min((int)result.path.size(), 200); 
		for (int k = 0; k < fadeLen; ++k) {
			int idx = result.path.size() - 1 - k;
			sf::Color c = result.path[idx].color;
			c.a = static_cast<sf::Uint8>(255.0f * ((float)k / fadeLen));
			result.path[idx].color = c;
		}
	}
	
	return result;
}

void updateGuiSize(tgui::TextArea::Ptr textbox, int lines)
{
    float lineHeight = textbox->getTextSize() * 1.2f;
    textbox->setSize(textbox->getSize().x, std::max(45.0f, lines * lineHeight + 10.0f));
}

class IUserFunction
{
	virtual void reset() {}
public:
	virtual ~IUserFunction() = default;
	virtual void on_selection(FunctionContext& context)
	{
		context.mass_slider->setVisible(false);
		context.object_type_selector->setVisible(false);
		context.new_object_info->setVisible(true); // Always show for instructions
	};
	virtual void execute(FunctionContext& context) {}
	virtual void handle_event(FunctionContext& context, sf::Event event) {}
	virtual void on_deselection(FunctionContext& context)
	{
        context.new_object_info->setVisible(false);
        context.mass_slider->setVisible(false);
        context.object_type_selector->setVisible(false);
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
		context.object_type_selector->setVisible(true);
		context.new_object_info->setVisible(true);
        context.new_object_info->setText("Select type/mass, then click and drag to launch.");
        updateGuiSize(context.new_object_info, 1);
	}

	void execute(FunctionContext& context) override
	{
		fillTextBox(context.new_object_info, context.mass_slider->getValue());
        updateGuiSize(context.new_object_info, 3);



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
				sf::CircleShape tempCircle(R.getRadius());
				tempCircle.setOrigin(R.getRadius(), R.getRadius());
				tempCircle.setPosition(start_pos);
				tempCircle.setFillColor(sf::Color(255, 0, 0, 100));
				context.window.draw(tempCircle);
				
				const auto to_now = context.mouse_pos_world - start_pos;
				
				// TRAJECTORY PREDICTION
				const auto speed_multiplier{ 0.002 };
				R.setPosition(start_pos);
				R.setVelocity({
					static_cast<float>(-speed_multiplier * to_now.x),
					static_cast<float>(-speed_multiplier * to_now.y)
				});
				
				auto result = predict_trajectory(context.space.planets, R);
				if (!result.path.empty())
				{
					context.window.draw(&result.path[0], result.path.size(), sf::PrimitiveType::LineStrip);
					
					if (result.reason == PredictionEndReason::Collision)
					{
						float size = 10.0f * context.zoom;
						sf::Vertex xLines[] = {
							sf::Vertex(result.endPoint + sf::Vector2f(-size, -size), sf::Color::Red),
							sf::Vertex(result.endPoint + sf::Vector2f(size, size), sf::Color::Red),
							sf::Vertex(result.endPoint + sf::Vector2f(-size, size), sf::Color::Red),
							sf::Vertex(result.endPoint + sf::Vector2f(size, -size), sf::Color::Red)
						};
						context.window.draw(xLines, 4, sf::PrimitiveType::Lines);
					}
					else if (result.reason == PredictionEndReason::Disintegration)
					{
						 float size = 15.0f * context.zoom;
						 
						 float vLen = std::hypot(result.endVelocity.x, result.endVelocity.y);
						 sf::Vector2f dir = (vLen > 0) ? result.endVelocity / vLen : sf::Vector2f(1, 0);
						 sf::Vector2f perp(-dir.y, dir.x);
						 
						 sf::Vertex coneLines[] = {
							sf::Vertex(result.endPoint, sf::Color::Red),
							sf::Vertex(result.endPoint + dir * size + perp * (size * 0.5f), sf::Color::Red),
							sf::Vertex(result.endPoint, sf::Color::Red),
							sf::Vertex(result.endPoint + dir * size - perp * (size * 0.5f), sf::Color::Red)
						 };
						 context.window.draw(coneLines, 4, sf::PrimitiveType::Lines);
					}
				}
			}
		}
		else if (mouseToggle)
		{
			mouseToggle = false;
			const auto to_now = context.mouse_pos_world - start_pos;
			const auto speed_multiplier{ 0.002 };
			Planet R(context.mass_slider->getValue(), start_pos.x, start_pos.y, -speed_multiplier * to_now.x, -speed_multiplier * to_now.y);
			context.space.set_ambient_temperature(R);
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
		context.object_type_selector->setVisible(true);
		context.new_object_info->setVisible(true);
        context.new_object_info->setText("Select type/mass, click parent, then click orbit.");
        updateGuiSize(context.new_object_info, 1);
	}

	void execute(FunctionContext& context) override
	{
		fillTextBox(context.new_object_info, context.mass_slider->getValue());
        updateGuiSize(context.new_object_info, 3);

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
					if (dist < planet.getRadius())
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
				dist = dist * (temp_planet.getMass()) / (temp_planet.getMass() + target->getMass());
				double angleb = atan2(context.mouse_pos_world.y - target->gety(), context.mouse_pos_world.x - target->getx());

				center_point.setPosition(target->getx() + dist * cos(angleb), target->gety() + dist * sin(angleb));
				context.window.draw(center_point);

				//DRAWING ROCHE LIMIT
				if (context.mass_slider->getValue() > MINIMUMBREAKUPSIZE 
					&& context.mass_slider->getValue() / target->getMass() < ROCHE_LIMIT_SIZE_DIFFERENCE)
				{
					double rocheRad = ROCHE_LIMIT_DIST_MULTIPLIER * (temp_planet.getRadius() + target->getRadius());

					sf::CircleShape viz(rocheRad);
					viz.setPosition(sf::Vector2f(target->getx(), target->gety()));
					viz.setOrigin(rocheRad, rocheRad);
					viz.setFillColor(sf::Color(0, 0, 0, 0));
					viz.setOutlineColor(sf::Color(255, 140, 0));
					viz.setOutlineThickness(context.zoom);
					context.window.draw(viz);

				}

				//DRAWING PLANET
				sf::CircleShape bump(temp_planet.getRadius());
				bump.setOrigin(temp_planet.getRadius(), temp_planet.getRadius());
				bump.setFillColor(sf::Color::Red);
				bump.setPosition(context.mouse_pos_world);
				context.window.draw(bump);

				if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{

					const auto speed = sqrt(G * (target->getMass() + context.mass_slider->getValue()) / rad);
					const auto angle = atan2(context.mouse_pos_world.y - target->gety(), context.mouse_pos_world.x - target->getx());

					//To keep total momentum change = 0
					const float normalizing_speed = context.mass_slider->getValue() * speed / (context.mass_slider->getValue() + target->getMass());

					target->setVelocity({ target->getxv() - normalizing_speed * cosf(angle + PI / 2.0),
											target->getyv() - normalizing_speed * sinf(angle + PI / 2.0) });

					Planet new_planet(context.mass_slider->getValue(),
						target->getx() + rad * cos(angle),
						target->gety() + rad * sin(angle),
						target->getxv() + speed * cos(angle + PI / 2.0),
						target->getyv() + speed * sin(angle + PI / 2.0));

					context.space.set_ambient_temperature(new_planet);
					context.space.addPlanet(std::move(new_planet));

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
    void on_selection(FunctionContext& context) override
    {
        IUserFunction::on_selection(context);
        context.new_object_info->setText("Click on an object to remove it.");
        updateGuiSize(context.new_object_info, 1);
    }

	void execute(FunctionContext& context) override
	{
		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) || context.is_mouse_on_widgets)
			return;

		for (auto& planet : context.space.planets)
		{
			const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
				planet.gety() - context.mouse_pos_world.y);
			if (dist < planet.getRadius())
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
    void on_selection(FunctionContext& context) override
    {
        IUserFunction::on_selection(context);
        context.new_object_info->setText("Click anywhere to spawn the spaceship.");
        updateGuiSize(context.new_object_info, 1);
    }

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
    void on_selection(FunctionContext& context) override
    {
        IUserFunction::on_selection(context);
        context.new_object_info->setText("Click an object to start adding rings.");
        updateGuiSize(context.new_object_info, 1);
    }

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
				if (dist < planet.getRadius())
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

				if (rad < parent->getRadius())
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

    void on_selection(FunctionContext& context) override
    {
        IUserFunction::on_selection(context);
        context.new_object_info->setText("Click and drag to spawn a random solar system.");
        updateGuiSize(context.new_object_info, 1);
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
    void on_selection(FunctionContext& context) override
    {
        IUserFunction::on_selection(context);
        context.new_object_info->setText("Click on an object to follow it.");
        updateGuiSize(context.new_object_info, 1);
    }

	void handle_event(FunctionContext& context, sf::Event event) override
	{
		if (event.type != sf::Event::EventType::MouseButtonReleased)
			return;

		if (event.mouseButton.button == sf::Mouse::Left && !context.is_mouse_on_widgets)
		{
            // Check spaceship
            if (context.space.ship.isExist())
            {
                 sf::Vector2f ship_pos = context.space.ship.getpos();
                 float dist = std::hypot(ship_pos.x - context.mouse_pos_world.x, ship_pos.y - context.mouse_pos_world.y);
                 // Simple hit box for ship (approx 10 units radius)
                 if (dist < 10.0f)
                 {
                     context.space.object_tracker.activate_for_spaceship();
                     return;
                 }
            }

			for (const auto& planet : context.space.planets)
			{
				const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y);
				if (dist >= planet.getRadius())
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
	void on_selection(FunctionContext& context) override
	{
        IUserFunction::on_selection(context);
		context.new_object_info->setText("Click on an object to see information.");
        updateGuiSize(context.new_object_info, 1);
	}

	void execute(FunctionContext& context) override
	{
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !context.is_mouse_on_widgets)
		{
			for (const auto& planet : context.space.planets)
			{
				const auto dist = std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y);
				if (dist >= planet.getRadius())
					continue;
				context.space.object_info.activate(planet.getId());
				return;
			}
			context.space.object_info.deactivate();
			context.new_object_info->setText("Click on an object to see information.");
            updateGuiSize(context.new_object_info, 1);
		}

		if (context.space.object_info.is_active())
		{
			Planet* target = context.space.findPlanetPtr(context.space.object_info.get_target_id());
			if (target)
			{
				const auto selected_temp_unit = static_cast<TemperatureUnit>(context.space.temperatureUnitSelector->getSelectedIndex());

				std::string info = target->getName() +
					"\nType: " + std::string(target->getTypeString(target->getType())) +
					"\nRadius: " + std::to_string(static_cast<int>(target->getRadius())) +
					"\nMass: " + std::to_string(static_cast<int>(target->getMass())) +
					"\nSpeed: " + std::to_string(std::hypot(target->getxv(), target->getyv())) +
					"\nTemperature: " + Space::temperature_info_string(target->getTemp(), selected_temp_unit);

				Planet* target_parent = context.space.findPlanetPtr(target->getStrongestAttractorId());
				if (target_parent)
					info += "\nDistance: " + std::to_string(static_cast<int>(std::hypot(target->getx() - target_parent->getx(),
						target->gety() - target_parent->gety())));

				if (target->getType() == TERRESTIAL)
				{
					info += "\n\nAtmo: " + std::to_string((int)target->getCurrentAtmosphere()) + " / " + std::to_string((int)target->getAtmospherePotensial()) + "kPa";
					if (target->getLife().getTypeEnum() == 0)
						info += "\n\n" + target->getFlavorTextLife();
				}

				if (target->getLife().getTypeEnum() != 0)
				{
					info += "\n\nBiomass: " + std::to_string((int)target->getLife().getBmass()) + "MT";
					info += "\n" + target->getLife().getType() + "\n" + target->getFlavorTextLife();
				}

				context.new_object_info->setText(info);
                
                // Count lines for scaling
                int lines = std::count(info.begin(), info.end(), '\n') + 1;
                updateGuiSize(context.new_object_info, lines);
			}
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
        context.new_object_info->setText("Select objects, then Ctrl-Click to place in orbit.");
        updateGuiSize(context.new_object_info, 1);
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
					planet.gety() - context.mouse_pos_world.y) < planet.getRadius())
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
				const float adjust_speed = context.mass_slider->getValue() * speed / (context.mass_slider->getValue() + massCenterInfoVector.z);


				Planet new_planet(context.mass_slider->getValue(),
					massCenterInfoVector.x + rad * cos(angle),
					massCenterInfoVector.y + rad * sin(angle),
					(massCenterVelocity.x + speed * cos(angle + PI / 2.0) - adjust_speed * cos(angle + PI / 2.0)),
					(massCenterVelocity.y + speed * sin(angle + PI / 2.0) - adjust_speed * sin(angle + PI / 2.0)));

				context.space.set_ambient_temperature(new_planet);
				context.space.addPlanet(std::move(new_planet));

				for (const auto id : object_ids)
				{
					if (Planet* planet = context.space.findPlanetPtr(id))
					{
						planet->setVelocity({ planet->getxv() - adjust_speed * cosf(angle + PI / 2.0), 
							planet->getyv() - adjust_speed * sinf(angle + PI / 2.0) });
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
			
			sf::CircleShape indicator(planet->getRadius() + 10);
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
			const auto massCenterVelocity = context.space.centerOfMassVelocity(object_ids);
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
				sf::CircleShape bump(temp.getRadius());
				bump.setOrigin(temp.getRadius(), temp.getRadius());
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
    void on_selection(FunctionContext& context) override
    {
        IUserFunction::on_selection(context);
        context.new_object_info->setText("Click on an object to explode it.");
        updateGuiSize(context.new_object_info, 1);
    }

	void handle_event(FunctionContext& context, sf::Event event) override
	{
		if (event.type != sf::Event::EventType::MouseButtonReleased)
			return;

		if (event.mouseButton.button == sf::Mouse::Left && !context.is_mouse_on_widgets)
		{
			for (const auto planet : context.space.planets)
			{
				if (std::hypot(planet.getx() - context.mouse_pos_world.x,
					planet.gety() - context.mouse_pos_world.y) < planet.getRadius())
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
    void on_selection(FunctionContext& context) override
    {
        IUserFunction::on_selection(context);
        context.new_object_info->setText("Click and drag to set the simulation boundary.");
        updateGuiSize(context.new_object_info, 1);
    }

	void execute(FunctionContext& context) override
	{
		if (context.space.auto_bound_active())
		{
			reset();
			return;
		}

		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (has_center && context.bound.getRadius() > BOUND_MIN_RAD)
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

		if (context.bound.getRadius() > BOUND_MIN_RAD)
			context.bound.render(context.window, context.zoom);

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
