#include "Components.h"
#include "SimulationClock.h"
#include "SimulatorConfig.h"
#include "World.h"

#include <cassert>
#include <chrono>

int main()
{
    SimulationClock clock;
    assert(clock.GetTickCount() == 0);
    clock.StartClock();
    SimulationWorld world;
    SimulatorConfig config; config.cities.push_back({"Test",1,1});
    StartSimulatedWorld(world,config);
    auto entity=world.population().id(1,0);
    auto person=world.population().get(entity); person.hunger=98; person.thirst=0; person.sleep=100;
    world.setPerson(entity,person);
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
    person=world.population().get(entity); person.hunger=98.25f; person.thirst=12.5f; person.sleep=33.75f; world.setPerson(entity,person);
    auto fractional = UpdateWorld(world, std::chrono::hours{1});
    assert(fractional.hungerBefore == 98.25 && fractional.hungerAfter == 99.25);
    fractional = UpdateWorld(world, std::chrono::hours{1});
    assert(fractional.hungerAfter == 100.0 && fractional.atMaximumHunger == 1);
    assert(world.population().get(entity).thirst == 12.5f && world.population().get(entity).sleep == 33.75f);
    clock.EndClock();
    assert(clock.GetTickCount() == 5);
    assert(clock.GetSimulatedTime() == std::chrono::hours{5});
    assert(world.population().get(entity).hunger == 100.0f);
    const auto elapsed = clock.GetRealElapsedTime();
    clock.AdvanceTick();
    assert(clock.GetTickCount() == 5);
    assert(clock.GetRealElapsedTime() == elapsed);
    clock.StartClock();
    assert(clock.GetTickCount() == 0);
}
