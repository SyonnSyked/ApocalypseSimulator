#pragma once

#include <entt/entity/registry.hpp>
#include <vector>
#include <string>
#include <cstdint>

struct SimulatorConfig;
struct Person;
struct GenderLabels {
    std::vector<std::string> values;
};
std::string PersonName(const Person &person);
const std::string &PersonGender(const entt::registry &registry, const Person &person);
void ValidatePersonConfig(const SimulatorConfig &config);
void ValidateEntityCapacity(const entt::registry &registry, std::size_t additional);

void CreatePopulation(entt::registry &registry, entt::entity city, size_t population,
                      const SimulatorConfig &config,
                      const std::vector<entt::entity> &districts = {});

void TickVitals();
void UpdateSurvivalDrive();
void SimulateSinglePerson();
void SimulateAllPeople();
