#pragma once

#include <entt/entity/registry.hpp>

struct SimulatorConfig;

void CreatePopulation(entt::registry &registry, entt::entity city, const SimulatorConfig &config);
