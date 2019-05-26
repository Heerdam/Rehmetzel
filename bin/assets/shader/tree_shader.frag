
#version 330 core

out vec4 FragColor;
  
flat in int index;
in vec2 uv;

in vec4 gl_FragCoord;

uniform sampler2D tex[3];

uniform vec2 viewportSize;
uniform float radius;

void main() {

	float w =  gl_FragCoord.x - viewportSize.x * 0.5;
	float h =  gl_FragCoord.y - viewportSize.y * 0.5;

	float dist = sqrt(w*w + h*h);
	
    FragColor = texture(tex[index], uv);
	float alpha = clamp(1 / radius * dist, 0.2, 1.0);
	FragColor.w = clamp(FragColor.w - (1 - alpha), 0.0, 1.0);
	
} 