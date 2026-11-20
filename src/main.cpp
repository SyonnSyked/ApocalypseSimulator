#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <cstdint>
#include <entt/entity/registry.hpp>

struct Person {
    const char *personName;
    const char *gender;
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

// Functions

struct SimulatorConfig {
    std::string cityName;
    size_t population = 0;
    float sizeSQKM = 0;
};

std::string Trim(const std::string &text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

SimulatorConfig LoadConfig(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Could not open config file: " + path);

    SimulatorConfig config;
    bool hasName = false, hasPopulation = false, hasSize = false;
    std::string line;
    size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        line = Trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        const auto equals = line.find('=');
        if (equals == std::string::npos)
            throw std::runtime_error("Expected key = value on line " + std::to_string(lineNumber));
        const auto key = Trim(line.substr(0, equals));
        const auto value = Trim(line.substr(equals + 1));
        if (key == "city_name" && !hasName && !value.empty()) {
            config.cityName = value;
            hasName = true;
        } else if (key == "population" && !hasPopulation) {
            size_t used = 0;
            if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
                throw std::runtime_error("Invalid population on line " +
                                         std::to_string(lineNumber));
            const auto number = std::stoull(value, &used);
            if (used != value.size() || number > std::numeric_limits<size_t>::max())
                throw std::runtime_error("Invalid population on line " +
                                         std::to_string(lineNumber));
            config.population = static_cast<size_t>(number);
            hasPopulation = true;
        } else if (key == "city_size_sq_km" && !hasSize) {
            size_t used = 0;
            config.sizeSQKM = std::stof(value, &used);
            if (used != value.size() || !std::isfinite(config.sizeSQKM) || config.sizeSQKM < 0)
                throw std::runtime_error("Invalid city size on line " + std::to_string(lineNumber));
            hasSize = true;
        } else {
            throw std::runtime_error("Unknown, duplicate, or empty config key on line " +
                                     std::to_string(lineNumber));
        }
    }
    if (!hasName || !hasPopulation || !hasSize)
        throw std::runtime_error("Config must include city_name, population, and city_size_sq_km");
    return config;
}

void CreatePopulation(entt::registry &registry, entt::entity city, size_t population)
{
    for (size_t i = 0; i < population; ++i) {
        const auto newPerson = registry.create();
        registry.emplace<Person>(newPerson, Person{});
        registry.emplace<PersonLocation>(newPerson, PersonLocation{city});
    }
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [config file]\n";
        return 1;
    }
    SimulatorConfig config;
    try {
        config = LoadConfig(argc == 2 ? argv[1] : "config/simulator.cfg");
    } catch (const std::exception &error) {
        std::cerr << "Config error: " << error.what() << '\n';
        return 1;
    }
    entt::registry world;
    const auto city = world.create();
    world.emplace<City>(city, config.cityName.c_str(), size_t{0}, config.sizeSQKM);

    CreatePopulation(world, city, config.population);

    size_t cityPopulation = 0;
    const auto people = world.view<Person, PersonLocation>();

    for (const auto person : people) {
        if (people.get<PersonLocation>(person).city == city) {
            ++cityPopulation;
        }
    }

    auto &cityData = world.get<City>(city);
    cityData.population = cityPopulation;
    std::cout << "City: " << cityData.cityName << '\n'
              << "Size: " << cityData.sizeSQKM << " sq km\n"
              << "Population: " << cityData.population << '\n';

    return 0;
}
