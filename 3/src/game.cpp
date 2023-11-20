#include <random>

#include "game.hpp"


void CharacteristicsModifier::apply(Characteristics &value) {
    if (max_health) {
        value.max_health = std::visit([&](auto &&v) -> auto { return (long)v.apply(value.max_health); }, *max_health);
    }
    
    if (defence) {
        value.defence = std::visit([&](auto &&v) -> auto { return (long)v.apply(value.defence); }, *defence);
    }

    if (speed) {
        value.speed = std::visit([&](auto &&v) -> auto { return (long)v.apply(value.speed); }, *speed);
    }
}


void Potion::apply(Actor &target) {
    modifier.apply(target.characteristics());
}


void Potion::use(Actor &target) {
    apply(target);
}


Tile &Tile::set_building(std::unique_ptr<Chest> building) {
    this->building = std::move(building);
    return *this;
}

long RangeOfValues::get_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> dis(min, max);
    return dis(gen);
}
