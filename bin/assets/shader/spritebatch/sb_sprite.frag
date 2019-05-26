#version 460 core

out vec4 FragColor;

in vec3 uv;
in vec4 col;
in vec4 sc;

layout (binding = 0) uniform sampler2D tex[32];

void main(){
	if((gl_FragCoord.x < sc.x || gl_FragCoord.x > sc.z ||
		gl_FragCoord.y < sc.y || gl_FragCoord.y > sc.w)) discard;
	FragColor = texture(tex[uint(uv.z)], uv.xy) + col;	
};