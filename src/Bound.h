#pragma once

#include <SFML/Graphics.hpp>

class Bound
{
    bool is_active = true;
    sf::CircleShape indicator;

public:
    Bound() = default;

    [[nodiscard]] sf::Vector2f getPos() const;
    void setPos(const sf::Vector2f& p);

    void setRad(float r);
    [[nodiscard]] float getRadius() const;

    void setActiveState(bool state);
    [[nodiscard]] bool isActive() const;

    [[nodiscard]] bool isOutside(const sf::Vector2f& p) const;

    void render(sf::RenderWindow& window, float zoom);
};