#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform vec2 imageSize;
uniform sampler2D indexTexture;
uniform sampler1D paletteTexture;
uniform vec2 pixelRatio;
uniform vec2 pos;
uniform bool isDragging;
uniform float contrast;
void main()
{
	float index = 2.0*texture(indexTexture, TexCoord).r - 1.0;
	if (contrast > 0.0) {
		index = index/contrast;
	}
	index = (clamp(index, -1.0, 1.0)+1.0)/2.0;
	FragColor = texture(paletteTexture, index);
	if ((abs(TexCoord.x-pos.x) < 0.001 || abs(TexCoord.y-pos.y) < 0.001) && isDragging)
	{
		FragColor = mix(FragColor, vec4(1.0, 1.0, 0.0, 1.0), 0.7);
	}
	if (selectedTrace >=0)
	{
		float tracePos = TexCoord.y * imageSize.y;
		float dist = abs(tracePos-float(selectedTrace))/pixelRatio.y;
		if (dist<2.0)
		{
			FragColor = mix(FragColor, vec4(0.0, 1.0, 0.0, 1.0), 0.7);
		}
		else if (dist < 3.0)
		{
			FragColor = mix(FragColor, vec4(0.0, 1.0, 0.0, 1.0), 0.3);
		}
	}
}