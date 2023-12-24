#include <boost/config.hpp>  // BOOST_SYMBOL_EXPORT

#include "../game.hpp"

class LockPick : public Item {
public:
    DeepCopy(LockPick) { Item::deepcopy_to(other); }

    static constexpr float picking_range = 1.5f;

    LockPick() = default;
    LockPick(size_t item_class_index) : Item(item_class_index) {}

    ItemUseResult use(Actor &target) override;
    std::shared_ptr<Item> deepcopy_item() const override { return deepcopy_shared(*this); }

private:
    const Tile *find_best_choice(Actor &target) const;

    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Item);
    }
};

BOOST_CLASS_EXPORT_KEY(LockPick);

const Tile *LockPick::find_best_choice(Actor &target) const {
    auto &level = Game::get().dungeon.current_level;
    if (!level) return nullptr;

    auto coords = level->get_tile_coordinates(target.position);
    if (!coords) return nullptr;

    auto iter = level->tiles.find([&coords, &level](const Tile &tile) -> bool {
        auto [i, j] = level->tiles.indices_of(tile);
        float distance =
            length_squared(sf::Vector2f(i, j) - sf::Vector2f(coords->first, coords->second));
        bool close_enough = distance <= picking_range * picking_range;
        return close_enough && tile.building;
    });

    const Tile *closest_tile = nullptr;
    float min_dist_sqared =
        length_squared(sf::Vector2f(level->tiles.row_count(), level->tiles.column_count()));
    for (; iter != level->tiles.end(); ++iter) {
        auto &tile = *iter;
        if (!tile.building) continue;
        auto [i, j] = level->tiles.indices_of(tile);
        std::cout << "iter " << i << " " << j << std::endl;

        float item_dist_sqared =
            length_squared(sf::Vector2f(i, j) - sf::Vector2f(coords->first, coords->second));
        if (item_dist_sqared < min_dist_sqared) {
            min_dist_sqared = item_dist_sqared;
            closest_tile = &tile;
        }
    }
    return closest_tile;
}

ItemUseResult LockPick::use(Actor &target) {
    auto &level = Game::get().dungeon.current_level;
    if (!level) return ItemUseResult();

    Tile *tile = (Tile *)find_best_choice(target);
    if (tile == nullptr) return ItemUseResult();
    if (!tile->building) return ItemUseResult();

    auto [x, y] = level->tiles.indices_of(*tile);

    auto result = tile->building->simulate_picking(target);

    auto tile_position = sf::Vector2f(x, y) * level->tile_coords_to_world_coords_factor();
    if (result.lock_picked) {
        for (auto &slot : tile->building->inventory.slots) {
            for (size_t k = 0; k < slot.size; ++k) {
                LayingItem laying_item(slot.item->deepcopy_item(), tile_position);
                level->laying_items.push_back(laying_item);
            }
        }
        tile->building = nullptr;
    }

    return ItemUseResult(result.pick_broken);
}

class LockPickPlugin : public ItemPlugin {
public:
    void add_classes_and_templates(std::vector<ItemPlugin::item_type> &result) const override {
        ItemClass lock_pick(
            "lock pick", "you sneaky pick", "lock_pick_with_fabric.png", 7.0f, Item::Kind::Custom
        );
        lock_pick.max_stack_size = 16;

        result.push_back(std::move(make_item_no_index<LockPick>(lock_pick)));
    }
};

extern "C" BOOST_SYMBOL_EXPORT LockPickPlugin item_plugin;
LockPickPlugin item_plugin;
