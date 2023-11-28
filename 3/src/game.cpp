#include "game.hpp"

#include <stdlib.h>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>

#include "SFML/Graphics/Rect.hpp"
#include "color_operations.hpp"
#include "vector_operations.hpp"

constexpr float PI = 3.14159265358979323846f;

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

sf::Color set_alpha(sf::Color color, sf::Uint8 transparency) {
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

Game &Game::get() {
    static std::shared_ptr<Game> game = nullptr;

    if (game == nullptr) {
        game = std::make_shared<Game>();
    }

    return *game;
}

static const char *const logo_name = "rock_eyebrow_meme.png";
static const char *const flor_tile_name = "dungeon_floor_4x4_yellow.png";
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
    if (is_in_game) return;

    is_in_game = true;
    clock.restart();

    dungeon.load_level(0);
}

void Game::stop_playing() {
    if (!is_in_game) return;

    is_in_game = false;

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
            (!is_in_game && is_pressed(event, sf::Keyboard::Escape)))
        {
            window.close();
            break;
        }

        if (is_in_game && is_pressed(event, sf::Keyboard::Escape)) {
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

bool Game::is_playing() const { return dungeon.player.alive; }

void Game::update(float delta_time) { dungeon.update(delta_time); }

void Game::handle_fixed_update(float delta_time) {
    fixed_delta_time_leftover += delta_time;
    while (fixed_delta_time_leftover >= fixed_delta_time) {
        fixed_delta_time_leftover -= fixed_delta_time;
        dungeon.fixed_update(fixed_delta_time);
    }
}

bool Game::run() {
    while (game_view.is_open()) {
        handle_events();

        float delta_time = clock.restart().asSeconds();
        if (is_playing()) {
            update(delta_time);
            handle_fixed_update(delta_time);
        }

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

long Game::actor_class_index_by_name(const std::string &name) {
    for (size_t i = 0; i < actor_classes.size(); i++) {
        if (actor_classes[i].name == name) return i;
    }
    return -1;
}

long Game::item_class_index_by_name(const std::string &name) {
    for (size_t i = 0; i < item_classes.size(); i++) {
        if (item_classes[i].name == name) return i;
    }
    return -1;
}

constexpr const char *game_name = "Epic Lab3 Game";

bool GameView::init(unsigned int width, unsigned int height) {
    window.create(
        sf::VideoMode(width, height, 32), game_name,
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    window.setVerticalSyncEnabled(true);

    float ratio = (float)window.getSize().x / (float)window.getSize().y;
    view.setSize(sf::Vector2f(Game::view_size * ratio, Game::view_size));
    view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
    window.setView(view);

    if (!font.loadFromFile(path_to_resources + "tuffy.ttf")) return false;

    if (!dungeon_level_view.init()) return false;
    inventory_view.init();
    experience_view.init();

    if (!logo_texture.loadFromFile(path_to_resources + logo_name)) return false;
    float scale = max(sf::Vector2f(view.getSize())) / 3.0f;
    setup_sprite(logo_texture, logo, {scale, scale});
    logo.setPosition({view.getSize().x / 2.0f, view.getSize().y * 2.0f / 3.0f});

    menu_message.setFont(font);
    menu_message.setCharacterSize(40);
    menu_message.setFillColor(sf::Color::White);
#ifdef SFML_SYSTEM_IOS
    menu_message.setString(
        std::string("Welcome to ") + game_name + "\nTouch the screen to start the game."
    );
#else
    menu_message.setString(
        std::string("Welcome to ") + game_name + "\n\nPress Enter to start the game."
    );
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
    if (!Game::get().is_in_game) {
        window.draw(logo);
        window.draw(menu_message);

        view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
        window.setView(view);
        return;
    }

    std::optional<DungeonLevel> &level = Game::get().dungeon.get_current_level();
    if (!level) {
        info_message.setString("No levels are loaded");
        center_text_origin(info_message);
        window.draw(info_message);

        view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
        window.setView(view);
        return;
    }

    float ratio = (float)window.getSize().x / (float)window.getSize().y;
    view.setSize(sf::Vector2f(Game::view_size * ratio, Game::view_size));
    view.setCenter(sf::Vector2f(Game::get().dungeon.player.position));
    window.setView(view);

    dungeon_level_view.draw(*level);
    inventory_view.draw(Game::get().dungeon.player.inventory);
    experience_view.draw(Game::get().dungeon.player.experience);

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
}

ItemUseResult LockPick::use(Actor &target) {
    std::optional<DungeonLevel> &level = Game::get().dungeon.get_current_level();
    if (!level) return ItemUseResult();

    auto coords = level->get_tile_coordinates(target.position);
    if (!coords) return ItemUseResult();

    Tile &tile = level->tiles[coords->first][coords->second];
    if (!tile.building) return ItemUseResult();

    auto result = tile.building->simulate_picking(target);

    auto tile_position =
        sf::Vector2f(coords->first, coords->second) * level->tile_coords_to_world_coords_factor();
    if (result.lock_picked) {
        for (auto &slot : tile.building->inventory.slots) {
            for (size_t k = 0; k < slot.size; ++k) {
                LayingItem laying_item(slot.item->deepcopy_item(), tile_position);
                level->laying_items.push_back(laying_item);
            }
        }
        tile.building = nullptr;
    }

    return ItemUseResult(result.pick_broken);
}

void LockPick::deepcopy_to(LockPick &other) const { Item::deepcopy_to(other); }

std::shared_ptr<Item> LockPick::deepcopy_item() const { return deepcopy_shared(*this); }

bool StackOfItems::add_item(std::shared_ptr<Item> new_item) {
    if (size == 0) {
        item = new_item;
        size = 1;
        return true;
    }

    if (item->item_class_index != new_item->item_class_index) return false;
    if (size >= item->get_class().max_stack_size) return false;

    ++size;
    return true;
}

void StackOfItems::remove_item(size_t amount) {
    amount = std::min(amount, size);
    size -= amount;
    if (size == 0) {
        item = nullptr;
    }
}

std::shared_ptr<Item> Inventory::get_item(size_t index) {
    if (0 <= index && index <= slots.size()) {
        if (slots[index].size != 0) {
            return slots[index].item;
        }
    }
    return nullptr;
}

void Inventory::use_item(size_t index, Actor &target) {
    std::shared_ptr<Item> item = get_item(index);
    if (!item) return;

    auto result = item->use(target);
    if (result.was_broken) {
        slots[index].remove_item(1);
    }
}

// void Inventory::recalculate_selection() {
//     if (0 <= selection && selection <= slots.size()) {
//         if (slots[selection].size != 0) {
//             return;
//         }
//     }

//     for (size_t i = 0; i < slots.size(); ++i) {
//         if (slots[i].size != 0) {
//             selection = i;
//             return;
//         }
//     }

//     selection = 0;
// }

bool Inventory::add_item(std::shared_ptr<Item> item) {
    // first check non empty slots to see if we can stack the item
    for (auto &slot : slots) {
        if (slot.size && slot.add_item(item)) {
            return true;
        }
    }

    // and only if we cannot stack the item, we should use one more slot
    for (auto &slot : slots) {
        if (!slot.size && slot.add_item(item)) {
            return true;
        }
    }

    return false;
}

float calculate_inventory_item_x(float length, size_t count, size_t i) {
    return (0.5f + (float)i) * length / (float)count;
}

void InventoryView::init() { stack_of_items_view.init(); }

void InventoryView::draw(const Inventory &inventory) {
    sf::View view = Game::get().game_view.view;

    float actual_size = inventory_item_size / Game::world_size;
    size_t count = inventory.slots.size();

    float x_base = view.getCenter().x + (1 - (float)count) * actual_size / 2;
    float y = view.getCenter().y + view.getSize().y / 2 - actual_size / 2;

    for (size_t i = 0; i < count; ++i) {
        sf::Vector2f position(x_base + actual_size * i, y);
        stack_of_items_view.draw(
            inventory.slots[i], position, inventory_item_size, i == inventory.selection
        );
    }
}

void ExperienceView::init() {
    level_text.setFont(Game::get().game_view.font);
    level_text.setCharacterSize(40);
    level_text.setFillColor(sf::Color::White);
    level_text.setOutlineColor(sf::Color::Black);
    level_text.setOutlineThickness(3);
}

void ExperienceView::draw(const Experience &experience) {
    sf::View view = Game::get().game_view.view;
    sf::Vector2f bar_position = view.getCenter() - view.getSize() / (2.0f + 0.1f);
    float width = view.getSize().x * relative_to_screen_width;
    till_next_level_bar.draw(
        bar_position, width,
        (float)experience.value / Experience::needs_exp_for_level(experience.level)
    );

    level_text.setString(std::to_string(experience.level));
    center_text_origin(level_text);
    sf::Vector2f text_position = bar_position;
    text_position.x += width + level_text.getGlobalBounds().width / 3.0f * 2.0f;
    text_position.y += width * till_next_level_bar.height_factor / 2.0f;
    level_text.setPosition(text_position);
    level_text.setScale(sf::Vector2f(1.0f, 1.0f) * text_ratio / Game::world_size);
    window.draw(level_text);
}

void LevelUpCanvas::draw() {}

void Dungeon::init() {
    for (auto &level : all_levels) {
        level.init();
    }
    Game::get().dungeon.player.init();
}

void Dungeon::update(float delta_time) {
    if (current_level) {
        current_level->update(delta_time);
    }
}

void Dungeon::fixed_update(float delta_time) {
    if (current_level) {
        current_level->fixed_update(delta_time);
    }
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

std::optional<std::pair<size_t, size_t>> DungeonLevel::get_tile_coordinates(sf::Vector2f position
) const {
    sf::Vector2f world_coords = position / tile_coords_to_world_coords_factor();
    if (position.x < 0 || position.x >= tiles.size() || position.y < 0 ||
        position.y >= tiles.row_size())
        return std::nullopt;
    return std::make_pair(position.x, position.y);
}

Tile *DungeonLevel::get_tile(sf::Vector2f position) {
    auto coords = get_tile_coordinates(position);
    if (!coords) return nullptr;
    return &tiles[coords->first][coords->second];
}

void DungeonLevel::resize_tiles(size_t width, size_t height) { tiles.resize(width, height); }

void DungeonLevel::regenerate() {
    regenerate_tiles();
    regenerate_enemies();
    regenerate_laying_items();
}

void DungeonLevel::regenerate_tiles() {
    RangeOfLong range_chest_spawn(0, 50);
    size_t max_level = 9;
    RangeOfLong range_chest_level(0, (max_level + 1) * (max_level + 1) - 1);
    RangeOfLong range_chest_item(0, Game::get().item_templates.size() - 1);

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

    RangeOfFloat range_x(2, tiles.size() - 2);
    RangeOfFloat range_y(2, tiles.row_size() - 2);

    for (size_t class_index = 1; class_index < Game::get().actor_classes.size(); ++class_index) {
        for (size_t i = 0; i < actors_spawned_per_class; ++i) {
            Enemy &enemy = enemies.emplace_back(Game::get().make_enemy(class_index));
            enemy.position = sf::Vector2f(range_x.get_random(), range_y.get_random());
            enemy.position *= tile_coords_to_world_coords_factor();
        }
    }
}

void DungeonLevel::regenerate_laying_items() {
    laying_items.clear();

    RangeOfFloat range_x(2, tiles.size() - 2);
    RangeOfFloat range_y(2, tiles.row_size() - 2);

    for (size_t class_index = 1; class_index < Game::get().item_classes.size(); ++class_index) {
        for (size_t i = 0; i < laying_items_spawned_per_class; ++i) {
            LayingItem &litem =
                laying_items.emplace_back(LayingItem(Game::get().make_item(class_index)));
            litem.position = sf::Vector2f(range_x.get_random(), range_y.get_random());
            litem.position *= tile_coords_to_world_coords_factor();
        }
    }
}

void DungeonLevel::update(float delta_time) {
    for (auto &enemy : enemies) {
        enemy.update(delta_time);
    }
    Game::get().dungeon.player.update(delta_time);

    for (auto &enemy : enemies) {
        enemy.apply_friction();
    }
    Game::get().dungeon.player.apply_friction();

    delete_dead_actors();
    delete_picked_up_items();
    handle_collitions();
}

void DungeonLevel::fixed_update(float delta_time) {
    for (auto &enemy : enemies) {
        enemy.fixed_update(delta_time);
    }
    Game::get().dungeon.player.fixed_update(delta_time);
}

void DungeonLevel::delete_dead_actors() {
    size_t c = 0;
    for (size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[c].ready_to_be_deleted()) {
            ++c;
        } else if (!enemies[i].ready_to_be_deleted()) {
            std::swap(enemies[c], enemies[i]);
            ++c;
        }
    }

    for (size_t i = c; i < enemies.size(); ++i) {
        enemies[i].on_deletion();
    }

    enemies.erase(std::next(enemies.begin(), c), enemies.end());
}

void DungeonLevel::delete_picked_up_items() {
    size_t c = 0;
    for (size_t i = 0; i < laying_items.size(); ++i) {
        if (!laying_items[c].picked_up) {
            ++c;
        } else if (!laying_items[i].picked_up) {
            std::swap(laying_items[c], laying_items[i]);
            ++c;
        }
    }
    laying_items.erase(std::next(laying_items.begin(), c), laying_items.end());
}

void DungeonLevel::handle_collitions() {
    {
        std::vector<RigidBody *> bodies(enemies.size() + 1);
        for (size_t i = 0; i < enemies.size(); ++i) {
            bodies[i] = &enemies[i];
        }
        bodies[enemies.size()] = &Game::get().dungeon.player;

        handle_rigid_body_level_collitions(bodies);
        handle_actor_actor_collitions(bodies);
        handle_rigid_body_level_collitions(bodies);
    }
    {
        std::vector<RigidBody *> item_bodies(laying_items.size());
        for (size_t i = 0; i < laying_items.size(); ++i) {
            item_bodies[i] = &laying_items[i];
        }
        handle_rigid_body_level_collitions(item_bodies);
    }
}

void DungeonLevel::handle_actor_actor_collitions(std::vector<RigidBody *> &bodies) {
    // optimisation: space partitioning algorithm to get rid of O(n^2)

    std::vector<sf::FloatRect> aabbs(bodies.size());
    for (size_t i = 0; i < bodies.size(); ++i) {
        aabbs[i] = bodies[i]->get_axes_aligned_bounding_box();
    }

    std::vector<sf::Vector2f> directions(bodies.size());

    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            sf::FloatRect intersection;
            if (!aabbs[i].intersects(aabbs[j], intersection)) continue;

            sf::Vector2f diff = ::center(aabbs[i]) - ::center(aabbs[j]);
            sf::Vector2f correction = intersection.getSize() * 2 / 3;

            // make sure the direction is correct
            if (diff.x < 0) {
                correction.x = -correction.x;
            }
            if (diff.y < 0) {
                correction.y = -correction.y;
            }

            // choose the axis to correct (minimal one)
            if (abs(correction.x) < abs(correction.y)) {
                correction.y = 0;
            } else {
                correction.x = 0;
            }

            directions[i] += correction;
            directions[j] -= correction;
        }
    }

    for (size_t i = 0; i < bodies.size(); ++i) {
        if (bodies[i]->pushable) bodies[i]->position += directions[i];
    }
}

void DungeonLevel::handle_rigid_body_level_collitions(std::vector<RigidBody *> &bodies) {
    std::vector<sf::FloatRect> aabbs(bodies.size());
    for (size_t i = 0; i < bodies.size(); ++i) {
        aabbs[i] = bodies[i]->get_axes_aligned_bounding_box();
    }

    float right_wall = tiles.size() * tile_coords_to_world_coords_factor();
    float down_wall = tiles.row_size() * tile_coords_to_world_coords_factor();

    for (size_t i = 0; i < bodies.size(); ++i) {
        if (aabbs[i].left < 0) {
            bodies[i]->position.x += 0 - aabbs[i].left;
            bodies[i]->velocity.x -= bodies[i]->velocity.x * rebounce_factor;
        }
        if (aabbs[i].top < 0) {
            bodies[i]->position.y += 0 - aabbs[i].top;
            bodies[i]->velocity.y -= bodies[i]->velocity.y * rebounce_factor;
        }

        float right = aabbs[i].left + aabbs[i].width;
        if (right > right_wall) {
            bodies[i]->position.x -= right - right_wall;
            bodies[i]->velocity.x -= bodies[i]->velocity.x * rebounce_factor;
        }
        float down = aabbs[i].top + aabbs[i].height;
        if (down > down_wall) {
            bodies[i]->position.y -= down - down_wall;
            bodies[i]->velocity.y -= bodies[i]->velocity.y * rebounce_factor;
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
        if (laying_item.picked_up) continue;
        actors_view.items_view.draw(*(laying_item.item), laying_item.position);
    }

    for (const auto &emeny : level.enemies) {
        actors_view.draw(emeny);
    }
    actors_view.draw(Game::get().dungeon.player);

    for (const auto &emeny : level.enemies) {
        actors_view.draw_ui(emeny);
    }
    actors_view.draw_ui(Game::get().dungeon.player);
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

float symmetric_linear_easing(float t, float p) {
    if (t < p) {
        return t / p;
    } else if (t < 1.0f - p) {
        return 1.0f;
    } else {
        return (1.0f - t) / p;
    }
}

void SpriteColorAnimator::update(sf::Time elapsed_time, sf::Sprite &sprite) {
    if (elapsed_time > duration) {
        sprite.setColor(inactive_color);
    } else {
        float t = symmetric_linear_easing(elapsed_time.asSeconds() / duration.asSeconds(), 0.3f);
        sprite.setColor(inactive_color * (1 - t) + active_color * t);
    }
}

void ActorsView::draw(const Actor &actor) {
    sf::Sprite &sprite = actor.get_class().sprite;
    sf::Vector2f saved = sprite.getScale();
    sprite.setScale(saved * actor.size / Game::world_size);
    sprite.setPosition(actor.position);
    taked_damage_animator.update(actor.since_last_taken_damage.getElapsedTime(), sprite);
    if (!actor.alive) {
        sprite.setColor(sprite.getColor() * death_color_multiplier);
    }
    window.draw(sprite);
    sprite.setScale(saved);

    if (actor.equipment.weapon()) items_view.draw(*actor.equipment.weapon(), actor.position);
}

void ActorsView::draw_ui(const Actor &actor) {
    float bar_width = actor.size / Game::world_size * 1.2f;
    health_bar.draw(
        actor.position - bar_width / 2.0f, bar_width,
        std::max(0.0f, actor.health) / actor.characteristics.max_health
    );
}

void ProgressBarView::draw(sf::Vector2f position, float bar_width, float ratio) {
    max_bar.setSize(sf::Vector2f(1.0f, height_factor) * bar_width);
    max_bar.setPosition(position);
    max_bar.setFillColor(max_bar_color);
    window.draw(max_bar);

    cur_bar.setSize({max_bar.getSize().x * ratio, max_bar.getSize().y});
    cur_bar.setPosition(max_bar.getPosition());
    cur_bar.setFillColor(cur_bar_color);
    window.draw(cur_bar);
}

void ItemsView::draw(const Item &item, sf::Vector2f position) {
    auto &cls = item.get_class();
    sf::Sprite &sprite = cls.sprite;
    sf::Vector2f saved = sprite.getScale();
    sprite.setScale(saved * cls.size / Game::world_size);
    sprite.setPosition(position);
    window.draw(sprite);
    sprite.setScale(saved);
}

void StackOfItemsView::init() {
    count_text.setFont(Game::get().game_view.font);
    count_text.setCharacterSize(40);
    count_text.setFillColor(sf::Color::White);
    count_text.setOutlineColor(sf::Color::Black);
    count_text.setOutlineThickness(3);
}

void StackOfItemsView::draw(
    const StackOfItems &stack, sf::Vector2f position, float size, bool selected
) {
    float resulting_size;
    sf::Sprite sprite;

    if (stack.size == 0) {
        resulting_size = size;
    } else {
        auto &cls = stack.item->get_class();
        sprite = cls.sprite;
        resulting_size = cls.size;
    }

    sf::Vector2f saved = sprite.getScale();

    sprite.setScale(saved * resulting_size / Game::world_size);
    sprite.setPosition(position);

    sf::FloatRect bounds = sprite.getGlobalBounds();

    sf::RectangleShape background;
    background.setPosition(bounds.getPosition());
    background.setSize(bounds.getSize());
    background.setFillColor(background_color);
    if (selected) {
        background.setOutlineColor(selection_color);
        background.setOutlineThickness(selection_thickness / Game::world_size);
    }
    window.draw(background);

    window.draw(sprite);

    if (stack.size > 1) {
        count_text.setString(std::to_string(stack.size));
        center_text_origin(count_text);
        count_text.setPosition(
            position + sprite.getGlobalBounds().getSize() / 2 -
            count_text.getGlobalBounds().getSize() * 2 / 3
        );
        count_text.setScale(saved * resulting_size * text_ratio / Game::world_size);
        window.draw(count_text);
    }
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

std::optional<DungeonLevel> &Dungeon::get_current_level() { return current_level; }

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
    RangeOfLong range(0, 1 + 2 * level);
    if ((float)range.get_random() / source.characteristics.luck <= 1) {
        return {true, false};
    }
    return {false, (float)range.get_random() * source.characteristics.luck <= 1};
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

std::shared_ptr<Item> &Equipment::weapon() { return items[Wearable::Count]; }

const std::shared_ptr<Item> &Equipment::weapon() const { return items[Wearable::Count]; }

Equipment::Wearables &Equipment::wearables() { return *reinterpret_cast<Wearables *>(&items); }

const Equipment::Wearables &Equipment::wearables() const {
    return *reinterpret_cast<const Wearables *>(&items);
}

bool Equipment::equip_wearable(std::shared_ptr<Item> item) {
    Wearable &as_wearable = dynamic_cast<Wearable &>(*item);

    auto &it = wearables()[as_wearable.kind];
    if (!it) {
        it = item;
        return true;
    }
    return false;
}

bool Equipment::equip_weapon(std::shared_ptr<Item> item) {
    if (!weapon()) {
        weapon() = item;
        return true;
    }
    return false;
}

void Equipment::deepcopy_to(Equipment &other) const {
    for (size_t i = 0; i < size; ++i) {
        if (items[i])
            other.items[i] = items[i]->deepcopy_item();
        else
            other.items[i] = nullptr;
    }
}

bool Actor::ready_to_be_deleted() const {
    return !alive &&
           (!is_moving() || since_last_taken_damage.getElapsedTime() > ready_to_be_deleted_after);
}

void Actor::recalculate_characteristics() {
    characteristics = base_characteristics;
    for (auto &it : equipment.items) {
        if (!it) continue;
        it->update_owner_characteristics(characteristics);
    }
}

void Actor::deepcopy_to(Actor &other) const {
    RigidBody::deepcopy_to(other);
    other.actor_class_index = actor_class_index;
    other.health = health;
    equipment.deepcopy_to(other.equipment);
    other.characteristics = characteristics;
    other.base_characteristics = base_characteristics;
    other.alive = alive;
}

float Actor::calculate_defence() {
    float defence = characteristics.defence;
    for (auto &it : equipment.wearables()) {
        if (!it) continue;
        defence += it->generate_defence();
    }
    return defence;
}

void Actor::take_damage(float amount, Actor &source) {
    if (!alive) return;
    since_last_taken_damage.restart();
    health -= std::max(0.0f, amount - calculate_defence());
    if (health <= 0.0f) {
        health = 0.0f;
        die(source);
    }
}

ActorClass &Actor::get_class() const { return Game::get().actor_classes[actor_class_index]; }

void Experience::gain(size_t amount) {
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

size_t Experience::as_value_after_death() { return level * level * level; }

size_t Experience::needs_exp_for_level(size_t level) { return 4 * level * level + 10 * level + 10; }

void Experience::level_up() { level += 1; }

void RigidBody::deepcopy_to(RigidBody &other) const {
    other.position = position;
    other.pushable = pushable;
    other.size = size;
}

bool RigidBody::is_moving(float epsilon) const {
    return velocity.x > epsilon || velocity.y > epsilon;
}

void RigidBody::move(sf::Vector2f direction, float speed, float delta_time) {
    position += normalized(direction) * speed * delta_time;
}

void RigidBody::apply_force(sf::Vector2f forece) { acceleration += forece / mass; }

void RigidBody::apply_impulse(sf::Vector2f impulse) { velocity += impulse / mass; }

void RigidBody::apply_friction() { apply_force(-(velocity)*friction_coefficient * mass); }

void RigidBody::fixed_update(float delta_time) {
    velocity += acceleration * delta_time;
    position += velocity * delta_time;
    acceleration = sf::Vector2f(0, 0);
}

sf::FloatRect RigidBody::get_axes_aligned_bounding_box() const {
    float size = this->size / Game::world_size;
    return sf::FloatRect(position.x - size / 2, position.y - size / 2, size, size);
}

sf::Vector2f center(const sf::FloatRect &a) {
    return sf::Vector2f(a.left + a.width / 2, a.top + a.height / 2);
}

void Player::init() {}

void Player::deepcopy_to(Player &other) const {
    Actor::deepcopy_to(other);
    other.inventory = inventory;
    other.experience = experience;
}

void Player::fixed_update(float delta_time) {
    RigidBody::fixed_update(delta_time);

    if (!alive) return;

    handle_movement(delta_time);
}

void Player::update(float delta_time) {
    if (!alive) return;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
        if (equipment.weapon()) {
            throw_out_item(equipment.weapon());
            equipment.weapon() = nullptr;
            recalculate_characteristics();
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) {
        for (auto &it : equipment.wearables()) {
            if (!it) continue;
            throw_out_item(it);
            it = nullptr;
            recalculate_characteristics();
        }
    }

    handle_equipment_use();
    handle_inventory_selection();
    handle_inventory_use();
    handle_picking_up_items();
}

void Player::handle_movement(float delta_time) {
    sf::Vector2f direction(0, 0);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        direction.y -= 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        direction.y += 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        direction.x -= 1;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        direction.x += 1;
    }

    move(direction, characteristics.speed, delta_time);
}

void Player::handle_equipment_use() {
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) return;

    if (!equipment.weapon()) return;

    equipment.weapon()->use(*this);
}

void Player::handle_inventory_selection() {
    std::vector<sf::Keyboard::Key> keys = {
        sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3, sf::Keyboard::Num4,
        sf::Keyboard::Num5, sf::Keyboard::Num6, sf::Keyboard::Num7, sf::Keyboard::Num8,
        sf::Keyboard::Num9, sf::Keyboard::Num0,
    };
    for (size_t i = 0; i < keys.size(); ++i) {
        if (sf::Keyboard::isKeyPressed(keys[i])) {
            inventory.selection = i;
        }
    }
}

void Player::handle_inventory_use() {
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::F)) return;

    inventory.use_item(inventory.selection, *this);
}

void Player::throw_out_item(std::shared_ptr<Item> item) const {
    float angle = RangeOfFloat(0, 2 * PI).get_random();
    float len = pick_up_range * 1.5;
    sf::Vector2f item_position = position + sf::Vector2f(std::cos(angle), std::sin(angle)) * len;
    Game::get().dungeon.get_current_level()->laying_items.push_back(LayingItem(item, item_position)
    );
}

bool Player::pick_up_item(std::shared_ptr<Item> item) {
    bool result;
    if (item->get_class().kind == Item::Kind::Weapon) {
        result = equipment.equip_weapon(item);
    } else if (item->get_class().kind == Item::Kind::Wearable) {
        result = equipment.equip_wearable(item);
    } else {
        result = inventory.add_item(item);
    }
    if (result) {
        recalculate_characteristics();
    }
    return result;
}

void Player::handle_picking_up_items() {
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) return;

    std::vector<LayingItem> &laying_items = Game::get().dungeon.get_current_level()->laying_items;

    // laying_items can become larger (but should not go smaller, but still) during the loop
    size_t size = laying_items.size();
    for (size_t i = 0; i < std::min(size, laying_items.size()); ++i) {
        LayingItem &item = laying_items[i];
        if (!item.picked_up && item.since_last_pick_up.getElapsedTime() > pick_up_timeout &&
            length_squared(item.position - position) <= pick_up_range * pick_up_range)
        {
            if (pick_up_item(item.item)) {
                item.picked_up = true;
            }
        }
    }
}

void Player::recalculate_characteristics() {
    Actor::recalculate_characteristics();
    for (auto &slot : inventory.slots) {
        if (!slot.item) continue;
        for (size_t i = 0; i < slot.size; ++i) {
            slot.item->update_owner_characteristics(characteristics);
        }
    }
}

void Player::die(Actor &reason) { alive = false; }

void Enemy::init() {}

void Enemy::fixed_update(float delta_time) {
    RigidBody::fixed_update(delta_time);

    if (!alive) return;

    handle_movement(delta_time);
}

void Enemy::update(float delta_time) {
    if (!alive) return;
    handle_equipment_use();
}

bool Enemy::pick_up_item(std::shared_ptr<Item> item) {
    if (item->get_class().kind == Item::Kind::Weapon) {
        return equipment.equip_weapon(item);
    } else if (item->get_class().kind == Item::Kind::Wearable) {
        return equipment.equip_wearable(item);
    } else {
        return false;
    }
}

void Enemy::handle_movement(float delta_time) {
    move(Game::get().dungeon.player.position - position, characteristics.speed, delta_time);
}

void Enemy::handle_equipment_use() {
    if (equipment.weapon()) {
        equipment.weapon()->use(*this);
    }
}

void Enemy::die(Actor &reason) {
    if (!alive) return;

    alive = false;
    reason.experience.gain(experience.as_value_after_death());
}

void Enemy::on_deletion() {
    std::optional<DungeonLevel> &level = Game::get().dungeon.get_current_level();
    if (!level) return;

    RangeOfLong range_chest_item(0, Game::get().item_templates.size() - 1);
    std::shared_ptr<Item> item = Game::get().make_item(range_chest_item.get_random());

    LayingItem laying_item(item, position);
    level->laying_items.push_back(laying_item);
}

void Enemy::deepcopy_to(Enemy &other) const { Actor::deepcopy_to(other); }

void Item::update_owner_characteristics(Characteristics &characteristics) {
    auto &artefact = get_class().artefact;
    if (!artefact) return;
    artefact->apply(characteristics);
}

void Item::deepcopy_to(Item &other) const { other.item_class_index = item_class_index; }

ItemClass &Item::get_class() const { return Game::get().item_classes[item_class_index]; }

void Potion::apply(Actor &target) { modifier.apply(target.characteristics); }

ItemUseResult Potion::use(Actor &target) {
    apply(target);
    return ItemUseResult();
}

void Weapon::deepcopy_to(Weapon &other) const {
    Item::deepcopy_to(other);
    other.enchantment = enchantment;
    other.damage_range = damage_range;
}

float Weapon::get_damage(Actor &target) { return damage_range.get_random(); }

void WeaponWithCooldown::deepcopy_to(WeaponWithCooldown &other) const {
    Weapon::deepcopy_to(other);
    other.since_last_use = since_last_use;
    other.cooldown_time = cooldown_time;
    other.on_cooldown = on_cooldown;
}

bool MeleeWeapon::try_to_attack(Actor &source, Actor &target) {
    if (!is_in_range(source, target.position)) return false;

    float damage = get_damage(target);
    if (enchantment) damage = enchantment->apply(damage, target);

    target.apply_force(
        -normalized(source.position - target.position) * damage / (float)damage_range.max *
        push_back_force_multiplier
    );
    target.take_damage(damage, source);

    return true;
}

bool MeleeWeapon::test_cooldown() {
    if (!on_cooldown) return false;
    if (since_last_use.getElapsedTime() > cooldown_time) return false;
    return true;
}

void MeleeWeapon::ensure_cooldown() {
    since_last_use.restart();
    on_cooldown = true;
}

ItemUseResult MeleeWeapon::use(Actor &source) {
    if (test_cooldown()) return ItemUseResult();

    bool reached_anything = false;

    if (source.actor_class_index == Game::player_class_index) {
        for (auto &enemy : Game::get().dungeon.get_current_level()->enemies) {
            reached_anything = try_to_attack(source, enemy) || reached_anything;
        }
    } else {
        reached_anything = try_to_attack(source, Game::get().dungeon.player) || reached_anything;
    }

    if (reached_anything) ensure_cooldown();
    return ItemUseResult();
}

void MeleeWeapon::deepcopy_to(MeleeWeapon &other) const {
    WeaponWithCooldown::deepcopy_to(other);
    other.push_back_force_multiplier = push_back_force_multiplier;
}

bool Hammer::is_in_range(const Actor &source, sf::Vector2f target) const {
    return length_squared(source.position - target) <= hit_range * hit_range;
}

void Hammer::deepcopy_to(Hammer &other) const {
    MeleeWeapon::deepcopy_to(other);
    other.hit_range = hit_range;
}

std::shared_ptr<Item> Hammer::deepcopy_item() const { return deepcopy_shared(*this); }

bool Sword::is_in_range(const Actor &source, sf::Vector2f target) const {
    return length_squared(source.position - target) <= hit_range * hit_range;
}

void Sword::deepcopy_to(Sword &other) const {
    MeleeWeapon::deepcopy_to(other);
    other.hit_range = hit_range;
}

std::shared_ptr<Item> Sword::deepcopy_item() const { return deepcopy_shared(*this); }

float Wearable::generate_defence() { return defence_range.get_random(); }

void Wearable::deepcopy_to(Wearable &other) const {
    Item::deepcopy_to(other);
    other.kind = kind;
    other.defence_range = defence_range;
}

void Shield::deepcopy_to(Shield &other) const { Wearable::deepcopy_to(other); }

std::shared_ptr<Item> Shield::deepcopy_item() const { return deepcopy_shared(*this); }
