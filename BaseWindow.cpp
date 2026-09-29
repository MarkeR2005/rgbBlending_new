//---------------------------------------------------------------------------
#pragma hdrstop
#include "BaseWindow.h"
#include "Shaders.h"
#include <algorithm>
#include <cmath>
#include <memory>
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
	//glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
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


	highlightProgram = create2DHighlightShaderProgram();
	horizonProgram = create2DHorizonShaderProgram();
    labelProgram = createCrossLabelShaderProgram();

	//For horizons
	glGenVertexArrays(1, &VAO1);
	glGenBuffers(1, &VBO1);

	glBindVertexArray(VAO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
    glGenVertexArrays(1, &labelVAO);
    glGenBuffers(1, &labelVBO);
    glBindVertexArray(labelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, labelVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glfwSetKeyCallback(handle, [](GLFWwindow* win, int key, int, int action, int mods) {
        auto* fw = static_cast<FlatWindow*>(glfwGetWindowUserPointer(win));
        if (!fw || action != GLFW_PRESS || (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER))) return;
        if (key == GLFW_KEY_H) fw->setHorizonsVisible(!fw->showHorizons);
        if (key == GLFW_KEY_C) fw->setCrossesVisible(!fw->showCrosses);
    });
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
    clearLabels(crossLabels);
    clearLabels(horizonLabels);
    if (labelVBO) glDeleteBuffers(1, &labelVBO);
    if (labelVAO) glDeleteVertexArrays(1, &labelVAO);
    if (labelProgram) glDeleteProgram(labelProgram);
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
    if (!showHorizons || horizons.empty()) return;
    glUseProgram(horizonProgram);
    glUniform1f(glGetUniformLocation(horizonProgram, "zoom"), zoom);
    glUniform2f(glGetUniformLocation(horizonProgram, "imageSize"), width, height);
    glUniform2f(glGetUniformLocation(horizonProgram, "offset"), offsetX, offsetY*dT);
    glUniform2f(glGetUniformLocation(horizonProgram, "windowSize"), Wwidth, Wheight);
    glUniform2f(glGetUniformLocation(horizonProgram, "pixelRatio"), pixelRatioX, pixelRatioY);
    glUniform3f(glGetUniformLocation(horizonProgram, "uColor"), 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    glBindVertexArray(VAO1);
    glLineWidth(3.0f);
    for (const auto& h : horizons) {
        // Split on missing ordinates so gaps do not join separate picks.
        std::vector<float> segment;
        auto flush = [&]() {
            if (segment.size() >= 4) {
                glBindBuffer(GL_ARRAY_BUFFER, VBO1);
                glBufferData(GL_ARRAY_BUFFER, segment.size()*sizeof(float), segment.data(), GL_DYNAMIC_DRAW);
                glDrawArrays(GL_LINE_STRIP, 0, segment.size()/2);
            }
            segment.clear();
        };
        for (size_t x = 0; x < h.points.size() && x < static_cast<size_t>(width); ++x) {
            if (!std::isfinite(h.points[x]) || h.points[x] < 0) { flush(); continue; }
            segment.push_back(static_cast<float>(x));
            segment.push_back(h.points[x]);
        }
        flush();
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    renderHorizonLabels();
}
void FlatWindow::renderCrosses()
{
    if (!showCrosses || crosses.empty()) return;
    glUseProgram(horizonProgram);
    glUniform1f(glGetUniformLocation(horizonProgram, "zoom"), zoom);
    glUniform2f(glGetUniformLocation(horizonProgram, "offset"), offsetX, offsetY*dT);
    glUniform2f(glGetUniformLocation(horizonProgram, "windowSize"), Wwidth, Wheight);
    glUniform2f(glGetUniformLocation(horizonProgram, "pixelRatio"), pixelRatioX, pixelRatioY);
    glUniform3f(glGetUniformLocation(horizonProgram, "uColor"), 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    glBindVertexArray(VAO1);
    glLineWidth(2.0f);
    for (const auto& cross : crosses) {
        if (cross.x < 0 || cross.x >= width) continue;
        float line[] = {static_cast<float>(cross.x), 0.0f,
                        static_cast<float>(cross.x), static_cast<float>(height)*dT};
        glBindBuffer(GL_ARRAY_BUFFER, VBO1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, 2);
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    renderCrossLabels();
}
void FlatWindow::drawLabel(const TextLabel& label, float x, float y) {
    if (!label.texture || x <= -label.width || x >= Wwidth ||
        y <= -label.height || y >= Wheight) return;
    const float w = static_cast<float>(label.width), h = static_cast<float>(label.height);
    const float quad[] = {x,y,0,0, x+w,y,1,0, x+w,y+h,1,1,
                          x,y,0,0, x+w,y+h,1,1, x,y+h,0,1};
    glBindTexture(GL_TEXTURE_2D, label.texture);
    glBindBuffer(GL_ARRAY_BUFFER, labelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void FlatWindow::renderCrossLabels() {
    glUseProgram(labelProgram);
    glUniform2f(glGetUniformLocation(labelProgram, "windowSize"), Wwidth, Wheight);
    glUniform1i(glGetUniformLocation(labelProgram, "label"), 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(labelVAO);
    for (size_t i = 0; i < crosses.size() && i < crossLabels.size(); ++i) {
        const float x = (crosses[i].x-offsetX)*zoom/pixelRatioX;
        drawLabel(crossLabels[i], x+4.0f, 4.0f);
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}
void FlatWindow::renderHorizonLabels() {
    glUseProgram(labelProgram);
    glUniform2f(glGetUniformLocation(labelProgram, "windowSize"), Wwidth, Wheight);
    glUniform1i(glGetUniformLocation(labelProgram, "label"), 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(labelVAO);
    for (size_t i = 0; i < horizons.size() && i < horizonLabels.size() &&
                       i < firstHorizonPoints.size(); ++i) {
        const int x = firstHorizonPoints[i];
        if (x < 0 || x >= static_cast<int>(horizons[i].points.size())) continue;
        const float sx = (x-offsetX)*zoom/pixelRatioX;
        const float sy = (horizons[i].points[x]-offsetY*dT)*zoom/pixelRatioY;
        drawLabel(horizonLabels[i], sx+3.0f,
                  sy-horizonLabels[i].height-4.0f);
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}
void FlatWindow::clearLabels(std::vector<TextLabel>& labels) {
    for (auto& label : labels) if (label.texture) glDeleteTextures(1, &label.texture);
    labels.clear();
}
FlatWindow::TextLabel FlatWindow::makeLabel(const std::wstring& name) {
    TextLabel label = {0, 0, 0};
    if (name.empty()) return label;
    std::unique_ptr<Graphics::TBitmap> bitmap(new Graphics::TBitmap);
    bitmap->PixelFormat = pf24bit;
    bitmap->Canvas->Font->Name = L"Arial";
    bitmap->Canvas->Font->Size = 10;
    label.width = std::max(1, bitmap->Canvas->TextWidth(name.c_str()) + 4);
    label.height = std::max(1, bitmap->Canvas->TextHeight(name.c_str()) + 4);
    bitmap->SetSize(label.width, label.height);
    bitmap->Canvas->Brush->Color = clBlack;
    bitmap->Canvas->FillRect(Rect(0, 0, label.width, label.height));
    bitmap->Canvas->Font->Name = L"Arial";
    bitmap->Canvas->Font->Size = 10;
    bitmap->Canvas->Font->Color = clWhite;
    bitmap->Canvas->Brush->Style = bsClear;
    bitmap->Canvas->TextOut(2, 2, name.c_str());
    std::vector<uint8_t> pixels(static_cast<size_t>(label.width)*label.height*3);
    for (int y = 0; y < label.height; ++y)
        std::copy_n(static_cast<uint8_t*>(bitmap->ScanLine[y]), label.width*3,
                    pixels.data()+static_cast<size_t>(y)*label.width*3);
    glGenTextures(1, &label.texture);
    glBindTexture(GL_TEXTURE_2D, label.texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, label.width, label.height, 0,
                 GL_BGR, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return label;
}
void FlatWindow::updateHorizonLabels() {
    if (!handle) return;
    glfwMakeContextCurrent(handle);
    clearLabels(horizonLabels);
    firstHorizonPoints.clear();
    for (const auto& horizon : horizons) {
        horizonLabels.push_back(makeLabel(horizon.name));
        int first = -1;
        for (size_t x = 0; x < horizon.points.size() && x < static_cast<size_t>(width); ++x) {
            if (std::isfinite(horizon.points[x]) && horizon.points[x] >= 0) {
                first = static_cast<int>(x);
                break;
            }
        }
        firstHorizonPoints.push_back(first);
    }
}
void FlatWindow::setCrosses(const std::vector<Cross>& value) {
    crosses = value;
    if (!handle) return;
    glfwMakeContextCurrent(handle);
    clearLabels(crossLabels);
    for (const auto& cross : crosses)
        crossLabels.push_back(makeLabel(cross.name));
    renderWindow();
}
void FlatWindow::setHorizons(const std::vector<Horizon>& value) {
    horizons = value;
    horNum = 0;
    firstPoint = true;
    updateHorizonLabels();
    renderWindow();
}
//--
void FlatWindow::postRender()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);


	renderHighlights();
	renderHorizons();
    renderCrosses();
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
		addPoint(imgX, imgY);

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

void FlatWindow::selectHorizon(size_t index, const std::wstring& name) {
    if (index > 10000) return;
    while (horizons.size() <= index) {
        Horizon next;
        next.name = L"Horizon " + std::to_wstring(horizons.size()+1);
        next.points.assign(width, -1.0f);
        horizons.push_back(std::move(next));
    }
    if (!name.empty()) horizons[index].name = name;
    updateHorizonLabels();
    horNum = index;
    firstPoint = true;
    isDrawing = true;
    renderWindow();
}
std::vector<float> FlatWindow::getHorizon() {
    std::vector<float> out;
    for (const auto& h : horizons) out.insert(out.end(), h.points.begin(), h.points.end());
    return out;
}
void FlatWindow::setHorizon(const std::vector<float>& data) {
    // Legacy text/binary format concatenates width-long picks.
    horizons.clear();
    if (width > 0) {
        for (size_t start = 0; start < data.size(); start += width) {
            Horizon h;
            h.name = L"Horizon " + std::to_wstring(horizons.size()+1);
            h.points.assign(data.begin()+start,
                            data.begin()+std::min(start+static_cast<size_t>(width), data.size()));
            horizons.push_back(std::move(h));
        }
    }
    horNum = 0;
    firstPoint = true;
    updateHorizonLabels();
    renderWindow();
}
void FlatWindow::addPoint(float x, float y) {
    if (horizons.empty()) {
        horizons.push_back({L"Horizon", std::vector<float>(width, -1.0f)});
        updateHorizonLabels();
    }
    if (firstPoint) {
        lastPosX = x;
        lastPosY = y*dT;
        firstPoint = false;
        return;
    }
    interpolateBetweenPoints(lastPosX, lastPosY, x, y*dT);
    lastPosX = x;
    lastPosY = y*dT;
}
void FlatWindow::interpolateBetweenPoints(float x1, float y1, float x2, float y2) {
    if (horNum >= horizons.size()) return;
    auto& picked = horizons[horNum].points;
    if (picked.size() < static_cast<size_t>(width)) picked.resize(width, -1.0f);
    int start = static_cast<int>(x1), end = static_cast<int>(x2);
    if (start > end) {std::swap(start, end); std::swap(y1, y2);}
    if (start == end) {
        if (start >= 0 && start < width) {
            picked[start] = y2;
            if (horNum < firstHorizonPoints.size() &&
                (firstHorizonPoints[horNum] < 0 || start < firstHorizonPoints[horNum]))
                firstHorizonPoints[horNum] = start;
        }
        return;
    }
    for (int i = std::max(0,start); i <= std::min(width-1,end); ++i) {
        const float t = static_cast<float>(i-start)/(end-start);
        picked[i] = y1+(y2-y1)*t;
        if (horNum < firstHorizonPoints.size() &&
            (firstHorizonPoints[horNum] < 0 || i < firstHorizonPoints[horNum]))
            firstHorizonPoints[horNum] = i;
    }
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

void FlatWindow::renderForScreenshot()
{
}

Graphics::TBitmap* FlatWindow::getScreenshotAsBitmap()
{
if (!handle) return nullptr;
Graphics::TBitmap* bmp = new Graphics::TBitmap;
bmp->PixelFormat = pf24bit;
float tzoom = zoom;
//--
float toffsetX = offsetX, toffsetY = offsetY;
float tWidth = Wwidth , tHeight = Wheight;

zoom = 1.0f;
offsetX = 0.0f;
offsetY = 0.0f;
Wwidth = width/pixelRatioX;
Wheight = height/pixelRatioY;
glfwSetWindowSize(handle, Wwidth, Wheight);
bmp->Width = Wwidth;
bmp->Height = Wheight+50;
renderForScreenshot();
glPixelStorei(GL_PACK_ALIGNMENT, 1);
for (int y = 0; y < Wheight; y++) {
    glReadPixels(0, y, Wwidth, 1, GL_BGR, GL_UNSIGNED_BYTE, bmp->ScanLine[Wheight -1 - y]);
}

zoom = tzoom;
offsetX = toffsetX ;
offsetY = toffsetY;
Wwidth = tWidth;
Wheight = tHeight;
glfwSetWindowSize(handle, Wwidth, Wheight);
return bmp;
}
