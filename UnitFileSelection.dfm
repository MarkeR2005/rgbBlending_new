object FileSelection: TFileSelection
  Left = 0
  Top = 0
  Caption = 'FileSelection'
  ClientHeight = 156
  ClientWidth = 320
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object Button2: TButton
    Left = 96
    Top = 83
    Width = 137
    Height = 25
    Caption = 'Go 3D'
    TabOrder = 0
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 96
    Top = 114
    Width = 137
    Height = 25
    Caption = 'Main'
    TabOrder = 1
    OnClick = Button3Click
  end
  object Button1: TButton
    Left = 96
    Top = 43
    Width = 137
    Height = 25
    Caption = 'Compute'
    TabOrder = 2
    OnClick = Button1Click
  end
end
