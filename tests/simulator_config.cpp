#include "Components.h"
#include "Population.h"
#include "SimulatorConfig.h"
#include "World.h"
#include <cstdio>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <limits>

int main()
{
    const char *path = "simulator_config_test.cfg";
    {
        std::ofstream file(path);
        file << "[city]\ncity_name = Test\npopulation = 25\ncity_size_sq_km = 1.5\n"
                "[city]\ncity_name = Second\npopulation = 3\ncity_size_sq_km = 2.5\n"
                "age_min = 42\nage_max = 42\nhp_min = 80\nhp_max = 80\n"
                "hunger_min = 12\nhunger_max = 12\nthirst_min = 13\nthirst_max = 13\n"
                "sleep_min = 90\nsleep_max = 90\ngenders = Test gender\n"
                "[district Shopping]\nshelter_min = 2\nshelter_max = 3\nsize_min = 1\nsize_max = 2\n"
                "food_min = 10\nfood_max = 20\nwater_min = 30\nwater_max = 40\n"
                "[district Medical]\nshelter_min = 4\nshelter_max = 4\nsize_min = 3\nsize_max = 3\n"
                "food_min = 5\nfood_max = 5\nwater_min = 6\nwater_max = 6\n";
    }
    auto config = LoadConfig(path);
    SimulationWorld world;
    StartSimulatedWorld(world, config);
    config.genders.clear();
    const auto &registry=world.registry();
    const auto &store=world.population();
    if (registry.view<City>().size()!=2 || registry.view<District>().size()!=4 || store.metrics().people!=28) return 1;
    for (auto city:registry.view<City>()) {
        const auto &c=registry.get<City>(city);
        if (c.districts.size()!=2) return 2;
        std::vector<bool> names(c.population+1);
        std::size_t total=0;
        for (auto district:c.districts) {
            const auto &d=registry.get<District>(district);
            if (d.city!=city || store.partition(d.partition).count!=d.population) return 3;
            if (d.districtName=="Shopping") {
                if (d.shelterAmount<2 || d.shelterAmount>3 || d.sizeSQKM<1 || d.sizeSQKM>2 || d.foodSupply<10 || d.foodSupply>20 || d.waterSupply<30 || d.waterSupply>40) return 4;
            } else if (d.districtName=="Medical") {
                if (d.shelterAmount!=4 || d.sizeSQKM!=3 || d.foodSupply!=5 || d.waterSupply!=6) return 5;
            } else return 6;
            for (std::size_t i=0;i<d.population;++i) {
                auto id=store.id(d.partition,i); auto person=store.get(id); auto location=store.location(id);
                if (person.age!=42 || person.hp!=80 || person.hunger!=12 || person.thirst!=13 || person.sleep!=90 || store.label(person.gender)!="Test gender" || location.city!=city || location.district!=district || location.road!=entt::null) return 7;
                auto name=store.name(id); auto index=std::stoul(name.substr(7));
                if (index==0 || index>c.population || names[index]) return 8;
                names[index]=true; ++total;
            }
        }
        if (total!=c.population) return 9;
    }
    auto ds=registry.view<District>(); auto it=ds.begin(); auto a=*it++; auto b=*it;
    auto road=CreateRoad(world,a,b,5,1.5f);
    if (registry.get<Road>(road).source!=a || registry.get<Road>(road).destination!=b || registry.get<Road>(road).maxCapacity!=5 || registry.get<Road>(road).maxFlowRate!=1.5f) return 10;
    bool rejected=false; try { CreateRoad(world,a,entt::null,1,1); } catch (const std::invalid_argument &) { rejected=true; }
    if (!rejected) return 11;
    { std::ofstream file(path,std::ios::app); file << "water_max = 2\n"; }
    rejected=false; try { LoadConfig(path); } catch (const std::exception &) { rejected=true; }
    std::remove(path); if (!rejected) return 12;
    SimulatorConfig huge; huge.cities.push_back({"Huge",std::numeric_limits<std::size_t>::max(),1});
    SimulationWorld empty; rejected=false;
    try { StartSimulatedWorld(empty,huge); } catch (const std::length_error &) { rejected=true; }
    if (!rejected || empty.registry().storage<entt::entity>()->free_list()!=0) return 13;
    SimulatorConfig noDistrict; noDistrict.cities.push_back({"No district",2,1}); StartSimulatedWorld(empty,noDistrict);
    if (empty.population().location(empty.population().id(1,0)).district!=entt::null) return 14;
    return 0;
}
