//--------------------------------------------------------------------------
#include "Structures.h"

#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <vector>
#include <GL/glew.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <array>
#include <map>
#include "InterfacesWindow.h"

#ifndef BaseWindowH
#define BaseWindowH

class FlatWindow : public IWindow, public ICallbackWindow, public IFlatWindow, public ISyncWindow
{
	public:
	FlatWindow();
	virtual ~FlatWindow();
    void DestroyWindow();
	//--
	virtual void initWindow(TPanel* parent) override;
	void setShaderProgram(GLuint program) override;
	void resizeWindow(int _width, int _height) override;

	virtual void preRender();
	virtual void render();
	virtual void renderHighlights();
	virtual void renderHorizons();
	virtual void postRender();

	virtual void renderWindow() override;
    void renderForScreenshot() override;
    Graphics::TBitmap* getScreenshotAsBitmap() override;

	//Getters
	void getOffset(float& ox, float& oy) override{ox = offsetX;oy = offsetY;};
	void getRatio(float& rx, float& ry) override{rx = pixelRatioX;ry = pixelRatioY;};
	float getZoom() override {return zoom;};
	void getSize(int& w, int& h) override {w = width;h = height;};
	GLFWwindow* getWindow() override;
	std::vector<float> getHorizon() override {return horizon;};
	float getDT() override {return dT;};
	//Setters
	virtual void setOffset(float ox, float oy) override {offsetX = ox;offsetY = oy;};
	virtual void setRatio(float rx, float ry) override {pixelRatioX = rx;pixelRatioY = ry;};
	void setZoom(float z) override {zoom = z;};
	void setDT(float dT_) override {dT = dT_;};
	void setContrast (float c) override {if (c < 0.0) return; contrast = c;};
	void setHorizon(const std::vector<float>& data) override;
	void setThin(){isThin = true;};
	void setSync(bool state) override {isSync = state;};
	bool getSync() override {return isSync;};

	void setDragging(bool state){isDragging = state;};
	//Handlers
	void handleCursorPosDrawCallback(double _xpos, double _ypos);
	void handleCursorPosMoveCallback(double _xpos, double _ypos);
	void handleMouseButtonCallback(int button, int action, int mods);
	void handleScrollCallback(double offsetx, double offsety);
	void processEvents();
	//AccesibleVariables

	void handleSync() override;

	void evalThinRX();

	void setMouseButtonCallback(std::function<void(int, int, int)> callback) override;
	void setCursorPosCallback(std::function<void(double, double)> callback) override;
	void setScrollCallback(std::function<void(double, double)> callback) override;
	void setCursorEnterCallback(std::function<void(int)> callback) override;
		//--
	protected:
	bool isSync = false;
    bool isDrawing = false;
	bool firstPoint = true;
	bool isThin = false;

	GLFWwindow* handle = nullptr;
	//--
	GLuint shaderProgram = 0;
	//--

	int width = 1,height = 1;
	//--
	float zoom = 0.1f;
	//--
	float offsetX = 0.0f, offsetY = 0.0f;
	//--
	float pixelRatioX = 1.0f, pixelRatioY = 1.0f;
	//--
    float dT = 1.0f;

	int Wwidth, Wheight;

	std::function<void(int, int, int)> mouseButtonCallback = [](int button, int action, int mods){};
	std::function<void(double, double)> cursorPosCallback = [](double x, double y){};
	std::function<void(double, double)> scrollCallback = [](double x, double y){};
	std::function<void(int)> cursorEnterCallback = [](int action){};


	private:
	int horNum = 0;
	std::vector<point2> highlighted_points = {};
	//static std::vector<FlatWindow*> instances_;

	static float staticZoom;
	static float staticOffsetY;
	static float staticOffsetX;

	GLuint VAO = 0;
	//--
	GLuint VBO = 0;
	GLuint VAO1 = 0;
	//--
	GLuint VBO1 = 0;
	//--

	float xpos = 0.0f, ypos = 0.0f;
	//--

	bool isDragging = false;
	//--
	double lastMouseX = 0.0;
	//--
	double lastMouseY = 0.0;
    int lastPosX = 0, lastPosY = 0;
    	//--
	float contrast = 1.0f;

	//--
	GLuint highlightProgram = 0, horizonProgram = 0;

	std::vector<float> horizon = {};
	void clampOffsets();
	// Обновление одной точки
	 void updateGPUData();
	 void updateGPUPoint(int indexL, int indexR);
	 void addPoint(float x1, float y1);
	 void interpolateBetweenPoints(float x1, float y1, float x2, float y2);
};
#endif
