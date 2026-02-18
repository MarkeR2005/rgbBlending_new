object FormUniversal: TFormUniversal
  Left = 0
  Top = 0
  Caption = 'FormUniversal'
  ClientHeight = 423
  ClientWidth = 582
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Menu = MainMenu1
  OnClose = FormClose
  TextHeight = 15
  object Panel1: TPanel
    Left = 0
    Top = 65
    Width = 15
    Height = 358
    Align = alLeft
    Enabled = False
    TabOrder = 0
    Visible = False
    object ScrollBar1: TScrollBar
      Left = 1
      Top = 1
      Width = 13
      Height = 356
      Align = alClient
      Kind = sbVertical
      PageSize = 0
      Position = 9
      TabOrder = 0
      OnChange = ScrollBar1Change
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 582
    Height = 65
    Align = alTop
    Caption = 'Panel2'
    Enabled = False
    TabOrder = 1
    Visible = False
    object Panel3: TPanel
      Left = 1
      Top = 1
      Width = 580
      Height = 20
      Align = alTop
      TabOrder = 0
      object CheckBoxR: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 18
        Align = alLeft
        Caption = 'R'
        TabOrder = 0
        OnClick = CheckBoxBClick
      end
      object Panel4: TPanel
        Left = 444
        Top = 1
        Width = 135
        Height = 18
        Align = alRight
        TabOrder = 1
        object LabelR: TLabel
          Left = 1
          Top = 1
          Width = 7
          Height = 15
          Align = alClient
          Caption = 'R'
        end
      end
      object ScrollBarR: TScrollBar
        Left = 41
        Top = 1
        Width = 403
        Height = 18
        Align = alClient
        PageSize = 0
        TabOrder = 2
        OnChange = ScrollBarRChange
      end
    end
    object Panel5: TPanel
      Left = 1
      Top = 42
      Width = 580
      Height = 23
      Align = alTop
      TabOrder = 1
      object CheckBoxB: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 21
        Align = alLeft
        Caption = 'B'
        TabOrder = 0
        OnClick = CheckBoxBClick
      end
      object Panel6: TPanel
        Left = 444
        Top = 1
        Width = 135
        Height = 21
        Align = alRight
        TabOrder = 1
        object LabelB: TLabel
          Left = 1
          Top = 1
          Width = 7
          Height = 15
          Align = alClient
          Caption = 'B'
        end
      end
      object ScrollBarB: TScrollBar
        Left = 41
        Top = 1
        Width = 403
        Height = 21
        Align = alClient
        PageSize = 0
        TabOrder = 2
        OnChange = ScrollBarRChange
      end
    end
    object Panel7: TPanel
      Left = 1
      Top = 21
      Width = 580
      Height = 21
      Align = alTop
      TabOrder = 2
      object CheckBoxG: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 19
        Align = alLeft
        Caption = 'G'
        TabOrder = 0
        OnClick = CheckBoxBClick
      end
      object Panel8: TPanel
        Left = 444
        Top = 1
        Width = 135
        Height = 19
        Align = alRight
        TabOrder = 1
        object LabelG: TLabel
          Left = 1
          Top = 1
          Width = 8
          Height = 15
          Align = alClient
          Caption = 'G'
        end
      end
      object ScrollBarG: TScrollBar
        Left = 41
        Top = 1
        Width = 403
        Height = 19
        Align = alClient
        PageSize = 0
        TabOrder = 2
        OnChange = ScrollBarRChange
      end
    end
  end
  object MainMenu1: TMainMenu
    Left = 504
    Top = 368
    object File1: TMenuItem
      Caption = 'File'
      object OpenAs: TMenuItem
        Caption = 'Open as '
        OnClick = OpenAsClick
      end
      object SaveAs: TMenuItem
        Caption = 'Save as'
        OnClick = SaveAsF
      end
      object OpenAsHor: TMenuItem
        Caption = 'Open Horizon as'
        OnClick = OpenAsHorF
      end
      object SaveAsHor: TMenuItem
        Caption = 'Save Horizon as'
        OnClick = SaveAsHorF
      end
    end
    object View1: TMenuItem
      Caption = 'View'
      object ContrastBar: TMenuItem
        Caption = 'ContrastBar'
        OnClick = ContrastBarClick
      end
      object ControlPanel: TMenuItem
        Caption = 'ControlPanel'
        OnClick = ControlPanelClick
      end
      object LeftAxe: TMenuItem
        Caption = 'LeftAxe'
        Checked = True
        OnClick = LeftAxeClick
      end
      object TopAxe: TMenuItem
        Caption = 'TopAxe'
        Checked = True
        OnClick = TopAxeClick
      end
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 472
    Top = 368
  end
  object SaveDialog1: TSaveDialog
    Left = 432
    Top = 368
  end
  object SaveDialog2: TSaveDialog
    Left = 400
    Top = 368
  end
end
