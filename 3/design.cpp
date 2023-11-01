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

class DungeonLevel;

class Game : public Node {
    GDCLASS(Game, Node)

private:
    int current_level_index;
    std::vector<DungeonLevel> all_levels;

protected:
    static void _bind_methods(){};

public:
    Game();
    ~Game();

    void update();
};

class Actor;
class Tile;

class DungeonLevel {
private:
    std::vector<Actor> all_actors;
    std::vector<std::vector<Tile>> tiles;

    void resize_tiles(size_t width, size_t height);
    std::optional<Tile> get_tile_of_an_actor(const Actor &actor);
    void update();
};

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

class CharacteristicsModifier {
    std::optional<ValueModifier<float>> max_health;
    std::optional<ValueModifier<float>> defence;
};

class Item {
    std::string name;
    Sprite2D icon;
};

class Potion : public Item {
    CharacteristicsModifier modifier;

    void apply(Actor target);
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

class Inventory {
    std::vector<Item> items;

    void add_item(Item item);
};

class Chest {
    Inventory inventory;
    int level;

    void interact();
    LockPickingResult try_to_pick(LockPicks picks);
};

class Tile {
    enum Kind {
        Flor,
        OpenDor,
        ClosedDor,
        UpLaddor,
        DownLaddor,
    };

    Kind kind;

    std::vector<Item> laying_items;
    std::optional<Chest> building;
};

class Experience {
    int level;
    int value;

    void gain(int amount);
};

class Equipment {
    std::unordered_map<Wearable::Kind, Wearable> wearable;
    std::vector<Weapon> weapons;
};

class Characteristics {
    float max_health;
    float defence;
};

// XXX: creature?
class Actor {
    ActorClass actor_class;
    Vector2 position;
    float health;
    Characteristics characteristics;
    Equipment equipment;

    void handle_movement();
    void update();
    float chance_to_take_damage();
    void take_damage(float amount, Actor source);
    void attack(Actor source);
    void die(Actor reason);
};

class Player : public Actor {
    Inventory inventory;
    Experience experience;

    void pick_up_item(Item item);
};

class Enemy : public Actor {};

#endif  // GAME_IMPL
