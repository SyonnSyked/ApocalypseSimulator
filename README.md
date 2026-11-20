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
