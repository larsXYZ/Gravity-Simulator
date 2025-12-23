#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include "CONSTANTS.h"

struct DistanceCalculationResult
{
	double dx{ 0.0 };
	double dy{ 0.0 };
	double dist{ 0.0 };
	double rad_dist{ 0.0 };
};

struct Acceleration2D
{
	float x{ 0.0 };
	float y{ 0.0 };
};

namespace PhysicsUtils {

template <typename T1, typename T2>
DistanceCalculationResult calculateDistance(const T1& p1, const T2& p2)
{
    DistanceCalculationResult result;
    result.dx = p2.getPosition().x - p1.getPosition().x;
    result.dy = p2.getPosition().y - p1.getPosition().y;
    result.dist = std::hypot(result.dx, result.dy);
    result.rad_dist = p1.getRadius() + p2.getRadius();
    return result;
}

// Specialization for types that don't have getPosition/getRadius but have position/radius members
struct PhysicsBody
{
	sf::Vector2f position;
	sf::Vector2f velocity;
	double mass;
	double radius;

    sf::Vector2f getPosition() const { return position; }
    double getRadius() const { return radius; }
    double getMass() const { return mass; }
};

template <typename T>
double accumulate_acceleration(const DistanceCalculationResult& distance_info,
    Acceleration2D& acceleration,
    const T& other_planet)
{
    const auto A = G * other_planet.getMass() / std::max(distance_info.dist * distance_info.dist, 0.01);
    const auto angle = atan2(distance_info.dy, distance_info.dx);

    acceleration.x += static_cast<float>(cos(angle) * A);
    acceleration.y += static_cast<float>(sin(angle) * A);

    return A;
}

// Specialization for PhysicsBody which uses .mass
inline double accumulate_acceleration_body(const DistanceCalculationResult& distance_info,
    Acceleration2D& acceleration,
    double other_mass)
{
    const auto A = G * other_mass / std::max(distance_info.dist * distance_info.dist, 0.01);
    const auto angle = atan2(distance_info.dy, distance_info.dx);

    acceleration.x += static_cast<float>(cos(angle) * A);
    acceleration.y += static_cast<float>(sin(angle) * A);

    return A;
}

// Helper for segment-circle intersection
inline bool segmentIntersectsCircle(sf::Vector2f start, sf::Vector2f end, sf::Vector2f circlePos, float radius, sf::Vector2f& intersection)
{
    sf::Vector2f d = end - start;
    sf::Vector2f f = start - circlePos;
    
    float a = d.x*d.x + d.y*d.y;
    float b = 2*(f.x*d.x + f.y*d.y);
    float c = f.x*f.x + f.y*f.y - radius*radius;

    float discriminant = b*b - 4*a*c;
    if(discriminant < 0) return false;

    discriminant = sqrt(discriminant);
    float t1 = (-b - discriminant)/(2*a);
    float t2 = (-b + discriminant)/(2*a);

    if(t1 >= 0 && t1 <= 1)
    {
        intersection = start + t1*d;
        return true;
    }
    if(t2 >= 0 && t2 <= 1)
    {
        intersection = start + t2*d;
        return true;
    }
    return false;
}

}
