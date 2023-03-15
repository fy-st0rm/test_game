#ifndef __GAME_H__
#define __GAME_H__

#include "isolate.h"
#include "renderer.h"

#include <time.h>

#define rand_range(l, u) rand() % (u - l + 1) + l

#define WIN_WIDTH  800
#define WIN_HEIGHT 600
#define CAM "camera"

#define GRAVITY 0.1f

// Player constants
#define PLAYER_SIZE 50
#define PLAYER_POS_X (WIN_WIDTH / 2) - (PLAYER_SIZE / 2)
#define PLAYER_POS_Y (WIN_HEIGHT / 2) - (PLAYER_SIZE / 2)

// Platform constants
#define PLATFORM_AMT 5
#define PLATFORM_GAP 200

#define PLATFORM_MAX_WIDTH 400
#define PLATFORM_MIN_WIDTH 100

#define PLATFORM_MAX_HEIGHT 400
#define PLATFORM_MIN_HEIGHT 100

// Entity
typedef enum {
	PLAYER,
	START_PLATFORM,
	PLATFORM
} entity_type;

typedef struct {
	iso_vec4 rect;
	iso_color color;
	entity_type type;
} entity;

entity* entity_new(iso_vec4 rect, iso_color color, entity_type type);
void    entity_delete(entity* ent);
void    entity_submit_to_ren(entity* ent, renderer_t* ren);
b8      entity_has_collided(entity* a, entity* b);

// Game
typedef struct {
	iso_app* app;
	renderer_t* ren;
	iso_camera* cam;

	f32 speed;
	f32 vert_force;
	f32 airtime;

	entity* player;
	entity* platforms[PLATFORM_AMT];
} game_t;

game_t* game_new(iso_app* app);
void    game_delete(game_t* game);
void    game_update(game_t* game, f32 dt);
void    game_event(game_t* game, SDL_Event event);

#endif // __GAME_H__
