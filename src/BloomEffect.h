#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class BloomEffect
{
	sf::Shader brightExtract;
	sf::Shader blur;
	sf::Shader combine;

	sf::RenderTexture sceneTexture;
	sf::RenderTexture brightTexture;
	sf::RenderTexture blurTempA;
	sf::RenderTexture blurTempB;

	bool available = false;
	unsigned int width = 0;
	unsigned int height = 0;

public:
	bool init(unsigned int w, unsigned int h);
	void resize(unsigned int w, unsigned int h);
	[[nodiscard]] bool isAvailable() const noexcept { return available; }

	// Get the scene texture to draw into (instead of drawing to the window)
	sf::RenderTexture& getSceneTarget() { return sceneTexture; }

	// Apply bloom and draw the final result to the window
	void apply(sf::RenderWindow& window, float threshold = 0.55f, float intensity = 0.7f, int passes = 2);
};
