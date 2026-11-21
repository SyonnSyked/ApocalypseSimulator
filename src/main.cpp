#include "Components.h"
#include "SimulatorConfig.h"
#include "World.h"

#include <exception>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [config file]\n";
        return 1;
    }
    SimulatorConfig config;
    try {
        config = LoadConfig(argc == 2 ? argv[1] : "../../config/simulator.cfg");
    } catch (const std::exception &error) {
        std::cerr << "Config error: " << error.what() << '\n';
        return 1;
    }

    entt::registry world;
    StartSimulatedWorld(world, config);
    for (const auto entity : world.view<City>()) {
        const auto &city = world.get<City>(entity);
        std::cout << "City: " << city.cityName << '\n'
                  << "Size: " << city.sizeSQKM << " sq km\n"
                  << "Population: " << city.population << '\n';
    }

    for (const auto entity : world.view<District>()) {
        const auto &district = world.get<District>(entity);
        std::cout << "  " << world.get<City>(district.city).cityName << " / "
                  << district.districtName << ": " << district.population << " people, "
                  << district.sizeSQKM << " sq km, " << district.shelterAmount << " shelters, "
                  << district.foodSupply << " food, " << district.waterSupply << " water\n";
    }

    return 0;
}
