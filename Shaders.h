//---------------------------------------------------------------------------
#include <GL/glew.h>
#include <string>
#ifndef ShadersH
#define ShadersH
//---------------------------------------------------------------------------
GLuint createShaderProgram(std::string vert, std::string frag);
GLuint createShaderProgram(std::string vert, std::string frag, std::string geom);
GLuint createShaderProgramFromFile(std::wstring fragLoc);
#endif
