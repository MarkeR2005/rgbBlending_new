#version 330 core
layout(location=0) in vec2 position;
layout(location=1) in vec2 texCoord;
uniform vec2 windowSize;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(position.x/windowSize.x*2.0-1.0, 1.0-position.y/windowSize.y*2.0, 0.0, 1.0);
    TexCoord = texCoord;
}
