#pragma once

#include <entt/entity/registry.hpp>

struct SimulatorConfig;

void CreatePopulation(entt::registry &registry, entt::entity city, size_t population,
                      const SimulatorConfig &config);

void TickVitals();
void UpdateSurvivalDrive();
void SimulateSinglePerson();
void SimulateAllPeople();
