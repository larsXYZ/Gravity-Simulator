#include "space.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

namespace LauncherConfig
{
    const sf::Vector2i WindowSize = {300, 195};
    const sf::Color BackgroundColor = sf::Color(20, 20, 20);
    const int FramerateLimit = 10;
    const std::string FontPath = "sansation.ttf";
    
    namespace Title
    {
        const sf::Vector2f Position = {-5, 0};
        const int CharacterSize = 30;
        const std::string Text = " GRAVITY SIMULATOR ";
    }
    
    namespace Version
    {
        const sf::Vector2f Position = {247, 33};
        const int CharacterSize = 25;
        const std::string Text = "v1.2";
        const sf::Color Color = sf::Color(252, 240, 3);
    }
}


void start(sf::RenderWindow& settingScreen, tgui::ListBox::Ptr resolutionList, tgui::ListBox::Ptr windowModeList, tgui::EditBox::Ptr customResX, tgui::EditBox::Ptr customResY);

std::string getSettingsPath()
{
	const char* appData = std::getenv("APPDATA");
	if (!appData)
		return "settings.txt";

	std::filesystem::path dir = std::filesystem::path(appData) / "Gravity-Simulator";

	if (!std::filesystem::exists(dir))
	{
		try
		{
			std::filesystem::create_directories(dir);
		}
		catch (...)
		{
			return "settings.txt";
		}
	}

	return (dir / "settings.txt").string();
}

void getPrevSettings(tgui::EditBox::Ptr& customResX, tgui::EditBox::Ptr& customResY, tgui::ListBox::Ptr& resolutionList, tgui::ListBox::Ptr& windowModeList)
{
	std::ifstream file;
	file.open(getSettingsPath());

	if (file.fail())
		return;
	
	std::string m;
	std::string x;
	std::string y;
	
	file >> m;
	file >> x;
	file >> y;

	file.close();

	if (m == "w")
		windowModeList->setSelectedItemByIndex(1);

	customResX->setText(x);
	customResY->setText(y);

	const auto& items = resolutionList->getItems();
	if (auto match = std::find(items.begin(), items.end(), x + " x " + y); 
		match != items.end())
		resolutionList->setSelectedItem(*match);
	else
		resolutionList->setSelectedItem("CUSTOM");
}

void saveSettings(int x, int y, bool fullscreen)
{
	std::ofstream file;
	file.open(getSettingsPath());

	if (file.fail())
		return;

	if (fullscreen) 
		file << "f" << " " << x << " " << y;
	else
		file << "w" << " " << x << " " << y;

	file.close();
}

void setup(sf::RenderWindow& s, sf::Text& t, sf::Font& tf, sf::Text& v, tgui::ListBox::Ptr& resolutionList, tgui::ListBox::Ptr& windowModeList, tgui::EditBox::Ptr& customResX, tgui::EditBox::Ptr& customResY, tgui::Button::Ptr& b, tgui::Gui& sg)
{
	tf.loadFromFile(LauncherConfig::FontPath);

	//WINDOW
	s.setFramerateLimit(LauncherConfig::FramerateLimit);

	//TITLE
	t.setFont(tf);
	t.setPosition(LauncherConfig::Title::Position);
	t.setStyle(sf::Text::Underlined);
	t.setString(LauncherConfig::Title::Text);
	t.setCharacterSize(LauncherConfig::Title::CharacterSize);

	//VERSIONTEXT
	v.setFont(tf);
	v.setFillColor(LauncherConfig::Version::Color);
	v.setPosition(LauncherConfig::Version::Position);
	v.setString(LauncherConfig::Version::Text);
	v.setCharacterSize(LauncherConfig::Version::CharacterSize);

	//GUI
	tgui::Font font(LauncherConfig::FontPath);
	sg.setFont(font);
	sg.add(resolutionList);
	sg.add(windowModeList);
	sg.add(customResX);
	sg.add(customResY);
	sg.add(b);

	//VIDEOMODE
	windowModeList->getScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
	windowModeList->setPosition(20, 80);
	windowModeList->setItemHeight(15);
	windowModeList->setSize(95, 30);
	windowModeList->setTextSize(15);
	windowModeList->addItem("FULLSCREEN");
	windowModeList->addItem("WINDOWED");
	windowModeList->setSelectedItemByIndex(0);

	//RESOLUTIONS
	resolutionList->getScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
	resolutionList->setPosition(20, 115);
	resolutionList->setItemHeight(15);
	resolutionList->setTextSize(15);

	resolutionList->addItem("2560 x 1440");
	resolutionList->addItem("1920 x 1080");
	resolutionList->addItem("1366 x 768");
	resolutionList->addItem("CUSTOM");
	resolutionList->setSize(95, resolutionList->getItemCount() * 15);
	resolutionList->setFocusable(false);

	//CUSTOM RESOLUTION
	customResX->setDefaultText("X");
	customResX->setPosition(125, 140);
	customResX->setSize(50, 15);
	customResX->setTextSize(15);
	customResX->setMaximumCharacters(4);
	
	customResY->setDefaultText("Y");
	customResY->setPosition(125, 160);
	customResY->setSize(50, 15);
	customResY->setTextSize(15);
	customResY->setMaximumCharacters(4);

	//STARTBUTTON
	b->setPosition(185, 140);
	b->setSize(95, 35);
	b->setText("START");
	b->setTextSize(15);
	b->onPress([&s, resolutionList, windowModeList, customResX, customResY](){start(s, resolutionList, windowModeList, customResX, customResY); });
}

void start(sf::RenderWindow& settingScreen, tgui::ListBox::Ptr resolutionList, tgui::ListBox::Ptr windowModeList, tgui::EditBox::Ptr customResX, tgui::EditBox::Ptr customResY)
{
	int x = 640;
	int y = 480;

	if (resolutionList->getSelectedItem() == "2560 x 1440")
	{
		x = 2560;
		y = 1440;
	}
	else if (resolutionList->getSelectedItem() == "1920 x 1080")
	{
		x = 1920;
		y = 1080;
	}
	else if (resolutionList->getSelectedItem() == "1366 x 768")
	{
		x = 1366;
		y = 768;
	}
	else if (resolutionList->getSelectedItem() == "CUSTOM")
	{
		x = Space::convertStringToDouble(customResX->getText().toStdString());
		y = Space::convertStringToDouble(customResY->getText().toStdString());
	}

	//STARTING
	const auto fullscreen = (windowModeList->getSelectedItemIndex() == 0);
	saveSettings(x, y, fullscreen);

	settingScreen.close();

	Space space;
	space.runSim({x, y}, fullscreen);
	exit(0);
}

void evaluateCustomResolutionInputsVisibility(tgui::ListBox::Ptr resSetup,
	tgui::EditBox::Ptr customResX,
	tgui::EditBox::Ptr customResY)
{
	customResX->setVisible(true);
	customResY->setVisible(true);
	if (resSetup->getSelectedItem() != "CUSTOM")
	{
		customResX->setVisible(false);
		customResY->setVisible(false);
	}
}

void evaluateStartButtonVisibility(tgui::ListBox::Ptr resSetup,
	tgui::EditBox::Ptr customResX,
	tgui::EditBox::Ptr customResY,
	tgui::Button::Ptr startButton)
{
	startButton->setVisible(true);
	if (resSetup->getSelectedItem() == "" || (resSetup->getSelectedItem() == "CUSTOM" && (customResX->getText() == "" || customResY->getText() == "")))
		startButton->setVisible(false);
}


void runLauncher()
{
	sf::RenderWindow settingScreen;
	settingScreen.create(sf::VideoMode(LauncherConfig::WindowSize.x, LauncherConfig::WindowSize.y), "", sf::Style::None);
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
			settingScreen.clear(LauncherConfig::BackgroundColor);
			settingGUI.handleEvent(event);

			if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			{
				settingScreen.close();
				exit(0);
			}
		}

		evaluateCustomResolutionInputsVisibility(resSetup, customResX, customResY);
		evaluateStartButtonVisibility(resSetup, customResX, customResY, startButton);

		settingScreen.draw(title);
		settingScreen.draw(version);
		settingGUI.draw();
		settingScreen.display();

		if (startButton->isVisible() && sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
			start(settingScreen, resSetup, modeSetup, customResX, customResY);
	}
}

#ifdef _DEBUG
int main(int argc, char** argv)
{
    runLauncher();
}
#else
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    runLauncher();
}
#endif