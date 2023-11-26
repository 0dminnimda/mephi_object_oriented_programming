#pragma once

#include "SFML/Graphics/Sprite.hpp"
#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
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

template <typename T>
class SetAbsoluteValue {
    T value;

public:
    T apply(T) { return this->value; }
};

template <typename T>
class AddToValue {
    T value;

public:
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

class Item {
public:
    DeepCopy(Item);

    enum Kind { Weapon, Wearable, Custom };

    size_t item_class_index;

    Item() {}
    Item(size_t item_class_index) : item_class_index(item_class_index) {}

    virtual ~Item() = default;

    virtual std::shared_ptr<Item> deepcopy_item() const = 0;

    virtual void use(Actor &target){};
    virtual void update_characteristics_as_an_equipment(Characteristics &characteristics){};
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

    void use(Actor &target) override;
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
    std::optional<CharacteristicsModifier> artefact;
    std::optional<Enchantment> enchantment;
    RangeOfLong damage_range;

public:
    DeepCopy(Weapon);

    Weapon(size_t item_class_index, RangeOfLong damage_range)
        : Item(item_class_index), damage_range(damage_range) {}

    void use(Actor &target) override;
    virtual void attack_by(Actor &source) = 0;
    virtual void try_to_attack(Actor &source, Actor &target) = 0;
    virtual float get_damage(Actor &target);
    virtual bool is_in_range(const Actor &source, sf::Vector2f target) const = 0;
};

class Hammer : public Weapon {
public:
    DeepCopy(Hammer);

    float hit_range;
    sf::Clock since_last_use;
    sf::Time cooldown_time;
    bool on_cooldown = false;

    Hammer(
        size_t item_class_index, RangeOfLong damage_range, float hit_range,
        sf::Time cooldown_time = sf::seconds(1.0f)
    )
        : Weapon(item_class_index, damage_range),
          hit_range(hit_range),
          cooldown_time(cooldown_time) {}

    std::shared_ptr<Item> deepcopy_item() const override;

    void attack_by(Actor &source) override;
    bool cooldown();
    void try_to_attack(Actor &source, Actor &target) override;
    bool is_in_range(const Actor &source, sf::Vector2f target) const override;
};

class Sword : public Weapon {
public:
    DeepCopy(Sword);

    float hit_range;
    sf::Clock since_last_use;
    sf::Time cooldown_time;
    bool on_cooldown = false;

    Sword(
        size_t item_class_index, RangeOfLong damage_range, float hit_range,
        sf::Time cooldown_time = sf::seconds(0.3f)
    )
        : Weapon(item_class_index, damage_range),
          hit_range(hit_range),
          cooldown_time(cooldown_time) {}

    std::shared_ptr<Item> deepcopy_item() const override;

    void attack_by(Actor &source) override;
    bool cooldown();
    void try_to_attack(Actor &source, Actor &target) override;
    bool is_in_range(const Actor &source, sf::Vector2f target) const override;
};

// aka armour or equipment
class Wearable : public Item {
public:
    enum Kind {
        Helmet,
        ChestPlate,
        Leggins,
        Boots,
        Shield,
        Amulet,
    };

private:
    std::optional<CharacteristicsModifier> artefact;
    RangeOfLong defence_range;

    Kind kind;
};

class LockPicks : public Item {
public:
    DeepCopy(LockPicks);

    size_t count;

    std::shared_ptr<Item> deepcopy_item() const override;
};

struct LockPickingResult {
public:
    bool lock_picked;
    bool pick_broken;
};

class Inventory {
public:
    std::vector<std::shared_ptr<Item>> items;

    void add_item(std::shared_ptr<Item> item);
    void open_inventory();
    void close_inventory();
};

class InventoryCanvas {
    sf::RenderWindow &window;

public:
    InventoryCanvas(sf::RenderWindow &window) : window(window) {}
    ~InventoryCanvas() = default;

    void on_open_inventory(Inventory &inventory);
    void on_close_inventory(Inventory &inventory);
    void draw();
};

class Chest {
public:
    Inventory inventory;
    const size_t level;

    explicit Chest(size_t level) : inventory(), level(level) {}

    LockPickingResult simulate_picking(const Actor &source);
    void try_to_pick(const Actor &source, LockPicks &picks, size_t i, size_t j);
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
    };

    std::shared_ptr<Chest> building;

    Kind kind;

    explicit Tile() : Tile(Barrier) {}
    explicit Tile(Kind kind) : building(nullptr), kind(kind) {}

    Tile &set_building(std::shared_ptr<Chest> building);
};

class Experience {
    size_t level;
    size_t value;

public:
    void gain(size_t amount, Actor &actor);
    static size_t needs_exp_for_level(size_t level);
    void level_up();
};

class LevelUpCanvas {
    sf::RenderWindow &window;

public:
    LevelUpCanvas(sf::RenderWindow &window) : window(window) {}
    ~LevelUpCanvas() = default;

    void on_level_up(Experience &exp);
    void draw();
};

class Equipment {
public:
    DeepCopy(Equipment);

    std::unordered_map<Wearable::Kind, std::shared_ptr<Wearable>> wearable;
    std::shared_ptr<Item> weapon;

    void equip_wearable(std::shared_ptr<Wearable> item);
    void equip_weapon(std::shared_ptr<Item> weapon);
};

class RigidBody {
public:
    DeepCopy(RigidBody);

    sf::Vector2f position;
    bool pushable = true;
    float size = 1.0f;

    RigidBody() = default;
    RigidBody(float size) : size(size) {}

    sf::FloatRect get_axes_aligned_bounding_box() const;
    bool intersects(const RigidBody &other, sf::Vector2f &intersection_point) const;
    bool intersects(const RigidBody &other) const;
};

sf::Vector2f center(const sf::FloatRect &a);
bool intersects(
    const sf::FloatRect &aabb1, const sf::FloatRect &aabb2, sf::Vector2f &intersection_point
);

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

class Actor : public RigidBody {
public:
    DeepCopy(Actor);

    size_t actor_class_index;
    float health;
    Equipment equipment;
    Characteristics characteristics;
    bool alive = true;
    sf::Clock since_last_taken_damage;

    Actor() = default;
    Actor(size_t class_index, float size, Characteristics characteristics)
        : RigidBody(size),
          actor_class_index(class_index),
          health(characteristics.max_health),
          characteristics(characteristics) {}
    virtual ~Actor() = default;

    virtual void take_damage(float amount, Actor &source);
    virtual void init(){};
    virtual void update(float delta_time){};
    virtual void die(Actor &reason){};
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

    float health_bar_height_factor = 0.1f;

    sf::RectangleShape max_health_bar;
    sf::RectangleShape current_health_bar;

    SpriteColorAnimator taked_damage_animator =
        SpriteColorAnimator(sf::Color::White, sf::Color(200, 0, 0), sf::seconds(0.15f));

public:
    ItemsView items_view;

    ActorsView(sf::RenderWindow &window) : window(window), items_view(window) {}
    ~ActorsView() = default;

    void draw(const Actor &actor);
    void draw_ui(const Actor &actor);
};

class Player : public Actor {
public:
    DeepCopy(Player);

    Inventory inventory;
    LockPicks lock_picks;
    Experience experience;
    float pick_up_range = 1.2f;

    Player() = default;
    Player(size_t class_index, float size, Characteristics characteristics)
        : Actor(class_index, size, characteristics) {}
    Player(const Actor &actor) : Actor(actor) {}

    void init() override;
    void update(float delta_time) override;
    void handle_movement(float delta_time);
    void handle_equipment_use();
    void handle_lock_picking();
    void die(Actor &reason) override;
    void handle_picking_up_items();
    void pick_up_item(Item &item);
};

class Enemy : public Actor {
public:
    DeepCopy(Enemy);

    Enemy() = default;
    Enemy(size_t class_index, float size, Characteristics characteristics)
        : Actor(class_index, size, characteristics) {}
    Enemy(const Actor &actor) : Actor(actor) {}

    void init() override;
    void update(float delta_time) override;
    void handle_movement(float delta_time);
    void handle_equipment_use();
    void die(Actor &reason) override;
};

class LayingItem : public RigidBody {
public:
    std::shared_ptr<Item> item;
    bool picked_up = false;

    LayingItem(std::shared_ptr<Item> item) : RigidBody(), item(item) {}
};

class DungeonLevel {
public:
    std::vector<Enemy> enemies;
    std::vector<LayingItem> laying_items;
    Matrix<Tile> tiles;
    sf::Vector2f initial_player_position;
    float tile_size;
    float chest_size_factor;
    size_t actors_spawned_per_class = 10;

    void init();
    float tile_coords_to_world_coords_factor() const;
    sf::Vector2f center() const;
    void resize_tiles(size_t width, size_t height);
    void regenerate_tiles();
    void regenerate_enemies();
    std::optional<std::pair<size_t, size_t>> get_tile_coordinates(sf::Vector2f position) const;
    Tile *get_tile(sf::Vector2f position);
    void add_laying_item(std::unique_ptr<LayingItem> item);
    void update(float delta_time);
    void handle_collitions();
    void handle_actor_actor_collitions(std::vector<RigidBody *> &bodies);
    void handle_actor_level_collitions(std::vector<RigidBody *> &bodies);
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

    ActorsView actors_view;

public:
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

private:
    DungeonLevelView dungeon_level_view;
    InventoryCanvas inventory_canvas;
    LevelUpCanvas level_up_canvas;

    sf::Texture logo_texture;
    sf::Sprite logo;

    sf::Font font;
    sf::Text menu_message;
    sf::Text info_message;
    sf::Text death_message;

public:
    GameView()
        : window(), dungeon_level_view(window), inventory_canvas(window), level_up_canvas(window) {}
    ~GameView() = default;

    bool init(unsigned int width, unsigned int height);
    void draw();
    bool is_open() const;
    void clear();
    void display();

    friend DungeonLevelView;
};

class Game {
private:
    sf::Clock clock;

public:
    GameView game_view;

    Dungeon dungeon;

    bool is_playing = false;
    static constexpr float view_size = 10.0f;   // sets up the view size
    static constexpr float world_size = 10.0f;  // adjusts the sizes of the objects

    std::vector<ActorClass> actor_classes;  // the first entry should be a player class
    static constexpr size_t player_class_index = 0;
    Player player_template;
    std::vector<Enemy> enemy_templates;  // follows the actor_class_index

    std::vector<ItemClass> item_classes;
    std::vector<std::unique_ptr<Item>> item_templates;  // follows the item_classes

    Game() = default;
    ~Game() = default;

    static Game &get();

    void pause();
    void unpause();
    void update(float delta_time);

    void handle_events();
    void start_playing();
    void stop_playing();
    bool init(unsigned int width, unsigned int height);
    bool run();

    size_t add_actor_class(const ActorClass &cls);
    Enemy make_enemy(size_t actor_class_index) const;

    size_t add_item_class(const ItemClass &cls);
    std::shared_ptr<Item> make_item(size_t item_class_index) const;
    // size_t add_item_template(std::unique_ptr<Item> item);
};

#endif  // GAME_H
