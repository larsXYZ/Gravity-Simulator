#include "object_info.h"

#include "Space.h"

ObjectInfo::ObjectInfo()
{
	font.loadFromFile("sansation.ttf");
	text.setFont(font);
}

bool ObjectInfo::is_active() const
{
	return target_id != -1;
}

void ObjectInfo::deactivate()
{
	target_id = -1;
}

void ObjectInfo::activate(int new_target_id)
{
	target_id = new_target_id;
}

void ObjectInfo::render(Space& space, sf::RenderWindow& window)
{
	Planet* target = space.findPlanetPtr(target_id);
	if (!target)
	{
		deactivate();
		return;
	}
	
	sf::Vector2f pos(target->getx(), target->gety());
	std::string planet_type = Planet::getTypeString(target->getType());
	
	sf::Vertex l[] =
	{
		sf::Vertex(sf::Vector2f(pos.x + 1.5 * target->getRad(), pos.y + 1.5 * target->getRad()),sf::Color::Cyan),
		sf::Vertex(pos, sf::Color::Cyan)
	};
	window.draw(l, 2, sf::Lines);
	
	sf::Vertex v[] =
	{
		sf::Vertex(sf::Vector2f(pos.x + 400 * target->getxv(), pos.y + 400 * target->getyv()),sf::Color::Red),
		sf::Vertex(pos, sf::Color::Red)
	};
	window.draw(v, 2, sf::Lines);
	
	if (space.get_iteration() % TRAILFREQ == 0)
		space.addTrail(pos, TRAILLIFE);

	Planet* target_parent = space.findPlanetPtr(target->getStrongestAttractorId());
	if (target_parent)
	{
		sf::Vertex g[] =
		{
			sf::Vertex(sf::Vector2f(target_parent->getx(), target_parent->gety()),sf::Color::Yellow),
			sf::Vertex(sf::Vector2f(pos.x, pos.y), sf::Color::Yellow)
		};
		window.draw(g, 2, sf::Lines);

		double rocheRad = ROCHE_LIMIT_DIST_MULTIPLIER * (target_parent->getRad() + target->getRad());
		sf::CircleShape omr(rocheRad);
		omr.setPosition(sf::Vector2f(target_parent->getx(), target_parent->gety()));
		omr.setOrigin(rocheRad, rocheRad);
		omr.setFillColor(sf::Color(0, 0, 0, 0));
		omr.setOutlineColor(sf::Color(255, 140, 0));
		window.draw(omr);

		//CENTER OF MASS
		sf::CircleShape middle(2);
		middle.setOrigin(2, 2);
		middle.setFillColor(sf::Color::Yellow);

		double dist = std::hypot(target_parent->getx() - pos.x, 
									target_parent->gety() - pos.y);

		dist *= (target_parent->getmass()) / (target->getmass() + target_parent->getmass());
		double angleb = atan2(target_parent->gety() - target->gety(), target_parent->getx() - target->getx());

		middle.setPosition(target->getx() + dist * cos(angleb),
			target->gety() + dist * sin(angleb));
		window.draw(middle);
		
		if (target->getType() == SMALLSTAR || target->getType() == STAR || target->getType() == BIGSTAR)
		{
			double goldilock_inner_rad = (tempConstTwo * target->getRad() * target->getRad() * target->temp()) / inner_goldi_temp;
			double goldilock_outer_rad = (tempConstTwo * target->getRad() * target->getRad() * target->temp()) / outer_goldi_temp;

			sf::CircleShape g(goldilock_inner_rad);
			g.setPointCount(60);
			g.setPosition(sf::Vector2f(target->getx(), target->gety()));
			g.setOrigin(goldilock_inner_rad, goldilock_inner_rad);
			g.setOutlineThickness(goldilock_outer_rad - goldilock_inner_rad);
			g.setFillColor(sf::Color(0, 0, 0, 0));
			g.setOutlineColor(sf::Color(0, 200, 0, goldi_strength));
			window.draw(g);
		}
		if (target_parent->fusionEnergy() > 0)
		{
			const auto goldilock_info = target_parent->getGoldilockInfo();

			sf::CircleShape g(goldilock_info.min_rad);
			g.setPointCount(60);
			g.setPosition(sf::Vector2f(target_parent->getx(), target_parent->gety()));
			g.setOrigin(goldilock_info.min_rad, goldilock_info.min_rad);
			g.setOutlineThickness(goldilock_info.max_rad - goldilock_info.min_rad);
			g.setFillColor(sf::Color(0, 0, 0, 0));
			g.setOutlineColor(sf::Color(0, 200, 0, goldi_strength));
			window.draw(g);
		}
	}
	
	if (target->getLife().getTypeEnum() >= 6) {
		for (const auto & planet : space.planets)
		{
			if (planet.getLife().getId() == target->getLife().getId())
			{
				sf::Vertex q[] =
				{
					sf::Vertex(sf::Vector2f(planet.getx(), planet.gety()),target->getLife().getCol()),
					sf::Vertex(pos,target->getLife().getCol())
				};
				window.draw(q, 2, sf::Lines);

				sf::CircleShape indicator(planet.getRad() + 5);
				indicator.setPosition(sf::Vector2f(planet.getx(), planet.gety()));
				indicator.setOrigin(planet.getRad() + 5, planet.getRad() + 5);
				indicator.setFillColor(sf::Color(0, 0, 0, 0));
				indicator.setOutlineColor(planet.getLife().getCol());
				indicator.setOutlineThickness(3.*space.click_and_drag_handler.get_zoom());
				window.draw(indicator);
			}
		}
	}
	
	text.setPosition(pos.x + 1.5 * target->getRad(), 
						pos.y + 1.5 * target->getRad());

	const auto selected_temp_unit = static_cast<TemperatureUnit>(space.temperatureUnitSelector->getSelectedIndex());

	text.setString(target->getName() + 
		"\nType: " + target->getTypeString(target->getType())+ 
		"\nRadius: " + std::to_string(static_cast<int>(target->getRad())) +
		"\nMass: " + std::to_string(static_cast<int>(target->getmass())) +
		"\nSpeed: " + std::to_string(std::hypot(target->getxv(), target->getyv())) + 
		"\nTemperature: " + Space::temperature_info_string(target->getTemp(), selected_temp_unit));

	if (target_parent)
		text.setString(text.getString() +
			"\nDistance: " + std::to_string(static_cast<int>(std::hypot(target->getx() - target_parent->getx(),
																			target->gety() - target_parent->gety()))));

	if (target->getType() == TERRESTIAL)
	{
		//FINDING GREENHOUSE EFFECT
		const auto dTherEnergy = target->thermalEnergy() - target->thermalEnergy() / (1 + greenHouseEffectMult * target->getCurrentAtmosphere());
		const auto dTemp = dTherEnergy / (target->getmass() * target->getTCap());
		std::string dTempString;
		if (selected_temp_unit == TemperatureUnit::KELVIN)
			dTempString = Space::temperature_info_string(dTemp, selected_temp_unit);
		else
			dTempString = Space::temperature_info_string(dTemp + 273.15, selected_temp_unit);

		text.setString(text.getString() + 
			"\n\nAtmo: " + std::to_string((int)target->getCurrentAtmosphere()) + " / " + std::to_string((int)target->getAtmospherePotensial()) + "kPa \nGreenhouse Effect: " + dTempString);
		if (target->getLife().getTypeEnum() == 0) 
			text.setString(text.getString() + "\n\n" + target->getFlavorTextLife());
	}

	if (target->getLife().getTypeEnum() != 0)
	{
		text.setString(text.getString() + "\n\nBiomass: " + std::to_string((int)target->getLife().getBmass()) + "MT");
		if (target->getLife().getTypeEnum() > 3) text.setString(text.getString() + "\n" + target->getLife().getDesc() + " (" + target->getLife().getType() + ")\n" + target->getFlavorTextLife());
		else text.setString(text.getString() + "\n" + target->getLife().getType() + "\n" + target->getFlavorTextLife());
	}
	text.setColor(sf::Color(255, 255, 255));
	text.setScale(space.click_and_drag_handler.get_zoom() * 0.5f * sf::Vector2f{ 1.f, 1.f });

	window.draw(text);
}
