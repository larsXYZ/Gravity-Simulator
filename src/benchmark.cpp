#include "space.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

int main(int argc, char* argv[]) {
    try {
        // Default values
        int num_planets = 250;
        int iterations = 25;

        // Parse command line arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "--objects" || arg == "-n" || arg == "-o") && i + 1 < argc) {
                num_planets = std::atoi(argv[++i]);
            } else if ((arg == "--iterations" || arg == "-i") && i + 1 < argc) {
                iterations = std::atoi(argv[++i]);
            }
        }

        // Initialize backend to satisfy TGUI requirements
        sf::RenderWindow dummy_window(sf::VideoMode(800, 600), "Benchmark Dummy");
        tgui::Gui gui{dummy_window};

        std::cout << "Initializing benchmark with " << num_planets << " planets..." << std::endl;

        Space space;
        
        for (int i = 0; i < num_planets; ++i) {
            double mass = 100.0 + (i % 10) * 10.0;
            double x = (i % 50) * 100.0 - 2500.0; 
            double y = (i / 50) * 100.0 - 2500.0;
            double vx = (i % 3) - 1.0; 
            double vy = ((i / 3) % 3) - 1.0;
            
            space.addPlanet(Planet(mass, x, y, vx, vy));
        }
        
        space.flushPlanets(); 

        std::cout << "Running simulation for " << iterations << " iterations..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            if (iterations > 0) {
                 if (iterations < 10 || i % (iterations / 10) == 0) {
                     std::cout << "\rProgress: " << (i * 100 / iterations) << "%" << std::flush;
                 }
            }
            space.update();
        }
        std::cout << "\rProgress: 100%" << std::endl;
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        std::cout << "Time elapsed: " << elapsed.count() << " seconds" << std::endl;
        if (iterations > 0)
            std::cout << "Average time per iteration: " << (elapsed.count() / iterations) * 1000.0 << " ms" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }

    return 0;
}