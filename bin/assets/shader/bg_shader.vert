
#version 460 core
layout (location = 0) in vec3 aPos;

out int index;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
	index = int(aPos.z);
}