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

// Clip a horizon segment to the visible pixel rectangle. The first returned
// endpoint is the leftmost visible point because horizon trace indices increase.
namespace {
std::string utf8Name(const std::wstring& value) {
    if (value.empty()) return {};
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, value.c_str(),
        static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (bytes <= 0) return {};
    std::string result(bytes, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
        &result[0], bytes, nullptr, nullptr);
    return result;
}
bool isHorizonPoint(float value) { return std::isfinite(value) && value != -1.0f; }
bool clipHorizonSegment(float& x0, float& y0, float& x1, float& y1,
                        float viewportWidth, float viewportHeight) {
    const float dx = x1-x0, dy = y1-y0;
    float enter = 0.0f, leave = 1.0f;
    auto clipEdge = [&](float p, float q) {
        if (p == 0.0f) return q >= 0.0f;
        const float t = q/p;
        if (p < 0.0f) enter = std::max(enter, t);
        else leave = std::min(leave, t);
        return enter <= leave;
    };
    if (!clipEdge(-dx, x0) || !clipEdge(dx, viewportWidth-x0) ||
        !clipEdge(-dy, y0) || !clipEdge(dy, viewportHeight-y0)) return false;
    const float startX = x0, startY = y0;
    x0 = startX + enter*dx; y0 = startY + enter*dy;
    x1 = startX + leave*dx; y1 = startY + leave*dy;
    return true;
}
}

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
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
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
    updateVisibleHorizonAnchors();
    glUseProgram(horizonProgram);
    glUniform1f(glGetUniformLocation(horizonProgram, "zoom"), zoom);
    glUniform2f(glGetUniformLocation(horizonProgram, "offset"), offsetX, offsetY*dT);
    glUniform2f(glGetUniformLocation(horizonProgram, "windowSize"), Wwidth, Wheight);
    glUniform2f(glGetUniformLocation(horizonProgram, "pixelRatio"), pixelRatioX, pixelRatioY);
    struct Geometry { std::vector<std::vector<float>> strips; std::vector<float> dots; };
    std::vector<Geometry> geometry(horizons.size());
    for (size_t i = 0; i < horizons.size(); ++i) {
        std::vector<float> strip;
        auto flush = [&]() {
            if (strip.size() >= 4) geometry[i].strips.push_back(std::move(strip));
            else if (strip.size() == 2) geometry[i].dots.insert(geometry[i].dots.end(), strip.begin(), strip.end());
            strip.clear();
        };
        const auto& points = horizons[i].points;
        for (size_t x = 0; x < points.size() && x < static_cast<size_t>(width); ++x) {
            if (!isHorizonPoint(points[x])) { flush(); continue; }
            strip.push_back(static_cast<float>(x));
            strip.push_back(points[x]);
        }
        flush();
    }
    auto draw = [&](const Geometry& shape, float stroke) {
        glBindVertexArray(VAO1);
        glLineWidth(stroke);
        for (const auto& strip : shape.strips) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO1);
            glBufferData(GL_ARRAY_BUFFER, strip.size()*sizeof(float), strip.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINE_STRIP, 0, strip.size()/2);
        }
        if (!shape.dots.empty()) {
            glPointSize(stroke);
            glBindBuffer(GL_ARRAY_BUFFER, VBO1);
            glBufferData(GL_ARRAY_BUFFER, shape.dots.size()*sizeof(float), shape.dots.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_POINTS, 0, shape.dots.size()/2);
        }
        glBindVertexArray(0);
    };
    const GLint colorUniform = glGetUniformLocation(horizonProgram, "uColor");
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    // Reserve every center pixel before any outlines are drawn.
    for (size_t i = 0; i < horizons.size(); ++i) {
        const auto& h = horizons[i];
        glUseProgram(horizonProgram);
        if (h.useColor) {
            glDisable(GL_BLEND);
            glUniform3f(colorUniform, h.red/255.0f, h.green/255.0f, h.blue/255.0f);
        } else {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
            glUniform3f(colorUniform, 1.0f, 1.0f, 1.0f);
        }
        draw(geometry[i], 3.0f);
    }
    renderHorizonLabels();
    glDisable(GL_BLEND);
    glUseProgram(horizonProgram);
    for (size_t i = 0; i < horizons.size(); ++i) {
        const auto& h = horizons[i];
        if (!h.contrast) continue;
        const bool bright = !h.useColor || (299*h.red + 587*h.green + 114*h.blue >= 128000);
        const float halo = bright ? 0.0f : 1.0f;
        glUniform3f(colorUniform, halo, halo, halo);
        draw(geometry[i], 7.0f);
    }
    renderHorizonLabels(true, -1.0f, 0.0f);
    renderHorizonLabels(true,  1.0f, 0.0f);
    renderHorizonLabels(true, 0.0f, -1.0f);
    renderHorizonLabels(true, 0.0f,  1.0f);
    glDisable(GL_STENCIL_TEST);
}
void FlatWindow::renderCrosses()
{
    if (!showCrosses || crosses.empty()) return;
    glUseProgram(horizonProgram);
    glUniform1f(glGetUniformLocation(horizonProgram, "zoom"), zoom);
    glUniform2f(glGetUniformLocation(horizonProgram, "offset"), offsetX, offsetY*dT);
    glUniform2f(glGetUniformLocation(horizonProgram, "windowSize"), Wwidth, Wheight);
    glUniform2f(glGetUniformLocation(horizonProgram, "pixelRatio"), pixelRatioX, pixelRatioY);
    const float crossColor = blackCrosses ? 0.0f : 1.0f;
    glUniform3f(glGetUniformLocation(horizonProgram, "uColor"),
                crossColor, crossColor, crossColor);
    glDisable(GL_BLEND);
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
    const float crossColor = blackCrosses ? 0.0f : 1.0f;
    glUniform3f(glGetUniformLocation(labelProgram, "textColor"),
                crossColor, crossColor, crossColor);
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(labelVAO);
    for (size_t i = 0; i < crosses.size() && i < crossLabels.size(); ++i) {
        const float x = (crosses[i].x-offsetX)*zoom/pixelRatioX;
        drawLabel(crossLabels[i], x+4.0f, 4.0f);
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}
void FlatWindow::updateVisibleHorizonAnchors() {
    visibleHorizonAnchors.assign(horizons.size(), {static_cast<float>(Wwidth)+1.0f, 0.0f});
    if (Wwidth <= 0 || Wheight <= 0 || zoom <= 0 || pixelRatioX <= 0 || pixelRatioY <= 0) return;
    const int left = std::max(0, static_cast<int>(std::floor(offsetX)) - 1);
    const int right = std::min(width - 1,
        static_cast<int>(std::ceil(offsetX + Wwidth*pixelRatioX/zoom)) + 1);
    for (size_t i = 0; i < horizons.size(); ++i) {
        const auto& points = horizons[i].points;
        const int last = std::min(right, static_cast<int>(points.size()) - 1);
        float anchorX = static_cast<float>(Wwidth) + 1, anchorY = 0;
        for (int x = left; x <= last; ++x) {
            if (!isHorizonPoint(points[x])) continue;
            float sx = (x-offsetX)*zoom/pixelRatioX;
            float sy = (points[x]-offsetY*dT)*zoom/pixelRatioY;
            if (sx >= 0 && sx < Wwidth && sy >= 0 && sy < Wheight && sx < anchorX) {
                anchorX = sx;
                anchorY = sy;
            }
            if (x == last || !isHorizonPoint(points[x+1])) continue;
            float ex = (x+1-offsetX)*zoom/pixelRatioX;
            float ey = (points[x+1]-offsetY*dT)*zoom/pixelRatioY;
            if (clipHorizonSegment(sx, sy, ex, ey, Wwidth, Wheight) && sx < anchorX) {
                anchorX = sx;
                anchorY = sy;
            }
        }
        visibleHorizonAnchors[i] = {anchorX, anchorY};
    }
}
void FlatWindow::renderHorizonLabels(bool outline, float dx, float dy) {
    if (Wwidth <= 0 || Wheight <= 0 || zoom <= 0 || pixelRatioX <= 0 || pixelRatioY <= 0) return;
    glUseProgram(labelProgram);
    glUniform2f(glGetUniformLocation(labelProgram, "windowSize"), Wwidth, Wheight);
    glUniform1i(glGetUniformLocation(labelProgram, "label"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(labelVAO);
    for (size_t i = 0; i < visibleHorizonAnchors.size() && i < horizonLabels.size(); ++i) {
        const auto& anchor = visibleHorizonAnchors[i];
        const auto& h = horizons[i];
        if (anchor.first >= Wwidth || (outline && !h.contrast)) continue;
        if (outline || h.useColor) glDisable(GL_BLEND);
        else {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        }
        float r = 1.0f, g = 1.0f, b = 1.0f;
        if (outline) {
            const bool bright = !h.useColor || (299*h.red + 587*h.green + 114*h.blue >= 128000);
            r = g = b = bright ? 0.0f : 1.0f;
        } else if (h.useColor) {
            r = h.red/255.0f; g = h.green/255.0f; b = h.blue/255.0f;
        }
        glUniform3f(glGetUniformLocation(labelProgram, "textColor"), r, g, b);
        drawLabel(horizonLabels[i], anchor.first+3.0f+dx,
            std::max(0.0f, anchor.second-horizonLabels[i].height-4.0f)+dy);
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
    for (const auto& horizon : horizons) {
        horizonLabels.push_back(makeLabel(horizon.name));
    }
}
std::string FlatWindow::hoveredOverlay(double screenX, double screenY) const {
    if (zoom <= 0 || pixelRatioX <= 0 || pixelRatioY <= 0) return {};
    const double trace = screenX*pixelRatioX/zoom+offsetX;
    // Compare in screen pixels so the hover tolerance stays the same at any zoom.
    if (showHorizons) {
        const int left = std::max(0, static_cast<int>(std::floor(trace))-1);
        for (const auto& horizon : horizons) {
            const int right = std::min(std::min(width-1, static_cast<int>(horizon.points.size())-1),
                                       static_cast<int>(std::ceil(trace))+1);
            for (int x = left; x <= right; ++x) {
                const float ordinate = horizon.points[x];
                if (!isHorizonPoint(ordinate)) continue;
                const double sx = (x-offsetX)*zoom/pixelRatioX;
                const double sy = (ordinate-offsetY*dT)*zoom/pixelRatioY;
                double distance2 = (screenX-sx)*(screenX-sx)+(screenY-sy)*(screenY-sy);
                if (x < right && isHorizonPoint(horizon.points[x+1])) {
                    const double ex = (x+1-offsetX)*zoom/pixelRatioX;
                    const double ey = (horizon.points[x+1]-offsetY*dT)*zoom/pixelRatioY;
                    const double dx = ex-sx, dy = ey-sy;
                    const double fraction = std::max(0.0, std::min(1.0,
                        ((screenX-sx)*dx+(screenY-sy)*dy)/(dx*dx+dy*dy)));
                    const double nearX = sx+fraction*dx, nearY = sy+fraction*dy;
                    distance2 = std::min(distance2,
                        (screenX-nearX)*(screenX-nearX)+(screenY-nearY)*(screenY-nearY));
                }
                if (distance2 <= 25.0) return "Hor: " + utf8Name(horizon.name);
            }
        }
    }
    if (showCrosses) {
        for (const auto& cross : crosses) {
            if (cross.x < 0 || cross.x >= width) continue;
            const double sx = (cross.x-offsetX)*zoom/pixelRatioX;
            if (std::abs(screenX-sx) <= 4.0) return "Cross: " + utf8Name(cross.name);
        }
    }
    return {};
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
    auto updated = value;
    for (auto& incoming : updated) {
        for (const auto& existing : horizons) {
            if (incoming.name == existing.name) {
                incoming.useColor = existing.useColor;
                incoming.contrast = existing.contrast;
                incoming.red = existing.red;
                incoming.green = existing.green;
                incoming.blue = existing.blue;
                break;
            }
        }
    }
    horizons = std::move(updated);
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
        eraseStroke = isDrawing && (mods & GLFW_MOD_SHIFT);
        glfwGetCursorPos(handle, &lastMouseX, &lastMouseY);
        if (isDrawing) handleCursorPosDrawCallback(lastMouseX, lastMouseY);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        isDragging = false;
        eraseStroke = false;
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
        if (eraseStroke) return;
        horizons.push_back({L"Horizon", std::vector<float>(width, -1.0f)});
        updateHorizonLabels();
    }
    if (firstPoint) {
        lastPosX = static_cast<int>(x);
        lastPosY = y*dT;
        firstPoint = false;
    }
    interpolateBetweenPoints(lastPosX, lastPosY, x, y*dT);
    lastPosX = static_cast<int>(x);
    lastPosY = y*dT;
}

void FlatWindow::interpolateBetweenPoints(float x1, float y1, float x2, float y2) {
    if (horNum >= horizons.size()) return;
    auto& picked = horizons[horNum].points;
    if (picked.size() < static_cast<size_t>(width) && !eraseStroke)
        picked.resize(width, -1.0f);
    int start = static_cast<int>(x1), end = static_cast<int>(x2);
    if (start > end) {std::swap(start, end); std::swap(y1, y2);}
    const int right = std::min(end, std::min(width, static_cast<int>(picked.size()))-1);
    for (int i = std::max(0, start); i <= right; ++i) {
        const float t = start == end ? 1.0f : static_cast<float>(i-start)/(end-start);
        const float y = y1+(y2-y1)*t;
        if (eraseStroke) {
            const float tolerance = 10.0f*pixelRatioY/zoom;
            if (isHorizonPoint(picked[i]) && std::abs(picked[i]-y) <= tolerance)
                picked[i] = -1.0f;
        } else picked[i] = y;
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

bool FlatWindow::setHorizonColor(size_t index, unsigned char r, unsigned char g, unsigned char b) {
    if (index >= horizons.size()) return false;
    auto& h = horizons[index];
    h.useColor = true;
    h.red = r; h.green = g; h.blue = b;
    renderWindow();
    return true;
}
bool FlatWindow::setHorizonNegative(size_t index) {
    if (index >= horizons.size()) return false;
    horizons[index].useColor = false;
    renderWindow();
    return true;
}
bool FlatWindow::setHorizonContrast(size_t index, bool value) {
    if (index >= horizons.size()) return false;
    horizons[index].contrast = value;
    renderWindow();
    return true;
}
