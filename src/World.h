#pragma once
#include "Population.h"
#include <chrono>
#include <cstddef>
struct DistrictConfig;
struct CityConfig;
struct SimulatorConfig;
struct WorldUpdateSummary { std::size_t people=0; double hungerBefore=0, hungerAfter=0; std::size_t atMaximumHunger=0; };
class SimulationWorld {
public:
    const entt::registry &registry() const { return registry_; }
    void setPerson(PersonId id, const PersonStats &stats) { population_.set(id, stats); }
    const PopulationStore &population() const { return population_; }
private:
    bool failed_ = false;
    entt::registry registry_;
    PopulationStore population_;
    friend entt::entity CreateRoad(SimulationWorld &, entt::entity, entt::entity, unsigned, float);
    friend entt::entity CreateDistrict(SimulationWorld &, entt::entity, const DistrictConfig &);
    friend entt::entity CreateCity(SimulationWorld &, const CityConfig &, const SimulatorConfig &);
    friend void StartSimulatedWorld(SimulationWorld &, const SimulatorConfig &);
    friend WorldUpdateSummary UpdateWorld(SimulationWorld &, std::chrono::hours);
};
entt::entity CreateRoad(SimulationWorld &, entt::entity, entt::entity, unsigned, float);
entt::entity CreateDistrict(SimulationWorld &, entt::entity, const DistrictConfig &);
entt::entity CreateCity(SimulationWorld &, const CityConfig &, const SimulatorConfig &);
void StartSimulatedWorld(SimulationWorld &, const SimulatorConfig &);
WorldUpdateSummary UpdateWorld(SimulationWorld &, std::chrono::hours);
