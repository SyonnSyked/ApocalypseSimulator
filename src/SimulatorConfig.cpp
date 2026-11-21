#include "SimulatorConfig.h"

#include <charconv>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace {
std::string_view Trim(std::string_view text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

} // namespace

SimulatorConfig LoadConfig(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Could not open config file: " + path);

    SimulatorConfig config;
    CityConfig city;
    bool hasName = false, hasPopulation = false, hasSize = false;
    bool inCity = false, cityFields = false;
    bool inDistrict = false;
    DistrictConfig district;
    std::unordered_map<std::string, bool> districtSeen;
    auto finishDistrict = [&] {
        if (districtSeen.size() != 8)
            throw std::runtime_error("Each district archetype must define all eight range endpoints");
        if (district.shelterMin > district.shelterMax || district.sizeMin > district.sizeMax ||
            district.foodMin > district.foodMax || district.waterMin > district.waterMax)
            throw std::runtime_error("District minimum must not exceed maximum: " + district.districtName);
        for (const auto &existing : config.districts)
            if (existing.districtName == district.districtName)
                throw std::runtime_error("Duplicate district archetype: " + district.districtName);
        config.districts.push_back(district);
        district = DistrictConfig{};
        districtSeen.clear();
    };
    auto finishCity = [&] {
        if (!hasName || !hasPopulation || !hasSize)
            throw std::runtime_error("Each city must include city_name, population, and city_size_sq_km");
        config.cities.push_back(city);
        city = CityConfig{};
        hasName = hasPopulation = hasSize = cityFields = false;
    };
    std::unordered_map<std::string_view, unsigned *> ranges = {
        {"age_min", &config.ageMin}, {"age_max", &config.ageMax},
        {"hp_min", &config.hpMin}, {"hp_max", &config.hpMax},
        {"hunger_min", &config.hungerMin}, {"hunger_max", &config.hungerMax},
        {"thirst_min", &config.thirstMin}, {"thirst_max", &config.thirstMax},
        {"sleep_min", &config.sleepMin}, {"sleep_max", &config.sleepMax}};
    std::unordered_map<std::string, bool> seen;
    std::string line;
    size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        const auto content = Trim(line);
        if (content.empty() || content[0] == '#')
            continue;
        if (content == "[city]") {
            if (inDistrict) { finishDistrict(); inDistrict = false; }
            if (inCity || cityFields) finishCity();
            inCity = true;
            continue;
        }
        if (content.size() >= 11 && content.substr(0, 10) == "[district " && content.back() == ']') {
            if (inDistrict) finishDistrict();
            if (inCity || cityFields) { finishCity(); inCity = false; }
            district.districtName = std::string(Trim(content.substr(10, content.size() - 11)));
            if (district.districtName.empty())
                throw std::runtime_error("Empty district archetype on line " + std::to_string(lineNumber));
            inDistrict = true;
            continue;
        }
        const auto equals = content.find('=');
        if (equals == std::string::npos)
            throw std::runtime_error("Expected key = value on line " + std::to_string(lineNumber));
        const auto key = Trim(content.substr(0, equals));
        const auto value = Trim(content.substr(equals + 1));
        if (inDistrict) {
            if (!districtSeen.emplace(std::string(key), true).second)
                throw std::runtime_error("Duplicate district key on line " + std::to_string(lineNumber));
            if (key == "shelter_min" || key == "shelter_max") {
                size_t number = 0;
                const auto parsed = std::from_chars(value.data(), value.data() + value.size(), number);
                if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
                    throw std::runtime_error("Invalid district shelter count on line " + std::to_string(lineNumber));
                (key == "shelter_min" ? district.shelterMin : district.shelterMax) = number;
            } else {
                float *target = nullptr;
                if (key == "size_min") target = &district.sizeMin;
                else if (key == "size_max") target = &district.sizeMax;
                else if (key == "food_min") target = &district.foodMin;
                else if (key == "food_max") target = &district.foodMax;
                else if (key == "water_min") target = &district.waterMin;
                else if (key == "water_max") target = &district.waterMax;
                if (!target) throw std::runtime_error("Unknown district key on line " + std::to_string(lineNumber));
                size_t used = 0;
                float number;
                try { number = std::stof(std::string(value), &used); }
                catch (const std::exception &) { throw std::runtime_error("Invalid district value on line " + std::to_string(lineNumber)); }
                if (used != value.size() || !std::isfinite(number) || number < 0)
                    throw std::runtime_error("Invalid district value on line " + std::to_string(lineNumber));
                *target = number;
            }
            continue;
        }
        const bool isCityKey = key == "city_name" || key == "population" || key == "city_size_sq_km";
        if (!isCityKey && !seen.emplace(std::string(key), true).second)
            throw std::runtime_error("Duplicate config key on line " + std::to_string(lineNumber));
        if (isCityKey) cityFields = true;
        if (key == "city_name" && !hasName && !value.empty()) {
            city.cityName = std::string(value);
            hasName = true;
        } else if (key == "population" && !hasPopulation) {
            size_t number = 0;
            const auto parsed = std::from_chars(value.data(), value.data() + value.size(), number);
            if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
                throw std::runtime_error("Invalid population on line " +
                                         std::to_string(lineNumber));
            city.population = number;
            hasPopulation = true;
        } else if (key == "city_size_sq_km" && !hasSize) {
            size_t used = 0;
            city.sizeSQKM = std::stof(std::string(value), &used);
            if (used != value.size() || !std::isfinite(city.sizeSQKM) || city.sizeSQKM < 0)
                throw std::runtime_error("Invalid city size on line " + std::to_string(lineNumber));
            hasSize = true;
        } else if (const auto it = ranges.find(key); it != ranges.end()) {
            unsigned number = 0;
            const auto parsed = std::from_chars(value.data(), value.data() + value.size(), number);
            if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || number > 100)
                throw std::runtime_error("Expected a whole number from 0 to 100 on line " + std::to_string(lineNumber));
            *it->second = number;
        } else if (key == "genders") {
            config.genders.clear();
            size_t start = 0;
            while (start <= value.size()) {
                const auto end = value.find(',', start);
                const auto item = Trim(value.substr(start, end == std::string_view::npos ? end : end - start));
                if (item.empty()) throw std::runtime_error("Empty gender on line " + std::to_string(lineNumber));
                config.genders.emplace_back(item);
                if (end == std::string_view::npos) break;
                start = end + 1;
            }
        } else {
            throw std::runtime_error("Unknown, duplicate, or empty config key on line " +
                                     std::to_string(lineNumber));
        }
    }
    if (inDistrict) finishDistrict();
    if (inCity || cityFields) finishCity();
    if (config.cities.empty()) throw std::runtime_error("Config must include at least one city");
    if (config.ageMin > config.ageMax || config.hpMin > config.hpMax ||
        config.hungerMin > config.hungerMax || config.thirstMin > config.thirstMax ||
        config.sleepMin > config.sleepMax)
        throw std::runtime_error("Each minimum must be less than or equal to its maximum");
    return config;
}

