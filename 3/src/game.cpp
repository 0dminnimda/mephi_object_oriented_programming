#include "game.hpp"

#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <random>

#include "SFML/Graphics.hpp"
#include "vector_operations.hpp"

void center_text_origin(sf::Text &text) {
    sf::FloatRect text_rect = text.getLocalBounds();
    text.setOrigin(
        text_rect.left + text_rect.width / 2.0f, text_rect.top + text_rect.height / 2.0f
    );
}

void setup_sprite(const sf::Texture &texture, sf::Sprite &sprite, sf::Vector2f relative_scale = {1.0f, 1.0f}) {
    sprite.setTexture(texture);
    sprite.setScale(relative_scale / sf::Vector2f(texture.getSize()));
    sprite.setOrigin(sf::Vector2f(texture.getSize()) / 2.0f);
}

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

void Weapon::use(Actor &target) { attack(target.position); }

void Weapon::attack(sf::Vector2f position) {}

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

void InventoryCanvas::draw() {}

void LevelUpCanvas::draw() {}

Game &Game::get() {
    static std::shared_ptr<Game> game = nullptr;

    if (game == nullptr) {
        game = std::make_shared<Game>();
    }

    return *game;
}

static const char *const logo_name = "rock_eyebrow_meme.png";
static const char *const flor_tile_name = "dungeon_floor.jpeg";
static const char *const open_dor_tile_name = "dungeon_open_door.jpeg";
static const char *const closed_dor_tile_name = "dungeon_closed_door.jpeg";

bool Game::init(unsigned int width, unsigned int height) { return game_view.init(width, height); }

void Game::start_playing() {
    if (is_playing) return;

    is_playing = true;
    clock.restart();

    load_level(0);

    DungeonLevel *level = Game::get().get_current_level();
    if (!level) return;

    level->player.position = sf::Vector2f(level->tiles.size(), level->tiles.row_size()) / 2.0f;
}

void Game::handle_events() {
    sf::RenderWindow &window = game_view.window;
    sf::View &view = game_view.view;

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

        if (event.type == sf::Event::Resized) {
            view.setSize(sf::Vector2f(event.size.width, event.size.height));
            view.setCenter(sf::Vector2f(window.getSize()) / 2.0f);
            window.setView(view);
            // cannot draw in step sadly https://en.sfml-dev.org/forums/index.php?topic=5858.0
        }
    }
}

void Game::update(float delta_time) {
    DungeonLevel *level = get_current_level();
    if (level) {
        level->update(delta_time);
    }
}

bool Game::run() {
    while (game_view.is_open()) {
        handle_events();

        float delta_time = clock.restart().asSeconds();
        // AITimer.getElapsedTime() > sf::seconds(0.1f)

        update(delta_time);

        game_view.clear();
        game_view.draw();
        game_view.display();
    }

    return true;
}

bool GameView::init(unsigned int width, unsigned int height) {
    window.create(
        sf::VideoMode(width, height, 32), "Epic Rock Game",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    window.setVerticalSyncEnabled(true);

    view.setSize(sf::Vector2f(window.getSize()));
    view.setCenter(sf::Vector2f(window.getSize()) / 2.0f);
    window.setView(view);

    if (!dungeon_level_view.init()) return false;

    if (!logo_texture.loadFromFile(path_to_resources + logo_name)) return false;
    setup_sprite(logo_texture, logo, sf::Vector2f(window.getSize()) / 2.0f);
    logo.setPosition({window.getSize().x / 2.0f, window.getSize().y * 2.0f / 3.0f});

    if (!font.loadFromFile(path_to_resources + "tuffy.ttf")) return false;

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

    return true;
}

bool GameView::is_open() const { return window.isOpen(); }

void GameView::clear() { window.clear(sf::Color(50, 50, 50)); }

void GameView::display() { window.display(); }

void GameView::draw() {
    if (!Game::get().is_playing) {
        window.draw(logo);
        window.draw(menu_message);
        return;
    }

    DungeonLevel *level = Game::get().get_current_level();
    if (!level) {
        info_message.setFillColor(sf::Color::White);
        info_message.setString("No levels are loaded");
        center_text_origin(info_message);
        info_message.setPosition({window.getSize().x / 2.0f, window.getSize().y / 2.0f});
        window.draw(info_message);
        return;
    }

    dungeon_level_view.draw(*level);

    Player &player = level->player;

    float ratio = (float)window.getSize().x / (float)window.getSize().y;
    view.setSize(sf::Vector2f(10.0f * ratio, 10.0f));
    view.setCenter(sf::Vector2f(player.position));
    window.setView(view);
}

bool DungeonLevelView::init() {
    if (!flor_tile_texture.loadFromFile(path_to_resources + flor_tile_name)) return false;
    if (!open_dor_tile_texture.loadFromFile(path_to_resources + open_dor_tile_name)) return false;
    if (!closed_dor_tile_texture.loadFromFile(path_to_resources + closed_dor_tile_name))
        return false;

    setup_sprite(flor_tile_texture, flor_tile_sprite);
    flor_tile_sprite.setOrigin({0, 0});
    
    setup_sprite(open_dor_tile_texture, open_dor_tile_sprite);
    open_dor_tile_sprite.setOrigin({0, 0});

    setup_sprite(closed_dor_tile_texture, closed_dor_tile_sprite);
    closed_dor_tile_sprite.setOrigin({0, 0});

    return true;
}

void DungeonLevelView::draw(const DungeonLevel &level) {
    for (size_t i = 0; i < level.tiles.size(); ++i) {
        auto &row = level.tiles[i];
        for (size_t j = 0; j < row.size(); ++j) {
            draw_tile(row[j], sf::Vector2f(i, j));
        }
    }

    for (const auto &emeny : level.enemies) {
        actors_view.draw(emeny);
    }
    actors_view.draw(level.player);
}

void DungeonLevelView::draw_tile(const Tile &tile, sf::Vector2f position) {
    sf::Sprite sprite;
    if (tile.kind == Tile::Flor) {
        sprite = flor_tile_sprite;
    } else if (tile.kind == Tile::OpenDor) {
        sprite = open_dor_tile_sprite;
    } else if (tile.kind == Tile::ClosedDor) {
        sprite = closed_dor_tile_sprite;
    }

    sprite.setPosition(position);
    window.draw(sprite);
}

void ActorsView::draw(const Actor &actor) {
    setup_sprite(actor.texture, actor_sprite);
    actor_sprite.setPosition(actor.position);
    window.draw(actor_sprite);
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
    for (auto &enemy : enemies) {
        enemy.update(delta_time);
    }
    player.update(delta_time);
}

void Player::update(float delta_time) {
    auto resulting = sf::Vector2f(0, 0);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        resulting.y -= 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        resulting.y += 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        resulting.x -= 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        resulting.x += 1;
    }

    float len = length(resulting);
    if (len)
        resulting = resulting / len;

    position += resulting * (float)characteristics().speed * delta_time;
}

void Player::attack(Actor &target) {}

void Player::die(Actor &reason) {}

void Player::pick_up_item(Item &item) {}

void Enemy::update(float delta_time) {}

void Enemy::attack(Actor &target) {}

void Enemy::die(Actor &reason) {}
