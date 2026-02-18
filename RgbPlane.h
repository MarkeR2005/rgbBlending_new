//---------------------------------------------------------------------------

#ifndef RgbPlaneH
#define RgbPlaneH
//---------------------------------------------------------------------------
#pragma once
#include "AbstractPlane.h"
#include <vector>
#include <array>

class RgbPlane : public AbstractPlane {
private:
    // Текстура массива для RGB каналов
    GLuint m_textureArray;

    // Настройки цветов
    int m_redChannel;
    int m_greenChannel;
    int m_blueChannel;

    // Включенные каналы
    bool m_redEnabled;
    bool m_greenEnabled;
    bool m_blueEnabled;

    // Инверсия цветов
    bool m_inverse;

    // Контраст
    float m_contrast;

    // Количество слоев в текстуре
    int m_layerCount;

    void setupShader() override;

public:
    RgbPlane(const glm::vec3& anchor, const glm::vec3& normal,
             const glm::vec2& size, GLuint shaderProgram, bool flipV = false, bool flipH = false);
    ~RgbPlane();

    void render(const glm::mat4& view, const glm::mat4& projection) override;

    // Управление текстурами
	void initTextureArray(const std::vector<bitMap>& textureData);
    void updateTextureLayer(int layer, const std::vector<uint8_t>& data);

    // Управление каналами
    void setChannels(int red, int green, int blue);
    void setChannelEnabled(bool red, bool green, bool blue);
    void setInverse(bool inverse);
    void setContrast(float contrast) override { m_contrast = contrast; }

    // Геттеры
    int getRedChannel() const { return m_redChannel; }
    int getGreenChannel() const { return m_greenChannel; }
    int getBlueChannel() const { return m_blueChannel; }
    bool isRedEnabled() const { return m_redEnabled; }
    bool isGreenEnabled() const { return m_greenEnabled; }
    bool isBlueEnabled() const { return m_blueEnabled; }
    bool isInverse() const { return m_inverse; }
    float getContrast() const { return m_contrast; }

    // Заглушка для интерфейса (не используется для RGB)
    void setColorMapping(int r, int g, int b) override {}

private:
    void calculateModelMatrix(glm::mat4& model) const;
};
#endif
