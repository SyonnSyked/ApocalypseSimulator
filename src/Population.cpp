#include "Population.h"
#include "Components.h"
#include "SimulatorConfig.h"

#include <random>
#include <limits>
#include <stdexcept>
#include <algorithm>

std::string PersonName(const Person &person)
{
    return "Person " + std::to_string(person.nameNumber);
}

const std::string &PersonGender(const entt::registry &registry, const Person &person)
{
    return registry.ctx().get<GenderLabels>().values.at(person.genderIndex);
}

void ValidatePersonConfig(const SimulatorConfig &config)
{
    if (config.genders.empty())
        throw std::invalid_argument("At least one gender is required");
    if (config.genders.size() > std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("Too many gender labels");
    if (config.ageMin > config.ageMax || config.hpMin > config.hpMax ||
        config.hungerMin > config.hungerMax || config.thirstMin > config.thirstMax ||
        config.sleepMin > config.sleepMax || config.ageMax > 100 || config.hpMax > 100 ||
        config.hungerMax > 100 || config.thirstMax > 100 || config.sleepMax > 100)
        throw std::invalid_argument("Invalid person attribute range");
}

void ValidateEntityCapacity(const entt::registry &registry, std::size_t additional)
{
    const auto used = registry.storage<entt::entity>()->free_list();
    constexpr auto capacity =
        static_cast<std::size_t>(entt::entt_traits<entt::entity>::entity_mask);
    if (used > capacity || additional > capacity - used)
        throw std::length_error("World exceeds EnTT entity capacity");
}

void CreatePopulation(entt::registry &registry, entt::entity city, size_t population,
                      const SimulatorConfig &config, const std::vector<entt::entity> &districts)
{
    ValidateEntityCapacity(registry, population);
    if (!registry.valid(city) || !registry.all_of<City>(city))
        throw std::invalid_argument("Invalid population city");
    if (population > std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("Person name number overflow");
    ValidatePersonConfig(config);
    for (auto district : districts) {
        if (!registry.valid(district) || !registry.all_of<District>(district) ||
            registry.get<District>(district).city != city)
            throw std::invalid_argument("Invalid population district");
        if (registry.get<District>(district).population >
            std::numeric_limits<std::size_t>::max() - population)
            throw std::overflow_error("District population overflow");
    }
    auto &labels = registry.ctx().emplace<GenderLabels>().values;
    std::vector<std::uint32_t> indices;
    indices.reserve(config.genders.size());
    for (const auto &label : config.genders) {
        auto it = std::find(labels.begin(), labels.end(), label);
        if (it == labels.end()) {
            if (labels.size() >= std::numeric_limits<std::uint32_t>::max())
                throw std::length_error("Too many gender labels");
            labels.push_back(label);
            indices.push_back(static_cast<std::uint32_t>(labels.size() - 1));
        } else
            indices.push_back(static_cast<std::uint32_t>(it - labels.begin()));
    }
    std::mt19937 random(std::random_device{}());
    std::uniform_int_distribution<unsigned> age(config.ageMin, config.ageMax),
        hp(config.hpMin, config.hpMax), hunger(config.hungerMin, config.hungerMax),
        thirst(config.thirstMin, config.thirstMax), sleep(config.sleepMin, config.sleepMax);
    std::uniform_int_distribution<size_t> gender(0, config.genders.size() - 1);
    for (size_t i = 0; i < population; ++i) {
        const auto newPerson = registry.create();
        const auto genderIndex = indices[gender(random)];
        const auto personAge = static_cast<std::uint8_t>(age(random));
        const auto personHp = static_cast<std::uint8_t>(hp(random));
        const auto personHunger = static_cast<float>(hunger(random));
        const auto personThirst = static_cast<float>(thirst(random));
        const auto personSleep = static_cast<float>(sleep(random));
        registry.emplace<Person>(newPerson, Person{static_cast<std::uint32_t>(i + 1), genderIndex,
                                                   personHunger, personThirst, personSleep,
                                                   personAge, personHp});
        const auto district = districts.empty() ? entt::null : districts[i % districts.size()];
        registry.emplace<PersonLocation>(newPerson, PersonLocation{city, district, entt::null});
        if (district != entt::null)
            ++registry.get<District>(district).population;
    }
}
