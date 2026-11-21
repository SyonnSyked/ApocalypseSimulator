#include "Components.h"
#include "ConsoleColors.h"
#include "SimulatorConfig.h"
#include "SimulationClock.h"
#include "World.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>
#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <fstream>
#endif

int main(int argc, char **argv)
{
    const bool color = ConsoleColors::Enable();
    const bool errorColor = ConsoleColors::Enable(true);
    const auto paint = [color](const char *value) { return ConsoleColors::Use(color, value); };
    const auto paintError = [errorColor](const char *value) {
        return ConsoleColors::Use(errorColor, value);
    };
    bool memoryStats = false;
    const char *configPath = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--memory")
            memoryStats = true;
        else if (!configPath)
            configPath = argv[i];
        else {
            std::cerr << "Usage: " << argv[0] << " [config file] [--memory]\n";
            return 1;
        }
    }
    if (argc > 3) {
        std::cerr << paintError(ConsoleColors::error) << "Usage: " << argv[0]
                  << " [config file] [--memory]" << paintError(ConsoleColors::reset) << '\n';
        return 1;
    }
    SimulatorConfig config;
    try {
        config = LoadConfig(configPath ? configPath : "../../config/simulator.cfg");
    } catch (const std::exception &error) {
        std::cerr << paintError(ConsoleColors::error) << "Config error: " << error.what()
                  << paintError(ConsoleColors::reset) << '\n';
        return 1;
    }

    SimulationWorld world;
    try {
        StartSimulatedWorld(world, config);
    } catch (const std::exception &error) {
        std::cerr << paintError(ConsoleColors::error)
                  << "World initialization error: " << error.what()
                  << paintError(ConsoleColors::reset) << '\n';
        return 1;
    }
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
        std::cout << paint(ConsoleColors::tick) << "Tick " << clock.GetTickCount() << "/"
                  << tickLimit << " completed; simulated time: " << clock.GetSimulatedTime().count()
                  << " hours." << paint(ConsoleColors::reset) << '\n';
        std::cout << paint(ConsoleColors::statistics);
        if (summary.people != 0) {
            std::cout << "  Average hunger: " << std::fixed << std::setprecision(2)
                      << summary.hungerBefore / summary.people << " -> "
                      << summary.hungerAfter / summary.people << " / 100\n"
                      << "  People at maximum hunger: " << summary.atMaximumHunger << " / "
                      << summary.people << '\n'
                      << std::defaultfloat << std::setprecision(6);
        } else {
            std::cout << "  No people to update.\n";
        }
        std::cout << paint(ConsoleColors::reset) << std::flush;
    }
    clock.EndClock();
    const auto realMilliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(clock.GetRealElapsedTime()).count();
    std::cout << paint(ConsoleColors::status) << "Simulation finished: " << clock.GetTickCount()
              << " ticks, " << clock.GetSimulatedTime().count() << " simulated hours, "
              << realMilliseconds << " ms real elapsed time." << paint(ConsoleColors::reset)
              << "\n\n";
    for (const auto cityEntity : world.registry().view<City>()) {
        const auto &city = world.registry().get<City>(cityEntity);
        std::cout << paint(ConsoleColors::city) << "City: " << city.cityName
                  << paint(ConsoleColors::reset) << '\n'
                  << "  Population: " << city.population << '\n'
                  << "  Size: " << city.sizeSQKM << " sq km\n"
                  << "  Districts:\n";

        for (const auto districtEntity : world.registry().view<District>()) {
            const auto &district = world.registry().get<District>(districtEntity);
            if (district.city != cityEntity)
                continue;
            std::cout << "    " << district.districtName << '\n'
                      << "      Population: " << district.population << '\n'
                      << "      Size: " << district.sizeSQKM << " sq km\n"
                      << "      Shelters: " << district.shelterAmount << '\n'
                      << "      Food: " << district.foodSupply << '\n'
                      << "      Water: " << district.waterSupply << '\n';
        }
        std::cout << '\n';
    }

    if (memoryStats) {
        const auto metrics = world.population().metrics();
        const auto &registry = world.registry();
        std::cout << "Memory statistics (bytes):\n"
                  << "  Total population: " << metrics.people
                  << "\n  Cities: " << registry.view<City>().size()
                  << "\n  Districts: " << registry.view<District>().size()
                  << "\n  Partitions: " << metrics.partitions << "\n  Pages: " << metrics.pages
                  << "\n  Occupied slots: " << metrics.people << "\n  Allocated slots: " << metrics.capacity
                  << "\n  Raw occupied attribute bytes: " << metrics.occupiedBytes
                  << "\n  Estimated allocated attribute bytes: " << metrics.allocatedBytes << '\n';
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS counters{};
        if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
            std::cout << "  Measured current working set: " << counters.WorkingSetSize
                      << "\n  Measured peak working set: " << counters.PeakWorkingSetSize << '\n';
#elif defined(__unix__) || defined(__APPLE__)
#ifdef __linux__
        std::ifstream statm("/proc/self/statm");
        std::uint64_t totalPages = 0, residentPages = 0;
        const auto pageSize = sysconf(_SC_PAGESIZE);
        if (pageSize > 0 && statm >> totalPages >> residentPages)
            std::cout << "  Measured current resident set: "
                      << residentPages * static_cast<std::uint64_t>(pageSize) << '\n';
#endif
        struct rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) == 0)
#ifdef __APPLE__
            std::cout << "  Measured peak resident set: " << usage.ru_maxrss << '\n';
#else
            std::cout << "  Measured peak resident set: "
                      << static_cast<std::uint64_t>(usage.ru_maxrss) * 1024 << '\n';
#endif
#else
        std::cout << "  Process memory measurement unavailable on this platform.\n";
#endif
    }
    return 0;
}
