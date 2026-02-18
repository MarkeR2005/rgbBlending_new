//---------------------------------------------------------------------------

#ifndef SeismicPlaneH
#define SeismicPlaneH
//---------------------------------------------------------------------------
#pragma once
#include "AbstractPlane.h"
#include <array>

class SeismicPlane : public AbstractPlane {
private:
    GLuint m_paletteTexture;
    float m_contrast;

	void setupShader() override;

public:
    SeismicPlane(const glm::vec3& anchor, const glm::vec3& normal,
                 const glm::vec2& size, GLuint shaderProgram, bool flipV = false, bool flipH = false);
    ~SeismicPlane();

    void render(const glm::mat4& view, const glm::mat4& projection) override;
    void setContrast(float contrast) override { m_contrast = contrast; }
    void setColorMapping(int r, int g, int b) override {} // Не используется для сейсмики

	void initPaletteTexture(const std::array<uint8_t, 256*3>& paletteData);
};
#endif
