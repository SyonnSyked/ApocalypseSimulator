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
    for (const auto cityEntity : world.view<City>()) {
        const auto &city = world.get<City>(cityEntity);
        std::cout << "City: " << city.cityName << '\n'
                  << "  Population: " << city.population << '\n'
                  << "  Size: " << city.sizeSQKM << " sq km\n"
                  << "  Districts:\n";

        for (const auto districtEntity : world.view<District>()) {
            const auto &district = world.get<District>(districtEntity);
            if (district.city != cityEntity) continue;
            std::cout << "    " << district.districtName << '\n'
                      << "      Population: " << district.population << '\n'
                      << "      Size: " << district.sizeSQKM << " sq km\n"
                      << "      Shelters: " << district.shelterAmount << '\n'
                      << "      Food: " << district.foodSupply << '\n'
                      << "      Water: " << district.waterSupply << '\n';
        }
        std::cout << '\n';
    }

    return 0;
}
