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
vec3 hsvToRgb(vec3 c) 
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
void main() 
{
	float sum_ampl = 0.0;
	float x_total = 0.0;
	float y_total = 0.0;
	for (int i = 0; i < count; i++) 
	{
		float value = 2.0*(texture(textureArray, vec3(TexCoord, i)).r)-1.0;
		if (contrast > 0.0) 
		{
			value = value/contrast;
		}
		value = (clamp(value, -1.0, 1.0)+1.0)/2.0;
		sum_ampl = sum_ampl + value;
		x_total = x_total + value * cos(float(i)/(count-1.0));
		y_total = y_total + value * sin(float(i)/(count-1.0));
	}
	if (sum_ampl == 0.0) 
	{
	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	return;
	}
	float angle = atan(y_total, x_total);
	if (angle < 0.0) 
	{
		angle = angle + 2.0 * 3.141592653589793;
	}
	float h_final = angle / (2.0 * 3.141592653589793);
	float s_final = length(vec2(x_total, y_total)) / sum_ampl;
	float v_final = sqrt(sum_ampl / float(count));
	vec3 hsv_color = vec3(h_final, s_final, v_final);
	vec3 rgb_color = hsvToRgb(hsv_color);
	FragColor = vec4(rgb_color, 1.0);
}