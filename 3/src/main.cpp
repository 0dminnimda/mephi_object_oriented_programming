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

    size_t player_id = game.add_actor_class(ActorClass(
        "player", "plays the game", "hide_the_plan.jpeg", 10.0f, Characteristics(100.0f, 0.0f, 5.0f)
    ));
    size_t goblin_id = game.add_actor_class(ActorClass(
        "goblin", "deez nuts", "rock_smiling.jpeg", 5.0f, Characteristics(20.0f, 0.0f, 2.0f)
    ));
    size_t pepe_id = game.add_actor_class(
        ActorClass("pepe", "hes angy", "pepe_angry.jpeg", 3.0f, Characteristics(10.0f, 0.0f, 4.0f))
    );

    DungeonLevel level;

    Hammer item(RangeOfValues(100, 120));
    level.player.pick_up_item(item);

    level.resize_tiles(30, 30);
    level.regenerate_tiles();

    level.initial_player_position = level.center();

    level.regenerate_enemies();

    game.add_level(level);

    if (!game.init(800, 600)) return EXIT_FAILURE;
    if (!game.run()) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

int main() {
    try {
        return sub_main();
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
