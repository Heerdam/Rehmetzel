
#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec3 a_uv;
layout (location = 3) in vec3 a_tang;
layout (location = 4) in vec3 a_bitang;

layout (location = 5) uniform mat4 combined;

out vec3 uv;
out vec3 norm;

void main() {

	gl_Position = combined * vec4(a_pos, 1.f);
	uv = a_uv;
	norm = a_norm;
};