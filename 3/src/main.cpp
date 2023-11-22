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

    game.actor_classes.push_back(ActorClass("player", "plays"));

    Matrix<Tile> tiles(100, 100);

    for (size_t i = 1; i < tiles.size() - 1; ++i) {
        for (size_t j = 1; j < tiles.row_size() - 1; ++j) {
            tiles[i][j].kind = Tile::Flor;
        }
    }
    tiles[4][4].kind = Tile::OpenDor;
    tiles[4][5].kind = Tile::ClosedDor;

    Player player(0, 0, Characteristics(0, 0, 5));
    if (!player.texture.loadFromFile(path_to_resources + "hide_the_plan.jpeg")) return EXIT_FAILURE;
    player.size = 10.0f;

    DungeonLevel level(tiles, player);

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
