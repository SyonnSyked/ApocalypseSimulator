cmake_minimum_required(VERSION 3.25)

project(ApocSim LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.16.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(EnTT)

add_executable(${PROJECT_NAME} src/main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Wpedantic>)

include(CTest)
if(BUILD_TESTING)
    add_executable(entt_integration_test tests/entt_integration.cpp)
    target_compile_features(entt_integration_test PRIVATE cxx_std_17)
    set_target_properties(entt_integration_test PROPERTIES CXX_EXTENSIONS OFF)
    target_link_libraries(entt_integration_test PRIVATE EnTT::EnTT)
    add_test(NAME entt_integration COMMAND entt_integration_test)
endif()
