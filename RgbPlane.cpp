//---------------------------------------------------------------------------

#pragma hdrstop

#include "RgbPlane.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#include "RgbPlane.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

RgbPlane::RgbPlane(const glm::vec3& anchor, const glm::vec3& normal,
				 const glm::vec2& size, GLuint shaderProgram, std::function<std::shared_ptr<RgbData>()> ret, bool flipV, bool flipH)
	: AbstractPlane(anchor, normal, size, shaderProgram, flipV, flipH),
      m_textureArray(0),
      m_redChannel(0), m_greenChannel(1), m_blueChannel(2),
      m_redEnabled(true), m_greenEnabled(true), m_blueEnabled(true),
      m_inverse(false), m_contrast(1.0f), m_layerCount(0), ret_func(ret) {
    setupShader();
}

RgbPlane::~RgbPlane() {
    if (m_textureArray) {
        glDeleteTextures(1, &m_textureArray);
    }
}

void RgbPlane::setupShader() {
    // Шейдер будет установлен при рендеринге
}

void RgbPlane::calculateModelMatrix(glm::mat4& model) const {
    model = glm::mat4(1.0f);

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

    // Применяем трансформации
    model = glm::translate(glm::mat4(1.0f), m_anchor) * rotation * glm::scale(glm::mat4(1.0f), glm::vec3(m_size.x, m_size.y, 1.0f));
}

void RgbPlane::render(const glm::mat4& view, const glm::mat4& projection) {
	if (!m_shaderProgram || !m_visible) return;

    glUseProgram(m_shaderProgram);

    // Вычисляем и передаем матрицу модели
    glm::mat4 model;
    calculateModelMatrix(model);

    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Привязываем текстуру массива
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "textureArray"), 0);

    // Передаем настройки каналов
    glUniform1i(glGetUniformLocation(m_shaderProgram, "R"), m_redChannel);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "G"), m_greenChannel);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "B"), m_blueChannel);

    glUniform1i(glGetUniformLocation(m_shaderProgram, "isR"), m_redEnabled);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "isG"), m_greenEnabled);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "isB"), m_blueEnabled);

    glUniform1i(glGetUniformLocation(m_shaderProgram, "inverse"), m_inverse);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "layerCount"), m_layerCount);

	glUniform1f(glGetUniformLocation(m_shaderProgram, "contrast"), m_contrast);
	glUniform1f(glGetUniformLocation(m_shaderProgram, "transparency"), m_transparency);

    // Рендерим плоскость
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//glDisable(GL_BLEND);
}

void RgbPlane::initTextureArray(const std::vector<bitMap>& textureData) {
	if (textureData.empty()) return;

	m_layerCount = textureData.size();

    // Определяем размеры текстур (предполагаем, что все одинакового размера)
	m_size.x = textureData[0].width;
	m_size.y = textureData[0].height;

    // Создаем текстуру массива
    glGenTextures(1, &m_textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

    // Выделяем память для текстуры массива
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, m_size.x, m_size.y, m_layerCount, 0,
                 GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // Загружаем данные для каждого слоя
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < m_layerCount; ++i) {
		if (!textureData[i].texture.empty()) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                            0, 0, i,                    // x, y, z offset
							m_size.x, m_size.y, 1,           // width, height, depth
                            GL_RED, GL_UNSIGNED_BYTE,
							textureData[i].texture.data());
        }
    }

    // Настраиваем параметры текстуры
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Проверка ошибок
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in RgbPlane::initTextureArray: " << err << std::endl;
    }
}

void RgbPlane::updateTextureLayer(int layer, const std::vector<uint8_t>& data) {
    if (m_textureArray == 0 || layer < 0 || layer >= m_layerCount || data.empty()) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, layer,
                    m_size.x, m_size.y, 1,
                    GL_RED, GL_UNSIGNED_BYTE,
                    data.data());
}

void RgbPlane::setChannels(int red, int green, int blue) {
    m_redChannel = std::max(0, std::min(red, m_layerCount - 1));
    m_greenChannel = std::max(0, std::min(green, m_layerCount - 1));
    m_blueChannel = std::max(0, std::min(blue, m_layerCount - 1));
}

void RgbPlane::setChannelEnabled(bool red, bool green, bool blue) {
    m_redEnabled = red;
    m_greenEnabled = green;
    m_blueEnabled = blue;
}

void RgbPlane::setInverse(bool inverse) {
    m_inverse = inverse;
}
