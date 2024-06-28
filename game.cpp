#ifndef GAME_C
#define GAME_C

static void
init_levels(){
    Level* level = 0;

    level = pm->levels + 0;
    level->asteroid_count_max = 3;
    level->asteroid_spawned = 0;
    level->asteroid_destroyed = 0;

    level = pm->levels + 1;
    level->asteroid_count_max = 5;
    level->asteroid_spawned = 0;
    level->asteroid_destroyed = 0;

    level = pm->levels + 2;
    level->asteroid_count_max = 7;
    level->asteroid_spawned = 0;
    level->asteroid_destroyed = 0;
}

static void
load_assets(Arena* arena){

    ScratchArena scratch = begin_scratch();

    Bitmap bm;
    bm = load_bitmap(scratch.arena, str8_literal("sprites/ship2.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Ship].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/circle.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Bullet].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/asteroid.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Asteroid].view, &bm);

    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame1.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame1].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame2.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame2].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame3.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame3].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame4.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame4].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/flame5.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Flame5].view, &bm);

    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion1.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion1].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion2.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion2].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion3.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion3].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion4.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion4].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion5.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion5].view, &bm);
    bm = load_bitmap(scratch.arena, str8_literal("sprites/explosion6.bmp"));
    init_texture_resource(&tm->assets.textures[TextureAsset_Explosion6].view, &bm);

    end_scratch(scratch);

    tm->assets.waves[WaveAsset_Track1] = load_wave(arena, str8_literal("sounds/track1.wav"));
    tm->assets.waves[WaveAsset_Track2] = load_wave(arena, str8_literal("sounds/track2.wav"));
    tm->assets.waves[WaveAsset_Track3] = load_wave(arena, str8_literal("sounds/track3.wav"));
    tm->assets.waves[WaveAsset_Track4] = load_wave(arena, str8_literal("sounds/track4.wav"));
    tm->assets.waves[WaveAsset_Track5] = load_wave(arena, str8_literal("sounds/track5.wav"));
    tm->assets.waves[WaveAsset_Rail1] =  load_wave(arena, str8_literal("sounds/rail1.wav"));
    tm->assets.waves[WaveAsset_Rail2] =  load_wave(arena, str8_literal("sounds/rail2.wav"));
    tm->assets.waves[WaveAsset_Rail3] =  load_wave(arena, str8_literal("sounds/rail3.wav"));
    tm->assets.waves[WaveAsset_Rail4] =  load_wave(arena, str8_literal("sounds/rail4.wav"));
    tm->assets.waves[WaveAsset_Rail5] =  load_wave(arena, str8_literal("sounds/rail5.wav"));

    tm->assets.fonts[FontAsset_Arial] =    load_font_ttf(arena, str8_literal("fonts/arial.ttf"), 24);
    tm->assets.fonts[FontAsset_Golos] =    load_font_ttf(arena, str8_literal("fonts/GolosText-Regular.ttf"), 36);
    tm->assets.fonts[FontAsset_Consolas] = load_font_ttf(arena, str8_literal("fonts/consola.ttf"), 36);
}

// todo: Move these to entity once you move PermanentMemory further up in the tool chain
static Entity*
entity_from_handle(EntityHandle handle){
    Entity *result = 0;
    if(handle.index < (s32)array_count(pm->entities)){
        Entity *e = pm->entities + handle.index;
        if(e->generation == handle.generation){
            result = e;
        }
    }
    return(result);
}

static EntityHandle
handle_from_entity(Entity *e){
    assert(e != 0);
    EntityHandle result = {0};
    if((e >= pm->entities) && (e < (pm->entities + array_count(pm->entities)))){
        result.index = e->index;
        result.generation = e->generation;
    }
    return(result);
}

static void
remove_entity(Entity* e){
    e->type = EntityType_None; // todo: remove this
    clear_flags(&e->flags, EntityFlag_Active);
    pm->free_entities[++pm->free_entities_at] = e->index;
    pm->entities_count--;
    *e = {0};
}

static Entity*
add_entity(EntityType type){
    if(pm->free_entities_at < ENTITIES_MAX){
        u32 free_entity_index = pm->free_entities[pm->free_entities_at--];
        Entity *e = pm->entities + free_entity_index;
        e->index = free_entity_index;
        set_flags(&e->flags, EntityFlag_Active);
        pm->generation[e->index]++;
        pm->entities_count++;
        e->generation = pm->generation[e->index]; // CONSIDER: this might not be necessary
        e->type = type;

        return(e);
    }
    return(0);
}

static Entity*
add_quad(v2 pos, v2 dim, RGBA color){
    Entity* e = add_entity(EntityType_Quad);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_texture(u32 texture, v2 pos, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Texture);
    if(e){
        e->dir = make_v2(1, 1);
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->dir = make_v2(0, 1);
        e->deg = 90;
        e->texture = texture;
    }
    else{
        print("Failed to add entity: Quad\n");
    }
    return(e);
}

static Entity*
add_ship(u32 texture, v2 pos, v2 dim, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Ship);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = -90;
        e->dir = make_v2(0, -1);
        e->accel_dir = make_v2(0, 0);
        e->speed = 200;
        e->velocity = 0;
        e->shoot_t = 1;
        e->texture = texture;
        e->collision_type = CollisionType_Explode;
        e->exploding = false;
        e->explosion_tex = TextureAsset_Explosion1;
        e->explosion_t = 0.0f;
        e->immune_t = 0.0f;
        e->immune = true;
        e->death_type = DeathType_Animate;
        e->health = 1;
        if(flags == 0){
            set_flags(&e->flags, EntityFlag_MoveWithCtrls | EntityFlag_CanCollide | EntityFlag_CanShoot | EntityFlag_Wrapping);
        }
        else{
            set_flags(&e->flags, flags);
        }
    }
    else{
        print("Failed to add entity: Ship\n");
    }
    return(e);
}

static Entity*
add_bullet(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Bullet);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = deg;
        e->dir = dir_from_deg(deg);
        e->speed = 500;
        e->velocity = 1;
        e->damage = 50;
        e->texture = texture;
        e->collision_type = CollisionType_SplinterOnDeath;
        e->death_type = DeathType_Particle;
        e->health = 1;
        if(flags == 0){
            set_flags(&e->flags, EntityFlag_MoveWithPhys | EntityFlag_CanCollide | EntityFlag_IsProjectile);
        }
        else{
            set_flags(&e->flags, flags);
        }
    }
    else{
        print("Failed to add entity: Bullet\n");
    }
    return(e);
}

static Entity*
add_asteroid(u32 texture, v2 pos, v2 dim, f32 deg, RGBA color, u32 flags){
    Entity* e = add_entity(EntityType_Asteroid);
    if(e){
        e->color = color;
        e->pos = pos;
        e->dim = dim;
        e->deg = deg;
        e->dir = dir_from_deg(deg);
        e->speed = 200;
        e->rot_speed = (f32)random_range_u32(150) + 50;
        e->velocity = 1;
        e->health = (s32)dim.w;
        e->texture = texture;
        e->collision_type = CollisionType_HealthOrSplinter;
        e->death_type = DeathType_Crumble;
        if(flags == 0){
            set_flags(&e->flags, EntityFlag_MoveWithPhys | EntityFlag_CanCollide | EntityFlag_Wrapping);
        }
        else{
            set_flags(&e->flags, flags);
        }
    }
    else{
        print("Failed to add entity: Asteroid\n");
    }
    return(e);
}

static void
entities_clear(){
    pm->free_entities_at = ENTITIES_MAX - 1;
    for(u32 i = pm->free_entities_at; i <= pm->free_entities_at; --i){
        Entity* e = pm->entities + i;
        e->type = EntityType_None;
        clear_flags(&e->flags, EntityFlag_Active);
        pm->free_entities[i] = pm->free_entities_at - i;
        pm->generation[i] = 0;
    }
    pm->entities_count = 0;
}

static void
serialize_data(String8 filename){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_WRITE, CREATE_NEW);
    if(file.handle != INVALID_HANDLE_VALUE){
        os_file_write(file, pm->entities, sizeof(Entity) * ENTITIES_MAX);
    }
    else{
        // log error
        print("Save file \"%s\" already exists: Could not serialize data\n", filename.str);
    }

    os_file_close(file);
    end_scratch(scratch);
}

static void
deserialize_data(String8 filename){
    ScratchArena scratch = begin_scratch();
    String8 full_path = str8_path_append(scratch.arena, saves_path, filename);

    File file = os_file_open(full_path, GENERIC_READ, OPEN_EXISTING);
    if(!file.size){
        //todo: log error
        print("Error: failed to open file <%s>\n", full_path.str);
        os_file_close(file);
        end_scratch(scratch);
        return;
    }

    String8 data = os_file_read(&pm->arena, file);
    entities_clear();

    u32 offset = 0;
    while(offset < data.size){
        Entity* e = (Entity*)(data.str + offset);
        switch(e->type){
            case EntityType_Ship:{
                Entity* ship = add_entity(EntityType_Ship);
                *ship = *e;
                ship->texture = TextureAsset_Ship;

                pm->ship = ship;
                pm->ship_loaded = true;
            } break;
            case EntityType_Bullet:{
                Entity* bullet = add_entity(EntityType_Bullet);
                *bullet = *e;
                bullet->texture = TextureAsset_Bullet;
            } break;
            case EntityType_Asteroid:{
                Entity* ast = add_entity(EntityType_Asteroid);
                *ast = *e;
                ast->texture = TextureAsset_Asteroid;
            } break;
        }
        offset += sizeof(Entity);
    }
    os_file_close(file);
    end_scratch(scratch);
}

static bool
handle_global_events(Event event){
    if(event.type == QUIT){
        should_quit = true;
        return(true);
    }
    if(event.type == MOUSE){
        v2 p0 = make_v2(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
        v2 p1 = make_v2((f32)controller.mouse.pos.x, (f32)controller.mouse.pos.y);
        v2 direction = direction_v2(p0, p1);

        f32 rad = rad_from_dir(direction);
        f32 deg = deg_from_dir(direction);
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ESCAPE){
                should_quit = true;
            }
            if(event.keycode == KeyCode_TILDE && !event.repeat){
                if(event.shift_pressed){
                    if(console.state == OPEN_BIG){
                        console_set_state(CLOSED);
                    }
                    else{
                        console_set_state(OPEN_BIG);
                    }
                }
                else{
                    if(console.state == OPEN || console.state == OPEN_BIG){
                        console_set_state(CLOSED);
                    }
                    else{
                        console_set_state(OPEN);
                    }

                }
                return(true);
            }
        }
        else{
        }
    }
    return(false);
}

static bool
handle_camera_events(Event event){
    if(event.type == MOUSE){
        controller.mouse.pos = event.mouse_pos;
        controller.mouse.dx = event.mouse_dx;
        controller.mouse.dy = event.mouse_dy;
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == KeyCode_ONE){
                if(pm->game_mode != GameMode_FirstPerson){
                    pm->game_mode = GameMode_FirstPerson;

                    controller.mouse.dx = 0;
                    controller.mouse.dy = 0;

                    POINT center = {(window.width/2), (window.height/2)};
                    ClientToScreen(window.handle, &center);
                    show_cursor(false);
                }
            }
            if(event.keycode == KeyCode_TWO){
                if(pm->game_mode != GameMode_Editor){
                    pm->game_mode = GameMode_Editor;
                    show_cursor(true);
                }
            }
            if(event.keycode == KeyCode_THREE){
                if(pm->game_mode != GameMode_Game){
                    pm->game_mode = GameMode_Game;
                    init_camera();
                    show_cursor(false);
                }
            }
        }
    }
    return(false);
}

static bool
handle_controller_events(Event event){
    if(event.type == MOUSE){
        controller.mouse.pos = event.mouse_pos;
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(!event.repeat){
                controller.button[event.keycode].pressed = true;
            }
            controller.button[event.keycode].held = true;
        }
        else{
            controller.button[event.keycode].held = false;
        }
    }
    return(false);
}

static void
reset_game(){
    pm->lives = MAX_LIVES;
    pm->score = 0;
    pm->level_index = 0;
    init_levels();
    pm->current_level = &pm->levels[0];

    reset_ship();
}

static void
reset_ship(){
    pm->ship->dir = make_v2(0, -1);
    pm->ship->pos = make_v2(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    pm->ship->deg = -90;
    pm->ship->dir = dir_from_deg(pm->ship->deg);
    pm->ship->accel_dir = make_v2(0, 0);
    pm->ship->velocity = 0;
    pm->ship->exploding = false;
    pm->ship->explosion_tex = TextureAsset_Explosion1;
    pm->ship->texture = TextureAsset_Ship;
    pm->ship->immune = true;
    pm->ship->immune_t = 0.0f;
    pm->ship->health = 1;
    set_flags(&pm->ship->flags, EntityFlag_Active | EntityFlag_MoveWithCtrls | EntityFlag_CanCollide | EntityFlag_CanShoot | EntityFlag_Wrapping);
}

static bool
game_won(){
    if(pm->level_index < MAX_LEVELS){
        return(false);
    }

    Level level = pm->levels[pm->level_index];
    if(level.asteroid_destroyed < level.asteroid_spawned){
        return(false);
    }

    return(true);
}

static void
update_game(Window* window, Memory* memory, Events* events){

    // NOTE: process events.
    while(!events_empty(events)){
        begin_timed_scope("events");
        Event event = events_next(events);

        bool handled;
        handled = handle_global_events(event);

        if(console_is_open()){
            handled = handle_console_events(event);
        }
        else{
            handled = handle_camera_events(event);
            handled = handle_controller_events(event);
        }
    }

    if(controller.button[KeyCode_R].pressed){
        reset_game();
        for(s32 i=0; i < array_count(pm->entities); i++){
            Entity* e = pm->entities + i;
            if(has_flags(e->flags, EntityFlag_Active) && e != pm->ship){
                remove_entity(e);
            }
        }
    }

    //----constant buffer----
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
    ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
    constants->screen_res = make_v2s32(window->width, window->height);
    d3d_context->Unmap(d3d_constant_buffer, 0);

    console_update();
    if(pm->game_mode == GameMode_Game && !game_won()){
        Level* level   = pm->current_level;
        Entity* ship   = pm->ship;
        Rect ship_rect = rect_from_entity(ship);

        if(ship->immune_t < 2){
            ship->immune_t += (f32)clock.dt;
        }
        else{
            ship->immune = false;
        }

        // ship behavior
        if(has_flags(ship->flags, EntityFlag_Active)){
            if(has_flags(ship->flags, EntityFlag_MoveWithCtrls)){
                ship->pos.x += (ship->accel_dir.x * ship->velocity * ship->speed) * (f32)clock.dt;
                ship->pos.y += (ship->accel_dir.y * ship->velocity * ship->speed) * (f32)clock.dt;

                if(controller.button[KeyCode_SPACEBAR].held){
                    ship->shoot_t += (f32)clock.dt;
                    if(ship->shoot_t >= 0.1f){
                        ship->shoot_t = 0.0;
                        v2 pos = make_v2(ship->pos.x + (50 * ship->dir.x), ship->pos.y + (50 * ship->dir.y));
                        Entity* child_e = add_bullet(TextureAsset_Bullet, pos, make_v2(40, 8), ship->deg);
                        child_e->origin = ship;

                        // play rail audio
                        u32 random_rail = random_range_u32(5) + 5; // hard coded for now, 5 rails
                        audio_play(random_rail, 0.1f, false);
                    }
                }
                else{
                    ship->shoot_t = 1;
                }

                // rotate left right
                if(controller.button[KeyCode_RIGHT].held || controller.button[KeyCode_D].held){
                    f32 d = deg_from_dir(ship->dir);
                    d += 200 * (f32)clock.dt;
                    ship->deg = d;
                    ship->dir = dir_from_deg(d);
                }
                if(controller.button[KeyCode_LEFT].held || controller.button[KeyCode_A].held){
                    f32 d = deg_from_dir(ship->dir);
                    d -= 200 * (f32)clock.dt;
                    ship->deg = d;
                    ship->dir = dir_from_deg(d);
                }

                // increase velocity
                if(controller.button[KeyCode_UP].held || controller.button[KeyCode_W].held){
                    ship->accelerating = true;
                    ship->velocity += (f32)clock.dt;
                    if(ship->velocity > 1){
                        ship->velocity = 1;
                    }
                    ship->accel_dir.x += ship->dir.x/100;
                    ship->accel_dir.y += ship->dir.y/100;
                    clamp_f32(-1, 1, &ship->accel_dir.x);
                    clamp_f32(-1, 1, &ship->accel_dir.y);
                }
                else{
                    ship->accelerating = false;
                    ship->velocity -= (f32)clock.dt/4;
                    if(ship->velocity < 0){
                        ship->velocity = 0;
                    }
                }
            }
        }

        // advance level
        if(level->asteroid_spawned == level->asteroid_count_max &&
           level->asteroid_spawned == level->asteroid_destroyed){
            pm->level_index++;
            if(pm->level_index < MAX_LEVELS){
                pm->current_level = &pm->levels[pm->level_index];
            }
        }

        // spawn asteroids
        pm->spawn_t += clock.dt;
        if(pm->spawn_t >= 0.5f){
            pm->spawn_t = 0.0;

            v2 dim;
            dim.x = (f32)random_range_u32(150) + 50;
            dim.y = dim.x;
            u32 side = random_range_u32(3);

            v2 pos = {0, 0};
            f32 deg = 0;
            if(side == 0){
                pos.x = -200;
                pos.y = (f32)random_range_u32(SCREEN_HEIGHT - 1);
                deg = (f32)random_range_u32(180) - 90.0f;
            }
            if(side == 1){
                pos.x = SCREEN_WIDTH + 200;
                pos.y = (f32)random_range_u32(SCREEN_HEIGHT - 1);

                s32 sign = (s32)random_range_u32(1);
                if(sign == 0){
                    sign = -1;
                }
                deg = ((f32)random_range_u32(90) + 90.0f) * (f32)sign;
            }
            if(side == 2){
                pos.x = (f32)random_range_u32(SCREEN_WIDTH - 1);
                pos.y = -200;

                deg = (f32)random_range_u32(180);
            }
            if(side == 3){
                pos.x = (f32)random_range_u32(SCREEN_WIDTH - 1);
                pos.y = SCREEN_HEIGHT + 200;

                deg = (f32)random_range_u32(180) - 180;
            }
            if(pm->current_level->asteroid_spawned < pm->current_level->asteroid_count_max){
                Entity* asteroid = add_asteroid(TextureAsset_Asteroid, pos, dim, deg);
                pm->current_level->asteroid_spawned++;
            }
        }

        // 64 byte cache lines
        // resolve entity motion
        for(s32 i = 0; i < array_count(pm->entities); ++i){
            begin_timed_scope("entity_motion");
            Entity *e = pm->entities + i;
            if(!has_flags(e->flags, EntityFlag_Active)){
                continue;
            }

            if(has_flags(e->flags, EntityFlag_MoveWithPhys)){
                e->pos.x += (e->dir.x * e->velocity * e->speed) * (f32)clock.dt;
                e->pos.y += (e->dir.y * e->velocity * e->speed) * (f32)clock.dt;
            }

            if(has_flags(e->flags, EntityFlag_Wrapping)){
                if(e->pos.x - e->dim.w/2 > SCREEN_WIDTH){
                    e->pos.x = 0 - e->dim.w/2;
                }
                if(e->pos.y - e->dim.h/2 > SCREEN_HEIGHT){
                    e->pos.y = 0 - e->dim.h/2;
                }
                if(e->pos.x + e->dim.w/2 < 0){
                    e->pos.x = SCREEN_WIDTH + e->dim.w/2;
                }
                if(e->pos.y + e->dim.h/2 < 0){
                    e->pos.y = SCREEN_HEIGHT + e->dim.h/2;
                }
            }
            else{
                if((e->pos.x + e->dim.w/2 < 0)             ||
                   (e->pos.y + e->dim.h/2 < 0)             ||
                   (e->pos.x - e->dim.w/2 > SCREEN_WIDTH)  ||
                   (e->pos.y - e->dim.h/2 > SCREEN_HEIGHT)){
                   remove_entity(e);
                }
            }
        }

        // resolve death
        for(s32 i = 0; i < array_count(pm->entities); ++i){
            begin_timed_scope("entity_motion");
            Entity *e = pm->entities + i;
            if(!has_flags(e->flags, EntityFlag_Active)){
                continue;
            }

            if(e->health <= 0){
                switch(e->death_type){
                    case DeathType_Crumble:{
                        if(e->dim.w > 100){
                            e->dim.w -= 50;
                            e->dim.h -= 50;
                            for(s32 splint_i=0; splint_i < 3; ++splint_i){
                                e->deg = random_range_f32(360);
                                add_asteroid(TextureAsset_Asteroid, e->pos, e->dim, e->deg);
                                pm->current_level->asteroid_spawned++;
                                pm->current_level->asteroid_count_max++;
                            }
                        }

                        pm->score += (u32)e->dim.w;
                        pm->current_level->asteroid_destroyed++;
                        remove_entity(e);
                    } break;
                    case DeathType_Particle:{
                        s32 p_count = (s32)random_range_u32(5) + 5;
                        for(s32 p_idx = 0; p_idx < p_count; p_idx++){
                            f32 deg = random_range_f32(360);
                            v2 dim = make_v2(10, 2);
                            Entity* e_p = add_bullet(TextureAsset_Bullet, e->pos, dim, deg);
                            clear_flags(&e_p->flags, EntityFlag_CanCollide);
                            set_flags(&e_p->flags, EntityFlag_Particle);
                            e_p->parent = pm->ship;
                            e_p->particle_t = 0.175f;
                        }
                        remove_entity(e);
                    } break;
                    case DeathType_Animate:{
                    } break;
                }
            }
        }

        // resolve particles
        for(s32 i = 0; i < array_count(pm->entities); ++i){
            begin_timed_scope("flag_loop");
            Entity *e = pm->entities + i;
            if(!has_flags(e->flags, EntityFlag_Active)){
                continue;
            }

            if(has_flags(e->flags, EntityFlag_Particle)){
                if(e->dim.w > 0){
                    e->dim.w -= (f32)clock.dt * 40;
                }
                else{
                    remove_entity(e);
                }
            }
        }
            //Rect e_rect = rect_from_entity(e);
            //if(!has_flags(e, EntityFlag_CanCollide)){
            //    for(s32 j = 0; j < array_count(pm->entities); ++j){
            //        Entity *e_inner = pm->entities + j;
            //        if(e == e_inner->parent){
            //            continue;
            //        }

            //        Rect e_inner_rect = rect_from_entity(e_inner);
            //        if(rect_collides_rect(e_inner_rect, e_rect)){

            //        }
            //    }
            //}

        // type loop
        for(s32 i = 0; i < array_count(pm->entities); ++i){
            begin_timed_scope("type_loop");
            Entity *e = pm->entities + i;
            Rect e_rect = rect_from_entity(e);

            switch(e->type){
                case EntityType_Ship:{

                    if(has_flags(pm->ship->flags, EntityFlag_Active)){
                        if(e->exploding){
                            e->accelerating = false;
                            e->texture = e->explosion_tex;
                            e->explosion_t += (f32)clock.dt;
                            if(e->explosion_t >= 0.030f){
                                if(e->texture != TextureAsset_Explosion6){
                                    e->explosion_tex++;
                                }
                                else{
                                    if(pm->lives){
                                        reset_ship();
                                    } else{
                                        clear_flags(&pm->ship->flags, EntityFlag_Active);
                                    }
                                }
                                e->explosion_t = 0.0f;
                            }
                        }
                        else{

#if 1
                            if(!e->immune){
                                for(s32 idx = 0; idx < array_count(pm->entities); ++idx){
                                    Entity *collide_e = pm->entities + idx;
                                    if(collide_e->type == EntityType_Asteroid){
                                        Rect collide_e_rect = rect_from_entity(collide_e);
                                        if(rect_collides_rect(e_rect, collide_e_rect)){
                                            pm->lives -= 1;
                                            e->health = 0;
                                            e->exploding = true;
                                            clear_flags(&e->flags, EntityFlag_MoveWithCtrls);
                                            pm->current_level->asteroid_destroyed++;
                                            remove_entity(collide_e);
                                            break;
                                        }
                                    }
                                }
                            }
#endif
                        }
                    }
                } break;
                case EntityType_Bullet:{
                    Rect rect_e = rect_from_entity(e);
                    if(!has_flags(e->flags, EntityFlag_CanCollide)){
                        break;
                    }
                    for(s32 idx = 0; idx < array_count(pm->entities); ++idx){
                        Entity *collide_e = pm->entities + idx;

                        if(collide_e == e->origin){
                            continue;
                        }
                        if(collide_e->type == EntityType_Asteroid){
                            Rect collide_e_rect = rect_from_entity(collide_e);

                            if(rect_collides_rect(collide_e_rect, rect_e)){

                                collide_e->health -= e->damage;
                                collide_e->color.r += 0.2f;
                                collide_e->color.g -= 0.4f;
                                collide_e->color.b -= 0.4f;

                                e->health = 0;
                                break;
                            }
                        }
                    }
                } break;
                case EntityType_Asteroid:{
                    e->deg += e->rot_speed * (f32)clock.dt;
                } break;
            }
        }
    }
}

#endif

