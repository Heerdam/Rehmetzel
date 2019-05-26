
#version 460 core

out vec4 frag;

in vec2 uv;

layout(location = 3) uniform uint state;
layout(location = 4) uniform uvec2 size;
layout(location = 5) uniform uint index;

layout(binding = 0) uniform sampler2D tex;
layout(binding = 1) uniform isampler2D texI;
layout(binding = 2) uniform usampler2D texU;

layout(binding = 3) uniform sampler2DRect texR;
layout(binding = 4) uniform isampler2DRect texRI;
layout(binding = 5) uniform usampler2DRect texRU;

layout(binding = 6) uniform sampler2DArray tex2d;

void main(){
	switch(state){
		case 0:
			frag = vec4(texture(tex, uv).xyz, 1.f);
		break;
		case 1:
			frag = vec4(normalize(texture(texI, uv)).xyz, 1.f);
		break;
		case 2:
			frag = vec4(normalize(texture(texU, uv)).xyz, 1.f);
		break;
		case 3:
			frag = vec4(texture(texR, uv * size).xyz, 1.f);
		break;
		case 4:
			frag = vec4(normalize(texture(texRI, uv * size)).xyz, 1.f);
		break;
		case 5:
			frag = vec4(normalize(texture(texRU, uv * size)).xyz, 1.f);
		break;
		case 6:
			frag = vec4(texture(tex2d, vec3(uv, index)).xyz, 1.f);
		break;
	};	 
}