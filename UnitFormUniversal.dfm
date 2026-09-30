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
  KeyPreview = True
  OnKeyDown = FormKeyDown
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
    Color = clInfoBk
    ParentBackground = False
    TabOrder = 1
    object Panel3: TPanel
      Left = 1
      Top = 1
      Width = 580
      Height = 20
      Align = alTop
      Color = clRed
      ParentBackground = False
      TabOrder = 0
      object CheckBoxR: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 18
        Align = alLeft
        Caption = 'R'
        Checked = True
        Color = clRed
        ParentColor = False
        State = cbChecked
        TabOrder = 0
        OnClick = CheckBoxBClick
      end
      object Panel4: TPanel
        Left = 444
        Top = 1
        Width = 135
        Height = 18
        Align = alRight
        Color = clRed
        ParentBackground = False
        TabOrder = 1
        object LabelR: TLabel
          Left = 1
          Top = 1
          Width = 7
          Height = 15
          Align = alClient
          Caption = 'R'
          Color = clRed
          ParentColor = False
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
      Color = clBlue
      ParentBackground = False
      TabOrder = 1
      object CheckBoxB: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 21
        Align = alLeft
        Caption = 'B'
        Checked = True
        Color = clBlue
        ParentColor = False
        State = cbChecked
        TabOrder = 0
        OnClick = CheckBoxBClick
      end
      object Panel6: TPanel
        Left = 444
        Top = 1
        Width = 135
        Height = 21
        Align = alRight
        Color = clBlue
        ParentBackground = False
        TabOrder = 1
        object LabelB: TLabel
          Left = 1
          Top = 1
          Width = 7
          Height = 15
          Align = alClient
          Caption = 'B'
          Color = clBlue
          ParentColor = False
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
      Color = clGreen
      ParentBackground = False
      TabOrder = 2
      object CheckBoxG: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 19
        Align = alLeft
        Caption = 'G'
        Checked = True
        Color = clGreen
        ParentColor = False
        State = cbChecked
        TabOrder = 0
        OnClick = CheckBoxBClick
      end
      object Panel8: TPanel
        Left = 444
        Top = 1
        Width = 135
        Height = 19
        Align = alRight
        Color = clGreen
        ParentBackground = False
        TabOrder = 1
        object LabelG: TLabel
          Left = 1
          Top = 1
          Width = 8
          Height = 15
          Align = alClient
          Caption = 'G'
          Color = clGreen
          ParentColor = False
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
        Enabled = False
        Visible = False
        OnClick = OpenAsClick
      end
      object SaveAs: TMenuItem
        Caption = 'Save as'
        OnClick = SaveAsF
      end
      object OpenAsHor: TMenuItem
        Caption = 'Open Horizon as'
        Enabled = False
        Visible = False
        OnClick = OpenAsHorF
      end
      object SaveAsHor: TMenuItem
        Caption = 'Save Horizon as'
        Enabled = False
        Visible = False
        OnClick = SaveAsHorF
      end
      object SaveScreenShot1: TMenuItem
        Caption = 'Save ScreenShot'
        OnClick = SaveScreenShot1Click
      end
    end
    object View1: TMenuItem
      Caption = 'View'
      object ShowHorizons: TMenuItem
        Caption = 'Horizons (H)'
        Checked = True
        OnClick = ShowHorizonsClick
      end
      object ShowCrosses: TMenuItem
        Caption = 'Crosses (C)'
        Checked = True
        OnClick = ShowCrossesClick
      end
      object ContrastBar: TMenuItem
        Caption = 'ContrastBar'
        OnClick = ContrastBarClick
      end
      object ControlPanel: TMenuItem
        Caption = 'ControlPanel'
        Checked = True
        OnClick = ControlPanelClick
      end
      object LeftAxe: TMenuItem
        Caption = 'LeftAxe'
        Checked = True
        OnClick = LeftAxeClick
      end
      object RightAxe: TMenuItem
        Caption = 'RightAxe'
        Checked = True
        OnClick = RightAxeClick
      end
      object TopAxe: TMenuItem
        Caption = 'TopAxe'
        Checked = True
        OnClick = TopAxeClick
      end
      object Changepalette1: TMenuItem
        Caption = 'Change palette'
        Visible = False
        OnClick = Changepalette1Click
      end
      object Changeshader1: TMenuItem
        Caption = 'Change shader'
        Enabled = False
        Visible = False
        OnClick = Changeshader1Click
      end
      object SetRatio1: TMenuItem
        Caption = 'SetRatio'
        OnClick = SetRatio1Click
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
