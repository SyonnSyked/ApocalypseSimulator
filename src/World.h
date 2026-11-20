#pragma once

#include <entt/entity/registry.hpp>

struct CityConfig;
struct SimulatorConfig;

entt::entity CreateCity(entt::registry &world, const CityConfig &city, const SimulatorConfig &config);
void StartSimulatedWorld(entt::registry &world, const SimulatorConfig &config);
