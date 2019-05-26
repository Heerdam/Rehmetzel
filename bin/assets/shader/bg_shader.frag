
#version 460 core
layout(origin_upper_left) in vec4 gl_FragCoord;

out vec4 FragColor;
  
flat in int indexFr;
in vec2 texCoord;

uniform sampler2D tex[9];

void main() {
    FragColor = texture(tex[indexFr], texCoord);

} 