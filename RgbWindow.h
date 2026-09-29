//---------------------------------------------------------------------------
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <vector>
#include <GL/glew.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include "BaseWindow.h"
#include "Structures.h"
#include <memory>
class RgbData;
#ifndef RgbWindowH
#define RgbWindowH
//---------------------------------------------------------------------------
class RgbWindow : public FlatWindow, public IRgbWindow
{
	public:
	RgbWindow();
	~RgbWindow() override;
	//--
	void initWindow(TPanel* parent) override;
	//--
	void initData(std::shared_ptr<RgbData> data);
	//--
	void renderWindow() override;

    void renderForScreenshot() override;

	//--
	bool inverse = false;
	//--
	void setColor(int _R, int _G, int _B) override;
	//--
	void setView(bool _isR, bool _isG, bool _isB) override;


	private:
	int R = 0, G = 0, B = 0;
	//--
	bool isR = true, isG = true, isB = true;

	GLuint indexTexture = 0;
	//
	std::shared_ptr<RgbData> source;
	bool textureDirty = true;
    int allocatedWidth = 0, allocatedHeight = 0;
	void updateComposite();
};
#endif

