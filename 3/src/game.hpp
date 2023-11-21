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

class Game;

static const std::shared_ptr<Game> game;

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
    std::vector<std::unique_ptr<Item>> items;

public:
    void add_item(Item &item);
    void open_inventory();
    void close_inventory();
};

class InventoryCanvas : public sf::Drawable {
public:
    ~InventoryCanvas() {}

    void on_open_inventory(Inventory &inventory);
    void on_close_inventory(Inventory &inventory);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
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
    Kind kind;

    Chest *building;

public:
    ~Tile() {
        delete building;
    }

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

class LevelUpCanvas : public sf::Drawable {
public:
    ~LevelUpCanvas() {}

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    void on_level_up(Experience &exp);
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
    sf::Vector2f position_;

public:
    sf::Vector2f position() { return position_; }
};

class ActorClass {
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
    Actor(size_t class_index, long health, Characteristics characteristics) : RigidBody(), actor_class_index_(class_index), health_(health), equipment_(), characteristics_(characteristics) {}
    virtual ~Actor() = default;

    size_t actor_class_index() const { return actor_class_index_; };
    long health() const { return health_; };
    const Equipment &equipment() const { return equipment_; };
    Characteristics &characteristics() { return characteristics_; };
    const Characteristics &characteristics() const { return characteristics_; };

    float chance_to_take_damage();
    void take_damage(long amount, Actor &source);

    virtual void update() {};
    virtual void handle_movement() {};
    virtual void attack(Actor &target) {};
    virtual void die(Actor &reason) {};
};

class Player : public Actor {
private:
    Inventory inventory;
    Experience experience;

public:
    void update() override;
    void handle_movement() override;
    void attack(Actor &target) override;
    void die(Actor &reason) override;

    void pick_up_item(Item &item);
};

class Enemy : public Actor {
public:
    void update() override;
    void handle_movement() override;
    void attack(Actor &target) override;
    void die(Actor &reason) override;
};

class LayingItem : public RigidBody {
public:
    std::shared_ptr<Item> item;
};

class DungeonLevel {
private:
    std::vector<Actor> actors;
    std::vector<LayingItem> laying_items;
    Matrix<Tile> tiles;

public:
    DungeonLevel(Matrix<Tile> tiles) : actors(), laying_items(), tiles(tiles) {}

    void resize_tiles(size_t width, size_t height);
    std::optional<Tile> get_tile_of_an_actor(const Actor &actor);
    void add_laying_item(std::unique_ptr<LayingItem> item);
    void update();
};

class Game {
private:
    int current_level_index = -1;
    std::vector<DungeonLevel> all_levels;
    InventoryCanvas inventory_canvas;
    LevelUpCanvas level_up_canvas;
    std::vector<ActorClass> actor_classes;
    bool is_playing = false;

    sf::RenderWindow window;
    sf::Clock clock;

    sf::Texture logo_texture;
    sf::Sprite logo;

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
    void pause();
    void unpause();
    void update();

    void add_level(DungeonLevel &level);
    void load_level(size_t index);
    void unload_current_level();
    DungeonLevel *get_current_level();

    void handle_events();
    void start_playing();
    int init(unsigned int width, unsigned int height);
    int run();
};

#endif  // GAME_H
