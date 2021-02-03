
#version 460 core

#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2

struct Light{
	vec4 type;
	vec4 position;
	vec4 direction;
	vec4 color;
	vec4 funcvalues;
	vec4 dis; //intensity, maxDistance, 0, 0
	vec4 sl; //sl_innerAngle, sl_outerAngle, sl_maxRadius
};

struct Material{
	vec4 COLOR_DIFFUSE;
	vec4 COLOR_SPECULAR;
	vec4 COLOR_AMBIENT;
	vec4 COLOR_EMISSIVE;
	vec4 COLOR_TRANSPARENT;
	vec4 vals; //OPACITY, SHININESS, SHININESS_STRENGTH
};

out vec4 fragColor;

in vec3 position;
in vec3 normal;
in vec2 uv;

layout(location = 6) uniform vec3 c_pos;
layout(location = 7) uniform uint matIndex;
layout(location = 8) uniform uint hasTexture;

layout (binding = 0) uniform sampler2D tex;

restrict readonly layout(binding = 1, std430) buffer dynamicLightBuffer {
	vec4 size;
	Light dLights[];
};

restrict readonly layout(binding = 2, std430) buffer matBuffer {
	Material materials[];
};

float DoAttenuation(in vec3 _vals, in float _d){
	return 1.0 / (_vals.x + _vals.y * _d + _vals.z * _d * _d);
};

void main(){
	
	Material mat = materials[matIndex];

	vec4 ambiante = vec4(0.f);
	vec4 diffuse = vec4(0.f);
	vec4 specular = vec4(0.f);

	vec3 surfacePos = position;
	vec4 surfaceColor = hasTexture == 1 ? texture(tex, uv) : vec4(0.f, 0.f, 0.f, 1.f);
	vec3 surfaceToCamera = normalize(c_pos - surfacePos);

	vec3 result = vec3(0);

	//LIGHT
	for(uint i = 0; i < int(size); ++i){
		Light light = dLights[i];
		 switch (int(light.type.x)) {
			case DIRECTIONAL_LIGHT:
			{
				vec3 surfaceToLight  = -light.direction.xyz;

				//ambient
				vec3 amb = vec3(mat.COLOR_AMBIENT) * light.dis.x * surfaceColor.rgb * light.color.rgb;

				 //diffuse
				float diffuseCoefficient = max(0.0f, dot(normal, surfaceToLight));
				vec3 diff = vec3(mat.COLOR_DIFFUSE) * diffuseCoefficient * surfaceColor.rgb * light.color.rgb;
    
				//specular
				float specularCoefficient = 0.0f;
				if(diffuseCoefficient > 0.0f)
					specularCoefficient = pow(max(0.0f, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), 0.3f);
				vec3 spec = specularCoefficient * vec3(mat.COLOR_SPECULAR) * light.color.rgb;

				//linear color (color before gamma correction)
				result = amb + (diff  + spec);
			}
			break;
        }
	}

	const vec3 gamma = vec3(1.0f/2.2f);
	fragColor = vec4(pow(result, gamma), surfaceColor.a);
}