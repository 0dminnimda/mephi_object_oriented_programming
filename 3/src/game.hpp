#pragma once

#include "SFML/Graphics/Sprite.hpp"
#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
#include <array>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "deepcopy.hpp"
#include "matrix.hpp"

#ifdef SFML_SYSTEM_IOS
const std::string path_to_resources = "";
#else
const std::string path_to_resources = "resources/";
#endif

sf::Color set_alpha(sf::Color color, sf::Uint8 transparency);

class ProgressBarView {
    sf::RenderWindow &window;

public:
    float height_factor;

    sf::Color max_bar_color;
    sf::Color cur_bar_color;

    sf::RectangleShape max_bar;
    sf::RectangleShape cur_bar;

    ProgressBarView(
        sf::RenderWindow &window, sf::Color max_bar_color, sf::Color cur_bar_color,
        float height_factor = 0.1f
    )
        : window(window),
          height_factor(height_factor),
          max_bar_color(max_bar_color),
          cur_bar_color(cur_bar_color) {}

    void draw(sf::Vector2f position, float bar_width, float ratio);
};

template <typename T>
class SetAbsoluteValue {
public:
    T value;

    T apply(T) { return this->value; }
};

template <typename T>
class AddToValue {
public:
    T value;

    T apply(T value) { return value + this->value; }
};

template <typename T>
using ValueModifier = std::variant<SetAbsoluteValue<T>, AddToValue<T>>;

class Characteristics {
public:
    float max_health;
    float defence;
    float speed;
    float luck = 1;
};

class CharacteristicsModifier {
public:
    std::optional<ValueModifier<float>> max_health;
    std::optional<ValueModifier<float>> defence;
    std::optional<ValueModifier<float>> speed;
    std::optional<ValueModifier<float>> luck;

    void apply(Characteristics &value);
};

class Actor;
class ItemClass;

class ItemUseResult {
public:
    bool was_broken = false;
};

class Item {
public:
    DeepCopy(Item);

    enum Kind { Weapon, Wearable, Custom, Count };

    size_t item_class_index;

    Item() {}
    Item(size_t item_class_index) : item_class_index(item_class_index) {}

    virtual ~Item() = default;

    virtual std::shared_ptr<Item> deepcopy_item() const = 0;

    virtual ItemUseResult use(Actor &target) { return ItemUseResult(false); }
    virtual void update_owner_characteristics(Characteristics &characteristics);
    virtual float generate_defence() { return 0; }

    ItemClass &get_class() const;
};

class ItemClass {
public:
    std::string name;
    std::string description;
    std::string texture_name;
    sf::Texture texture;
    sf::Sprite sprite;
    float size;
    Item::Kind kind;
    std::optional<CharacteristicsModifier> artefact;
    size_t max_stack_size = 1;

    ItemClass(
        std::string name, std::string description, std::string texture_name, float size,
        Item::Kind kind
    )
        : name(name),
          description(description),
          texture_name(texture_name),
          size(size),
          kind(kind) {}
    ~ItemClass() = default;

    bool init();
};

class ItemsView {
private:
    sf::RenderWindow &window;

public:
    ItemsView(sf::RenderWindow &window) : window(window) {}
    ~ItemsView() = default;

    void draw(const Item &item, sf::Vector2f position);
};

class Potion : public Item {
    CharacteristicsModifier modifier;

public:
    Potion(size_t item_class_index, CharacteristicsModifier modifier)
        : Item(item_class_index), modifier(modifier) {}

    ItemUseResult use(Actor &target) override;
    void apply(Actor &target);
};

template <typename T>
class RangeOfValues {
public:
    T min;
    T max;

    RangeOfValues(T min, T max) : min(min), max(max) {}

    T get_random() {
        std::random_device rd;
        std::mt19937 gen(rd());
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dis(min, max);
            return dis(gen);
        } else {
            std::uniform_real_distribution<T> dis(min, max);
            return dis(gen);
        }
    }
};

using RangeOfLong = RangeOfValues<long>;
using RangeOfFloat = RangeOfValues<float>;

class Enchantment {
    size_t target_actor_class_index;
    float damage_multiplier;

public:
    float apply(float value, const Actor &target) const;
};

class Weapon : public Item {
protected:
    std::optional<Enchantment> enchantment;
    RangeOfLong damage_range;

public:
    DeepCopy(Weapon);

    Weapon(size_t item_class_index, RangeOfLong damage_range)
        : Item(item_class_index), damage_range(damage_range) {}

    virtual bool try_to_attack(Actor &source, Actor &target) = 0;
    virtual float get_damage(Actor &target);
    virtual bool is_in_range(const Actor &source, sf::Vector2f target) const = 0;
};

class WeaponWithCooldown : public Weapon {
public:
    DeepCopy(WeaponWithCooldown);

    sf::Clock since_last_use;
    sf::Time cooldown_time;
    bool on_cooldown = false;

    WeaponWithCooldown(size_t item_class_index, RangeOfLong damage_range, sf::Time cooldown_time)
        : Weapon(item_class_index, damage_range), cooldown_time(cooldown_time) {}

    bool test_cooldown();
    void ensure_cooldown();
};

class MeleeWeapon : public WeaponWithCooldown {
public:
    DeepCopy(MeleeWeapon);

    float push_back_force_multiplier;

    MeleeWeapon(
        size_t item_class_index, RangeOfLong damage_range, float push_back_force_multiplier,
        sf::Time cooldown_time
    )
        : WeaponWithCooldown(item_class_index, damage_range, cooldown_time),
          push_back_force_multiplier(push_back_force_multiplier) {}

    ItemUseResult use(Actor &source) override;
    bool test_cooldown();
    void ensure_cooldown();
    bool try_to_attack(Actor &source, Actor &target) override;
};

class Hammer : public MeleeWeapon {
public:
    DeepCopy(Hammer);

    float hit_range;

    Hammer(
        size_t item_class_index, RangeOfLong damage_range, float hit_range,
        float push_back_force_multiplier, sf::Time cooldown_time
    )
        : MeleeWeapon(item_class_index, damage_range, push_back_force_multiplier, cooldown_time),
          hit_range(hit_range) {}

    std::shared_ptr<Item> deepcopy_item() const override;

    bool is_in_range(const Actor &source, sf::Vector2f target) const override;
};

class Sword : public MeleeWeapon {
public:
    DeepCopy(Sword);

    // TODO: add direction dependant range
    float hit_range;

    Sword(
        size_t item_class_index, RangeOfLong damage_range, float hit_range,
        float push_back_force_multiplier, sf::Time cooldown_time
    )
        : MeleeWeapon(item_class_index, damage_range, push_back_force_multiplier, cooldown_time),
          hit_range(hit_range) {}

    std::shared_ptr<Item> deepcopy_item() const override;

    bool is_in_range(const Actor &source, sf::Vector2f target) const override;
};

// aka armour or equipment
class Wearable : public Item {
public:
    DeepCopy(Wearable);

    enum Kind {
        Helmet = 0,
        ChestPlate,
        Leggins,
        Boots,
        Shield,
        Amulet,
        Count,
    };

    Kind kind;
    RangeOfLong defence_range;

    Wearable(size_t item_class_index, Kind kind, RangeOfLong defence_range)
        : Item(item_class_index), kind(kind), defence_range(defence_range) {}

    float generate_defence() override;
};

class Shield : public Wearable {
public:
    DeepCopy(Shield);

    Shield(size_t item_class_index, RangeOfLong defence_range)
        : Wearable(item_class_index, Wearable::Shield, defence_range) {}

    std::shared_ptr<Item> deepcopy_item() const override;
};

class LockPick : public Item {
public:
    DeepCopy(LockPick);

    LockPick() = default;
    LockPick(size_t item_class_index) : Item(item_class_index) {}

    ItemUseResult use(Actor &target) override;
    std::shared_ptr<Item> deepcopy_item() const override;
};

struct LockPickingResult {
public:
    bool lock_picked;
    bool pick_broken;
};

class StackOfItems {
public:
    DeepCopy(StackOfItems);

    std::shared_ptr<Item> item = nullptr;
    size_t size = 0;

    operator bool() const;
    bool add_item(std::shared_ptr<Item> item);
    void remove_items(size_t amount);
    void use(Actor &target);
};

class Inventory {
public:
    DeepCopy(Inventory);

    size_t max_size;
    std::vector<StackOfItems> slots;
    size_t selection = 0;

    Inventory() : Inventory(10) {}
    Inventory(size_t max_size) : max_size(max_size), slots(max_size) {}

    // void recalculate_selection();
    bool add_item(std::shared_ptr<Item> item);
    void use_item(size_t index, Actor &target);
    StackOfItems *get_slot(size_t index);
};

class StackOfItemsView {
    sf::RenderWindow &window;

    float text_ratio = 2.0f;
    float selection_thickness = 0.3f;
    sf::Color selection_color = sf::Color::Yellow;
    sf::Color background_color = set_alpha(sf::Color::White, 127);

    sf::Text count_text;

public:
    StackOfItemsView(sf::RenderWindow &window) : window(window) {}

    void init();
    void draw(const StackOfItems &stack, sf::Vector2f position, float size, bool selected);
};

class HolderOfItemsView {
    sf::RenderWindow &window;

    StackOfItemsView stack_of_items_view;

    float item_size = 10.0f;

public:
    HolderOfItemsView(sf::RenderWindow &window) : window(window), stack_of_items_view(window) {}

    void init();
    void draw(const StackOfItems *slots, size_t count, size_t selection);
};

class Chest {
public:
    Inventory inventory;
    const size_t level;

    explicit Chest(size_t level) : inventory(1), level(level) {}

    LockPickingResult simulate_picking(const Actor &source);
};

class Tile {
public:
    enum Kind {
        Barrier,
        Flor,
        OpenDor,
        ClosedDor,
        UpLaddor,
        DownLaddor,
        Count,
    };

    std::shared_ptr<Chest> building;

    Kind kind;

    explicit Tile() : Tile(Barrier) {}
    explicit Tile(Kind kind) : building(nullptr), kind(kind) {}

    Tile &set_building(std::shared_ptr<Chest> building);
};

class Experience {
public:
    size_t level;
    size_t value;

    Experience() = default;
    Experience(size_t level) : level(level), value(0) {}

    void gain(size_t amount);
    static size_t needs_exp_for_level(size_t level);
    size_t as_value_after_death();
    void level_up();
};

class ExperienceView {
    sf::RenderWindow &window;

public:
    float relative_to_screen_width = 0.3f;
    float text_ratio = 0.1f;

    ProgressBarView till_next_level_bar;

    sf::Text level_text;

    ExperienceView(sf::RenderWindow &window)
        : window(window),
          till_next_level_bar(window, sf::Color::White, sf::Color(0xFF, 0xA5, 0x00)) {}

    void init();
    void draw(const Experience &experience);
};

class LevelUpCanvas {
    sf::RenderWindow &window;

public:
    LevelUpCanvas(sf::RenderWindow &window) : window(window) {}

    void on_level_up(Experience &exp);
    void draw();
};

class Equipment {
public:
    DeepCopy(Equipment);

    // Wearable::Count (wearables) + 1 (weapon)
    using Slots = std::array<StackOfItems, Wearable::Count + 1>;
    using Wearables = std::array<StackOfItems, Wearable::Count>;
    Slots slots;
    size_t selection = 0;

    Equipment() : slots({0}) {}

    StackOfItems &weapon();
    const StackOfItems &weapon() const;
    Wearables &wearables();
    const Wearables &wearables() const;

    bool equip_wearable(std::shared_ptr<Item> item);
    bool equip_weapon(std::shared_ptr<Item> weapon);
    StackOfItems *get_slot(size_t index);
};

class RigidBody {
public:
    DeepCopy(RigidBody);

    float size = 1.0f;
    float mass = 1.0f;
    float friction_coefficient = 2.0f;  // mu * gravity

    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;

    bool pushable = true;

    RigidBody() = default;
    RigidBody(float size, float mass) : size(size), mass(mass) {}
    RigidBody(sf::Vector2f position) : position(position) {}
    RigidBody(sf::Vector2f position, float size, float mass)
        : size(size), mass(mass), position(position) {}

    void apply_force(sf::Vector2f forece);
    void apply_impulse(sf::Vector2f impulse);
    void apply_friction();
    virtual void fixed_update(float delta_time);

    void move(sf::Vector2f direction, float speed, float delta_time);
    bool is_moving(float epsilon = 0.001f) const;

    sf::FloatRect get_axes_aligned_bounding_box() const;
};

sf::Vector2f center(const sf::FloatRect &a);

class ActorClass {
public:
    std::string name;
    std::string description;
    std::string texture_name;
    sf::Texture texture;
    sf::Sprite sprite;

    ActorClass(std::string name, std::string description, std::string texture_name)
        : name(name), description(description), texture_name(texture_name) {}
    ~ActorClass() = default;

    bool init();
};

static const sf::Time ready_to_be_deleted_after = sf::seconds(2.0f);

class Actor : public RigidBody {
public:
    DeepCopy(Actor);

    size_t actor_class_index;
    float health;
    Equipment equipment;
    Experience experience;
    Characteristics base_characteristics;
    Characteristics characteristics;
    bool alive = true;
    sf::Clock since_last_taken_damage;

    Actor() = default;
    Actor(size_t class_index, float size, Characteristics characteristics, size_t level)
        : RigidBody(size, size),
          actor_class_index(class_index),
          health(characteristics.max_health),
          characteristics(characteristics),
          base_characteristics(characteristics),
          experience(level) {}
    virtual ~Actor() = default;

    virtual void init(){};
    virtual void update(float delta_time){};
    virtual void die(Actor &reason){};
    virtual bool pick_up_item(std::shared_ptr<Item> item) { return false; };
    virtual void on_deletion(){};
    virtual void recalculate_characteristics();

    void take_damage(float amount, Actor &source);
    float calculate_defence();
    bool ready_to_be_deleted() const;
    ActorClass &get_class() const;
};

float symmetric_linear_easing(float t, float p);

class SpriteColorAnimator {
public:
    sf::Color inactive_color;
    sf::Color active_color;

    sf::Time duration;

    void update(sf::Time elapsed_time, sf::Sprite &sprite);
};

class ActorsView {
private:
    sf::RenderWindow &window;

    float death_color_multiplier = 0.4f;

    ProgressBarView health_bar;

    SpriteColorAnimator taked_damage_animator =
        SpriteColorAnimator(sf::Color::White, sf::Color(200, 0, 0), sf::seconds(0.15f));

public:
    ItemsView items_view;

    ActorsView(sf::RenderWindow &window)
        : window(window),
          health_bar(window, set_alpha(sf::Color::White, 127), set_alpha(sf::Color::Green, 127)),
          items_view(window) {}

    void draw(const Actor &actor);
    void draw_ui(const Actor &actor);
};

static const sf::Time pick_up_timeout = sf::seconds(1.0f);

class Player : public Actor {
public:
    DeepCopy(Player);

    Inventory inventory;
    static constexpr float pick_up_range = 1.2f;

    Player() = default;
    Player(size_t class_index, float size, Characteristics characteristics, size_t level)
        : Actor(class_index, size, characteristics, level) {}
    Player(const Actor &actor) : Actor(actor) {}

    void init() override;
    void fixed_update(float delta_time) override;
    void update(float delta_time) override;
    void handle_movement(float delta_time);
    void handle_equipment_use();
    void handle_inventory_use();
    void handle_slot_selection();
    void handle_picking_up_items();
    void handle_throwing_items();
    void die(Actor &reason) override;
    bool pick_up_item(std::shared_ptr<Item> item) override;
    void throw_out_item(std::shared_ptr<Item> item) const;
    void recalculate_characteristics() override;
};

class Enemy : public Actor {
public:
    DeepCopy(Enemy);

    Enemy() = default;
    Enemy(size_t class_index, float size, Characteristics characteristics, size_t level)
        : Actor(class_index, size, characteristics, level) {}
    Enemy(const Actor &actor) : Actor(actor) {}

    void init() override;
    void fixed_update(float delta_time) override;
    void update(float delta_time) override;
    void handle_movement(float delta_time);
    void handle_equipment_use();
    void die(Actor &reason) override;
    void on_deletion() override;
    bool pick_up_item(std::shared_ptr<Item> item) override;
};

class LayingItem : public RigidBody {
public:
    std::shared_ptr<Item> item;
    bool picked_up = false;
    sf::Clock since_last_pick_up;

    LayingItem(std::shared_ptr<Item> item) : RigidBody(), item(item) {}
    LayingItem(std::shared_ptr<Item> item, sf::Vector2f position)
        : RigidBody(position), item(item) {}
};

class DungeonLevel {
public:
    std::vector<Enemy> enemies;
    std::vector<LayingItem> laying_items;
    Matrix<Tile> tiles;
    sf::Vector2f initial_player_position;
    float tile_size = 10.0f;
    float chest_size_factor = 1.0f;
    size_t actors_spawned_per_class = 50;
    size_t laying_items_spawned_per_class = 5;
    float rebounce_factor = 0.8f;

    void init();
    float tile_coords_to_world_coords_factor() const;
    sf::Vector2f center() const;
    void resize_tiles(size_t width, size_t height);
    void regenerate();
    void regenerate_tiles();
    void regenerate_enemies();
    void regenerate_laying_items();
    std::optional<std::pair<size_t, size_t>> get_tile_coordinates(sf::Vector2f position) const;
    Tile *get_tile(sf::Vector2f position);
    void add_laying_item(std::unique_ptr<LayingItem> item);
    void update(float delta_time);
    void fixed_update(float delta_time);
    void handle_collitions();
    void handle_actor_actor_collitions(std::vector<RigidBody *> &bodies);
    void handle_rigid_body_level_collitions(std::vector<RigidBody *> &bodies);
    void delete_dead_actors();
    void delete_picked_up_items();
};

class DungeonLevelView {
private:
    sf::RenderWindow &window;

    sf::Texture flor_tile_texture;
    sf::Sprite flor_tile_sprite;

    sf::Texture open_dor_tile_texture;
    sf::Sprite open_dor_tile_sprite;

    sf::Texture closed_dor_tile_texture;
    sf::Sprite closed_dor_tile_sprite;

    sf::Texture barrier_tile_texture;
    sf::Sprite barrier_tile_sprite;

    sf::Texture chest_texture;
    sf::Sprite chest_sprite;

public:
    ActorsView actors_view;

    DungeonLevelView(sf::RenderWindow &window) : window(window), actors_view(window) {}
    ~DungeonLevelView() = default;

    bool init();
    void draw(const DungeonLevel &level);

private:
    void draw_tile(const Tile &tile, sf::Vector2f position, float size, float chest_size_factor);
};

class Dungeon {
public:
    std::vector<DungeonLevel> all_levels;
    std::optional<DungeonLevel> current_level;

    Player player;

    void init();
    void update(float delta_time);
    void fixed_update(float delta_time);
    void add_level(const DungeonLevel &level);
    bool load_level(size_t index);
    void on_load_level(DungeonLevel &level);
    void unload_current_level();
    std::optional<DungeonLevel> &get_current_level();
};

class GameView {
public:
    sf::RenderWindow window;
    sf::View view;

#ifdef DEBUG
    std::vector<sf::Vector2f> debug_points;
#endif  // DEBUG

    DungeonLevelView dungeon_level_view;
    HolderOfItemsView holder_of_items_view;
    LevelUpCanvas level_up_canvas;
    ExperienceView experience_view;

    sf::Texture logo_texture;
    sf::Sprite logo;

    sf::Font font;
    sf::Text menu_message;
    sf::Text info_message;
    sf::Text death_message;

    GameView()
        : window(),
          dungeon_level_view(window),
          holder_of_items_view(window),
          level_up_canvas(window),
          experience_view(window) {}
    ~GameView() = default;

    bool init(unsigned int width, unsigned int height);
    void draw();
    bool is_open() const;
    void clear();
    void display();
};

class Game {
private:
    sf::Clock clock;

public:
    GameView game_view;

    Dungeon dungeon;

    bool is_in_game = false;
    bool is_playing() const;

    static constexpr float view_size = 10.0f;   // sets up the view size
    static constexpr float world_size = 10.0f;  // adjusts the sizes of the objects

    std::vector<ActorClass> actor_classes;  // the first entry should be a player class
    static constexpr size_t player_class_index = 0;
    Player player_template;
    std::vector<Enemy> enemy_templates;  // follows the actor_class_index

    std::vector<ItemClass> item_classes;
    std::vector<std::unique_ptr<Item>> item_templates;  // follows the item_classes

    static constexpr float fixed_delta_time = 1.0f / 60.0f;
    float fixed_delta_time_leftover = 0.0f;

    bool is_inventory_selected = true;
    float time_scale = 0.05f;
    float time_scale_epsilon = 1e-6;

    Game() = default;
    ~Game() = default;

    static Game &get();

    bool init(unsigned int width, unsigned int height);
    void update(float delta_time);
    void handle_fixed_update(float delta_time);
    bool run();
    void handle_events();
    void start_playing();
    void stop_playing();

    size_t add_actor_class(const ActorClass &cls);
    long actor_class_index_by_name(const std::string &name);
    Enemy make_enemy(size_t actor_class_index) const;

    size_t add_item_class(const ItemClass &cls);
    long item_class_index_by_name(const std::string &name);
    std::shared_ptr<Item> make_item(size_t item_class_index) const;
};

#endif  // GAME_H
