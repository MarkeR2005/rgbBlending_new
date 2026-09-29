object SimpleDialog: TSimpleDialog
  Left = 0
  Top = 0
  Caption = 'Caption'
  ClientHeight = 441
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 624
    Height = 400
    Align = alClient
    Color = clInfoBk
    ParentBackground = False
    TabOrder = 0
    object Description: TLabel
      Left = 1
      Top = 1
      Width = 622
      Height = 398
      Align = alClient
      ExplicitWidth = 3
      ExplicitHeight = 15
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 400
    Width = 624
    Height = 41
    Align = alBottom
    Caption = 'Panel2'
    TabOrder = 1
    object Continue: TButton
      Left = 432
      Top = 1
      Width = 191
      Height = 39
      Align = alRight
      Caption = 'Continue'
      TabOrder = 0
      OnClick = ContinueClick
    end
    object Delete: TButton
      Left = 1
      Top = 1
      Width = 192
      Height = 39
      Align = alLeft
      Caption = 'Delete'
      TabOrder = 1
      OnClick = DeleteClick
    end
    object Cancel: TButton
      Left = 193
      Top = 1
      Width = 239
      Height = 39
      Align = alClient
      Caption = 'Cancel'
      TabOrder = 2
      OnClick = CancelClick
    end
  end
end
