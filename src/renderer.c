#include "renderer.h"

renderer_t* renderer_new(iso_app* app) {
	renderer_t* ren = iso_alloc(sizeof(renderer_t));
	ren->app = app;
	ren->buff_sz = 0;

	// Generating indices
	u32 indices[MAX_IDX_CNT];
	for (int offset = 0, i = 0; i < MAX_IDX_CNT; i += 6) 
	{
		indices[0 + i] = 0 + offset;
		indices[1 + i] = 1 + offset;
		indices[2 + i] = 2 + offset;

		indices[3 + i] = 2 + offset;
		indices[4 + i] = 3 + offset;
		indices[5 + i] = 0 + offset;
		offset += 4;
	}

	// VBO
	ren->vbo = app->graphics->api.vertex_buffer_new(
		app->graphics,
		(iso_graphics_vertex_buffer_def) {
			.name = VBO,
			.size = MAX_VBO_SIZE,
			.data = NULL,
			.usage = ISO_GRAPHICS_DYNAMIC
		}
	);

	// IBO
	ren->ibo = app->graphics->api.index_buffer_new(
		app->graphics,
		(iso_graphics_index_buffer_def) {
			.name = IBO,
			.size = sizeof(indices),
			.data = indices,
			.usage = ISO_GRAPHICS_STATIC
		}
	);

	// Shader
	ren->shader = app->graphics->api.shader_new(
		app->graphics,
		(iso_graphics_shader_def) {
			.name = SHADER,
			.f_src = FRAG_SHADER,
			.v_src = VERT_SHADER,
			.source_type = ISO_SHADER_FROM_FILE
		}
	);

	// Creating pipline
	ren->pip = app->graphics->api.render_pipeline_new(
		app->graphics,
		(iso_graphics_render_pipeline_def) {
			.name = PIP,
			.buffers = {
				.vbo = ren->vbo,
				.ibo = ren->ibo,
				.shader = ren->shader
			},

			.amt = 2,
			.layout = (iso_graphics_vertex_layout_def[]) {
				{ .amt = 3, .type = ISO_GRAPHICS_FLOAT },
				{ .amt = 4, .type = ISO_GRAPHICS_FLOAT }
			}
		}
	);

	return ren;
}

void renderer_delete(renderer_t* ren) {
	ren->app->graphics->api.vertex_buffer_delete(ren->app->graphics, VBO);
	ren->app->graphics->api.index_buffer_delete(ren->app->graphics, IBO);
	ren->app->graphics->api.shader_delete(ren->app->graphics, SHADER);
	ren->app->graphics->api.render_pipeline_delete(ren->app->graphics, PIP);
	iso_free(ren);
}

void renderer_push_vertex(renderer_t* ren, vertex_t vert) {
	if ((ren->buff_sz / 7) / 4 >= MAX_QUAD_CNT) {
		renderer_end(ren);
		renderer_begin(ren);
	}
	ren->buffer[ren->buff_sz++] = vert.pos.x;
	ren->buffer[ren->buff_sz++] = vert.pos.y;
	ren->buffer[ren->buff_sz++] = vert.pos.z;
	ren->buffer[ren->buff_sz++] = vert.color.x;
	ren->buffer[ren->buff_sz++] = vert.color.y;
	ren->buffer[ren->buff_sz++] = vert.color.z;
	ren->buffer[ren->buff_sz++] = vert.color.w;
}

void renderer_begin(renderer_t* ren) {
	ren->buff_sz = 0;

	ren->app->graphics->api.render_pipeline_begin(ren->app->graphics, PIP);
}

void renderer_end(renderer_t* ren) {
	ren->app->graphics->api.vertex_buffer_update(
		ren->app->graphics,
		VBO,
		(iso_graphics_buffer_update_def) {
			.start_sz = 0,
			.end_sz   = ren->buff_sz * sizeof(f32),
			.data     = ren->buffer
		}
	);
	ren->app->graphics->api.render_pipeline_end(
		ren->app->graphics,
		PIP,
		((ren->buff_sz / 7) / 4) * 6
	);
}

void renderer_print_buffer(renderer_t *ren) {
	for (i32 i = 0; i < ren->buff_sz; i+=7) {
		printf("%f ", ren->buffer[i]);
		printf("%f ", ren->buffer[i+1]);
		printf("%f ", ren->buffer[i+2]);
		printf("%f ", ren->buffer[i+3]);
		printf("%f ", ren->buffer[i+4]);
		printf("%f ", ren->buffer[i+5]);
		printf("%f\n", ren->buffer[i+6]);
	}
}
