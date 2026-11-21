#include "Components.h"
#include "SimulationClock.h"
#include "World.h"

#include <cassert>
#include <chrono>

int main()
{
    SimulationClock clock;
    assert(clock.GetTickCount() == 0);
    clock.StartClock();
    entt::registry world;
    const auto entity = world.create();
    world.emplace<Person>(entity, Person{1, 0, 98.0f, 0.0f, 100.0f, 30, 100});
    for (int i = 0; i < 5; ++i) {
        const auto summary = UpdateWorld(world, std::chrono::hours{1});
        assert(summary.people == 1);
        if (i == 0) {
            assert(summary.hungerBefore == 98.0);
            assert(summary.hungerAfter == 99.0);
            assert(summary.atMaximumHunger == 0);
        }
        if (i == 1) assert(summary.atMaximumHunger == 1);
        clock.AdvanceTick();
    }
    world.get<Person>(entity).hunger = 98.25f;
    world.get<Person>(entity).thirst = 12.5f;
    world.get<Person>(entity).sleep = 33.75f;
    auto fractional = UpdateWorld(world, std::chrono::hours{1});
    assert(fractional.hungerBefore == 98.25 && fractional.hungerAfter == 99.25);
    fractional = UpdateWorld(world, std::chrono::hours{1});
    assert(fractional.hungerAfter == 100.0 && fractional.atMaximumHunger == 1);
    assert(world.get<Person>(entity).thirst == 12.5f && world.get<Person>(entity).sleep == 33.75f);
    clock.EndClock();
    assert(clock.GetTickCount() == 5);
    assert(clock.GetSimulatedTime() == std::chrono::hours{5});
    assert(world.get<Person>(entity).hunger == 100.0f);
    const auto elapsed = clock.GetRealElapsedTime();
    clock.AdvanceTick();
    assert(clock.GetTickCount() == 5);
    assert(clock.GetRealElapsedTime() == elapsed);
    clock.StartClock();
    assert(clock.GetTickCount() == 0);
}
