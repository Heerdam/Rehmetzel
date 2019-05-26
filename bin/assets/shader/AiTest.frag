
#version 460 core

flat in int toDraw;

out vec4 FragColor;

void main() {
	if(toDraw == 0)
		FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	else
		FragColor = vec4(1.0, 1.0, 1.0, 1.0);
};