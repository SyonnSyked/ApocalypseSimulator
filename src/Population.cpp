#include "Population.h"
#include "SimulatorConfig.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

PopulationPage::PopulationPage(std::size_t cap) : capacity(cap), hunger(new float[cap]), thirst(new float[cap]),
    sleep(new float[cap]), gender(new std::uint32_t[cap]), age(new std::uint8_t[cap]), hp(new std::uint8_t[cap]) {}
std::uint32_t PopulationStore::createPartition(entt::entity city, entt::entity district, std::uint32_t first,
                                               std::uint32_t stride, std::size_t expected) {
    if (!stride || expected > UINT32_MAX || partitions_.size() >= UINT32_MAX)
        throw std::length_error("Population partition exceeds ID limits");
    partitions_.push_back(PopulationPartition{city, district, first, stride, 0, expected, {}});
    return static_cast<std::uint32_t>(partitions_.size());
}
std::uint32_t PopulationStore::addLabel(const std::string &value) {
    auto it = std::find(labels_.begin(), labels_.end(), value);
    if (it != labels_.end()) return static_cast<std::uint32_t>(it - labels_.begin());
    if (labels_.size() >= UINT32_MAX) throw std::length_error("Too many gender labels");
    labels_.push_back(value); return static_cast<std::uint32_t>(labels_.size() - 1);
}
std::string PopulationStore::label(std::uint32_t index) const { return labels_.at(index); }
const PopulationPartition &PopulationStore::partition(std::uint32_t h) const {
    if (!h || h > partitions_.size()) throw std::out_of_range("Invalid population partition");
    return partitions_[h-1];
}
PersonId PopulationStore::id(std::uint32_t h, std::size_t slot) const {
    if (slot >= partition(h).count) throw std::out_of_range("Invalid person slot");
    return { (static_cast<std::uint64_t>(h) << 32) | static_cast<std::uint32_t>(slot) };
}
PersonId PopulationStore::createPerson(std::uint32_t h, const PersonStats &s) {
    partition(h); auto &p = partitions_[h-1];
    if (s.gender >= labels_.size()) throw std::out_of_range("Invalid gender index");
    if (p.count >= p.expected) throw std::length_error("Population partition full");
    if (p.pages.empty() || p.pages.back().size == p.pages.back().capacity)
        p.pages.emplace_back(std::min(pageCapacity, p.expected - p.count));
    auto &page = p.pages.back(); const auto n = page.size++;
    page.hunger[n]=s.hunger; page.thirst[n]=s.thirst; page.sleep[n]=s.sleep;
    page.gender[n]=s.gender; page.age[n]=s.age; page.hp[n]=s.hp; ++p.count;
    return id(h, p.count-1);
}
const PopulationPage &PopulationStore::resolve(PersonId value, std::size_t &offset) const {
    const auto h = static_cast<std::uint32_t>(value.value >> 32);
    const auto slot = static_cast<std::uint32_t>(value.value);
    const auto &p = partition(h);
    if (slot >= p.count) throw std::out_of_range("Invalid person ID");
    offset = slot % pageCapacity; return p.pages.at(slot / pageCapacity);
}
PersonStats PopulationStore::get(PersonId value) const {
    std::size_t n; const auto &p = resolve(value,n);
    return {p.hunger[n],p.thirst[n],p.sleep[n],p.gender[n],p.age[n],p.hp[n]};
}
void PopulationStore::set(PersonId value, const PersonStats &s) {
    if (s.gender >= labels_.size()) throw std::out_of_range("Invalid gender index");
    std::size_t n; resolve(value,n); auto &p = partitions_[static_cast<std::uint32_t>(value.value >> 32)-1].pages[static_cast<std::uint32_t>(value.value)/pageCapacity];
    p.hunger[n]=s.hunger; p.thirst[n]=s.thirst; p.sleep[n]=s.sleep; p.gender[n]=s.gender; p.age[n]=s.age; p.hp[n]=s.hp;
}
std::string PopulationStore::name(PersonId value) const {
    std::size_t n; resolve(value,n); const auto &p=partition(static_cast<std::uint32_t>(value.value>>32));
    return "Person " + std::to_string(static_cast<std::uint64_t>(p.firstName) + static_cast<std::uint64_t>(static_cast<std::uint32_t>(value.value))*p.nameStride);
}
PersonLocation PopulationStore::location(PersonId value) const {
    std::size_t n; resolve(value,n); const auto &p=partition(static_cast<std::uint32_t>(value.value>>32));
    return {p.city,p.district,entt::null};
}
PopulationMetrics PopulationStore::metrics() const {
    PopulationMetrics m; m.partitions=partitions_.size();
    for (const auto &p: partitions_) { m.people+=p.count; m.pages+=p.pages.size(); for (const auto &page:p.pages) m.capacity+=page.capacity; }
    m.occupiedBytes=static_cast<std::uint64_t>(m.people)*bytesPerSlot;
    m.allocatedBytes=static_cast<std::uint64_t>(m.capacity)*bytesPerSlot; return m;
}
void ValidatePersonConfig(const SimulatorConfig &c) {
    if (c.genders.empty() || c.genders.size()>UINT32_MAX) throw std::invalid_argument("Invalid gender labels");
    if (c.ageMin>c.ageMax || c.hpMin>c.hpMax || c.hungerMin>c.hungerMax || c.thirstMin>c.thirstMax || c.sleepMin>c.sleepMax ||
        c.ageMax>100 || c.hpMax>100 || c.hungerMax>100 || c.thirstMax>100 || c.sleepMax>100) throw std::invalid_argument("Invalid person attribute range");
}
void ValidateEntityCapacity(const entt::registry &r, std::size_t additional) {
    const auto used=r.storage<entt::entity>()->free_list(); constexpr auto cap=entt::entt_traits<entt::entity>::entity_mask;
    if (used>cap || additional>cap-used) throw std::length_error("World exceeds EnTT entity capacity");
}
