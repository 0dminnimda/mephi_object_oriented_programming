#pragma once

#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

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
};

class CharacteristicsModifier {
    std::optional<ValueModifier<float>> max_health;
    std::optional<ValueModifier<float>> defence;
    std::optional<ValueModifier<float>> speed;

public:
    CharacteristicsModifier(
        ValueModifier<float> max_health, ValueModifier<float> defence, ValueModifier<float> speed
    )
        : max_health(max_health), defence(defence), speed(speed) {}

    void apply(Characteristics &value);
};

class Actor;

class ItemClass {
public:
    std::string name;
    std::string description;
    std::string texture_name;
    sf::Texture texture;
    sf::Sprite sprite;
    float size;

    ItemClass(std::string name, std::string description, std::string texture_name, float size)
        : name(name), description(description), texture_name(texture_name), size(size) {}
    ~ItemClass() = default;

    bool init();
};

class Item {
public:
    size_t item_class_index;

    Item(size_t item_class_index) : item_class_index(item_class_index) {}

    virtual ~Item() = default;
    virtual void use(Actor &target){};
    virtual std::shared_ptr<Item> copy() const = 0;
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

class RangeOfValues {
    long min;
    long max;

public:
    RangeOfValues(long min, long max) : min(min), max(max) {}

    long get_random();
};

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
    RangeOfValues damage_range;

public:
    Weapon(size_t item_class_index, RangeOfValues damage_range)
        : Item(item_class_index), damage_range(damage_range) {}

    void use(Actor &target) override;
    virtual void attack(Actor &source) = 0;
    virtual void try_to_attack(Actor &source, Actor &target) = 0;
    virtual float get_damage(Actor &target);
    virtual bool is_in_range(const Actor &source, sf::Vector2f target) const = 0;
};

class Hammer : public Weapon {
public:
    float hit_range;
    sf::Clock cooldown_timer;
    sf::Time cooldown_time;
    bool on_cooldown = false;

    Hammer(
        size_t item_class_index, RangeOfValues damage_range, float hit_range,
        sf::Time cooldown_time = sf::seconds(1.0f)
    )
        : Weapon(item_class_index, damage_range),
          hit_range(hit_range),
          cooldown_time(cooldown_time) {}

    std::shared_ptr<Item> copy() const override;

    void attack(Actor &source) override;
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
    RangeOfValues defence_range;

    Kind kind;
};

class LockPicks : public Item {
    size_t count;
};

struct LockPickingResult {
    bool lock_picked;
    bool pick_borken;
};

class Inventory {
private:
    std::vector<std::shared_ptr<Item>> items;

public:
    void add_item(Item &item);
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

    LockPickingResult try_to_pick(LockPicks &picks);
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

private:
    Chest *building;

public:
    Kind kind;

    ~Tile() { delete building; }

    explicit Tile() : Tile(Barrier) {}
    explicit Tile(Kind kind) : building(nullptr), kind(kind) {}

    Tile &set_building(std::unique_ptr<Chest> building);
};

class Experience {
    int level;
    int value;

public:
    void gain(int amount, Actor &actor);
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
    std::unordered_map<Wearable::Kind, std::shared_ptr<Wearable>> wearable;
    std::shared_ptr<Weapon> weapon;

    void equip_wearable(std::shared_ptr<Wearable> item);
    void equip_weapon(std::shared_ptr<Weapon> weapon);
};

class RigidBody {
public:
    // sf::Vector2f velocity_;
    sf::Vector2f position;
    float size = 1.0f;

    RigidBody() = default;
    RigidBody(float size) : size(size) {}
};

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
    size_t actor_class_index;
    float health;
    Equipment equipment;
    Characteristics characteristics;
    bool alive = true;

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
    virtual void attack(Actor &target){};
    virtual void die(Actor &reason){};
};

class ActorsView {
private:
    sf::RenderWindow &window;

    ItemsView items_view;

public:
    ActorsView(sf::RenderWindow &window) : window(window), items_view(window) {}
    ~ActorsView() = default;

    void draw(const Actor &actor);
};

class Player : public Actor {
private:
    Inventory inventory;
    Experience experience;

public:
    Player() = default;
    Player(size_t class_index, float size, Characteristics characteristics)
        : Actor(class_index, size, characteristics) {}
    Player(const Actor &actor) : Actor(actor) {}
    Player(Actor &&actor) : Actor(actor) {}

    void init() override;
    void update(float delta_time) override;
    void handle_movement(float delta_time);
    void handle_equipment_use();
    void attack(Actor &target) override;
    void die(Actor &reason) override;

    void pick_up_item(Item &item);
};

class Enemy : public Actor {
public:
    Enemy() = default;
    Enemy(size_t class_index, float size, Characteristics characteristics)
        : Actor(class_index, size, characteristics) {}
    Enemy(const Actor &actor) : Actor(actor) {}
    Enemy(Actor &&actor) : Actor(actor) {}

    Enemy copy() const;

    void init() override;
    void update(float delta_time) override;
    void attack(Actor &target) override;
    void die(Actor &reason) override;
};

class LayingItem : public RigidBody {
public:
    std::shared_ptr<Item> item;
};

class DungeonLevel {
    // private:
public:
    std::vector<Enemy> enemies;
    std::vector<LayingItem> laying_items;
    Matrix<Tile> tiles;
    sf::Vector2f initial_player_position;
    float tile_size;

    void init();
    float tile_coords_to_world_coords_factor() const;
    sf::Vector2f center() const;
    void resize_tiles(size_t width, size_t height);
    void regenerate_tiles();
    void regenerate_enemies();
    std::optional<std::pair<size_t, size_t>> get_tile_coordinates(const sf::Vector2f &position
    ) const;
    void add_laying_item(std::unique_ptr<LayingItem> item);
    void update(float delta_time);
    void handle_collitions();
    void delete_the_dead();
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

    ActorsView actors_view;

public:
    DungeonLevelView(sf::RenderWindow &window) : window(window), actors_view(window) {}
    ~DungeonLevelView() = default;

    bool init();
    void draw(const DungeonLevel &level);

private:
    void draw_tile(const Tile &tile, sf::Vector2f position, float size);
};

class GameView {
public:
    sf::RenderWindow window;
    sf::View view;

private:
    DungeonLevelView dungeon_level_view;
    InventoryCanvas inventory_canvas;
    LevelUpCanvas level_up_canvas;

    sf::Texture logo_texture;
    sf::Sprite logo;

    sf::Font font;
    sf::Text menu_message;
    sf::Text info_message;

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
    std::vector<DungeonLevel> all_levels;
    std::optional<DungeonLevel> current_level;

    sf::Clock clock;

    GameView game_view;

public:
    bool is_playing = false;
    static constexpr float view_size = 10.0f;   // sets up the view size
    static constexpr float world_size = 10.0f;  // adjusts the sizes of the objects

    std::vector<ActorClass> actor_classes;  // the first entry should be a player class
    std::vector<Enemy> enemy_templates;     // follows the actor_class_index
    static constexpr size_t player_class_index = 0;

    std::vector<ItemClass> item_classes;
    std::vector<std::unique_ptr<Item>> item_templates;  // follows the item_classes

    Player player;

    Game() = default;
    ~Game() = default;

    static Game &get();

    void pause();
    void unpause();
    void update(float delta_time);

    void add_level(const DungeonLevel &level);
    bool load_level(size_t index);
    void unload_current_level();
    DungeonLevel *get_current_level();

    void handle_events();
    void start_playing();
    void stop_playing();
    bool init(unsigned int width, unsigned int height);
    bool run();

    size_t add_actor_class(const ActorClass &cls);
    Enemy make_enemy(size_t actor_class_index) const;

    size_t add_item_class(const ItemClass &cls);
    // size_t add_item_template(std::unique_ptr<Item> item);
};

#endif  // GAME_H
