#pragma hdrstop
#include "RgbWindow.h"
#include "RgbData.h"
#include "Shaders.h"
#include <vcl.h>
#include <GL/glew.h>
#include <array>
#include <map>
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
    setShaderProgram(createShaderProgram("universal", "rgb_composite"));
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
    glfwMakeContextCurrent(handle);
    if (!indexTexture) glGenTextures(1, &indexTexture);
    glBindTexture(GL_TEXTURE_2D, indexTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    updateComposite();
}
void RgbWindow::updateComposite() {
    if (!textureDirty || !source || !handle) return;
    glfwMakeContextCurrent(handle);
    const int maxIndex = source->getSize().f - 1;
    const std::array<int, 3> channels = {{R, G, B}};
    std::map<int, bitMap> selected;
    for (int index : channels) {
        if (index < 0 || index > maxIndex) throw Exception("RGB channel out of range");
        if (selected.find(index) == selected.end()) selected.emplace(index, source->getTexture(index));
    }
    const bitMap& first = selected.at(R);
    width = first.width;
    height = first.height;
    if (width <= 0 || height <= 0) throw Exception("Empty RGB layer");
    const size_t pixelCount = static_cast<size_t>(width) * height;
    std::vector<uint8_t> composite(pixelCount * 3);
    for (size_t i = 0; i < pixelCount; ++i) {
        for (int c = 0; c < 3; ++c) {
            const auto& layer = selected.at(channels[c]).texture;
            if (layer.size() != pixelCount) throw Exception("Invalid RGB layer size");
            composite[3*i+c] = layer[i];
        }
    }
    glBindTexture(GL_TEXTURE_2D, indexTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (allocatedWidth != width || allocatedHeight != height) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, composite.data());
        allocatedWidth = width;
        allocatedHeight = height;
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGB, GL_UNSIGNED_BYTE, composite.data());
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
