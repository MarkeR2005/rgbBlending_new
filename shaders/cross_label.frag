#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D label;
void main() {
    if (texture(label, TexCoord).r < 0.35) discard;
    FragColor = vec4(1.0);
}
