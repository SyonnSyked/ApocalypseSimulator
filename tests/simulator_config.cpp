#include "Components.h"
#include "Population.h"
#include "SimulatorConfig.h"
#include "World.h"
#include <cstdio>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <limits>

int main()
{
    const char *path = "simulator_config_test.cfg";
    {
        std::ofstream file(path);
        file << "[city]\ncity_name = Test\npopulation = 25\ncity_size_sq_km = 1.5\n"
                "[city]\ncity_name = Second\npopulation = 3\ncity_size_sq_km = 2.5\n"
                "age_min = 42\nage_max = 42\nhp_min = 80\nhp_max = 80\n"
                "hunger_min = 12\nhunger_max = 12\nthirst_min = 13\nthirst_max = 13\n"
                "sleep_min = 90\nsleep_max = 90\ngenders = Test gender\n"
                "[district Shopping]\nshelter_min = 2\nshelter_max = 3\nsize_min = 1\nsize_max = 2\n"
                "food_min = 10\nfood_max = 20\nwater_min = 30\nwater_max = 40\n"
                "[district Medical]\nshelter_min = 4\nshelter_max = 4\nsize_min = 3\nsize_max = 3\n"
                "food_min = 5\nfood_max = 5\nwater_min = 6\nwater_max = 6\n";
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
    size_t districts = 0;
    for (auto entity : world.view<District>()) {
        const auto &district = world.get<District>(entity);
        if (!world.all_of<City>(district.city)) return 6;
        size_t residents = 0;
        for (auto person : world.view<PersonLocation>()) {
            const auto &location = world.get<PersonLocation>(person);
            if (location.district == entity) {
                if (location.city != district.city || location.road != entt::null) return 7;
                ++residents;
            }
        }
        if (residents != district.population) return 8;
        if (district.districtName == "Shopping") {
            if (district.shelterAmount < 2 || district.shelterAmount > 3 ||
                district.sizeSQKM < 1 || district.sizeSQKM > 2 || district.foodSupply < 10 ||
                district.foodSupply > 20 || district.waterSupply < 30 || district.waterSupply > 40) return 9;
        } else if (district.districtName == "Medical") {
            if (district.shelterAmount != 4 || district.sizeSQKM != 3 ||
                district.foodSupply != 5 || district.waterSupply != 6) return 10;
        } else return 11;
        ++districts;
    }
    if (districts != 4) return 12;
    // Names restart per city, and labels survive destruction of the input configuration.
    config.genders.clear();
    size_t count = 0;
    for (auto entity : world.view<Person, PersonLocation>()) {
        const auto &person = world.get<Person>(entity);
        if (PersonName(person).empty() || PersonGender(world, person) != "Test gender" || person.age != 42 ||
            person.hp != 80 || person.hunger != 12 || person.thirst != 13 || person.sleep != 90 ||
            !world.all_of<City>(world.get<PersonLocation>(entity).city)) return 1;
        const auto city = world.get<PersonLocation>(entity).city;
        const auto limit = world.get<City>(city).population;
        if (person.nameNumber < 1 || person.nameNumber > limit ||
            PersonName(person) != "Person " + std::to_string(person.nameNumber)) return 13;
        ++count;
    }
    for (auto city : world.view<City>()) {
        std::vector<bool> names(world.get<City>(city).population + 1);
        for (auto entity : world.view<Person, PersonLocation>()) {
            if (world.get<PersonLocation>(entity).city != city) continue;
            auto number = world.get<Person>(entity).nameNumber;
            if (names[number]) return 14;
            names[number] = true;
        }
    }
    auto ds = world.view<District>();
    auto it = ds.begin(); auto first = *it++; auto second = *it;
    auto road = CreateRoad(world, first, second, 5, 1.5f);
    if (world.get<Road>(road).source != first || world.get<Road>(road).destination != second) return 15;
    bool badRoad = false;
    try { CreateRoad(world, first, entt::null, 1, 1); } catch (const std::invalid_argument &) { badRoad = true; }
    if (!badRoad) return 16;
    if (count != 28) return 2;
    {
        std::ofstream file(path, std::ios::app);
        file << "water_max = 2\n";
    }
    bool rejected = false;
    try { LoadConfig(path); } catch (const std::exception &) { rejected = true; }
    SimulatorConfig huge;
    huge.cities.push_back({"Huge", std::numeric_limits<std::size_t>::max(), 1});
    entt::registry empty;
    bool capacityRejected = false;
    try { StartSimulatedWorld(empty, huge); } catch (const std::length_error &) { capacityRejected = true; }
    if (!capacityRejected || empty.storage<entt::entity>().free_list() != 0) return 17;
    huge.cities[0].population = entt::entt_traits<entt::entity>::entity_mask;
    capacityRejected = false;
    try { StartSimulatedWorld(empty, huge); } catch (const std::length_error &) { capacityRejected = true; }
    if (!capacityRejected || empty.storage<entt::entity>().free_list() != 0) return 18;
    SimulatorConfig noDistrict;
    noDistrict.cities.push_back({"Empty districts", 2, 1});
    StartSimulatedWorld(empty, noDistrict);
    for (auto person : empty.view<PersonLocation>()) if (empty.get<PersonLocation>(person).district != entt::null) return 19;
    std::remove(path);
    return rejected ? 0 : 3;
}
