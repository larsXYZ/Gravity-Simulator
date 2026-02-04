#pragma once
#include <SFML/Graphics.hpp>
#include "../CONSTANTS.h"

class SimObject {
protected:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration{0.f, 0.f};
    int id;
    double mass;
    double radius = 0.0;
    double density = 0.0;
    double tCapacity = 1;
    double tEnergy = 0.0;
public:
    SimObject(sf::Vector2f pos = {0.f, 0.f}, sf::Vector2f vel = {0.f, 0.f}, int id_ = -1)
        : position(pos), velocity(vel), id(id_), mass(0.0) {}
    virtual ~SimObject() = default;

    [[nodiscard]] virtual sf::Vector2f getPosition() const { return position; }
    [[nodiscard]] virtual sf::Vector2f getVelocity() const { return velocity; }
    [[nodiscard]] virtual sf::Vector2f getAcceleration() const { return acceleration; }
    [[nodiscard]] virtual int getId() const { return id; }
    [[nodiscard]] virtual double getMass() const { return mass; }
    [[nodiscard]] virtual double getRadius() const { return radius; }
    [[nodiscard]] virtual double getDensity() const { return density; }
    [[nodiscard]] virtual double getTemp() const noexcept { return tEnergy / (getMass() * getTCap()); }
    [[nodiscard]] virtual double getTCap() const noexcept { return tCapacity; }
    [[nodiscard]] virtual double getThermalEnergy() const noexcept { return getTemp() * getMass() * getTCap(); }
    
    float getx() const { return position.x; }
    float gety() const { return position.y; }
    float getxv() const { return velocity.x; }
    float getyv() const { return velocity.y; }

    void clampTemperature() noexcept {
        if (mass <= 0) return;
        double max_energy = MAX_TEMP * mass * tCapacity;
        if (tEnergy > max_energy) tEnergy = max_energy;
        else if (tEnergy < 0) tEnergy = 0;
    }

    virtual void setPosition(sf::Vector2f pos) { position = pos; }
    virtual void setVelocity(sf::Vector2f vel) { velocity = vel; }
    virtual void setAcceleration(sf::Vector2f acc) { acceleration = acc; }
    virtual void setId(int id_) { id = id_; }
    virtual void setMass(double m) { mass = m; }
    virtual void setRadius(double r) { radius = r; }
    virtual void setDensity(double d) { density = d; }
    virtual void setTemp(double t) noexcept { 
        tEnergy = getMass() * t * tCapacity; 
        clampTemperature();
    }
    virtual void increaseThermalEnergy(double e) noexcept { setTemp(getTemp() + e / (getMass() * getTCap())); }
    virtual void setTCap(double cap) noexcept { tCapacity = cap; }
    virtual void setThermalEnergy(double e) noexcept { 
        tEnergy = e; 
        clampTemperature();
    }

    virtual void update(double timestep) = 0;
    virtual void render(sf::RenderWindow& window) const = 0;
};
