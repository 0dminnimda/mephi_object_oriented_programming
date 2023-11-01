#pragma once

#include <optional>
#include <unordered_map>
#ifndef GAME_IMPL
#define GAME_IMPL

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace godot;

class Game;

static const Game *game = nullptr;

template <typename T>
class SetAbsoluteValue {
    T value;

    T apply(T value);
};

template <typename T>
class AddToValue {
    T value;

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

    void apply(Characteristics &value);
};

class Actor;

class Item {
    std::string name;
    Ref<Texture2D> icon;

    virtual void use(Actor &target) {}
};

class Potion : public Item {
    CharacteristicsModifier modifier;

    void use(Actor &target) override;
    void apply(Actor &target);
};

class RangeOfValues {
    float min;
    float max;

    float get_random();
};

class ActorClass {};

class Enchantment {
    ActorClass target_actor_class;
    float damage_multiplier;
};

class Weapon : public Item {
    std::optional<CharacteristicsModifier> artefact;
    std::optional<Enchantment> enchantment;
    RangeOfValues damage_range;

    void use(Actor &target) override;
    void attack(Vector2 pos);
};

// aka armour or equipment
class Wearable : public Item {
public:
    enum Kind {
        // ...
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

class Inventory : public CanvasItem {
    GDCLASS(Inventory, CanvasItem)

protected:
    static void _bind_methods(){};

private:
    std::vector<Item> items;

public:
    void add_item(Item &item);
    void _draw();
};

class Chest {
    Inventory inventory;
    int level;

public:
    void interact();
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

class Experience : public CanvasItem {
    GDCLASS(Experience, CanvasItem)

protected:
    static void _bind_methods(){};

private:
    int level;
    int value;

public:
    void gain(int amount, Actor &actor);
    void level_up();
};

class Equipment {
    std::unordered_map<Wearable::Kind, Wearable> wearable;
    std::vector<Weapon> weapons;
};

// XXX: creature?
class Actor {
    ActorClass actor_class;
    Vector2 position;
    float health;
    Characteristics characteristics;
    Equipment equipment;

public:
    void handle_movement();
    void update();
    float chance_to_take_damage();
    void take_damage(float amount, Actor &source);
    void attack(Actor &target);
    void die(Actor &reason);
};

class Player : public Actor {
    Inventory inventory;
    Experience experience;

public:
    void pick_up_item(Item item);
};

class Enemy : public Actor {};

class LayingItem : public Sprite2D {
    GDCLASS(LayingItem, Node)

protected:
    static void _bind_methods(){};

private:
    Item item;

public:
    void _draw();
};

class DungeonLevel {
private:
    std::vector<Actor> actors;
    std::vector<Ref<LayingItem>> laying_items;
    std::vector<std::vector<Tile>> tiles;

public:
    void resize_tiles(size_t width, size_t height);
    std::optional<Tile> get_tile_of_an_actor(const Actor &actor);
    void update();
};

class Game : public Node {
    GDCLASS(Game, Node)

protected:
    static void _bind_methods(){};

private:
    int current_level_index = -1;
    std::vector<DungeonLevel> all_levels;

public:
    void update();
};

#endif  // GAME_IMPL
