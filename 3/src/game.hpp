#pragma once

#ifndef GAME_H
#define GAME_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "SFML/Graphics/Drawable.hpp"
#include <SFML/Graphics.hpp>

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
    T apply(T) {
        return this->value;
    }
};

template <typename T>
class AddToValue {
    T value;

public:
    T apply(T value) {
        return value + this->value;
    }
};

template <typename T>
using ValueModifier = std::variant<SetAbsoluteValue<T>, AddToValue<T>>;

class Characteristics {
public:
    long max_health;
    long defence;
    long speed;
};

class CharacteristicsModifier {
    std::optional<ValueModifier<float>> max_health;
    std::optional<ValueModifier<float>> defence;
    std::optional<ValueModifier<float>> speed;

public:
    CharacteristicsModifier(ValueModifier<float> max_health, ValueModifier<float> defence, ValueModifier<float> speed) :
        max_health(max_health), defence(defence), speed(speed) {}

    void apply(Characteristics &value);
};

class Actor;

class Item {
    std::string name;
    std::shared_ptr<sf::Texture> icon;

public:
    virtual ~Item() = default;
    virtual void use(Actor &target) {};
};

class Potion : public Item {
    CharacteristicsModifier modifier;

public:
    Potion(CharacteristicsModifier modifier) : modifier(modifier) {} 

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
};

class Weapon : public Item {
    std::optional<CharacteristicsModifier> artefact;
    std::optional<Enchantment> enchantment;
    RangeOfValues damage_range;

public:
    void use(Actor &target) override;
    void attack(sf::Vector2f position);
    long get_damage(Actor &target);
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

    ~Tile() {
        delete building;
    }

    explicit Tile() : Tile(Flor) {}
    explicit Tile(Kind kind) : kind(kind), building(nullptr) {}

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
    std::unordered_map<Wearable::Kind, Wearable> wearable;
    std::optional<Weapon> weapon;

public:
    Equipment() : wearable(), weapon() {}
    void equip_wearable(Wearable &item);
    void equip_weapon(Weapon &item);
};

class RigidBody {
protected:
    sf::Vector2f velocity_;

public:
    sf::Vector2f position;
    float size = 1.0f;
};

class ActorClass {
public:
    std::string name;
    std::string description;
};

// XXX: creature?
class Actor : public RigidBody {
private:
    // std::string name;
    const size_t actor_class_index_;
    long health_;
    Equipment equipment_;
    Characteristics characteristics_;

public:
    sf::Texture texture;

    Actor(size_t class_index, long health, Characteristics characteristics) : RigidBody(), actor_class_index_(class_index), health_(health), characteristics_(characteristics) {}
    virtual ~Actor() = default;

    size_t actor_class_index() const { return actor_class_index_; };
    long health() const { return health_; };
    const Equipment &equipment() const { return equipment_; };
    Characteristics &characteristics() { return characteristics_; };
    const Characteristics &characteristics() const { return characteristics_; };

    float chance_to_take_damage();
    void take_damage(long amount, Actor &source);

    virtual void update(float delta_time) {};
    virtual void handle_movement() {};
    virtual void attack(Actor &target) {};
    virtual void die(Actor &reason) {};
};

class ActorsView {
private:
    sf::RenderWindow &window;

    sf::Sprite actor_sprite;

public:
    ActorsView(sf::RenderWindow &window) : window(window) {}
    ~ActorsView() = default;

    void draw(const Actor &actor);

private:
    void draw_tile(const Tile &tile, sf::Vector2f position, float max_tiles_size);
};

class Player : public Actor {
private:
    Inventory inventory;
    Experience experience;

public:
    Player(size_t class_index, long health, Characteristics characteristics) : Actor(class_index, health, characteristics) {}

    void update(float delta_time) override;
    void handle_movement() override;
    void attack(Actor &target) override;
    void die(Actor &reason) override;

    void pick_up_item(Item &item);
};

class Enemy : public Actor {
public:
    void update(float delta_time) override;
    void handle_movement() override;
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
    Player player;
    std::vector<LayingItem> laying_items;
    Matrix<Tile> tiles;

    DungeonLevel(Matrix<Tile> tiles, Player player) : enemies(), player(player), laying_items(), tiles(tiles) {}

    void resize_tiles(size_t width, size_t height);
    std::optional<Tile> get_tile_of_an_actor(const Actor &actor);
    void add_laying_item(std::unique_ptr<LayingItem> item);
    void update(float delta_time);
};

class DungeonLevelView {
private:
    sf::RenderWindow &window;

    sf::Texture flor_tile_texture;
    sf::Texture open_dor_tile_texture;
    sf::Texture closed_dor_tile_texture;
    sf::Sprite tile_sprite;

    ActorsView actors_view;

public:
    DungeonLevelView(sf::RenderWindow &window) : window(window), actors_view(window) {}
    ~DungeonLevelView() = default;

    bool init();
    void draw(const DungeonLevel &level);

private:
    void draw_tile(const Tile &tile, sf::Vector2f position, float max_tiles_size);
};

class GameView {
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
    sf::RenderWindow window;

    GameView() : window(), dungeon_level_view(window), inventory_canvas(window), level_up_canvas(window) {}
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
    int current_level_index = -1;
    std::vector<DungeonLevel> all_levels;

    sf::Clock clock;

    GameView game_view;

public:
    bool is_playing = false;

    std::vector<ActorClass> actor_classes;

    Game() = default;
    ~Game() = default;

    static Game &get();

    void pause();
    void unpause();
    void update(float delta_time);

    void add_level(const DungeonLevel &level);
    void load_level(size_t index);
    void unload_current_level();
    DungeonLevel *get_current_level();

    void handle_events();
    void start_playing();
    bool init(unsigned int width, unsigned int height);
    bool run();
};

#endif  // GAME_H
