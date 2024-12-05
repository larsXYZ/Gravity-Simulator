#include "rom.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <TGUI/TGUI.hpp>

//FUNKSJONER

void start(tgui::ListBox::Ptr res, tgui::ListBox::Ptr mode, tgui::EditBox::Ptr c1, tgui::EditBox::Ptr c2);

void getPrevSettings(tgui::EditBox::Ptr& c1, tgui::EditBox::Ptr& c2, tgui::ListBox::Ptr& res, tgui::ListBox::Ptr& mode)
{
	std::ifstream fil;
	fil.open("prevSettings.txt");

	if (fil.fail()) return;
	
	std::string m;
	std::string x;
	std::string y;
	
	fil >> m;
	fil >> x;
	fil >> y;

	if (m == "w") mode->setSelectedItemByIndex(1);

	c1->setText(x);
	c2->setText(y);

	res->setSelectedItem("CUSTOM");
	fil.close();
}

void saveSettings(int x, int y, bool m)
{
	std::ofstream fil;
	fil.open("prevSettings.txt");

	if (fil.fail()) return;

	if (m) 	fil << "f" << " " << x << " " << y;
	else fil << "w" << " " << x << " " << y;

	fil.close();
}

void setup(sf::RenderWindow& s, sf::Text& t, sf::Font& tf, sf::Text& v, tgui::ListBox::Ptr& res, tgui::ListBox::Ptr& mode, tgui::EditBox::Ptr& c1, tgui::EditBox::Ptr& c2, tgui::Button::Ptr& b, tgui::Gui& sg)
{
	tf.loadFromFile("sansation.ttf");

	//WINDOW
	s.setFramerateLimit(10);

	//TITLE
	t.setFont(tf);
	t.setPosition(-5, 0);
	t.setStyle(sf::Text::Underlined);
	t.setString(" GRAVITY SIMULATOR ");
	t.setCharacterSize(30);

	//VERSIONTEXT
	v.setFont(tf);
	v.setColor(sf::Color(51, 255, 255));
	v.setPosition(247, 33);
	v.setString("v1.0");
	v.setCharacterSize(25);

	//GUI
	tgui::Font font("sansation.ttf");
	sg.setFont(font);
	sg.add(res);
	sg.add(mode);
	sg.add(c1);
	sg.add(c2);
	sg.add(b);

	//VIDEOMODE
	//mode->setScrollbar(nullptr);
	mode->setPosition(20, 80);
	mode->setItemHeight(15);
	mode->setSize(95, 30);
	mode->setTextSize(15);
	mode->addItem("FULLSCREEN");
	mode->addItem("WINDOWED");
	mode->setSelectedItemByIndex(0);

	//RESOLUTIONS
	//res->setScrollbar(nullptr);
	res->setPosition(20, 115);
	res->setItemHeight(15);
	res->setTextSize(15);

	res->addItem("2560 x 1440");
	res->addItem("1920 x 1080");
	res->addItem("1366 x 768");
	res->addItem("CUSTOM");
	res->setSize(95, res->getItemCount() * 15);

	//CUSTOM RESOLUTION
	c1->setDefaultText("X");
	c1->setPosition(125, 140);
	c1->setSize(50, 15);
	c1->setTextSize(15);
	c1->setMaximumCharacters(4);
	
	c2->setDefaultText("Y");
	c2->setPosition(125, 160);
	c2->setSize(50, 15);
	c2->setTextSize(15);
	c2->setMaximumCharacters(4);

	//STARTBUTTON
	b->setPosition(185, 140);
	b->setSize(95, 35);
	b->setText("START");
	b->setTextSize(15);
	b->onPress([=](){start(res, mode, c1, c2); });

}

void start(tgui::ListBox::Ptr res, tgui::ListBox::Ptr mode, tgui::EditBox::Ptr c1, tgui::EditBox::Ptr c2)
{

	//CHECKING FOR FULLSCREEN
	bool fullscreen;
	if (mode->getSelectedItemIndex() == 0) fullscreen = true;
	else fullscreen = false;

	//CHECKING FOR RESOLUTION
	int x;
	int y;

	if (res->getSelectedItem() == ("2560 x 1440"))
	{
		x = 2560;
		y = 1440;
	}
	else 	if (res->getSelectedItem() == ("1920 x 1080"))
	{
		x = 1920;
		y = 1080;
	}
	else 	if (res->getSelectedItem() == ("1366 x 768"))
	{
		x = 1366;
		y = 768;
	}
	else 	if (res->getSelectedItem() == ("CUSTOM"))
	{
		x = Rom::convertStringToDouble(c1->getText().toStdString());
		y = Rom::convertStringToDouble(c2->getText().toStdString());
	}
	else
	{
		x = 640;
		y = 480;
	}

	//STARTING
	Rom sim(x, y, fullscreen);
	saveSettings(x, y, fullscreen);
	
	sim.runSim();
	exit(0);

}

int main()
{

	sf::RenderWindow settingScreen;
	settingScreen.create(sf::VideoMode(300, 195), "", sf::Style::None);
	tgui::Gui settingGUI{ settingScreen };
	sf::Event event;
	sf::Text title;
	sf::Text version;
	sf::Font titleFont;
	tgui::ListBox::Ptr resSetup = std::make_shared<tgui::ListBox>();
	tgui::ListBox::Ptr modeSetup = std::make_shared<tgui::ListBox>();
	tgui::EditBox::Ptr customResX = std::make_shared<tgui::EditBox>();
	tgui::EditBox::Ptr customResY = std::make_shared<tgui::EditBox>();
	tgui::Button::Ptr startButton = std::make_shared<tgui::Button>();

	setup(settingScreen, title, titleFont, version, resSetup, modeSetup, customResX, customResY, startButton, settingGUI);
	getPrevSettings(customResX,customResY,resSetup,modeSetup);

	while (settingScreen.isOpen())
	{
		while (settingScreen.pollEvent(event))
		{
			settingScreen.clear(sf::Color(20, 20, 20));
			settingGUI.handleEvent(event);

			while (settingScreen.pollEvent(event))
			{
				//LUKKER VINDUET
				if (settingScreen.hasFocus())
				{
					if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) settingScreen.close();
				}
				else if (event.type == sf::Event::Closed) settingScreen.close();
			}

			//DELETE CUSTOM RESOLUTION INPUT
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace))
			{
				if (customResX->isFocused()) customResX->setText("");
				if (customResY->isFocused()) customResY->setText("");
			}

			//ENTER STARTER SIM
			if (startButton->isVisible() && sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
			{
				start(resSetup, modeSetup, customResX, customResY);
			}

			//SKJULER CUSTOM RESOLUTION OPTIONS
			if (resSetup->getSelectedItem() != "CUSTOM") customResX->setVisible(false), customResY->setVisible(false);
			else customResX->setVisible(true), customResY->setVisible(true);

			//SKJULER START BUTTON
			if (resSetup->getSelectedItem() == "") startButton->setVisible(false);
			else if (resSetup->getSelectedItem() == "CUSTOM" && (customResX->getText() == "" || customResY->getText() == "")) startButton->setVisible(false);
			else startButton->setVisible(true);

			settingScreen.draw(title);
			settingScreen.draw(version);
			settingGUI.draw();
			settingScreen.display();
		}
	}
}
