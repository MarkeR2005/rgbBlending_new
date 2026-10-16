#include <vcl.h>
#pragma hdrstop
#include "VolumeWindowContainer.h"
#include "UnitFormSection2D.h"
#include "SeismicPlane.h"
#include "RgbPlane.h"
#include "Reader.h"
#include "SeismicData.h"
#include "RgbData.h"
#include "Shaders.h"
#include "Dialog.h"

#pragma package(smart_init)

__fastcall TVolumeWindowContainer::TVolumeWindowContainer(TComponent* Owner)
    : TPanel(Owner)
{
    BevelOuter = bvNone;
    DoubleBuffered = true;
    OnResize = ResizeViewport;
    viewPanel = new TPanel(this);
    viewPanel->Parent = this;
    viewPanel->Align = alClient;
    actions = new TPopupMenu(this);
    PopupMenu = actions;
}

void TVolumeWindowContainer::initializeVolume()
{
    window = std::make_unique<ViewportWindow>();
    window->initWindow(viewPanel);
    window->setCamera(glm::vec3(1000.0f, 0.0f, 1000.0f),
        glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    window->setPerspective(45.0f, 0.1f, 10000.0f);
    window->setMouseButtonCallback([this](int button, int action, int) {
        if (button != GLFW_MOUSE_BUTTON_RIGHT || action != GLFW_PRESS) return;
        POINT point;
        GetCursorPos(&point);
        actions->Popup(point.x, point.y);
    });
    setupButtons();
    ResizeViewport(this);
}

void TVolumeWindowContainer::setFile(const std::wstring& value)
{
    fileName = value;
    frequencies.clear();
    if (ExtractFileExt(fileName.c_str()) != L".ibm")
        readSizeRGB(fileName, frequencies);
}

float TVolumeWindowContainer::getFreqByIndex(int idx) const
{
    return idx >= 0 && idx < static_cast<int>(frequencies.size())
        ? frequencies[idx] : -1.0f;
}

void TVolumeWindowContainer::setViewParams(viewParams params)
{
    if (!window) return;
    window->setChannels(params.r, params.g, params.b);
    window->setChannelEnabled(params.isR, params.isG, params.isB);
    window->renderWindow();
}

void __fastcall TVolumeWindowContainer::ResizeViewport(TObject*)
{
    if (window && viewPanel->Width > 0 && viewPanel->Height > 0)
        window->resizeWindow(viewPanel->Width, viewPanel->Height);
}

void TVolumeWindowContainer::addButton(const char* caption, std::function<void()> function)
{
    auto* item = new TMenuItem(actions);
    item->Caption = caption;
    item->OnClick = UseFunction;
    actions->Items->Add(item);
    functions[item] = std::move(function);
}

void __fastcall TVolumeWindowContainer::UseFunction(TObject* Sender)
{
    auto found = functions.find(Sender);
    if (found != functions.end()) found->second();
}

void TVolumeWindowContainer::createSection(const std::shared_ptr<IBaseData>& data)
{
    if (!data) return;
    std::unique_ptr<TFormSection2D> form(new TFormSection2D(Application->MainForm, basePath));
    form->initFromData(data);
    form->Show();
    form.release();
}

void TVolumeWindowContainer::askForPlane(PlaneDirection direction)
{
    int index = 0;
    std::unique_ptr<TAbstractDialog> dialog(new TAbstractDialog(nullptr));
    dialog->AddInput<int>("Plane index", index);
    if (dialog->Execute() && dialog->ContinuePressed)
        addPlane(direction, index);
}

void TVolumeWindowContainer::addPlane(PlaneDirection direction, int index)
{
    if (!window || fileName.empty()) return;
    const bool seismic = ExtractFileExt(fileName.c_str()) == L".ibm";
    int samples = 0, traces = 0, lines = 0;
    if (seismic) {
        const size dimensions = readSize(fileName);
        samples = dimensions.samples;
        traces = dimensions.traces;
        lines = dimensions.lines;
    } else {
        std::vector<float> frequencyValues;
        const size4 dimensions = readSizeRGB(fileName, frequencyValues);
        samples = dimensions.samples;
        traces = dimensions.traces;
        lines = dimensions.lines;
    }

    glm::vec3 normal(0.0f, 0.0f, 1.0f);
    glm::vec3 position(0.0f);
    glm::vec2 planeSize(1.0f);
    if (direction == PlaneDirection::Slice) {
        normal = glm::vec3(0.0f, 1.0f, 0.0f);
        position.y = static_cast<float>(samples)/2 - index;
        planeSize = glm::vec2(lines, traces);
    } else if (direction == PlaneDirection::Inline) {
        position.z = static_cast<float>(lines)/2 - index;
        planeSize = glm::vec2(traces, samples);
    } else {
        normal = glm::vec3(1.0f, 0.0f, 0.0f);
        position.x = static_cast<float>(traces)/2 - index;
        planeSize = glm::vec2(lines, samples);
    }
    const bool flipVertical = true;
    const bool flipHorizontal = direction == PlaneDirection::Crossline;
    const std::wstring sourceFile = fileName;

    if (seismic) {
        auto readPlane = [sourceFile, index, direction]() {
            if (direction == PlaneDirection::Slice) {
                Traces traces = readTimeSliceRegular(
                    ChangeFileExt(sourceFile.c_str(), L".sbm").c_str(), index);
                return std::make_shared<SeismicData>(traces.data, traces.dt,
                    traces.samplesNumber, traces.tracesNumber);
            }
            if (direction == PlaneDirection::Inline) {
                Traces traces = readInlineRegular(sourceFile, index);
                return std::make_shared<SeismicData>(traces.data, traces.dt,
                    traces.samplesNumber, traces.tracesNumber);
            }
            Traces traces = readXlineRegular(sourceFile, index);
            return std::make_shared<SeismicData>(traces.data, traces.dt,
                traces.samplesNumber, traces.tracesNumber);
        };
        auto data = readPlane();
        if (!data->getCM()) data->setCM(colorManager);
        auto plane = std::make_shared<SeismicPlane>(position, normal, planeSize,
            createShaderProgram("volumetric", "seismic"), readPlane,
            flipVertical, flipHorizontal);
        plane->updateTexture(data->getTexture());
        plane->initPaletteTexture(PALETTE);
        window->addPlane(plane);
    } else {
        auto readPlane = [sourceFile, index, direction]() {
            if (direction == PlaneDirection::Slice)
                return readTimeSliceRGB(
                    ChangeFileExt(sourceFile.c_str(), L".srgb").c_str(), index);
            if (direction == PlaneDirection::Inline)
                return readInlineRGB(sourceFile, index);
            return readCrosslineRGB(sourceFile, index);
        };
        auto data = readPlane();
        if (!data->getCM()) data->setCM(colorManager);
        auto plane = std::make_shared<RgbPlane>(position, normal, planeSize,
            createShaderProgram("volumetric", "rgb"), readPlane,
            flipVertical, flipHorizontal);
        plane->initTextureArray(data->getTexture());
        window->addPlane(plane);
    }
}

void TVolumeWindowContainer::setupButtons()
{
    addButton("Add slice", [this] {askForPlane(PlaneDirection::Slice);});
    addButton("Add inline", [this] {askForPlane(PlaneDirection::Inline);});
    addButton("Add crossline", [this] {askForPlane(PlaneDirection::Crossline);});
    addButton("Delete panel", [this] {
        auto selected = window->getSelectedPlane();
        if (!selected) return;
        window->removePlane(selected);
        window->renderWindow();
    });
    addButton("Show panel", [this] {
        auto selected = window->getSelectedPlane();
        if (selected) createSection(selected->getData());
    });
    addButton("Set Transparency", [this] {
        auto selected = window->getSelectedPlane();
        if (!selected) return;
        float percentage = 0.0f;
        std::unique_ptr<TAbstractDialog> dialog(new TAbstractDialog(nullptr));
        dialog->AddInput<float>("Transparency (%)", percentage);
        if (dialog->Execute() && dialog->ContinuePressed) {
            selected->setTransparency(percentage/100.0f);
            window->renderWindow();
        }
    });
}
