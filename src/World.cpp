#include "World.h"
#include "Components.h"
#include "Population.h"
#include "SimulatorConfig.h"

#include <random>
#include <vector>

entt::entity CreateDistrict(entt::registry &world, entt::entity city, const DistrictConfig &district)
{
    static thread_local std::mt19937 random(std::random_device{}());
    auto real = [&](float min, float max) { return std::uniform_real_distribution<float>(min, max)(random); };
    const auto entity = world.create();
    world.emplace<District>(entity, District{district.districtName, city, 0,
        std::uniform_int_distribution<size_t>(district.shelterMin, district.shelterMax)(random),
        real(district.sizeMin, district.sizeMax), real(district.foodMin, district.foodMax),
        real(district.waterMin, district.waterMax)});
    return entity;
}

entt::entity CreateCity(entt::registry &world, const CityConfig &city, const SimulatorConfig &config)
{
    const auto entity = world.create();
    world.emplace<City>(entity, City{city.cityName, city.population, city.sizeSQKM});
    std::vector<entt::entity> districts;
    for (const auto &district : config.districts)
        districts.push_back(CreateDistrict(world, entity, district));
    CreatePopulation(world, entity, city.population, config, districts);
    return entity;
}

void StartSimulatedWorld(entt::registry &world, const SimulatorConfig &config)
{
    for (const auto &city : config.cities) CreateCity(world, city, config);
}
