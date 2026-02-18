#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2DArray textureArray;
uniform sampler2D alphaTexture;
uniform int count;
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
vec3 hsvToRgb__(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
vec3 hsvToRgb(vec3 c)
{
    float h = c.x;
    float s = c.y;
    float v = c.z;
    vec3 rgb;
    if (h < 0.5){
        rgb = vec3(1.0-2*h,2*h,0);
    }
    else {
        rgb = vec3(0.0,2.0-2*h,2*h-1.0);
    }
    return mix(vec3(v), rgb, s);
}
void main() 
{
	float en = 0.0;
	int id_max = 0;
	float en_max = 0.0;
	for (int i = 0; i < count; i++)
	{
		float value = texture(textureArray, vec3(TexCoord, i)).r;
		if (value>en_max)
		{
			en_max = value;
			id_max = i;
		}
	}
        int width = 0;
        int l = id_max - 1;
        while (l>0)
        {
                float value = texture(textureArray, vec3(TexCoord, l)).r;
                if (value > contrast){
                width = width+1;
                }
                l = l-1;
        }
        int r = id_max + 1;
                while (r<count)
        {
                float value = texture(textureArray, vec3(TexCoord, r)).r;
                if (value > en_max*contrast){
                width = width+1;
                }
                r++;
        }
	float sat = 1.0 - float(width) / count;
	vec3 accumulated = vec3(float(id_max)/count, sat, 1.0);
	vec3 rgb = hsvToRgb(accumulated);
	FragColor = vec4(clamp(rgb, 0.0, 1.0), 1.0);
}