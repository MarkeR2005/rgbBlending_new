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
    virtual void renderCrosses();
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
	std::vector<float> getHorizon() override;
    const std::vector<Horizon>& getHorizons() const {return horizons;}
    void setHorizons(const std::vector<Horizon>& value);
    void setCrosses(const std::vector<Cross>& value);
    std::string hoveredOverlay(double screenX, double screenY) const;
    void setHorizonsVisible(bool value) {showHorizons = value; renderWindow();}
    void setCrossesVisible(bool value) {showCrosses = value; renderWindow();}
    bool horizonsVisible() const {return showHorizons;}
    bool crossesVisible() const {return showCrosses;}
    void setHorizonEditing(bool value) {isDrawing = value; firstPoint = true;}
    bool horizonEditing() const {return isDrawing;}
    void selectHorizon(size_t index, const std::wstring& name = L"");
    bool setHorizonColor(size_t index, unsigned char r, unsigned char g, unsigned char b);
    bool setHorizonNegative(size_t index);
    bool setHorizonContrast(size_t index, bool value);
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
    void panByPixels(int dx, int dy);
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
	size_t horNum = 0;
    bool showHorizons = true, showCrosses = true;
    bool blackCrosses = false;
    std::vector<Horizon> horizons;
    std::vector<Cross> crosses;
    struct TextLabel { GLuint texture; int width; int height; };
    std::vector<TextLabel> crossLabels, horizonLabels;
    std::vector<std::pair<float, float>> visibleHorizonAnchors;
    GLuint labelProgram = 0, labelVAO = 0, labelVBO = 0;
    void clearLabels(std::vector<TextLabel>& labels);
    TextLabel makeLabel(const std::wstring& name);
    void updateHorizonLabels();
    void drawLabel(const TextLabel& label, float x, float y);
    void renderCrossLabels();
    void renderHorizonLabels(bool outline = false, float dx = 0.0f, float dy = 0.0f);
    void updateVisibleHorizonAnchors(const std::vector<std::vector<std::vector<float>>>& geometry);
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
    bool eraseStroke = false;
	//--
	double lastMouseX = 0.0;
	//--
	double lastMouseY = 0.0;
    int lastPosX = 0;
    float lastPosY = 0.0f;
    	//--
	float contrast = 1.0f;

	//--
	GLuint highlightProgram = 0, horizonProgram = 0;

	void clampOffsets();
	// Обновление одной точки
	 void addPoint(float x1, float y1);
	 void interpolateBetweenPoints(float x1, float y1, float x2, float y2);
};
#endif
