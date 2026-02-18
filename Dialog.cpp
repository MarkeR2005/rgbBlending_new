#include "Dialog.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

// Специализации для разных типов данных

// int
template<>
void TTypedInputElement<int>::ApplyValue()
{
    try {
        *FVariable = StrToInt(FEdit->Text);
    }
    catch (...) {
        FEdit->Text = IntToStr(*FVariable);
    }
}

template<>
void TTypedInputElement<int>::UpdateDisplay()
{
    FEdit->Text = IntToStr(*FVariable);
}

// double
template<>
void TTypedInputElement<double>::ApplyValue()
{
    try {
        *FVariable = StrToFloat(FEdit->Text);
    }
    catch (...) {
        FEdit->Text = FloatToStr(*FVariable);
    }
}

template<>
void TTypedInputElement<double>::UpdateDisplay()
{
    FEdit->Text = FloatToStr(*FVariable);
}

// float
template<>
void TTypedInputElement<float>::ApplyValue()
{
    try {
        *FVariable = StrToFloat(FEdit->Text);
    }
    catch (...) {
        FEdit->Text = FloatToStr(*FVariable);
    }
}

template<>
void TTypedInputElement<float>::UpdateDisplay()
{
    FEdit->Text = FloatToStr(*FVariable);
}

// std::string
template<>
void TTypedInputElement<std::string>::ApplyValue()
{
    *FVariable = AnsiString(FEdit->Text).c_str();
}

template<>
void TTypedInputElement<std::string>::UpdateDisplay()
{
    FEdit->Text = FVariable->c_str();
}

// AnsiString
template<>
void TTypedInputElement<AnsiString>::ApplyValue()
{
    *FVariable = FEdit->Text;
}

template<>
void TTypedInputElement<AnsiString>::UpdateDisplay()
{
    FEdit->Text = *FVariable;
}

// Реализация TAbstractDialog

__fastcall TAbstractDialog::TAbstractDialog(TComponent* Owner)
    : TForm(Owner), FNextTop(10), FContinuePressed(false)
{
}

__fastcall TAbstractDialog::~TAbstractDialog()
{
    for (auto element : FInputElements) {
        delete element;
    }
    FInputElements.clear();
}

void __fastcall TAbstractDialog::FormCreate(TObject *Sender)
{
    // Дополнительная инициализация если нужна
}

TPanel* TAbstractDialog::CreateInputPanel(const std::string& name)
{
    TPanel* panel = new TPanel(ScrollBox1);
    panel->Parent = ScrollBox1;
    panel->Width = ScrollBox1->Width - 25;
    panel->Height = 35;
    panel->Left = 10;
    panel->Top = FNextTop;
    panel->BevelOuter = bvNone;

    // Метка с названием
    TLabel* label = new TLabel(panel);
    label->Parent = panel;
    label->Caption = name.c_str();
    label->Left = 10;
    label->Top = 9;
    label->Width = 180;

    // Поле ввода
    TEdit* edit = new TEdit(panel);
    edit->Parent = panel;
    edit->Left = 200;
    edit->Top = 6;
    edit->Width = panel->Width - 210;

    FNextTop += panel->Height + 5;

    return panel;
}

bool TAbstractDialog::Execute()
{
    FContinuePressed = false;

    // Автоподстройка высоты окна
    int contentHeight = FNextTop + BottomPanel->Height + 50;
    if (contentHeight < 300) Height = 300;
    else if (contentHeight > 600) Height = 600;
    else Height = contentHeight;

    return (ShowModal() == mrOk);
}

void __fastcall TAbstractDialog::ContinueButtonClick(TObject *Sender)
{
    // Применяем значения ко всем элементам
    for (auto element : FInputElements) {
        element->ApplyValue();
    }

    FContinuePressed = true;
    ModalResult = mrOk;
}

void __fastcall TAbstractDialog::CancelButtonClick(TObject *Sender)
{
    FContinuePressed = false;
    ModalResult = mrCancel;
}
