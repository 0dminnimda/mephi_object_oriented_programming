Vector2
Sprite2D


# static const Game *game


class Game:
    Player *player
    current_level_index: int
    all_levels: list[DungeonLevel]

    def update(): ...


class DungeonLevel:
    all_actors: list[Actor]
    tiles: list[list[Tile]]

    def resize_tiles(width: int, height: int): ...
    def get_tile_of_an_actor(actor: Actor) -> Tile | None: ...
    def update(): ...


class Tile:
    enum Kind:
        Flor
        OpenDor
        ClosedDor
        UpLaddor
        DownLaddor

    kind: Kind

    laying_items: list[Item]
    building: Chest


class Building:
    def interact(): ...
    # like draw() but already in godot


class Experience:
    level: int
    value: int

    def gain(amount: int) -> None: ...


class ActorClass:
    ...


class Equipment:
    wearable: dict[Wearable::Kind, Wearable]
    weapons: list[Weapon]


class Characteristics:
    max_health: float
    defence: float


# class ValueModifier:
#     def apply(value: T) -> T: ...


class SetAbsoluteValue(ValueModifier):
    value: T

    def apply(value: T) -> T: ...


class AddToValue(ValueModifier):
    value: T

    def apply(value: T) -> T: ...


ValueModifier = SetAbsoluteValue | AddToValue


class CharacteristicsModifier:
    max_health: ValueModifier<float> | None
    defence: ValueModifier<float> | None


class Actor:  # XXX: creature?
    actor_class: ActorClass
    position: Vector2
    health: float
    characteristics: Characteristics
    equipment: Equipment

    def handle_movement(): ...
    def update(): ...
    def chance_to_take_damage() -> float: ...
    def take_damage(amount: float, source: Actor): ...
    def attack(source: Actor): ...
    def die(reason: Actor): ...


class Inventory:
    items: list[Item]

    def add_item(item: Item): ...


class Player(Actor):
    inventory: Inventory
    experience: Experience

    def pick_up_item(item: Item): ...


class Enemy(Actor):
    pass


class Item:
    name: str
    icon: Sprite2D


class Potion(Item):
    modifier: CharacteristicsModifier

    def apply(target: Actor): ...


class RangeOfValues:
    min: float
    max: float

    def get_random() -> float: ...


class Enchantment:
    target_actor_class: ActorClass
    damage_multiplier: float


class Weapon(Item):
    artefact: CharacteristicsModifier | None
    enchantment: Enchantment | None
    damage_range: RangeOfValues

    def attack(pos: Vector2): ...


class Wearable(Item):  # aka armour or equipment
    artefact: CharacteristicsModifier | None
    defence_range: RangeOfValues

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
