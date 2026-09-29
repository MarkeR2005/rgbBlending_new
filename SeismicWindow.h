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
#ifndef SeismicWindowH
#define SeismicWindowH
//---------------------------------------------------------------------------
class SeismicWindow : public FlatWindow
{
	public:
	SeismicWindow();
	~SeismicWindow() override;
	//--
	void initWindow(TPanel* parent) override;
	//--
	void initPaletteTexture(const std::array<uint8_t, 256*3>& paletteData);
	//--
	void initIndexTexture(const bitMap& texture);
	//--
	void renderWindow() override;
	//--
    void renderForScreenshot() override;

	protected:
	//--
	GLuint indexTexture = 0;
	//--
	GLuint paletteTexture = 0;
};
#endif
