
#version 460 core

layout (binding = 6) uniform sampler2DArray tex_ao;
layout (binding = 7) uniform sampler2DArray tex_bc;
layout (binding = 8) uniform sampler2DArray tex_h;
layout (binding = 9) uniform sampler2DArray tex_n;
layout (binding = 10) uniform sampler2DArray tex_r;


out vec4 fragColor;

in vec3 uv;
in vec3 norm;

void main() {

	fragColor = vec4(texture(tex_bc, uv).rgb, 1.f);
	//fragColor = vec4(uv.xyz, 1);

};