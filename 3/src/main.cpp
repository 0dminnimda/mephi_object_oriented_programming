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

int sub_main() {
    Game &game = Game::get();

    // game.save_config("template_config.txt");
    if (!game.load_config("config.toml")) {
        game.setup_default_actors();
        game.setup_default_items();

        {
            DungeonLevel level;
            level.resize_tiles(30, 30);
            level.regenerate();
            game.dungeon.add_level(level);
        }
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
