#include "object_info.h"

#include "Space.h"
#include "roche_limit.h"
#include <iomanip>
#include <sstream>

// Helper for double to string with precision
static std::string d2s(double d, int precision = 2) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << d;
    return ss.str();
}

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
	if (m_space) m_space->trail.clear();
	if (panel)
		panel->setVisible(false);
}

void ObjectInfo::activate(int new_target_id)
{
	target_id = new_target_id;
	if (m_space) m_space->trail.clear();
}

void ObjectInfo::set_visible(bool visible)
{
	if (panel)
	{
		panel->setVisible(visible && target_id != -1);
		if (!panel->isVisible() && m_space)
			m_space->trail.clear();
	}
}

void ObjectInfo::setup(Space& space, tgui::Gui& gui)
{
	m_space = &space;

	panel = tgui::Panel::create();
	panel->setSize(220, 260);
	panel->setPosition("100% - 230", 100);
	panel->setVisible(false);
	panel->getRenderer()->setBackgroundColor(sf::Color(255, 255, 255, 200));
	panel->getRenderer()->setBorderColor(sf::Color::Black);
	panel->getRenderer()->setBorders(1);
	gui.add(panel);

		float y = 5;
		float labelWidth = 75;
		float boxX = 80;
		float boxWidth = 130;
		float rowHeight = 30;
	
		auto addRow = [&](const std::string& labelText, tgui::EditBox::Ptr& box, bool numeric = true) {
			auto label = tgui::Label::create(labelText);
			label->setPosition(5, y + 4); 
			label->setSize(labelWidth, 22);
			label->getRenderer()->setTextColor(sf::Color::Black);
			label->setTextSize(13);
			panel->add(label);
	
			box = tgui::EditBox::create();
			box->setPosition(boxX, y);
			box->setSize(boxWidth, 24);		box->setTextSize(13);
		if (numeric)
			box->setInputValidator(tgui::EditBox::Validator::Float);
		panel->add(box);
		y += rowHeight;
	};

	addRow("Name:", nameBox, false);
	nameBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				p->setName(val.toStdString());
			}
		}
	});

	addRow("Mass:", massBox);
	massBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					p->setMass(std::stod(val.toStdString()));
					p->updateRadiAndType(); // Update radius and type based on mass
				} catch (...) {}
			}
		}
	});

	addRow("Temp (K):", tempBox);
	tempBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					p->setTemp(std::stod(val.toStdString()));
				} catch (...) {}
			}
		}
	});

	addRow("Pos X:", xBox);
	xBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					sf::Vector2f pos = p->getPosition();
					pos.x = std::stod(val.toStdString());
					p->setPosition(pos);
				} catch (...) {}
			}
		}
	});

	addRow("Pos Y:", yBox);
	yBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					sf::Vector2f pos = p->getPosition();
					pos.y = std::stod(val.toStdString());
					p->setPosition(pos);
				} catch (...) {}
			}
		}
	});

	addRow("Vel X:", vxBox);
	vxBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					sf::Vector2f vel = p->getVelocity();
					vel.x = std::stod(val.toStdString());
					p->setVelocity(vel);
				} catch (...) {}
			}
		}
	});

	addRow("Vel Y:", vyBox);
	vyBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					sf::Vector2f vel = p->getVelocity();
					vel.y = std::stod(val.toStdString());
					p->setVelocity(vel);
				} catch (...) {}
			}
		}
	});

	addRow("Atmo:", atmoBox);
	atmoBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					p->setAtmosphere(std::stod(val.toStdString()));
				} catch (...) {}
			}
		}
	});

	addRow("Atmo Pot:", atmoPotBox);
	atmoPotBox->onReturnKeyPress([this](const tgui::String& val) {
		if (m_space && target_id != -1) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				try {
					p->setAtmospherePotensial(std::stod(val.toStdString()));
				} catch (...) {}
			}
		}
	});

	auto lifeLabel = tgui::Label::create("Life Level:");
	lifeLabel->setPosition(5, y + 4);
	lifeLabel->setSize(labelWidth, 22);
	lifeLabel->getRenderer()->setTextColor(sf::Color::Black);
	lifeLabel->setTextSize(13);
	panel->add(lifeLabel);

	lifeLevelSelector = tgui::ComboBox::create();
	lifeLevelSelector->setPosition(boxX, y);
	lifeLevelSelector->setSize(boxWidth, 24);
	lifeLevelSelector->setTextSize(13);
	lifeLevelSelector->addItem("Lifeless");
	lifeLevelSelector->addItem("Unicellular");
	lifeLevelSelector->addItem("Multicellular (S)");
	lifeLevelSelector->addItem("Multicellular (C)");
	lifeLevelSelector->addItem("Tribal");
	lifeLevelSelector->addItem("Global");
	lifeLevelSelector->addItem("Interplanetary");
	lifeLevelSelector->addItem("Colony");
	lifeLevelSelector->onItemSelect([this](const tgui::String& item) {
		if (m_space && target_id != -1 && !ignore_change_signals) {
			if (auto* p = m_space->findPlanetPtr(target_id)) {
				if (item == "Lifeless") p->setLifeLevel(lType::NONE);
				else if (item == "Unicellular") p->setLifeLevel(lType::SINGLECELL);
				else if (item == "Multicellular (S)") p->setLifeLevel(lType::MULTICELL_SIMPLE);
				else if (item == "Multicellular (C)") p->setLifeLevel(lType::MULTICELL_COMPLEX);
				else if (item == "Tribal") p->setLifeLevel(lType::INTELLIGENT_TRIBAL);
				else if (item == "Global") p->setLifeLevel(lType::INTELLIGENT_GLOBAL);
				else if (item == "Interplanetary") p->setLifeLevel(lType::INTELLIGENT_INTERPLANETARY);
				else if (item == "Colony") p->setLifeLevel(lType::COLONY);
			}
		}
	});
	panel->add(lifeLevelSelector);
	y += rowHeight;

	panel->setSize(220, y + 5);
}

bool ObjectInfo::is_focused(sf::RenderWindow& window) const
{
	if (!panel || !panel->isVisible()) return false;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		m_lastInteractionClock.restart();

	if (nameBox && nameBox->isFocused()) return true;
	if (massBox && massBox->isFocused()) return true;
	if (tempBox && tempBox->isFocused()) return true;
	if (xBox && xBox->isFocused()) return true;
	if (yBox && yBox->isFocused()) return true;
	if (vxBox && vxBox->isFocused()) return true;
	if (vyBox && vyBox->isFocused()) return true;
	if (atmoBox && atmoBox->isFocused()) return true;
	if (atmoPotBox && atmoPotBox->isFocused()) return true;
	
	if (lifeLevelSelector) {
		if (lifeLevelSelector->isFocused()) return true;
		
		// Fallback: if isFocused() doesn't work well for ComboBox in this version of TGUI
		// we check if mouse is over it while the panel is visible.
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		if (lifeLevelSelector->isMouseOnWidget(sf::Vector2f(mousePos.x, mousePos.y)))
			return true;
	}
	
	return false;
}

void ObjectInfo::update_ui_values(Space& space, sf::RenderWindow& window)
{
	if (target_id == -1 || !panel || !panel->isVisible()) return;

	Planet* target = space.findPlanetPtr(target_id);
	if (!target) {
		deactivate();
		return;
	}

	if (!nameBox->isFocused()) nameBox->setText(target->getName());
	if (!massBox->isFocused()) massBox->setText(d2s(target->getMass()));
	if (!tempBox->isFocused()) tempBox->setText(d2s(target->getTemp()));
	if (!xBox->isFocused()) xBox->setText(d2s(target->getx()));
	if (!yBox->isFocused()) yBox->setText(d2s(target->gety()));
	if (!vxBox->isFocused()) vxBox->setText(d2s(target->getxv(), 4));
	if (!vyBox->isFocused()) vyBox->setText(d2s(target->getyv(), 4));
	if (!atmoBox->isFocused()) atmoBox->setText(d2s(target->getCurrentAtmosphere()));
	if (!atmoPotBox->isFocused()) atmoPotBox->setText(d2s(target->getAtmospherePotensial()));

	const auto curr_val = lifeLevelSelector->getSelectedItemIndex();
	const auto new_val = static_cast<int>(target->getLife().getTypeEnum());
	
	// Ugly fix: If the user has interacted with the mouse in the last second, don't update the box
	bool recentlyInteracted = m_lastInteractionClock.getElapsedTime().asSeconds() < 1.0f;

	if (lifeLevelSelector && !is_focused(window) && !recentlyInteracted && curr_val != new_val)
	{
		ignore_change_signals = true;
		lifeLevelSelector->setSelectedItemByIndex(new_val);
		ignore_change_signals = false;
	}
}

void ObjectInfo::render(Space& space, sf::RenderWindow& window)
{
	Planet* target = space.findPlanetPtr(target_id);
	if (!target)
	{
		deactivate();
		return;
	}

	update_ui_values(space, window);
	
	sf::Vector2f pos(target->getx(), target->gety());
	
    // Draw indicator square
    float rectSize = target->getRadius() * 2.2f;
    if (rectSize < 10.0f) rectSize = 10.0f; // Minimum size for visibility

    sf::RectangleShape indicator(sf::Vector2f(rectSize, rectSize));
    indicator.setOrigin(rectSize / 2.0f, rectSize / 2.0f);
    indicator.setPosition(pos);
    indicator.setFillColor(sf::Color::Transparent);
    indicator.setOutlineColor(sf::Color(0, 255, 255, 100)); // Faint Cyan
    indicator.setOutlineThickness(1.0f);
    window.draw(indicator);
	
	sf::Vertex v[] =
	{
		sf::Vertex(sf::Vector2f(pos.x + 400 * target->getxv(), pos.y + 400 * target->getyv()),sf::Color::Red),
		sf::Vertex(pos, sf::Color::Red)
	};
	window.draw(v, 2, sf::Lines);
	
	if (space.get_iteration() % TRAILFREQ == 0)
		space.addTrail(pos, TRAILLIFE);

	if (!space.renderLifeAlwaysCheckBox->isChecked())
	{
		space.drawLifeVisuals(window, *target);
		space.drawCivConnections(window, *target, true);
	}

	Planet* target_parent = space.findPlanetPtr(target->getStrongestAttractorId());
	if (target_parent)
	{
		sf::Vertex g[] =
		{
			sf::Vertex(sf::Vector2f(target_parent->getx(), target_parent->gety()),sf::Color(255, 255, 0, 80)),
			sf::Vertex(sf::Vector2f(pos.x, pos.y), sf::Color(255, 255, 0, 80))
		};
		window.draw(g, 2, sf::Lines);

		double rocheRad = RocheLimit::calculateLimitRadius(target_parent->getRadius() + target->getRadius());
		sf::CircleShape omr(rocheRad);
		omr.setPosition(sf::Vector2f(target_parent->getx(), target_parent->gety()));
		omr.setOrigin(rocheRad, rocheRad);
		omr.setFillColor(sf::Color(0, 0, 0, 0));
		omr.setOutlineColor(sf::Color(255, 140, 0, 80));
		window.draw(omr);

		//CENTER OF MASS
		sf::CircleShape middle(2);
		middle.setOrigin(2, 2);
		middle.setFillColor(sf::Color(255, 255, 0, 150));

		double dist = std::hypot(target_parent->getx() - pos.x, 
									target_parent->gety() - pos.y);

		dist *= (target_parent->getMass()) / (target->getMass() + target_parent->getMass());
		double angleb = atan2(target_parent->gety() - target->gety(), target_parent->getx() - target->getx());

		middle.setPosition(target->getx() + dist * cos(angleb),
			target->gety() + dist * sin(angleb));
		window.draw(middle);
		
		if (target->getType() == SMALLSTAR || target->getType() == STAR || target->getType() == BIGSTAR)
		{
			double goldilock_inner_rad = (tempConstTwo * target->getRadius() * target->getRadius() * target->getTemp()) / inner_goldi_temp;
			double goldilock_outer_rad = (tempConstTwo * target->getRadius() * target->getRadius() * target->getTemp()) / outer_goldi_temp;

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
}