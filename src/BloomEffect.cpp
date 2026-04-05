#include "BloomEffect.h"

bool BloomEffect::init(unsigned int w, unsigned int h)
{
	if (!sf::Shader::isAvailable())
	{
		std::cerr << "BloomEffect: Shaders not available on this system" << std::endl;
		return false;
	}

	if (!brightExtract.loadFromFile("resources/shaders/bright_extract.frag", sf::Shader::Fragment))
	{
		std::cerr << "BloomEffect: Failed to load bright_extract.frag" << std::endl;
		return false;
	}

	if (!blur.loadFromFile("resources/shaders/blur.frag", sf::Shader::Fragment))
	{
		std::cerr << "BloomEffect: Failed to load blur.frag" << std::endl;
		return false;
	}

	if (!combine.loadFromFile("resources/shaders/bloom_combine.frag", sf::Shader::Fragment))
	{
		std::cerr << "BloomEffect: Failed to load bloom_combine.frag" << std::endl;
		return false;
	}

	resize(w, h);
	available = true;
	std::cout << "BloomEffect: Initialized (" << w << "x" << h << ")" << std::endl;
	return true;
}

void BloomEffect::resize(unsigned int w, unsigned int h)
{
	if (w == width && h == height)
		return;

	width = w;
	height = h;

	// Scene at full resolution
	sceneTexture.create(w, h);
	sceneTexture.setSmooth(true);

	// Bloom textures at half resolution for performance + wider blur
	unsigned int bw = w / 2;
	unsigned int bh = h / 2;
	brightTexture.create(bw, bh);
	brightTexture.setSmooth(true);
	blurTempA.create(bw, bh);
	blurTempA.setSmooth(true);
	blurTempB.create(bw, bh);
	blurTempB.setSmooth(true);
}

void BloomEffect::apply(sf::RenderWindow& window, float threshold, float intensity, int passes)
{
	sceneTexture.display();

	unsigned int bw = width / 2;
	unsigned int bh = height / 2;

	// Step 1: Extract bright pixels
	brightExtract.setUniform("texture", sceneTexture.getTexture());
	brightExtract.setUniform("threshold", threshold);

	brightTexture.clear(sf::Color::Black);
	sf::Sprite sceneSprite(sceneTexture.getTexture());
	sceneSprite.setScale(
		static_cast<float>(bw) / width,
		static_cast<float>(bh) / height
	);
	brightTexture.draw(sceneSprite, &brightExtract);
	brightTexture.display();

	// Step 2: Ping-pong gaussian blur
	sf::RenderTexture* src = &brightTexture;
	sf::RenderTexture* dst = &blurTempA;

	for (int i = 0; i < passes; i++)
	{
		// Horizontal blur
		blur.setUniform("texture", src->getTexture());
		blur.setUniform("direction", sf::Glsl::Vec2(1.0f / bw, 0.0f));
		dst->clear(sf::Color::Black);
		sf::Sprite s(src->getTexture());
		dst->draw(s, &blur);
		dst->display();

		// Vertical blur
		sf::RenderTexture* next = (dst == &blurTempA) ? &blurTempB : &blurTempA;
		blur.setUniform("texture", dst->getTexture());
		blur.setUniform("direction", sf::Glsl::Vec2(0.0f, 1.0f / bh));
		next->clear(sf::Color::Black);
		sf::Sprite s2(dst->getTexture());
		next->draw(s2, &blur);
		next->display();

		src = next;
		dst = (src == &blurTempA) ? &blurTempB : &blurTempA;
	}

	// Step 3: Combine scene + bloom and draw to window
	combine.setUniform("scene", sceneTexture.getTexture());
	combine.setUniform("bloom", src->getTexture());
	combine.setUniform("bloomIntensity", intensity);

	// Draw to window in default view (pixel coordinates)
	sf::View origView = window.getView();
	window.setView(window.getDefaultView());

	sf::Sprite finalSprite(sceneTexture.getTexture());
	window.draw(finalSprite, &combine);

	window.setView(origView);
}
