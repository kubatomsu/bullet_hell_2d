
#include "game.h"
#include "graphics.h"
#include "entity.h"
#include "input.h"
#include "engine.h"
#include "m_math.h"

unsigned int player_texture_run_horizontal;
unsigned int player_texture_run_up;
unsigned int player_texture_run_down;
unsigned int player_fist_texture;
unsigned int enemy_texture;
int player_entity_id = -1;
int enemy_count = 0;
entity_t* player_entity;


// id = unique number for each entity.
// entity = pointer to entity that we should update
void exaple_update_function(int id, entity_t* entity) {
    
    // all entity variables can be found in entity.cpp in struct entity_t
    entity->position.x += 0.1f;
    
}

/* PLAYER */
m_v2 camera_shake;
float camera_shake_strength;
const float camera_shake_length = 6.0f;
const float camera_shake_freq = 2;
m_v2 player_cursor = {0.5,0.5};
float fist_time = 0;
const float mouse_speed = 0.01;

void player_fist_update(int id, entity_t* entity) {
    
    if(!player_entity) return;
    
    player_cursor += input_mouse_offset* mouse_speed * m_v2{1,-1};
    m_v2 cursor_ndir = m_v2_normalize(player_cursor);
    player_cursor = cursor_ndir * 2;
    cursor_pos = player_entity->position + player_cursor;
    fist_time -= delta_time;
    entity->velocity = entity->velocity / (1.0f + (delta_time * 25));
    if(input_pressed(GLFW_MOUSE_BUTTON_1)) {
        entity->velocity += cursor_ndir * 150;
        camera_shake_strength += 1;
        entity->color *= 8;
        fist_time = 0.2f;
    }
    entity->color = m_v3_lerp(entity->color, {1,1,1}, delta_time * 10);
    
    /*
    entity->position = m_v2_lerp(entity->position, player_entity->position + player_cursor, delta_time* 15);*/
    
    entity->velocity += ((player_entity->position + player_cursor) - entity->position) * 3;
    entity->velocity += player_entity->velocity * 0.05f;
    
    entity->position += entity->velocity * delta_time;
}

void player_fist_on_collision(int id, entity_t* entity, int other_id, entity_t* other_entity) {
    if(other_id == player_entity_id || other_entity->flags.collision_static || fist_time < 0) return;
    
    other_entity->velocity += m_v2_normalize(player_cursor) * 5;
    other_entity->color *= 4;
}

void player_update(int id, entity_t* entity) { 
    m_v2 dir = {};
    if (entity->health <= 0) game_load_level(); /**Game crashes on load level**/
    
#define  PLAYER_ON_RUN entity->texture_scale.x = 1.0f / 4.0f;
#define PLAYER_ON_STOP_RUNNING entity->texture_scale.x = 0.25f; entity->time = 0;
    
    
    
    if(input_down(GLFW_KEY_W)){
        dir.y++;
        entity->texture = player_texture_run_up;
        PLAYER_ON_RUN;
    }
    if(input_down(GLFW_KEY_S)){
        dir.y--;
        entity->texture = player_texture_run_down;
        PLAYER_ON_RUN;
    }
    if(input_down(GLFW_KEY_D)){
        dir.x++;
        entity->scale.x = 1;
        entity->texture = player_texture_run_horizontal;
        PLAYER_ON_RUN;
    }
    if(input_down(GLFW_KEY_A)){
        dir.x--;
        entity->scale.x = -1;
        entity->texture = player_texture_run_horizontal;
        PLAYER_ON_RUN;
        
    }
    if(input_down(GLFW_KEY_X)){
        enemy_count = 0;
    }
    
    
    
    
    entity->velocity = entity->velocity / ( 1.0f + (delta_time * 18.0f));
    entity->velocity += dir * delta_time * 300.0f;
    
    if(input_pressed(GLFW_KEY_SPACE)) entity->velocity += dir * 180.0f;
    
    entity->position += entity->velocity * delta_time;
    
    if(dir != m_v2{}) {
        if(entity->time > 1.0f){
            entity->texture_offset.x += 1.0f;
            entity->time = 0;
            entity->mode = 0;
        }
        entity->time += delta_time * 14;
    }else{
        entity->texture_offset.x = entity->texture == player_texture_run_horizontal ? 0.0 : 1.0f;
        entity->time = 0;
    }
    
    
    camera_shake = m_v2{
        sinf(engine_time * 20.2345 * camera_shake_freq)*2 + sinf(engine_time *camera_shake_freq * 58.2342) + cosf(-engine_time * camera_shake_freq * 234.2312) + cosf(-engine_time*camera_shake_freq * 2344.2312)/2,
        sinf(engine_time*camera_shake_freq * 18.987)*2 + sinf(-engine_time*camera_shake_freq * 73.2342) + cosf(-engine_time*camera_shake_freq * 194.2312) + cosf(-engine_time*camera_shake_freq * 2784.2312)/2,
    };
    camera_shake_strength *= 1.0f - (camera_shake_length * delta_time);
    
    
    main_camera.position = m_v2_lerp(
                                     main_camera.position,
                                     entity->position +
                                     entity->velocity * 0.1f +
                                     player_cursor * 0.35f + 
                                     camera_shake * camera_shake_strength,
                                     delta_time * 8.0f
                                     );
    
}
/* PICK UPS */
void pickup_on_collision(int id, entity_t* entity, int other_id, entity_t* other_entity)
{
    if (other_entity == player_entity){
        player_entity->health++;
        entity_destroy(id);
    }
}

/* ENEMIES */
//projectile
void projectile_update_1(int id, entity_t* entity)
{
    entity->time += delta_time;
    entity->position += entity->velocity*delta_time;
    entity->color -= m_v3{0.8,1,1} * delta_time * 2;
    if (entity->collision_count > 0 && entity-> time >= 0.2f){
        entity_destroy(id);
    }
    
}
void projectile_on_collision(int id, entity_t* entity, int other_id, entity_t* other_entity){
    if ( entity-> time >= 0.2f){other_entity->health--;}
}
//enemy
void enemy_update(int id, entity_t* entity) {
    entity->time += delta_time;
    //destroy entity on 0 health
    if (entity->health <= 0){
        //health pick up
        entity_t e = ENTITY_DEFAULT;
        e.position = entity->position;
        e.texture = enemy_texture;
        e.color = {1, 0, 0};
        e.scale = {0.5f,0.5f};
        enemy_count--;
        e.on_collision_func = pickup_on_collision;
        entity_spawn(e);
        entity_destroy(id);
    }
    if(entity->time > 1.0f){
        for(int i = 0; i < 1; i++) { //create projectile entity
            entity_t e = ENTITY_DEFAULT;
            const float bullet_speed = 5;
            e.velocity = m_v2_normalize(player_entity->position - entity->position)*bullet_speed;
            e.position =  entity->position+e.velocity/bullet_speed;
            e.update_func = projectile_update_1;
            e.time = 0;
            e.texture = enemy_texture;
            e.on_collision_func = projectile_on_collision;
            
            entity_spawn(e);
            entity->time = -5;
        }
    }
}
void spawn_enemys(int id, entity_t* entity){
    printf("%i---------------------------", enemy_count);
    if (enemy_count <= 0){
        // enemies
        for(int i = 0; i < 10; i++) {
            entity_t e = ENTITY_DEFAULT;
            e.position = m_randv2() * 10;
            e.update_func = enemy_update;
            e.texture = enemy_texture;
            e.health = 1;
            enemy_count++;
            entity_spawn(e);
        }
    }
}
/* GAME */

void game_initialize() {
    
    player_texture_run_down = texture_import("player_run_down.png", GL_NEAREST, GL_REPEAT);
    player_texture_run_up = texture_import("player_run_up.png", GL_NEAREST, GL_REPEAT);
    player_texture_run_horizontal = texture_import("player_run_horizontal.png", GL_NEAREST, GL_REPEAT);
    
    player_fist_texture = texture_import("fist.png", GL_LINEAR, GL_REPEAT);
    
    enemy_texture = texture_import("test.png", GL_LINEAR, GL_REPEAT);
    
}

void vel_update(int id, entity_t* entity) {
    entity->velocity = entity->velocity / (1.0f + (delta_time * 4));
    entity->position += entity->velocity * delta_time;
}

void game_load_level() {
    
    // player
    entity_t e = ENTITY_DEFAULT;
    e.position = { 0,-4 };
    e.texture = player_texture_run_up;
    e.update_func = player_update;
    e.color = {1,1,1};
    e.texture_scale = {0.25,1};
    e.health = 4;
    player_entity_id = entity_spawn(e);
    
    // player fist
    entity_t player_f = ENTITY_DEFAULT;
    player_f.position = e.position + m_v2{0,1};
    player_f.texture = player_fist_texture;
    player_f.update_func = player_fist_update;
    player_f.on_collision_func = player_fist_on_collision;
    entity_spawn(player_f);
    
    for(int i = 0; i < 10; i++) {
        entity_t e = ENTITY_DEFAULT;
        e.position = m_randv2() * 10;
        e.scale = {2,2};
        e.texture = 0;
        //e.flags.collision_static = 1;
        e.update_func = vel_update;
        entity_spawn(e);
    }
    // enemy spawner
    entity_t entity_spawner = ENTITY_DEFAULT;
    entity_spawner.update_func = spawn_enemys;
    entity_spawn(entity_spawner);
    
};

void game_early_update() {
    
}

void game_late_update() {
    if(player_entity_id >= 0) {
        player_entity = entity_get(player_entity_id);
    }
    
    printf("player ptr = 0x%x\n",(int)player_entity);
    
}

void game_on_render_update() {
    
    for(int i = 0; i < player_entity->health; i++) {
        draw_quad(
                  camera_ndc_to_world(&main_camera, { -0.95f + (float)i / 15.0f, 0.95}),
                  {(float)main_camera.distance * 0.04f,(float)main_camera.distance * 0.04f},
                  enemy_texture,
                  {1,1},
                  {},
                  {1,0,0}
                  );
    }
}