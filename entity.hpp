#ifndef ENTITY_H
#define ENTITY_H

// originator of another entity
// handle to originator, entity cant collide with its originator
// collision layers (1,2,3,4,5)
typedef enum EntityFlag {
    EntityFlag_Active =        (1 << 0),
    EntityFlag_MoveWithCtrls = (1 << 1),
    EntityFlag_CanCollide =    (1 << 2),
    EntityFlag_CanShoot =      (1 << 3),
    EntityFlag_MoveWithPhys =  (1 << 4),
    EntityFlag_IsProjectile =  (1 << 5),
    EntityFlag_Wrapping =      (1 << 6),
    EntityFlag_Particle =      (1 << 7),
} EntityFlag;

typedef enum CollisionType {
    CollisionType_None,
    CollisionType_Explode,
    CollisionType_Health,
    CollisionType_SplinterOnDeath,
    CollisionType_HealthOrSplinter,
    CollisionType_Count,
} CollisionType;

typedef enum DeathType{
    DeathType_Crumble,
    DeathType_Particle,
    DeathType_Animate
} DeathType;

typedef enum EntityType {EntityType_None, EntityType_Quad, EntityType_Texture, EntityType_Text, EntityType_Line, EntityType_Ship, EntityType_Bullet, EntityType_Asteroid} EntityType;

typedef struct Entity{
    Entity* origin;
    Entity* parent;
    Entity* next;
    Entity* prev;

    EntityType type;
    DeathType death_type;
    u32 flags;
    u32 collision_type;
    u32 index;
    u32 generation;

    v2 pos;
    v2 dim;
    v2 dir;
    v2 accel_dir;
    f32 deg;
    v2 scale;
    RGBA color;

    f32 speed;
    f32 rot_speed;
    f32 shoot_t;
    f32 velocity;

    s32 health;
    s32 damage;
    bool in_play;
    f32 particle_t;
    bool accelerating;
    bool exploding;
    u32  explosion_tex;
    f32 explosion_t;
    f32 immune_t;
    bool immune;

    u32 texture;
} Entity;

static bool has_flags(u32 rflags, u32 lflags);
static void set_flags(u32* rflags, u32 lflags);
static void clear_flags(u32* rflags, u32 lflags);

typedef struct EntityHandle{
    u32 index;
    u32 generation;
} EntityHandle;

static EntityHandle zero_entity_handle();
//static Entity* entity_from_handle(PermanentMemory* pm, EntityHandle handle);
//static EntityHandle handle_from_entity(PermanentMemory* pm, Entity *e);
static Rect rect_from_entity(Entity* e);


#endif
