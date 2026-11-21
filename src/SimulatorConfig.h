#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct DistrictConfig {
    std::string districtName;
    size_t shelterMin = 0, shelterMax = 0;
    float sizeMin = 0, sizeMax = 0;
    float foodMin = 0, foodMax = 0;
    float waterMin = 0, waterMax = 0;
};

struct CityConfig {
    std::string cityName;
    size_t population = 0;
    float sizeSQKM = 0;
};

struct SimulatorConfig {
    std::vector<CityConfig> cities;
    std::vector<DistrictConfig> districts;
    unsigned ageMin = 0, ageMax = 100;
    unsigned hpMin = 70, hpMax = 100;
    unsigned hungerMin = 0, hungerMax = 30;
    unsigned thirstMin = 0, thirstMax = 30;
    unsigned sleepMin = 60, sleepMax = 100;
    std::vector<std::string> genders = {"Female", "Male"};
};

SimulatorConfig LoadConfig(const std::string &path);
