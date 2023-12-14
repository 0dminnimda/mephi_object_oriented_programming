#include <stdexcept>
#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/vector_operations.hpp"
#include "../src/game.cpp"

namespace fs = std::filesystem;

const static std::string save_path = (fs::path(__FILE__).parent_path() / "test_save.txt").string();

TEST_CASE("suit") {
    SUBCASE("serialization") {
        
        SUBCASE("saving") {
            Game &game = Game::get();

            game.setup_default_actors();
            game.setup_default_items();
            game.save(save_path);

            CHECK(true /*deeznuts*/);
        }

        SUBCASE("loading") {
            Game &game = Game::get();
            game.load(save_path);

            CHECK(true /*deeznuts*/);
        }
    }
    /*SUBCASE("fuck") {
        DungeonLevel level;
        level.resize_tiles(30, 30);
        level.regenerate();
        //game.dungeon.add_level(level);

        CHECK(level.tiles.size() == 30*30);
    }*/
    
    SUBCASE("Testing dot function") {
       sf::Vector2<float> a(1.0f, 2.0f);
       sf::Vector2<float> b(3.0f, 4.0f);
       CHECK(dot(a, b) == 11.0f);
    }

    SUBCASE("Testing length function") {
       sf::Vector2<float> a(3.0f, 4.0f);
       CHECK(length(a) == doctest::Approx(5.0f));
    }

    SUBCASE("Testing vector operations") {
       SUBCASE("Testing dot function") {
           sf::Vector2<float> a(1.0f, 2.0f);
           sf::Vector2<float> b(3.0f, 4.0f);
           CHECK(dot(a, b) == 11.0f);
       }
    
       SUBCASE("Testing length function") {
           sf::Vector2<float> a(3.0f, 4.0f);
           CHECK(length(a) == doctest::Approx(5.0f));
       }
    }
}
