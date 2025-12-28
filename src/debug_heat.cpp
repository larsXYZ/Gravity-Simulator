#include "space.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>

int main() {
    try {
        // Initialize backend to satisfy TGUI requirements
        sf::RenderWindow dummy_window(sf::VideoMode(800, 600), "Heat Debug");
        tgui::Gui gui{dummy_window};

        std::cout << "Initializing Heat Debug..." << std::endl;

        Space space;
        
        // Add a Star (Mass 1000) at origin
        Planet star(1000.0, 0, 0, 0, 0);
        star.setTemp(6000.0); // Hot star
        int star_id = space.addPlanet(std::move(star));
        
        // Add a Planet (Mass 10) at distance 100
        Planet planet(10.0, 100, 0, 0, 0);
        planet.setTemp(300.0); // Earth-like start
        int planet_id = space.addPlanet(std::move(planet));
        
        space.flushPlanets(); 

        Planet* pStar = space.findPlanetPtr(star_id);
        Planet* pPlanet = space.findPlanetPtr(planet_id);

        if (!pStar || !pPlanet) {
            std::cerr << "Failed to find planets!" << std::endl;
            return 1;
        }

        std::cout << "Initial Temperatures:" << std::endl;
        std::cout << "Star: " << pStar->getTemp() << " K" << std::endl;
        std::cout << "Planet: " << pPlanet->getTemp() << " K" << std::endl;

        std::cout << "\nRunning simulation for 100 iterations..." << std::endl;
        
        for (int i = 1; i <= 100; ++i) {
            space.update();
            if (i % 10 == 0) {
                std::cout << "Iteration " << i 
                          << ": Star Temp = " << pStar->getTemp() 
                          << " K, Planet Temp = " << pPlanet->getTemp() << " K" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
