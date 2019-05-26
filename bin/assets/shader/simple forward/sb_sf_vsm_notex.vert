
#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_col;

layout(location = 3) uniform mat4 m_transform;
layout(location = 4) uniform mat3 m_transInvTrans;

layout(location = 5) uniform mat4 c_comb;
layout(location = 6) uniform mat4 c_shadow;

out vec3 position;
out vec3 normal;
out vec4 color;
out vec4 shadowP;

void main(){
	gl_Position = c_comb * vec4(a_position.xyz, 1.f);
	position = vec3(m_transform * vec4(a_position.xyz, 1.f));
	normal = normalize(m_transInvTrans * a_normal);
	color = a_col;
	shadowP = c_shadow * m_transform * vec4(a_position.xyz, 1.f);
};