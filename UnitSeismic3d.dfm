object Seismic3d: TSeismic3d
  Left = 0
  Top = 0
  Caption = 'Seismic'
  ClientHeight = 436
  ClientWidth = 604
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Menu = MainMenu1
  TextHeight = 15
  object Panel1: TPanel
    Left = 0
    Top = 65
    Width = 604
    Height = 371
    Align = alClient
    TabOrder = 0
    OnResize = Panel1Resize
    ExplicitTop = 0
    ExplicitWidth = 608
    ExplicitHeight = 437
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 604
    Height = 65
    Align = alTop
    Caption = 'Panel2'
    Enabled = False
    TabOrder = 1
    Visible = False
    ExplicitLeft = 1
    ExplicitTop = 1
    ExplicitWidth = 606
    object Panel3: TPanel
      Left = 1
      Top = 1
      Width = 606
      Height = 20
      Align = alTop
      TabOrder = 0
      ExplicitWidth = 604
      object CheckBoxR: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 18
        Align = alLeft
        Caption = 'R'
        TabOrder = 0
        OnClick = CheckBoxRClick
      end
      object Panel4: TPanel
        Left = 470
        Top = 1
        Width = 135
        Height = 18
        Align = alRight
        TabOrder = 1
        ExplicitLeft = 468
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
        Width = 429
        Height = 18
        Align = alClient
        PageSize = 0
        TabOrder = 2
        OnChange = ScrollBar1Change
        ExplicitWidth = 427
      end
    end
    object Panel5: TPanel
      Left = 1
      Top = 42
      Width = 606
      Height = 23
      Align = alTop
      TabOrder = 1
      ExplicitWidth = 604
      object CheckBoxB: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 21
        Align = alLeft
        Caption = 'B'
        TabOrder = 0
        OnClick = CheckBoxRClick
      end
      object Panel6: TPanel
        Left = 470
        Top = 1
        Width = 135
        Height = 21
        Align = alRight
        TabOrder = 1
        ExplicitLeft = 468
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
        Width = 429
        Height = 21
        Align = alClient
        PageSize = 0
        TabOrder = 2
        OnChange = ScrollBar1Change
        ExplicitWidth = 427
      end
    end
    object Panel7: TPanel
      Left = 1
      Top = 21
      Width = 606
      Height = 21
      Align = alTop
      TabOrder = 2
      ExplicitWidth = 604
      object CheckBoxG: TCheckBox
        Left = 1
        Top = 1
        Width = 40
        Height = 19
        Align = alLeft
        Caption = 'G'
        TabOrder = 0
        OnClick = CheckBoxRClick
      end
      object Panel8: TPanel
        Left = 470
        Top = 1
        Width = 135
        Height = 19
        Align = alRight
        TabOrder = 1
        ExplicitLeft = 468
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
        Width = 429
        Height = 19
        Align = alClient
        PageSize = 0
        TabOrder = 2
        OnChange = ScrollBar1Change
        ExplicitWidth = 427
      end
    end
  end
  object MainMenu1: TMainMenu
    Left = 537
    Top = 409
    object File1: TMenuItem
      Caption = 'File'
      object File2: TMenuItem
        Caption = 'Open'
        OnClick = File2Click
      end
      object AddSlice: TMenuItem
        Caption = 'AddSlice'
        OnClick = AddClick
      end
      object AddInline: TMenuItem
        Caption = 'AddInline'
        OnClick = AddClick
      end
      object AddCrossline: TMenuItem
        Caption = 'AddCrossline'
        OnClick = AddClick
      end
    end
    object View1: TMenuItem
      Caption = 'View'
      object View2: TMenuItem
        Caption = 'ControlPanel'
        OnClick = View2Click
      end
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 568
    Top = 408
  end
  object PopupMenu1: TPopupMenu
    Left = 496
    Top = 408
    object DeletePlane1: TMenuItem
      Caption = 'DeletePlane'
      OnClick = DeletePlane1Click
    end
    object NewWindowPlane: TMenuItem
      Caption = 'OpenOnNewWindow'
    end
    object AddInline1: TMenuItem
      Caption = 'AddInline'
      OnClick = AddClick
    end
    object AddCrossline1: TMenuItem
      Caption = 'AddCrossline'
      OnClick = AddClick
    end
    object AddSlice1: TMenuItem
      Caption = 'AddSlice'
      OnClick = AddClick
    end
    object SetTransparency1: TMenuItem
      Caption = 'SetTransparency'
      OnClick = SetTransparency1Click
    end
  end
end
