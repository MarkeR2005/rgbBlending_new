//---------------------------------------------------------------------------
#pragma hdrstop
#include "BaseWindow.h"
#include "Shaders.h"
#include <algorithm>
#include <vcl.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)

float FlatWindow::staticZoom = 1.0f;
float FlatWindow::staticOffsetY = 0.0f;
float FlatWindow::staticOffsetX = 0.0f;

FlatWindow::FlatWindow()
{
registerSync(this);
}
//--
void FlatWindow::initWindow(TPanel* parent)
{
	//Проверка GLFW
	if (!glfwInit())
	{
		throw Exception("GLFW initialization failed");
	}
	if (handle) return;
	//Установка параметров и инициализация окна
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	handle = glfwCreateWindow(parent->Width, parent->Height, "MAIN", NULL, NULL);
	//Проверка инициализации
	if (!handle)
	{
		throw Exception("Window initialization failed");
	}
	//Привязка окна
	HWND hWndGL = glfwGetWin32Window(handle);
	::SetParent(hWndGL, parent->Handle);
	::SetWindowLongPtr(hWndGL, GWL_STYLE, WS_CHILD | WS_VISIBLE);
	::MoveWindow(hWndGL, 0, 0, parent->Width, parent->Height, TRUE);
	glfwMakeContextCurrent(handle);
	//Инициализация opengl
	glewExperimental = GL_TRUE;
	//Проверка инициализации
	if (glewInit()!=GLEW_OK)
	{
		throw Exception("GLEW initialization failed");
	}
	//Создаём прямоугольник
	float vertices[] =
	{
		// position     // texCoord
		0.0f, 0.0f,  0.0f, 0.0f,  // левый нижний
		 1.0f, 0.0f,  1.0f, 0.0f,  // правый нижний
		 1.0f,  1.0f,  1.0f, 1.0f,  // правый верхний
		0.0f,  1.0f,  0.0f, 1.0f   // левый верхний
	};
	//Устанавливаем параметры геометрии
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0,2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 4*sizeof(float)
	, (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);


	highlightProgram = createShaderProgram("universal", "highlight");
	horizonProgram = createShaderProgram("test", "horizon");

	//For horizons
	glGenVertexArrays(1, &VAO1);
	glGenBuffers(1, &VBO1);

	glBindVertexArray(VAO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glfwSetWindowUserPointer(handle, this);

	glfwSetMouseButtonCallback(handle, [](GLFWwindow* win, int button, int action, int mods)
	{
		FlatWindow* fw = static_cast<FlatWindow*>(glfwGetWindowUserPointer(win));
		if (fw) {
			fw->handleMouseButtonCallback(button, action, mods);
			fw->mouseButtonCallback(button, action, mods);
			fw->renderWindow();
		}
	});
	glfwSetCursorPosCallback(handle, [](GLFWwindow* win, double xpos, double ypos)
	{
		FlatWindow* fw = static_cast<FlatWindow*>(glfwGetWindowUserPointer(win));
		if (fw) {
			if (fw->isDrawing) {
				fw->handleCursorPosDrawCallback(xpos, ypos);
			}
			else {
				fw->handleCursorPosMoveCallback(xpos, ypos);
			}
			fw->cursorPosCallback(xpos, ypos);
//			if (fw->getSync()) {
//				ISyncWindow::emitToAll();
//				return;
//			}
			fw->renderWindow();
		}
	});
	glfwSetScrollCallback(handle, [](GLFWwindow* win, double offsetx, double offsety)
	{
		FlatWindow* fw = static_cast<FlatWindow*>(glfwGetWindowUserPointer(win));
		if (fw) {
			fw->handleScrollCallback(offsetx, offsety);
			fw->scrollCallback(offsetx, offsety);
//			if (fw->getSync()) {
//				ISyncWindow::emitToAll();
//				return;
//			}
			fw->renderWindow();
		}
	});
    glfwSetCursorEnterCallback(handle, [](GLFWwindow* win, int entered){
		FlatWindow* fw = static_cast<FlatWindow*>(glfwGetWindowUserPointer(win));
		if (fw) {
			fw->setDragging(false);
			fw->cursorEnterCallback(entered);
        }
	});
}
//--
//--
FlatWindow::~FlatWindow(){
	if (!handle) {
		return;
	}
	glfwMakeContextCurrent(handle);
	if (VBO != 0) {
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
	if (VAO != 0) {
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}
	if (VBO1 != 0) {
		glDeleteBuffers(1, &VBO1);
		VBO1 = 0;
	}
	if (VAO1 != 0) {
		glDeleteVertexArrays(1, &VAO1);
		VAO1 = 0;
	}
	if (shaderProgram != 0) {
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
	}
	if (highlightProgram != 0) {
		glDeleteProgram(highlightProgram);
		highlightProgram = 0;
	}
	if (horizonProgram != 0) {
		glDeleteProgram(horizonProgram);
		horizonProgram = 0;
	}

	if (handle != nullptr) {
		glfwSetMouseButtonCallback(handle, nullptr);
		glfwSetCursorPosCallback(handle, nullptr);
		glfwSetWindowUserPointer(handle, nullptr);
		glfwDestroyWindow(handle);
	}
}
//
void FlatWindow::handleSync() {
	setZoom(staticZoom);
	float ox, oy;
	getOffset(ox,oy);
	if (!isThin) {
		 ox = staticOffsetX;
	}
	setOffset(ox, staticOffsetY);
	evalThinRX();
	renderWindow();
}
//
void FlatWindow::setShaderProgram(GLuint program)
{
	shaderProgram = program;
}
//--
void FlatWindow::preRender()
{
	glfwMakeContextCurrent(handle);
	//Очистка области для рендеринга
	glfwGetFramebufferSize(handle, &Wwidth, &Wheight);
	glViewport(0, 0, Wwidth, Wheight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
//--
void FlatWindow::render()
{
	//Выбор программы для рендеринга
	glUseProgram(shaderProgram);
	//Получаем параметры
	glfwGetFramebufferSize(handle, &Wwidth, &Wheight);
	//Передача данных

	glUniform1f(glGetUniformLocation(shaderProgram, "zoom"), zoom);
	glUniform2f(glGetUniformLocation(shaderProgram, "imageSize"), width, height);
	glUniform2f(glGetUniformLocation(shaderProgram, "offset"), offsetX, offsetY);
	glUniform2f(glGetUniformLocation(shaderProgram, "windowSize"), Wwidth, Wheight);
	glUniform2f(glGetUniformLocation(shaderProgram, "pixelRatio"), pixelRatioX, pixelRatioY/dT);
	glUniform1f(glGetUniformLocation(shaderProgram, "contrast"), contrast);
}
//--
void FlatWindow::renderHighlights()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Выбор программы для рендеринга
	glUseProgram(highlightProgram);
	//Передача данных
	glfwGetFramebufferSize(handle, &Wwidth, &Wheight);
	//Передача данных
	glUniform1f(glGetUniformLocation(highlightProgram, "zoom"), zoom);
	glUniform2f(glGetUniformLocation(highlightProgram, "imageSize"), width, height);
	glUniform2f(glGetUniformLocation(highlightProgram, "offset"), offsetX, offsetY);
	glUniform2f(glGetUniformLocation(highlightProgram, "windowSize"), Wwidth, Wheight);
	glUniform2f(glGetUniformLocation(highlightProgram, "pixelRatio"), pixelRatioX, pixelRatioY/dT);
	for (point2 highlight : highlighted_points) {
		glUniform2f(glGetUniformLocation(highlightProgram, "highlightPos"), highlight.x, highlight.y);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);
	}
	glDisable(GL_BLEND);
}
//--
void FlatWindow::renderHorizons()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (horizon.empty()) return;

	glUseProgram(horizonProgram);

	glfwGetFramebufferSize(handle, &Wwidth, &Wheight);
	//Передача данных
	glUniform1f(glGetUniformLocation(horizonProgram, "zoom"), zoom);
	glUniform2f(glGetUniformLocation(horizonProgram, "imageSize"), width, height);
	glUniform2f(glGetUniformLocation(horizonProgram, "offset"), offsetX, offsetY*dT);
	glUniform2f(glGetUniformLocation(horizonProgram, "windowSize"), Wwidth, Wheight);
	glUniform2f(glGetUniformLocation(horizonProgram, "pixelRatio"), pixelRatioX, pixelRatioY);

	for (int i = 0; i < horizon.size(); i += width) {
		glUniform3f(glGetUniformLocation(horizonProgram, "uColor"), 0.0, (float)i/horizon.size(), 0.0);
		glBindVertexArray(VAO1);
		glLineWidth(5.0f);
		glDrawArrays(GL_LINE_STRIP_ADJACENCY, i, width);
		glBindVertexArray(0);
	}
	glDisable(GL_BLEND);
}
//--
void FlatWindow::postRender()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);


	renderHighlights();
	renderHorizons();
	glfwSwapBuffers(handle);
}
//--
void FlatWindow::renderWindow(){};
void FlatWindow::processEvents(){glfwPollEvents();};
//--
void FlatWindow::resizeWindow(int _width, int _height)
{
	if (handle)
	{
		// Обновление размеров GLFW окна
		glfwSetWindowSize(handle, _width, _height);
		glfwMakeContextCurrent(handle);
		glViewport(0, 0, _width, _height);
		processEvents();
		clampOffsets();
		evalThinRX();
        //renderWindow();
	}
}
void FlatWindow::evalThinRX(){
     if (isThin) {
			pixelRatioX = width*1.0/Wwidth*zoom;
	 }
}
//--
GLFWwindow* FlatWindow::getWindow()
{
	return handle;
}
//--
void FlatWindow::handleMouseButtonCallback(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		isDragging = true;
		glfwGetCursorPos(handle, &lastMouseX, &lastMouseY);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		isDragging = false;
		firstPoint = true;
	}
}
//
void FlatWindow::handleCursorPosDrawCallback(double _xpos, double _ypos) {
	if (isDragging) {
		int imgX = _xpos*pixelRatioX/zoom + offsetX;
		int imgY = _ypos*pixelRatioY/zoom/dT + offsetY;
		addPoint(imgX+width*horNum, imgY);

		renderWindow();
	}
}
//
void FlatWindow::handleCursorPosMoveCallback(double _xpos, double _ypos) {
	if (isDragging) {
		double dx = _xpos - lastMouseX;
		double dy = _ypos - lastMouseY;
		lastMouseX = _xpos;
		lastMouseY = _ypos;
		offsetX -= dx/zoom*pixelRatioX;
		offsetY -= dy/zoom*pixelRatioY/dT;
		clampOffsets();
		if (isSync) {
			staticOffsetY = offsetY;
			if (!isThin) {
			staticOffsetX = offsetX;
			}
			ISyncWindow::emitToAll();
			return;
		}
		renderWindow();
	}

}



void FlatWindow::handleScrollCallback(double xoffset, double yoffset) {
	// Настройки скорости
	const double scrollSpeed = 50.0;
	const double zoomSpeed = 0.1;

	// Масштабирование относительно центра мыши
	double mouseX, mouseY;
	glfwGetCursorPos(handle, &mouseX, &mouseY);

	// Сохраняем позицию в координатах изображения до масштабирования
	double prevZoom = zoom;

	// Изменяем масштаб
	zoom = std::max<float>(0.1, std::min(10.0, zoom * (1.0 + yoffset * zoomSpeed)));

	clampOffsets();

	offsetX += mouseX*(1/prevZoom-1/zoom);
	offsetY += mouseY*(1/prevZoom-1/zoom);

	evalThinRX();
	processEvents();
	clampOffsets();

	if (isSync) {
		staticZoom = zoom;
		staticOffsetY = offsetY;
		if (!isThin) {
			staticOffsetX = offsetX;
		}
		ISyncWindow::emitToAll();
		return;
	}
	renderWindow();
}

void FlatWindow::clampOffsets() {
	// Рассчитываем видимую область изображения

	zoom = std::max(zoom, static_cast<float>(Wheight)/height*pixelRatioY/dT);
	if (!isThin) {
		zoom = std::max(zoom, static_cast<float>(Wwidth)/width*pixelRatioX);
	}

	float visibleWidth = Wwidth / zoom * pixelRatioX;
	float visibleHeight = Wheight / zoom * pixelRatioY/dT;
	// Ограничиваем смещение
	offsetX = std::max(0.0f, std::min(offsetX, static_cast<float>(width) - visibleWidth));
	offsetY = std::max(0.0f, std::min(offsetY, static_cast<float>(height) - visibleHeight));
}

 void FlatWindow::setHorizon(const std::vector<float>& data) {
		horizon = data;
		updateGPUData();
	}


 void FlatWindow::updateGPUData() {
		std::vector<float> vertices;
		vertices.reserve(horizon.size() * 2);
		// Преобразуем в координаты OpenGL: X от 0 до 1, Y от 0 до 1
		for (size_t i = 0; i < horizon.size(); ++i) {
			float x = static_cast<float>(i%width);
			vertices.push_back(x);
			vertices.push_back(horizon[i]);
		}
		glBindBuffer(GL_ARRAY_BUFFER, VBO1);
		glBufferData(GL_ARRAY_BUFFER,
					vertices.size() * sizeof(float),
					vertices.data(),
					GL_DYNAMIC_DRAW);
	}

	void FlatWindow::updateGPUPoint(int indexL, int indexR) {
		// Обновляем только одну точку в GPU буфере
		for (int index = indexL; index <= indexR; index++) {
		float x = static_cast<float>(index%width);
		float point[2] = {x, horizon[index]};

		glBindBuffer(GL_ARRAY_BUFFER, VBO1);
		glBufferSubData(GL_ARRAY_BUFFER,
					   index * 2 * sizeof(float),
					   2 * sizeof(float),
					   point);
		}
	}
	void FlatWindow::addPoint(float x, float y) {
		if (firstPoint) {
            // Первая точка - просто запоминаем
			lastPosX = x;
			lastPosY = y*dT;
            firstPoint = false;
            return;
        }

		// Интерполируем между последней точкой и новой
		interpolateBetweenPoints(lastPosX, lastPosY, x, y*dT);

        // Обновляем последнюю точку
		lastPosX = x;
		lastPosY = y*dT;
	}
	void FlatWindow::interpolateBetweenPoints(float x1, float y1, float x2, float y2) {
		int startIdx = static_cast<int>(x1);
		int endIdx = static_cast<int>(x2);

        // Обеспечиваем правильный порядок
		if (startIdx > endIdx) {
			std::swap(startIdx, endIdx);
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        // Интерполируем все точки между startIdx и endIdx
        for (int i = startIdx; i <= endIdx && i < width*(horNum+1); ++i) {
            float t = (i - startIdx) / (float)(endIdx - startIdx + 1);
            t = std::max(0.0f, std::min(1.0f, t)); // Кламп [0, 1]

            // Линейная интерполяция
			horizon[i] = y1 + (y2 - y1) * t;
		}
		updateGPUPoint(startIdx, endIdx);
	}

	void FlatWindow::setMouseButtonCallback(std::function<void(int, int, int)> callback){
	mouseButtonCallback = callback;
}
void FlatWindow::setCursorPosCallback(std::function<void(double, double)> callback){
	cursorPosCallback = callback;
}
void FlatWindow::setScrollCallback(std::function<void(double, double)> callback){
	scrollCallback = callback;
}
void FlatWindow::setCursorEnterCallback(std::function<void(int)> callback){
	cursorEnterCallback = callback;
}
