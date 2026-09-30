//---------------------------------------------------------------------------
#pragma hdrstop
#include "Shaders.h"
#include <iostream>
#include <string>
#include <exception>
#include <fstream>
#include <sstream>
#include <vcl.h>
#include <vector>
//---------------------------------------------------------------------------
#pragma package(smart_init)
std::string readSource (std::string fileName)
{
System::UnicodeString fullPath = ExtractFileDir(Application->ExeName) + ("\\shaders\\" + fileName).c_str();

	std::ifstream file(fullPath.c_str(), std::ios::binary);
	// Читаем побайтово и проверяем каждый символ
    std::string content;
    char ch;
    while (file.get(ch)) {
		// Пропускаем только нулевые байты, все остальные включаем
		if (ch != '\0') {
            content += ch;
		}
	}
	return content;
}
std::string readSourceFromFile(std::wstring fileName)
{
	std::ifstream file(fileName, std::ios::binary);
	// Читаем побайтово и проверяем каждый символ
    std::string content;
    char ch;
    while (file.get(ch)) {
		// Пропускаем только нулевые байты, все остальные включаем
		if (ch != '\0') {
            content += ch;
		}
	}
	return content;
}

GLuint compileShaderFromFile(GLenum type, std::wstring fileName)
{
	std::string contents = readSourceFromFile(fileName);
    if (contents.empty()) throw Exception("Shader file missing or empty");
    const char* source = contents.c_str();
	//ShowMessage(source);
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "CompileErr" << std::endl << infoLog << std::endl;
		ShowMessage(infoLog);
		glDeleteShader(shader);
		throw std::exception(infoLog);
	}
	return shader;
}
GLuint compileShader(GLenum type, std::string fileName)
{
	std::string contents = readSource(fileName.c_str());
    if (contents.empty()) {
        throw Exception(("Shader file missing or empty: " + fileName).c_str());
    }
    const char* source = contents.c_str();
	//ShowMessage(source);
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "CompileErr" << std::endl << infoLog << std::endl;
		ShowMessage(infoLog);
		glDeleteShader(shader);
		throw std::exception(infoLog);
	}
	return shader;
}
//--
GLuint createShaderProgram(std::string vert, std::string frag)
{
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vert+".vert");
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, frag+".frag");
	if (!vertexShader || !fragmentShader)
	{
		return 0;
	}
	GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
		ShowMessage(infoLog);
		throw std::exception(infoLog);
	}
    glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
    return program;
}
//--
GLuint createShaderProgram(std::string vert, std::string frag, std::string geom)
{
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vert+".vert");
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, frag+".frag");
	GLuint geometryShader = compileShader(GL_FRAGMENT_SHADER, geom+".geom");
	if (!vertexShader || !fragmentShader)
	{
		return 0;
	}
	GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glAttachShader(program, geometryShader);
    glLinkProgram(program);
    GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cerr << "programErr" << std::endl << infoLog << std::endl;
		throw std::exception(infoLog);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(geometryShader);
	return program;
}

GLuint createShaderProgramFromFile(std::wstring fragLoc)
{
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, "universal.vert");
	GLuint fragmentShader = compileShaderFromFile(GL_FRAGMENT_SHADER, fragLoc);
	if (!vertexShader || !fragmentShader)
	{
		return 0;
	}
	GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
		ShowMessage(infoLog);
		throw std::exception(infoLog);
	}
    glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
    return program;
}

namespace {
GLuint compileEmbedded(GLenum type, const char* source, const char* name) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        const std::string message = std::string("Shader ") + name + ": " + log;
        glDeleteShader(shader);
        throw Exception(message.c_str());
    }
    return shader;
}
GLuint linkEmbedded(const char* vertex, const char* fragment, const char* name) {
    const GLuint vs = compileEmbedded(GL_VERTEX_SHADER, vertex, name);
    GLuint fs = 0, program = 0;
    try {
        fs = compileEmbedded(GL_FRAGMENT_SHADER, fragment, name);
        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        GLint ok = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[2048] = {};
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);
            throw Exception((std::string("Program ") + name + ": " + log).c_str());
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    } catch (...) {
        glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        if (program) glDeleteProgram(program);
        throw;
    }
}
const char* rgbVertex2D = R"GLSL(#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 TexCoord;
uniform vec2 offset;
uniform vec2 windowSize;
uniform vec2 imageSize;
uniform float zoom;
uniform vec2 pixelRatio;
void main() {
    vec2 imgPos = (position * imageSize - offset) * zoom / pixelRatio;
    vec2 normPos = imgPos / windowSize * 2.0 - 1.0;
    gl_Position = vec4(normPos.x, -normPos.y, 0.0, 1.0);
    TexCoord = texCoord;
})GLSL";
const char* rgbFragment2D = R"GLSL(#version 330 core
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
})GLSL";
const char* highlightFragment2D = R"GLSL(#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform vec2 imageSize;
uniform vec2 pixelRatio;
uniform vec2 highlightPos;
uniform float zoom;
void main() {
    vec2 thisPos = TexCoord * imageSize;
    if (highlightPos.x < 0.0) {
        if (abs(thisPos.y-highlightPos.y)/pixelRatio.y >= 1.0/zoom) discard;
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (highlightPos.y < 0.0) {
        if (abs(thisPos.x-highlightPos.x)/pixelRatio.x >= 1.0/zoom) discard;
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        vec2 delta = thisPos-highlightPos;
        if (dot(delta, delta) >= 3.0/(zoom*zoom)) discard;
        FragColor = vec4(0.0, 1.0, 1.0, 1.0);
    }
})GLSL";
const char* horizonVertex2D = R"GLSL(#version 330 core
layout(location = 0) in vec2 position;
uniform vec2 offset;
uniform vec2 windowSize;
uniform float zoom;
uniform vec2 pixelRatio;
void main() {
    vec2 imgPos = (position-offset)*zoom/pixelRatio;
    vec2 normPos = imgPos/windowSize*2.0-1.0;
    gl_Position = vec4(normPos.x, -normPos.y, 0.0, 1.0);
})GLSL";
const char* horizonFragment2D = R"GLSL(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() { FragColor = vec4(uColor, 1.0); }
)GLSL";
const char* labelVertex2D = R"GLSL(#version 330 core
layout(location=0) in vec2 position;
layout(location=1) in vec2 texCoord;
uniform vec2 windowSize;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(position.x/windowSize.x*2.0-1.0,
                       1.0-position.y/windowSize.y*2.0, 0.0, 1.0);
    TexCoord = texCoord;
})GLSL";
const char* labelFragment2D = R"GLSL(#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D label;
uniform vec3 textColor;
void main() {
    if (texture(label, TexCoord).r < 0.35) discard;
    FragColor = vec4(textColor, 1.0);
})GLSL";
}
GLuint create2DRgbShaderProgram() {
    return linkEmbedded(rgbVertex2D, rgbFragment2D, "2D RGB");
}
GLuint createCrossLabelShaderProgram() {
    return linkEmbedded(labelVertex2D, labelFragment2D, "cross label");
}
GLuint create2DHighlightShaderProgram() {
    return linkEmbedded(rgbVertex2D, highlightFragment2D, "2D highlight");
}
GLuint create2DHorizonShaderProgram() {
    return linkEmbedded(horizonVertex2D, horizonFragment2D, "2D horizon");
}
