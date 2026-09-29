//---------------------------------------------------------------------------

#include <functional>
#include "vcl.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "AbstractPlane.h"

#ifndef InterfacesWindowH
#define InterfacesWindowH
//---------------------------------------------------------------------------

class IWindow
{
	public:
	virtual void initWindow(TPanel* parent) = 0;
	virtual void renderWindow() = 0;
	virtual GLFWwindow* getWindow() = 0;
	virtual void resizeWindow(int _width, int _height) = 0;
};

class ICallbackWindow
{
	public:
	virtual void setMouseButtonCallback(std::function<void(int, int, int)> callback) = 0;
	virtual void setCursorPosCallback(std::function<void(double, double)> callback) = 0;
	virtual void setScrollCallback(std::function<void(double, double)> callback) = 0;
	virtual void setCursorEnterCallback(std::function<void(int)> callback) = 0;
};

class ISyncWindow
{
	private:
	static std::vector<ISyncWindow*> instances_;
	public:
	static void registerSync (ISyncWindow* syncable){
		ISyncWindow::instances_.push_back(syncable);
	}
	~ISyncWindow(){
		auto it = std::find(instances_.begin(), instances_.end(), this);
		if (it != instances_.end()) {
			ISyncWindow::instances_.erase(it);
		}
	}
	static void emitToAll();
	virtual void handleSync() = 0;
	virtual void setSync(bool state) = 0;
    virtual bool getSync() = 0;
};
 class IFlatWindow
{
	public:
    virtual void setShaderProgram(GLuint program) = 0;
	virtual void getOffset(float& ox, float& oy) = 0;
	virtual void getRatio(float& rx, float& ry) = 0;
	virtual float getZoom() = 0;
	virtual void getSize(int& w, int& h) = 0;
	virtual std::vector<float> getHorizon() = 0;
	virtual float getDT() = 0;

	virtual void setOffset(float ox, float oy) = 0;
	virtual void setRatio(float rx, float ry) = 0;
	virtual void setZoom(float z) = 0;
	virtual void setDT(float dT_) = 0;
	virtual void setContrast (float c) = 0;
	virtual void setHorizon(const std::vector<float>& data) = 0;

    virtual void renderForScreenshot() = 0;
    virtual Graphics::TBitmap* getScreenshotAsBitmap() = 0;
};
class IRgbWindow
{
	public:
	virtual void setColor(int r, int g, int b) = 0;
	virtual void setView(bool r, bool g, bool b) = 0;
};
 class IVolumeWindow
{
	public:
	virtual void setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) = 0;
	virtual void setPerspective(float fov, float nearPlane, float farPlane) = 0;

    virtual void setChannels(int red, int green, int blue) = 0;
    virtual void setChannelEnabled(bool red, bool green, bool blue) = 0;

	virtual void addPlane(std::shared_ptr<AbstractPlane> plane) = 0;
	virtual void removePlane(std::shared_ptr<AbstractPlane> plane) = 0;
	virtual void clearPlanes() = 0;
	virtual std::shared_ptr<AbstractPlane> getSelectedPlane() = 0;
};
#endif
