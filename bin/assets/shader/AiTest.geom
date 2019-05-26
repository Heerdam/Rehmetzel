
#version 460 core

#define SEGMENTS 36

layout (points) in;
layout (line_strip, max_vertices = SEGMENTS + 1) out;

flat in float out_radius[];
flat out int toDraw;

const float PI = 3.1415926535897932384626433832795;

uniform mat4 transform;

void main() {
	if(out_radius[0] < 0){
		toDraw = 0;
		return;
	}else toDraw = 1;

	//draw circle
	float angle = 2.0 * PI / SEGMENTS;
	float cos_ = cos(angle);
	float sin_ = sin(angle);
	float cx = out_radius[0], cy = 0.0;
	for (int i = 0; i < SEGMENTS + 1; ++i){		
		gl_Position = transform * (gl_in[0].gl_Position + vec4(cx, cy, 0.0, 0.0)); 
		EmitVertex();
		float temp = cx;
		cx = cos_ * cx - sin_ * cy;
		cy = sin_ * temp + cos_ * cy;
	}
	EndPrimitive();

};