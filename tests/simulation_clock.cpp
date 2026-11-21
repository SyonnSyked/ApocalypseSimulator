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
    world.emplace<Person>(entity, Person{"Test", "", 30, 100, 98.0f, 0.0f, 100.0f});
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
