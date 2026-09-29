object Choose: TChoose
  Left = 0
  Top = 0
  Caption = 'Choose'
  ClientHeight = 441
  ClientWidth = 624
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object VST: TVirtualStringTree
    Left = 0
    Top = 0
    Width = 624
    Height = 400
    Align = alClient
    Header.AutoSizeIndex = 0
    TabOrder = 0
    OnClick = VSTClick
    OnFreeNode = VSTFreeNode
    OnGetText = VSTGetText
    Touch.InteractiveGestures = [igPan, igPressAndTap]
    Touch.InteractiveGestureOptions = [igoPanSingleFingerHorizontal, igoPanSingleFingerVertical, igoPanInertia, igoPanGutter, igoParentPassthrough]
    Columns = <
      item
        Position = 0
        Text = 'File'
        Width = 200
      end
      item
        Position = 1
        Text = 'Description'
        Width = 500
      end>
  end
  object Panel1: TPanel
    Left = 0
    Top = 400
    Width = 624
    Height = 41
    Align = alBottom
    Caption = 'Panel1'
    TabOrder = 1
    object New: TButton
      Left = 1
      Top = 1
      Width = 216
      Height = 39
      Align = alLeft
      Caption = 'Create New'
      TabOrder = 0
      OnClick = NewClick
    end
    object ViewSeis: TButton
      Left = 217
      Top = 1
      Width = 191
      Height = 39
      Align = alClient
      Caption = 'Seis'
      TabOrder = 1
      OnClick = ViewSeisClick
    end
    object Close: TButton
      Left = 408
      Top = 1
      Width = 215
      Height = 39
      Align = alRight
      Caption = 'Close'
      TabOrder = 2
      OnClick = CloseClick
    end
  end
end
