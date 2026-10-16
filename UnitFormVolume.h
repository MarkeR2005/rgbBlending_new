#ifndef UnitFormVolumeH
#define UnitFormVolumeH

#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.StdCtrls.hpp>
#include "VolumeWindowContainer.h"

// Cube controls and lifetime are independent of the 2D section form.
class TFormVolume : public TForm {
private:
    TPanel* channelsPanel = nullptr;
    TScrollBar* channels[3] = {};
    TCheckBox* enabled[3] = {};
    TLabel* frequencies[3] = {};
    TVolumeWindowContainer* container = nullptr;
    void __fastcall ChannelsChanged(TObject* Sender);
    void __fastcall FormClose(TObject* Sender, TCloseAction& Action);
public:
    __fastcall TFormVolume(TComponent* Owner, System::UnicodeString basePath);
    void initForCube(System::UnicodeString filename);
};

#endif
