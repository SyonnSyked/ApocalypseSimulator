cmake_minimum_required(VERSION 3.25)

project(ApocSim LANGUAGES CXX)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.16.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(EnTT)

add_library(simulator src/SimulatorConfig.cpp src/Population.cpp src/World.cpp
                      src/PhaseExecutor.cpp src/SimulationClock.cpp)
target_compile_features(simulator PUBLIC cxx_std_17)
target_include_directories(simulator PUBLIC src)
target_link_libraries(simulator PUBLIC EnTT::EnTT Threads::Threads)
set_target_properties(simulator PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(simulator PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Wpedantic>)

add_executable(${PROJECT_NAME} src/main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE simulator)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE psapi)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Wpedantic>)

include(CTest)
if(BUILD_TESTING)
    add_executable(population_store_test tests/population_store.cpp)
    target_link_libraries(population_store_test PRIVATE simulator)
    add_test(NAME population_store COMMAND population_store_test)
    add_executable(simulation_clock_test tests/simulation_clock.cpp)
    target_link_libraries(simulation_clock_test PRIVATE simulator)
    add_test(NAME simulation_clock COMMAND simulation_clock_test)
    add_executable(phase_executor_test tests/phase_executor.cpp)
    target_link_libraries(phase_executor_test PRIVATE simulator)
    add_test(NAME phase_executor COMMAND phase_executor_test)
    add_executable(entt_integration_test tests/entt_integration.cpp)
    target_compile_features(entt_integration_test PRIVATE cxx_std_17)
    set_target_properties(entt_integration_test PROPERTIES CXX_EXTENSIONS OFF)
    target_link_libraries(entt_integration_test PRIVATE EnTT::EnTT)
    add_test(NAME entt_integration COMMAND entt_integration_test)
    add_executable(simulator_config_test tests/simulator_config.cpp)
    target_compile_features(simulator_config_test PRIVATE cxx_std_17)
    target_link_libraries(simulator_config_test PRIVATE simulator)
    add_test(NAME simulator_config COMMAND simulator_config_test)
endif()
