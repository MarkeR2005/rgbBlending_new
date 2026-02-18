#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform vec2 imageSize;
uniform sampler2D indexTexture;
uniform sampler1D paletteTexture;
uniform vec2 pixelRatio;
uniform float contrast;
uniform float transparency;
void main()
{
	float index = 2.0*texture(indexTexture, TexCoord).r - 1.0;
	if (contrast > 0.0) {
		index = index/contrast;
	}
	index = (clamp(index, -1.0, 1.0)+1.0)/2.0;
	vec4 color = texture(paletteTexture, index);
        FragColor = vec4(color.x, color.y, color.z, 1.0-transparency);
}