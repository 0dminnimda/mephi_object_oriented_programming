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

    size_t player_id =
        game.add_actor_class(ActorClass("player", "plays the game", "hide_the_plan.jpeg"));
    size_t goblin_id = game.add_actor_class(ActorClass("goblin", "deez nuts", "rock_smiling.jpeg"));
    size_t pepe_id = game.add_actor_class(ActorClass("pepe", "hes angy", "pepe_angry.jpeg"));

    Player player(player_id, 100.0f, 10.0f, Characteristics(0.0f, 0.0f, 5.0f));
    DungeonLevel level(player);

    level.resize_tiles(30, 30);
    level.regenerate();

    level.initial_player_position = level.center();

    for (size_t i = 0; i < 10; ++i) {
        Enemy enemy(goblin_id, 20.0f, 5.0f, Characteristics(0.0f, 0.0f, 2.0f));
        enemy.position.x = i;
        enemy.position.y = i;
        enemy.characteristics().speed += (i % 2 == 0 ? 0 : -1);
        level.enemies.push_back(enemy);
    }

    for (size_t i = 0; i < 10; ++i) {
        Enemy enemy(pepe_id, 10.0f, 3.0f, Characteristics(0.0f, 0.0f, 4.0f));
        enemy.position.x = 2 * i;
        enemy.position.y = i;
        level.enemies.push_back(enemy);
    }

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
