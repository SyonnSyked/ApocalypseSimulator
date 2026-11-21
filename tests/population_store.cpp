#include "World.h"
#include "SimulatorConfig.h"
#include <stdexcept>
#include <limits>
#include <vector>

int main() {
    for (std::size_t count : {std::size_t(0),std::size_t(1),std::size_t(16383),std::size_t(16384),std::size_t(16385),std::size_t(50000)}) {
        SimulatorConfig c; c.cities.push_back({"Test",count,1}); c.ageMin=c.ageMax=42; c.hpMin=c.hpMax=81;
        c.hungerMin=c.hungerMax=12; c.thirstMin=c.thirstMax=13; c.sleepMin=c.sleepMax=14;
        SimulationWorld w; StartSimulatedWorld(w,c); const auto &p=w.population(); auto m=p.metrics();
        if (m.people!=count || m.capacity!=count || m.pages!=(count+16383)/16384 || m.partitions!=1 || m.occupiedBytes!=count*18) return 1;
        if (w.registry().storage<entt::entity>()->free_list()!=1) return 2;
        if (count) {
            auto first=p.id(1,0); auto last=p.id(1,count-1); auto v=p.get(last);
            if (p.name(first)!="Person 1" || p.name(last)!="Person "+std::to_string(count) || v.age!=42 || v.hp!=81 || v.hunger!=12 || v.thirst!=13 || v.sleep!=14) return 3;
            if (count>1 && first.value==last.value) return 4;
            v.hunger=99.5f; v.thirst=4.25f; v.sleep=7.75f; w.setPerson(last,v);
            auto summary=UpdateWorld(w,std::chrono::hours(1));
            if (summary.people!=count || summary.atMaximumHunger!=1 || w.population().get(last).hunger!=100 || w.population().get(last).thirst!=4.25f || w.population().get(last).sleep!=7.75f) return 5;
        }
        bool invalid=false; try { p.get(PersonId{}); } catch (const std::out_of_range &) { invalid=true; } if (!invalid) return 6;
    }
    for (int i=0;i<3;++i) {
        SimulatorConfig c; c.cities.push_back({"A",3,1}); c.cities.push_back({"B",3,1});
        c.genders={"A very long custom gender label", "Other"}; SimulationWorld w; StartSimulatedWorld(w,c); c.genders.clear();
        if (w.population().id(1,0).value==w.population().id(2,0).value) return 7;
        auto id=w.population().id(1,0); auto saved=w.population().get(id); auto label=w.population().label(saved.gender);
        if (label!="A very long custom gender label" && label!="Other") return 8;
        SimulatorConfig extra; extra.genders={"A new label"};
        CreateCity(w, CityConfig{"C",1,1}, extra);
        if (w.population().label(saved.gender)!=label || w.population().get(id).age!=saved.age) return 9;
    }
    return 0;
}
