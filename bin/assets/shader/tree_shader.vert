
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 _uv;

flat out int index;
out vec2 uv;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(aPos.x, aPos.y, 0.0, 1.0); 
	index = int(aPos.z);
	uv = _uv;
}