#include "game.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "vector_operations.hpp"


void CharacteristicsModifier::apply(Characteristics &value) {
    if (max_health) {
        value.max_health = std::visit(
            [&](auto &&v) -> auto { return (long)v.apply(value.max_health); }, *max_health
        );
    }

    if (defence) {
        value.defence =
            std::visit([&](auto &&v) -> auto { return (long)v.apply(value.defence); }, *defence);
    }

    if (speed) {
        value.speed =
            std::visit([&](auto &&v) -> auto { return (long)v.apply(value.speed); }, *speed);
    }
}

void Potion::apply(Actor &target) { modifier.apply(target.characteristics()); }

void Potion::use(Actor &target) { apply(target); }

void Weapon::use(Actor &target) {
    attack(target.position());
}

void Weapon::attack(sf::Vector2f position) {
    
}

Tile &Tile::set_building(std::unique_ptr<Chest> building) {
    this->building = building.get();
    return *this;
}

long RangeOfValues::get_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> dis(min, max);
    return dis(gen);
}

void InventoryCanvas::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    
}

void LevelUpCanvas::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    
}

#ifdef SFML_SYSTEM_IOS
    static const std::string path_to_resources = "";
#else
    static const std::string path_to_resources = "resources/";
#endif

static const char * const logo_name = "rock_eyebrow_meme.png";
// static const char * const flor_tile_name = "dungeon_floor.jpeg";
// static const char * const open_dor_tile_name = "dungeon_open_door.jpeg";
static const char * const flor_tile_name = "rock_eyebrow_meme.png";
static const char * const open_dor_tile_name = "hide_the_plan.jpeg";
static const char * const closed_dor_tile_name = "dungeon_closed_door.jpeg";

void center_text_origin(sf::Text &text) {
    // const sf::String string = text.getString();

    // float max_height = 0;
    // const sf::Font &font = *text.getFont();
    // for (size_t characterIndex = 0; characterIndex < string.getSize(); ++characterIndex)
    // {
    //     auto bounds = font.getGlyph(string[characterIndex], text.getCharacterSize(), false).bounds;
    //     max_height = std::max(max_height, bounds.height);
    // }

    sf::FloatRect text_rect = text.getLocalBounds();
    text.setOrigin(text_rect.left + text_rect.width / 2.0f, text_rect.top  + text_rect.height / 2.0f);
}

int Game::init(unsigned int width, unsigned int height) {
    window.create(
        sf::VideoMode(width, height, 32), "Epic Rock Game",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize | sf::Style::Fullscreen
    );
    window.setVerticalSyncEnabled(true);

    if (!flor_tile_texture.loadFromFile(path_to_resources + flor_tile_name)) return EXIT_FAILURE;
    if (!open_dor_tile_texture.loadFromFile(path_to_resources + open_dor_tile_name)) return EXIT_FAILURE;
    if (!closed_dor_tile_texture.loadFromFile(path_to_resources + closed_dor_tile_name)) return EXIT_FAILURE;

    if (!logo_texture.loadFromFile(path_to_resources + logo_name)) return EXIT_FAILURE;
    logo.setTexture(logo_texture);
    float scale = min(sf::Vector2f(window.getSize()) / sf::Vector2f(logo_texture.getSize()) / 2.0f);
    logo.setScale({scale, scale});
    logo.setOrigin(sf::Vector2f(logo_texture.getSize()) / 2.0f);
    logo.setPosition({window.getSize().x / 2.0f, window.getSize().y * 2.0f / 3.0f});

    if (!font.loadFromFile(path_to_resources + "tuffy.ttf")) return EXIT_FAILURE;

    menu_message.setFont(font);
    menu_message.setCharacterSize(40);
    menu_message.setFillColor(sf::Color::White);
#ifdef SFML_SYSTEM_IOS
    menu_message.setString("Welcome to Epic Lab3 Game!\nTouch the screen to start the game.");
#else
    menu_message.setString("Welcome to Epic Lab3 Game!\n\nPress space to start the game.");
#endif
    center_text_origin(menu_message);
    menu_message.setPosition({window.getSize().x / 2.0f, window.getSize().y / 6.0f});

    info_message.setFont(font);
    info_message.setCharacterSize(40);

    return EXIT_SUCCESS;
}

void Game::start_playing() {
    if (!is_playing) {
        is_playing = true;
        clock.restart();

        load_level(0);
        // Reset the position of the paddles and ball
        // leftPaddle.setPosition(10.f + paddleSize.x / 2.f, gameHeight / 2.f);
        // rightPaddle.setPosition(gameWidth - 10.f - paddleSize.x / 2.f, gameHeight / 2.f);
        // ball.setPosition(gameWidth / 2.f, gameHeight / 2.f);
    }
}

void Game::handle_events() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if ((event.type == sf::Event::Closed) ||
            ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
        {
            window.close();
            break;
        }

        if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) ||
            (event.type == sf::Event::TouchBegan))
        {
            start_playing();
        }

        // if (event.type == sf::Event::Resized) {
        //     sf::View view;
        //     view.setSize(gameWidth, gameHeight);
        //     view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
        //     window.setView(view);
        // }
    }
}

void Game::update(float delta_time) {
    DungeonLevel *level = get_current_level();
    if (level) {
        level->update(delta_time);
    }
}

int Game::run() {
    while (window.isOpen()) {
        handle_events();

        float delta_time = clock.restart().asSeconds();
        // AITimer.getElapsedTime() > sf::seconds(0.1f)

        update(delta_time);

        window.clear(sf::Color(50, 50, 50));
        draw(window);
        window.display();
    }

    return EXIT_SUCCESS;
}

// void DungeonLevel::draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) {
    
// }

void Game::draw(sf::RenderTarget& target, sf::RenderStates states) {
    if (!is_playing) {
        target.draw(logo, states);
        target.draw(menu_message, states);
        return;
    }

    DungeonLevel *level_p = get_current_level();
    if (!level_p) {
        info_message.setFillColor(sf::Color::White);
        info_message.setString("No levels are loaded");
        center_text_origin(info_message);
        info_message.setPosition({window.getSize().x / 2.0f, window.getSize().y / 2.0f});
        target.draw(info_message, states);
        return;
    }
    DungeonLevel &level = *level_p;

    float window_size = min(window.getSize());
    float size = std::max(level.tiles.row_size(), level.tiles.size());

    for (size_t i = 0; i < level.tiles.size(); ++i) {
        auto &row = level.tiles[i];
        for (size_t j = 0; j < row.size(); ++j) {
            sf::Vector2f position = sf::Vector2f(i, j) / size * window_size;

            if (row[j].kind == Tile::Flor) {
                tile.setTexture(flor_tile_texture);
            } else if (row[j].kind == Tile::OpenDor) {
                tile.setTexture(open_dor_tile_texture);
            } else if (row[j].kind == Tile::ClosedDor) {
                tile.setTexture(closed_dor_tile_texture);
            }

            float scale = min(sf::Vector2f(window.getSize()) / sf::Vector2f(tile.getTexture()->getSize()) / size);
            tile.setScale({scale, scale});
            tile.setOrigin({0, 0});
            tile.setPosition(position);
            target.draw(tile, states);
        }
    }
}

void Game::add_level(const DungeonLevel &level) { all_levels.push_back(level); }

void Game::load_level(size_t index) { current_level_index = index; }

void Game::unload_current_level() { current_level_index = -1; }

DungeonLevel *Game::get_current_level() {
    if (current_level_index < 0 || current_level_index >= all_levels.size()) {
        return nullptr;
    }
    return &all_levels[current_level_index];
}

void DungeonLevel::update(float delta_time) {
    // for (auto &enemy : enemies) {
    //     enemy.update(delta_time);
    // }
    // player.update(delta_time);
}
