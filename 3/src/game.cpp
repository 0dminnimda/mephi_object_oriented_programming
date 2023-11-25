#include "game.hpp"

#include <stdlib.h>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <random>

#include "vector_operations.hpp"

void debug_draw_point(const sf::Vector2f &point) {
    sf::CircleShape circle(0.05f);
    circle.setPosition(point);
    circle.setFillColor(sf::Color::Red);
    Game::get().game_view.window.draw(circle);
}

void debug_draw_permanent_point(const sf::Vector2f &point) {
#ifdef DEBUG
    Game::get().game_view.debug_points.push_back(point);
#endif  // DEBUG
}

sf::Color set_transparency(sf::Color color, sf::Uint8 transparency) {
    color.a = transparency;
    return color;
}

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

Tile &Tile::set_building(std::shared_ptr<Chest> building) {
    this->building = building;
    return *this;
}

long RangeOfValues::get_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> dis(min, max);
    return dis(gen);
}

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
static const char *const chest_name = "chest.png";

bool Game::init(unsigned int width, unsigned int height) {
    for (auto &it : actor_classes) {
        if (!it.init()) return false;
    }
    for (auto &it : item_classes) {
        if (!it.init()) return false;
    }
    dungeon.init();
    return game_view.init(width, height);
}

void Game::start_playing() {
    if (is_playing) return;

    is_playing = true;
    clock.restart();

    dungeon.load_level(0);
}

void Game::stop_playing() {
    if (!is_playing) return;

    is_playing = false;

    dungeon.unload_current_level();
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

        if (is_pressed(event, sf::Keyboard::Enter) || (event.type == sf::Event::TouchBegan)) {
            start_playing();
        }

        if (event.type == sf::Event::Resized) {
            float ratio = (float)event.size.width / (float)event.size.height;
            view.setSize(sf::Vector2f(Game::view_size * ratio, Game::view_size));
            window.setView(view);
            // cannot draw in step sadly https://en.sfml-dev.org/forums/index.php?topic=5858.0
        }
    }
}

void Game::update(float delta_time) {
    if (!dungeon.player.alive) return;

    DungeonLevel *level = dungeon.get_current_level();
    if (level) {
        level->update(delta_time);
    }
}

bool Game::run() {
    while (game_view.is_open()) {
        handle_events();

        float delta_time = clock.restart().asSeconds();
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

size_t Game::add_item_class(const ItemClass &cls) {
    item_classes.push_back(cls);
    return item_classes.size() - 1;
}

Enemy Game::make_enemy(size_t actor_class_index) const {
    return deepcopy(enemy_templates.at(actor_class_index));
}

std::shared_ptr<Item> Game::make_item(size_t item_class_index) const {
    return item_templates.at(item_class_index)->deepcopy_item();
}

bool GameView::init(unsigned int width, unsigned int height) {
    window.create(
        sf::VideoMode(width, height, 32), "Epic Lab3 Game",
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    window.setVerticalSyncEnabled(true);

    float ratio = (float)window.getSize().x / (float)window.getSize().y;
    view.setSize(sf::Vector2f(Game::view_size * ratio, Game::view_size));
    view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
    window.setView(view);

    if (!dungeon_level_view.init()) return false;

    if (!logo_texture.loadFromFile(path_to_resources + logo_name)) return false;
    float scale = max(sf::Vector2f(view.getSize())) / 3.0f;
    setup_sprite(logo_texture, logo, {scale, scale});
    logo.setPosition({view.getSize().x / 2.0f, view.getSize().y * 2.0f / 3.0f});

    if (!font.loadFromFile(path_to_resources + "tuffy.ttf")) return false;

    menu_message.setFont(font);
    menu_message.setCharacterSize(40);
    menu_message.setFillColor(sf::Color::White);
#ifdef SFML_SYSTEM_IOS
    menu_message.setString("Welcome to Epic Lab3 Game!\nTouch the screen to start the game.");
#else
    menu_message.setString("Welcome to Epic Lab3 Game!\n\nPress Enter to start the game.");
#endif
    center_text_origin(menu_message);
    menu_message.setPosition({view.getSize().x / 2.0f, view.getSize().y / 6.0f});
    menu_message.setScale(
        sf::Vector2f(Game::view_size, Game::view_size) / (float)std::min(width, height)
    );

    info_message.setFont(font);
    info_message.setCharacterSize(40);
    info_message.setFillColor(sf::Color::White);
    info_message.setPosition(sf::Vector2f(view.getSize()) / 2.0f);
    info_message.setScale(
        sf::Vector2f(Game::view_size, Game::view_size) / (float)std::min(width, height)
    );

    death_message.setFont(font);
    death_message.setCharacterSize(60);
    death_message.setFillColor(sf::Color::Red);
    death_message.setOutlineThickness(5);
    death_message.setOutlineColor(sf::Color::Black);
    death_message.setPosition(sf::Vector2f(view.getSize()) / 2.0f);
    death_message.setScale(
        sf::Vector2f(Game::view_size, Game::view_size) / (float)std::min(width, height)
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

    DungeonLevel *level = Game::get().dungeon.get_current_level();
    if (!level) {
        info_message.setString("No levels are loaded");
        center_text_origin(info_message);
        window.draw(info_message);

        view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
        window.setView(view);
        return;
    }

    dungeon_level_view.draw(*level);

    if (!Game::get().dungeon.player.alive) {
        death_message.setString("YOU DIED");
        center_text_origin(death_message);
        death_message.setPosition(sf::Vector2f(Game::get().dungeon.player.position));
        window.draw(death_message);
    }

#ifdef DEBUG
    for (auto &point : debug_points) {
        debug_draw_point(point);
    }
#endif  // DEBUG

    float ratio = (float)window.getSize().x / (float)window.getSize().y;
    view.setSize(sf::Vector2f(Game::view_size * ratio, Game::view_size));
    view.setCenter(sf::Vector2f(Game::get().dungeon.player.position));
    window.setView(view);
}

void LockPicks::deepcopy_to(LockPicks &other) const {
    Item::deepcopy_to(other);
    other.count = count;
}

std::shared_ptr<Item> LockPicks::deepcopy_item() const { return deepcopy_shared(*this); }

void Inventory::add_item(std::shared_ptr<Item> item) { items.push_back(item); }

void InventoryCanvas::draw() {}

void LevelUpCanvas::draw() {}

void Dungeon::init() {
    for (auto &level : all_levels) {
        level.init();
    }
    Game::get().dungeon.player.init();
}

void DungeonLevel::init() {
    for (auto &emeny : enemies) {
        emeny.init();
    }
}

float DungeonLevel::tile_coords_to_world_coords_factor() const {
    return tile_size / Game::world_size;
}

sf::Vector2f DungeonLevel::center() const {
    return sf::Vector2f(tiles.size(), tiles.row_size()) * tile_coords_to_world_coords_factor() /
           2.0f;
}

std::optional<std::pair<size_t, size_t>> DungeonLevel::get_tile_coordinates(
    const sf::Vector2f &position
) const {
    sf::Vector2f world_coords = position / tile_coords_to_world_coords_factor();
    if (position.x < 0 || position.x >= tiles.size() || position.y < 0 ||
        position.y >= tiles.row_size())
        return std::nullopt;
    return std::make_pair(position.x, position.y);
}

void DungeonLevel::resize_tiles(size_t width, size_t height) { tiles.resize(width, height); }

void DungeonLevel::regenerate_tiles() {
    RangeOfValues range_chest_spawn(0, 50);
    size_t max_level = 9;
    RangeOfValues range_chest_level(0, (max_level + 1) * (max_level + 1) - 1);
    RangeOfValues range_chest_item(0, Game::get().item_templates.size() - 1);

    for (size_t i = 0; i < tiles.size(); ++i) {
        for (size_t j = 0; j < tiles.row_size(); ++j) {
            if (i == 0 || i == tiles.size() - 1 || j == 0 || j == tiles.row_size() - 1)
                tiles[i][j].kind = Tile::Barrier;
            else
                tiles[i][j].kind = Tile::Flor;

            if (tiles[i][j].kind == Tile::Flor && range_chest_spawn.get_random() == 0) {
                size_t level = max_level - (size_t)std::sqrt(range_chest_level.get_random());
                auto chest = std::make_shared<Chest>(level);
                chest->inventory.add_item(Game::get().make_item(range_chest_item.get_random()));
                tiles[i][j].set_building(chest);
            }
        }
    }

    tiles[4][4].kind = Tile::OpenDor;
    tiles[4][5].kind = Tile::ClosedDor;
}

void DungeonLevel::regenerate_enemies() {
    enemies.clear();

    RangeOfValues range_x(0, tiles.size());
    RangeOfValues range_y(0, tiles.row_size());
    RangeOfValues range_wiggle(-10, 10);

    for (size_t class_index = 1; class_index < Game::get().actor_classes.size(); ++class_index) {
        for (size_t i = 0; i < 10; ++i) {
            Enemy &enemy = enemies.emplace_back(Game::get().make_enemy(class_index));
            enemy.position = sf::Vector2f(
                range_x.get_random() + (float)range_wiggle.get_random() / 10,
                range_y.get_random() + (float)range_wiggle.get_random() / 10
            );
            enemy.position *= tile_coords_to_world_coords_factor();
        }
    }
}

bool DungeonLevelView::init() {
    if (!flor_tile_texture.loadFromFile(path_to_resources + flor_tile_name)) return false;
    if (!open_dor_tile_texture.loadFromFile(path_to_resources + open_dor_tile_name)) return false;
    if (!closed_dor_tile_texture.loadFromFile(path_to_resources + closed_dor_tile_name))
        return false;
    if (!barrier_tile_texture.loadFromFile(path_to_resources + barrier_tile_name)) return false;
    if (!chest_texture.loadFromFile(path_to_resources + chest_name)) return false;

    setup_sprite(flor_tile_texture, flor_tile_sprite);
    flor_tile_sprite.setOrigin({0, 0});

    setup_sprite(open_dor_tile_texture, open_dor_tile_sprite);
    open_dor_tile_sprite.setOrigin({0, 0});

    setup_sprite(closed_dor_tile_texture, closed_dor_tile_sprite);
    closed_dor_tile_sprite.setOrigin({0, 0});

    setup_sprite(barrier_tile_texture, barrier_tile_sprite);
    barrier_tile_sprite.setOrigin({0, 0});

    setup_sprite(chest_texture, chest_sprite);
    chest_sprite.setOrigin({0, 0});

    return true;
}

void DungeonLevelView::draw(const DungeonLevel &level) {
    for (size_t i = 0; i < level.tiles.size(); ++i) {
        auto &row = level.tiles[i];
        for (size_t j = 0; j < row.size(); ++j) {
            draw_tile(
                row[j], sf::Vector2f(i, j), level.tile_coords_to_world_coords_factor(),
                level.chest_size_factor
            );
        }
    }

    for (const LayingItem &laying_item : level.laying_items) {
        actors_view.items_view.draw(*(laying_item.item), laying_item.position);
    }

    for (const auto &emeny : level.enemies) {
        actors_view.draw(emeny);
    }
    actors_view.draw(Game::get().dungeon.player);
}

void DungeonLevelView::draw_tile(
    const Tile &tile, sf::Vector2f position, float factor, float chest_size_factor
) {
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

    sf::Vector2f saved = sprite.getScale();
    sprite.setScale(saved * factor);
    sprite.setPosition(position * factor);
    window.draw(sprite);
    sprite.setScale(saved);

    if (tile.building) {
        saved = chest_sprite.getScale();
        chest_sprite.setScale(saved * factor * chest_size_factor);
        chest_sprite.setPosition(position * factor);
        window.draw(chest_sprite);
        chest_sprite.setScale(saved);
    }
}

void ActorsView::draw(const Actor &actor) {
    sf::Sprite &sprite = Game::get().actor_classes[actor.actor_class_index].sprite;
    sf::Vector2f saved = sprite.getScale();
    sprite.setScale(saved * actor.size / Game::world_size);
    sprite.setPosition(actor.position);
    window.draw(sprite);
    sprite.setScale(saved);

    if (actor.equipment.weapon) items_view.draw(*actor.equipment.weapon, actor.position);

    float bar_width = actor.size / Game::world_size * 1.2f;
    max_health_bar.setSize(sf::Vector2f(1.0f, health_bar_height_factor) * bar_width);
    max_health_bar.setPosition(actor.position - sf::Vector2f(bar_width, bar_width) / 2.0f);
    max_health_bar.setFillColor(sf::Color(255, 255, 255, 127));
    window.draw(max_health_bar);

    float health_ratio = std::max(0.0f, actor.health) / actor.characteristics.max_health;

    current_health_bar.setSize({max_health_bar.getSize().x * health_ratio, max_health_bar.getSize().y});
    current_health_bar.setPosition(max_health_bar.getPosition());
    current_health_bar.setFillColor(set_transparency(sf::Color::Green, 127));
    window.draw(current_health_bar);
}

void ItemsView::draw(const Item &item, sf::Vector2f position) {
    auto &cls = Game::get().item_classes[item.item_class_index];
    sf::Sprite &sprite = cls.sprite;
    sf::Vector2f saved = sprite.getScale();
    sprite.setScale(saved / Game::world_size * cls.size);
    sprite.setPosition(position);
    window.draw(sprite);
    sprite.setScale(saved);
}

void Dungeon::add_level(const DungeonLevel &level) { all_levels.push_back(level); }

bool Dungeon::load_level(size_t index) {
    if (index < 0 || index >= all_levels.size()) {
        return false;
    }
    current_level = all_levels[index];
    on_load_level(*current_level);
    return true;
}

void Dungeon::on_load_level(DungeonLevel &level) {
    Game::get().player_template.deepcopy_to(player);
    player.position = level.initial_player_position;
}

void Dungeon::unload_current_level() { current_level = std::nullopt; }

DungeonLevel *Dungeon::get_current_level() {
    if (!current_level) {
        return nullptr;
    }
    return &(*current_level);  // some funky C++ with it's * for std::optional
}

void DungeonLevel::handle_collitions() {}

void DungeonLevel::delete_the_dead() {
    size_t c = 0;
    size_t i = 0;
    for (; i < enemies.size(); ++i) {
        if (enemies[c].alive) {
            ++c;
        } else if (enemies[i].alive) {
            std::swap(enemies[c], enemies[i]);
            ++c;
        }
    }
    enemies.erase(std::next(enemies.begin(), c), enemies.end());
}

void DungeonLevel::update(float delta_time) {
    for (auto &enemy : enemies) {
        enemy.update(delta_time);
    }
    Game::get().dungeon.player.update(delta_time);

    delete_the_dead();
    handle_collitions();
}

bool ActorClass::init() {
    if (!texture.loadFromFile(path_to_resources + texture_name)) return false;
    setup_sprite(texture, sprite);
    return true;
}

bool ItemClass::init() {
    if (!texture.loadFromFile(path_to_resources + texture_name)) return false;
    setup_sprite(texture, sprite);
    return true;
}

LockPickingResult Chest::simulate_picking(const Actor &source) {
    RangeOfValues range(0, 1 + 2 * level);
    if ((float)range.get_random() / source.characteristics.luck <= 1) {
        return {true, false};
    }
    return {false, (float)range.get_random() * source.characteristics.luck <= 1};
}

void Chest::try_to_pick(const Actor &source, LockPicks &picks, size_t i, size_t j) {
    if (picks.count == 0) return;

    auto result = simulate_picking(source);
    std::cout << "lock_picked = " << result.lock_picked << " pick_broken = " << result.pick_broken
              << std::endl;
    if (result.pick_broken) {
        picks.count -= 1;
    }

    DungeonLevel *level = Game::get().dungeon.get_current_level();
    if (!level) return;

    auto tile_position = sf::Vector2f(i, j) * level->tile_coords_to_world_coords_factor();
    if (result.lock_picked) {
        for (std::shared_ptr<Item> item : inventory.items) {
            LayingItem laying_item(item);
            laying_item.position = tile_position;
            level->laying_items.push_back(laying_item);
        }
        level->tiles[i][j].building = nullptr;
    }
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

float Enchantment::apply(float value, const Actor &target) const {
    if (target.actor_class_index == target_actor_class_index) {
        return value * damage_multiplier;
    }
    return value;
}

void Equipment::equip_weapon(std::shared_ptr<Item> weapon) { this->weapon = weapon; }

void Equipment::deepcopy_to(Equipment &other) const {
    other.wearable = wearable;
    if (weapon)
        other.weapon = weapon->deepcopy_item();
    else
        other.weapon = nullptr;
}

void Actor::take_damage(float amount, Actor &source) {
    health -= std::max(0.0f, amount - characteristics.defence);
    if (health <= 0.0f) {
        health = 0.0f;
        die(source);
    }
}

void Experience::gain(size_t amount, Actor &actor) {
    value += amount;
    for (;;) {
        size_t needs = needs_exp_for_level(level);
        if (needs <= value) {
            value -= needs;
            level_up();
        } else {
            break;
        }
    }
}

size_t Experience::needs_exp_for_level(size_t level) { return 4 * level * level + 10 * level + 10; }

void Experience::level_up() { level += 1; }

void RigidBody::deepcopy_to(RigidBody &other) const {
    other.position = position;
    other.size = size;
}

void Actor::deepcopy_to(Actor &other) const {
    RigidBody::deepcopy_to(other);
    other.actor_class_index = actor_class_index;
    other.health = health;
    equipment.deepcopy_to(other.equipment);
    other.characteristics = characteristics;
    other.alive = alive;
}

void Player::init() {}

void Player::deepcopy_to(Player &other) const {
    Actor::deepcopy_to(other);
    other.inventory = inventory;
    other.lock_picks = lock_picks;
    other.experience = experience;
}

void Player::update(float delta_time) {
    handle_movement(delta_time);
    handle_equipment_use();
    handle_picking();
}

void Player::handle_movement(float delta_time) {
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

    position += normalized(resulting) * (float)characteristics.speed * delta_time;
}

void Player::handle_equipment_use() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        if (equipment.weapon) {
            equipment.weapon->use(*this);
        }
    }
}

void Player::handle_picking() {
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) return;

    DungeonLevel *level = Game::get().dungeon.get_current_level();
    if (!level) return;

    auto coords = level->get_tile_coordinates(position);
    if (!coords) return;

    Tile &tile = level->tiles[coords->first][coords->second];
    if (!tile.building) return;

    tile.building->try_to_pick(*this, lock_picks, coords->first, coords->second);
}

void Player::die(Actor &reason) { alive = false; }

void Player::pick_up_item(Item &item) {}

void Enemy::init() {}

void Enemy::update(float delta_time) {
    handle_movement(delta_time);
    handle_equipment_use();
}

void Enemy::handle_movement(float delta_time) {
    sf::Vector2f direction = Game::get().dungeon.player.position - position;
    position += normalized(direction) * (float)characteristics.speed * delta_time;
}

void Enemy::handle_equipment_use() {
    if (equipment.weapon) {
        equipment.weapon->use(*this);
    }
}

void Enemy::die(Actor &reason) {
    std::cout << "Enemy (" << actor_class_index << ") died from (" << reason.actor_class_index
              << ") with " << health << " health" << std::endl;
    alive = false;

    DungeonLevel *level = Game::get().dungeon.get_current_level();
    if (!level) return;

    RangeOfValues range_chest_item(0, Game::get().item_templates.size() - 1);
    std::shared_ptr<Item> item = Game::get().make_item(range_chest_item.get_random());

    LayingItem laying_item(item);
    laying_item.position = position;
    level->laying_items.push_back(laying_item);
}
void Enemy::deepcopy_to(Enemy &other) const { Actor::deepcopy_to(other); }

void Item::deepcopy_to(Item &other) const { other.item_class_index = item_class_index; }

void Potion::apply(Actor &target) { modifier.apply(target.characteristics); }

void Potion::use(Actor &target) { apply(target); }

void Weapon::deepcopy_to(Weapon &other) const {
    Item::deepcopy_to(other);
    other.artefact = artefact;
    other.enchantment = enchantment;
    other.damage_range = damage_range;
}

void Weapon::use(Actor &target) { attack_by(target); }

float Weapon::get_damage(Actor &target) { return damage_range.get_random(); }

bool Hammer::cooldown() {
    if (!on_cooldown) {
        cooldown_timer.restart();
        on_cooldown = true;
        return false;
    }

    if (cooldown_timer.getElapsedTime() > cooldown_time) {
        cooldown_timer.restart();
        return false;
    }

    return true;
}

void Hammer::attack_by(Actor &source) {
    if (cooldown()) return;

    if (source.actor_class_index == Game::player_class_index) {
        for (auto &enemy : Game::get().dungeon.get_current_level()->enemies) {
            try_to_attack(source, enemy);
        }
    } else {
        try_to_attack(source, Game::get().dungeon.player);
    }
}

void Hammer::try_to_attack(Actor &source, Actor &target) {
    if (!is_in_range(source, target.position)) return;

    float damage = get_damage(target);
    if (enchantment) damage = enchantment->apply(damage, target);

    target.take_damage(damage, source);
}

bool Hammer::is_in_range(const Actor &source, sf::Vector2f target) const {
    return length_squared(source.position - target) <= hit_range * hit_range;
}

void Hammer::deepcopy_to(Hammer &other) const {
    Weapon::deepcopy_to(other);
    other.hit_range = hit_range;
    other.cooldown_timer = cooldown_timer;
    other.cooldown_time = cooldown_time;
    other.on_cooldown = on_cooldown;
}

std::shared_ptr<Item> Hammer::deepcopy_item() const { return deepcopy_shared(*this); }
