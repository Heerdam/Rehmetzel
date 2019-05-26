
#version 460 core

out vec4 fragColor;

in vec4 position;
	
void main() {
	float depth = (position.z / position.w) * 0.5f + 0.5f;

	float moment1 = depth;
	float moment2 = pow(depth, 2);

	float dx = dFdx(depth);
	float dy = dFdy(depth);

	moment2 += 0.25f * (pow(dx, 2) + pow(dy, 2));

	fragColor = vec4(moment1, moment2, 0.0f, 0.0f);
};