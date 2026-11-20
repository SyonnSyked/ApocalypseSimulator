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
    const char *cityName;
    size_t population;

    float sizeSQKM;
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
};

