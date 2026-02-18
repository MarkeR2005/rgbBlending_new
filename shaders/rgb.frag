#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2DArray textureArray;
uniform int R;
uniform int G;
uniform int B;
uniform bool isR;
uniform bool isG;
uniform bool isB;
uniform vec2 pos;
uniform bool isDragging;
uniform bool inverse;
uniform float contrast;
uniform float transparency;
void main()
{
	float r = 2.0*(isR ? texture(textureArray, vec3(TexCoord, R)).r : 0.0)-1.0;
	float g = 2.0*(isG ? texture(textureArray, vec3(TexCoord, G)).r : 0.0)-1.0;
	float b = 2.0*(isB ? texture(textureArray, vec3(TexCoord, B)).r : 0.0)-1.0;
	if (inverse)
	{
		r=isR ? 1.0-r : 0;
		g=isG ? 1.0-g : 0;
		b=isB ? 1.0-b : 0;
	}
	if (contrast > 0.0) 
	{
		r = r/contrast;
		g = g/contrast;
		b = b/contrast;
	}
	r = (clamp(r, -1.0, 1.0)+1)/2.0;
	g = (clamp(g, -1.0, 1.0)+1)/2.0;
	b = (clamp(b, -1.0, 1.0)+1)/2.0;
	FragColor = vec4(r,g,b,1.0-transparency);
}