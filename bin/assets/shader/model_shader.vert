
#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normals;
layout(location = 2) in vec2 a_uv;
layout(location = 3) in float a_matIndex;

uniform mat4 camera;

out vec3 normal;
out vec2 uv;
out uint matIndex;

void main(){
	gl_Position = camera * vec4(a_position.xyz, 1.f);
	normal = normalize(a_normals);
	uv = a_uv;
	matIndex = uint(a_matIndex);
};