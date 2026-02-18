//---------------------------------------------------------------------------

#pragma hdrstop

#include "AbstractPlane.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#include "Shaders.h"
#include <iostream>

AbstractPlane::AbstractPlane(const glm::vec3& anchor, const glm::vec3& normal,
						   const glm::vec2& size, GLuint shaderProgram, bool flipV, bool flipH)
    : m_anchor(anchor), m_normal(glm::normalize(normal)), m_size(size),
      m_shaderProgram(shaderProgram), m_textureID(0),
	  m_VAO(0), m_VBO(0), m_EBO(0), m_highlightVAO(0), m_highlightVBO(0),
      m_visible(true), m_highlighted(false), m_flipVertical(flipV), m_flipHorizontal(flipH) {
    setupGeometry();
    setupHighlightGeometry();
}

AbstractPlane::~AbstractPlane() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_highlightVAO) glDeleteVertexArrays(1, &m_highlightVAO);
    if (m_highlightVBO) glDeleteBuffers(1, &m_highlightVBO);
    if (m_textureID) glDeleteTextures(1, &m_textureID);
}

void AbstractPlane::setupGeometry() {
    // Основная геометрия плоскости (прямоугольник)
    float vertices[] = {
        // position     // texCoord
        -0.5f, -0.5f, 0.0f, m_flipHorizontal ? 1.0f : 0.0f, m_flipVertical ? 1.0f : 0.0f,
         0.5f, -0.5f, 0.0f, m_flipHorizontal ? 0.0f : 1.0f, m_flipVertical ? 1.0f : 0.0f,
         0.5f,  0.5f, 0.0f, m_flipHorizontal ? 0.0f : 1.0f, m_flipVertical ? 0.0f : 1.0f,
        -0.5f,  0.5f, 0.0f, m_flipHorizontal ? 1.0f : 0.0f, m_flipVertical ? 0.0f : 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void AbstractPlane::setupHighlightGeometry() {
    // Геометрия для контура выделения (линии по краям)
    float vertices[] = {
        // Линия 1
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        // Линия 2
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        // Линия 3
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
        // Линия 4
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };

    glGenVertexArrays(1, &m_highlightVAO);
    glGenBuffers(1, &m_highlightVBO);

    glBindVertexArray(m_highlightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void AbstractPlane::renderHighlight(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_highlighted || !m_visible) return;

    // Используем простой шейдер для выделения
	GLuint highlightShader = createShaderProgram("volumetric", "volumetric"); // Будет установлен извне
    glUseProgram(highlightShader);

    // Вычисляем матрицу модели так же, как в основном рендеринге
    glm::mat4 model = glm::mat4(1.0f);

    glm::vec3 arbitrary = glm::vec3(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(m_normal, arbitrary)) > 0.9f) {
        arbitrary = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 right = glm::normalize(glm::cross(m_normal, arbitrary));
    glm::vec3 up = glm::normalize(glm::cross(right, m_normal));

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(m_normal, 0.0f);

    model = glm::translate(glm::mat4(1.0f), m_anchor) * rotation * glm::scale(glm::mat4(1.0f), glm::vec3(m_size.x, m_size.y, 1.0f));

    // Передаем матрицы
    glUniformMatrix4fv(glGetUniformLocation(highlightShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(highlightShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(highlightShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Цвет выделения (желтый)
    glUniform3f(glGetUniformLocation(highlightShader, "color"), 1.0f, 1.0f, 0.0f);

    // Рендерим контур
    glBindVertexArray(m_highlightVAO);
    glDrawArrays(GL_LINES, 0, 8); // 8 вершин для 4 линий
    glBindVertexArray(0);
}

void AbstractPlane::updateTexture(const bitMap& texture) {
	if (m_textureID == 0) {
		glGenTextures(1, &m_textureID);
    }
	m_size.x = texture.width;
	m_size.y = texture.height;

	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, texture.width, texture.height, 0,
				 GL_RED, GL_UNSIGNED_BYTE, texture.texture.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

bool AbstractPlane::intersectsRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                 glm::vec3& intersectionPoint) const {
    if (!m_visible) return false;

	glm::mat4 model = glm::mat4(1.0f);

    // Находим оси в плоскости
    glm::vec3 arbitrary = glm::vec3(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(m_normal, arbitrary)) > 0.9f) {
        arbitrary = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 right = glm::normalize(glm::cross(m_normal, arbitrary));
    glm::vec3 up = glm::normalize(glm::cross(right, m_normal));

    // Строим матрицу ориентации
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(m_normal, 0.0f);

    // Применяем трансформации в правильном порядке
    model = glm::translate(glm::mat4(1.0f), m_anchor) * rotation * glm::scale(glm::mat4(1.0f), glm::vec3(m_size.x, m_size.y, 1.0f));

    // Преобразуем луч в пространство модели плоскости
    glm::mat4 invModel = glm::inverse(model);
    glm::vec3 localRayOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 localRayDir = glm::vec3(invModel * glm::vec4(rayDir, 0.0f));

    // Проверяем пересечение с плоскостью Z=0 в локальном пространстве
    if (std::abs(localRayDir.z) < 1e-6) return false; // параллельно

    float t = -localRayOrigin.z / localRayDir.z;
    if (t < 0) return false; // позади камеры

    glm::vec3 localIntersection = localRayOrigin + localRayDir * t;

    // Проверяем, находится ли точка внутри прямоугольника [-0.5, 0.5]
    if (localIntersection.x >= -0.5f && localIntersection.x <= 0.5f &&
        localIntersection.y >= -0.5f && localIntersection.y <= 0.5f) {
        intersectionPoint = glm::vec3(model * glm::vec4(localIntersection, 1.0f));
        return true;
    }

	return false;
}

bool AbstractPlane::intersectsWith(const AbstractPlane& other, glm::vec3& linePoint, glm::vec3& lineDir) const {
    // Вычисляем направление линии пересечения (векторное произведение нормалей)
    lineDir = glm::cross(m_normal, other.m_normal);

    // Если плоскости параллельны
    float dirLength = glm::length(lineDir);
    if (dirLength < 1e-6f) {
        return false;
    }

    lineDir = glm::normalize(lineDir);

    // Находим точку на линии пересечения
    // Решаем систему уравнений для двух плоскостей:
    // n1 • p = d1, n2 • p = d2
    // где d1 = n1 • anchor1, d2 = n2 • anchor2

    float d1 = glm::dot(m_normal, m_anchor);
    float d2 = glm::dot(other.m_normal, other.m_anchor);

    // Используем метод с определителями
    glm::vec3 n1 = m_normal;
    glm::vec3 n2 = other.m_normal;

    // Вычисляем вспомогательные векторы
    glm::vec3 n1n2 = glm::cross(n1, n2);
    glm::vec3 point = glm::cross(d1 * n2 - d2 * n1, n1n2) / glm::dot(n1n2, n1n2);

    linePoint = point;
    return true;
}
float AbstractPlane::calculateDistanceToCamera(const glm::vec3& cameraPos) const {
	return glm::distance(cameraPos, getAnchor());
}
