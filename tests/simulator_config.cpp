#include "Components.h"
#include "Population.h"
#include "SimulatorConfig.h"
#include <cstdio>
#include <exception>
#include <fstream>

int main()
{
    const char *path = "simulator_config_test.cfg";
    {
        std::ofstream file(path);
        file << "city_name = Test\npopulation = 25\ncity_size_sq_km = 1.5\n"
                "age_min = 42\nage_max = 42\nhp_min = 80\nhp_max = 80\n"
                "hunger_min = 12\nhunger_max = 12\nthirst_min = 13\nthirst_max = 13\n"
                "sleep_min = 90\nsleep_max = 90\ngenders = Test gender\n";
    }
    auto config = LoadConfig(path);
    entt::registry world;
    auto city = world.create();
    CreatePopulation(world, city, config);
    size_t count = 0;
    for (auto entity : world.view<Person, PersonLocation>()) {
        const auto &person = world.get<Person>(entity);
        if (person.personName.empty() || person.gender != "Test gender" || person.age != 42 ||
            person.hp != 80 || person.hunger != 12 || person.thirst != 13 || person.sleep != 90 ||
            world.get<PersonLocation>(entity).city != city) return 1;
        ++count;
    }
    if (count != 25) return 2;
    {
        std::ofstream file(path, std::ios::app);
        file << "age_min = 2\n";
    }
    bool rejected = false;
    try { LoadConfig(path); } catch (const std::exception &) { rejected = true; }
    std::remove(path);
    return rejected ? 0 : 3;
}
