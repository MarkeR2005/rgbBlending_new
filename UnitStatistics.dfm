object UnitStatistic: TUnitStatistic
  Left = 0
  Top = 0
  Caption = 'Spectrum'
  ClientHeight = 1061
  ClientWidth = 2544
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  PixelsPerInch = 96
  TextHeight = 15
  object ScrollBox1: TScrollBox
    Left = 0
    Top = 0
    Width = 2544
    Height = 1061
    Align = alClient
    AutoSize = True
    TabOrder = 0
    ExplicitWidth = 624
    ExplicitHeight = 441
    object Panel3: TPanel
      Left = 0
      Top = 0
      Width = 2540
      Height = 1057
      Align = alClient
      Caption = 'Panel3'
      TabOrder = 0
      OnResize = Draw
      ExplicitWidth = 620
      ExplicitHeight = 437
      object Image1: TImage
        Left = 1
        Top = 1
        Width = 2538
        Height = 1055
        Align = alClient
        AutoSize = True
        ExplicitLeft = 320
        ExplicitTop = 320
        ExplicitWidth = 105
        ExplicitHeight = 105
      end
    end
  end
end
