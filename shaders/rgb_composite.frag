#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D composite;
uniform bool isR, isG, isB, inverse;
uniform float contrast;
void main() {
    vec3 sampleColor = texture(composite, TexCoord).rgb;
    vec3 active = vec3(isR ? 1.0 : 0.0, isG ? 1.0 : 0.0, isB ? 1.0 : 0.0);
    vec3 value = 2.0 * sampleColor * active - 1.0;
    if (inverse) value = (vec3(1.0) - value) * active;
    if (contrast > 0.0) value /= contrast;
    FragColor = vec4((clamp(value, -1.0, 1.0) + 1.0) * 0.5, 1.0);
}
