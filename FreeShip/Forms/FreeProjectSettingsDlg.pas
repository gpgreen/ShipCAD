{#############################################################################################}
{    This code is distributed as part of the FREE!ship project. FREE!ship is an               }
{    open source surface-modelling program based on subdivision surfaces and intended for     }
{    designing ships.                                                                         }
{                                                                                             }
{    Copyright © 2005, by Martijn van Engeland                                                }
{    e-mail                  : Info@FREEship.org                                              }
{    FREE!ship project page  : https://sourceforge.net/projects/freeship                      }
{    FREE!ship homepage      : www.FREEship.org                                               }
{                                                                                             }
{    This program is free software; you can redistribute it and/or modify it under            }
{    the terms of the GNU General Public License as published by the                          }
{    Free Software Foundation; either version 2 of the License, or (at your option)           }
{    any later version.                                                                       }
{                                                                                             }
{    This program is distributed in the hope that it will be useful, but WITHOUT ANY          }
{    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          }
{    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 }
{                                                                                             }
{    You should have received a copy of the GNU General Public License along with             }
{    this program; if not, write to the Free Software Foundation, Inc.,                       }
{    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    }
{                                                                                             }
{#############################################################################################}

unit FreeProjectSettingsDlg;

interface

uses Windows,
     Messages,
     SysUtils,
     Variants,
     Classes,
     Graphics,
     Controls,
     Forms,
     Dialogs,
     ExtCtrls,
     StdCtrls,
     FreeGeometry,
     Buttons, ComCtrls;

type TFREEProjectSettingsDialog   = class(TForm)
                                          Panel1: TPanel;
                                          Panel3: TPanel;
                                          ColorDialog: TColorDialog;
                                          BitBtn1: TSpeedButton;
                                          BitBtn2: TSpeedButton;
                                          PageControl1: TPageControl;
                                          TabSheet1: TTabSheet;
                                          Panel: TPanel;
                                          Label1: TLabel;
                                          Label7: TLabel;
                                          Label8: TLabel;
                                          Label15: TLabel;
                                          Label16: TLabel;
                                          Edit1: TEdit;
                                          Edit7: TEdit;
                                          CheckBox1: TCheckBox;
                                          Panel4: TPanel;
                                          Unitbox: TComboBox;
                                          Edit9: TEdit;
                                          Edit10: TEdit;
                                          CheckBox4: TCheckBox;
                                          TabSheet2: TTabSheet;
                                          Panel2: TPanel;
                                          TabSheet3: TTabSheet;
                                          Panel5: TPanel;
                                          Label2: TLabel;
                                          Edit2: TEdit;
                                          Label9: TLabel;
                                          Label10: TLabel;
                                          Edit3: TEdit;
                                          Label3: TLabel;
                                          Label4: TLabel;
                                          Edit4: TEdit;
                                          Label11: TLabel;
                                          Label13: TLabel;
                                          Edit8: TEdit;
                                          CheckBox2: TCheckBox;
                                          Label14: TLabel;
                                          GroupBox1: TGroupBox;
                                          Label18: TLabel;
                                          Label19: TLabel;
                                          CheckBox5: TCheckBox;
                                          CheckBox6: TCheckBox;
                                          CheckBox7: TCheckBox;
                                          CheckBox8: TCheckBox;
                                          CheckBox9: TCheckBox;
                                          Label5: TLabel;
                                          Edit5: TEdit;
                                          Label12: TLabel;
                                          Label6: TLabel;
                                          Edit6: TEdit;
                                          Label17: TLabel;
                                          ComboBox1: TComboBox;
                                          CheckBox3: TCheckBox;
                                          CheckBox10: TCheckBox;
                                          procedure Edit2KeyPress(Sender: TObject; var Key: Char);
                                          procedure Edit2Exit(Sender: TObject);
                                          procedure Edit3KeyPress(Sender: TObject; var Key: Char);
                                          procedure EditExit(Sender: TObject);
                                          procedure Edit4KeyPress(Sender: TObject; var Key: Char);
                                          procedure Edit4Exit(Sender: TObject);
                                          procedure Edit5KeyPress(Sender: TObject; var Key: Char);
                                          procedure Edit5Exit(Sender: TObject);
                                          procedure Edit6KeyPress(Sender: TObject; var Key: Char);
                                          procedure Edit6Exit(Sender: TObject);
                                          procedure Panel4Click(Sender: TObject);
                                          procedure UnitboxClick(Sender: TObject);
                                          procedure Edit8Exit(Sender: TObject);
                                          procedure Edit8KeyPress(Sender: TObject; var Key: Char);
                                          procedure CheckBox2Click(Sender: TObject);
                                          procedure BitBtn1Click(Sender: TObject);
                                          procedure BitBtn2Click(Sender: TObject);
                                          procedure FormShow(Sender: TObject);
                                       private   { Private declarations }
                                          ConversionFactor:Double;
                                          function FGetConversionFactor:double;
                                          function FGetBeam:double;
                                          procedure FSetBeam(Val:double);
                                          function FGetCoefficient:double;
                                          procedure FSetCoefficient(Val:double);
                                          function FGetDensity:double;
                                          procedure FSetDensity(Val:double);
                                          function FGetDraft:double;
                                          procedure FSetDraft(Val:double);
                                          function FGetLength:double;
                                          procedure FSetLength(Val:double);
                                          function FGetMainframe:double;
                                          procedure FSetMainframe(Val:double);
                                          procedure FSetUnitCaptions;
                                       public    { Public declarations }
                                          function Execute:Boolean;
                                          property Beam              : double read FGetBeam write FSetBeam;
                                          property Coefficient       : double read FGetCoefficient write FSetCoefficient;
                                          property Density           : double read FGetDensity write FSetDensity;
                                          property Draft             : double read FGetDraft write FSetDraft;
                                          property Length            : double read FGetLength write FSetLength;
                                          property Mainframe         : double read FGetMainframe write FSetMainframe;
                                    end;

var FREEProjectSettingsDialog: TFREEProjectSettingsDialog;

implementation

uses FreeshipUnit;

{$R *.dfm}

function TFREEProjectSettingsDialog.FGetConversionFactor:double;
begin
   if Unitbox.ItemIndex=1 then Result:=1/0.3048
                          else Result:=1.0;
end;{TFREEProjectSettingsDialog.FGetConversionFactor}

function TFREEProjectSettingsDialog.FGetBeam:double;
begin
   if Edit3.Text='' then Result:=0.0
                    else Result:=StrToFloat(Edit3.Text);
end;{TFREEProjectSettingsDialog.FGetBeam}

procedure TFREEProjectSettingsDialog.FSetBeam(Val:double);
begin
   Edit3.Text:=FloatToStrF(Val,ffFixed,7,4);
end;{TFREEProjectSettingsDialog.FSetBeam}

function TFREEProjectSettingsDialog.FGetCoefficient:double;
begin
   if Edit6.Text='' then Result:=0.0
                    else Result:=StrToFloat(Edit6.Text);
end;{TFREEProjectSettingsDialog.FGetCoefficient}

procedure TFREEProjectSettingsDialog.FSetCoefficient(Val:double);
begin
   Edit6.Text:=FloatToStrF(Val,ffFixed,7,4);
end;{TFREEProjectSettingsDialog.FSetCoefficient}

function TFREEProjectSettingsDialog.FGetDensity:double;
begin
   if Edit5.Text='' then Result:=0.0
                    else Result:=StrToFloat(Edit5.Text);
end;{TFREEProjectSettingsDialog.FGetDensity}

procedure TFREEProjectSettingsDialog.FSetDensity(Val:double);
begin
   Edit5.Text:=FloatToStrF(Val,ffFixed,7,4);
end;{TFREEProjectSettingsDialog.FSetDensity}

function TFREEProjectSettingsDialog.FGetDraft:double;
begin
   if Edit4.Text='' then Result:=0.0
                    else Result:=StrToFloat(Edit4.Text);
end;{TFREEProjectSettingsDialog.FGetDraft}

procedure TFREEProjectSettingsDialog.FSetDraft(Val:double);
begin
   Edit4.Text:=FloatToStrF(Val,ffFixed,7,4);
end;{TFREEProjectSettingsDialog.FSetDraft}

function TFREEProjectSettingsDialog.FGetLength:double;
begin
   if Edit2.Text='' then Result:=0.0
                    else Result:=StrToFloat(Edit2.Text);
end;{TFREEProjectSettingsDialog.FGetLength}

procedure TFREEProjectSettingsDialog.FSetLength(Val:double);
begin
   Edit2.Text:=FloatToStrF(Val,ffFixed,7,4);
   if Checkbox2.Checked then Mainframe:=0.5*Length;
end;{TFREEProjectSettingsDialog.FSetLength}

function TFREEProjectSettingsDialog.FGetMainframe:double;
begin
   if Edit8.Text='' then Result:=0.0
                    else Result:=StrToFloat(Edit8.Text);
end;{TFREEProjectSettingsDialog.FGetMainframe}

procedure TFREEProjectSettingsDialog.FSetMainframe(Val:double);
begin
   Edit8.Text:=FloatToStrF(Val,ffFixed,7,4);
end;{TFREEProjectSettingsDialog.FSetMainframe}

procedure TFREEProjectSettingsDialog.FSetUnitCaptions;
var Str : string;
begin
   if UnitBox.ItemIndex=1 then Str:=LengthStr(fuImperial)
                          else Str:=Lengthstr(fuMetric);
   Label9.Caption:=Str;
   Label10.Caption:=Str;
   Label11.Caption:=Str;
   Label14.Caption:=Str;
   if UnitBox.ItemIndex=1 then Str:=DensityStr(fuImperial)
                          else Str:=DensityStr(fuMetric);
   Label12.Caption:=Str;
end;{TFREEProjectSettingsDialog.FSetUnitCaptions}

function TFREEProjectSettingsDialog.Execute:Boolean;
begin
   Conversionfactor:=FGetConversionFactor;
   FSetUnitCaptions;
   ShowModal;
   Result:=Modalresult=mrOk;
end;{TFREEProjectSettingsDialog.Execute}

procedure TFREEProjectSettingsDialog.Edit2KeyPress(Sender: TObject;var Key: Char);
begin
   if (Key in [#8,'1'..'9','0','-',#13]) or (Key=DecimalSeparator) then else key:=#0;
end;{TFREEProjectSettingsDialog.Edit2KeyPress}

procedure TFREEProjectSettingsDialog.Edit2Exit(Sender: TObject);
begin
   Length:=Length; // force repaint
end;{TFREEProjectSettingsDialog.Edit2Exit}

procedure TFREEProjectSettingsDialog.Edit3KeyPress(Sender: TObject; var Key: Char);
begin
   if (Key in [#8,'1'..'9','0','-',#13]) or (Key=DecimalSeparator) then else key:=#0;
end;{TFREEProjectSettingsDialog.Edit3KeyPress}

procedure TFREEProjectSettingsDialog.EditExit(Sender: TObject);
begin
   Beam:=Beam;
end;{TFREEProjectSettingsDialog.Edit3Exit}

procedure TFREEProjectSettingsDialog.Edit4KeyPress(Sender: TObject;  var Key: Char);
begin
   if (Key in [#8,'1'..'9','0','-',#13]) or (Key=DecimalSeparator) then else key:=#0;
end;{TFREEProjectSettingsDialog.Edit4KeyPress}

procedure TFREEProjectSettingsDialog.Edit4Exit(Sender: TObject);
begin
   Draft:=Draft;
end;{TFREEProjectSettingsDialog.Edit4Exit}

procedure TFREEProjectSettingsDialog.Edit5KeyPress(Sender: TObject;var Key: Char);
begin
   if (Key in [#8,'1'..'9','0','-',#13]) or (Key=DecimalSeparator) then else key:=#0;
end;{TFREEProjectSettingsDialog.Edit5KeyPress}

procedure TFREEProjectSettingsDialog.Edit5Exit(Sender: TObject);
begin
   Density:=Density;
end;{TFREEProjectSettingsDialog.Edit5Exit}

procedure TFREEProjectSettingsDialog.Edit6KeyPress(Sender: TObject;var Key: Char);
begin
   if (Key in [#8,'1'..'9','0','-',#13]) or (Key=DecimalSeparator) then else key:=#0;
end;{TFREEProjectSettingsDialog.Edit6KeyPress}

procedure TFREEProjectSettingsDialog.Edit6Exit(Sender: TObject);
begin
   Coefficient:=Coefficient;
end;{TFREEProjectSettingsDialog.Edit6Exit}

procedure TFREEProjectSettingsDialog.Panel4Click(Sender: TObject);
begin
   ColorDialog.Color:=panel4.Color;
   if ColorDialog.Execute then
   begin
      Panel4.Color:=ColorDialog.Color;
   end;
end;{TFREEProjectSettingsDialog.Panel4Click}

procedure TFREEProjectSettingsDialog.UnitboxClick(Sender: TObject);
begin
   Length:=Length/ConversionFactor;
   Beam:=Beam/Conversionfactor;
   Draft:=Draft/Conversionfactor;
   Mainframe:=MainFrame/ConversionFactor;
   Case Unitbox.ItemIndex of
      0 : Density:=Density/WeightConversionFactor;
      1 : Density:=Density*WeightConversionFactor;
   end;
   Conversionfactor:=FGetConversionFactor;
   Length:=Length*ConversionFactor;
   Beam:=Beam*Conversionfactor;
   Draft:=Draft*Conversionfactor;
   if not checkbox2.Checked then Mainframe:=MainFrame*ConversionFactor;
   FSetUnitCaptions;
end;{TFREEProjectSettingsDialog.UnitboxClick}

procedure TFREEProjectSettingsDialog.Edit8Exit(Sender: TObject);
begin
   Mainframe:=Mainframe;
end;{TFREEProjectSettingsDialog.Edit8Exit}

procedure TFREEProjectSettingsDialog.Edit8KeyPress(Sender: TObject;var Key: Char);
begin
   if (Key in [#8,'1'..'9','0','-',#13]) or (Key=DecimalSeparator) then else key:=#0;
end;{TFREEProjectSettingsDialog.Edit8KeyPress}

procedure TFREEProjectSettingsDialog.CheckBox2Click(Sender: TObject);
begin
   if Checkbox2.Checked then
   begin
      Label13.Enabled:=False;
      Label14.Enabled:=False;
      Edit8.Color:=clBtnFace;
      Edit8.Font.Color:=clDkGray;
      Edit8.Enabled:=False;
   end else
   begin
      Label13.Enabled:=True;
      Label14.Enabled:=True;
      Edit8.Color:=clWindow;
      Edit8.Font.Color:=clBlack;
      Edit8.Enabled:=True;
   end;
end;{TFREEProjectSettingsDialog.CheckBox2Click}

procedure TFREEProjectSettingsDialog.BitBtn1Click(Sender: TObject);
begin
   Modalresult:=mrOK;
end;{TFREEProjectSettingsDialog.BitBtn1Click}

procedure TFREEProjectSettingsDialog.BitBtn2Click(Sender: TObject);
begin
   Modalresult:=mrCancel;
end;{TFREEProjectSettingsDialog.BitBtn2Click}

procedure TFREEProjectSettingsDialog.FormShow(Sender: TObject);
begin
   Pagecontrol1.ActivePage:=Tabsheet1;
   ActiveControl:=Edit1;
end;{TFREEProjectSettingsDialog.FormShow}

end.
