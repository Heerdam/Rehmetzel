
#version 460 core

layout(location = 0) in unsigned int index;

struct Debug{
	float radius;
	vec2 position;
};

layout(binding = 0, std430) buffer debugBuffer{
	float gridWidth;
	float gridHeight;
	uint cellsX;
	uint cellsY;
	uint size;
	Debug data[1000];
};

flat out float out_radius;

void main() {
	//if(index >= size){
	//	out_radius = -1.0;
	//	return;
	//};
	Debug d = data[index];
	gl_Position = vec4(d.position.x, d.position.y, 0.0, 1.0); 
	out_radius = d.radius;

	//out_radius = 50.0;
	//float dist = (out_radius + 5.0) * float(index);
	//gl_Position = vec4(dist, 0.0, 0.0, 1.0); 
};