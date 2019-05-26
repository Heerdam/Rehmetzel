
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

out vec4 fragColor;

in vec3 position;
in vec3 normal;
in vec4 color;
in vec4 shadowP;

layout (binding = 1) uniform sampler2D shadowMap;

layout(location = 7) uniform vec3 c_pos;

restrict readonly layout(binding = 2, std430) buffer dynamicLightBuffer {
	vec4 size;
	Light dLights[];
};

float DoAttenuation(in vec3 _vals, in float _d){
	return 1.0 / (_vals.x + _vals.y * _d + _vals.z * _d * _d);
};

float chebyshevUpperBound(in float _dis, in vec2 _uv) {
	vec2 moments = texture2D(shadowMap, _uv).xy;	
	// Surface is fully lit. as the current fragment is before the light occluder
	if (_dis <= moments.x) return 1.0f;
	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// how likely this pixel is to be lit (p_max)
	float variance = max(moments.y - pow(moments.x, 2), 0.00001);
	return variance / (variance + pow(_dis - moments.x, 2));
}

void main(){
	
	vec4 ambiante = vec4(0.f);
	vec4 diffuse = vec4(0.f);
	vec4 specular = vec4(0.f);

	vec3 surfacePos = position;
	vec4 surfaceColor = color;
	vec3 surfaceToCamera = normalize(c_pos - surfacePos);

	vec3 result = vec3(0);

	//SHADOW
	vec4 scPostW = shadowP / shadowP.w; 
	scPostW = scPostW * 0.5f + 0.5f;
	bool outsideShadowMap = scPostW.w <= 0.0f || (scPostW.x < 0.f || scPostW.y < 0.f) || (scPostW.x >= 1.f || scPostW.y >= 1.f);
	float shadowFactor = !outsideShadowMap ? chebyshevUpperBound(scPostW.z, scPostW.xy) : 1.f;

	//LIGHT
	for(uint i = 0; i < int(size); ++i){
		Light light = dLights[i];
		 switch (int(light.type.x)) {
			case DIRECTIONAL_LIGHT:
			{
				vec3 surfaceToLight  = -light.direction.xyz;

				//ambient
				vec3 amb =  0.1f * light.dis.x * surfaceColor.rgb * light.color.rgb;

				 //diffuse
				float diffuseCoefficient = max(0.0f, dot(normal, surfaceToLight));
				vec3 diff = 0.1f * diffuseCoefficient * surfaceColor.rgb * light.color.rgb;
    
				//specular
				float specularCoefficient = 0.0f;
				if(diffuseCoefficient > 0.0f)
					specularCoefficient = pow(max(0.0f, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), 0.3f);
				vec3 spec = specularCoefficient * 0.0f * light.color.rgb;

				//linear color (color before gamma correction)
				result = amb + (diff  + spec) * shadowFactor;
			}
			break;
        }
	}

	const vec3 gamma = vec3(1.0f/2.2f);
	fragColor = vec4(pow(result, gamma), surfaceColor.a);
}