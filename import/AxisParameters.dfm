object AxisParametersForm: TAxisParametersForm
  Left = 492
  Top = 287
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Axis parameters'
  ClientHeight = 246
  ClientWidth = 585
  Color = clInfoBk
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = [fsBold]
  KeyPreview = True
  Position = poOwnerFormCenter
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  DesignSize = (
    585
    246)
  TextHeight = 13
  object Panel2: TPanel
    Left = 8
    Top = 8
    Width = 569
    Height = 202
    Anchors = [akLeft, akTop, akRight, akBottom]
    Caption = 'Panel2'
    Color = clBackground
    TabOrder = 0
    DesignSize = (
      569
      202)
    object Panel1: TPanel
      Left = 8
      Top = 8
      Width = 553
      Height = 186
      Anchors = [akLeft, akTop, akRight, akBottom]
      Color = clInfoBk
      TabOrder = 0
      DesignSize = (
        553
        186)
      object VGB: TGroupBox
        Left = 280
        Top = 8
        Width = 265
        Height = 171
        Anchors = [akLeft, akTop, akBottom]
        Caption = 'Vertical axis'
        Enabled = False
        TabOrder = 0
        DesignSize = (
          265
          171)
        object VExL: TLabel
          Left = 8
          Top = 93
          Width = 249
          Height = 68
          Alignment = taCenter
          Anchors = [akLeft, akTop, akRight, akBottom]
          AutoSize = False
          Caption = 'Ex: 12345'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clRed
          Font.Height = -19
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          Layout = tlCenter
        end
        object VAutoCB: TCheckBox
          Left = 8
          Top = 24
          Width = 145
          Height = 17
          Caption = 'Automatic Calculation'
          Checked = True
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          State = cbChecked
          TabOrder = 0
          OnClick = VAutoCBClick
        end
        object VLE: TLabeledEdit
          Left = 8
          Top = 64
          Width = 97
          Height = 21
          EditLabel.Width = 109
          EditLabel.Height = 13
          EditLabel.Caption = 'Major ticks interval'
          EditLabel.Font.Charset = DEFAULT_CHARSET
          EditLabel.Font.Color = clWindowText
          EditLabel.Font.Height = -11
          EditLabel.Font.Name = 'MS Sans Serif'
          EditLabel.Font.Style = [fsBold]
          EditLabel.ParentFont = False
          TabOrder = 1
          Text = '0'
        end
        object VMajorTicksUD: TUpDown
          Left = 105
          Top = 64
          Width = 15
          Height = 21
          Associate = VLE
          Max = 10000
          TabOrder = 2
          Thousands = False
        end
        object VMinorCB: TCheckBox
          Left = 128
          Top = 72
          Width = 129
          Height = 17
          Caption = 'Show Minor Ticks'
          Checked = True
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          State = cbChecked
          TabOrder = 3
        end
        object CancelSB: TButton
          Left = -2
          Top = 136
          Width = 75
          Height = 25
          Caption = 'CancelSB'
          TabOrder = 4
        end
        object OKSB: TButton
          Left = 94
          Top = 91
          Width = 75
          Height = 25
          Caption = 'OKSB'
          TabOrder = 5
        end
        object HFSB: TButton
          Left = 160
          Top = 136
          Width = 75
          Height = 25
          Caption = 'HFSB'
          TabOrder = 6
          OnClick = HFSBClick
        end
        object VFSB: TButton
          Left = 79
          Top = 136
          Width = 75
          Height = 25
          Caption = 'VFSB'
          TabOrder = 7
          OnClick = VFSBClick
        end
      end
      object HGB: TGroupBox
        Left = 7
        Top = 8
        Width = 265
        Height = 171
        Anchors = [akLeft, akTop, akBottom]
        Caption = 'Horizontal axis'
        TabOrder = 1
        DesignSize = (
          265
          171)
        object HExL: TLabel
          Left = 8
          Top = 93
          Width = 249
          Height = 68
          Alignment = taCenter
          Anchors = [akLeft, akTop, akRight, akBottom]
          AutoSize = False
          Caption = 'Ex: 12345'
          Font.Charset = RUSSIAN_CHARSET
          Font.Color = clBlack
          Font.Height = -64
          Font.Name = 'Arial Narrow'
          Font.Style = [fsBold]
          ParentFont = False
          Layout = tlCenter
        end
        object HAutoCB: TCheckBox
          Left = 8
          Top = 24
          Width = 145
          Height = 17
          Caption = 'Automatic Calculation'
          Checked = True
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          State = cbChecked
          TabOrder = 0
          OnClick = HAutoCBClick
        end
        object HLE: TLabeledEdit
          Left = 8
          Top = 64
          Width = 97
          Height = 21
          EditLabel.Width = 109
          EditLabel.Height = 13
          EditLabel.Caption = 'Major ticks interval'
          EditLabel.Font.Charset = DEFAULT_CHARSET
          EditLabel.Font.Color = clWindowText
          EditLabel.Font.Height = -11
          EditLabel.Font.Name = 'MS Sans Serif'
          EditLabel.Font.Style = [fsBold]
          EditLabel.ParentFont = False
          TabOrder = 1
          Text = '0'
        end
        object HMajorTicksUD: TUpDown
          Left = 105
          Top = 64
          Width = 15
          Height = 21
          Associate = HLE
          Max = 10000
          TabOrder = 2
          Thousands = False
        end
        object HMinorCB: TCheckBox
          Left = 128
          Top = 72
          Width = 129
          Height = 17
          Caption = 'Show Minor Ticks'
          Checked = True
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          State = cbChecked
          TabOrder = 3
        end
      end
    end
  end
  object FontDialog1: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Left = 8
    Top = 208
  end
end
