#include "game.hpp"

#if 1
#include <stdlib.h>
#else
// manna see magic on wsl?)
#include <cstdlib>
#endif

#ifdef NDEBUG
#undef NDEBUG
#include <cassert>
#define NDEBUG
#else
#include <cassert>
#endif

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <boost/version.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/dll/import.hpp>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#include "color_operations.hpp"
#include "shared.hpp"
#include "toml++/toml.hpp"
#include "vector_operations.hpp"

#if BOOST_VERSION >= 107600
   #define boost_dll_import boost::dll::import_symbol
#else
   #define boost_dll_import boost::dll::import
#endif

namespace fs = std::filesystem;

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

EnemyThreads::~EnemyThreads() {
    shutdown();

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void EnemyThreads::init() {
    threads.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back(std::thread(&EnemyThreads::run, this, i));
    }
}

void EnemyThreads::run(size_t id) {
    while (1) {
        {
            std::unique_lock<std::mutex> lck(sync_mutex);
            children_cv.wait(lck, [&] { return can_start_update || is_shutting_down; });
            if (is_shutting_down) return;
        }

        Game::get().update_enemies_in_thread(delta_time, id);

        sync_point->arrive_and_wait();
    }
}

void EnemyThreads::start_updates() {
    auto on_completion = [this]() noexcept {
        // The work for this batch is done. Signal the main thread.
        std::lock_guard<std::mutex> lck(sync_mutex);
        can_start_update = false;
        parent_cv.notify_one();
    };

    sync_point = std::make_unique<std::barrier<std::function<void()>>>(thread_count, on_completion);

    {
        std::unique_lock<std::mutex> lck(sync_mutex);
        can_start_update = true;
    }
    children_cv.notify_all();
}

void EnemyThreads::join_updates() {
    std::unique_lock<std::mutex> lck(sync_mutex);
    parent_cv.wait(lck, [&] { return !can_start_update; });
}

void EnemyThreads::shutdown() {
    {
        std::unique_lock<std::mutex> lck(sync_mutex);
        can_start_update = false;
        is_shutting_down = true;
    }
    children_cv.notify_all();
}

Game &Game::get(bool brand_new) {
    static std::shared_ptr<Game> game = nullptr;

    if (game == nullptr || brand_new) {
        game = std::make_shared<Game>();
    }

    return *game;
}

static const char *const logo_name = "rock_eyebrow_meme.png";
static const char *const flor_tile_name = "dungeon_floor_4x4_yellow.png";
static const char *const open_dor_tile_name = "dungeon_open_door.jpeg";
static const char *const closed_dor_tile_name = "dungeon_closed_door.jpeg";
static const char *const barrier_tile_name = "black.jpeg";
static const char *const up_laddor_tile_name = "up_ladder.jpeg";
static const char *const down_laddor_tile_name = "down_ladder.jpeg";
static const char *const chest_name = "chest.png";

bool Game::init(unsigned int width, unsigned int height) {
    for (auto &it : actor_classes) {
        if (!it.init()) return false;
    }
    for (auto &it : item_classes) {
        if (!it.init()) return false;
    }
    dungeon.init();
    enemy_threads.init();
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
    have_won = false;

    dungeon.unload_current_level();
}

void Game::save(const std::string &filename) {
    TRY_CATCH_ALL({
        std::ofstream ofs(filename);
        boost::archive::text_oarchive oa(ofs);
        oa << *this;
    })
    //  catch (...) {
    //     std::cout << "Failed to save game" << std::endl;
    // }
}

void Game::load(const std::string &filename) {
    TRY_CATCH_ALL({
        std::ifstream ifs(filename);
        boost::archive::text_iarchive ia(ifs);
        ia >> *this;
    })
    // } catch (...) {
    //     std::cout << "Failed to load game" << std::endl;
    // }
}

void Game::setup_default_actors() {
    size_t player_id =
        add_actor_class(ActorClass("player", "plays the game", "hide_the_plan.jpeg"));
    size_t goblin_id = add_actor_class(ActorClass("goblin", "deez nuts", "rock_smiling.jpeg"));
    size_t pepe_id = add_actor_class(ActorClass("pepe", "he angy", "pepe_angry.jpeg"));

    enemy_templates.resize(actor_classes.size());
    player_template = Player(player_id, 10.0f, Characteristics(100.0f, 4.0f, 5.0f), 0);
    player_template.pushable = false;
    player_template.mass = 10000.0f;
    enemy_templates[pepe_id] = Enemy(pepe_id, 5.0f, Characteristics(40.0f, 0.0f, 4.0f), 1);
    enemy_templates[goblin_id] = Enemy(goblin_id, 7.0f, Characteristics(40.0f, 2.0f, 2.0f), 2);
}

void Game::import_item_plugin(const ItemPlugin &plugin) {
    std::vector<ItemPlugin::item_type> vec;
    plugin.add_classes_and_templates(vec);

    size_t index = item_classes.size();
    for (auto &it : vec) {
        item_classes.push_back(it.first);
        item_templates.push_back(std::move(it.second));
        item_templates[index]->item_class_index = index;
        ++index;
    }
}

void Game::import_item_plugin_from_file(const std::string &filename) {
    boost::dll::fs::path lib_path(filename);
    std::cout << "Loading plugin: " << filename << std::endl;
    std::shared_ptr<ItemPlugin> plugin = boost_dll_import<ItemPlugin>(
        lib_path, "item_plugin", boost::dll::load_mode::append_decorations
    );
    loaded_item_plugins.push_back(plugin);
    import_item_plugin(*plugin);
}

void Game::load_item_plugins(const std::string &directory) {
    fs::path dir = directory;
    for (const auto &entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == boost::dll::shared_library::suffix()) {
            import_item_plugin_from_file(entry.path().string());
        }
    }
}

void Game::setup_default_items() {
    long hammer_id = item_class_index_by_name("hammer");
    if (hammer_id != -1) {
        player_template.pick_up_item(make_item(hammer_id));
    } else {
        std::cerr << "Could not load hammer" << std::endl;
    }

    long sword_id = item_class_index_by_name("sword");
    if (sword_id != -1) {
        enemy_templates[actor_class_index_by_name("goblin")].pick_up_item(make_item(sword_id));
    } else {
        std::cerr << "Could not load hammer" << std::endl;
    }
}

void Game::handle_save_load() {
    if (keys_pressed_on_this_frame[sf::Keyboard::L]) {
        load("save.txt");
    }
    if (keys_pressed_on_this_frame[sf::Keyboard::O]) {
        save("save.txt");
    }
}

template <typename T, typename P>
boost::optional<T> get_as__with_typename(P &parent, const char *path, const char *type_name) {
    toml::node_view<toml::node> el = parent.at_path(path);
    if (!el) return boost::none;

    if (auto value = el.value<T>(); value) {
        return *value;
    }

    std::cerr << "TypeError: expected " << path << " to be of type " << type_name << ", got "
              << el.type() << std::endl;
    return boost::none;
}

#define get_as(table, path, T) get_as__with_typename<T>(table, path, #T)

#define STR(x) #x

template <typename P>
boost::optional<toml::array &> get_as_array(P &parent, const char *path) {
    toml::node_view<toml::node> el = parent.at_path(path);
    if (!el) return boost::none;

    toml::array *value = el.as_array();
    if (value) {
        return *value;
    }

    std::cerr << "TypeError: expected " << path << " to be of type " << STR(toml::array) << ", got "
              << el.type() << std::endl;
    return boost::none;
}

template <typename T, typename P>
T get_as_or__with_typename(P &parent, const char *path, const char *type_name, T default_value) {
    if (auto value = get_as__with_typename<T, P>(parent, path, type_name); value) {
        return *value;
    }
    return default_value;
}

#define get_as_or(table, path, T, default_value) \
    get_as_or__with_typename<T>(table, path, #T, default_value)

template <typename P>
void load_level_from_config(P parent, Dungeon &dungeon) {
    DungeonLevel level;
    if (auto value = get_as(parent, "actors_spawned_per_class", size_t); value) {
        level.actors_spawned_per_class = *value;
    }
    if (auto value = get_as(parent, "laying_items_spawned_per_class", size_t); value) {
        level.laying_items_spawned_per_class = *value;
    }
    size_t size = get_as_or(parent, "size", size_t, 30);
    level.resize_tiles(size, size);
    level.regenerate();
    dungeon.add_level(level);
}

bool Game::load_config(const std::string &filename) {
    toml::table table;
    try {
        table = toml::parse_file(filename);
    } catch (const toml::parse_error &err) {
        std::cerr << "Error parsing file '" << *err.source().path << "':\n"
                  << err.description() << "\n (" << err.source().begin << ")\n";
        return false;
    }

    std::string item_plugins_directory =
        get_as_or(table, "item_plugins_directory", std::string, "item_plugins");

    setup_default_actors();
    load_item_plugins(item_plugins_directory);
    setup_default_items();

    if (auto arr = get_as_array(table, "levels"); arr) {
        arr->for_each([&](auto &&el) {
            if constexpr (toml::is_table<decltype(el)>) {
                load_level_from_config(el, dungeon);
            } else {
                std::cerr << "TypeError: all 'levels' items must be arrays, not " << el.type()
                          << std::endl;
            }
        });
    }

    return true;
}

bool is_pressed(const sf::Event &event, sf::Keyboard::Key key) {
    return (event.type == sf::Event::KeyPressed) && (event.key.code == key);
}

void Game::handle_events() {
    sf::RenderWindow &window = game_view.window;
    sf::View &view = game_view.view;

    std::fill(keys_pressed_on_this_frame.begin(), keys_pressed_on_this_frame.end(), false);

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

        if (event.type == sf::Event::KeyPressed) {
            keys_pressed_on_this_frame[event.key.code] = true;
        }
    }
}

bool Game::is_playing() const { return dungeon.player.alive && !have_won; }

void Game::update(float delta_time) { dungeon.update(delta_time); }

void Game::update_enemies_in_thread(float delta_time, size_t id) {
    dungeon.update_enemies_in_thread(delta_time, id);
}

void Game::handle_fixed_update(float delta_time) {
    fixed_delta_time_leftover += delta_time;
    while (fixed_delta_time_leftover >= fixed_delta_time) {
        fixed_delta_time_leftover -= fixed_delta_time;
        dungeon.fixed_update(fixed_delta_time);
    }
}

bool Game::run() {
    float accumulated_time = 0.0f;

    while (game_view.is_open()) {
        handle_events();
        handle_save_load();

        accumulated_time += time_scale;
        if (accumulated_time >= (1.0f - time_scale_epsilon) && is_playing()) {
            accumulated_time = 0.0f;
            float delta_time = clock.restart().asSeconds();
            delta_time *= time_scale;
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
    holder_of_items_view.init();
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

    important_message.setFont(font);
    important_message.setCharacterSize(60);
    important_message.setOutlineThickness(5);
    important_message.setOutlineColor(sf::Color::Black);
    important_message.setPosition(sf::Vector2f(view.getSize()) / 2.0f);
    important_message.setScale(
        sf::Vector2f(Game::view_size, Game::view_size) / (float)std::min(width, height)
    );

    return true;
}

bool GameView::is_open() const { return window.isOpen(); }

void GameView::clear() { window.clear(sf::Color(50, 50, 50)); }

void GameView::display() { window.display(); }

sf::FloatRect GameView::get_display_rect(float scale) const {
    auto size = view.getSize() * scale;
    auto pos = view.getCenter();
    return sf::FloatRect(pos.x - size.x / 2.0f, pos.y - size.y / 2.0f, size.x, size.y);
}

void GameView::draw() {
    if (!Game::get().is_in_game) {
        window.draw(logo);
        window.draw(menu_message);

        view.setCenter(sf::Vector2f(view.getSize()) / 2.0f);
        window.setView(view);
        return;
    }

    auto &level = Game::get().dungeon.current_level;
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
    if (Game::get().is_inventory_selected) {
        Inventory &inventory = Game::get().dungeon.player.inventory;
        holder_of_items_view.draw(
            inventory.slots.data(), inventory.slots.size(), inventory.selection
        );
    } else {
        Equipment &equipment = Game::get().dungeon.player.equipment;
        holder_of_items_view.draw(
            equipment.slots.data(), equipment.slots.size(), equipment.selection
        );
    }
    experience_view.draw(Game::get().dungeon.player.experience);

    if (!Game::get().dungeon.player.alive) {
        important_message.setString("YOU DIED");
        center_text_origin(important_message);
        important_message.setPosition(sf::Vector2f(Game::get().dungeon.player.position));
        important_message.setFillColor(sf::Color::Red);
        window.draw(important_message);
    } else if (Game::get().have_won) {
        important_message.setString("YOU WON");
        center_text_origin(important_message);
        important_message.setPosition(sf::Vector2f(Game::get().dungeon.player.position));
        important_message.setFillColor(sf::Color::Green);
        window.draw(important_message);
    }

#ifdef DEBUG
    for (auto &point : debug_points) {
        debug_draw_point(point);
    }
#endif  // DEBUG
}

StackOfItems::operator bool() const {
    if (size)
        assert((item != nullptr) && "Welp, ur fuct. StackOfItems has nullptr item when size > 0");
    return size;
}

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

void StackOfItems::remove_items(size_t amount) {
    amount = std::min(amount, size);
    size -= amount;
    if (size == 0) {
        item = nullptr;
    }
}

void StackOfItems::use(Actor &target) {
    auto result = item->use(target);
    if (result.was_broken) {
        remove_items(1);
    }
}

DeepCopyCls(StackOfItems) {
    if (item) {
        assert(size != 0 && "Welp, ur fuct. StackOfItems is not nullptr item when size = 0");
        other.item = item->deepcopy_item();
        assert(other.item && "Welp, ur fuct. deepcopy_item produces nullptr");
    } else {
        assert(size == 0 && "Welp, ur fuct. StackOfItems has nullptr item when size != 0");
        other.item = nullptr;
    }
    other.size = size;
}

StackOfItems *Inventory::get_slot(size_t index) {
    if (0 <= index && index <= slots.size()) {
        if (slots[index]) {
            return &slots[index];
        }
    }
    return nullptr;
}

void Inventory::use_item(size_t index, Actor &target) {
    StackOfItems *slot = get_slot(index);
    if (!slot) return;
    slot->use(target);
}

// void Inventory::recalculate_selection() {
//     if (0 <= selection && selection <= slots.size()) {
//         if (slots[selection]) {
//             return;
//         }
//     }

//     for (size_t i = 0; i < slots.size(); ++i) {
//         if (slots[i]) {
//             selection = i;
//             return;
//         }
//     }

//     selection = 0;
// }

bool Inventory::add_item(std::shared_ptr<Item> item) {
    // first check non empty slots to see if we can stack the item
    for (auto &slot : slots) {
        if (slot && slot.add_item(item)) {
            return true;
        }
    }

    // and only if we cannot stack the item, we should use one more slot
    for (auto &slot : slots) {
        if (!slot && slot.add_item(item)) {
            return true;
        }
    }

    return false;
}

DeepCopyCls(Inventory) {
    other.max_size = max_size;
    other.slots.resize(slots.size());
    for (size_t i = 0; i < slots.size(); ++i) {
        slots[i].deepcopy_to(other.slots[i]);
    }
    other.selection = selection;
}

float calculate_inventory_item_x(float length, size_t count, size_t i) {
    return (0.5f + (float)i) * length / (float)count;
}

void HolderOfItemsView::init() { stack_of_items_view.init(); }

void HolderOfItemsView::draw(const StackOfItems *slots, size_t count, size_t selection) {
    sf::View view = Game::get().game_view.view;

    float actual_size = item_size / Game::world_size;

    float x_base = view.getCenter().x + (1 - (float)count) * actual_size / 2;
    float y = view.getCenter().y + view.getSize().y / 2 - actual_size / 2;

    for (size_t i = 0; i < count; ++i) {
        sf::Vector2f position(x_base + actual_size * i, y);
        stack_of_items_view.draw(slots[i], position, item_size - 3, i == selection);
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

void Dungeon::update_enemies_in_thread(float delta_time, size_t id) {
    if (current_level) {
        current_level->update_enemies_in_thread(delta_time, id);
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
    return sf::Vector2f(tiles.row_count(), tiles.column_count()) *
           tile_coords_to_world_coords_factor() / 2.0f;
}

boost::optional<std::pair<size_t, size_t>> DungeonLevel::get_tile_coordinates(sf::Vector2f position
) const {
    sf::Vector2f world_coords = position / tile_coords_to_world_coords_factor();
    if (position.x < 0 || position.x >= tiles.row_count() || position.y < 0 ||
        position.y >= tiles.column_count())
        return boost::none;
    return std::make_pair<size_t, size_t>(position.x, position.y);
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
    RangeOfLong range_chest_level(0, (max_chest_level + 1) * (max_chest_level + 1) - 1);
    RangeOfLong range_chest_item(0, Game::get().item_templates.size() - 1);

    for (size_t i = 0; i < tiles.row_count(); ++i) {
        for (size_t j = 0; j < tiles.column_count(); ++j) {
            if (i == 0 || i == tiles.row_count() - 1 || j == 0 || j == tiles.column_count() - 1)
                tiles[i][j].kind = Tile::Barrier;
            else
                tiles[i][j].kind = Tile::Flor;

            if (tiles[i][j].kind == Tile::Flor && range_chest_spawn.get_random() == 0) {
                size_t level = max_chest_level - (size_t)std::sqrt(range_chest_level.get_random());
                auto chest = std::make_shared<Chest>(level);
                chest->inventory.add_item(Game::get().make_item(range_chest_item.get_random()));
                tiles[i][j].set_building(chest);
            }
        }
    }

    RangeOfLong range_x(0, tiles.row_count() - 1);
    RangeOfLong range_y(0, tiles.column_count() - 1);

    long x1 = range_x.get_random();
    long y1 = range_y.get_random();
    tiles[x1][y1].kind = Tile::UpLaddor;
    initial_player_position = sf::Vector2f(x1, y1) * tile_coords_to_world_coords_factor();

    long x2 = range_x.get_random();
    long y2 = range_y.get_random();
    if (x1 == x2) {
        if (x2 == 0) {
            x2 += 1;
        } else {
            x2 -= 1;
        }
    }
    if (y1 == y2) {
        if (y2 == 0) {
            y2 += 1;
        } else {
            y2 -= 1;
        }
    }
    tiles[x2][y2].kind = Tile::DownLaddor;

    // tiles[4][4].kind = Tile::OpenDor;
    // tiles[4][5].kind = Tile::ClosedDor;
}

void DungeonLevel::regenerate_enemies() {
    enemies.clear();

    RangeOfFloat range_x(2, tiles.row_count() - 2);
    RangeOfFloat range_y(2, tiles.column_count() - 2);

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

    RangeOfFloat range_x(2, tiles.row_count() - 2);
    RangeOfFloat range_y(2, tiles.column_count() - 2);

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
    Game::get().enemy_threads.delta_time = delta_time;
    Game::get().enemy_threads.start_updates();
    Game::get().enemy_threads.join_updates();

    Game::get().dungeon.player.update(delta_time);

    for (auto &enemy : enemies) {
        enemy.apply_friction();
    }
    Game::get().dungeon.player.apply_friction();

    delete_dead_actors();
    delete_picked_up_items();
}

void DungeonLevel::update_enemies_in_thread(float delta_time, size_t id) {
    size_t start = id * enemies.size() / thread_count;
    size_t end = (id + 1) * enemies.size() / thread_count;

    for (size_t i = start; i < end; ++i) {
        enemies[i].update(delta_time);
    }
}

void DungeonLevel::fixed_update(float delta_time) {
    for (auto &enemy : enemies) {
        enemy.fixed_update(delta_time);
    }
    Game::get().dungeon.player.fixed_update(delta_time);

    handle_collitions();
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

    float right_wall = tiles.row_count() * tile_coords_to_world_coords_factor();
    float down_wall = tiles.column_count() * tile_coords_to_world_coords_factor();

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
    if (!up_laddor_tile_texture.loadFromFile(path_to_resources + up_laddor_tile_name)) return false;
    if (!down_laddor_tile_texture.loadFromFile(path_to_resources + down_laddor_tile_name))
        return false;

    setup_sprite(flor_tile_texture, flor_tile_sprite);
    flor_tile_sprite.setOrigin({0, 0});

    setup_sprite(open_dor_tile_texture, open_dor_tile_sprite);
    open_dor_tile_sprite.setOrigin({0, 0});

    setup_sprite(closed_dor_tile_texture, closed_dor_tile_sprite);
    closed_dor_tile_sprite.setOrigin({0, 0});

    setup_sprite(up_laddor_tile_texture, up_laddor_tile_sprite);
    up_laddor_tile_sprite.setOrigin({0, 0});

    setup_sprite(down_laddor_tile_texture, down_laddor_tile_sprite);
    down_laddor_tile_sprite.setOrigin({0, 0});

    setup_sprite(barrier_tile_texture, barrier_tile_sprite);
    barrier_tile_sprite.setOrigin({0, 0});

    setup_sprite(chest_texture, chest_sprite);
    chest_sprite.setOrigin({0, 0});

    return true;
}

void DungeonLevelView::draw(const DungeonLevel &level) {
    sf::FloatRect view_rect = Game::get().game_view.get_display_rect();

    sf::Vector2f start_of_view_f =
        (view_rect.getPosition() - tile_border_size) / level.tile_coords_to_world_coords_factor();
    sf::Vector2f end_of_view_f = start_of_view_f + (view_rect.getSize() + 2 * tile_border_size) /
                                                       level.tile_coords_to_world_coords_factor();

    start_of_view_f = max({0, 0}, start_of_view_f);
    end_of_view_f =
        min(sf::Vector2f(level.tiles.row_count(), level.tiles.column_count()), end_of_view_f);

    sf::Vector2f start_of_view(start_of_view_f);
    sf::Vector2f end_of_view(end_of_view_f);

    for (size_t i = start_of_view.x; i < end_of_view.x; ++i) {
        auto &row = level.tiles[i];
        for (size_t j = start_of_view.y; j < end_of_view.y; ++j) {
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
    if (tile.kind == Tile::Barrier) {
        sprite = barrier_tile_sprite;
    } else if (tile.kind == Tile::Flor) {
        sprite = flor_tile_sprite;
    } else if (tile.kind == Tile::OpenDor) {
        sprite = open_dor_tile_sprite;
    } else if (tile.kind == Tile::ClosedDor) {
        sprite = closed_dor_tile_sprite;
    } else if (tile.kind == Tile::UpLaddor) {
        sprite = up_laddor_tile_sprite;
    } else if (tile.kind == Tile::DownLaddor) {
        sprite = down_laddor_tile_sprite;
    }

    sf::Vector2f saved = sprite.getScale();
    sprite.setScale(saved * factor);
    sprite.setPosition(position * factor);
    Game::get().game_view.draw_culled(sprite);
    sprite.setScale(saved);

    if (tile.building) {
        saved = chest_sprite.getScale();
        chest_sprite.setScale(saved * factor * chest_size_factor);
        chest_sprite.setPosition(position * factor);
        Game::get().game_view.draw_culled(chest_sprite);
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
    Game::get().game_view.draw_culled(sprite);
    sprite.setScale(saved);

    if (actor.equipment.weapon()) items_view.draw(*(actor.equipment.weapon().item), actor.position);
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
    Game::get().game_view.draw_culled(max_bar);

    cur_bar.setSize({max_bar.getSize().x * ratio, max_bar.getSize().y});
    cur_bar.setPosition(max_bar.getPosition());
    cur_bar.setFillColor(cur_bar_color);
    Game::get().game_view.draw_culled(cur_bar);
}

void ItemsView::draw(const Item &item, sf::Vector2f position) {
    auto &cls = item.get_class();
    sf::Sprite &sprite = cls.sprite;
    sf::Vector2f saved = sprite.getScale();
    sprite.setScale(saved * cls.size / Game::world_size);
    sprite.setPosition(position);
    Game::get().game_view.draw_culled(sprite);
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
    sf::Sprite sprite;
    if (stack.size != 0) {
        auto &cls = stack.item->get_class();
        sprite = cls.sprite;
    }

    sf::Vector2f saved = sprite.getScale();

    sprite.setScale(saved * size / Game::world_size);
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
        count_text.setScale(saved * size * text_ratio / Game::world_size);
        window.draw(count_text);
    }
}

void Dungeon::add_level(const DungeonLevel &level) { all_levels.push_back(level); }

bool Dungeon::load_level(size_t index) {
    if (index < 0 || index >= all_levels.size()) {
        return false;
    }
    current_level = all_levels[index];
    current_level_index = index;
    on_load_level(*current_level);
    return true;
}

void Dungeon::on_load_level(DungeonLevel &level) {
    Game::get().player_template.deepcopy_to(player);
    player.position = level.initial_player_position;
}

void Dungeon::unload_current_level() {
    current_level = boost::none;
    current_level_index = -1;
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
    RangeOfLong range(0, 1 + 2 * level);
    if ((float)range.get_random() / source.characteristics.luck <= 1) {
        return {true, false};
    }
    return {false, (float)range.get_random() * source.characteristics.luck <= 1};
}

void CharacteristicsModifier::apply(Characteristics &value) {
    if (max_health) {
        value.max_health = boost::apply_visitor(
            [&](auto &&v) -> auto { return v.apply(value.max_health); }, *max_health
        );
    }

    if (defence) {
        value.defence = boost::apply_visitor(
            [&](auto &&v) -> auto { return v.apply(value.defence); }, *defence
        );
    }

    if (speed) {
        value.speed =
            boost::apply_visitor([&](auto &&v) -> auto { return v.apply(value.speed); }, *speed);
    }
}

float Enchantment::apply(float value, const Actor &target) const {
    if (target.actor_class_index == target_actor_class_index) {
        return value * damage_multiplier;
    }
    return value;
}

StackOfItems &Equipment::weapon() { return slots[Wearable::Count]; }

const StackOfItems &Equipment::weapon() const { return slots[Wearable::Count]; }

Equipment::Wearables &Equipment::wearables() { return *reinterpret_cast<Wearables *>(&slots); }

const Equipment::Wearables &Equipment::wearables() const {
    return *reinterpret_cast<const Wearables *>(&slots);
}

bool Equipment::equip_wearable(std::shared_ptr<Item> item) {
    Wearable &as_wearable = dynamic_cast<Wearable &>(*item);

    auto &it = wearables()[as_wearable.kind];
    if (!it) {
        it.add_item(item);
        return true;
    }
    return false;
}

bool Equipment::equip_weapon(std::shared_ptr<Item> item) {
    if (!weapon()) {
        weapon().add_item(item);
        return true;
    }
    return false;
}

StackOfItems *Equipment::get_slot(size_t index) {
    if (0 <= index && index <= slots.size()) {
        if (slots[index]) {
            return &slots[index];
        }
    }
    return nullptr;
}

DeepCopyCls(Equipment) {
    for (size_t i = 0; i < slots.size(); ++i) {
        slots[i].deepcopy_to(other.slots[i]);
    }
    other.selection = selection;
}

bool Actor::ready_to_be_deleted() const {
    return !alive &&
           (!is_moving() || since_last_taken_damage.getElapsedTime() > ready_to_be_deleted_after);
}

void Actor::recalculate_characteristics() {
    characteristics = base_characteristics;
    for (auto &it : equipment.slots) {
        if (!it) continue;
        it.item->update_owner_characteristics(characteristics);
    }
}

DeepCopyCls(Actor) {
    RigidBody::deepcopy_to(other);
    other.actor_class_index = actor_class_index;
    other.health = health;
    equipment.deepcopy_to(other.equipment);
    other.experience = experience;
    other.characteristics = characteristics;
    other.base_characteristics = base_characteristics;
    other.alive = alive;
}

float Actor::calculate_defence() {
    float defence = characteristics.defence;
    for (auto &it : equipment.wearables()) {
        if (!it) continue;
        defence += it.item->generate_defence();
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

DeepCopyCls(RigidBody) {
    other.position = position;
    other.pushable = pushable;
    other.size = size;
    other.mut = std::make_shared<std::mutex>();
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

DeepCopyCls(Player) {
    Actor::deepcopy_to(other);
    inventory.deepcopy_to(other.inventory);
}

void Player::fixed_update(float delta_time) {
    RigidBody::fixed_update(delta_time);

    if (!alive) return;

    handle_movement(delta_time);
}

void Player::update(float delta_time) {
    if (!alive) return;

    handle_equipment_use();
    handle_slot_selection();
    handle_inventory_use();
    handle_picking_up_items();
    handle_throwing_items();
    handle_level_transition();
}

void Player::handle_level_transition() {
    if (!Game::get().keys_pressed_on_this_frame[sf::Keyboard::R]) return;

    auto &level = Game::get().dungeon.current_level;
    if (!level) return;

    auto coords = level->get_tile_coordinates(position);
    if (!coords) return;

    Tile &tile = level->tiles[coords->first][coords->second];

    // if (tile.kind == Tile::UpLaddor) {
    //     if (Game::get().dungeon.current_level_index > 1) {
    //         Game::get().dungeon.load_level(Game::get().dungeon.current_level_index - 1);
    //     }
    // } else
    if (tile.kind == Tile::DownLaddor) {
        if (Game::get().dungeon.current_level_index < Game::get().dungeon.all_levels.size() - 1) {
            Game::get().dungeon.load_level(Game::get().dungeon.current_level_index + 1);
        } else {
            Game::get().have_won = true;
        }
    }
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

    if (equipment.weapon()) {
        equipment.weapon().use(*this);
    }
}

void Player::handle_slot_selection() {
    if (Game::get().keys_pressed_on_this_frame[sf::Keyboard::Q]) {
        Game::get().is_inventory_selected = !Game::get().is_inventory_selected;
    }

    std::vector<sf::Keyboard::Key> keys = {
        sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3, sf::Keyboard::Num4,
        sf::Keyboard::Num5, sf::Keyboard::Num6, sf::Keyboard::Num7, sf::Keyboard::Num8,
        sf::Keyboard::Num9, sf::Keyboard::Num0,
    };
    for (size_t i = 0; i < keys.size(); ++i) {
        if (sf::Keyboard::isKeyPressed(keys[i])) {
            if (Game::get().is_inventory_selected) {
                inventory.selection = i;
            } else {
                equipment.selection = i;
            }
        }
    }
}

void Player::handle_throwing_items() {
    if (Game::get().keys_pressed_on_this_frame[sf::Keyboard::T]) {
        StackOfItems *slot;
        if (Game::get().is_inventory_selected) {
            slot = inventory.get_slot(inventory.selection);
        } else {
            slot = equipment.get_slot(equipment.selection);
        }
        if (slot && *slot) {
            throw_out_item(slot->item);
            slot->remove_items(1);
            recalculate_characteristics();
        }
    }
}

void Player::handle_inventory_use() {
    if (!Game::get().keys_pressed_on_this_frame[sf::Keyboard::F]) return;

    inventory.use_item(inventory.selection, *this);
}

void Player::throw_out_item(std::shared_ptr<Item> item) const {
    float angle = RangeOfFloat(0, 2 * PI).get_random();
    float len = pick_up_range * 1.5;
    sf::Vector2f item_position = position + sf::Vector2f(std::cos(angle), std::sin(angle)) * len;
    Game::get().dungeon.current_level->laying_items.push_back(LayingItem(item, item_position));
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

    std::vector<LayingItem> &laying_items = Game::get().dungeon.current_level->laying_items;

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
        equipment.weapon().use(*this);
    }
}

void Enemy::die(Actor &reason) {
    if (!alive) return;

    alive = false;
    reason.experience.gain(experience.as_value_after_death());
}

void Enemy::on_deletion() {
    auto &level = Game::get().dungeon.current_level;
    if (!level) return;

    RangeOfLong range_chest_item(0, Game::get().item_templates.size() - 1);
    std::shared_ptr<Item> item = Game::get().make_item(range_chest_item.get_random());

    LayingItem laying_item(item, position);
    level->laying_items.push_back(laying_item);
}

DeepCopyCls(Enemy) { Actor::deepcopy_to(other); }

void Item::update_owner_characteristics(Characteristics &characteristics) {
    auto &artefact = get_class().artefact;
    if (!artefact) return;
    artefact->apply(characteristics);
}

DeepCopyCls(Item) { other.item_class_index = item_class_index; }

ItemClass &Item::get_class() const { return Game::get().item_classes[item_class_index]; }

void Potion::apply(Actor &target) { modifier.apply(target.characteristics); }

ItemUseResult Potion::use(Actor &target) {
    apply(target);
    return ItemUseResult();
}

DeepCopyCls(Weapon) {
    Item::deepcopy_to(other);
    other.enchantment = enchantment;
    other.damage_range = damage_range;
}

float Weapon::get_damage(Actor &target) { return damage_range.get_random(); }

DeepCopyCls(WeaponWithCooldown) {
    Weapon::deepcopy_to(other);
    other.since_last_use = since_last_use;
    other.cooldown_time = cooldown_time;
    other.on_cooldown = on_cooldown;
}

bool MeleeWeapon::try_to_attack(Actor &source, Actor &target) {
    std::unique_lock<std::mutex> lck(*target.mut);

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
        for (auto &enemy : Game::get().dungeon.current_level->enemies) {
            reached_anything = try_to_attack(source, enemy) || reached_anything;
        }
    } else {
        reached_anything = try_to_attack(source, Game::get().dungeon.player) || reached_anything;
    }

    if (reached_anything) ensure_cooldown();
    return ItemUseResult();
}

DeepCopyCls(MeleeWeapon) {
    WeaponWithCooldown::deepcopy_to(other);
    other.push_back_force_multiplier = push_back_force_multiplier;
}

float Wearable::generate_defence() { return defence_range.get_random(); }

DeepCopyCls(Wearable) {
    Item::deepcopy_to(other);
    other.kind = kind;
    other.defence_range = defence_range;
}
