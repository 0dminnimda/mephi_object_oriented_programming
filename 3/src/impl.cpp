#include <random>

#include "design.hpp"


void CharacteristicsModifier::apply(Characteristics &value) {
    max_health.apply(value.max_health);
    defence.apply(value.defence);
    speed.apply(value.speed);
}


void Potion::apply(Actor &target) {
    modifier.apply(target.characteristics());
}


void Potion::use(Actor &target) override {
    apply(target);
}


Tile &Tile::set_building(std::unique_ptr<Chest> building) {
    this->building = building;
    return *this;
}

float RangeOfValues::get_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}
