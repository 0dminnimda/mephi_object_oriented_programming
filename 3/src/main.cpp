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
