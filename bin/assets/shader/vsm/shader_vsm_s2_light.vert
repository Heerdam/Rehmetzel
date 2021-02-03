
#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(location = 3) uniform mat4 m_transform; //model transform
layout(location = 4) uniform mat3 m_transInvTrans; //model inv trans

layout(location = 5) uniform mat4 c_comb; //camera combined

out vec3 position;
out vec3 normal;
out vec2 uv;

void main(){
	gl_Position = c_comb * m_transform * vec4(a_position.xyz, 1.f);
	position = vec3(m_transform * vec4(a_position.xyz, 1.f));
	normal = normalize(m_transInvTrans * a_normal);
	uv = a_uv;
};