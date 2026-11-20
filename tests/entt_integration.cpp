#include <entt/entity/registry.hpp>

struct Population {
    int value;
};

int main()
{
    entt::registry world;
    const auto city = world.create();
    world.emplace<Population>(city, 100);

    int total = 0;
    for (const auto entity : world.view<Population>()) {
        total += world.get<Population>(entity).value;
    }
    if (total != 100) {
        return 1;
    }

    world.destroy(city);
    return world.valid(city) || !world.view<Population>().empty();
}
