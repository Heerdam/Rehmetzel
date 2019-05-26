
#version 460 core

#define POINT_LIGHT 0
#define SPOT_LIGHT 1

struct Light{
	uint type;
	vec3 position;
	vec3 direction;
	vec4 color;
	uint intensity;
	vec3 funcvalues;
	float maxDistance;
	//spot light
	float sl_innerAngle;
	float sl_outerAngle;
	float sl_maxRadius;
};

restrict readonly layout(binding = 0, std430) buffer dynamicLightBuffer {
	Light dLights[];
};

restrict readonly layout(binding = 1, std430) buffer staticLightBuffer {
	Light sLights[];
};

restrict readonly layout(binding = 2, std430) buffer o_LightIndexListBuffer {
	uint o_LightIndexList[];
};

restrict readonly layout(binding = 3, std430) buffer t_LightIndexListBuffer {
	uint t_LightIndexList[];
};

restrict readonly layout (binding = 4, rg32ui) uniform uimage2D  o_LightGrid;
restrict readonly layout (binding = 5, rg32ui) uniform uimage2D t_LightGrid;

layout (location = 6) in uniform sampler2D o_light;
layout (location = 7) in uniform sampler2D o_diff;
layout (location = 8) in uniform sampler2D o_spec;
layout (location = 9) in uniform sampler2D o_norm;
layout (location = 10) in uniform sampler2D o_depth;

vec4 DoSpecular(vec4 _color, float _specularPower, vec3 _V, vec3 _L, vec3 _N ) {
    vec3 R = normalize(reflect(-_L, _N));
    float RdotV = max( dot(R, _V), 0.0);
    return _color * pow(RdotV, _specularPower);
}

vec4 DoDiffuse(vec4 _color, vec3 _L, vec3 _N ) {
    float NdotL = max(dot(_N, _L), 0.0);
    return _color * NdotL;
}

float DoAttenuation(in vec3 _vals, in float _d){
	return 1.0 / (_vals.x + _vals.y * _d + _vals.z * _d * _d);
};

in vec2 uv;

out vec4 fragColor;

void main() {

	// View vector
	vec4 V = normalize(vec4(0.0, 0.0, 0.0, 1.0) - gl_FragCoord);
 
	vec4 light = texture(o_light, uv);
	vec4 diffuse = texture(o_diff, uv);
	vec4 specular = texture(o_spec, uv);
	vec3 normal = texture(o_norm, uv).xyz;
	vec4 depth = texture(o_depth, uv);
 
	// Unpack the specular power from the alpha component of the specular color.
	float specularPower = exp2(specular.a * 10.5);

	// Get the index of the current pixel in the light grid.
    ivec2 tileIndex = ivec2(floor(gl_FragCoord.xy / 30));

	// Get the start position and offset of the light in the light index list.
    uint startOffset = imageLoad(o_LightGrid, tileIndex).x; 
    uint lightCount = imageLoad(o_LightGrid, tileIndex).y;

    for (uint i = 0; i < lightCount; ++i) {
        uint lightIndex = o_LightIndexList[startOffset + i];
        Light light = sLights[lightIndex];
		vec3 L = normalize(-light.direction);
        switch (light.type) {
			case POINT_LIGHT:
			{
				//result = DoPointLight( light, mat, V, P, N );

				vec3 L = light.position - gl_FragCoord.xyz;
				float dis = length(L);
				L = L / dis;
 
				float attenuation = DoAttenuation(light.funcvalues, dis);
 
				diffuse += DoDiffuse(light.color, L, normal) * attenuation * light.intensity;
				specular += DoSpecular(light.color, specularPower, V.xyz, L, normal) * attenuation * light.intensity;
			}
			break;
			case SPOT_LIGHT:
			{
				//result = DoSpotLight( light, mat, V, P, N );

				vec3 L = light.position - gl_FragCoord.xyz;
				float dis = length(L);
				L = L / dis;
 
				float attenuation = DoAttenuation(light.funcvalues, dis);
				
				// If the cosine angle of the light's direction 
				// vector and the vector from the light source to the point being 
				// shaded is less than minCos, then the spotlight contribution will be 0.
				float minCos = cos(radians(light.sl_outerAngle));
				// If the cosine angle of the light's direction vector
				// and the vector from the light source to the point being shaded
				// is greater than maxCos, then the spotlight contribution will be 1.
				float maxCos = mix(minCos, 1.0, 0.5);
				float cosAngle = dot(light.direction, -L);
				// Blend between the minimum and maximum cosine angles.
				float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
 
				diffuse += DoDiffuse(light.color, L, normal) * attenuation * spotIntensity * light.intensity;
				specular += DoSpecular(light.color, specularPower, V.xyz, L, normal) * attenuation * spotIntensity * light.intensity;
 	
			}
			break;
        }
    }
     
    diffuse *= vec4(lit.Diffuse.rgb, 1.0); // Discard the alpha value from the lighting calculations.
    specular *= lit.Specular;

	fragColor = vec4((light + diffuse + specular).rgb, 1.0);
};