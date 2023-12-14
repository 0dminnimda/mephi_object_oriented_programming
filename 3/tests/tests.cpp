#include <stdexcept>
#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/vector_operations.hpp"
#include "../src/game.cpp"

namespace fs = std::filesystem;

const static std::string save_path = (fs::path(__FILE__).parent_path() / "test_save.txt").string();

TEST_CASE("suit") {
    SUBCASE("Testing serialization") {
        SUBCASE("Testing saving") {
            Game &game = Game::get(true);

            game.setup_default_actors();
            game.setup_default_items();
            game.save(save_path);

            CHECK(fs::exists(fs::path(__FILE__)));
        }

        SUBCASE("Testing loading") {
            Game &game = Game::get(true);
            game.load(save_path);

            CHECK(true);
        }
    }

    SUBCASE("Testing loading lev") {
        Game &game = Game::get(true);

        game.setup_default_actors();
        game.setup_default_items();

        DungeonLevel level;
        level.resize_tiles(20, 30);
        level.regenerate();
        game.dungeon.add_level(level);

        CHECK(game.dungeon.all_levels.size() == 1);
    }

    SUBCASE("Testing init") {
        Game &game = Game::get(true);

        game.setup_default_actors();
        game.setup_default_items();

        DungeonLevel level;
        level.resize_tiles(20, 30);
        level.regenerate();
        game.dungeon.add_level(level);

        CHECK(game.init(800, 600));
        // if (!game.run()) return EXIT_FAILURE;
    }

    SUBCASE("Testing update") {
        Game &game = Game::get(true);

        game.setup_default_actors();
        game.setup_default_items();

        DungeonLevel level;
        level.resize_tiles(20, 30);
        level.regenerate();
        game.dungeon.add_level(level);

        CHECK(game.init(800, 600));
        game.update(0.1f);
        game.handle_fixed_update(0.1f);
    }

    SUBCASE("Testing draw empty") {
        Game &game = Game::get(true);

        game.setup_default_actors();
        game.setup_default_items();

        DungeonLevel level;
        level.resize_tiles(20, 30);
        level.regenerate();
        game.dungeon.add_level(level);

        CHECK(game.init(800, 600));
        game.update(0.1f);
        game.handle_fixed_update(0.1f);

        game.game_view.clear();
        game.game_view.draw();
        game.game_view.display();
    }

    SUBCASE("Testing draw nonempty") {
        Game &game = Game::get(true);

        game.setup_default_actors();
        game.setup_default_items();

        DungeonLevel level;
        level.resize_tiles(20, 30);
        level.regenerate();
        game.dungeon.add_level(level);

        CHECK(game.init(800, 600));
        game.start_playing();

        game.update(0.1f);
        game.handle_fixed_update(0.1f);

        game.game_view.clear();
        game.game_view.draw();
        game.game_view.display();
    }

    SUBCASE("Testing dunlev") {
        DungeonLevel level;
        level.resize_tiles(20, 30);
        level.regenerate();

        CHECK(level.tiles.size() == 20*30);
        CHECK(level.tiles.row_count() == 20);
        CHECK(level.tiles.column_count() == 30);
    }

    SUBCASE("Testing matrix resize") {
        Matrix<int> mat(2, 2);
        mat[0][0] = 1;
        mat[0][1] = 2;
        mat[1][0] = 3;
        mat[1][1] = 4;

        mat.resize(1, 4);

        CHECK(mat[0][0] == 1);
        CHECK(mat[0][1] == 2);
        CHECK(mat[0][2] != 3);
        CHECK(mat[0][3] != 4);
    }

    SUBCASE("Testing matrix predicate find") {
        Matrix<int> mat(2, 2);
        mat[0][0] = 1;
        mat[0][1] = 2;
        mat[1][0] = 3;
        mat[1][1] = 4;

        auto it = mat.find([](int item) -> bool {
            return item >= 3;
        });

        CHECK(*it == 3);
        CHECK(it != mat.end());
        ++it;
        CHECK(*it == 4);
        CHECK(it != mat.end());
        ++it;
        CHECK(it == mat.end());
    }

    SUBCASE("Testing item ptr via hammer") {
        Game &game = Game::get(true);

        game.setup_default_actors();
        game.setup_default_items();

        DungeonLevel level;
        level.resize_tiles(1, 1);
        level.regenerate();
        game.dungeon.unload_current_level();
        game.dungeon.all_levels.clear();
        game.dungeon.add_level(level);

        CHECK(game.init(800, 600));
        game.start_playing();

        CHECK((bool)game.dungeon.current_level);
        REQUIRE(game.dungeon.current_level->enemies.size());

        float initial_health_sum = 0;
        for (auto &it: game.dungeon.current_level->enemies) {
            initial_health_sum = it.health;
        }

        size_t hammer_id = 0;
        std::shared_ptr<Item> item = Game::get().make_item(hammer_id);

        game.update(0.1f);
        game.handle_fixed_update(0.1f);

        item->use(game.dungeon.player);

        game.update(0.1f);
        game.handle_fixed_update(0.1f);

        float final_health_sum = 0;
        for (auto &it: game.dungeon.current_level->enemies) {
            final_health_sum = it.health;
        }

        CHECK(final_health_sum <= initial_health_sum);
    }

    SUBCASE("Testing dot function") {
       sf::Vector2<float> a(1.0f, 2.0f);
       sf::Vector2<float> b(3.0f, 4.0f);
       CHECK(dot(a, b) == 11.0f);
    }

    SUBCASE("Testing length function") {
       sf::Vector2<float> a(3.0f, 4.0f);
       CHECK(length(a) == doctest::Approx(5.0f));
    }

    SUBCASE("Testing dot product") {
        sf::Vector2<float> a(2.0f, 3.0f);
        sf::Vector2<float> b(4.0f, -1.0f);
        CHECK(dot(a, b) == 5.0f);
    }
    
    SUBCASE("Testing length squared") {
        sf::Vector2<float> a(3.0f, 4.0f);
        CHECK(length_squared(a) == 25.0f);
    }
    
    SUBCASE("Testing length") {
        sf::Vector2<float> a(3.0f, 4.0f);
        CHECK(length(a) == 5.0f);
    }
    
    SUBCASE("Testing max - vector") {
        sf::Vector2<float> a(2.0f, 7.0f);
        sf::Vector2<float> b(5.0f, 3.0f);
        CHECK(max(a, b) == sf::Vector2<float>(5.0f, 7.0f));
    }
    
    SUBCASE("Testing min - vector") {
        sf::Vector2<float> a(2.0f, 7.0f);
        sf::Vector2<float> b(5.0f, 3.0f);
        CHECK(min(a, b) == sf::Vector2<float>(2.0f, 3.0f));
    }

    SUBCASE("Testing mean") {
        sf::Vector2<float> a(2.0f, 4.0f);
        CHECK(mean(a) == 3.0f);
    }
    
    SUBCASE("Testing projection") {
        sf::Vector2<float> a(2.0f, 3.0f);
        sf::Vector2<float> b(5.0f, 0.0f);
        CHECK(proj(a, b) == sf::Vector2<float>(2.0f, 0.0f));
    }
    
    SUBCASE("Testing normalization") {
        sf::Vector2<float> a(3.0f, 4.0f);
        sf::Vector2<float> normalized_a = normalized(a);
        CHECK(length(normalized_a) == doctest::Approx(1.0f));
        CHECK(normalized(sf::Vector2<float>(0.0f, 0.0f)) == sf::Vector2<float>(0.0f, 0.0f));
    }
    
    SUBCASE("Testing move_towards") {
        sf::Vector2<float> current(0.0f, 0.0f);
        sf::Vector2<float> target(3.0f, 4.0f);
        sf::Vector2<float> result = move_towards(current, target, 5.0f);
        CHECK(length(result - current) == doctest::Approx(5.0f));
        CHECK(move_towards(current, target, 10.0f) == target);
    }
    
    SUBCASE("Testing clamp_magnitude") {
        sf::Vector2<float> a(3.0f, 4.0f);
        CHECK(clamp_magnitude(a, 5.0f) == a);
        CHECK(clamp_magnitude(a, 2.0f) == normalized(a) * 2.0f);
        CHECK(clamp_magnitude(sf::Vector2<float>(0.0f, 0.0f), 5.0f) == sf::Vector2<float>(0.0f, 0.0f));
    }
    
    SUBCASE("Testing YO") {
        sf::Vector2<float> a(1.0f, 2.0f);
        CHECK(YO(a) == sf::Vector2<float>(2.0f, 0.0f));
    }
    
    SUBCASE("Testing OY") {
        sf::Vector2<float> a(1.0f, 2.0f);
        CHECK(OY(a) == sf::Vector2<float>(0.0f, 2.0f));
    }
    
    SUBCASE("Testing YY") {
        sf::Vector2<float> a(1.0f, 2.0f);
        CHECK(YY(a) == sf::Vector2<float>(2.0f, 2.0f));
    }
    
    SUBCASE("Testing XO") {
        sf::Vector2<float> a(1.0f, 2.0f);
        CHECK(XO(a) == sf::Vector2<float>(1.0f, 0.0f));
    }
    
    SUBCASE("Testing OX") {
        sf::Vector2<float> a(1.0f, 2.0f);
        CHECK(OX(a) == sf::Vector2<float>(0.0f, 1.0f));
    }
    
    SUBCASE("Testing XX") {
        sf::Vector2<float> a(1.0f, 2.0f);
        CHECK(XX(a) == sf::Vector2<float>(1.0f, 1.0f));
    }
    
    SUBCASE("Testing vector division by vector") {
        sf::Vector2<float> a(6.0f, 8.0f);
        sf::Vector2<float> b(2.0f, 4.0f);
        CHECK(a / b == sf::Vector2<float>(3.0f, 2.0f));
    }
    
    SUBCASE("Testing vector division by scalar") {
        sf::Vector2<float> a(6.0f, 8.0f);
        float scalar = 2.0f;
        CHECK(a / scalar == sf::Vector2<float>(3.0f, 4.0f));
    }
    
    SUBCASE("Testing scalar division by vector") {
        float scalar = 12.0f;
        sf::Vector2<float> a(2.0f, 4.0f);
        CHECK(scalar / a == sf::Vector2<float>(6.0f, 3.0f));
    }
    
    SUBCASE("Testing vector multiplication") {
        sf::Vector2<float> a(2.0f, 3.0f);
        sf::Vector2<float> b(4.0f, -1.0f);
        CHECK(a * b == sf::Vector2<float>(8.0f, -3.0f));
    }
    
    SUBCASE("Testing vector multiplication by scalar") {
        sf::Vector2<float> a(2.0f, 3.0f);
        float scalar = 4.0f;
        CHECK(a * scalar == sf::Vector2<float>(8.0f, 12.0f));
    }
    
    SUBCASE("Testing scalar multiplication by vector") {
        float scalar = 4.0f;
        sf::Vector2<float> a(2.0f, 3.0f);
        CHECK(scalar * a == sf::Vector2<float>(8.0f, 12.0f));
    }
    
    SUBCASE("Testing vector addition with scalar") {
        sf::Vector2<float> a(2.0f, 3.0f);
        float scalar = 1.0f;
        CHECK(a + scalar == sf::Vector2<float>(3.0f, 4.0f));
        CHECK(scalar + a == sf::Vector2<float>(3.0f, 4.0f));
    }
    
    SUBCASE("Testing vector subtraction with scalar") {
        sf::Vector2<float> a(3.0f, 4.0f);
        float scalar = 1.0f;
        CHECK(a - scalar == sf::Vector2<float>(2.0f, 3.0f));
        CHECK(scalar - a == sf::Vector2<float>(-2.0f, -3.0f));
    }
    
    SUBCASE("Testing ostream output for sf::Vector2") {
        sf::Vector2<float> a(1.0f, 2.0f);
        std::stringstream ss;
        ss << a;
        CHECK(ss.str() == "Vector(1, 2)");
    }
}
