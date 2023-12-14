#include <stdlib.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

#include "game.hpp"
#include "shared.hpp"

using std::min, std::max;

// float signed_distance_to_axis_aligned_rect(
//     const sf::Vector2f &point, const sf::Vector2f &top_left, const sf::Vector2f &bottom_right
// ) {
//     sf::Vector2f d = max(top_left - point, point - bottom_right);
//     return length(max(sf::Vector2f(0, 0), d)) + min(0.0f, max(d.x, d.y));
// }

// float signed_distance_from_rect_to_circle(
//     const sf::RectangleShape &rect, const sf::CircleShape &circle
// ) {
//     // rect.getGlobalBounds()
//     return signed_distance_to_axis_aligned_rect(
//                circle.getPosition(), rect.getPosition() - rect.getSize() / 2.0f,
//                rect.getPosition() + rect.getSize() / 2.0f
//            ) -
//            circle.getRadius();
// }

// void setup_default_actors(Game &game) {
//     size_t player_id =
//         game.add_actor_class(ActorClass("player", "plays the game", "hide_the_plan.jpeg"));
//     size_t goblin_id = game.add_actor_class(ActorClass("goblin", "deez nuts", "rock_smiling.jpeg"));
//     size_t pepe_id = game.add_actor_class(ActorClass("pepe", "he angy", "pepe_angry.jpeg"));

//     game.enemy_templates.resize(game.actor_classes.size());
//     game.player_template = Player(player_id, 10.0f, Characteristics(100.0f, 4.0f, 5.0f), 0);
//     game.player_template.pushable = false;
//     game.player_template.mass = 10000.0f;
//     game.enemy_templates[pepe_id] = Enemy(pepe_id, 5.0f, Characteristics(40.0f, 0.0f, 4.0f), 1);
//     game.enemy_templates[goblin_id] = Enemy(goblin_id, 7.0f, Characteristics(40.0f, 2.0f, 2.0f), 2);
// }

// void setup_default_items(Game &game) {
//     size_t hammer_id = game.add_item_class(
//         ItemClass("hammer", "smashes in the face", "hammer.png", 13.0f, Item::Kind::Weapon)
//     );
//     size_t sword_id = game.add_item_class(ItemClass(
//         "sword", "you can cut yourself just by looking at it", "sword_silver.png", 8.0f,
//         Item::Kind::Weapon
//     ));
//     size_t lock_pick_id = game.add_item_class(ItemClass(
//         "lock pick", "you sneaky pick", "lock_pick_with_fabric.png", 7.0f, Item::Kind::Custom
//     ));
//     game.item_classes[lock_pick_id].max_stack_size = 16;
//     size_t wooden_shield_id = game.add_item_class(ItemClass(
//         "wooden shield", "use protection", "shield_wood_metal.png", 8.0f, Item::Kind::Wearable
//     ));
//     size_t golden_shield_id = game.add_item_class(
//         ItemClass("golden shield", "magic", "shield_gold.png", 8.0f, Item::Kind::Wearable)
//     );
//     game.item_classes[golden_shield_id].artefact = CharacteristicsModifier();
//     game.item_classes[golden_shield_id].artefact->speed = AddToValue<float>(3.0f);

//     game.item_templates.resize(game.item_classes.size());
//     game.item_templates[hammer_id] =
//         std::make_unique<Hammer>(hammer_id, RangeOfLong(20, 40), 6.0f, 10000.0f, sf::seconds(1.0f));
//     game.item_templates[sword_id] =
//         std::make_unique<Sword>(sword_id, RangeOfLong(3, 5), 2.0f, 10.0f, sf::seconds(0.3f));
//     game.item_templates[lock_pick_id] = std::make_unique<LockPick>(lock_pick_id);
//     game.item_templates[wooden_shield_id] =
//         std::make_unique<Shield>(wooden_shield_id, RangeOfLong(10, 20));
//     game.item_templates[golden_shield_id] =
//         std::make_unique<Shield>(golden_shield_id, RangeOfLong(5, 10));

//     game.player_template.pick_up_item(game.make_item(hammer_id));
//     game.enemy_templates[game.actor_class_index_by_name("goblin")].pick_up_item(
//         game.make_item(sword_id)
//     );
// }

int sub_main() {
    Game &game = Game::get();

    // game.save_config("template_config.txt");
    if (!game.load_config("config.txt")) {
        game.setup_default_actors();
        game.setup_default_items();
    }

{
    DungeonLevel level;
    level.resize_tiles(30, 30);
    level.regenerate();
    game.dungeon.add_level(level);
}

{
    DungeonLevel level;
    level.resize_tiles(30, 30);
    level.regenerate();
    game.dungeon.add_level(level);
}

    if (!game.init(800, 600)) return EXIT_FAILURE;
    if (!game.run()) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

int main() {
    int result = EXIT_FAILURE;
    TRY_CATCH_ALL({ result = sub_main(); })
    return result;
}
