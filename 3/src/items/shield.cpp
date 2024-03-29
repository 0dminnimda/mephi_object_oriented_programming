#include <boost/config.hpp>  // BOOST_SYMBOL_EXPORT

#include "../game.hpp"

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
    void add_classes_and_templates(std::vector<ItemPlugin::item_type> &result) const override {
        ItemClass wooden_shield(
            "wooden shield", "use protection", "shield_wood_metal.png", 8.0f, Item::Kind::Wearable
        );

        ItemClass golden_shield(
            "golden shield", "magic", "shield_gold.png", 8.0f, Item::Kind::Wearable
        );
        golden_shield.artefact = CharacteristicsModifier();
        golden_shield.artefact->speed = AddToValue<float>(3.0f);

        result.push_back(std::move(make_item_no_index<Shield>(wooden_shield, RangeOfLong(10, 20))));
        result.push_back(std::move(make_item_no_index<Shield>(golden_shield, RangeOfLong(5, 10))));
    }
};

extern "C" BOOST_SYMBOL_EXPORT ShieldPlugin item_plugin;
ShieldPlugin item_plugin;
