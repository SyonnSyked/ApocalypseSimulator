#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct SimulatorConfig {
    std::string cityName;
    size_t population = 0;
    float sizeSQKM = 0;
    unsigned ageMin = 0, ageMax = 100;
    unsigned hpMin = 70, hpMax = 100;
    unsigned hungerMin = 0, hungerMax = 30;
    unsigned thirstMin = 0, thirstMax = 30;
    unsigned sleepMin = 60, sleepMax = 100;
    std::vector<std::string> genders = {"Female", "Male"};
};

SimulatorConfig LoadConfig(const std::string &path);
