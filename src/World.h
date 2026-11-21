#pragma once

#include <entt/entity/registry.hpp>

struct DistrictConfig;
struct CityConfig;
struct SimulatorConfig;

entt::entity CreateDistrict(entt::registry &world, entt::entity city, const DistrictConfig &district);
entt::entity CreateCity(entt::registry &world, const CityConfig &city,
                        const SimulatorConfig &config);
void StartSimulatedWorld(entt::registry &world, const SimulatorConfig &config);
