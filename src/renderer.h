#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "isolate.h"

#define VERT_SHADER "vert.glsl"
#define FRAG_SHADER "frag.glsl"

#define VBO "vbo"
#define IBO "ibo"
#define SHADER "shader"
#define PIP "pip"

typedef struct {
	iso_vec3 pos;
	iso_vec4 color;
} vertex_t;

#define MAX_QUAD_CNT  1000
#define MAX_VERT_CNT  MAX_QUAD_CNT  * 4
#define MAX_BUFF_SIZE MAX_VERT_CNT  * 7
#define MAX_VBO_SIZE  MAX_BUFF_SIZE * sizeof(f32)
#define MAX_IDX_CNT   MAX_QUAD_CNT  * 6
#define MAX_IBO_SIZE  MAX_IDX_CNT   * sizeof(u32)

typedef struct {
	iso_app* app;

	iso_graphics_vertex_buffer* vbo;
	iso_graphics_index_buffer* ibo;
	iso_graphics_shader* shader;
	iso_graphics_render_pipeline* pip;

	float buffer[MAX_BUFF_SIZE];
	u32   buff_sz;
} renderer_t;

renderer_t* renderer_new(iso_app* app);
void        renderer_delete(renderer_t* ren);

void        renderer_push_vertex(renderer_t* ren, vertex_t vert);
void        renderer_begin(renderer_t* ren);
void        renderer_end(renderer_t* ren);

void        renderer_print_buffer(renderer_t* ren);

#endif // __RENDERER_H__
