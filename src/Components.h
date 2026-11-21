#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <entt/entity/entity.hpp>

struct Person {
    std::uint32_t nameNumber;
    std::uint32_t genderIndex;
    float hunger;
    float thirst;
    float sleep;
    std::uint8_t age;
    std::uint8_t hp;
};

struct City {
    std::string cityName;
    size_t population;

    float sizeSQKM;
};

struct District {
    std::string districtName;
    entt::entity city = entt::null;
    size_t population;
    size_t shelterAmount;

    float sizeSQKM;

    float foodSupply;
    float waterSupply;
};

struct Road {
    entt::entity source = entt::null;
    entt::entity destination = entt::null;

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
    entt::entity city = entt::null;
    entt::entity district = entt::null;
    entt::entity road = entt::null;
};
