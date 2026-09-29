//---------------------------------------------------------------------------

#ifndef ViewportWindowH
#define ViewportWindowH
//---------------------------------------------------------------------------
#pragma once
#include "AbstractPlane.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <Windows.h>
#include <vector>
#include <memory>
#include <vcl.h>
#include "InterfacesWindow.h"


class ViewportWindow : public IWindow, public ICallbackWindow, public IVolumeWindow {
private:
	GLFWwindow* m_window;
    std::vector<std::shared_ptr<AbstractPlane>> m_planes;

	// Выделенная плоскость
	std::shared_ptr<AbstractPlane> m_selectedPlane;

    // Настройки рендеринга
	bool m_showIntersections = false;
	bool m_showHighlights = true;

    // Шейдеры
    GLuint m_highlightShader;
    GLuint m_intersectionShader;

    // Камера
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraTarget;
    glm::vec3 m_cameraUp;
    float m_fov;
    float m_nearPlane, m_farPlane;

    // Управление
    bool m_isRotating;
    bool m_isPanning;
    double m_lastMouseX, m_lastMouseY;

    int m_width, m_height;

    // Матрицы
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    // Колбэки
    std::function<void(std::shared_ptr<AbstractPlane>)> m_planeSelectionCallback;

    void updateMatrices();
    glm::vec3 screenToWorldRay(double mouseX, double mouseY) const;
    void renderIntersections();
	void renderHighlights();

	std::function<void(int, int, int)> mouseButtonCallback = [](int button, int action, int mods){};
	std::function<void(double, double)> cursorPosCallback = [](double x, double y){};
	std::function<void(double, double)> scrollCallback = [](double x, double y){};
	std::function<void(int)> cursorEnterCallback = [](int action){};


protected:

 	void preRender();
	void render();
	void renderHighlightsLayer();
	void renderIntersectionsLayer();
	void postRender();
    	// Выделение плоскостей
	void selectPlane(std::shared_ptr<AbstractPlane> plane);
	void deselectPlane();


	// Настройки рендеринга
	void setShowIntersections(bool show) { m_showIntersections = show; }
	void setShowHighlights(bool show) { m_showHighlights = show; }
	bool getShowIntersections() const { return m_showIntersections; }
	bool getShowHighlights() const { return m_showHighlights; }


        // Обработка ввода
	void handleMouseButton(int button, int action, int mods);
	void handleCursorPos(double xpos, double ypos);
	void handleScroll(double xoffset, double yoffset);

	// Ray picking
	std::shared_ptr<AbstractPlane> pickPlane(double mouseX, double mouseY, glm::vec3& intersection);

public:
    ViewportWindow();
    ~ViewportWindow();

	void initWindow(TPanel* parent) override;

    // Основной цикл рендеринга (аналог renderWindow в 2D)
	// Основной цикл рендеринга

	void renderWindow() override;

	void resizeWindow(int width, int height) override;
	GLFWwindow* getWindow() override;

	// Управление камерой
	void setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) override;
	void setPerspective(float fov, float nearPlane, float farPlane) override;

    // Управление плоскостями
	void addPlane(std::shared_ptr<AbstractPlane> plane) override;
	void removePlane(std::shared_ptr<AbstractPlane> plane) override;
	void clearPlanes() override;
	std::shared_ptr<AbstractPlane> getSelectedPlane() override { return m_selectedPlane; }


	void setChannels(int red, int green, int blue) override;
    void setChannelEnabled(bool red, bool green, bool blue) override;

    // Колбэки
    void setPlaneSelectionCallback(std::function<void(std::shared_ptr<AbstractPlane>)> callback) {
		m_planeSelectionCallback = callback;
	}
	void setMouseButtonCallback(std::function<void(int, int, int)> callback) override;
	void setCursorPosCallback(std::function<void(double, double)> callback) override;
	void setScrollCallback(std::function<void(double, double)> callback) override;
	void setCursorEnterCallback(std::function<void(int)> callback) override;



    bool shouldClose() { return glfwWindowShouldClose(m_window); }

    void processEvents() { glfwPollEvents(); }

private:
    static bool s_glfwInitialized;
    void clampCamera();
};
#endif
