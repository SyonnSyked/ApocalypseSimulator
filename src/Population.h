#pragma once
#include "Components.h"
#include <entt/entity/registry.hpp>
#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>

struct SimulatorConfig;
struct CityConfig;
struct DistrictConfig;
struct WorldUpdateSummary;
class SimulationWorld;
struct PersonId { std::uint64_t value = 0; };
struct PersonStats {
    float hunger = 0, thirst = 0, sleep = 0;
    std::uint32_t gender = 0;
    std::uint8_t age = 0, hp = 0;
};
struct PopulationPage {
    std::size_t size = 0, capacity = 0;
    std::unique_ptr<float[]> hunger, thirst, sleep;
    std::unique_ptr<std::uint32_t[]> gender;
    std::unique_ptr<std::uint8_t[]> age, hp;
    explicit PopulationPage(std::size_t capacity);
};
struct PopulationPartition {
    entt::entity city = entt::null, district = entt::null;
    std::uint32_t firstName = 1, nameStride = 1;
    std::size_t count = 0, expected = 0;
    std::vector<PopulationPage> pages;
};
struct PopulationMetrics {
    std::size_t people = 0, partitions = 0, pages = 0, capacity = 0;
    std::uint64_t occupiedBytes = 0, allocatedBytes = 0;
};
class PopulationStore {
public:
    static constexpr std::size_t pageCapacity = 16384;
    static constexpr std::size_t bytesPerSlot = 3 * sizeof(float) + sizeof(std::uint32_t) + 2 * sizeof(std::uint8_t);
    std::string label(std::uint32_t index) const;
    PersonStats get(PersonId id) const;
    void set(PersonId id, const PersonStats &stats);
    std::string name(PersonId id) const;
    PersonLocation location(PersonId id) const;
    PersonId id(std::uint32_t partition, std::size_t slot) const;
    const PopulationPartition &partition(std::uint32_t handle) const;
    PopulationMetrics metrics() const;
    const std::vector<PopulationPartition> &partitions() const { return partitions_; }
private:
    friend entt::entity CreateCity(SimulationWorld &, const CityConfig &, const SimulatorConfig &);
    friend entt::entity CreateDistrict(SimulationWorld &, entt::entity, const DistrictConfig &);
    friend WorldUpdateSummary UpdateWorld(SimulationWorld &, std::chrono::hours);
    std::uint32_t createPartition(entt::entity city, entt::entity district, std::uint32_t firstName,
                                  std::uint32_t stride, std::size_t expected);
    std::uint32_t addLabel(const std::string &label);
    PersonId createPerson(std::uint32_t partition, const PersonStats &stats);
    std::vector<PopulationPartition> &pagesForUpdate() { return partitions_; }
    std::vector<PopulationPartition> partitions_;
    std::deque<std::string> labels_;
    const PopulationPage &resolve(PersonId id, std::size_t &offset) const;
};
void ValidatePersonConfig(const SimulatorConfig &config);
void ValidateEntityCapacity(const entt::registry &registry, std::size_t additional);
