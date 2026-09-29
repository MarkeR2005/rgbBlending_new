//---------------------------------------------------------------------------

#pragma hdrstop

#include "ViewportWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#include "ViewportWindow.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include "Shaders.h"
#include <algorithm>
#include "RgbPlane.h"

ViewportWindow::ViewportWindow()
    : m_window(nullptr), m_width(800), m_height(600),
      m_cameraPos(0.0f, 0.0f, 5.0f), m_cameraTarget(0.0f, 0.0f, 0.0f),
      m_cameraUp(0.0f, 1.0f, 0.0f), m_fov(45.0f), m_nearPlane(0.1f), m_farPlane(1000.0f),
	  m_isRotating(false), m_isPanning(false), m_lastMouseX(0.0), m_lastMouseY(0.0) {

}

ViewportWindow::~ViewportWindow() {
    if (m_window) {
		glfwDestroyWindow(m_window);
    }
}

GLFWwindow* ViewportWindow::getWindow() {
return m_window;
}

void ViewportWindow::initWindow(TPanel* parent) {
    if (!glfwInit()) {
		return;
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_window = glfwCreateWindow(parent->Width, parent->Height, "TEST3D", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
		return;
	}

	HWND hWndGL = glfwGetWin32Window(m_window);
    ::SetParent(hWndGL, parent->Handle);
    ::SetWindowLongPtr(hWndGL, GWL_STYLE, WS_CHILD | WS_VISIBLE);
	::MoveWindow(hWndGL, 0, 0, parent->Width, parent->Height, TRUE);

    glfwMakeContextCurrent(m_window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
		return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_width = parent->Width;
	m_height = parent->Height;
	glfwSetWindowUserPointer(m_window, this);

	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
		ViewportWindow* vp = static_cast<ViewportWindow*>(glfwGetWindowUserPointer(window));
		if (vp) {
		vp->handleMouseButton(button, action, mods);
		vp->mouseButtonCallback(button, action, mods);
		vp->renderWindow();
		}
	});
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
		ViewportWindow* vp = static_cast<ViewportWindow*>(glfwGetWindowUserPointer(window));
		if (vp) {
		vp->handleCursorPos(xpos, ypos);
		vp->cursorPosCallback(xpos, ypos);
		vp->renderWindow();
		}
	});

	glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
		ViewportWindow* vp = static_cast<ViewportWindow*>(glfwGetWindowUserPointer(window));
		if (vp) {
		vp->handleScroll(xoffset, yoffset);
		vp->scrollCallback(xoffset, yoffset);
		vp->renderWindow();
		}
	});
}
void ViewportWindow::preRender() {
    glfwMakeContextCurrent(m_window);

    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ViewportWindow::render() {
    updateMatrices();

	std::vector<std::shared_ptr<AbstractPlane>> opaquePlanes;
	std::vector<std::shared_ptr<AbstractPlane>> transparentPlanes;
	// Рендерим видимые плоскости
    for (auto& plane : m_planes) {
		if (plane->isVisible()) {
			if (plane->isTransparent()) {
                transparentPlanes.push_back(plane);
            } else {
                opaquePlanes.push_back(plane);
			}
		}
	}

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE); // Разрешаем запись в буфер глубины

    for (auto& plane : opaquePlanes) {
        plane->render(m_viewMatrix, m_projectionMatrix);
	}

    if (!transparentPlanes.empty()) {
		// Сортируем прозрачные плоскости по расстоянию от камеры (от дальних к ближним)
		glm::vec3 cameraPos = m_cameraPos;
        std::sort(transparentPlanes.begin(), transparentPlanes.end(),
			[&cameraPos](const std::shared_ptr<AbstractPlane>& a,
						const std::shared_ptr<AbstractPlane>& b) {
				float distA = a->calculateDistanceToCamera(cameraPos);
				float distB = b->calculateDistanceToCamera(cameraPos);
                return distA > distB; // Сначала рендерим дальние объекты
			});

        // Включаем смешивание для прозрачных объектов
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Отключаем запись в буфер глубины для прозрачных объектов
        glDisable(GL_CULL_FACE);

        for (auto& plane : transparentPlanes) {
            plane->render(m_viewMatrix, m_projectionMatrix);
        }

        // Восстанавливаем настройки
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
	}
}

void ViewportWindow::renderHighlightsLayer() {
    if (!m_showHighlights) return;

    // Рендерим выделение для выбранной плоскости
    if (m_selectedPlane && m_selectedPlane->isVisible()) {
		m_selectedPlane->renderHighlight(m_viewMatrix, m_projectionMatrix);
    }
}

void ViewportWindow::renderIntersectionsLayer() {
    if (!m_showIntersections || m_planes.size() < 2) return;
	 m_intersectionShader = createShaderProgram("volumetric1", "volumetric");
    glUseProgram(m_intersectionShader);

    // Передаем матрицы
    glUniformMatrix4fv(glGetUniformLocation(m_intersectionShader, "view"), 1, GL_FALSE, glm::value_ptr(m_viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_intersectionShader, "projection"), 1, GL_FALSE, glm::value_ptr(m_projectionMatrix));

    // Цвет линий пересечений (красный)
    glUniform3f(glGetUniformLocation(m_intersectionShader, "color"), 1.0f, 0.0f, 0.0f);

    // Настройки рендеринга линий
    glLineWidth(3.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Создаем VAO для линий один раз
    static GLuint lineVAO = 0, lineVBO = 0;
    if (lineVAO == 0) {
        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);
    }

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Рендерим линии пересечений для всех пар видимых плоскостей
    for (size_t i = 0; i < m_planes.size(); ++i) {
        if (!m_planes[i]->isVisible()) continue;

        for (size_t j = i + 1; j < m_planes.size(); ++j) {
            if (!m_planes[j]->isVisible()) continue;

            glm::vec3 linePoint, lineDir;
            if (m_planes[i]->intersectsWith(*m_planes[j], linePoint, lineDir)) {
                // Вычисляем точки линии в мировых координатах
                // Удлиняем линию в обе стороны от точки пересечения
                float lineLength = 50.0f; // Длина линии
                glm::vec3 lineStart = linePoint - lineDir * lineLength;
                glm::vec3 lineEnd = linePoint + lineDir * lineLength;

                float lineVertices[] = {
                    lineStart.x, lineStart.y, lineStart.z,
                    lineEnd.x, lineEnd.y, lineEnd.z
                };

                // Обновляем буфер и рисуем линию
                glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);
                glDrawArrays(GL_LINES, 0, 2);
            }
        }
    }

    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void ViewportWindow::postRender() {
	glfwSwapBuffers(m_window);
}


void ViewportWindow::renderWindow(){
	preRender();
	render();
	renderHighlightsLayer();
	renderIntersectionsLayer();
	postRender();
}

void ViewportWindow::updateMatrices() {
    float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
    m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraTarget, m_cameraUp);
}

void ViewportWindow::resizeWindow(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_window) {
        glfwSetWindowSize(m_window, width, height);
    }
}

void ViewportWindow::handleMouseButton(int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        glfwGetCursorPos(m_window, &m_lastMouseX, &m_lastMouseY);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            m_isRotating = true;
        } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            m_isPanning = true;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            // Обработка правого клика для выделения плоскостей
            double xpos, ypos;
            glfwGetCursorPos(m_window, &xpos, &ypos);

            glm::vec3 intersection;
            auto pickedPlane = pickPlane(xpos, ypos, intersection);

            if (pickedPlane) {
                // Если зажат Shift, добавляем/убираем из выделения
                if (mods & GLFW_MOD_SHIFT) {
                    if (pickedPlane == m_selectedPlane) {
                        deselectPlane();
                    } else {
                        selectPlane(pickedPlane);
                    }
                } else {
                    // Обычный клик - выделяем одну плоскость
                    selectPlane(pickedPlane);
                }

                // Вызываем колбэк
                if (m_planeSelectionCallback) {
                    m_planeSelectionCallback(pickedPlane);
                }

                renderWindow();
            } else {
                // Клик в пустоту - снимаем выделение
                deselectPlane();
                if (m_planeSelectionCallback) {
                    m_planeSelectionCallback(nullptr);
                }
                renderWindow();
            }
        }
    } else if (action == GLFW_RELEASE) {
        m_isRotating = false;
        m_isPanning = false;
    }
}

void ViewportWindow::handleCursorPos(double xpos, double ypos) {
    if (m_isRotating) {
        // Вращение камеры вокруг target
        double dx = xpos - m_lastMouseX;
        double dy = ypos - m_lastMouseY;

        // Вычисляем сферические координаты
        glm::vec3 direction = m_cameraPos - m_cameraTarget;
        float radius = glm::length(direction);

        // Используем std:: для математических функций
        float theta = std::atan2(direction.x, direction.z);
        float phi = std::acos(direction.y / radius);

        theta -= dx * 0.01f;
        phi -= dy * 0.01f;
        phi = glm::clamp(phi, 0.1f, 3.1f);

        m_cameraPos.x = m_cameraTarget.x + radius * std::sin(phi) * std::sin(theta);
        m_cameraPos.y = m_cameraTarget.y + radius * std::cos(phi);
        m_cameraPos.z = m_cameraTarget.z + radius * std::sin(phi) * std::cos(theta);

        m_lastMouseX = xpos;
        m_lastMouseY = ypos;

        renderWindow();
    } else if (m_isPanning) {
        // Панорамирование камеры
        double dx = xpos - m_lastMouseX;
        double dy = ypos - m_lastMouseY;

        glm::vec3 forward = glm::normalize(m_cameraTarget - m_cameraPos);
        glm::vec3 right = glm::normalize(glm::cross(forward, m_cameraUp));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));

        float panSpeed = 0.001f * glm::length(m_cameraPos - m_cameraTarget);

        m_cameraPos -= right * static_cast<float>(dx) * panSpeed;
        m_cameraPos += up * static_cast<float>(dy) * panSpeed;
        m_cameraTarget -= right * static_cast<float>(dx) * panSpeed;
        m_cameraTarget += up * static_cast<float>(dy) * panSpeed;

        m_lastMouseX = xpos;
        m_lastMouseY = ypos;

        renderWindow();
    }
}

void ViewportWindow::handleScroll(double xoffset, double yoffset) {
    // Приближение/отдаление (аналог zoom в 2D)
	glm::vec3 direction = glm::normalize(m_cameraPos - m_cameraTarget);
	m_cameraPos += direction * static_cast<float>(yoffset) * 50.f;

    clampCamera();
    renderWindow();
}

void ViewportWindow::clampCamera() {
    // Ограничиваем минимальное расстояние до target
    float minDistance = 0.1f;
    float distance = glm::length(m_cameraPos - m_cameraTarget);
    if (distance < minDistance) {
        m_cameraPos = m_cameraTarget + glm::normalize(m_cameraPos - m_cameraTarget) * minDistance;
    }
}

glm::vec3 ViewportWindow::screenToWorldRay(double mouseX, double mouseY) const {
    // Преобразуем координаты мыши в NDC
    float x = (2.0f * static_cast<float>(mouseX)) / m_width - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(mouseY)) / m_height;

    // Ray в clip space
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    // Переводим в eye space
    glm::vec4 rayEye = glm::inverse(m_projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Переводим в world space
    glm::vec3 rayWorld = glm::vec3(glm::inverse(m_viewMatrix) * rayEye);
    return glm::normalize(rayWorld);
}

std::shared_ptr<AbstractPlane> ViewportWindow::pickPlane(double mouseX, double mouseY, glm::vec3& intersection) {
    glm::vec3 rayOrigin = m_cameraPos;
    glm::vec3 rayDir = screenToWorldRay(mouseX, mouseY);

    std::shared_ptr<AbstractPlane> pickedPlane = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    glm::vec3 closestIntersection;

    for (auto& plane : m_planes) {
        glm::vec3 intersectionPoint;
        if (plane->intersectsRay(rayOrigin, rayDir, intersectionPoint)) {
            float distance = glm::length(intersectionPoint - rayOrigin);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestIntersection = intersectionPoint;
                pickedPlane = plane;
            }
        }
    }

    if (pickedPlane) {
        intersection = closestIntersection;
    }

    return pickedPlane;
}

void ViewportWindow::setCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
    m_cameraPos = position;
    m_cameraTarget = target;
    m_cameraUp = up;
}

void ViewportWindow::setPerspective(float fov, float nearPlane, float farPlane) {
    m_fov = fov;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

void ViewportWindow::addPlane(std::shared_ptr<AbstractPlane> plane) {
    m_planes.push_back(plane);
}

void ViewportWindow::clearPlanes() {
    m_planes.clear();
}
void ViewportWindow::selectPlane(std::shared_ptr<AbstractPlane> plane) {
    // Снимаем выделение с предыдущей плоскости
    if (m_selectedPlane) {
        m_selectedPlane->setHighlighted(false);
    }

    // Выделяем новую плоскость
    m_selectedPlane = plane;
    if (m_selectedPlane) {
        m_selectedPlane->setHighlighted(true);
    }
}

void ViewportWindow::deselectPlane() {
    if (m_selectedPlane) {
        m_selectedPlane->setHighlighted(false);
        m_selectedPlane = nullptr;
    }
}

void ViewportWindow::removePlane(std::shared_ptr<AbstractPlane> plane) {
    // Если удаляем выделенную плоскость - снимаем выделение
    if (plane == m_selectedPlane) {
        deselectPlane();
    }

    auto it = std::find(m_planes.begin(), m_planes.end(), plane);
    if (it != m_planes.end()) {
        m_planes.erase(it);
    }
}
void ViewportWindow::setChannels(int red, int green, int blue){
	for (auto plane : m_planes) {
		auto rgb = dynamic_cast<RgbPlane*>(plane.get());
		if (rgb) {
			rgb->setChannels(red, green, blue);
		}
	}
}
void ViewportWindow::setChannelEnabled(bool red, bool green, bool blue){
for (auto plane : m_planes) {
		auto rgb = dynamic_cast<RgbPlane*>(plane.get());
		if (rgb) {
			rgb->setChannelEnabled(red, green, blue);
		}
	}
}

void ViewportWindow::setMouseButtonCallback(std::function<void(int, int, int)> callback){
	mouseButtonCallback = callback;
}
void ViewportWindow::setCursorPosCallback(std::function<void(double, double)> callback){
	cursorPosCallback = callback;
}
void ViewportWindow::setScrollCallback(std::function<void(double, double)> callback){
	scrollCallback = callback;
}
void ViewportWindow::setCursorEnterCallback(std::function<void(int)> callback){
	cursorEnterCallback = callback;
}

