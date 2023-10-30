Vector2


class Experience:
    level: int
    value: int

    def gain(amount: int) -> None: ...


class Health:
    value: float
    maximum: float


class ActorClass:
    ...


class Actor:  # XXX: creature?
    actor_class: ActorClass
    position: Vector2
    health: Health
    defence: float

    def chance_to_take_damage() -> float: ...
    def take_damage(amount: float, source: Actor): ...
    def attack(source: Actor): ...
    def die(reason: Actor): ...


class Inventory:
    items: list[Item]


class Equipment:
    items: dict[Wearable::Kind, Wearable | None]


class Player(Actor):
    equipment: Equipment
    inventory: Inventory
    experience: Experience


class Enemy(Actor):
    weapon: Weapon


class Item:
    pass


class Potion(Item):
    def apply(target: Actor): ...


class SpacialDistribution:
    def fast_in_bounds(pos: Vector2) -> bool: ...
    def in_bounds(pos: Vector2) -> bool: ...
    def eval_at(pos: Vector2) -> bool: ...


class Weapon(Item):
    distribution: SpacialDistribution

    def attack(pos: Vector2, ...all_actors?): ...


class Wearable(Item):  # aka armour or equipment
    enum Kind:
        ...

    kind: Kind


class LockPicks(Item):
    pass


class Chest:
    inventory: Inventory
