#include <boost/config.hpp>  // BOOST_SYMBOL_EXPORT

#include "game.hpp"

class Shield : public Wearable {
public:
    DeepCopy(Shield) { Wearable::deepcopy_to(other); }

    Shield() = default;
    Shield(size_t item_class_index, RangeOfLong defence_range)
        : Wearable(item_class_index, Wearable::Shield, defence_range) {}

    std::shared_ptr<Item> deepcopy_item() const override { return deepcopy_shared(*this); }

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wearable);
    }
};

BOOST_CLASS_EXPORT_KEY(Shield);

class ShieldPlugin : public ItemPlugin {
public:
    std::vector<std::pair<ItemClass, std::unique_ptr<Item>>> get_item_classes_and_templates(
    ) const override {
        ItemClass wooden_shield(
            "wooden shield", "use protection", "shield_wood_metal.png", 8.0f, Item::Kind::Wearable
        );

        ItemClass golden_shield(
            "golden shield", "magic", "shield_gold.png", 8.0f, Item::Kind::Wearable
        );
        golden_shield.artefact = CharacteristicsModifier();
        golden_shield.artefact->speed = AddToValue<float>(3.0f);

        std::vector<ItemPlugin::item_type> result;
        result.push_back(std::move(make_item<Shield>(wooden_shield, 0, RangeOfLong(10, 20))));
        result.push_back(std::move(make_item<Shield>(golden_shield, 0, RangeOfLong(5, 10))));
        return result;
    }
};

// extern "C" BOOST_SYMBOL_EXPORT ShieldPlugin shield_plugin;
ShieldPlugin shield_plugin;
