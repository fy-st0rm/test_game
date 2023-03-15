#include "game.h"
#include "iso_camera/iso_camera.h"
#include "iso_math.h"

entity* entity_new(iso_vec4 rect, iso_color color, entity_type type) {
	entity* ent = iso_alloc(sizeof(entity));
	ent->rect   = rect;
	ent->color  = color;
	ent->type   = type;
	return ent;
}

void entity_delete(entity* ent) {
	iso_free(ent);
}

void entity_submit_to_ren(entity* ent, renderer_t* ren) {
	iso_vec4 rect = ent->rect;
	iso_color col = ent->color;
	renderer_push_vertex(ren, (vertex_t) {  rect.x,  rect.y,                   0.0f, col.r, col.g, col.b, col.a });
	renderer_push_vertex(ren, (vertex_t) {  rect.x,  rect.y + rect.w,          0.0f, col.r, col.g, col.b, col.a });
	renderer_push_vertex(ren, (vertex_t) {  rect.x + rect.z,  rect.y + rect.w, 0.0f, col.r, col.g, col.b, col.a });
	renderer_push_vertex(ren, (vertex_t) {  rect.x + rect.z,  rect.y,          0.0f, col.r, col.g, col.b, col.a });
}

b8 entity_has_collided(entity* a, entity* b) {
	iso_vec4 ar = a->rect;
	iso_vec4 br = b->rect;

	if ((ar.x < br.x && br.x < ar.x + ar.z) ||
			(br.x < ar.x && ar.x < br.x + br.z)) {
		if ((br.y < ar.y && ar.y < br.y + br.w) ||
				(ar.y < br.y && br.y < ar.y + ar.w)) {
			return true;
		}
	}
	return false;
}

game_t* game_new(iso_app* app) {
	game_t* game = iso_alloc(sizeof(game_t));
	game->app = app;
	game->ren = renderer_new(app);
	game->speed = 0.5f;
	game->vert_force = 0.0f;

	srand(time(NULL));

	// Camera
	game->cam = app->camera_man->api.camera_new(
		app->camera_man,
		(iso_camera_def) {
			.name = CAM,
			.pos  = (iso_vec3) { 0, 0, 0 },
			.rot  = (iso_rotation) { .angle = 0.0f, .axes = (iso_vec3) { 0, 0, 1 } },
			.type = ISO_ORTHOGRAPHIC_CAMERA,
			.ortho_viewport = (iso_camera_ortho_viewport_def) {
				.left   = 0.0f,
				.right  = WIN_WIDTH,
				.top    = WIN_HEIGHT,
				.bottom = 0.0f,
				.near   = -1.0f,
				.far    = 1000.0f
			}
		}
	);

	// Player
	game->player = entity_new(
			(iso_vec4) {PLAYER_POS_X, PLAYER_POS_Y, PLAYER_SIZE, PLAYER_SIZE},
			(iso_color) {1.0f, 1.0f, 1.0f, 1.0f},
			PLAYER
	);

	// Start Platform
	i32 x = 0;
	game->platforms[0] = entity_new(
			(iso_vec4) { x, 0, WIN_WIDTH * 2, 200},
			(iso_color) { 1, 1, 1, 1 },
			START_PLATFORM
	);
	x += WIN_WIDTH * 2 + PLATFORM_GAP;

	// Platforms
	for (i32 i = 1; i < PLATFORM_AMT; i++) {
		f32 w = rand_range(PLATFORM_MIN_WIDTH, PLATFORM_MAX_WIDTH);
		f32 h = rand_range(PLATFORM_MIN_HEIGHT, PLATFORM_MAX_HEIGHT);
		game->platforms[i] = entity_new(
			(iso_vec4) {x, 0, w, h},
			(iso_color) {1, 1, 1, 1},
			PLATFORM
		);
		x += w + PLATFORM_GAP;
	}

	return game;
}

void game_delete(game_t* game) {
	renderer_delete(game->ren);
	game->app->camera_man->api.camera_delete(game->app->camera_man, CAM);

	// Deleting entities
	entity_delete(game->player);
	for (i32 i = 0; i < PLATFORM_AMT; i++) {
		entity_delete(game->platforms[i]);
	}

	iso_free(game);
}

void game_event(game_t* game, SDL_Event event) {
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
			case SDLK_SPACE: {
				if (game->airtime < 16) game->vert_force = -1.5f;
				break;
			}
		}
	}
}

void game_update(game_t* game, f32 dt) {
	renderer_begin(game->ren);

	// Updating camera
	{
		game->app->camera_man->api.camera_update(game->app->camera_man, CAM);
		iso_mat4 mvp = game->app->camera_man->memory.get_camera(game->app->camera_man, CAM)->mvp;

		// Sending uniform
		game->app->graphics->api.uniform_set(
			game->app->graphics,
			(iso_graphics_uniform_def) {
				.name   = "mvp",
				.shader = game->app->graphics->memory.get_shader(game->app->graphics, SHADER),
				.data   = &mvp,
				.type   = ISO_GRAPHICS_UNIFORM_MAT4
			}
		);
	}

	// Updating player
	{
		iso_vec2 movement = { 1, 0 };

		// Adding vertical force
		movement.y -= game->vert_force * dt;
		game->vert_force += GRAVITY;
		if (game->vert_force > 1) game->vert_force = 1;

		// Horizontal collision check
		for (i32 i = 0; i < PLATFORM_AMT; i++) {
			if (entity_has_collided(game->player, game->platforms[i])) {
				iso_vec4 a = game->player->rect;
				iso_vec4 b = game->platforms[i]->rect;

				if (movement.x > 0) {
					a.x = b.x - a.z;
				}
				else if (movement.x < 0) {
					a.x = b.x + b.z;
				}

				game->player->rect = a;
			}
		}

		// Vertical collision check
		b8 land = false;
		game->player->rect.y += movement.y;
		for (i32 i = 0; i < PLATFORM_AMT; i++) {
			if (entity_has_collided(game->player, game->platforms[i])) {
				iso_vec4 a = game->player->rect;
				iso_vec4 b = game->platforms[i]->rect;

				if (movement.y < 0) {
					a.y = b.y + b.w;
					land = true;
				}
				else if (movement.y > 0) {
					a.y = b.y - a.w;
				}

				game->player->rect = a;
			}
		}

		if (land) game->airtime = 0;
		else game->airtime++;

		// Gameover
		if (game->player->rect.x < 0 || game->player->rect.y < 0) {
			//game->app->state = ISO_APP_CLOSED;
		}
	}

	// Updating platforms
	{
		// Sorting platforms wrt x
		for (i32 i = 0; i < PLATFORM_AMT; i++) {
			for (i32 j = i + 1; j < PLATFORM_AMT; j++) {
				if (game->platforms[i]->rect.x > game->platforms[j]->rect.x) {
					entity* tmp = game->platforms[i];
					game->platforms[i] = game->platforms[j];
					game->platforms[j] = tmp;
				}
			}
		}

		// Moving platforms
		for (i32 i = 0; i < PLATFORM_AMT; i++) {
			entity* ent = game->platforms[i];
			ent->rect.x -= game->speed * dt;

			if (ent->rect.x + ent->rect.z <= 0 && ent->type != START_PLATFORM) {
				entity* last = game->platforms[PLATFORM_AMT - 1];
				ent->rect.x = last->rect.x + last->rect.z + PLATFORM_GAP;
				ent->rect.z = rand_range(PLATFORM_MIN_WIDTH, PLATFORM_MAX_WIDTH);
				ent->rect.w = rand_range(PLATFORM_MIN_HEIGHT, PLATFORM_MAX_HEIGHT);
			}
		}
	}

	// Rendering entities
	{
		entity_submit_to_ren(game->player, game->ren);
		for (i32 i = 0; i < PLATFORM_AMT; i++) {
			entity_submit_to_ren(game->platforms[i], game->ren);
		}
	}

	renderer_end(game->ren);
}
