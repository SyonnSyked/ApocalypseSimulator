#pragma once

#include <entt/entity/registry.hpp>
#include <chrono>
#include <cstddef>

struct DistrictConfig;
struct CityConfig;
struct SimulatorConfig;

struct WorldUpdateSummary {
    std::size_t people = 0;
    double hungerBefore = 0;
    double hungerAfter = 0;
    std::size_t atMaximumHunger = 0;
};

entt::entity CreateRoad(entt::registry &world, entt::entity source, entt::entity destination,
                        unsigned int maxCapacity, float maxFlowRate);
entt::entity CreateDistrict(entt::registry &world, entt::entity city,
                            const DistrictConfig &district);
entt::entity CreateCity(entt::registry &world, const CityConfig &city,
                        const SimulatorConfig &config);
void StartSimulatedWorld(entt::registry &world, const SimulatorConfig &config);
WorldUpdateSummary UpdateWorld(entt::registry &world, std::chrono::hours elapsed);
