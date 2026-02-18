//---------------------------------------------------------------------------

#ifndef DialogH
#define DialogH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <vector>
#include <string>

class TInputElement;

class TAbstractDialog : public TForm
{
__published:
    TScrollBox *ScrollBox1;
    TPanel *BottomPanel;
    TButton *ContinueButton;
    TButton *CancelButton;

    void __fastcall ContinueButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);

private:
    std::vector<TInputElement*> FInputElements;
    int FNextTop;
    bool FContinuePressed;

    TPanel* CreateInputPanel(const std::string& name);

public:
    __fastcall TAbstractDialog(TComponent* Owner);
    __fastcall ~TAbstractDialog();

    template<typename T>
    void AddInput(const std::string& name, T& variable);

    bool Execute();
    __property bool ContinuePressed = {read=FContinuePressed};
};

// Базовый класс для элементов ввода
class TInputElement : public TObject
{
public:
    virtual __fastcall ~TInputElement() {}
    virtual void ApplyValue() = 0;
    virtual void UpdateDisplay() = 0;
};

// Шаблонный класс для элементов ввода
template<typename T>
class TTypedInputElement : public TInputElement
{
private:
    T* FVariable;
    TEdit* FEdit;
    std::string FName;

public:
    __fastcall TTypedInputElement(const std::string& name, T* variable, TEdit* edit)
        : FVariable(variable), FEdit(edit), FName(name)
    {
        UpdateDisplay();
    }

    void ApplyValue() override;
    void UpdateDisplay() override;
};

// Специализации для разных типов данных
template<>
void TTypedInputElement<int>::ApplyValue();

template<>
void TTypedInputElement<int>::UpdateDisplay();

template<>
void TTypedInputElement<double>::ApplyValue();

template<>
void TTypedInputElement<double>::UpdateDisplay();

template<>
void TTypedInputElement<float>::ApplyValue();

template<>
void TTypedInputElement<float>::UpdateDisplay();

template<>
void TTypedInputElement<std::string>::ApplyValue();

template<>
void TTypedInputElement<std::string>::UpdateDisplay();

template<>
void TTypedInputElement<AnsiString>::ApplyValue();

template<>
void TTypedInputElement<AnsiString>::UpdateDisplay();

// Реализация шаблонного метода AddInput
template<typename T>
void TAbstractDialog::AddInput(const std::string& name, T& variable)
{
    TPanel* panel = CreateInputPanel(name);
    TEdit* edit = dynamic_cast<TEdit*>(panel->Controls[1]);

    if (edit) {
        TTypedInputElement<T>* inputElement =
            new TTypedInputElement<T>(name, &variable, edit);
        FInputElements.push_back(inputElement);
    }
}

#endif
