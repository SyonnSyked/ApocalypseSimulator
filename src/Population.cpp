#include "Population.h"
#include "Components.h"
#include "SimulatorConfig.h"

#include <random>

void CreatePopulation(entt::registry &registry, entt::entity city, size_t population,
                      const SimulatorConfig &config, const std::vector<entt::entity> &districts)
{
    std::mt19937 random(std::random_device{}());
    std::uniform_int_distribution<unsigned> age(config.ageMin, config.ageMax),
        hp(config.hpMin, config.hpMax), hunger(config.hungerMin, config.hungerMax),
        thirst(config.thirstMin, config.thirstMax), sleep(config.sleepMin, config.sleepMax);
    std::uniform_int_distribution<size_t> gender(0, config.genders.size() - 1);
    for (size_t i = 0; i < population; ++i) {
        const auto newPerson = registry.create();
        registry.emplace<Person>(
            newPerson,
            Person{"Person " + std::to_string(i + 1), config.genders[gender(random)],
                   static_cast<uint8_t>(age(random)), static_cast<uint8_t>(hp(random)),
                   static_cast<float>(hunger(random)), static_cast<float>(thirst(random)),
                   static_cast<float>(sleep(random))});
        const auto district = districts.empty() ? entt::null : districts[i % districts.size()];
        registry.emplace<PersonLocation>(newPerson, PersonLocation{city, district, entt::null});
        if (district != entt::null) ++registry.get<District>(district).population;
    }
}
