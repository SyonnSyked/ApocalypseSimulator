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

Each person gets a numbered name, a randomly selected gender from the comma-separated `genders` list, and uniformly random whole-number values within the inclusive configured ranges. Age is in years; HP, hunger, thirst, and sleep use 0â€“100 simulation scales. Higher hunger or thirst means greater need, and higher sleep means more rested. These are starting values, not a demographic or medical model. Person settings may be omitted to use the defaults shown in the supplied config. Food and drink are not yet created by the simulator.

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

## Memory and ownership

Run with `--memory` (before or after the optional config path) to print component sizes,
a raw Person-component estimate, and process working-set/resident measurements where
supported. The estimate is not measured RAM: it excludes entity IDs, sparse and packed
indexes, page slack, other components, allocator overhead and the rest of the process.
Default output is unchanged.

A `Person` stores a 32-bit city-local generated-name number, a 32-bit gender index,
two byte-sized age/HP values and three unchanged `float` vital values. `PersonName`
reconstructs the name on demand; `PersonGender` resolves the index through labels
owned by the registry context. The registry copies and deduplicates labels, so the
configuration may be destroyed. These helpers replace direct `personName` and
`gender` fields in the source API. The label reference returned by `PersonGender`
should not be retained while more populations are created, since the label vector
can grow. Each city starts its name numbers at one.

The registry owns cities, districts, roads, people and locations. A road holds district
entity IDs, not district copies; `CreateRoad` validates endpoints when creating one.
Callers that later destroy districts must first remove or update roads and locations
that reference them, and check `registry.valid` before dereferencing an entity that
may have been destroyed. Initial location assignment remains round-robin, or null
when there are no districts. Initialization remains single-threaded.

On 64-bit MinGW GCC 15.1, the old/new sizes in bytes are: Person 80/24 (alignment
8/4), PersonLocation 12/12, City 48/48, District 72/72, Road 152/16. At 600,000
people the raw Person estimate falls from 48,000,000 to 14,400,000 bytes, saving
33,600,000 bytes (32.04 MiB). A single equivalent release run of the supplied
configuration on Windows measured a peak working set of 76,005,376 bytes before
and 41,807,872 bytes after. These process measurements include much more than
components and will vary across runs and platforms.

EnTT 3.16's default 32-bit entity type has a 20-bit entity index; at most 1,048,575
live entities, including cities, districts and roads, fit. Initialization checks the
whole requested world before creating it. This is an identifier ceiling, not a
promise that memory is available. Even ignoring that limit, 330 million current
Person components alone require 7.92 billion bytes (7.38 GiB), and their locations
another 3.96 billion bytes (3.69 GiB): 11.88 billion bytes (11.06 GiB) before EnTT
indexes, entity storage, allocation overhead, other world data or the application.
Thus this optimization does not meet the future 8 GiB goal. A national-scale design
needs a separate evaluation of entity representation, location storage, iteration,
and measured peak allocation behavior; merely widening EnTT IDs would add overhead
without solving the memory budget.
