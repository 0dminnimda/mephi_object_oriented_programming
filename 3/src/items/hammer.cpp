#include <boost/config.hpp>  // BOOST_SYMBOL_EXPORT

#include "../game.hpp"

class Hammer : public MeleeWeapon {
public:
    float hit_range;

    Hammer() = default;
    Hammer(
        size_t item_class_index, RangeOfLong damage_range, float hit_range,
        float push_back_force_multiplier, sf::Time cooldown_time
    )
        : MeleeWeapon(item_class_index, damage_range, push_back_force_multiplier, cooldown_time),
          hit_range(hit_range) {}

    DeepCopy(Hammer) {
        MeleeWeapon::deepcopy_to(other);
        other.hit_range = hit_range;
    }

    std::shared_ptr<Item> deepcopy_item() const override { return deepcopy_shared(*this); }

    bool is_in_range(const Actor &source, sf::Vector2f target) const override {
        return length_squared(source.position - target) <= hit_range * hit_range;
    }

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(MeleeWeapon);
        ar &hit_range;
    }
};

BOOST_CLASS_EXPORT_KEY(Hammer);

class HammerPlugin : public ItemPlugin {
public:
    void add_classes_and_templates(std::vector<ItemPlugin::item_type> &result) const override {
        ItemClass hammer("hammer", "smashes in the face", "hammer.png", 13.0f, Item::Kind::Weapon);

        result.push_back(std::move(make_item_no_index<Hammer>(
            hammer, RangeOfLong(20, 40), 6.0f, 10000.0f, sf::seconds(1.0f)
        )));
    }
};

extern "C" BOOST_SYMBOL_EXPORT HammerPlugin item_plugin;
HammerPlugin item_plugin;
