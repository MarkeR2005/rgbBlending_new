#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 TexCoord;
uniform vec2 offset;
uniform vec2 windowSize;
uniform vec2 imageSize;
uniform float zoom;
uniform vec2 pixelRatio;
void main()
{
	vec2 imgPos = position * imageSize - offset;
	imgPos = imgPos * zoom;
	imgPos.x/=pixelRatio.x;
	imgPos.y/=pixelRatio.y;
	vec2 normPos = (imgPos / windowSize) * 2.0 - 1.0;
        gl_Position = vec4(normPos.x, -normPos.y, 0.0, 1.0);
        TexCoord = texCoord;
}
