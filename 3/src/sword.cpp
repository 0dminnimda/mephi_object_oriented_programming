#include "game.hpp"
#include "vector_operations.hpp"

class Sword : public MeleeWeapon {
public:
    DeepCopy(Sword);

    // TODO: add direction dependant range
    float hit_range;

    Sword() = default;
    Sword(
        size_t item_class_index, RangeOfLong damage_range, float hit_range,
        float push_back_force_multiplier, sf::Time cooldown_time
    )
        : MeleeWeapon(item_class_index, damage_range, push_back_force_multiplier, cooldown_time),
          hit_range(hit_range) {}

    std::shared_ptr<Item> deepcopy_item() const override;

    bool is_in_range(const Actor &source, sf::Vector2f target) const override;

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(MeleeWeapon);
        ar &hit_range;
    }
};

BOOST_CLASS_EXPORT_KEY(Sword);

bool Sword::is_in_range(const Actor &source, sf::Vector2f target) const {
    return length_squared(source.position - target) <= hit_range * hit_range;
}

void Sword::deepcopy_to(Sword &other) const {
    MeleeWeapon::deepcopy_to(other);
    other.hit_range = hit_range;
}

std::shared_ptr<Item> Sword::deepcopy_item() const { return deepcopy_shared(*this); }

class SwordPlugin : public ItemPlugin {
public:
    std::vector<std::pair<ItemClass, std::unique_ptr<Item>>> get_item_classes_and_templates(
    ) const override {
        ItemClass sword(
            "sword", "you can cut yourself just by looking at it", "sword_silver.png", 8.0f,
            Item::Kind::Weapon
        );

        std::vector<ItemPlugin::item_type> result;
        result.push_back(std::move(
            make_item<Sword>(sword, 0, RangeOfLong(3, 5), 2.0f, 10.0f, sf::seconds(0.3f))
        ));
        return result;
    }
};

// extern "C" BOOST_SYMBOL_EXPORT SwordPlugin sword_plugin;
SwordPlugin sword_plugin;
