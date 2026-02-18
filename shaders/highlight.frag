#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform vec2 imageSize;
uniform vec2 pixelRatio;
uniform vec2 highlightPos;
uniform float zoom;
void main()
{
	vec2 thisPos = vec2(TexCoord.x * imageSize.x, TexCoord.y * imageSize.y);
    if (highlightPos.x < 0) {
        float dist = abs(thisPos.y-float(highlightPos.y))/pixelRatio.y;
        if (dist < 1.0/zoom)
        {
            FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
    }
    else if (highlightPos.y < 0) {
        float dist = abs(thisPos.x-float(highlightPos.x))/pixelRatio.x;
        if (dist < 1.0/zoom)
        {
            FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
    }
    else {
        if ((thisPos.x-highlightPos.x)*(thisPos.x-highlightPos.x)+(thisPos.y-highlightPos.y)*(thisPos.y-highlightPos.y) < 3.0/(zoom*zoom)) {
            FragColor = vec4(0.0, 1.0, 1.0, 1.0);
        }
    }
}