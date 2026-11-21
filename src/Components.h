#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <entt/entity/entity.hpp>

struct City {
    std::string cityName;
    size_t population;

    float sizeSQKM;
    std::vector<entt::entity> districts;
    std::uint32_t fallbackPartition = 0;
};

struct District {
    std::string districtName;
    entt::entity city = entt::null;
    size_t population;
    size_t shelterAmount;

    float sizeSQKM;

    float foodSupply;
    float waterSupply;
    std::uint32_t partition = 0;
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
