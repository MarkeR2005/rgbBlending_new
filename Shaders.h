//---------------------------------------------------------------------------
#include <GL/glew.h>
#include <string>
#ifndef ShadersH
#define ShadersH
//---------------------------------------------------------------------------
GLuint createShaderProgram(std::string vert, std::string frag);
GLuint createShaderProgram(std::string vert, std::string frag, std::string geom);
GLuint createShaderProgramFromFile(std::wstring fragLoc);
GLuint create2DRgbShaderProgram();
GLuint createCrossLabelShaderProgram();
GLuint create2DHighlightShaderProgram();
GLuint create2DHorizonShaderProgram();
#endif
