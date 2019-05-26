
#version 460 core

//Diffuse
//Specular
//Normals
layout (location = 0) out vec4 o_light;
layout (location = 1) out vec4 o_diff;
layout (location = 2) out vec4 o_spec;
layout (location = 3) out vec3 o_norm;

struct Material{
	vec4 emission;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
	float transparency;
};

struct Light{
	vec3 dir;
	vec4 color;
	uint intensity;
};

restrict readonly layout(binding = 0, std430) buffer matBuffer {
	Material mat[];
};

restrict readonly layout(binding = 1, std430) buffer dirLights {
	Light lights[];
};

in vec3 normal;
in vec2 uv;
in uint matIndex;

uniform sampler2D tex;

vec4 DoSpecular(vec4 _color, float _specularPower, vec3 _V, vec3 _L, vec3 _N ) {
    vec3 R = normalize(reflect(-_L, _N));
    float RdotV = max( dot(R, _V), 0.0);
    return _color * pow(RdotV, _specularPower);
}

vec4 DoDiffuse(vec4 _color, vec3 _L, vec3 _N ) {
    float NdotL = max(dot(_N, _L), 0.0);
    return _color * NdotL;
}

void main(){
	o_norm = normal;
	o_spec = vec4(mat[matIndex].specular.xyz, clamp(log2(mat[matIndex].shininess)/10.5, 0.0, 1.0));
	o_diff = mat[matIndex].diffuse;
	o_light = mat[matIndex].ambient + mat[matIndex].diffuse;

	//eyePos: The position of the camera in view space (which is always (0, 0, 0))
	//P: The position of the point being shaded in view space
	//N: The normal of the point being shaded in view space.

	vec4 V = normalize(vec4(0.0, 0.0, 0.0, 1.0) - gl_FragCoord);

	for(int i = 0; i < lights.length(); ++i){
		vec3 L = normalize(-lights[i].dir);
		vec4 diffuse = DoDiffuse(lights[i].color, L, normal) * lights[i].intensity;
		vec4 specular = DoSpecular(lights[i].color, mat[matIndex].shininess, V.xyz, L, normal) * lights[i].intensity;
		o_diff += diffuse;
		o_spec += specular;
	}	
};