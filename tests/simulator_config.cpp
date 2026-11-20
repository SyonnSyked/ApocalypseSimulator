#include "Components.h"
#include "Population.h"
#include "SimulatorConfig.h"
#include "World.h"
#include <cstdio>
#include <exception>
#include <fstream>

int main()
{
    const char *path = "simulator_config_test.cfg";
    {
        std::ofstream file(path);
        file << "[city]\ncity_name = Test\npopulation = 25\ncity_size_sq_km = 1.5\n"
                "[city]\ncity_name = Second\npopulation = 3\ncity_size_sq_km = 2.5\n"
                "age_min = 42\nage_max = 42\nhp_min = 80\nhp_max = 80\n"
                "hunger_min = 12\nhunger_max = 12\nthirst_min = 13\nthirst_max = 13\n"
                "sleep_min = 90\nsleep_max = 90\ngenders = Test gender\n";
    }
    auto config = LoadConfig(path);
    entt::registry world;
    StartSimulatedWorld(world, config);
    size_t cities = 0;
    for (auto entity : world.view<City>()) {
        const auto &city = world.get<City>(entity);
        size_t inhabitants = 0;
        for (auto person : world.view<PersonLocation>())
            if (world.get<PersonLocation>(person).city == entity) ++inhabitants;
        if ((city.cityName == "Test" && (city.population != 25 || city.sizeSQKM != 1.5f)) ||
            (city.cityName == "Second" && (city.population != 3 || city.sizeSQKM != 2.5f)) ||
            (city.cityName != "Test" && city.cityName != "Second") || inhabitants != city.population) return 4;
        ++cities;
    }
    if (cities != 2) return 5;
    size_t count = 0;
    for (auto entity : world.view<Person, PersonLocation>()) {
        const auto &person = world.get<Person>(entity);
        if (person.personName.empty() || person.gender != "Test gender" || person.age != 42 ||
            person.hp != 80 || person.hunger != 12 || person.thirst != 13 || person.sleep != 90 ||
            !world.all_of<City>(world.get<PersonLocation>(entity).city)) return 1;
        ++count;
    }
    if (count != 28) return 2;
    {
        std::ofstream file(path, std::ios::app);
        file << "age_min = 2\n";
    }
    bool rejected = false;
    try { LoadConfig(path); } catch (const std::exception &) { rejected = true; }
    std::remove(path);
    return rejected ? 0 : 3;
}
