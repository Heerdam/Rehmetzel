
#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 18) out;

in int index[];

out vec2 texCoord;
flat out int indexFr;

//float rand(vec2 co){
 //   return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
//}

/**
       v3

   V4      V2
   |   V0    |
   V5      V1

       v6

	t1: v0 v1 v2
	t2: v0 v2 v4
	t3: v0 v4 v5
	t4: v0 v5 v1

	t5: v4 v2 v3
	t6: v1 v5 v6
*/

const float diam = 100;
const float PI = 3.1415926535897932384626433832795;
const float rad = 30*PI/180;
const float px = cos(rad) * diam;
const float py = sin(rad) * diam;

uniform mat4 transform;

void main() {  

	indexFr = index[0];
	
	//float u = clamp(rand(vec2(gl_in[0].gl_Position.x, 0)), 0, 1) - 0.5;
	//float v = clamp(rand(vec2(0, gl_in[0].gl_Position.y)), 0, 1) - 0.5;

	//centre
	float uc = 0.5;
	float vc = 0.5;

	float lu = 1.0;
	float ru = 0.0;

	float uv = 0.75;
	float lv = 0.25;

	float tv = 1;
	float fv = 0;

	//-------------- T1 --------------\\

	//V0
	gl_Position = transform * gl_in[0].gl_Position; 
	texCoord = vec2(uc, vc);
    EmitVertex();

	//V1
    gl_Position = transform * (gl_in[0].gl_Position + vec4(px, -py, 0.0, 0.0));
	texCoord = vec2(lu, lv);
    EmitVertex();

	//V2
    gl_Position = transform * (gl_in[0].gl_Position + vec4(px, py, 0.0, 0.0));
	texCoord = vec2(lu, uv);
    EmitVertex();
	EndPrimitive();

	//-------------- T2 --------------\\

	//V0
	gl_Position = transform * gl_in[0].gl_Position; 
	texCoord = vec2(uc, vc);
    EmitVertex();

	//V2
    gl_Position = transform * (gl_in[0].gl_Position + vec4(px, py, 0.0, 0.0));
	texCoord = vec2(lu, uv);
    EmitVertex();
	
	//V4
    gl_Position = transform * (gl_in[0].gl_Position + vec4(-px, py, 0.0, 0.0)); 
	texCoord = vec2(ru, uv);
    EmitVertex();
	EndPrimitive();

	
	//-------------- T3 --------------\\

	//V0
	gl_Position = transform * gl_in[0].gl_Position; 
	texCoord = vec2(uc, vc);
    EmitVertex();

	//V4
    gl_Position = transform * (gl_in[0].gl_Position + vec4(-px, py, 0.0, 0.0)); 
	texCoord = vec2(ru, uv);
    EmitVertex();

	//V5
    gl_Position = transform * (gl_in[0].gl_Position + vec4(-px, -py, 0.0, 0.0)); 
	texCoord = vec2(ru, lv);
    EmitVertex();
	EndPrimitive();
	
	
	//-------------- T4 --------------\\

	//V0
	gl_Position = transform * gl_in[0].gl_Position; 
	texCoord = vec2(uc, vc);
    EmitVertex();

	//V5
    gl_Position = transform * (gl_in[0].gl_Position + vec4(-px, -py, 0.0, 0.0)); 
	texCoord = vec2(ru, lv);
    EmitVertex();

	//V1
    gl_Position = transform * (gl_in[0].gl_Position + vec4(px, -py, 0.0, 0.0));
	texCoord = vec2(lu, lv);
    EmitVertex();
	EndPrimitive();

	
	//-------------- T5 --------------\\

	//V4
    gl_Position = transform * (gl_in[0].gl_Position + vec4(-px, py, 0.0, 0.0)); 
	texCoord = vec2(ru, uv);
    EmitVertex();

	//V2
    gl_Position = transform * (gl_in[0].gl_Position + vec4(px, py, 0.0, 0.0));
	texCoord = vec2(lu, uv);
	EmitVertex();

	//V3
    gl_Position = transform * (gl_in[0].gl_Position + vec4(0, diam, 0.0, 0.0));
	texCoord = vec2(uc, tv);
	EmitVertex();
	EndPrimitive();

	//-------------- T6 --------------\\

	//V1
    gl_Position = transform * (gl_in[0].gl_Position + vec4(px, -py, 0.0, 0.0));
	texCoord = vec2(lu, lv);
    EmitVertex();

	//V5
    gl_Position = transform * (gl_in[0].gl_Position + vec4(-px, -py, 0.0, 0.0)); 
	texCoord = vec2(ru, lv);
    EmitVertex();

	//V6
    gl_Position = transform * (gl_in[0].gl_Position + vec4(0, -diam, 0.0, 0.0));
	texCoord = vec2(uc, fv);
	EmitVertex();
	EndPrimitive();

}  