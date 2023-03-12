#version 460 core
layout(location = 0) out vec4 color;
in vec4 out_color;

void main()
{
	color = out_color;
};
