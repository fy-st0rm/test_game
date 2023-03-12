#include "game.h"

game_t* game_new(iso_app* app) {
	game_t* game = iso_alloc(sizeof(game_t));
	game->app = app;
	game->renderer = renderer_new(app);
	return game;
}

void game_delete(game_t* game) {
	renderer_delete(game->renderer);
	iso_free(game);
}

void game_update(game_t* game) {
	renderer_begin(game->renderer);

	renderer_push_vertex(game->renderer, (vertex_t) { -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f });
	renderer_push_vertex(game->renderer, (vertex_t) { -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f });
	renderer_push_vertex(game->renderer, (vertex_t) {  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f });
	renderer_push_vertex(game->renderer, (vertex_t) {  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f });

	renderer_end(game->renderer);
}
