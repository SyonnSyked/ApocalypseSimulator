#include "Components.h"
#include "ConsoleColors.h"
#include "SimulatorConfig.h"
#include "SimulationClock.h"
#include "World.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <thread>

int main(int argc, char **argv)
{
    const bool color = ConsoleColors::Enable();
    const bool errorColor = ConsoleColors::Enable(true);
    const auto paint = [color](const char *value) { return ConsoleColors::Use(color, value); };
    const auto paintError = [errorColor](const char *value) { return ConsoleColors::Use(errorColor, value); };
    if (argc > 2) {
        std::cerr << paintError(ConsoleColors::error) << "Usage: " << argv[0]
                  << " [config file]" << paintError(ConsoleColors::reset) << '\n';
        return 1;
    }
    SimulatorConfig config;
    try {
        config = LoadConfig(argc == 2 ? argv[1] : "../../config/simulator.cfg");
    } catch (const std::exception &error) {
        std::cerr << paintError(ConsoleColors::error) << "Config error: " << error.what()
                  << paintError(ConsoleColors::reset) << '\n';
        return 1;
    }

    entt::registry world;
    StartSimulatedWorld(world, config);
    SimulationClock clock;
    clock.StartClock();
    // Wait between updates so each report has time to be read.
    constexpr int tickLimit = 5;
    constexpr auto tickPause = std::chrono::seconds{1};
    std::cout << paint(ConsoleColors::status) << "Simulation started: " << tickLimit
              << " ticks, 1 simulated hour per tick, with a 1-second pause before each update."
              << paint(ConsoleColors::reset) << std::endl;
    for (int i = 0; i < tickLimit; ++i) {
        std::this_thread::sleep_for(tickPause);
        const auto summary = UpdateWorld(world, std::chrono::hours{1});
        clock.AdvanceTick();
        std::cout << paint(ConsoleColors::tick) << "Tick " << clock.GetTickCount() << "/" << tickLimit
                  << " completed; simulated time: " << clock.GetSimulatedTime().count()
                  << " hours." << paint(ConsoleColors::reset) << '\n';
        std::cout << paint(ConsoleColors::statistics);
        if (summary.people != 0) {
            std::cout << "  Average hunger: " << std::fixed << std::setprecision(2)
                      << summary.hungerBefore / summary.people << " -> "
                      << summary.hungerAfter / summary.people << " / 100\n"
                      << "  People at maximum hunger: " << summary.atMaximumHunger
                      << " / " << summary.people << '\n' << std::defaultfloat
                      << std::setprecision(6);
        } else {
            std::cout << "  No people to update.\n";
        }
        std::cout << paint(ConsoleColors::reset) << std::flush;
    }
    clock.EndClock();
    const auto realMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        clock.GetRealElapsedTime()).count();
    std::cout << paint(ConsoleColors::status) << "Simulation finished: " << clock.GetTickCount() << " ticks, "
              << clock.GetSimulatedTime().count() << " simulated hours, "
              << realMilliseconds << " ms real elapsed time." << paint(ConsoleColors::reset) << "\n\n";
    for (const auto cityEntity : world.view<City>()) {
        const auto &city = world.get<City>(cityEntity);
        std::cout << paint(ConsoleColors::city) << "City: " << city.cityName
                  << paint(ConsoleColors::reset) << '\n'
                  << "  Population: " << city.population << '\n'
                  << "  Size: " << city.sizeSQKM << " sq km\n"
                  << "  Districts:\n";

        for (const auto districtEntity : world.view<District>()) {
            const auto &district = world.get<District>(districtEntity);
            if (district.city != cityEntity) continue;
            std::cout << "    " << district.districtName << '\n'
                      << "      Population: " << district.population << '\n'
                      << "      Size: " << district.sizeSQKM << " sq km\n"
                      << "      Shelters: " << district.shelterAmount << '\n'
                      << "      Food: " << district.foodSupply << '\n'
                      << "      Water: " << district.waterSupply << '\n';
        }
        std::cout << '\n';
    }

    return 0;
}
