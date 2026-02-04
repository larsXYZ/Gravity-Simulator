#include "Bound.h"

#include "CONSTANTS.h"

Bound::Bound()
{
    indicator.setPosition(sf::Vector2f(0.0f, 0.0f));
    indicator.setOrigin(START_RADIUS, START_RADIUS);
    indicator.setRadius(static_cast<float>(START_RADIUS));
    indicator.setFillColor(sf::Color::Transparent);
    indicator.setOutlineColor(sf::Color(255, 0, 0, 50));
    indicator.setOutlineThickness(10.0f);
    indicator.setPointCount(100);
}

[[nodiscard]] sf::Vector2f Bound::getPos() const
{
    return indicator.getPosition();
}

void Bound::setPos(const sf::Vector2f& p)
{
    indicator.setPosition(p);
}

void Bound::setRad(const float r)
{
    indicator.setRadius(r);
    indicator.setOrigin(r, r);
}

[[nodiscard]] float Bound::getRadius() const
{
    return indicator.getRadius();
}

void Bound::setActiveState(const bool state)
{
    is_active = state;
}

[[nodiscard]] bool Bound::isActive() const
{
    return is_active;
}

[[nodiscard]] bool Bound::isOutside(const sf::Vector2f& p) const
{
    const auto pos = getPos();
    const float dx = p.x - pos.x;
    const float dy = p.y - pos.y;
    return std::hypot(dx, dy) > indicator.getRadius();
}

void Bound::render(sf::RenderWindow& window, const float zoom)
{
    indicator.setOutlineThickness(40.0f * zoom);
    window.draw(indicator);
}