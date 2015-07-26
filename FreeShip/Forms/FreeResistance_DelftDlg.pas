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
unit FreeResistance_DelftDlg;

interface

uses SysUtils,
     Forms,
     FreeNumInput,
     TeEngine,
     Series,
     Dialogs,
     TeeProcs,
     Chart,
     StdCtrls,
     ComCtrls,
     Controls,
     Buttons,
     Classes,
     Printers,
     FreeGeometry,
     FreeshipUnit,
     Math,
     ExtCtrls,
     ImgList,
     ToolWin,
     Menus;

const Matrix1 : array[0..13,0..9] of extended=((-6.735654,38.368310,-0.008193,0.055234,-1.997242,-38.860810,0.956591,-0.002171,0.272895,-0.017516),
                                               (-0.382870,38.172900,0.007243,0.026644,-5.295332,-39.550320,1.219563,0.000052,0.824568,-0.047842),
                                               (-1.503526,24.408030,0.012200,0.067221,-2.448582,-31.913700,2.216098,0.000074,0.244345,-0.015887),
                                               (11.292180,-14.519470,0.047182,0.085176,-2.673016,-11.418190,5.654065,0.007021,-0.094934,0.006325),
                                               (22.178670,-49.167840,0.085998,0.150725,-2.878684,7.167049,8.600272,0.012981,-0.327085,0.018271),
                                               (25.908670,-74.756680,0.153521,0.188568,-0.889467,24.121370,10.485160,0.025348,-0.854940,0.048449),
                                               (40.975590,-114.285500,0.207226,0.250827,-3.072662,53.015700,13.021770,0.035943,-0.715457,0.039874),
                                               (45.837590,-184.764600,0.357031,0.338343,3.871658,132.256800,10.860540,0.066809,-1.719215,0.095977),
                                               (89.203820,-393.0127,0.617466,0.460472,11.54327,331.1197,8.598136,0.104073,-2.815203,0.155960),
                                               (212.678800,-801.790800,1.087307,0.538938,10.802730,667.644500,12.398150,0.166473,-3.026131,0.165055),
                                               (336.235400,-1085.134000,1.644191,0.532702,-1.224173,831.144500,26.183210,0.238795,-2.450470,0.139154),
                                               (566.547600,-1609.632000,2.016090,0.265722,-29.244120,1154.091,51.46575,0.288046,-0.178354,0.018446),
                                               (743.410700,-1708.263000,2.435809,0.013553,-81.161890,937.401400,115.600600,0.365071,1.838967,-0.062023),
                                               (1200.62,-2751.715,3.208577,0.254920,-132.0424,1489.269,196.3406,0.528225,1.379102,0.013577));
      Matrix2 : array[0..11,0..5] of extended=((180.100400,-31.502570,-7.451141,2.195042,2.689623,0.006480),
                                               (243.999400,-44.525510,-11.154560,2.179046,3.857403,0.009676),
                                               (282.987300,-51.519530,-12.973100,2.274505,4.343662,0.011066),
                                               (313.410900,-56.582570,-14.419780,2.326117,4.690432,0.012147),
                                               (373.003800,-59.190290,-16.069750,2.519156,4.766793,0.014147),
                                               (356.457200,-62.853950,-16.851120,2.437056,5.078768,0.014980),
                                               (324.735700,-51.312520,-15.345950,2.334146,3.855368,0.013695),
                                               (301.126800,-39.796310,-15.022990,2.059657,2.545676,0.013588),
                                               (292.057100,-31.853030,-15.585480,1.847926,1.569917,0.014014),
                                               (284.464100,-25.145580,-16.154230,1.703981,0.817921,0.014575),
                                               (256.636700,-19.319220,-13.084500,2.152824,0.348305,0.011343),
                                               (304.180300,-30.115120,-15.854290,2.863173,1.524379,0.014031));
type TFreeResistance_Delft   = class(TForm)
                                    PrintDialog: TPrintDialog;
                                    PageControl1: TPageControl;
                                    General: TTabSheet;
                                    Panel1: TPanel;
                                    GroupBox1: TGroupBox;
                                    Label1: TLabel;
                                    Label2: TLabel;
                                    Label3: TLabel;
                                    Label4: TLabel;
                                    Label5: TLabel;
                                    StartSpeedBox: TFreeNumInput;
                                    EndSpeedBox: TFreeNumInput;
                                    StepSpeedBox: TFreeNumInput;
                                    DensityBox: TFreeNumInput;
                                    ViscosityBox: TFreeNumInput;
                                    GroupBox2: TGroupBox;
                                    Label13: TLabel;
                                    Label20: TLabel;
                                    Label21: TLabel;
                                    Label22: TLabel;
                                    Label23: TLabel;
                                    Label24: TLabel;
                                    Label25: TLabel;
                                    Label26: TLabel;
                                    Label27: TLabel;
                                    LwlBox: TFreeNumInput;
                                    BwlBox: TFreeNumInput;
                                    DraftBox: TFreeNumInput;
                                    DraftTotalBox: TFreeNumInput;
                                    WettedSurfacebox: TFreeNumInput;
                                    EstimateBox: TCheckBox;
                                    WlAreabox: TFreeNumInput;
                                    DisplacementBox: TFreeNumInput;
                                    LCBBox: TFreeNumInput;
                                    CpBox: TFreeNumInput;
                                    CheckBox2: TCheckBox;
                                    GroupBox3: TGroupBox;
                                    Label28: TLabel;
                                    Label29: TLabel;
                                    KeelChordLengthbox: TFreeNumInput;
                                    KeelAreaBox: TFreeNumInput;
                                    GroupBox4: TGroupBox;
                                    Label6: TLabel;
                                    Label7: TLabel;
                                    RudderChordLengthbox: TFreeNumInput;
                                    RudderAreaBox: TFreeNumInput;
                                    Results: TTabSheet;
                                    Panel5: TPanel;
                                    Resultsmemo: TMemo;
                                    Chart: TChart;
                                    Series1: TLineSeries;
                                    Series2: TLineSeries;
                                    Series3: TLineSeries;
                                    Series4: TLineSeries;
                                    ToolBar1: TToolBar;
                                    ToolButton20: TToolButton;
    _ToolButton10: TToolButton;
    _ToolButton14: TToolButton;
                                    ToolButton25: TToolButton;
                                    ToolButton7: TToolButton;
                                    MenuImages: TImageList;
                                    PrintButton: TToolButton;
    _Label8: TLabel;
    _Label9: TLabel;
    _Label10: TLabel;
    _Label11: TLabel;
    _Label12: TLabel;
    _Label14: TLabel;
    _Label15: TLabel;
    _Label16: TLabel;
    _Label17: TLabel;
    _Label18: TLabel;
    _Label19: TLabel;
    _Label30: TLabel;
    _Label31: TLabel;
    _Label32: TLabel;
    _Label33: TLabel;
                                    Label34: TLabel;
    _Label36: TLabel;
                                    procedure CheckBox2Click(Sender: TObject);
                                    procedure ToolButton25Click(Sender: TObject);
                                    procedure ToolButton7Click(Sender: TObject);
                                    procedure ToolButton20Click(Sender: TObject);
                                    procedure PrintButtonClick(Sender: TObject);
                                    procedure DraftTotalBoxAfterSetValue(Sender: TObject);
                                    procedure StartSpeedBoxAfterSetValue(Sender: TObject);
                                    procedure LwlBoxAfterSetValue(Sender: TObject);
                                    procedure KeelChordLengthboxAfterSetValue(Sender: TObject);
                                    procedure EstimateBoxClick(Sender: TObject);
                                 private
                                    FFreeship         : TFreeship;
                                    B_T,L_D,A_D,L_B   : single;
                                    Am,Cm,Cwp         : single;
                                    function OptimumCP(Speed:single):single;
                                    function OptimumLCB(Speed:single):single;
                                    procedure CalculateResistance(ConvertedSpeed,LCB,Cp:single;var Rf,Rr,Rt: single);
                                    function FGetStartSpeed:single;
                                    procedure FSetStartSpeed(val:single);
                                    function FGetEndSpeed:single;
                                    procedure FSetEndSpeed(val:single);
                                    function FGetDensity:single;
                                    procedure FSetDensity(val:single);
                                    function FGetDisplacement:single;
                                    procedure FSetDisplacement(val:single);
                                    function FGetDraft:single;
                                    procedure FSetDraft(val:single);
                                    function FGetDraftTotal:single;
                                    procedure FSetDraftTotal(val:single);
                                    function FGetLwl:single;
                                    procedure FSetLwl(val:single);
                                    function FGetLCB:single;
                                    procedure FSetLCB(val:single);
                                    function FGetKeelChordLength:single;
                                    procedure FSetKeelChordLength(val:single);
                                    function FGetKeelArea:single;
                                    procedure FSetKeelArea(val:single);
                                    function FGetRudderChordLength:single;
                                    procedure FSetRudderChordLength(val:single);
                                    function FGetRudderArea:single;
                                    procedure FSetRudderArea(val:single);
                                    function FGetBwl:single;
                                    procedure FSetBwl(val:single);
                                    function FGetCp:single;
                                    procedure FSetCp(val:single);
                                    function FGetViscosity:single;
                                    procedure FSetViscosity(val:single);
                                    function FGetWettedSurface:single;
                                    procedure FSetWettedSurface(val:single);
                                    function FGetWlArea:single;
                                    procedure FSetWlArea(val:single);
                                    function FGetStepSpeed:single;
                                    procedure FSetStepSpeed(val:single);
                                    function FGetExtractFromHull:boolean;
                                    procedure FSetExtractFromHull(Val:Boolean);
                                 public
                                    function CorrectInputdata:boolean;
                                    procedure Calculate;
                                    function Execute(Freeship:TFreeship;AutoExtract:Boolean):Boolean;
                                    property Bwl               : single read FGetBwl write FSetBwl;
                                    property Cp                : single read FGetCp write FSetCp;
                                    property Density           : single read FGetDensity write FSetDensity;
                                    property Displacement      : single read FGetDisplacement write FSetDisplacement;
                                    property Draft             : single read FGetDraft write FSetDraft;
                                    property DraftTotal        : single read FGetDraftTotal write FSetDraftTotal;
                                    property EndSpeed          : single read FGetEndSpeed write FSetEndSpeed;
                                    property ExtractFromHull   : boolean read FGetExtractFromHull write FSetExtractFromHull;
                                    property KeelChordLength   : single read FGetKeelChordLength write FSetKeelChordLength;
                                    property KeelArea          : single read FGetKeelArea write FSetKeelArea;
                                    property LCB               : single read FGetLCB write FSetLCB;
                                    property Lwl               : single read FGetLwl write FSetLwl;
                                    property RudderChordLength : single read FGetRudderChordLength write FSetRudderChordLength;
                                    property RudderArea        : single read FGetRudderArea write FSetRudderArea;
                                    property StartSpeed        : single read FGetStartSpeed write FSetStartSpeed;
                                    property StepSpeed         : single read FGetStepSpeed write FSetStepSpeed;
                                    property Viscosity         : single read FGetViscosity write FSetViscosity;
                                    property WettedSurface     : single read FGetWettedSurface write FSetWettedSurface;
                                    property WlArea            : single read FGetWlArea write FSetWlArea;
                              end;

var FreeResistance_Delft: TFreeResistance_Delft;

implementation

uses FreeLanguageSupport;

{$R *.DFM}

function TFreeResistance_Delft.CorrectInputdata:boolean;
begin
   Result:=False;
   if (Draft<=0) or (Lwl<=0) or (Bwl<=0) or (DraftTotal<=Draft) or
      (WettedSurface<=0) or (WlArea<=0) or (Displacement<=0) or
      (Cp<=0) or (Viscosity<=0) then exit;

   B_T:=Bwl/Draft;
   L_D:=Lwl/power(Displacement,1/3);
   A_D:=WlArea/power(Displacement,2/3);
   L_B:=Lwl/Bwl;
   Am:=Displacement/(Lwl*Cp);
   Cm:=Am/(Bwl*Draft);
   Cwp:=WlArea/(Lwl*Bwl);

   Result:=True;
end;{TFreeResistance_Delft.CorrectInputdata}

procedure TFreeResistance_Delft.Calculate;
var ConvertedSpeed   : single;
    FroudeNumber     : single;
    Rf,Rr,Rt         : single;
    index            : integer;
    CPopt,LCBopt     : array of single;
    Speed            : single;
    Line,Tmp         : string;
    Units            : TFreeUnitType;
    Stop             : Boolean;
begin
   Series1.Clear;
   Series2.Clear;
   Series3.Clear;
   Series4.Clear;
   ResultsMemo.Visible:=false;
   PrintButton.Enabled:=False;

   if CorrectInputdata then
   begin
      Units:=FFreeship.ProjectSettings.ProjectUnits;
      ResultsMemo.Text:='';
      FFreeship.CreateOutputHeader(Userstring(249)+'.',ResultsMemo.Lines);
      ResultsMemo.Lines.Add('----------------------------------------------------------------------------------');
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add(Userstring(250));
      ResultsMemo.Lines.Add('----------------------------------------------------------------------------------');
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add(Userstring(251));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(252),30)+' : '+FloatToStrF(StartSpeed,ffFixed,6,2)+' [kn]');
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(253),30)+' : '+FloatToStrF(EndSpeed,ffFixed,6,2)+' [kn]');
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(254),30)+' : '+FloatToStrF(StepSpeed,ffFixed,6,2)+' [kn]');
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(50),30)+' : '+FloatToStrF(Density,ffFixed,8,3)+#32+DensityStr(Units));
      if Units=fuImperial then ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(255),30)+' : '+FloatToStrF(Viscosity,ffFixed,8,4)+'*10-6 [ft2/s]')
                          else ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(255),30)+' : '+FloatToStrF(Viscosity,ffFixed,8,4)+'*10-6 [m2/s]');
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add(Userstring(256));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(17),30)+' : '+FloatToStrF(Lwl,ffFixed,6,3)+#32+LengthStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(18),30)+' : '+FloatToStrF(Bwl,ffFixed,6,3)+#32+LengthStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(257),30)+' : '+FloatToStrF(Draft,ffFixed,6,3)+#32+LengthStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(258),30)+' : '+FloatToStrF(DraftTotal,ffFixed,6,3)+#32+LengthStr(Units));
      if EstimateBox.Checked then ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(10),30)+' : '+FloatToStrF(WettedSurface,ffFixed,6,2)+#32+AreaStr(Units)+' ('+Userstring(266)+')')
                             else ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(10),30)+' : '+FloatToStrF(WettedSurface,ffFixed,6,2)+#32+AreaStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(19),30)+' : '+FloatToStrF(WlArea,ffFixed,6,2)+#32+AreaStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(4),30)+' : '+FloatToStrF(Displacement,ffFixed,6,3)+#32+VolStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(11),30)+' : '+FloatToStrF(LCB,ffFixed,6,3)+' [%]');
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(8),30)+' : '+FloatToStrF(Cp,ffFixed,6,4));
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add(Userstring(259));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(260),30)+' : '+FloatToStrF(KeelChordLength,ffFixed,6,3)+#32+LengthStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(261),30)+' : '+FloatToStrF(KeelArea,ffFixed,6,2)+#32+AreaStr(Units));
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add(Userstring(262));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(260),30)+' : '+FloatToStrF(RudderChordLength,ffFixed,6,2)+#32+LengthStr(Units));
      ResultsMemo.Lines.Add(Space(6)+Makelength(Userstring(261),30)+' : '+FloatToStrF(RudderArea,ffFixed,6,2)+#32+AreaStr(Units));
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add('');
      ResultsMemo.Lines.Add(Userstring(263));
      ResultsMemo.Lines.Add('----------------------------------------------------------------------------------');
      ResultsMemo.Lines.Add('');

      ResultsMemo.Lines.Add('Cwp           = '+FloatToStrF(Cwp,ffFixed,6,4));
      ResultsMemo.Lines.Add('Cm            = '+FloatToStrF(Cm,ffFixed,6,4));
      ResultsMemo.Lines.Add('Am            = '+FloatToStrF(Am,ffFixed,6,2)+#32+AreaStr(Units));
      ResultsMemo.Lines.Add('Lwl/Bwl       = '+FloatToStrF(Lwl/Bwl,ffFixed,6,3));
      ResultsMemo.Lines.Add('Bwl/Tc        = '+FloatToStrF(Bwl/Draft,ffFixed,6,3));
      ResultsMemo.Lines.Add('Lwl/Displ^3   = '+FloatToStrF(Lwl/Power(Displacement,1/3),ffFixed,6,3));
      ResultsMemo.Lines.Add('Tc/T          = '+FloatToStrF(Draft/DraftTotal,ffFixed,6,3));
      ResultsMemo.Lines.Add('Aw/Displ^0.67 = '+FloatToStrF(WlArea/Power(Displacement,2/3),ffFixed,7,3));
      ResultsMemo.Lines.Add('');
      if StepSpeed=0.0 then StepSpeed:=0.1;

      Stop:=False;
      if (L_B<2.76) or (L_B>5.00) then
      begin
         ResultsMemo.Lines.Add(Userstring(267)+' [2.76 - 5.00]');
         Stop:=True;
      end;
      if (B_T<2.46) or (B_T>19.32) then
      begin
         ResultsMemo.Lines.Add(Userstring(268)+' [2.46 - 19.32]');
         Stop:=True;
      end;
      if (L_D<4.34) or (L_D>8.50) then
      begin
         ResultsMemo.Lines.Add(Userstring(269)+' [4.34 - 8.50]');
         Stop:=True;
      end;
      if (LCB<-6) or (LCB>0) then
      begin
         ResultsMemo.Lines.Add(Userstring(270)+' [-6.00 - 0.00]');
         Stop:=True;
      end;
      if (Cp<0.52) or (Cp>0.60) then
      begin
         ResultsMemo.Lines.Add(Userstring(271)+' [0.52 - 0.60]');
         Stop:=True;
      end;
      if not stop then
      begin

         ResultsMemo.Lines.Add('');
         ResultsMemo.Lines.Add('');
         ResultsMemo.Lines.Add(Userstring(264));
         ResultsMemo.Lines.Add('+-------+-------+-------+---------+---------+-----------+---------+---------+---------+');
         ResultsMemo.Lines.Add('| Speed | Speed | Speed |   R_f   |   R_r   |    R_T    |  Power  | Cp opt. | LCB opt |');
         ResultsMemo.Lines.Add('|  [kn] | [m/s] |  [Fn] |   [N]   |   [N]   |    [N]    |   [kW]  |   [-]   |   [%]   | ');
         ResultsMemo.Lines.Add('+-------+-------+-------+---------+---------+-----------+---------+---------+---------+');
         Speed:=StartSpeed;
         Index:=0;
         Setlength(CpOpt,Trunc((EndSpeed-StartSpeed)/StepSpeed)+10);
         Setlength(LCBOpt,Trunc((EndSpeed-StartSpeed)/StepSpeed)+10);
         While Speed<=EndSpeed do
         begin
            ConvertedSpeed:=Speed*1852/3600;
            if FFreeship.ProjectSettings.ProjectUnits=fuImperial then FroudeNumber:=ConvertedSpeed/SQRT(9.81*Foot*Lwl)
                                                                 else FroudeNumber:=ConvertedSpeed/SQRT(9.81*Lwl);
            CalculateResistance(ConvertedSpeed,LCB,Cp,Rf,Rr,Rt);

            Str(Speed:6:2,Line);
            Line:='|'+Line+' |';
            Str(ConvertedSpeed:6:2,Tmp);
            Line:=Line+Tmp+' |';
            Str(FroudeNumber:6:3,Tmp);
            Line:=Line+Tmp+' |';
            Str(Rf:8:1,Tmp);
            Line:=Line+Tmp+' |';

            if ((FroudeNumber>=0) and (FroudeNumber<=0.45)) or
               ((FroudeNumber>0.475)  and (FroudeNumber<=0.75)) then
            begin
               Series1.AddXY(Speed,Rf,'',clTeeColor);
               Series2.AddXY(Speed,Rr,'',clTeeColor);
               Series3.AddXY(Speed,Rt,'',clTeeColor);
               Series4.AddXY(Speed,Rt*ConvertedSpeed*0.001,'',clTeeColor);
               inc(Index);
               CPopt[index]:=OptimumCP(ConvertedSpeed);
               LCBopt[index]:=OptimumLCB(ConvertedSpeed);
               Str(Rr:8:1,Tmp);
               Line:=Line+Tmp+' |';
               Str(Rt:10:1,Tmp);
               Line:=Line+Tmp+' |';
               Str(Rt*ConvertedSpeed*0.001:8:2,Tmp);
               Line:=Line+Tmp+' |';
               Str(CPopt[index]:8:3,Tmp);
               Line:=Line+Tmp+' |';
               Str(LCBopt[index]:8:3,Tmp);
               Line:=Line+Tmp+' |';
            end else Line:=Line+' ------- | --------- | ------- | ------- | ------- |';
            ResultsMemo.Lines.Add(Line);
            Speed:=Speed+StepSpeed;
         end;
         ResultsMemo.Lines.Add('+-------+-------+-------+---------+---------+-----------+---------+---------+---------+');
      end;
      ResultsMemo.Visible:=True;
      PrintButton.Enabled:=True;
   end;
end;{TFreeResistance_Delft.Calculate}

   function TFreeResistance_Delft.FGetStartSpeed:single;
begin
   Result:=StartSpeedbox.Value;
end;{TFreeResistance_Delft.FGetStartSpeed}

procedure TFreeResistance_Delft.FSetStartSpeed(val:single);
begin
   StartSpeedbox.Value:=Val;
end;{TFreeResistance_Delft.FSetStartSpeed}

function TFreeResistance_Delft.FGetEndSpeed:single;
begin
   Result:=EndSpeedbox.Value;
end;{TFreeResistance_Delft.FGetEndSpeed}

procedure TFreeResistance_Delft.FSetEndSpeed(val:single);
begin
   EndSpeedbox.Value:=Val;
end;{TFreeResistance_Delft.FSetEndSpeed}

function TFreeResistance_Delft.FGetDensity:single;
begin
   Result:=Densitybox.Value;
end;{TFreeResistance_Delft.FGetDensity}

procedure TFreeResistance_Delft.FSetDensity(val:single);
begin
   Densitybox.Value:=Val;

end;{TFreeResistance_Delft.FSetDensity}

function TFreeResistance_Delft.FGetDisplacement:single;
begin
   Result:=Displacementbox.Value;
end;{TFreeResistance_Delft.FGetDisplacement}

procedure TFreeResistance_Delft.FSetDisplacement(val:single);
begin
   Displacementbox.Value:=Val;
end;{TFreeResistance_Delft.FSetDisplacement}

function TFreeResistance_Delft.FGetDraft:single;
begin
   Result:=Draftbox.Value;
end;{TFreeResistance_Delft.FGetDraft}

procedure TFreeResistance_Delft.FSetDraft(val:single);
begin
   Draftbox.Value:=Val;
end;{TFreeResistance_Delft.FSetDraft}

function TFreeResistance_Delft.FGetDraftTotal:single;
begin
   Result:=DraftTotalbox.Value;
end;{TFreeResistance_Delft.FGetDraftTotal}

procedure TFreeResistance_Delft.FSetDraftTotal(val:single);
begin
   DraftTotalbox.Value:=Val;
end;{TFreeResistance_Delft.FSetDraftTotal}

function TFreeResistance_Delft.FGetLwl:single;
begin
   Result:=Lwlbox.Value;
end;{TFreeResistance_Delft.FGetLwl}

procedure TFreeResistance_Delft.FSetLwl(val:single);
begin
   Lwlbox.Value:=Val;
end;{TFreeResistance_Delft.FSetLwl}

function TFreeResistance_Delft.FGetLCB:single;
begin
   Result:=LCBbox.Value;
end;{TFreeResistance_Delft.FGetLCB}

procedure TFreeResistance_Delft.FSetLCB(val:single);
begin
   LCBbox.Value:=Val;
end;{TFreeResistance_Delft.FSetLCB}

function TFreeResistance_Delft.FGetKeelChordLength:single;
begin
   Result:=KeelChordLengthbox.Value;
end;{TFreeResistance_Delft.FGetKeelChordLength}

procedure TFreeResistance_Delft.FSetKeelChordLength(val:single);
begin
   KeelChordLengthbox.Value:=Val;
end;{TFreeResistance_Delft.FSetKeelChordLength}

function TFreeResistance_Delft.FGetKeelArea:single;
begin
   Result:=KeelAreabox.Value;
end;{TFreeResistance_Delft.FGetKeelArea}

procedure TFreeResistance_Delft.FSetKeelArea(val:single);
begin
   KeelAreabox.Value:=Val;
end;{TFreeResistance_Delft.FSetKeelArea}

function TFreeResistance_Delft.FGetRudderChordLength:single;
begin
   Result:=RudderChordLengthbox.Value;
end;{TFreeResistance_Delft.FGetRudderChordLength}

procedure TFreeResistance_Delft.FSetRudderChordLength(val:single);
begin
   RudderChordLengthbox.Value:=Val;
end;{TFreeResistance_Delft.FSetRudderChordLength}

function TFreeResistance_Delft.FGetRudderArea:single;
begin
   Result:=RudderAreabox.Value;
end;{TFreeResistance_Delft.FGetRudderArea}

procedure TFreeResistance_Delft.FSetRudderArea(val:single);
begin
   RudderAreabox.Value:=Val;
end;{TFreeResistance_Delft.FSetRudderArea}

function TFreeResistance_Delft.FGetBwl:single;
begin
   Result:=Bwlbox.Value;
end;{TFreeResistance_Delft.FGetBwl}

procedure TFreeResistance_Delft.FSetBwl(val:single);
begin
   Bwlbox.Value:=Val;
end;{TFreeResistance_Delft.FSetBwl}

function TFreeResistance_Delft.FGetCp:single;
begin
   Result:=Cpbox.Value;
end;{TFreeResistance_Delft.FGetCp}

procedure TFreeResistance_Delft.FSetCp(val:single);
begin
   Cpbox.Value:=Val;
end;{TFreeResistance_Delft.FSetCp}

function TFreeResistance_Delft.FGetViscosity:single;
begin
   Result:=Viscositybox.Value;
end;{TFreeResistance_Delft.FGetViscosity}

procedure TFreeResistance_Delft.FSetViscosity(val:single);
begin
   Viscositybox.Value:=Val;
end;{TFreeResistance_Delft.FSetViscosity}

function TFreeResistance_Delft.FGetWettedSurface:single;
var C23,ScbFact,Am,Cm,Cwp:single;
begin
   if EstimateBox.Checked then
   begin
      if (Draft>0) and (Lwl>0) and (Bwl>0) and (Cp>0) and (Displacement>0) then
      begin
         Am:=Displacement/(Lwl*Cp);
         Cm:=Am/(Bwl*Draft);
         Cwp:=WlArea/(Lwl*Bwl);
         C23:=0.453+0.443*(Cp*Cm)-0.286*Cm-0.00347*(Bwl/Draft)+0.37*Cwp;
         ScbFact:=0.616*C23+0.111*Cm*Cm*Cm+0.245*(C23/Cm)-0.0228;
         Result:=ScbFact*Lwl*(2*Draft+Bwl)*sqrt(Cm);
      end else Result:=0;
   end else Result:=WettedSurfacebox.Value;
end;{TFreeResistance_Delft.FGetWettedSurface}

procedure TFreeResistance_Delft.FSetWettedSurface(val:single);
begin
   WettedSurfacebox.Value:=Val;
end;{TFreeResistance_Delft.FSetWettedSurface}

function TFreeResistance_Delft.FGetWlArea:single;
begin
   Result:=WlAreabox.Value;
end;{TFreeResistance_Delft.FGetWlArea}

procedure TFreeResistance_Delft.FSetWlArea(val:single);
begin
   WlAreabox.Value:=Val;
end;{TFreeResistance_Delft.FSetWlArea}

function TFreeResistance_Delft.FGetStepSpeed:single;
begin
   Result:=StepSpeedbox.Value;
end;{TFreeResistance_Delft.FGetStepSpeed}

procedure TFreeResistance_Delft.FSetStepSpeed(val:single);
begin
   StepSpeedbox.Value:=Val;
end;{TFreeResistance_Delft.FSetStepSpeed}

function TFreeResistance_Delft.FGetExtractFromHull:boolean;
begin
   Result:=CheckBox2.Checked;
end;{TFreeResistance_Delft.FGetExtractFromHull}

procedure TFreeResistance_Delft.FSetExtractFromHull(Val:Boolean);
begin
   if Checkbox2.Checked<>val then Checkbox2.Checked:=Val;
end;{TFreeResistance_Delft.FSetExtractFromHullVal}

function TFreeResistance_Delft.Execute(Freeship:TFreeship;AutoExtract:Boolean):Boolean;
var Units : TFreeUnitType;
begin
   FFreeship:=Freeship;
   Chart.Title.Text.Text:=Userstring(265);
   Chart.LeftAxis.Title.Caption:=Userstring(272)+' [N]';
   Chart.BottomAxis.Title.Caption:=Userstring(273)+' [kn]';
   Units:=FFreeship.ProjectSettings.ProjectUnits;
   DensityBox.Enabled:=False;
   ViscosityBox.Enabled:=False;
   Checkbox2.Enabled:=FFreeship.Surface.NumberOfControlFaces>1;
   Checkbox2.Checked:=AutoExtract;
   if Checkbox2.Checked then CheckBox2Click(self);
   Label34.Caption:=DensityStr(Units);
   _Label8.Caption:=LengthStr(Units);
   _Label9.Caption:=LengthStr(Units);
   _Label10.Caption:=LengthStr(Units);
   _Label11.Caption:=LengthStr(Units);
   _Label12.Caption:=AreaStr(Units);
   _Label14.Caption:=AreaStr(Units);
   _Label15.Caption:=VolStr(Units);
   _Label16.Caption:='[%]';
   _Label17.Caption:=LengthStr(Units);
   _Label18.Caption:=AreaStr(Units);
   _Label19.Caption:=LengthStr(Units);
   _Label30.Caption:=AreaStr(Units);
   // Skip translation
   if Units=fuMetric then _Label36.Caption:='*10-6  [m2/s]'
                     else _Label36.Caption:='*10-6  [ft2/s]';
   // End Skip translation
   Viscosity:=FindWaterViscosity(Density,Units);
   Calculate;
   ShowModal;
   Result:=ModalResult=mrOk;
end;{TFreeResistance_Delft.Execute}

procedure TFreeResistance_Delft.CalculateResistance(ConvertedSpeed,LCB,Cp:single;var Rf,Rr,Rt: single);
var Rn_hull,Cf_hull,Rf_hull       : single;
    Rn_keel,Cf_keel,Rf_keel       : single;
    Rn_rudder,Cf_Rudder,Rf_rudder : single;
    fraction,FroudeNumber         : single;
    factors                       : array[0..9] of extended;
    a,Lower                       : integer;

    Keel_chord,Keel_Area          : TFloatType;
    Rudder_chord,Rudder_Area      : TFloatType;
    LengthWaterline               : TFloatType;
    WaterDensity                  : TFloatType;
    WaterViscosity                : TFloatType;
    Displ                         : TFloatType;
    WetArea                       : TFloatType;
begin
   Rf:=0.0;
   Rr:=0.0;
   Rt:=0.0;
   if Convertedspeed<=0 then exit;

   if FFreeship.ProjectSettings.ProjectUnits=fuImperial then
   begin
      LengthWaterline:=Lwl*Foot;
      WaterDensity:=Density/WeightConversionFactor;
      Keel_chord:=KeelChordLength*Foot;
      Keel_Area:=KeelArea*Foot*Foot;
      Rudder_chord:=RudderChordLength*Foot;
      Rudder_Area:=RudderArea*Foot*Foot;
      WaterViscosity:=Viscosity*0.3048*0.3048*1e-6;
      Displ:=Displacement*foot*foot*foot;
      WetArea:=WettedSurface*Foot*Foot;
   end else
   begin
      LengthWaterline:=Lwl;
      WaterDensity:=Density;
      Keel_chord:=KeelChordLength;
      Keel_Area:=KeelArea;
      Rudder_chord:=RudderChordLength;
      Rudder_Area:=RudderArea;
      WaterViscosity:=Viscosity*1e-6;
      Displ:=Displacement;
      WetArea:=WettedSurface;
   end;


   FroudeNumber:=ConvertedSpeed/SQRT(9.81*LengthWaterline);
   Rn_hull:=(ConvertedSpeed*0.7*LengthWaterline)/WaterViscosity;
   Cf_hull:=0.075/sqr(LOG10(Rn_hull)-2);
   Rf_hull:=Cf_hull*0.5*1000*WaterDensity*sqr(ConvertedSpeed)*WetArea;
   if (Keel_Chord>0) and (Keel_Area>0) then
   begin
      Rn_keel:=(ConvertedSpeed*Keel_Chord)/WaterViscosity;
      Cf_keel:=0.075/sqr(LOG10(Rn_Keel)-2);
      Rf_keel:=Cf_keel*0.5*1000*WaterDensity*sqr(ConvertedSpeed)*Keel_Area;
   end else Rf_keel:=0.0;

   if (Rudder_chord>0) and (Rudder_Area>0) then
   begin
      Rn_Rudder:=(ConvertedSpeed*Rudder_chord)/WaterViscosity;
      Cf_Rudder:=0.075/sqr(LOG10(Rn_Rudder)-2);
      Rf_Rudder:=Cf_Rudder*0.5*1000*WaterDensity*sqr(ConvertedSpeed)*Rudder_Area;
   end else Rf_Rudder:=0.0;

   Rf:=Rf_hull+Rf_keel+Rf_rudder;
   Rr:=0;
   if FroudeNumber<=0.45 then
   begin
      if FroudeNumber<0.125 then
      begin
         Fraction:=FroudeNumber/0.125;
         for a:=0 to 9 do factors[a]:=fraction*(Matrix1[0,a]);

      end else
      begin
         Lower:=trunc((FroudeNumber-0.125)/0.025);
         Fraction:=(FroudeNumber-(0.125+Lower*0.025))/0.025;
         for a:=0 to 9 do factors[a]:=Matrix1[Lower,a]+fraction*(Matrix1[Lower+1,a]-Matrix1[Lower,a]);
      end;
      Rr:=9.81*Displ*WaterDensity*
          (factors[0]+
           factors[1]*Cp+
           factors[2]*LCB+
           factors[3]*B_T+
           factors[4]*L_D+
           factors[5]*Cp*Cp+
           factors[6]*Cp*L_D+
           factors[7]*LCB*LCB+
           factors[8]*L_D*L_D+
           factors[9]*L_D*L_D*L_D);
   end else if (FroudeNumber>0.475) and (FroudeNumber<=0.75) then
   begin
      Lower:=trunc((FroudeNumber-0.475)/0.025);
      Fraction:=(FroudeNumber-(0.475+Lower*0.025))/0.025;
      for a:=0 to 5 do factors[a]:=Matrix2[Lower,a]+fraction*(Matrix2[Lower+1,a]-Matrix2[Lower,a]);
      Rr:=Displ*WaterDensity*
          (factors[0]+
           factors[1]*L_B+
           factors[2]*A_D+
           factors[3]*LCB+
           factors[4]*L_B*L_B+
           factors[5]*L_B*A_D*A_D*A_D);
   end;
   Rt:=Rf+Rr;
end;{TFreeResistance_Delft.CalculateResistance}

function TFreeResistance_Delft.OptimumCP(Speed:single):single;
var TmpCp    : single;
    RMin     : single;
    Rf,Rr,Rt : single;
    Optimum  : single;
begin
   result:=0;
   exit;
   RMin:=1e1000;
   TmpCp:=0.52;
   Optimum:=Cp;
   while TmpCp<=0.6 do
   begin
      CalculateResistance(Speed,LCB,TmpCp,Rf,Rr,Rt);
      if Rt<Rmin then
      begin
         Rmin:=Rt;
         Optimum:=TmpCp;
      end;
      TmpCp:=TmpCp+0.001;
   end;
   Result:=Optimum;
end;{TFreeResistance_Delft.OptimumCP}

function TFreeResistance_Delft.OptimumLCB(Speed:single):single;
var TmpLCB   : single;
    RMin     : single;
    Rf,Rr,Rt : single;
    Optimum  : single;
begin
   result:=0;
   exit;
   RMin:=1e1000;
   TmpLCB:=-6;
   Optimum:=TmpLCB;
   while TmpLCB<=0 do
   begin
      CalculateResistance(Speed,TmpLCB,Cp,Rf,Rr,Rt);
      if Rt<Rmin then
      begin
         Rmin:=Rt;
         Optimum:=TmpLCB;
      end;
      TmpLCB:=TmpLCB+0.001;
   end;
   Result:=Optimum;
end;{TFreeResistance_Delft.OptimumLCB}

procedure TFreeResistance_Delft.CheckBox2Click(Sender: TObject);
begin
   LwlBox.Enabled:=not Checkbox2.Checked;
   BwlBox.Enabled:=not Checkbox2.Checked;
   WettedSurfaceBox.Enabled:=not Checkbox2.Checked;
   WlAreaBox.Enabled:=not Checkbox2.Checked;
   DisplacementBox.Enabled:=not Checkbox2.Checked;
   LCBBox.Enabled:=not Checkbox2.Checked;
   CpBox.Enabled:=not Checkbox2.Checked;
   EstimateBox.Enabled:=not Checkbox2.Checked;
   if CheckBox2.Checked then
   begin
      EstimateBox.Checked:=false;
      if DraftTotal=0.0 then DraftTotal:=FFreeship.ProjectSettings.ProjectDraft;
      if FFreeship<>nil then Density:=FFreeship.ProjectSettings.ProjectWaterDensity;
      DraftTotalBoxAfterSetValue(self);
   end;
end;{TFreeResistance_Delft.CheckBox2Click}

procedure TFreeResistance_Delft.ToolButton25Click(Sender: TObject);
begin
   ModalResult:=mrOk;
end;{TFreeResistance_Delft.ToolButton25Click}

procedure TFreeResistance_Delft.ToolButton7Click(Sender: TObject);
begin
   ModalResult:=mrcancel;
end;{TFreeResistance_Delft.ToolButton7Click}

procedure TFreeResistance_Delft.ToolButton20Click(Sender: TObject);
begin
   Calculate;
end;{TFreeResistance_Delft.ToolButton20Click}

procedure TFreeResistance_Delft.PrintButtonClick(Sender: TObject);
var Line      : Integer;
    PrintText : TextFile;
begin
  if PrintDialog.Execute then
  begin
    AssignPrn(PrintText);
    Rewrite(PrintText);
    Printer.Canvas.Font.Assign(ResultsMemo.Font);
    for Line := 0 to ResultsMemo.Lines.Count - 1 do
      Writeln(PrintText, ResultsMemo.Lines[Line]);
    CloseFile(PrintText);
  end;
end;{TFreeResistance_Delft.ToolButton1Click}

procedure TFreeResistance_Delft.DraftTotalBoxAfterSetValue(Sender: TObject);
var HydObject  : TFreeHydrostaticCalc;
begin
   if (CheckBox2.Checked) and (FFreeship<>nil) then
   begin
      HydObject:=TFreeHydrostaticCalc.Create(FFreeship);
      HydObject.Draft:=DraftTotal;
      HydObject.Calculate;

      Lwl:=HydObject.Data.LengthWaterline;
      Bwl:=HydObject.Data.BeamWaterline;
      WettedSurface:=HydObject.Data.WettedSurface;
      WlArea:=HydObject.Data.Waterplanearea;
      Displacement:=HydObject.Data.Volume;
      if Lwl<>0 then LCB:=100*(HydObject.Data.CenterOfBuoyancy.X-(HydObject.Data.WlMin.X+0.5*Lwl))/Lwl
                else LCB:=0;
      Cp:=HydObject.Data.PrismCoefficient;
      HydObject.Destroy;
   end;
   Calculate;
end;{TFreeResistance_Delft.DraftTotalBoxAfterSetValue}

procedure TFreeResistance_Delft.StartSpeedBoxAfterSetValue(Sender: TObject);
begin
   Calculate;
end;{TFreeResistance_Delft.StartSpeedBoxAfterSetValue}

procedure TFreeResistance_Delft.LwlBoxAfterSetValue(Sender: TObject);
begin
   Calculate;
end;{TFreeResistance_Delft.LwlBoxAfterSetValue}

procedure TFreeResistance_Delft.KeelChordLengthboxAfterSetValue(Sender: TObject);
begin
   Calculate;
end;{TFreeResistance_Delft.KeelChordLengthboxAfterSetValue}

procedure TFreeResistance_Delft.EstimateBoxClick(Sender: TObject);
begin
   WettedSurfaceBox.Enabled:=not EstimateBox.Checked;
   Calculate;
end;{TFreeResistance_Delft.EstimateBoxClick}

end.
