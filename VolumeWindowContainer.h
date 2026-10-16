#ifndef VolumeWindowContainerH
#define VolumeWindowContainerH

#include <vcl.h>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ColorManager.h"
#include "ViewportWindow.h"
#include "ViewParams.h"

// Owns only the 3D viewport and its plane actions. Section overlays and axes
// live in TSectionWindowContainer.
class TVolumeWindowContainer : public TPanel {
private:
    enum class PlaneDirection { Slice, Inline, Crossline };
    TPanel* viewPanel = nullptr;
    TPopupMenu* actions = nullptr;
    std::unique_ptr<ViewportWindow> window;
    std::shared_ptr<ColorManager> colorManager = std::make_shared<ColorManager>();
    std::map<TObject*, std::function<void()>> functions;
    std::wstring fileName;
    System::UnicodeString basePath;
    std::vector<float> frequencies;

    void __fastcall ResizeViewport(TObject* Sender);
    void __fastcall UseFunction(TObject* Sender);
    void addButton(const char* caption, std::function<void()> function);
    void setupButtons();
    void askForPlane(PlaneDirection direction);
    void addPlane(PlaneDirection direction, int index);
    void createSection(const std::shared_ptr<IBaseData>& data);
public:
    __fastcall TVolumeWindowContainer(TComponent* Owner);
    void initializeVolume();
    void setBasePath(System::UnicodeString value) {basePath = value;}
    void setFile(const std::wstring& value);
    int getFreqSize() const {return static_cast<int>(frequencies.size());}
    float getFreqByIndex(int idx) const;
    void setViewParams(viewParams params);
};

#endif
