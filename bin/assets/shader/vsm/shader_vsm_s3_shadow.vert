
#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

layout(location = 2) uniform mat4 m_transform; //transform of the model
layout(location = 3) uniform mat4 c_comb; //transform of the view
layout(location = 4) uniform mat4 c_shadow; //transform of the light

out vec3 position;
out vec3 normal;
out vec2 uv;
out vec4 shadowPos;

void main(){
	gl_Position = c_comb * m_transform * vec4(a_position.xyz, 1.f);
	position = vec3(m_transform * vec4(a_position.xyz, 1.f));
	shadowPos = c_shadow * m_transform * vec4(a_position.xyz, 1.f);
	uv = a_uv;	
};