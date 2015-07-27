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

unit FreeKeelWizardDlg;

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
     Buttons,
     ExtCtrls,
     StdCtrls,
     FreeshipUnit,
     FreeGeometry,
     FasterList,
     CheckLst,
     Math,
     FreeNumInput,
     ComCtrls,
     TeEngine,
     Series,
     TeeProcs,
     Chart;

const Nh           = 26;
      NacaProfiles : array[0..25,1..Nh] of double =((0.0, 0.00500, 0.00750, 0.01250, 0.02500, 0.05000, 0.07500, 0.10000, 0.15000, 0.20000, 0.25000, 0.30000, 0.35000, 0.40000, 0.45000, 0.50000, 0.55000, 0.60000, 0.65000, 0.70000, 0.75000, 0.80000, 0.85000, 0.90000, 0.95000, 1.0),  // First row contains X-coordinates
                                                    (0.0, 0.00503, 0.00609, 0.00771, 0.01057, 0.01462, 0.01766, 0.02010, 0.02386, 0.02656, 0.02841, 0.02954, 0.03000, 0.02971, 0.02877, 0.02723, 0.02517, 0.02267, 0.01982, 0.01670, 0.01342, 0.01008, 0.00683, 0.00383, 0.00138, 0.0),  // 63-006
                                                    (0.0, 0.00749, 0.00906, 0.01151, 0.01582, 0.02196, 0.02655, 0.03024, 0.03591, 0.03997, 0.04275, 0.04442, 0.04500, 0.04447, 0.04296, 0.04056, 0.03739, 0.03358, 0.02928, 0.02458, 0.01966, 0.01471, 0.00990, 0.00550, 0.00196, 0.0),  // 63-009
                                                    (0.0, 0.00829, 0.01004, 0.01275, 0.01756, 0.02440, 0.02950, 0.03362, 0.03994, 0.04445, 0.04753, 0.04938, 0.05000, 0.04938, 0.04766, 0.04496, 0.04140, 0.03715, 0.03234, 0.02712, 0.02166, 0.01618, 0.01088, 0.00604, 0.00214, 0.0),  // 63-010
                                                    (0.0, 0.00985, 0.01194, 0.01519, 0.02102, 0.02925, 0.03542, 0.04039, 0.04799, 0.05342, 0.05712, 0.05930, 0.06000, 0.05920, 0.05704, 0.05370, 0.04935, 0.04420, 0.03840, 0.03210, 0.02556, 0.01902, 0.01274, 0.00707, 0.00250, 0.0),  // 63-012
                                                    (0.0, 0.01204, 0.01462, 0.01878, 0.02610, 0.03648, 0.04427, 0.05055, 0.06011, 0.06693, 0.07155, 0.07421, 0.07500, 0.07386, 0.07099, 0.06665, 0.06108, 0.05453, 0.04721, 0.03934, 0.03119, 0.02310, 0.01541, 0.00852, 0.00300, 0.0),  // 63-015
                                                    (0.0, 0.01404, 0.01713, 0.02217, 0.03104, 0.04362, 0.05308, 0.06068, 0.07225, 0.08048, 0.08600, 0.08913, 0.09000, 0.08845, 0.08482, 0.07942, 0.07256, 0.06455, 0.05567, 0.04622, 0.03650, 0.02691, 0.01787, 0.00985, 0.00348, 0.0),  // 63-018
                                                    (0.0, 0.01583, 0.01937, 0.02527, 0.03577, 0.05065, 0.06182, 0.07080, 0.08441, 0.09410, 0.10053, 0.10412, 0.10500, 0.10298, 0.09854, 0.09206, 0.08390, 0.07441, 0.06396, 0.05290, 0.04160, 0.03054, 0.02021, 0.01113, 0.00392, 0.0),  // 63-021
                                                    (0.0, 0.00494, 0.00596, 0.00754, 0.01024, 0.01405, 0.01692, 0.01928, 0.02298, 0.02572, 0.02772, 0.02907, 0.02981, 0.02995, 0.02919, 0.02775, 0.02575, 0.02331, 0.02050, 0.01740, 0.01412, 0.01072, 0.00737, 0.00423, 0.00157, 0.0),  // 64-006
                                                    (0.0, 0.00658, 0.00794, 0.01005, 0.01365, 0.01875, 0.02259, 0.02574, 0.03069, 0.03437, 0.03704, 0.03884, 0.03979, 0.03992, 0.03883, 0.03684, 0.03411, 0.03081, 0.02704, 0.02291, 0.01854, 0.01404, 0.00961, 0.00550, 0.00206, 0.0),  // 64-008
                                                    (0.0, 0.00739, 0.00892, 0.01128, 0.01533, 0.02109, 0.02543, 0.02898, 0.03455, 0.03868, 0.04170, 0.04373, 0.04479, 0.04490, 0.04364, 0.04136, 0.03826, 0.03452, 0.03026, 0.02561, 0.02069, 0.01564, 0.01069, 0.00611, 0.00227, 0.0),  // 64-009
                                                    (0.0, 0.00904, 0.00969, 0.01225, 0.01688, 0.02327, 0.02805, 0.03190, 0.03813, 0.04272, 0.04606, 0.04837, 0.04968, 0.04995, 0.04894, 0.04684, 0.04388, 0.04021, 0.03597, 0.03127, 0.02623, 0.02103, 0.01582, 0.01062, 0.00541, 0.0),  // 64-010
                                                    (0.0, 0.00978, 0.01179, 0.01490, 0.02035, 0.02810, 0.03394, 0.03871, 0.04620, 0.05173, 0.05576, 0.05844, 0.05978, 0.05981, 0.05798, 0.05480, 0.05056, 0.04548, 0.03974, 0.03350, 0.02695, 0.02029, 0.01382, 0.00786, 0.00288, 0.0),  // 64-012
                                                    (0.0, 0.01208, 0.01456, 0.01842, 0.02528, 0.03504, 0.04240, 0.04842, 0.05785, 0.06480, 0.06985, 0.07319, 0.07482, 0.07473, 0.07224, 0.06810, 0.06266, 0.05620, 0.04895, 0.04113, 0.03296, 0.02472, 0.01677, 0.00950, 0.00346, 0.0),  // 64-015
                                                    (0.0, 0.01428, 0.01720, 0.02177, 0.03005, 0.04186, 0.05076, 0.05803, 0.06942, 0.07782, 0.08391, 0.08789, 0.08979, 0.08952, 0.08630, 0.08114, 0.07445, 0.06658, 0.05782, 0.04842, 0.03866, 0.02888, 0.01951, 0.01101, 0.00400, 0.0),  // 64-018
                                                    (0.0, 0.01646, 0.01985, 0.02517, 0.03485, 0.04871, 0.05915, 0.06769, 0.08108, 0.09095, 0.09807, 0.10269, 0.10481, 0.10431, 0.10030, 0.09404, 0.08607, 0.07678, 0.06649, 0.05549, 0.04416, 0.03287, 0.02213, 0.01245, 0.00449, 0.0),  // 64-021
                                                    (0.0, 0.00476, 0.00574, 0.00717, 0.00956, 0.01310, 0.01589, 0.01824, 0.02197, 0.02482, 0.02697, 0.02852, 0.02952, 0.02998, 0.02983, 0.02900, 0.02741, 0.02518, 0.02246, 0.01935, 0.01594, 0.01233, 0.00865, 0.00510, 0.00195, 0.0),  // 65-006
                                                    (0.0, 0.00627, 0.00756, 0.00945, 0.01267, 0.01745, 0.02118, 0.02432, 0.02931, 0.03312, 0.03599, 0.03805, 0.03938, 0.03998, 0.03974, 0.03857, 0.03638, 0.03337, 0.02971, 0.02553, 0.02096, 0.01617, 0.01131, 0.00664, 0.00252, 0.0),  // 65-008
                                                    (0.0, 0.00700, 0.00845, 0.01058, 0.01421, 0.01961, 0.02383, 0.02736, 0.03299, 0.03727, 0.04050, 0.04282, 0.04431, 0.04496, 0.04469, 0.04336, 0.04086, 0.03743, 0.03328, 0.02856, 0.02342, 0.01805, 0.01260, 0.00738, 0.00280, 0.0),  // 65-009
                                                    (0.0, 0.00772, 0.00932, 0.01169, 0.01574, 0.02177, 0.02647, 0.03040, 0.03666, 0.04143, 0.04503, 0.04760, 0.04924, 0.04996, 0.04963, 0.04812, 0.04530, 0.04146, 0.03682, 0.03156, 0.02584, 0.01987, 0.01385, 0.00810, 0.00306, 0.0),  // 65-010
                                                    (0.0, 0.00923, 0.01109, 0.01387, 0.01875, 0.02606, 0.03172, 0.03647, 0.04402, 0.04975, 0.05406, 0.05716, 0.05912, 0.05997, 0.05949, 0.05757, 0.05412, 0.04943, 0.04381, 0.03743, 0.03059, 0.02345, 0.01630, 0.00947, 0.00356, 0.0),  // 65-012
                                                    (0.0, 0.01124, 0.01356, 0.01702, 0.02324, 0.03245, 0.03959, 0.04555, 0.05504, 0.06223, 0.06764, 0.07152, 0.07396, 0.07498, 0.07427, 0.07168, 0.06720, 0.06118, 0.05403, 0.04600, 0.03744, 0.02858, 0.01977, 0.01144, 0.00428, 0.0),  // 65-015
                                                    (0.0, 0.01337, 0.01608, 0.02014, 0.02751, 0.03866, 0.04733, 0.05457, 0.06606, 0.07476, 0.08129, 0.08595, 0.08886, 0.08999, 0.08901, 0.08568, 0.08008, 0.07267, 0.06395, 0.05426, 0.04396, 0.03338, 0.02295, 0.01319, 0.00490, 0.0),  // 65-018
                                                    (0.0, 0.01522, 0.01838, 0.02301, 0.03154, 0.04472, 0.05498, 0.06352, 0.07700, 0.08720, 0.09487, 0.10036, 0.10375, 0.10499, 0.10366, 0.09952, 0.09277, 0.08390, 0.07360, 0.06224, 0.05024, 0.03800, 0.02598, 0.01484, 0.00546, 0.0),  // 65-021
                                                    (0.0, 0.00687, 0.00824, 0.01030, 0.01368, 0.01880, 0.02283, 0.02626, 0.03178, 0.03601, 0.03927, 0.04173, 0.04348, 0.04457, 0.04499, 0.04475, 0.04381, 0.04204, 0.03882, 0.03428, 0.02877, 0.02263, 0.01611, 0.00961, 0.00374, 0.0),  // 66-009
                                                    (0.0, 0.00759, 0.00913, 0.01141, 0.01516, 0.02087, 0.02536, 0.02917, 0.03530, 0.04001, 0.04363, 0.04636, 0.04832, 0.04953, 0.05000, 0.04971, 0.04865, 0.04665, 0.04302, 0.03787, 0.03176, 0.02494, 0.01773, 0.01054, 0.00408, 0.0)); // 65-010


type TFreeKeelWizardDialog  = class(TForm)
                                 Panel1: TPanel;
                                 Panel3: TPanel;
                                 BitBtn1: TSpeedButton;
                                 BitBtn2: TSpeedButton;
                                 ComboBox: TComboBox;
                                 Label1: TLabel;
                                 Label2: TLabel;
                                 ComboBox1: TComboBox;
                                 Label3: TLabel;
                                 Label4: TLabel;
                                 Label5: TLabel;
                                 Input1: TFreeNumInput;
                                 Input2: TFreeNumInput;
                                 Input3: TFreeNumInput;
                                 Label6: TLabel;
                                 Input4: TFreeNumInput;
                                 GroupBox1: TGroupBox;
                                 Label7: TLabel;
    _Label8: TLabel;
                                 Label9: TLabel;
    _Label10: TLabel;
                                 Label11: TLabel;
    _Label12: TLabel;
                                 Label13: TLabel;
    _Label14: TLabel;
                                 Label15: TLabel;
    _Label16: TLabel;
                                 Label17: TLabel;
    _ComboBox2: TComboBox;
                                 Label18: TLabel;
    _Label19: TLabel;
                                 Label20: TLabel;
    _Label21: TLabel;
                                 Label22: TLabel;
    _Label23: TLabel;
                                 SpeedButton1: TSpeedButton;
                                 Label24: TLabel;
    _Label25: TLabel;
                                 Label26: TLabel;
                                 Label27: TLabel;
                                 Input5: TFreeNumInput;
                                 Input6: TFreeNumInput;
                                 TrackBar1: TTrackBar;
                                 Label28: TLabel;
                                 PageControl: TPageControl;
                                 TabSheet1: TTabSheet;
                                 TabSheet2: TTabSheet;
                                 Viewport: TFreeViewport;
                                 Chart: TChart;
                                 Series1: TLineSeries;
                                 Series2: TLineSeries;
                                 SpeedButton2: TSpeedButton;
                                 procedure BitBtn1Click(Sender: TObject);
                                 procedure BitBtn2Click(Sender: TObject);
                                 procedure ComboBox1Click(Sender: TObject);
                                 procedure Input1AfterSetValue(Sender: TObject);
                                 procedure ViewportRequestExtents(Sender: TObject; var Min,Max: T3DCoordinate);
                                 procedure ViewportRedraw(Sender: TObject);
                                 procedure SpeedButton1Click(Sender: TObject);
                                 procedure Input5AfterSetValue(Sender: TObject);
                                 procedure Input6AfterSetValue(Sender: TObject);
                                 procedure TrackBar1Change(Sender: TObject);
                                 procedure SpeedButton2Click(Sender: TObject);
                              private   { Private declarations }
                                 FFreeship         : TFreeShip;
                                 FProfile          : TFreeSpline;
                                 COG               : T2DCoordinate;
                                 Span              : TFloatType;
                                 Area,MeanChord    : TFloatType;
                                 GeomAspectRatio   : TFloatType;
                                 EffAspectRatio    : TFloatType;
                                 Volume            : TFloatType;
                                 VolCOG            : T3DCoordinate;
                                 WettedArea        : TFloatType;
                                 function FGetCols:Integer;
                                 function FGetRows:Integer;
                                 procedure UpdateData;
                              public    { Public declarations }
                                 Mesh : array of array of T3DCoordinate;
                                 function Execute(Freeship:TFreeShip):Boolean;
                                 procedure SendToSurface(Surface:TFreeSubdivisionsurface);
                                 property Cols  : Integer read FGetCols;
                                 property Rows  : Integer read FGetRows;
                           end;

var FreeKeelWizardDialog: TFreeKeelWizardDialog;

implementation

{$R *.dfm}

function TFreeKeelWizardDialog.FGetCols:Integer;
begin
   Result:=Input6.AsInteger;
   if Result<3 then Result:=3;
end;{TFreeKeelWizardDialog.FGetCols}

function TFreeKeelWizardDialog.FGetRows:Integer;
begin
   Result:=Input5.AsInteger;
   if Result<2 then Result:=2;
end;{TFreeKeelWizardDialog.FGetRows}

procedure TFreeKeelWizardDialog.UpdateData;
const Cl2D=0.10;
var L,a,b      : TFloatType;
    I,J,Index  : Integer;
    VertInd    : Double;
    P          : T3DCoordinate;
    P_1,P_2    : T3DCoordinate;
    P_3,P_4    : T3DCoordinate;
    Angle      : TFloatType;
    Height     : TFloatType;
    Chord      : TFloatType;
    Start      : TFloatType;
    DeltaA     : TFloatType;
    MaxY       : TFloatType;
    P1,P2      : T2DCoordinate;
    Results    : TFreeIntersectionData;
    Plane      : T3DPlane;
    Spline     : TFreeSpline;
    VertDist   : Double;
    Factor,Cl,Cd:double;

    procedure ProcessTriangle(P1,P2,P3:T3DCoordinate);
    var VolumeMoment : T3DCoordinate;
        Vol          : TFloatType;
        Center       : T3DCoordinate;
        ax,ay,az     : TFloatType;
    begin
       Center.X:=(P1.X+P2.X+P3.X)/3;
       Center.Y:=(P1.Y+P2.Y+P3.Y)/3;
       Center.Z:=(P1.Z+P2.Z+P3.Z)/3;
       Vol:=((P1.z)*(P2.x*P3.y-P2.y*P3.x)+
             (P1.y)*(P2.z*P3.x-P2.x*P3.z)+
             (P1.x)*(P2.y*P3.z-P2.z*P3.y))/6;
       if Vol<>0 then
       begin
          VolumeMoment.X:=0.75*Center.X*Vol;
          VolumeMoment.Y:=0.75*Center.Y*Vol;
          VolumeMoment.Z:=0.75*Center.Z*Vol;
          Volume:=Volume+Vol;
          VolCOG.X:=VolCOG.X+VolumeMoment.X;
          VolCOG.Y:=VolCOG.Y+VolumeMoment.Y;
          VolCOG.Z:=VolCOG.Z+VolumeMoment.Z;
       end;
       ax:=0.5*((P1.y-P2.y)*(P1.z+P2.z)+(P2.y-P3.y)*(P2.z+P3.z)+
                (P3.y-P1.y)*(P3.z+P1.z));
       ay:=0.5*((P1.z-P2.z)*(P1.x+P2.x)+(P2.z-P3.z)*(P2.x+P3.x)+
                (P3.z-P1.z)*(P3.x+P1.x));
       az:=0.5*((P1.x-P2.x)*(P1.y+P2.y)+(P2.x-P3.x)*(P2.y+P3.y)+
                (P3.x-P1.x)*(P3.y+P1.y));
       WettedArea:=WettedArea+Sqrt(ax*ax+ay*ay+az*az);
    end;{ProcessTriangle}

begin
   FProfile.Clear;
   FProfile.Color:=clBlack;
   FProfile.Fragments:=500;

   _label8.Caption:='';
   _label10.Caption:='';
   _label12.Caption:='';
   _label14.Caption:='';
   _label16.Caption:='';
   _label19.Caption:='';
   _label21.Caption:='';
   _label23.Caption:='';
   _label25.Caption:='';

   Series1.Clear;
   Series2.Clear;

   Factor:=1+Trackbar1.Position/Trackbar1.Max;

   if Combobox1.ItemIndex=0 then
   begin
      FProfile.Add(SetPoint(0,0,0));
      FProfile.Add(SetPoint(Input4.Value,0,-Input3.Value));
      FProfile.Knuckle[FProfile.NumberOfPoints-1]:=True;
      FProfile.Add(SetPoint(Input4.Value+Input2.Value,0,-Input3.Value));
      FProfile.Knuckle[FProfile.NumberOfPoints-1]:=True;
      FProfile.Add(SetPoint(Input1.Value,0,0));
      FProfile.Knuckle[FProfile.NumberOfPoints-1]:=True;
      FProfile.Add(FProfile.Point[0]);
   end else
   begin
      L:=Input3.Value;
      Case combobox1.ItemIndex of
         1 : a:=0.50*input1.Value;
         2 : a:=0.0;
         3 : a:=0.75*input1.Value;
         else a:=0.0;
      end;
      b:=Input1.Value-a;
      if a>0 then
      begin
         for I:=0 to 10 do
         begin
            Angle:=I*0.1*90;
            P.X:=a-a*cos(DegToRad(Angle));
            P.Y:=0.0;
            P.Z:=-L*sin(DegToRad(Angle));
            FProfile.Add(P);
         end;
      end else
      begin
         FProfile.Add(SetPoint(0,0,0));
         FProfile.Add(SetPoint(0,0,-L));
         FProfile.Knuckle[FProfile.NumberOfPoints-1]:=True;
      end;
      for I:=1 to 10 do
      begin
         Angle:=I*0.1*90;
         P.X:=a+b*sin(DegToRad(Angle));
         P.Y:=0.0;
         P.Z:=-L*cos(DegToRad(Angle));
         FProfile.Add(P);
      end;
      FProfile.Knuckle[FProfile.NumberOfPoints-1]:=True;
      FProfile.Add(FProfile.Point[0]);
   end;

   Area:=0;
   COG.X:=0.0;
   COG.Y:=0.0;
   MeanChord:=0.0;
   Span:=Input3.Value;
   GeomAspectRatio:=0.0;
   EffAspectRatio:=0.0;
   Volume:=0.0;
   Fillchar(VolCOG,SizeOf(T3DCoordinate),0);
   WettedArea:=0.0;
   MaxY:=0;

   if FProfile.NumberOfPoints>1 then
   begin
      Setlength(Mesh,Rows);
      for I:=1 to Rows do setlength(Mesh[I-1],Cols);

      // Vertical spacing between rows increases from bottom to top
      VertInd:=0;
      for I:=0 to Rows-1 do
      begin
         if I=0 then VertInd:=0
                else VertInd:=VertInd+Power(Factor,I-1);
      end;
      VertDist:=Span/VertInd;

      Index:=_ComboBox2.ItemIndex+1;
      Plane.a:=0.0;
      Plane.b:=0.0;
      Plane.c:=1.0;
      Spline:=TFreeSpline.Create;
      Spline.Capacity:=Nh;
      for I:=1 to Nh do
      begin
         P.X:=NacaProfiles[0,I];
         P.Y:=NacaProfiles[Index,I];
         P.Z:=0;
         Spline.Add(P);
      end;

      VertInd:=0;
      for I:=0 to Rows-1 do
      begin
         Chord:=0.0;
         Start:=0.0;
         if I=0 then VertInd:=0
                else VertInd:=VertInd+Power(Factor,I-1);
         Height:=Span-VertInd*VertDist;
         if (I>0) and (I<Rows-1) then
         begin
            Plane.d:=Height;
            if FProfile.IntersectPlane(Plane,Results) then if Results.NumberOfIntersections=2 then
            begin
               Chord:=Results.Points[1].X-Results.Points[0].X;
               Start:=Results.Points[0].X;
            end;
         end else if I=Rows-1 then
         begin
            Chord:=Input1.Value;
            Start:=0.0;
         end else
         begin
            if ComboBox1.ItemIndex=0 then
            begin
               Chord:=Input2.Value;
               Start:=Input4.Value;
            end else
            begin
               chord:=0.0;
               Case ComboBox1.ItemIndex of
                  1 : Start:=0.5*Input1.Value;
                  3 : start:=0.75*Input1.Value;
                  else Start:=0;
               end;
            end;
         end;

         for J:=0 to Cols-1 do
         begin
            P:=Spline.Value(J/(Cols-1));
            Mesh[I,J].X:=Start+Chord-P.X*Chord;
            Mesh[I,J].Y:=P.Y*Chord;
            Mesh[I,J].Z:=-Height;
            if Mesh[I,J].Y>MaxY then MaxY:=Mesh[I,J].Y;
         end;
      end;
      Spline.Destroy;
      // calculate volume, center of bouyancy and wetted area
      begin
         for I:=2 to Rows do
         begin
            for J:=2 to Cols do
            begin
               P_1:=Mesh[I-1,J-1];
               P_2:=Mesh[I-1,J-2];
               P_3:=Mesh[I-2,J-2];
               P_4:=Mesh[I-2,J-1];
               ProcessTriangle(P_1,P_2,P_3);
               ProcessTriangle(P_1,P_3,P_4);
               P_1.Y:=-P_1.Y;
               P_2.Y:=-P_2.Y;
               P_3.Y:=-P_3.Y;
               P_4.Y:=-P_4.Y;
               ProcessTriangle(P_1,P_4,P_3);
               ProcessTriangle(P_1,P_3,P_2);
            end;
         end;
      end;

      // Calculate planform area
      if Span<>0.0 then
      begin
         P:=FProfile.Value(1.0);
         P1.X:=P.X;
         P1.Y:=P.Z;
         for I:=0 to FProfile.Fragments do
         begin
            P:=FProfile.Value(I/FProfile.Fragments);
            P2.X:=P.X;
            P2.Y:=P.Z;
            DeltaA:=0.5*(P2.X+P1.X)*(P2.Y-P1.Y);
            Area:=Area+DeltaA;
            COG.X:=COG.X+DeltaA*0.25*(P2.X+P1.X);
            COG.Y:=COG.Y+DeltaA*0.50*(P2.Y+P1.Y);
            P1:=P2;
         end;
         if Area<>0 then
         begin
            COG.X:=COG.X/Area;
            COG.Y:=COG.Y/Area;
         end;
      end;

      if Span<>0 then MeanChord:=Area/Span;
      if MeanChord<>0 then GeomAspectRatio:=Span/MeanChord;
      EffAspectRatio:=2*GeomAspectRatio;

      if EffAspectRatio>0 then
      begin
         // Calculate lift and drag
         for I:=0 to 45 do
         begin
            Angle:=I/3;
            Cl:=Angle* Cl2d/(1+2/EffAspectRatio);
            Cd:=(Cl*Cl)/(Pi*EffAspectRatio);
            Series1.AddXY(Angle,Cl);
            Series2.AddXY(Angle,Cd);
         end;
      end;
      _Label19.Caption:=FloatToStrF(Volume,ffFixed,7,3)+#32+VolStr(FFreeship.ProjectSettings.ProjectUnits);
      _Label21.Caption:=FloatToStrF(VolCOG.X,ffFixed,7,3)+', '+FloatToStrF(VolCOG.Z,ffFixed,7,3)+#32+LengthStr(FFreeship.ProjectSettings.ProjectUnits);
      _Label23.Caption:=FloatToStrF(WettedArea,ffFixed,7,3)+#32+AreaStr(FFreeship.ProjectSettings.ProjectUnits);
      _Label8.Caption:=FloatToStrF(Area,ffFixed,7,3)+#32+AreaStr(FFreeship.ProjectSettings.ProjectUnits);
      _Label10.Caption:=FloatToStrF(COG.X,ffFixed,7,3)+', '+FloatToStrF(COG.Y,ffFixed,7,3)+#32+LengthStr(FFreeship.ProjectSettings.ProjectUnits);
      _Label12.Caption:=FloatToStrF(MeanChord,ffFixed,7,3)+#32+LengthStr(FFreeship.ProjectSettings.ProjectUnits);
      _Label14.Caption:=FloatToStrF(GeomAspectRatio,ffFixed,7,3);
      _Label16.Caption:=FloatToStrF(EffAspectRatio,ffFixed,7,3);
      _Label25.Caption:=FloatToStrF(2*MaxY,ffFixed,7,3)+#32+LengthStr(FFreeship.ProjectSettings.ProjectUnits);
   end;
   Viewport.ZoomExtents;
end;{TFreeKeelWizardDialog.UpdateData}

function TFreeKeelWizardDialog.Execute(Freeship:TFreeShip):Boolean;
begin
   FFreeship:=Freeship;
   FProfile:=TFreeSpline.Create;
   ComboBox1Click(self);
   ShowModal;
   FProfile.Destroy;
   FProfile:=nil;
   Result:=ModalResult=mrOK;
end;{TFreeKeelWizardDialog.Execute}

procedure TFreeKeelWizardDialog.BitBtn1Click(Sender: TObject);
begin
   Modalresult:=mrOK;
end;{TFreeKeelWizardDialog.BitBtn1Click}

procedure TFreeKeelWizardDialog.BitBtn2Click(Sender: TObject);
begin
   Modalresult:=mrCancel;
end;{TFreeKeelWizardDialog.BitBtn2Click}

procedure TFreeKeelWizardDialog.ComboBox1Click(Sender: TObject);
begin
   Input2.Enabled:=ComboBox1.ItemIndex=0;
   Input4.Enabled:=Input2.Enabled;
   if not Input2.Enabled then Input2.Value:=0.0;
   UpdateData;
end;{TFreeKeelWizardDialog.ComboBox1Click}

procedure TFreeKeelWizardDialog.Input1AfterSetValue(Sender: TObject);
begin
   UpdateData;
end;{TFreeKeelWizardDialog.Input1AfterSetValue}

procedure TFreeKeelWizardDialog.ViewportRequestExtents(Sender: TObject; var Min, Max: T3DCoordinate);
var I,J  : Integer;
    P    : T3DCoordinate;
begin
   if FProfile<>nil then if FProfile.NumberOfPoints>1 then
   begin
      FProfile.Extents(Min,Max);
      for I:=1 to Rows do
         for J:=1 to Cols do
      begin
         P:=Mesh[I-1,J-1];
         MinMax(P,Min,Max);
         P.Y:=-P.Y;
         MinMax(P,Min,Max);
      end;
   end;
   {
   if Max.X-Min.X<=0 then Max.X:=Min.X+1;
   if Max.Y-Min.Y<=0 then Max.Y:=Min.Y+1;
   if Max.Z-Min.Z<=0 then Max.Z:=Min.Z+1;
   }
end;{TFreeKeelWizardDialog.ViewportRequestExtents}

procedure TFreeKeelWizardDialog.ViewportRedraw(Sender: TObject);

    procedure DrawPoint(P:T3DCoordinate;Text:string);
    var Pt    : TPoint;
        Size  : Integer;
    begin
      Pt:=Viewport.Project(P);
      if Text<>'' then
      begin
         // Skip translation
         Viewport.FontName:='Arial';
         // End Skip translation
         Viewport.FontColor:=FFreeship.Preferences.HydrostaticsFontColor;
         size:=Round(Sqrt(Viewport.Zoom)*7);
         if size<2 then size:=2;
         Viewport.FontSize:=size;
      end;
      Size:=Round(Sqrt(Viewport.Zoom)*(FFreeship.Preferences.PointSize+1));
      if size<1 then size:=1;
      Viewport.BrushStyle:=bsClear;
      if Viewport.Printing then Size:=round(Size*Viewport.PrintResolution/150);
      Viewport.PenColor:=clDkGray;//Black;
      Viewport.BrushColor:=clWhite;
      Viewport.BrushStyle:=bsSolid;
      // Draw entire circle in white;
      Viewport.DrawingCanvas.Ellipse(Pt.X-Size,Pt.Y-Size,Pt.X+Size,Pt.Y+Size);
      // Draw upper left part in black
      Viewport.BrushColor:=clBlack;
      Viewport.DrawingCanvas.Pie(Pt.X-Size,Pt.Y-Size,Pt.X+Size,Pt.Y+Size,Pt.X-1,Pt.Y-Size,Pt.X-Size,Pt.Y-1);
      // Draw lower right part in black
      Viewport.DrawingCanvas.Pie(Pt.X-Size,Pt.Y-Size,Pt.X+Size,Pt.Y+Size,Pt.X-1,Pt.Y+Size,Pt.X+Size,Pt.Y-1);
      Viewport.BrushStyle:=bsClear;
      if Text<>'' then Viewport.DrawingCanvas.TextOut(Pt.X+2*size,Pt.Y,Text);
    end;{DrawPoint}

var Pts  : array of TPoint;
    P1,P2: T3DCoordinate;
    P3,P4: T3DCoordinate;
    I,J  : Integer;
begin
   // create aft profile
   if FProfile<>nil then
   begin

      if Viewport.ViewportMode=vmWireFrame then
      begin
         if FProfile.NumberOfPoints>1 then
         begin
            FProfile.Draw(Viewport);
            Setlength(Pts,Cols);
            for I:=1 to Rows do //if (I=0) or (I=Rows) then
            begin
               if I in [1,Rows] then Viewport.PenColor:=clBlack
                                else Viewport.PenColor:=clSilver;
               for J:=1 to Cols do Pts[J-1]:=Viewport.Project(Mesh[I-1,J-1]);
               Viewport.DrawingCanvas.Polyline(Pts);
               for J:=1 to Cols do
               begin
                  P1:=Mesh[I-1,J-1];
                  P1.Y:=-P1.Y;
                  Pts[J-1]:=Viewport.Project(P1);
               end;
               Viewport.DrawingCanvas.Polyline(Pts);
            end;
            Setlength(Pts,Rows);
            for J:=1 to Cols do //if not odd(J) then
            begin
               if J in [1,Cols] then Viewport.PenColor:=clBlack
                                else Viewport.PenColor:=clSilver;
               for I:=1 to Rows do Pts[I-1]:=Viewport.Project(Mesh[I-1,J-1]);
               Viewport.DrawingCanvas.Polyline(Pts);
               for I:=1 to Rows do
               begin
                  P1:=Mesh[I-1,J-1];
                  P1.Y:=-P1.Y;
                  Pts[I-1]:=Viewport.Project(P1);
               end;
               Viewport.DrawingCanvas.Polyline(Pts);
            end;
            DrawPoint(SetPoint(COG.X,0,COG.Y),'');
         end;
      end else
      begin
         for I:=2 to Rows do
         begin
            for J:=2 to Cols do
            begin
               P1:=Mesh[I-1][J-1];
               P2:=Mesh[I-1][J-2];
               P3:=Mesh[I-2][J-2];
               P4:=Mesh[I-2][J-1];
               Viewport.ShadeTriangle(P1,P2,P3,GetRValue(FFreeship.Preferences.LayerColor),GetGValue(FFreeship.Preferences.LayerColor),GetBValue(FFreeship.Preferences.LayerColor));
               Viewport.ShadeTriangle(P1,P3,P4,GetRValue(FFreeship.Preferences.LayerColor),GetGValue(FFreeship.Preferences.LayerColor),GetBValue(FFreeship.Preferences.LayerColor));
               P1.Y:=-P1.Y;
               P2.Y:=-P2.Y;
               P3.Y:=-P3.Y;
               P4.Y:=-P4.Y;
               Viewport.ShadeTriangle(P1,P2,P3,GetRValue(FFreeship.Preferences.LayerColor),GetGValue(FFreeship.Preferences.LayerColor),GetBValue(FFreeship.Preferences.LayerColor));
               Viewport.ShadeTriangle(P1,P3,P4,GetRValue(FFreeship.Preferences.LayerColor),GetGValue(FFreeship.Preferences.LayerColor),GetBValue(FFreeship.Preferences.LayerColor));
            end;
         end;
      end;
   end;

end;{TFreeKeelWizardDialog.ViewportRedraw}

procedure TFreeKeelWizardDialog.SpeedButton1Click(Sender: TObject);
var Str  : String;
begin
   Str:=Lowercase(Combobox.Text)+#32+_Combobox2.Text;
   FFreeship.Edit.CreateUndoObject('Add '+str,True);
   SendToSurface(FFreeship.Surface);
   FFreeship.FileChanged:=True;
   if Assigned(FFreeship.OnUpdateGeometryInfo) then FFreeship.OnUpdateGeometryInfo(FFreeship);
   FFreeship.Redraw;
end;{TFreeKeelWizardDialog.SpeedButton1Click}

procedure TFreeKeelWizardDialog.Input5AfterSetValue(Sender: TObject);
begin
   UpdateData;
end;{TFreeKeelWizardDialog.Input5AfterSetValue}

procedure TFreeKeelWizardDialog.Input6AfterSetValue(Sender: TObject);
begin
   UpdateData;
end;{TFreeKeelWizardDialog.Input6AfterSetValue}

procedure TFreeKeelWizardDialog.TrackBar1Change(Sender: TObject);
begin
   UpdateData;
end;{TFreeKeelWizardDialog.TrackBar1Change}

procedure TFreeKeelWizardDialog.SendToSurface(Surface:TFreeSubdivisionsurface);
var I,J           : Integer;
    FacePoints    : TFasterList;
    Grid          : TFreeSubdivisionGrid;
    P             : TFreeSubdivisionControlPoint;
    Edge          : TFreeSubdivisionedge;
    Layer         : TFreeSubdivisionLayer;
    P3D           : T3DCoordinate;
    Str           : string;
    PrevCursor    : TCursor;
begin
   PrevCursor:=Screen.Cursor;
   try
      Str:=Lowercase(Combobox.Text)+#32+_Combobox2.Text;
      Layer:=Surface.AddNewLayer;
      Str:=Combobox1.Text+#32+Lowercase(ComboBox.Text)+#32+_ComboBox2.Text;
      Layer.Name:=Str;
      Layer.Color:=FFreeship.Preferences.LayerColor;
      Setlength(Grid,Rows+2);
      for I:=1 to Rows+2 do Setlength(Grid[I-1],Cols);
      for I:=2 to Rows+1 do
      begin
         if (I=2) and (ComboBox1.ItemIndex>0) then
         begin
            P:=TFreeSubdivisionControlPoint.Create(Surface);
            Surface.AddControlPoint(P);
            P.Coordinate:=Mesh[I-2,0];
            Grid[I-1,0]:=P;
            for J:=2 to Cols do Grid[I-1][J-1]:=grid[I-1][0];
         end else
         begin
            for J:=1 to Cols do
            begin
               P:=TFreeSubdivisionControlPoint.Create(Surface);
               Surface.AddControlPoint(P);
               P.Coordinate:=Mesh[I-2,J-1];
               Grid[I-1,J-1]:=P;
            end;
         end;
      end;
      for J:=1 to Cols do
      begin
         if ComboBox1.ItemIndex=0 then
         begin
            // Close bottom
            P:=Grid[1][J-1] as TFreeSubdivisionControlpoint;
            if abs(P.Coordinate.Y)<1e-4then
            begin
               // do nothing
               Grid[0,J-1]:=P;
            end else
            begin
               // close at bottom
               P3D:=P.Coordinate;
               P3D.Y:=0.0;
               P:=TFreeSubdivisionControlPoint.Create(Surface);
               Surface.AddControlPoint(P);
               P.Coordinate:=P3D;
               Grid[0,J-1]:=P;
            end;
         end else Grid[0,J-1]:=Grid[1,J-1];

         // Close Top
         P:=Grid[Rows][J-1] as TFreeSubdivisionControlpoint;
         if abs(P.Coordinate.Y)<1e-4then
         begin
            // do nothing
            Grid[Rows+1,J-1]:=P;
         end else
         begin
            // close at top
            P3D:=P.Coordinate;
            P3D.Y:=0.0;
            P:=TFreeSubdivisionControlPoint.Create(Surface);
            Surface.AddControlPoint(P);
            P.Coordinate:=P3D;
            Grid[Rows+1,J-1]:=P;
         end;
      end;
      FacePoints:=TFasterList.Create;
      for I:=2 to Rows+2 do
      begin
         for J:=2 to Cols do
         begin
            FacePoints.Clear;
            if FacePoints.IndexOf(Grid[I-1,J-1])=-1 then FacePoints.Add(Grid[I-1,J-1]);
            if FacePoints.IndexOf(Grid[I-1,J-2])=-1 then FacePoints.Add(Grid[I-1,J-2]);
            if FacePoints.IndexOf(Grid[I-2,J-2])=-1 then FacePoints.Add(Grid[I-2,J-2]);
            if FacePoints.IndexOf(Grid[I-2,J-1])=-1 then FacePoints.Add(Grid[I-2,J-1]);
            if FacePoints.Count>=3 then Surface.AddControlFace(FacePoints,True,Layer);
         end;
      end;
      FacePoints.Destroy;
      // set crease edges at top and bottom
      for J:=2 to Cols do
      begin
         if ComboBox1.ItemIndex=0 then
         begin
            Edge:=Surface.EdgeExists(Grid[1,J-2],Grid[1,J-1]);
            if Edge<>nil then Edge.Crease:=true;
         end;
         Edge:=Surface.EdgeExists(Grid[Rows,J-2],Grid[Rows,J-1]);
         if Edge<>nil then Edge.Crease:=true;
      end;
   finally
      Screen.Cursor:=prevCursor;
   end;
end;{TFreeKeelWizardDialog.SendToSurface}

procedure TFreeKeelWizardDialog.SpeedButton2Click(Sender: TObject);
var Surface : TFreeSubdivisionSurface;
    Faces   : TFasterList;
    I       : Integer;
begin
   Surface:=TFreeSubdivisionSurface.Create;
   SendToSurface(Surface);
   Faces:=TFasterList.Create;
   Faces.Capacity:=Surface.NumberOfControlFaces;
   for I:=1 to Surface.NumberOfControlFaces do Faces.Add(Surface.ControlFace[I-1]);
   FFreeship.SavePart(Faces);
   Faces.Destroy;
   Surface.Destroy;
end;{TFreeKeelWizardDialog.SendToSurface}

end.
