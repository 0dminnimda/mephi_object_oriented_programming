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

int main() {
    Game &game = Game::get();

    game.actor_classes.push_back(ActorClass(
        "player",
        "plays"
    ));

    Matrix<Tile> tiles(10, 10);

    tiles[4][4].kind = Tile::OpenDor;
    tiles[4][5].kind = Tile::ClosedDor;

    Player player(0, 0, Characteristics());
    DungeonLevel level(tiles, player);

    game.add_level(level);

    int result = game.init(800, 600);
    if (result != EXIT_SUCCESS) return result;
    return game.run();
}

