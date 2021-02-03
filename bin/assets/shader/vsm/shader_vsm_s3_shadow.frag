
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

in vec2 uv;
in vec4 shadowPos;

layout (binding = 0) uniform sampler2D colorMap;
layout (binding = 1) uniform sampler2D shadowdepthMap;
layout (binding = 2) uniform sampler2D shadowMap;

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
	vec4 shadowP = vec4(shadowPos.xy, texture(shadowdepthMap, uv).z, shadowPos.w);
	//SHADOW
	vec4 scPostW = shadowP / shadowP.w; 
	scPostW = scPostW * 0.5f + 0.5f;
	bool outsideShadowMap = scPostW.w <= 0.0f || (scPostW.x < 0.f || scPostW.y < 0.f) || (scPostW.x >= 1.f || scPostW.y >= 1.f);
	float shadowFactor = !outsideShadowMap ? chebyshevUpperBound(scPostW.z, scPostW.xy) : 1.f;

	vec4 surfaceColor = texture(colorMap, uv);
	fragColor = vec4(surfaceColor.xyz * shadowFactor, surfaceColor.a);
}