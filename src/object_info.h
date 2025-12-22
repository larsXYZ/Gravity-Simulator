#pragma once

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

class Space;

class ObjectInfo
{
	int target_id{ -1 };
	sf::Font font;
	sf::Text text;
	
	tgui::Panel::Ptr panel;
	tgui::EditBox::Ptr nameBox;
	tgui::EditBox::Ptr massBox;
	tgui::EditBox::Ptr tempBox;
	tgui::EditBox::Ptr xBox;
	tgui::EditBox::Ptr yBox;
	tgui::EditBox::Ptr vxBox;
	tgui::EditBox::Ptr vyBox;
	tgui::EditBox::Ptr atmoBox;
	tgui::EditBox::Ptr atmoPotBox;
	
	bool ignore_change_signals{ false };
	Space* m_space{ nullptr };

public:
	ObjectInfo();

	bool is_active() const;
	int get_target_id() const { return target_id; }
	void deactivate();
	void activate(int new_target_id);
	void set_visible(bool visible);
	void render(Space& space, sf::RenderWindow & window);
	
	void setup(Space& space, tgui::Gui& gui);
	void update_ui_values(Space& space);
	
	bool is_focused() const;
};