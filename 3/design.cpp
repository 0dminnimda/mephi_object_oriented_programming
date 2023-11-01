#pragma once

#ifndef GAME_IMPL
#define GAME_IMPL

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace godot;

class Game;

static const Ref<Game> game;

template <typename T>
class SetAbsoluteValue {
    T value;

public:
    T apply(T value);
};

template <typename T>
class AddToValue {
    T value;

public:
    T apply(T value);
};

template <typename T>
using ValueModifier = std::variant<SetAbsoluteValue<T>, AddToValue<T>>;

class Characteristics {
    float max_health;
    float defence;
    float speed;
};

class CharacteristicsModifier {
    std::optional<ValueModifier<float>> max_health;
    std::optional<ValueModifier<float>> defence;
    std::optional<ValueModifier<float>> speed;

public:
    void apply(Characteristics &value);
};

class Actor;

class Item {
    std::string name;
    Ref<Texture2D> icon;

public:
    virtual void use(Actor &target) {}
};

class Potion : public Item {
    CharacteristicsModifier modifier;

public:
    void use(Actor &target) override;
    void apply(Actor &target);
};

class RangeOfValues {
    float min;
    float max;

public:
    float get_random();
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
    void attack(Vector2 pos);
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

class InventoryCanvas : public CanvasItem {
    GDCLASS(InventoryCanvas, CanvasItem)

protected:
    static void _bind_methods(){};

public:
    void on_open_inventory(Inventory &inventory);
    void on_close_inventory(Inventory &inventory);
    void _draw();
};

class Chest {
    Inventory inventory;
    int level;

public:
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

    std::optional<Chest> building;
};

class Experience {
    int level;
    int value;

public:
    void gain(int amount, Actor &actor);
    void level_up();
};

class LevelUpCanvas : public CanvasItem {
    GDCLASS(LevelUpCanvas, CanvasItem)

protected:
    static void _bind_methods(){};

public:
    void _draw();
    void on_level_up(Experience &exp);
};

class Equipment {
    std::unordered_map<Wearable::Kind, Wearable> wearable;
    Weapon weapon;
};

class ActorClass {
    std::string name;
    std::string description;
};

// XXX: creature?
class Actor : public RigidBody2D {
    GDCLASS(Actor, RigidBody2D)

protected:
    static void _bind_methods(){};

private:
    size_t actor_class_index;
    float health;
    Characteristics characteristics;
    Equipment equipment;

public:
    float chance_to_take_damage();
    void take_damage(float amount, Actor &source);

    virtual void update();
    virtual void handle_movement();
    virtual void attack(Actor &target);
    virtual void die(Actor &reason);
};

class Player : public Actor {
    GDCLASS(Player, Actor)

protected:
    static void _bind_methods(){};

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
    void update() override;
    void handle_movement() override;
    void attack(Actor &target) override;
    void die(Actor &reason) override;
};

class LayingItem : public RigidBody2D {
    GDCLASS(LayingItem, RigidBody2D)

protected:
    static void _bind_methods(){};

public:
    std::unique_ptr<Item> item;
};

template <typename T>
class Row {
    std::vector<T> items;

public:
    T &operator[](size_t index);
};

template <typename T>
class Matrix {
    std::vector<Row<T>> rows;

public:
    Row<T> &operator[](size_t index);
};

class DungeonLevel {
private:
    std::vector<Actor> actors;
    std::vector<Ref<LayingItem>> laying_items;
    Matrix<Tile> tiles;

public:
    void resize_tiles(size_t width, size_t height);
    std::optional<Tile> get_tile_of_an_actor(const Actor &actor);
    void update();
};

class Game : public CanvasItem {
    GDCLASS(Game, CanvasItem)

protected:
    static void _bind_methods(){};

private:
    int current_level_index = -1;
    std::vector<DungeonLevel> all_levels;
    InventoryCanvas inventory_canvas;
    LevelUpCanvas level_up_canvas;
    std::vector<ActorClass> actor_classes;
    bool is_paused = false;

public:
    void _draw();
    void pause();
    void unpause();
    void update();
};

#endif  // GAME_IMPL
