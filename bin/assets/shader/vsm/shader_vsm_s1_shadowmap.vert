
#version 460 core

layout(location = 0) in vec3 a_position;

out vec4 position;

layout(location = 1) uniform mat4 lightSpaceMatrix;
layout(location = 2) uniform mat4 m_transform;

void main() {	
	position  = gl_Position = lightSpaceMatrix * m_transform * vec4(a_position, 1.0);
};