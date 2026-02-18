//---------------------------------------------------------------------------
#pragma hdrstop
#include "SeismicWindow.h"
#include "Shaders.h"

#include <vcl.h>
#include <algorithm>
//---------------------------------------------------------------------------
#pragma package(smart_init)
SeismicWindow::SeismicWindow():FlatWindow()
{
}
//--
SeismicWindow::~SeismicWindow()
{
if (!handle) {
	return;
}
glfwMakeContextCurrent(handle);
	if (paletteTexture != 0) {
		glDeleteTextures(1, &paletteTexture);
		paletteTexture = 0;
	}
	if (indexTexture != 0) {
		glDeleteTextures(1, &indexTexture);
		indexTexture = 0;
	}
}
void SeismicWindow::initWindow(TPanel* parent)
{
	//Инициализируем базовое окно
	FlatWindow::initWindow(parent);
	//Создаём и устанавливаем шейдерную программу
	shaderProgram = createShaderProgram("universal", "seismic");
	setShaderProgram(shaderProgram);
}
//--
void SeismicWindow::initPaletteTexture(const std::array<uint8_t, 256*3>& paletteData)
{
	//Проверка инициализации
	if (!handle) throw Exception("Uninitialized window");
	glfwMakeContextCurrent(handle);
	//Создание текстуры
	glGenTextures(1, &paletteTexture);
	glBindTexture(GL_TEXTURE_1D, paletteTexture);
	//Заполнение текстуры
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE
	, paletteData.data());
	//Настраиваем текстуру
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}
//--
void SeismicWindow::initIndexTexture(const bitMap& texture)
{
	//Проверка инициализации
	if (!handle) throw Exception("Uninitialized window");
	glfwMakeContextCurrent(handle);
	//Выставляем параметры
	width = texture.width;
	height = texture.height;
	//Создаём текстуру
	glGenTextures(1, &indexTexture);
	glBindTexture(GL_TEXTURE_2D, indexTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Критически важно для 1-байтовых данных!
	//Заполняем текстуру
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,
				 GL_RED, GL_UNSIGNED_BYTE, texture.texture.data());
	//Настраиваем текстуру
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//Проверка ошибок
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		throw Exception(err);
	}
}
//--
void SeismicWindow::renderWindow()
{
	//Проверка инициализции
	if (!handle) return;
	//Очистка
	FlatWindow::preRender();
	//Родительский рендер
	FlatWindow::render();
	//Собственный рендер
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, indexTexture);
	glUniform1i(glGetUniformLocation(shaderProgram, "indexTexture"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture((GL_TEXTURE_1D), paletteTexture);
	glUniform1i(glGetUniformLocation(shaderProgram, "paletteTexture"), 1);
	//glUniform1i(glGetUniformLocation(shaderProgram, "selectedTrace"), selectedTrace);
	//Очистка буфферов
	FlatWindow::postRender();
}

