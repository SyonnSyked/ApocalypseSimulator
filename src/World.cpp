#include "World.h"
#include "Components.h"
#include "Population.h"
#include "SimulatorConfig.h"

entt::entity CreateCity(entt::registry &world, const CityConfig &city, const SimulatorConfig &config)
{
    const auto entity = world.create();
    world.emplace<City>(entity, City{city.cityName, city.population, city.sizeSQKM});
    CreatePopulation(world, entity, city.population, config);
    return entity;
}

void StartSimulatedWorld(entt::registry &world, const SimulatorConfig &config)
{
    for (const auto &city : config.cities) CreateCity(world, city, config);
}
