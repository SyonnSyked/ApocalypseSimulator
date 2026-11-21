# Apocalypse Simulator

Apocalypse Simulator is a C++17 simulation using the EnTT entity-component system. It currently creates configurable cities and populations and prints their details.

## Requirements

- CMake 3.25 or newer
- GCC with C++17 support (`g++`)
- Ninja
- Git and an internet connection for the first configure (to fetch EnTT)

Make sure the required tools are available on your shell's `PATH`. On Windows, use a GCC toolchain such as MinGW-w64 and keep its `bin` directory on `PATH` when building and running the executable.

## Build and run

From the repository root in PowerShell:

```powershell
cmake --preset dev
cmake --build --preset dev
.\build\dev\ApocSim.exe config/simulator.cfg
```

Edit `config/simulator.cfg` to set city names, initial populations, city sizes, and shared person attribute ranges. Add a `[city]` section for each city:

```ini
[city]
city_name = Houston
population = 1000
city_size_sq_km = 12.5

[city]
city_name = Dallas
population = 500
city_size_sq_km = 10
```

The program creates the configured people in each city and prints the city details. The original single-city format without a `[city]` heading still works. Pass the config path as shown above, or run from `build/dev` to use the default `../../config/simulator.cfg` path.

Each person gets a numbered name, a randomly selected gender from the comma-separated `genders` list, and uniformly random whole-number values within the inclusive configured ranges. Age is in years; HP, hunger, thirst, and sleep use 0–100 simulation scales. Higher hunger or thirst means greater need, and higher sleep means more rested. These are starting values, not a demographic or medical model. Person settings may be omitted to use the defaults shown in the supplied config. Food and drink are not yet created by the simulator.

For an optimized build, use `cmake --preset release` and `cmake --build --preset release`; the executable is in `build/release`. On Linux or macOS, run `./build/dev/ApocSim` instead. Both presets generate `compile_commands.json` in their build directories.

## Neovim and clangd

Configure the `dev` preset before opening the project. The project-local `.clangd` points clangd to `build/dev/compile_commands.json`, which supplies the compiler flags and EnTT include directory for completion and diagnostics. Reconfigure after changing CMake targets or dependencies, and restart clangd in Neovim if diagnostics are stale. If you use a different build directory, update `.clangd` to point to that directory.

## District archetypes

Add `[district NAME]` sections after the city and person settings. Each section
creates one independently randomized district in every city; names may be reused
across cities but each archetype name must be unique in the config. Define all
of `shelter_min`, `shelter_max`, `size_min`, `size_max`, `food_min`, `food_max`,
`water_min`, and `water_max`. Shelter counts are inclusive whole-number ranges;
other ranges contain nonnegative finite numbers. Size is square kilometers and
food and water are abstract supply units. The supplied ranges are illustrative,
not calibrated. District sizes are sampled independently and do not partition
or constrain the configured city area.

Each city's people are distributed evenly among its districts at creation, and
`District::population` is the actual initial count, not a separately sampled
value. Locations retain their city and district entity references; the road is
initially unset. With no district sections, legacy configs continue to work and
people have no district assigned. Districts and resources are initialized only;
no movement or resource consumption is performed yet.

## Simulation clock

After creating the world, the program runs five ticks and exits, pausing for one
real second before each update so its progress can be read. Each
completed tick represents one simulated hour. As a first illustrative world
update, each person's hunger increases by one point per hour, capped at 100;
this rate is a placeholder, not a calibrated model. The clock also measures
real elapsed time separately, using `steady_clock`. Each tick reports average
hunger before and after the update and the number of people at maximum hunger.
The pause is in addition to update time; it is not a fixed real-time schedule.
Longer runs and configurable pacing can be added to the simulation loop later.

Console colors are grouped by purpose in `src/ConsoleColors.h`. Change the
`error`, `status`, `tick`, `statistics`, or `city` palette entries to another
named color in that file, then rebuild. Colors are disabled for redirected
output and when the `NO_COLOR` environment variable is set. On Windows, a
console that supports virtual terminal colors is required.

## Concurrency foundation

A reusable worker pool and its ownership rules are documented in
[docs/CONCURRENCY.md](docs/CONCURRENCY.md). World creation and output remain
single-threaded; the world update does not use the worker pool yet.
