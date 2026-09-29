//---------------------------------------------------------------------------

#pragma hdrstop

#include "SeismicPlane.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#include "SeismicPlane.h"
#include <iostream>

SeismicPlane::SeismicPlane(const glm::vec3& anchor, const glm::vec3& normal,
						 const glm::vec2& size, GLuint shaderProgram, std::function<std::shared_ptr<SeismicData>()> ret, bool flipV, bool flipH)
	: AbstractPlane(anchor, normal, size, shaderProgram, flipV, flipH),
	  m_paletteTexture(0), m_contrast(1.0f), ret_func(ret) {
    setupShader();
}

SeismicPlane::~SeismicPlane() {
    if (m_paletteTexture) glDeleteTextures(1, &m_paletteTexture);
}

void SeismicPlane::setupShader() {
    // Шейдер будет установлен при рендеринге
}

void SeismicPlane::initPaletteTexture(const std::array<uint8_t, 256*3>& paletteData) {
    glGenTextures(1, &m_paletteTexture);
    glBindTexture(GL_TEXTURE_1D, m_paletteTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, paletteData.data());
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

void SeismicPlane::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_shaderProgram) return;

    glUseProgram(m_shaderProgram);

    // Передаем матрицы (аналог 2D uniform'ов)
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

    // Вычисляем и передаем матрицу модели
    glm::mat4 model = glm::mat4(1.0f);

    // Находим оси в плоскости (аналог pixelRatio в 2D)
    glm::vec3 arbitrary = glm::vec3(0.0f, 1.0f, 0.0f);
    if (std::abs(glm::dot(m_normal, arbitrary)) > 0.9f) {
        arbitrary = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 right = glm::normalize(glm::cross(m_normal, arbitrary));
    glm::vec3 up = glm::normalize(glm::cross(right, m_normal));

    // Строим матрицу модели (аналог transform в 2D)
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(m_normal, 0.0f);

    model = glm::translate(model, m_anchor);
    model = model * rotation;
    model = glm::scale(model, glm::vec3(m_size.x, m_size.y, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

    // Передаем текстуры (аналог 2D текстур)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "indexTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, m_paletteTexture);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "paletteTexture"), 1);

    // Передаем параметры (аналоги 2D uniform'ов)
	glUniform1f(glGetUniformLocation(m_shaderProgram, "contrast"), m_contrast);
	glUniform1f(glGetUniformLocation(m_shaderProgram, "transparency"), m_transparency);

    // Рендерим
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}