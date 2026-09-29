//---------------------------------------------------------------------------

#ifndef SeismicPlaneH
#define SeismicPlaneH
//---------------------------------------------------------------------------
#pragma once
#include "AbstractPlane.h"
#include <array>
#include "SeismicData.h"

class SeismicPlane : public AbstractPlane {
private:
    GLuint m_paletteTexture;
    float m_contrast;

	void setupShader() override;
	std::function<std::shared_ptr<SeismicData>()> ret_func = [](){return std::make_shared<SeismicData>();};
public:
    SeismicPlane(const glm::vec3& anchor, const glm::vec3& normal,
                 const glm::vec2& size, GLuint shaderProgram, std::function<std::shared_ptr<SeismicData>()> ret, bool flipV = false, bool flipH = false);
    ~SeismicPlane();
	std::shared_ptr<IBaseData> getData() override {
        return ret_func();
	}
    void render(const glm::mat4& view, const glm::mat4& projection) override;
    void setContrast(float contrast) override { m_contrast = contrast; }
    void setColorMapping(int r, int g, int b) override {} // Не используется для сейсмики

	void initPaletteTexture(const std::array<uint8_t, 256*3>& paletteData);
};
#endif
