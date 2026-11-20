#include "Components.h"
#include "Population.h"
#include "SimulatorConfig.h"

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
        config = LoadConfig(argc == 2 ? argv[1] : "config/simulator.cfg");
    } catch (const std::exception &error) {
        std::cerr << "Config error: " << error.what() << '\n';
        return 1;
    }
    entt::registry world;
    const auto city = world.create();
    world.emplace<City>(city, config.cityName.c_str(), size_t{0}, config.sizeSQKM);

    CreatePopulation(world, city, config);

    size_t cityPopulation = 0;
    const auto people = world.view<Person, PersonLocation>();

    for (const auto person : people) {
        if (people.get<PersonLocation>(person).city == city) {
            ++cityPopulation;
        }
    }

    auto &cityData = world.get<City>(city);
    cityData.population = cityPopulation;
    std::cout << "City: " << cityData.cityName << '\n'
              << "Size: " << cityData.sizeSQKM << " sq km\n"
              << "Population: " << cityData.population << '\n';

    return 0;
}
