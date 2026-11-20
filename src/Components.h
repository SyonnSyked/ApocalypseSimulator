#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <entt/entity/entity.hpp>

struct Person {
    std::string personName;
    std::string gender;
    uint8_t age;
    uint8_t hp;

    float hunger;
    float thirst;
    float sleep;
};

struct City {
    std::string cityName;
    size_t population;

    float sizeSQKM;
};

struct District {
    std::string cityName;
    size_t population;
    size_t shelterAmount;

    float sizeSQKM;

    float foodSupply;
    float waterSupply;
};

struct Road {
    District source;
    District destination;

    unsigned int maxCapacity;

    float maxFlowRate;
};

struct FoodItem {
    const char *foodName;

    float satiation;
};

struct DrinkItem {
    const char *drinkName;

    float refreshment;
};

struct PersonLocation {
    entt::entity city;
    entt::entity district;
    entt::entity road;
};
