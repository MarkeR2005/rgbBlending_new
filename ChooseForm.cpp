#include <vcl.h>
#pragma hdrstop

#include "ChooseForm.h"
#include <System.IOUtils.hpp>
#include <VirtualTrees.hpp>
#include <cstdio>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VirtualTrees"
#pragma link "VirtualTrees.AncestorVCL"
#pragma link "VirtualTrees.BaseAncestorVCL"
#pragma link "VirtualTrees.BaseTree"
#include "UnitSimpleDialog.h"
#pragma resource "*.dfm"

#define Read(x,y) Read(x, (Longint)(y))
TChoose *Choose;
//---------------------------------------------------------------------------
__fastcall TChoose::TChoose(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TChoose::LoadRGBFiles(const UnicodeString& path)
{
    if (!DirectoryExists(path)) {
//        ShowMessage(L"Папка не найдена: " + path);
		CreateDirectory(path.c_str(), NULL);
        return;
    }

    VST->BeginUpdate();
    try {
        VST->Clear();
        // Устанавливаем размер пользовательских данных для каждого узла
        VST->NodeDataSize = sizeof(TRGBFileData);

        TStringDynArray files = TDirectory::GetFiles(path, L"*.*rgb*", TSearchOption::soAllDirectories);

        for (int i = 0; i < files.Length; ++i) {
            PVirtualNode node = VST->AddChild(NULL);
            // Получаем указатель на память под данные
            TRGBFileData* data = (TRGBFileData*)VST->GetNodeData(node);
            data->FullPath = files[i];
            data->FileName = ExtractFileName(files[i]);
        }

        VST->FullExpand();
    }
    __finally {
        VST->EndUpdate();
    }
}
void TChoose::SetRGBPath(const UnicodeString& path)
{
    curPath = path;
    LoadRGBFiles(path);
}



void __fastcall TChoose::VSTClick(TObject *Sender)
{
    TVirtualStringTree* tree = dynamic_cast<TVirtualStringTree*>(Sender);
    if (!tree) return;

    TVirtualNode* node = tree->GetFirstSelected();
    if (node) {
        // ИСПРАВЛЕНО: Приводим void* к нашему типу данных
        TRGBFileData* data = (TRGBFileData*)tree->GetNodeData(node);
        if (data) {
            chosen = data->FullPath;
            ShowFileDescription(data->FullPath);
        }
    }
}

void TChoose::ShowFileDescription(const UnicodeString& filePath)
{
    if (!TFile::Exists(filePath)) {
        ShowMessage(L"Файл не найден: " + filePath);
        return;
    }

    UnicodeString desc = L"=== Описание RGB-файла ===\n";
    desc += L"Имя: " + ExtractFileName(filePath) + L"\n";
    desc += L"Путь: " + filePath + L"\n";
    desc += L"Размер: " + IntToStr(TFile::GetSize(filePath)) + L" байт\n";
    desc += L"Изменён: " + DateTimeToStr(TFile::GetLastWriteTime(filePath)) + L"\n";

    std::unique_ptr<TFileStream> stream(new TFileStream (filePath.c_str(), fmOpenRead));
    try {

        float dT;
        int samples;
        int traces;
        int filters;
        float freqStart;
        float freqFinish;
        std::string name;
        std::string procedures;

        // Читаем параметры
        stream->Read(&dT, sizeof(dT));
        stream->Read(&samples, sizeof(samples));
        stream->Read(&traces, sizeof(traces));
        stream->Read(&filters, sizeof(filters));


        // Читаем frequencies
        stream->Read(&freqStart, sizeof(freqStart));
        stream->Read(&freqFinish, sizeof(freqFinish));

        // Читаем строки
        int len;
        stream->Read(&len, sizeof(len));
        name.resize(len);
        stream->Read(&name[0], len);

        stream->Read(&len, sizeof(len));
        procedures.resize(len);
        stream->Read(&procedures[0], len);

        desc += L"dt = " + FloatToStr(dT) + L"\n";
        desc += L"отсчетов " + IntToStr(samples) + L"\n";
        desc += L"трасс = " + IntToStr(traces) + L"\n";
        desc += L"фильтров = " + IntToStr(filters) + L"\n";
        desc += L"f нач = " + FloatToStr(freqStart) + L"\n";
        desc += L"f кон = " + FloatToStr(freqFinish) + L"\n";
        desc += L"Преобразования " + UnicodeString(procedures.c_str())	 + L"\n";

    } catch (const Exception& e) {
        desc += L"\nНе удалось прочитать содержимое: " + e.Message;
    }

    TSimpleDialog* dialog = new TSimpleDialog(nullptr);
    dialog->setDescription(desc);
    if (dialog->Execute()) {
        execType = 3;
    	ModalResult = mrOk;
    }
    else if (dialog->shouldDelete)
    {
        if(MessageDlg("Вы действительно хотите удалить этот файл?",
                   mtConfirmation,          // Тип: вопрос, предупреждение, ошибка...
                   TMsgDlgButtons() << mbYes << mbNo, // Кнопки: Да и Нет
                   0) == mrYes) {
    	DeleteFile(filePath);
        LoadRGBFiles(curPath);
                   }
    }
    delete dialog;
}
void __fastcall TChoose::VSTGetText(TBaseVirtualTree *Sender, PVirtualNode Node, TColumnIndex Column,
          TVstTextType TextType, UnicodeString &CellText)
{
    TRGBFileData* data = (TRGBFileData*)Sender->GetNodeData(Node);
    if (!data) {
        CellText = L"";
        return;
    }

    // Column — это индекс колонки (0, 1, 2...)
    if (Column == 0) {
        CellText = data->FileName;
    } else if (Column == 1) {
        CellText = data->FullPath;
    } else {
        CellText = L"";
    }
}
//---------------------------------------------------------------------------

void __fastcall TChoose::VSTFreeNode(TBaseVirtualTree *Sender, PVirtualNode Node)

{
	TRGBFileData* data = (TRGBFileData*)Sender->GetNodeData(Node);
    if (data) {
        // Вызов деструктора для структуры, чтобы корректно освободить UnicodeString
        data->~TRGBFileData();
    }
}
//---------------------------------------------------------------------------

void __fastcall TChoose::CloseClick(TObject *Sender)
{
    execType = 0;
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
bool TChoose::Execute()
{
    execType = 0;
    return (ShowModal() == mrOk);
}
void __fastcall TChoose::ViewSeisClick(TObject *Sender)
{
    execType = 1;
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TChoose::NewClick(TObject *Sender)
{
	execType = 2;
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------

