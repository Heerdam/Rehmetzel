/**
#version 330 core
layout (location = 0) in vec2 a_Pos;
layout (location = 1) in float a_index;
layout (location = 2) in float a_typ;
layout (location = 3) in vec2 a_uv;
layout (location = 4) in vec4 a_col1;
layout (location = 5) in vec4 a_col2;

flat out int index;
flat out int type;

out vec2 uv;
out vec4 col1;
out vec4 col2;
uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(a_Pos.x, a_Pos.y, 0.0, 1.0);
	index = int(a_index);
	type = int(a_typ);
	uv = a_uv;
	col1 = a_col1;
	col2 = a_col2;
}*/