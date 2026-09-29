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
