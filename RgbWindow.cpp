#pragma hdrstop
#include "RgbWindow.h"
#include "RgbData.h"
#include "Shaders.h"
#include <vcl.h>
#include <GL/glew.h>
#include <array>
#include <map>
#include <list>
#include <algorithm>
#pragma package(smart_init)

RgbWindow::RgbWindow(): FlatWindow() {}
RgbWindow::~RgbWindow() {
    if (handle && indexTexture) {
        glfwMakeContextCurrent(handle);
        glDeleteTextures(1, &indexTexture);
    }
}
void RgbWindow::initWindow(TPanel* parent) {
    FlatWindow::initWindow(parent);
    setShaderProgram(create2DRgbShaderProgram());
}
void RgbWindow::initData(std::shared_ptr<RgbData> data) {
    source = std::move(data);
    if (!source || source->getSize().f <= 0) throw Exception("Empty RGB data");
    const auto dimensions = source->getSize();
    width = dimensions.x;
    height = dimensions.t;
    R = 0;
    G = 0;
    B = 0;
    textureDirty = true;
    layerCache.clear();
    cachedBytes = 0;
    compositePixels.clear();
    glfwMakeContextCurrent(handle);
    if (!indexTexture) glGenTextures(1, &indexTexture);
    glBindTexture(GL_TEXTURE_2D, indexTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    updateComposite();
}
bool RgbWindow::getPixelComponents(int x, int y, int& r, int& g, int& b) const {
    if (x < 0 || y < 0 || x >= width || y >= height ||
        compositePixels.size() != static_cast<size_t>(width)*height*3) return false;
    const size_t index = (static_cast<size_t>(y)*width+x)*3;
    r = compositePixels[index];
    g = compositePixels[index+1];
    b = compositePixels[index+2];
    return true;
}
bool RgbWindow::hasCachedLayers(int r, int g, int b) const {
    const std::array<int, 3> indices = {{r, g, b}};
    for (int index : indices) {
        bool found = false;
        for (const auto& cached : layerCache)
            if (cached.index == index) {found = true; break;}
        if (!found) return false;
    }
    return true;
}
std::shared_ptr<bitMap> RgbWindow::loadLayer(int index) {
    for (auto it = layerCache.begin(); it != layerCache.end(); ++it) {
        if (it->index == index) {
            auto layer = it->bitmap;
            layerCache.splice(layerCache.begin(), layerCache, it);
            return layer;
        }
    }
    auto layer = std::make_shared<bitMap>(source->getTexture(index));
    const size_t bytes = layer->texture.size();
    const size_t budget = 128u * 1024u * 1024u;
    if (bytes <= budget) {
        layerCache.push_front({index, layer});
        cachedBytes += bytes;
        while (layerCache.size() > 8 || cachedBytes > budget) {
            cachedBytes -= layerCache.back().bitmap->texture.size();
            layerCache.pop_back();
        }
    }
    return layer;
}
void RgbWindow::updateComposite() {
    if (!textureDirty || !source || !handle) return;
    glfwMakeContextCurrent(handle);
    const int maxIndex = source->getSize().f - 1;
    const std::array<int, 3> channels = {{R, G, B}};
    std::array<std::shared_ptr<bitMap>, 3> selected;
    for (int c = 0; c < 3; ++c) {
        const int index = channels[c];
        if (index < 0 || index > maxIndex) throw Exception("RGB channel out of range");
        // Avoid converting the same frequency twice if two channels coincide.
        int same = 0;
        while (same < c && channels[same] != index) ++same;
        selected[c] = same < c ? selected[same] : loadLayer(index);
    }
    width = selected[0]->width;
    height = selected[0]->height;
    if (width <= 0 || height <= 0) throw Exception("Empty RGB layer");
    const size_t pixelCount = static_cast<size_t>(width) * height;
    const auto& red = selected[0]->texture;
    const auto& green = selected[1]->texture;
    const auto& blue = selected[2]->texture;
    if (red.size() != pixelCount || green.size() != pixelCount || blue.size() != pixelCount)
        throw Exception("Invalid RGB layer size");
    compositePixels.resize(pixelCount * 3);
    uint8_t* dst = compositePixels.data();
    const uint8_t* r = red.data();
    const uint8_t* g = green.data();
    const uint8_t* b = blue.data();
    for (size_t i = 0; i < pixelCount; ++i) {
        dst[3*i] = r[i];
        dst[3*i+1] = g[i];
        dst[3*i+2] = b[i];
    }
    glBindTexture(GL_TEXTURE_2D, indexTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (allocatedWidth != width || allocatedHeight != height) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, compositePixels.data());
        allocatedWidth = width;
        allocatedHeight = height;
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGB, GL_UNSIGNED_BYTE, compositePixels.data());
    }
    if (glGetError() != GL_NO_ERROR) throw Exception("RGB texture upload failed");
    textureDirty = false;
}
void RgbWindow::renderForScreenshot() {
    if (!handle) return;
    updateComposite();
    preRender();
    render();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, indexTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "composite"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "isR"), isR);
    glUniform1i(glGetUniformLocation(shaderProgram, "isG"), isG);
    glUniform1i(glGetUniformLocation(shaderProgram, "isB"), isB);
    glUniform1i(glGetUniformLocation(shaderProgram, "inverse"), inverse);
    postRender();
    glFlush();
}
void RgbWindow::renderWindow() {
    if (!handle) return;
    renderForScreenshot();
    glfwSwapBuffers(handle);
}
void RgbWindow::setColor(int r, int g, int b) {
    if (R != r || G != g || B != b) {
        R = r; G = g; B = b;
        textureDirty = true;
    }
}
void RgbWindow::setView(bool r, bool g, bool b) {
    isR = r; isG = g; isB = b;
}
