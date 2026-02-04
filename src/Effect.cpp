#include "Effect.h"

Effect::Effect(sf::Vector2f p, double s, int /*i*/, sf::Vector2f v, int l)
	: pos_(p), size_(s), vel_(v), levelId_(l)
{
}

void Effect::setPos(const sf::Vector2f& p)
{
	pos_ = p;
}

void Effect::setSize(double s)
{
	size_ = s;
}

void Effect::setColor(const sf::Color& c)
{
	col_ = c;
}

sf::Vector2f Effect::getPos() const
{
	return pos_;
}

double Effect::getSize() const
{
	return size_;
}

sf::Color Effect::getColor() const
{
	return col_;
}

void Effect::setID(int i)
{
	id_ = i;
}

int Effect::getID() const
{
	return id_;
}

int Effect::getAge(double t)
{
	timeExist_ += static_cast<int>(t);
	return timeExist_;
}

void Effect::move(int t)
{
	setPos(pos_ + vel_ * static_cast<float>(t));
}

int Effect::maxLifeTime() const
{
	return levelId_;
}

void Effect::setVel(const sf::Vector2f& a)
{
	vel_ += a;
}

void Explosion::render(sf::RenderWindow& w)
{
	const auto age = getAge(0);

	// LIGHT - Flash
	if (age <= 2)
	{
		sf::Color col = sf::Color(255, 255, 200);
		col.a = EXPLOSION_LIGHT_START_STRENGTH;
		sf::VertexArray vertexArr(sf::TrianglesFan);
		vertexArr.append(sf::Vertex(getPos(), col));
		col.a = 0;

		const double deltaAng = 2.0 * PI / static_cast<double>(LIGHT_NUMBER_OF_VERTECES);
		double ang = 0;
		auto rad = static_cast<double>(getSize()) * getSize() * EXPLOSION_FLASH_SIZE;

		if (age == 2)
		{
			rad /= 2.0;
		}

		for (int nr = 1; nr < LIGHT_NUMBER_OF_VERTECES; ++nr)
		{
			sf::Vector2f pos(getPos().x + std::cos(ang) * rad, getPos().y + std::sin(ang) * rad);
			vertexArr.append(sf::Vertex(pos, col));
			ang += deltaAng;
		}
		vertexArr.append(sf::Vertex(sf::Vector2f(getPos().x + rad, getPos().y), col));
		w.draw(vertexArr);
	}

	const float lifeRatio = static_cast<float>(age) / static_cast<float>(maxLifeTime());
	const float expansion = static_cast<float>(getSize()) * lifeRatio;

	// Main Body - Layered
	// Inner (White/Hot)
	sf::CircleShape core(expansion * 0.5f);
	core.setFillColor(sf::Color(255, 255, 220, static_cast<sf::Uint8>(255 * (1.0f - lifeRatio))));
	core.setPosition(getPos());
	core.setOrigin(expansion * 0.5f, expansion * 0.5f);
	w.draw(core);

	// Middle (Yellow/Orange)
	sf::CircleShape mid(expansion * 0.8f);
	mid.setFillColor(sf::Color(255, 150, 0, static_cast<sf::Uint8>(150 * (1.0f - lifeRatio))));
	mid.setPosition(getPos());
	mid.setOrigin(expansion * 0.8f, expansion * 0.8f);
	w.draw(mid);

	// Shockwave Ring
	sf::CircleShape shockwave(expansion * 1.2f);
	shockwave.setFillColor(sf::Color::Transparent);
	shockwave.setOutlineColor(sf::Color(200, 200, 255, static_cast<sf::Uint8>(100 * (1.0f - lifeRatio))));
	shockwave.setOutlineThickness(2.0f);
	shockwave.setPosition(getPos());
	shockwave.setOrigin(expansion * 1.2f, expansion * 1.2f);
	w.draw(shockwave);
}

void StarshineFade::render(sf::RenderWindow& w)
{
	const float lifeRatio = static_cast<float>(getAge(0)) / static_cast<float>(maxLifeTime());
	if (lifeRatio >= 1.0f)
	{
		return;
	}

	sf::Color col = color_;
	const float fade = 1.0f - lifeRatio;

	// Fading long range light
	sf::Color lrCol = col;
	lrCol.a = static_cast<sf::Uint8>(50 * fade);

	// Fading short range light
	sf::Color srCol = col;
	srCol.a = static_cast<sf::Uint8>(250 * std::max(0.0f, 1.0f - 7.0f * lifeRatio));
}