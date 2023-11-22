#include "game.hpp"

#include <stdlib.h>

#include <SFML/System.hpp>
#include <algorithm>
#include <iostream>
#include <random>

#include "vector_operations.hpp"

void center_text_origin(sf::Text &text) {
    sf::FloatRect text_rect = text.getLocalBounds();
    text.setOrigin(
        text_rect.left + text_rect.width / 2.0f, text_rect.top + text_rect.height / 2.0f
    );
}

void setup_sprite(
    const sf::Texture &texture, sf::Sprite &sprite, sf::Vector2f relative_scale = {1.0f, 1.0f}
) {
    sprite.setTexture(texture);
    sprite.setScale(relative_scale / sf::Vector2f(texture.getSize()));
    sprite.setOrigin(sf::Vector2f(texture.getSize()) / 2.0f);
}

void CharacteristicsModifier::apply(Characteristics &value) {
    if (max_health) {
        value.max_health =
            std::visit([&](auto &&v) -> auto { return v.apply(value.max_health); }, *max_health);
    }

    if (defence) {
        value.defence =
            std::visit([&](auto &&v) -> auto { return v.apply(value.defence); }, *defence);
    }

    if (speed) {
        value.speed = std::visit([&](auto &&v) -> auto { return v.apply(value.speed); }, *speed);
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
static const char *const barrier_tile_name = "black.jpeg";

bool Game::init(unsigned int width, unsigned int height) {
    for (auto &cls : actor_classes) {
        if (!cls.init()) return false;
    }
    return game_view.init(width, height);
}

void Game::start_playing() {
    if (is_playing) return;

    is_playing = true;
    clock.restart();

    load_level(0);

    DungeonLevel *level = Game::get().get_current_level();
    if (!level) return;

    Game::get().player.position = level->initial_player_position;
}

void Game::stop_playing() {
    if (!is_playing) return;

    is_playing = false;

    unload_current_level();
}

bool is_pressed(const sf::Event &event, sf::Keyboard::Key key) {
    return (event.type == sf::Event::KeyPressed) && (event.key.code == key);
}

void Game::handle_events() {
    sf::RenderWindow &window = game_view.window;
    sf::View &view = game_view.view;

    sf::Event event;
    while (window.pollEvent(event)) {
        if ((event.type == sf::Event::Closed) ||
            (!is_playing && is_pressed(event, sf::Keyboard::Escape)))
        {
            window.close();
            break;
        }

        if (is_playing && is_pressed(event, sf::Keyboard::Escape)) {
            stop_playing();
        }

        if (is_pressed(event, sf::Keyboard::Space) || (event.type == sf::Event::TouchBegan)) {
            start_playing();
        }

        if (event.type == sf::Event::Resized) {
            float ratio = (float)event.size.width / (float)event.size.height;
            view.setSize(sf::Vector2f(Game::virtual_size * ratio, Game::virtual_size));
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

size_t Game::add_actor_class(const ActorClass &cls) {
    actor_classes.push_back(cls);
    return actor_classes.size() - 1;
}

size_t Game::add_item_template(std::unique_ptr<Item> item) {
    item_templates.push_back(std::move(item));
    return item_templates.size() - 1;
}

bool GameView::init(unsigned int width, unsigned int height) {
    window.create(
        sf::VideoMode(width, height, 32), "Epic Lab3 Game",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    window.setVerticalSyncEnabled(true);

    float ratio = (float)window.getSize().x / (float)window.getSize().y;
    view.setSize(sf::Vector2f(Game::virtual_size * ratio, Game::virtual_size));
    view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
    window.setView(view);

    if (!dungeon_level_view.init()) return false;

    if (!logo_texture.loadFromFile(path_to_resources + logo_name)) return false;
    setup_sprite(logo_texture, logo, sf::Vector2f(view.getSize()) / 2.0f);
    logo.setPosition({view.getSize().x / 2.0f, view.getSize().y * 2.0f / 3.0f});

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
    menu_message.setPosition({view.getSize().x / 2.0f, view.getSize().y / 6.0f});
    menu_message.setScale(
        sf::Vector2f(Game::virtual_size, Game::virtual_size) / (float)std::min(width, height)
    );

    info_message.setFont(font);
    info_message.setCharacterSize(40);
    info_message.setPosition(sf::Vector2f(view.getSize()) / 2.0f);
    info_message.setScale(
        sf::Vector2f(Game::virtual_size, Game::virtual_size) / (float)std::min(width, height)
    );

    return true;
}

bool GameView::is_open() const { return window.isOpen(); }

void GameView::clear() { window.clear(sf::Color(50, 50, 50)); }

void GameView::display() { window.display(); }

void GameView::draw() {
    if (!Game::get().is_playing) {
        window.draw(logo);
        window.draw(menu_message);

        view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
        window.setView(view);
        return;
    }

    DungeonLevel *level = Game::get().get_current_level();
    if (!level) {
        info_message.setFillColor(sf::Color::White);
        info_message.setString("No levels are loaded");
        center_text_origin(info_message);
        window.draw(info_message);

        view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
        window.setView(view);
        return;
    }

    dungeon_level_view.draw(*level);

    float ratio = (float)window.getSize().x / (float)window.getSize().y;
    view.setSize(sf::Vector2f(Game::virtual_size * ratio, Game::virtual_size));
    view.setCenter(sf::Vector2f(Game::get().player.position));
    window.setView(view);
}

void DungeonLevel::init() {
    for (auto &emeny : enemies) {
        emeny.init();
    }
}

sf::Vector2f DungeonLevel::center() const {
    return sf::Vector2f(tiles.size(), tiles.row_size()) / 2.0f;
}

void DungeonLevel::resize_tiles(size_t width, size_t height) { tiles.resize(width, height); }

void DungeonLevel::regenerate_tiles() {
    for (size_t i = 0; i < tiles.size(); ++i) {
        for (size_t j = 0; j < tiles.row_size(); ++j) {
            if (i == 0 || i == tiles.size() - 1 || j == 0 || j == tiles.row_size() - 1)
                tiles[i][j].kind = Tile::Barrier;
            else
                tiles[i][j].kind = Tile::Flor;
        }
    }

    tiles[4][4].kind = Tile::OpenDor;
    tiles[4][5].kind = Tile::ClosedDor;
}

void DungeonLevel::regenerate_enemies() {
    enemies.clear();

    RangeOfValues range_x(0, tiles.size());
    RangeOfValues range_y(0, tiles.row_size());

    for (size_t class_index = 1; class_index < Game::get().actor_classes.size(); ++class_index) {
        for (size_t i = 0; i < 10; ++i) {
            Enemy &enemy = enemies.emplace_back(Game::get().make_enemy(class_index));
            enemy.position = sf::Vector2f(range_x.get_random(), range_y.get_random());
        }
    }
}

bool DungeonLevelView::init() {
    if (!flor_tile_texture.loadFromFile(path_to_resources + flor_tile_name)) return false;
    if (!open_dor_tile_texture.loadFromFile(path_to_resources + open_dor_tile_name)) return false;
    if (!closed_dor_tile_texture.loadFromFile(path_to_resources + closed_dor_tile_name))
        return false;
    if (!barrier_tile_texture.loadFromFile(path_to_resources + barrier_tile_name)) return false;

    setup_sprite(flor_tile_texture, flor_tile_sprite);
    flor_tile_sprite.setOrigin({0, 0});

    setup_sprite(open_dor_tile_texture, open_dor_tile_sprite);
    open_dor_tile_sprite.setOrigin({0, 0});

    setup_sprite(closed_dor_tile_texture, closed_dor_tile_sprite);
    closed_dor_tile_sprite.setOrigin({0, 0});

    setup_sprite(barrier_tile_texture, barrier_tile_sprite);
    barrier_tile_sprite.setOrigin({0, 0});

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
    actors_view.draw(Game::get().player);
}

void DungeonLevelView::draw_tile(const Tile &tile, sf::Vector2f position) {
    sf::Sprite sprite;
    if (tile.kind == Tile::Flor) {
        sprite = flor_tile_sprite;
    } else if (tile.kind == Tile::OpenDor) {
        sprite = open_dor_tile_sprite;
    } else if (tile.kind == Tile::ClosedDor) {
        sprite = closed_dor_tile_sprite;
    } else if (tile.kind == Tile::Barrier) {
        sprite = barrier_tile_sprite;
    }

    sprite.setPosition(position);
    window.draw(sprite);
}

void ActorsView::draw(const Actor &actor) {
    sf::Sprite &sprite = Game::get().actor_classes[actor.actor_class_index()].sprite;
    sprite.setPosition(actor.position);
    window.draw(sprite);
}

void Game::add_level(const DungeonLevel &level) { all_levels.push_back(level); }

bool Game::load_level(size_t index) {
    if (index < 0 || index >= all_levels.size()) {
        return false;
    }
    current_level = all_levels[index];
    return true;
}

void Game::unload_current_level() { current_level = std::nullopt; }

Enemy Game::make_enemy(size_t actor_class_index) const {
    return enemy_templates.at(actor_class_index).copy();
}

DungeonLevel *Game::get_current_level() {
    if (!current_level) {
        return nullptr;
    }
    return &(*current_level);  // some funky C++ with it's * for std::optional
}

void DungeonLevel::handle_collitions() {}

void DungeonLevel::update(float delta_time) {
    for (auto &enemy : enemies) {
        enemy.update(delta_time);
    }
    Game::get().player.update(delta_time);

    handle_collitions();
}

bool ActorClass::init() {
    if (!texture.loadFromFile(path_to_resources + texture_name)) return false;
    setup_sprite(texture, sprite);
    return true;
}

void Equipment::equip_weapon(std::shared_ptr<Weapon> weapon) { this->weapon = weapon; }

void Player::init() {}

void Player::update(float delta_time) {
    auto resulting = sf::Vector2f(0, 0);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        resulting.y -= 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        resulting.y += 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        resulting.x -= 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        resulting.x += 1;
    }

    position += normalized(resulting) * (float)characteristics().speed * delta_time;
}

void Player::attack(Actor &target) {}

void Player::die(Actor &reason) {}

void Player::pick_up_item(Item &item) {}

void Enemy::init() {}

void Enemy::update(float delta_time) {
    sf::Vector2f direction = Game::get().player.position - position;
    position += normalized(direction) * (float)characteristics().speed * delta_time;
}

void Enemy::attack(Actor &target) {}

void Enemy::die(Actor &reason) {}

Enemy Enemy::copy() const { return Enemy(*this); }

void Hammer::attack(sf::Vector2f position) {}

float Hammer::get_damage(Actor &target) { return damage_range.get_random(); }

std::shared_ptr<Item> Hammer::copy() const {
    return std::make_shared<Hammer>(*this);
}
