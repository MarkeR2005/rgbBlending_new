object AbstractDialog: TAbstractDialog
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099
  ClientHeight = 400
  ClientWidth = 450
  Color = clInfoBk
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Position = poScreenCenter
  OnCreate = FormCreate
  TextHeight = 13
  object BottomPanel: TPanel
    Left = 0
    Top = 359
    Width = 450
    Height = 41
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object ContinueButton: TButton
      Left = 230
      Top = 8
      Width = 100
      Height = 25
      Caption = #1055#1088#1086#1076#1086#1083#1078#1080#1090#1100
      Default = True
      TabOrder = 0
      OnClick = ContinueButtonClick
    end
    object CancelButton: TButton
      Left = 340
      Top = 8
      Width = 100
      Height = 25
      Cancel = True
      Caption = #1054#1090#1084#1077#1085#1072
      TabOrder = 1
      OnClick = CancelButtonClick
    end
  end
  object ScrollBox1: TScrollBox
    Left = 0
    Top = 0
    Width = 450
    Height = 359
    Align = alClient
    BorderStyle = bsNone
    TabOrder = 1
  end
end
