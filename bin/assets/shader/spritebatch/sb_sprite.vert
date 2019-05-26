
#version 460 core

layout (location = 0) in vec2 a_Pos;
layout (location = 1) in vec3 a_uv;
layout (location = 2) in vec4 a_col;
layout (location = 3) in vec4 a_sc;

out vec3 uv;
out vec4 col;
out vec4 sc;

void main() {
	gl_Position = vec4(a_Pos, 0.0, 1.0);
	uv = a_uv;
	col = a_col;
	sc = a_sc;
};