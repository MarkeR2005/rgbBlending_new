//---------------------------------------------------------------------------

#ifndef AbstractPlaneH
#define AbstractPlaneH
//---------------------------------------------------------------------------
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>
#include <functional>
#include "Structures.h"

class AbstractPlane {
protected:
    glm::vec3 m_anchor;
    glm::vec3 m_normal;
	glm::vec2 m_size;

	bool m_flipHorizontal = false;
	bool m_flipVertical = false;

    GLuint m_shaderProgram;
	GLuint m_textureID;
	float m_transparency = 0.0f;

    // Геометрия
    GLuint m_VAO, m_VBO, m_EBO;
    GLuint m_highlightVAO, m_highlightVBO; // Для контура выделения

    // Состояние
	bool m_visible = true;
	bool m_highlighted = false;

    virtual void setupGeometry();
    virtual void setupHighlightGeometry(); // Геометрия для контура
	virtual void setupShader() = 0;

public:
    AbstractPlane(const glm::vec3& anchor, const glm::vec3& normal,
                  const glm::vec2& size, GLuint shaderProgram, bool flipV = false, bool flipH = false);
    virtual ~AbstractPlane();

    virtual void render(const glm::mat4& view, const glm::mat4& projection) = 0;
    virtual void renderHighlight(const glm::mat4& view, const glm::mat4& projection);
	virtual void updateTexture(const bitMap& texture);

    // Для ray intersection
    virtual bool intersectsRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                              glm::vec3& intersectionPoint) const;

    // Геттеры/сеттеры
    const glm::vec3& getAnchor() const { return m_anchor; }
    const glm::vec3& getNormal() const { return m_normal; }
    const glm::vec2& getSize() const { return m_size; }
    GLuint getTextureID() const { return m_textureID; }

    bool isVisible() const { return m_visible; }
	void setVisible(bool visible) { m_visible = visible; }
	void setTransparency(float tr) { m_transparency = tr; }
    bool isTransparent() { return m_transparency > 0.0f; }

    bool isHighlighted() const { return m_highlighted; }
	void setHighlighted(bool highlighted) { m_highlighted = highlighted; }

    void setAnchor(const glm::vec3& anchor) { m_anchor = anchor; }
    void setNormal(const glm::vec3& normal) { m_normal = glm::normalize(normal); }
    void setSize(const glm::vec2& size) { m_size = size; }

    // Дополнительные методы
    virtual void setContrast(float contrast) = 0;
    virtual void setColorMapping(int r, int g, int b) = 0;

    // Для вычисления пересечений с другими плоскостями
	virtual bool intersectsWith(const AbstractPlane& other, glm::vec3& linePoint, glm::vec3& lineDir) const;
    	virtual float calculateDistanceToCamera(const glm::vec3& cameraPos) const;
};
#endif
