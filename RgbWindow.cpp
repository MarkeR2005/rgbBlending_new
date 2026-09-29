//---------------------------------------------------------------------------
#pragma hdrstop
#include "RgbWindow.h"
#include "Shaders.h"

#include <vcl.h>
#include <GL/glew.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
RgbWindow::RgbWindow():FlatWindow()
{
}
//--
RgbWindow::~RgbWindow()
{
if (!handle) {
	return;
}
glfwMakeContextCurrent(handle);
	if (indexTexture != 0) {
		glDeleteTextures(1, &indexTexture);
		indexTexture = 0;
	}
}
//--
void RgbWindow::initWindow(TPanel* parent)
{
	//Инициализируем базовое окно
	FlatWindow::initWindow(parent);
	//Создаём и устанавливаем шейдерную программу
	GLuint program = createShaderProgram("universal", "rgb");
	FlatWindow::setShaderProgram(program);
}
//--
void RgbWindow::initTexture(const std::vector<bitMap>& textures)
{
	//Проверка инициализации
	if (!handle) {
    OutputDebugStringA("CTX");
    throw Exception("Uninitialized window");
    }
	glfwMakeContextCurrent(handle);
    if (glfwGetCurrentContext() != handle) {
    const char* description = nullptr;
    int errorCode = glfwGetError(&description);
    OutputDebugStringA(description);
    throw Exception(description);
}
GLenum glew_err = glewInit();
    if (glew_err != GLEW_OK) {
     OutputDebugStringA("GLEW");
        throw Exception(reinterpret_cast<const char*>(glewGetErrorString(glew_err)));
    }
    if (!GLEW_VERSION_3_0) {
    OutputDebugStringA("GLEW_VER");
        throw Exception("OpenGL 3.0 not supported");
    }
const GLubyte* version = glGetString(GL_VERSION);
if (!version) throw Exception("OpenGL context not available");
	//Выставляем параметры
	width = textures[0].width;
	height = textures[0].height;
	size = textures.size();
	//Создаём текстуру
	glGenTextures(1, &indexTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, indexTexture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, width, height, size, 0,
				 GL_RED, GL_UNSIGNED_BYTE, nullptr);
	//Заполняем текстуру
    //ShowMessage("Filling");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Критически важно для 1-байтовых данных!
	for (int i = 0; i < size; ++i)
	{
		if (!textures[i].texture.empty())
		{
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                            0, 0, i,
							width, height, 1,
							GL_RED, GL_UNSIGNED_BYTE,
							textures[i].texture.data());
        }
	}
	//Настраиваем текстуру
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// Проверка ошибок
    GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
        OutputDebugStringA("GL_Error");
		throw Exception(err);
	}
}
//--
void RgbWindow::renderWindow()
{
	//Проверка инициализции
	if (!handle) return;
	//Очистка
	FlatWindow::preRender();
	//Родительский рендер
	FlatWindow::render();
	//Собственный рендер
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, indexTexture);
	glUniform1i(glGetUniformLocation(shaderProgram, "textureArray"), 0);
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, alphaTexture);
	//glUniform1i(glGetUniformLocation(shaderProgram, "alphaTexture"), 1);
	glUniform1i(glGetUniformLocation(shaderProgram, "R"), R);
	glUniform1i(glGetUniformLocation(shaderProgram, "isR"), isR);
	glUniform1i(glGetUniformLocation(shaderProgram, "G"), G);
	glUniform1i(glGetUniformLocation(shaderProgram, "isG"), isG);
	glUniform1i(glGetUniformLocation(shaderProgram, "B"), B);
	glUniform1i(glGetUniformLocation(shaderProgram, "isB"), isB);
	glUniform1i(glGetUniformLocation(shaderProgram, "inverse"), inverse);
	glUniform1i(glGetUniformLocation(shaderProgram, "count"), size);
	//Очистка буфферов
	FlatWindow::postRender();
    glfwSwapBuffers(handle);
}
//--
void RgbWindow::setColor(int _R, int _G, int _B)
{
	R=_R;
	B=_B;
	G=_G;
}
//--
void RgbWindow::setView(bool _isR, bool _isG, bool _isB)
{
	isR=_isR;
	isG=_isG;
	isB=_isB;
}

void RgbWindow::renderForScreenshot()
{
	//Проверка инициализции
	if (!handle) return;
	//Очистка
	FlatWindow::preRender();
	//Родительский рендер
	FlatWindow::render();
	//Собственный рендер
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, indexTexture);
	glUniform1i(glGetUniformLocation(shaderProgram, "textureArray"), 0);
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, alphaTexture);
	//glUniform1i(glGetUniformLocation(shaderProgram, "alphaTexture"), 1);
	glUniform1i(glGetUniformLocation(shaderProgram, "R"), R);
	glUniform1i(glGetUniformLocation(shaderProgram, "isR"), isR);
	glUniform1i(glGetUniformLocation(shaderProgram, "G"), G);
	glUniform1i(glGetUniformLocation(shaderProgram, "isG"), isG);
	glUniform1i(glGetUniformLocation(shaderProgram, "B"), B);
	glUniform1i(glGetUniformLocation(shaderProgram, "isB"), isB);
	glUniform1i(glGetUniformLocation(shaderProgram, "inverse"), inverse);
	glUniform1i(glGetUniformLocation(shaderProgram, "count"), size);
	//Очистка буфферов
	FlatWindow::postRender();
    glFlush();
}
