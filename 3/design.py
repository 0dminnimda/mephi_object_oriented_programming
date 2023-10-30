Vector2
Sprite2D


class Game:
    current_level_index: int
    all_levels: list[DungeonLevel]


class DungeonLevel:
    all_actors: list[Actor]
    tiles: list[list[Tile]]


class Tile:
    enum Kind:
        ...

    kind: Kind

    laying_items: list[Item]
    building: Building


class Building:
    pass
    # like draw() but already in godot


class Experience:
    level: int
    value: int

    def gain(amount: int) -> None: ...


class ActorClass:
    ...


class Equipment:
    wearable: dict[Wearable::Kind, Wearable | None]
    weapons: list[Weapon]


class Characteristics:
    max_health: float
    defence: float


class ValueModifier:
    def apply(value: T) -> T: ...


class SetAbsoluteValue(ValueModifier):
    value: T

    def apply(value: T) -> T: ...


class AddToValue(ValueModifier):
    value: T

    def apply(value: T) -> T: ...


# ValueModifier = SetAbsoluteValue | AddToValue


class CharacteristicsModifier:
    max_health: ValueModifier<float>
    defence: ValueModifier<float>


class Actor:  # XXX: creature?
    actor_class: ActorClass
    position: Vector2
    health: float
    characteristics: Characteristics
    equipment: Equipment

    def chance_to_take_damage() -> float: ...
    def take_damage(amount: float, source: Actor): ...
    def attack(source: Actor): ...
    def die(reason: Actor): ...


class Inventory:
    items: list[Item]


class Player(Actor):
    inventory: Inventory
    experience: Experience


class Enemy(Actor):
    pass


class Item:
    name: str
    icon: Sprite2D


class Potion(Item):
    modifier: CharacteristicsModifier

    def apply(target: Actor): ...


class SpacialDistribution:
    def fast_in_bounds(pos: Vector2) -> bool: ...
    def in_bounds(pos: Vector2) -> bool: ...
    def eval_at(pos: Vector2) -> bool: ...


class Enchantment:
    target_actor_class: ActorClass
    damage_multiplier: float


class Weapon(Item):
    modifier: CharacteristicsModifier | None
    enchantment: Enchantment | None
    distribution: SpacialDistribution

    def attack(pos: Vector2): ...


class Wearable(Item):  # aka armour or equipment
    modifier: CharacteristicsModifier | None
    distribution: SpacialDistribution

    enum Kind:
        ...

    kind: Kind


class LockPicks(Item):
    count: int


class LockPickingResult:
    lock_picked: bool
    pick_borken: bool


class Chest(Building):
    inventory: Inventory
    level: int

    def try_to_pick(picks: LockPicks) -> LockPickingResult: ...
