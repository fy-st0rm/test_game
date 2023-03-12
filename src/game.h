#ifndef __GAME_H__
#define __GAME_H__

#include "isolate.h"
#include "renderer.h"

typedef struct {
	iso_app* app;
	renderer_t* renderer;
} game_t;

game_t* game_new(iso_app* app);
void    game_delete(game_t* game);
void    game_update(game_t* game);

#endif // __GAME_H__
