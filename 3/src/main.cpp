#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>

using std::min, std::max;

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

#include "game.hpp"
#include "vector_operations.hpp"

float signed_distance_to_axis_aligned_rect(const sf::Vector2f &point, const sf::Vector2f &top_left, const sf::Vector2f &bottom_right)
{
    sf::Vector2f d = max(top_left-point, point-bottom_right);
    return length(max(sf::Vector2f(0, 0), d)) + min(0.0f, max(d.x, d.y));
}


float signed_distance_from_rect_to_circle(const sf::RectangleShape &rect, const sf::CircleShape &circle) {
    return signed_distance_to_axis_aligned_rect(circle.getPosition(), rect.getPosition() - rect.getSize() / 2.0f, rect.getPosition() + rect.getSize() / 2.0f) - circle.getRadius();
}

int sub_main() {
    Game &game = Game::get();

    game.actor_classes.push_back(ActorClass(
        "player",
        "plays"
    ));

    Matrix<Tile> tiles(20, 20);

    tiles[4][4].kind = Tile::OpenDor;
    tiles[4][5].kind = Tile::ClosedDor;

    Player player(0, 0, Characteristics());
    if (!player.texture.loadFromFile(path_to_resources + "hide_the_plan.jpeg")) return EXIT_FAILURE;
    player.position = sf::Vector2f(30, 30);
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
