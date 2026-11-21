#include "World.h"
#include "SimulatorConfig.h"
#include <algorithm>
#include <limits>
#include <random>
#include <stdexcept>

entt::entity CreateRoad(SimulationWorld &w, entt::entity source, entt::entity destination, unsigned capacity, float flow) {
    if (w.failed_) throw std::logic_error("World is invalid after failed initialization");
    auto &r=w.registry_;
    if (!r.valid(source) || !r.valid(destination) || !r.all_of<District>(source) || !r.all_of<District>(destination))
        throw std::invalid_argument("Invalid road endpoint");
    ValidateEntityCapacity(r,1); const auto e=r.create(); r.emplace<Road>(e,Road{source,destination,capacity,flow}); return e;
}
entt::entity CreateDistrict(SimulationWorld &w, entt::entity city, const DistrictConfig &d) try {
    if (w.failed_) throw std::logic_error("World is invalid after failed initialization");
    auto &r=w.registry_;
    if (!r.valid(city) || !r.all_of<City>(city)) throw std::invalid_argument("Invalid district city");
    ValidateEntityCapacity(r,1);
    static thread_local std::mt19937 random(std::random_device{}());
    auto real=[&](float a,float b){ return std::uniform_real_distribution<float>(a,b)(random); };
    auto e=r.create();
    r.emplace<District>(e,District{d.districtName,city,0,std::uniform_int_distribution<std::size_t>(d.shelterMin,d.shelterMax)(random),real(d.sizeMin,d.sizeMax),real(d.foodMin,d.foodMax),real(d.waterMin,d.waterMax),0});
    r.get<City>(city).districts.push_back(e);
    auto h=w.population_.createPartition(city,e,1,1,0); r.get<District>(e).partition=h;
    return e;
} catch (...) { w.registry_.clear(); w.population_=PopulationStore{}; w.failed_=true; throw; }
entt::entity CreateCity(SimulationWorld &w, const CityConfig &c, const SimulatorConfig &config) try {
    if (w.failed_) throw std::logic_error("World is invalid after failed initialization");
    auto &r=w.registry_;
    ValidatePersonConfig(config);
    if (c.population>UINT32_MAX || config.districts.size()>UINT32_MAX) throw std::length_error("City population or district count exceeds ID limits");
    ValidateEntityCapacity(r,config.districts.size()+1);
    auto city=r.create(); r.emplace<City>(city,City{c.cityName,c.population,c.sizeSQKM,{ },0});
    // Create all districts before adding partitions, preserving stable entity references.
    static thread_local std::mt19937 districtRandom(std::random_device{}());
    for (const auto &d:config.districts) {
        auto real=[&](float a,float b){return std::uniform_real_distribution<float>(a,b)(districtRandom);};
        auto e=r.create(); r.emplace<District>(e,District{d.districtName,city,0,std::uniform_int_distribution<std::size_t>(d.shelterMin,d.shelterMax)(districtRandom),real(d.sizeMin,d.sizeMax),real(d.foodMin,d.foodMax),real(d.waterMin,d.waterMax),0});
        r.get<City>(city).districts.push_back(e);
    }
    auto &districts=r.get<City>(city).districts;
    const auto n=std::max<std::size_t>(1,districts.size());
    std::vector<std::uint32_t> handles; handles.reserve(n);
    for (std::size_t i=0;i<n;++i) {
        const auto count=c.population/n+(i<c.population%n ? 1:0);
        const auto district=districts.empty()?entt::null:districts[i];
        auto h=w.population_.createPartition(city,district,static_cast<std::uint32_t>(i+1),static_cast<std::uint32_t>(n),count);
        handles.push_back(h);
        if (district==entt::null) r.get<City>(city).fallbackPartition=h;
        else { auto &d=r.get<District>(district); d.partition=h; d.population=count; }
    }
    std::vector<std::uint32_t> labels; labels.reserve(config.genders.size());
    for (auto &label:config.genders) labels.push_back(w.population_.addLabel(label));
    std::mt19937 random(std::random_device{}());
    std::uniform_int_distribution<unsigned> age(config.ageMin,config.ageMax),hp(config.hpMin,config.hpMax),hunger(config.hungerMin,config.hungerMax),thirst(config.thirstMin,config.thirstMax),sleep(config.sleepMin,config.sleepMax);
    std::uniform_int_distribution<std::size_t> gender(0,labels.size()-1);
    for (std::size_t i=0;i<c.population;++i) {
        PersonStats s; s.gender=labels[gender(random)]; s.age=static_cast<std::uint8_t>(age(random)); s.hp=static_cast<std::uint8_t>(hp(random));
        s.hunger=static_cast<float>(hunger(random)); s.thirst=static_cast<float>(thirst(random)); s.sleep=static_cast<float>(sleep(random));
        w.population_.createPerson(handles[i%n],s);
    }
    return city;
} catch (...) { w.registry_.clear(); w.population_=PopulationStore{}; w.failed_=true; throw; }
void StartSimulatedWorld(SimulationWorld &w, const SimulatorConfig &config) {
    if (w.failed_) throw std::logic_error("World is invalid after failed initialization");
    if (w.registry_.storage<entt::entity>().free_list()!=0 || w.population_.metrics().partitions!=0) throw std::invalid_argument("World must be empty");
    ValidatePersonConfig(config);
    std::size_t total=0, people=0, partitions=0;
    for (const auto &c:config.cities) {
        if (c.population>UINT32_MAX || config.districts.size()>UINT32_MAX) throw std::length_error("Population exceeds ID limits");
        if (config.districts.size()>=std::numeric_limits<std::size_t>::max()-total) throw std::length_error("World entity count overflow");
        total+=config.districts.size()+1;
        const auto count=std::max<std::size_t>(1,config.districts.size());
        if (people > std::numeric_limits<std::size_t>::max()-c.population ||
            partitions > UINT32_MAX-count ||
            people > std::numeric_limits<std::uint64_t>::max()/PopulationStore::bytesPerSlot-c.population)
            throw std::length_error("World population or partition count overflow");
        people+=c.population; partitions+=count;
    }
    ValidateEntityCapacity(w.registry_,total);
    try { for (const auto &c:config.cities) CreateCity(w,c,config); }
    catch (...) { w.registry_.clear(); w.population_=PopulationStore{}; w.failed_=true; throw; }
}
WorldUpdateSummary UpdateWorld(SimulationWorld &w, std::chrono::hours elapsed) {
    WorldUpdateSummary result;
    for (auto &partition:w.population_.pagesForUpdate()) for (auto &page:partition.pages) for (std::size_t i=0;i<page.size;++i) {
        ++result.people; result.hungerBefore+=page.hunger[i];
        page.hunger[i]=std::min(100.0f,page.hunger[i]+static_cast<float>(elapsed.count()));
        result.hungerAfter+=page.hunger[i]; if (page.hunger[i]>=100.0f) ++result.atMaximumHunger;
    }
    return result;
}
