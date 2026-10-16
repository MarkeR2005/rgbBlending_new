#include <vcl.h>
#pragma hdrstop
#include "UnitFormVolume.h"
#include <algorithm>

#pragma package(smart_init)
#pragma resource "*.dfm"

__fastcall TFormVolume::TFormVolume(TComponent* Owner, System::UnicodeString basePath)
    : TForm(Owner)
{
    Caption = basePath;
    Width = 1100;
    Height = 750;
    Position = poScreenCenter;
    OnClose = FormClose;

    channelsPanel = new TPanel(this);
    channelsPanel->Parent = this;
    channelsPanel->Align = alTop;
    channelsPanel->Height = 92;
    channelsPanel->Visible = false;
    const wchar_t* names[] = {L"R", L"G", L"B"};
    for (int i = 0; i < 3; ++i) {
        enabled[i] = new TCheckBox(this);
        enabled[i]->Parent = channelsPanel;
        enabled[i]->SetBounds(10, 5 + i * 28, 35, 23);
        enabled[i]->Caption = names[i];
        enabled[i]->Checked = true;
        enabled[i]->OnClick = ChannelsChanged;

        channels[i] = new TScrollBar(this);
        channels[i]->Parent = channelsPanel;
        channels[i]->SetBounds(50, 5 + i * 28, 235, 23);
        channels[i]->OnChange = ChannelsChanged;

        frequencies[i] = new TLabel(this);
        frequencies[i]->Parent = channelsPanel;
        frequencies[i]->SetBounds(295, 9 + i * 28, 100, 20);
    }

    container = new TWindowContainer(this);
    container->setBasePath(basePath);
    container->Parent = this;
    container->Align = alClient;
}

void TFormVolume::initForCube(System::UnicodeString filename)
{
    container->initializeVolume();
    container->setFile(filename.c_str());
    const int count = container->getFreqSize();
    if (count < 0) throw "Bad cube data";
    channelsPanel->Visible = count > 0;
    if (count > 0) {
        for (auto* channel : channels)
            channel->Max = std::max(count - 1, 0);
        ChannelsChanged(nullptr);
    }
}

void __fastcall TFormVolume::ChannelsChanged(TObject*)
{
    for (int i = 0; i < 3; ++i)
        frequencies[i]->Caption = FloatToStrF(
            container->getFreqByIndex(channels[i]->Position), ffFixed, 4, 2) + L" Hz";
    const viewParams params = {
        enabled[0]->Checked, enabled[1]->Checked, enabled[2]->Checked,
        channels[0]->Position, channels[1]->Position, channels[2]->Position, 1.0f
    };
    container->setViewParams(params);
}

void __fastcall TFormVolume::FormClose(TObject*, TCloseAction& Action)
{
    Action = caFree;
}
