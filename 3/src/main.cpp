#include <stdlib.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

using std::min, std::max;

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

#include "game.hpp"
#include "vector_operations.hpp"

#define TRY_CATCH_ALL(code)                                    \
    {                                                          \
        try {                                                  \
            code;                                              \
        } catch (const std::exception &e) {                    \
            std::cout << "Error: " << e.what() << std::endl;   \
        } catch (...) {                                        \
            std::cout << "Unknwon error occured" << std::endl; \
        }                                                      \
    }

float signed_distance_to_axis_aligned_rect(
    const sf::Vector2f &point, const sf::Vector2f &top_left, const sf::Vector2f &bottom_right
) {
    sf::Vector2f d = max(top_left - point, point - bottom_right);
    return length(max(sf::Vector2f(0, 0), d)) + min(0.0f, max(d.x, d.y));
}

float signed_distance_from_rect_to_circle(
    const sf::RectangleShape &rect, const sf::CircleShape &circle
) {
    return signed_distance_to_axis_aligned_rect(
               circle.getPosition(), rect.getPosition() - rect.getSize() / 2.0f,
               rect.getPosition() + rect.getSize() / 2.0f
           ) -
           circle.getRadius();
}

int sub_main() {
    Game &game = Game::get();

    size_t player_id =
        game.add_actor_class(ActorClass("player", "plays the game", "hide_the_plan.jpeg"));
    size_t goblin_id = game.add_actor_class(ActorClass("goblin", "deez nuts", "rock_smiling.jpeg"));
    size_t pepe_id = game.add_actor_class(ActorClass("pepe", "hes angy", "pepe_angry.jpeg"));

    game.enemy_templates.resize(game.actor_classes.size());
    game.player_template = Player(player_id, 10.0f, Characteristics(100.0f, 20.0f, 5.0f));
    game.player_template.lock_picks.count = 100;
    game.player_template.pushable = false;
    game.player_template.mass = 1000.0f;
    game.enemy_templates[goblin_id] = Enemy(goblin_id, 7.0f, Characteristics(40.0f, 2.0f, 2.0f));
    game.enemy_templates[pepe_id] = Enemy(pepe_id, 5.0f, Characteristics(40.0f, 0.0f, 4.0f));

    size_t hammer_id = game.add_item_class(
        ItemClass("hammer", "smashes in the face", "hammer.png", 13.0f, Item::Kind::Weapon)
    );
    size_t sword_id = game.add_item_class(ItemClass(
        "sword", "you can cut yourself just by looking at it", "sword_silver.png", 8.0f,
        Item::Kind::Weapon
    ));
    size_t lock_pick_id = game.add_item_class(ItemClass(
        "lock pick", "you sneaky pick", "lock_pick_with_fabric.png", 7.0f,
        Item::Kind::Custom
    ));

    game.item_templates.resize(game.item_classes.size());
    game.item_templates[hammer_id] = std::make_unique<Hammer>(hammer_id, RangeOfLong(10, 20), 3.0f, 10000.0f);
    game.item_templates[sword_id] = std::make_unique<Sword>(sword_id, RangeOfLong(3, 5), 2.0f, 100.0f);
    game.item_templates[lock_pick_id] = std::make_unique<LockPicks>(lock_pick_id, 20);

    DungeonLevel level;

    game.player_template.pick_up_item(game.make_item(hammer_id));
    game.enemy_templates[goblin_id].pick_up_item(game.make_item(sword_id));

    level.tile_size = 10.0f;
    level.chest_size_factor = 1.0f;

    level.resize_tiles(30, 30);
    level.regenerate_tiles();

    level.initial_player_position = level.center();

    level.regenerate_enemies();

    game.dungeon.add_level(level);

    if (!game.init(1200, 900)) return EXIT_FAILURE;
    if (!game.run()) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

int main() {
    int result = EXIT_FAILURE;
    TRY_CATCH_ALL({ result = sub_main(); })
    return result;
}
