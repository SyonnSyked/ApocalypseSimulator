#pragma once

#include <entt/entity/registry.hpp>
#include <vector>

struct SimulatorConfig;

void CreatePopulation(entt::registry &registry, entt::entity city, size_t population,
                      const SimulatorConfig &config, const std::vector<entt::entity> &districts = {});

void TickVitals();
void UpdateSurvivalDrive();
void SimulateSinglePerson();
void SimulateAllPeople();
