#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

out vec4 out_color;
uniform mat4 mvp;
void main()
{
	out_color = in_color;
	gl_Position = mvp * vec4(in_position, 1);
};
