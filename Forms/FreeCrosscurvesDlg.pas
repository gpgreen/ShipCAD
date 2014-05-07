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
unit FreeCrosscurvesDlg;

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
     StdCtrls,
     Buttons,
     FreeGeometry,
     ExtCtrls,
     TeEngine,
     Series,
     TeeProcs,
     Chart,
     FreeshipUnit,
     FreeNumInput,
     ComCtrls,
     ToolWin,
     ImgList,
     Printers,
     Grids;

type TFreeCrosscurvesDialog  = class(TForm)
                                 MenuImages: TImageList;
                                 ToolBar1: TToolBar;
                                 _ToolButton10: TToolButton;
                                 PrintButton: TToolButton;
                                 _ToolButton14: TToolButton;
                                 ToolButton25: TToolButton;
                                 ToolButton7: TToolButton;
                                 PrintDialog: TPrintDialog;
                                 Panel1: TPanel;
                                 Panel: TPanel;
                                 GroupBox1: TGroupBox;
                                 DisplBox: TListBox;
                                 FreeNumInput1: TFreeNumInput;
                                 Button1: TButton;
                                 Button2: TButton;
                                 CheckBox1: TCheckBox;
                                 Label1: TLabel;
                                 FreeNumInput2: TFreeNumInput;
                                 Label2: TLabel;
                                 FreeNumInput3: TFreeNumInput;
                                 Label3: TLabel;
                                 FreeNumInput4: TFreeNumInput;
                                 GroupBox2: TGroupBox;
                                 HeelBox: TListBox;
                                 FreeNumInput5: TFreeNumInput;
                                 Button3: TButton;
                                 Button4: TButton;
    ToolButton1: TToolButton;
    PageControl1: TPageControl;
    Splitter1: TSplitter;
    TabSheet1: TTabSheet;
    Chart: TChart;
    TabSheet2: TTabSheet;
    Grid: TStringGrid;
    ToolButton2: TToolButton;
    SaveButton: TToolButton;
    SaveDialog: TSaveDialog;
                                 procedure ToolButton25Click(Sender: TObject);
                                 procedure ToolButton7Click(Sender: TObject);
                                 procedure Button1Click(Sender: TObject);
                                 procedure Button2Click(Sender: TObject);
                                 procedure CheckBox1Click(Sender: TObject);
                                 procedure DisplBoxMouseDown(Sender: TObject; Button: TMouseButton;Shift: TShiftState; X, Y: Integer);
                                 procedure Button3Click(Sender: TObject);
                                 procedure Button4Click(Sender: TObject);
                                 procedure HeelBoxClick(Sender: TObject);
    procedure ToolButton1Click(Sender: TObject);
    procedure GridDrawCell(Sender: TObject; ACol, ARow: Integer;
      Rect: TRect; State: TGridDrawState);
    procedure PrintButtonClick(Sender: TObject);
    procedure ToolButton2Click(Sender: TObject);
    procedure SaveButtonClick(Sender: TObject);
                              private{ Private declarations }
                                 FFreeship:TFreeship;
                                 FAbortCalculation:Boolean;
                              public { Public declarations }
                                 function Execute(Freeship:TFreeship):Boolean;
                                 procedure CheckDisplacements(var Data:TFloatArray;var NData:Integer);
                                 procedure GetDisplacements(var Data:TFloatArray;var NData:Integer);
                                 procedure SetDisplacements(Data:TFloatArray;NData:Integer);
                                 procedure UpdateDisplacementData;
                                 procedure UpdateHeelingAngleData;
                                 procedure CheckHeelingAngles(var Data:TFloatArray;var NData:Integer);
                                 procedure GetHeelingAngles(var Data:TFloatArray;var NData:Integer);
                                 procedure SetHeelingAngles(Data:TFloatArray;NData:Integer);
                              end;

var FreeCrosscurvesDialog: TFreeCrosscurvesDialog;

implementation

uses FreeLanguageSupport,
     Math;

{$R *.dfm}

function TFreeCrosscurvesDialog.Execute(Freeship:TFreeship):Boolean;
begin
   FFreeship:=Freeship;
   Pagecontrol1.ActivePage:=Tabsheet1;
   Chart.Title.Text.Text:=Userstring(293);
   Chart.LeftAxis.Title.Caption:='KN sin(ø)'+#32+LengthStr(FFreeship.ProjectSettings.ProjectUnits);
   Chart.BottomAxis.Title.Caption:=Userstring(4)+#32+WeightStr(FFreeship.ProjectSettings.ProjectUnits);
   UpdateDisplacementData;
   UpdateHeelingAngleData;
   ShowModal;
   Result:=modalResult=mrOK;
end;{TFreeCrosscurvesDialog.Execute}

procedure TFreeCrosscurvesDialog.CheckDisplacements(var Data:TFloatArray;var NData:Integer);
begin
   SortFloatArray(Data,NData);
   if NData=0 then
   begin
      Setlength(Data,NData+1);
      Data[NData]:=0.0;
      inc(NData);
   end;
end;{TFreeCrosscurvesDialog.CheckDisplacements}

procedure TFreeCrosscurvesDialog.GetDisplacements(var Data:TFloatArray;var NData:Integer);
var I:Integer;
begin
   Setlength(Data,DisplBox.Items.Count);
   NData:=DisplBox.Items.Count;
   for I:=1 to NData do Data[I-1]:=StrToFloat(DisplBox.Items[I-1]);
   CheckDisplacements(Data,NData);
end;{TFreeCrosscurvesDialog.GetDisplacements}

procedure TFreeCrosscurvesDialog.SetDisplacements(Data:TFloatArray;NData:Integer);
var I : Integer;
begin
   CheckDisplacements(Data,NData);
   DisplBox.Items.BeginUpdate;
   DisplBox.Clear;
   try
      for I:=1 to NData do DisplBox.Items.Add(FloatToStrF(Data[I-1],ffFixed,7,NumberOfDecimals(Data[I-1])));
   finally
      DisplBox.Items.EndUpdate;
   end;
end;{TFreeCrosscurvesDialog.SetDisplacements}

procedure TFreeCrosscurvesDialog.ToolButton25Click(Sender: TObject);
begin
   ModalResult:=mrOK;
end;{TFreeCrosscurvesDialog.ToolButton25Click}

procedure TFreeCrosscurvesDialog.ToolButton7Click(Sender: TObject);
begin
   ModalResult:=mrCancel;
end;{TFreeCrosscurvesDialog.ToolButton7Click}

procedure TFreeCrosscurvesDialog.Button1Click(Sender: TObject);
var Data    : TFloatArray;
    NData   : Integer;
begin
   GetDisplacements(Data,NData);
   setlength(Data,NData+1);
   Data[NData]:=FreeNumInput1.Value;
   inc(NData);
   SetDisplacements(Data,NData);
   UpdateDisplacementData;
   FreeNumInput1.SetFocus;
end;{TFreeCrosscurvesDialog.Button1Click}

procedure TFreeCrosscurvesDialog.Button2Click(Sender: TObject);
var Index:Integer;
begin
   Index:=DisplBox.ItemIndex;
   if (Index<>-1) and (DisplBox.Count>0) then
   begin
      DisplBox.Items.Delete(Index);
      if Index>DisplBox.Count-1 then Index:=DisplBox.Count-1;
      DisplBox.ItemIndex:=Index;
      UpdateDisplacementData;
   end;
end;{TFreeCrosscurvesDialog.Button2Click}

procedure TFreeCrosscurvesDialog.UpdateDisplacementData;
begin
   FreeNuminput1.Enabled:=not checkbox1.Checked;
   Button1.Enabled:=not checkbox1.Checked;
   Button2.Enabled:=(not checkbox1.Checked) and (Displbox.ItemIndex<>-1);
   FreeNuminput2.Enabled:=checkbox1.Checked;
   FreeNuminput3.Enabled:=checkbox1.Checked;
   FreeNuminput4.Enabled:=checkbox1.Checked;
   DisplBox.Enabled:=not Checkbox1.Checked;
   if Checkbox1.Checked then DisplBox.Color:=clBtnFace
                        else DisplBox.Color:=clWindow;
end;{TFreeCrosscurvesDialog.UpdateDisplacementData}

procedure TFreeCrosscurvesDialog.UpdateHeelingAngleData;
begin
   Button4.Enabled:=Heelbox.ItemIndex<>-1;
end;{TFreeCrosscurvesDialog.UpdateHeelingAngleData}

procedure TFreeCrosscurvesDialog.CheckBox1Click(Sender: TObject);
begin
   UpdateDisplacementData;
end;{TFreeCrosscurvesDialog.CheckBox1Click}

procedure TFreeCrosscurvesDialog.DisplBoxMouseDown(Sender: TObject;Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
   UpdateDisplacementData;
end;{TFreeCrosscurvesDialog.DisplBoxMouseDown}

procedure TFreeCrosscurvesDialog.CheckHeelingAngles(var Data:TFloatArray;var NData:Integer);
var I:Integer;
begin
   SortFloatArray(Data,NData);
   while (NData>0) and (Data[0]<0) do
   begin
      for I:=2 to NData do Data[I-2]:=Data[I-1];
      Dec(NData);
   end;
   if NData>0 then if Data[0]>1e-5 then
   begin
      Setlength(Data,NData+1);
      Data[NData]:=0.0;
      inc(NData);
      SortFloatArray(Data,NData);
   end;
   if NData=0 then
   begin
      Setlength(Data,NData+1);
      Data[NData]:=0.0;
      inc(NData);
   end;
end;{TFreeCrosscurvesDialog.CheckHeelingAngles}

procedure TFreeCrosscurvesDialog.GetHeelingAngles(var Data:TFloatArray;var NData:Integer);
var I:Integer;
begin
   Setlength(Data,HeelBox.Items.Count);
   NData:=HeelBox.Items.Count;
   for I:=1 to NData do Data[I-1]:=StrToFloat(HeelBox.Items[I-1]);
   CheckHeelingAngles(Data,NData);
end;{TFreeCrosscurvesDialog.GetHeelingAngles}

procedure TFreeCrosscurvesDialog.SetHeelingAngles(Data:TFloatArray;NData:Integer);
var I : Integer;
begin
   CheckHeelingAngles(Data,NData);
   HeelBox.Items.BeginUpdate;
   HeelBox.Clear;
   try
      for I:=1 to NData do HeelBox.Items.Add(FloatToStrF(Data[I-1],ffFixed,7,2));
   finally
      HeelBox.Items.EndUpdate;
   end;
end;{TFreeCrosscurvesDialog.SetHeelingAngles}

procedure TFreeCrosscurvesDialog.Button3Click(Sender: TObject);
var Data    : TFloatArray;
    NData   : Integer;
begin
   GetHeelingAngles(Data,NData);
   setlength(Data,NData+1);
   Data[NData]:=FreeNumInput5.Value;
   inc(NData);
   SetHeelingAngles(Data,NData);
   UpdateHeelingAngleData;
   FreeNumInput5.SetFocus;
end;{TFreeCrosscurvesDialog.Button3Click}

procedure TFreeCrosscurvesDialog.Button4Click(Sender: TObject);
var Index:Integer;
begin
   Index:=HeelBox.ItemIndex;
   if (Index<>-1) and (HeelBox.Count>0) then
   begin
      HeelBox.Items.Delete(Index);
      if Index>HeelBox.Count-1 then Index:=HeelBox.Count-1;
      HeelBox.ItemIndex:=Index;
      UpdateHeelingAngleData;
   end;
end;{TFreeCrosscurvesDialog.Button4Click}

procedure TFreeCrosscurvesDialog.HeelBoxClick(Sender: TObject);
begin
   UpdateHeelingAngleData;
end;{TFreeCrosscurvesDialog.HeelBoxClick}

procedure TFreeCrosscurvesDialog.ToolButton1Click(Sender: TObject);
var NHeel         : Integer;
    NDispl        : Integer;
    I,J,N         : Integer;
    Value         : TFloatType;
    Angles        : TFloatArray;
    Displacements : TFloatArray;
    Calculation   : TFreehydrostaticCalc;
    Data          : TFreeCrosscurvesData;
    Series        : TLineSeries;
    PrevCursor    : TCursor;
begin
   GetHeelingAngles(Angles,NHeel);
   if CheckBox1.Checked then
   begin
      if FreeNuminput4.Value>0 then
      begin
         N:=round((FreeNuminput3.Value-FreeNuminput2.Value)/FreeNuminput4.Value)+10;
         if N>0 then setlength(Displacements,N);
         NDispl:=0;
         Value:=FreeNuminput2.Value;
         while (Value<=FreeNuminput3.Value) or (abs(Value-FreeNuminput3.Value)<1e-4) do
         begin
            Displacements[NDispl]:=Value;
            inc(NDispl);
            Value:=Value+FreeNuminput4.Value;
         end;
      end else MessageDlg(Userstring(292)+'!',mtError,[mbOk],0);
   end else GetDisplacements(Displacements,NDispl);
   PrevCursor:=Screen.Cursor;
   Screen.Cursor:=crHourglass;
   try
      FAbortCalculation:=False;
      ToolButton2.Enabled:=True;
      Calculation:=TFreehydrostaticCalc.Create(FFreeship);
      for I:=Chart.SeriesCount downto 1 do Chart.Series[I-1].Destroy;
      if NDispl=1 then
      begin
         Grid.ColCount:=2;
         Grid.RowCount:=NHeel+1;
      end else
      begin
         Grid.ColCount:=NHeel+1;
         Grid.RowCount:=NDispl+1;
      end;
      for I:=1 to Grid.Rowcount do
         for J:=1 to Grid.Colcount do Grid.Cells[J-1,I-1]:='';
      I:=1;
      if NDispl=1 then Chart.BottomAxis.Title.Caption:=Userstring(295)+' [º]'
                  else Chart.BottomAxis.Title.Caption:=Userstring(4)+#32+WeightStr(FFreeship.ProjectSettings.ProjectUnits);
      Chart.UndoZoom;
      while (I<=NHeel) and (not FAbortCalculation) do
      begin
         Calculation.HeelingAngle:=Angles[I-1];
         Calculation.Trim:=0;
         if (NDispl=1) and (I=1) then
         begin
            Series:=TLineSeries.Create(Chart);
            Chart.AddSeries(Series);
            Series.Title:='KN sin(ø)';
            Series.LinePen.Color:=clBlack;
            Series.LinePen.Visible:=True;
            Series.LinePen.Style:=psSolid;
            Series.ShowInLegend:=false;
            Grid.Cells[I,0]:=FloatToStrF(Angles[I-1],ffFixed,7,1)+'º';
         end else if NDispl<>1 then
         begin
            Series:=TLineSeries.Create(Chart);
            Series.AllowSinglePoint:=True;
            Chart.AddSeries(Series);
            Series.Title:=FloatToStrF(Angles[I-1],ffFixed,7,1)+'º';
            Grid.Cells[I,0]:=FloatToStrF(Angles[I-1],ffFixed,7,1)+'º';
         end;
         Series.LinePen.Width:=1;
         J:=1;
         while (j<=NDispl) and (not FAbortCalculation) do
         begin
            if I=1 then
            begin
               if NDispl=1 then Grid.Cells[1,0]:='KN sin(ø)'
                           else Grid.Cells[0,J]:=FloatToStrF(Displacements[J-1],ffFixed,7,NumberOfDecimals(Displacements[J-1]));
            end;
            if NDispl=1 then Grid.Cells[0,I]:=FloatToStrF(Angles[I-1],ffFixed,7,1)+'º';

            if Calculation.Balance(Displacements[J-1],False,Data) then
            begin
               if NDispl=1 then
               begin
                  Series.AddXY(Angles[I-1],Data.CenterOfBuoyancy.Y);
                  Grid.Cells[1,I]:=FloatToStrF(Data.CenterOfBuoyancy.Y,ffFixed,7,3);
               end else
               begin
                  Series.AddXY(Data.Displacement,Data.CenterOfBuoyancy.Y);
                  Grid.Cells[I,J]:=FloatToStrF(Data.CenterOfBuoyancy.Y,ffFixed,7,3);
               end;
               Chart.Refresh;
               Grid.Repaint;
            end else
            begin
               if NDispl=1 then Grid.Cells[1,I]:='<->'
                           else Grid.Cells[I,J]:='<->';
            end;
            Application.ProcessMessages;
            inc(J);
         end;
         inc(I);
         if Series.Count=0 then series.Destroy;
      end;
      Calculation.Destroy;
   finally
      Screen.Cursor:=PrevCursor;
      ToolButton2.Enabled:=False;
      PrintButton.Enabled:=True;
      SaveButton.Enabled:=True;
      if FAbortCalculation then MessageDlg(Userstring(0294)+'!',mtInformation,[mbOk],0);
   end;
end;{TFreeCrosscurvesDialog.ToolButton1Click}

procedure TFreeCrosscurvesDialog.GridDrawCell(Sender: TObject; ACol,ARow: Integer; Rect: TRect; State: TGridDrawState);
var W    : Integer;
    Str  : string;
    Back : TColor;
begin
   if (ACol=0) or (ARow=0) then
   begin
      if Grid.Canvas.Font.Style=[] then Grid.Canvas.Font.Style:=[fsBold];
   end else
   begin
      if Grid.Canvas.Font.Style=[fsBold] then Grid.Canvas.Font.Style:=[];
   end;
   if (ARow>0) and (ACol>0) then
   begin
      if gdSelected in state then
      begin
      end else if not Odd(ARow) then
      begin
         Back:=RGB(235,235,235);
         if Grid.Canvas.Brush.Color<>Back then Grid.Canvas.Brush.Color:=Back;
      end else
      begin
         if Grid.Canvas.Brush.Color<>clWindow then Grid.Canvas.Brush.Color:=clWindow;
      end;
      Grid.Canvas.Rectangle(Rect);
   end;
   Str:=Grid.Cells[ACol,ARow];
   W:=Grid.Canvas.TextWidth(Str);
   if (ARow=0) then Grid.Canvas.TextRect(Rect,(Rect.Left+Rect.Right-W) div 2,Rect.Top+3,Str)
               else Grid.Canvas.TextRect(Rect,Rect.Right-W-5,Rect.Top+3,Str);
end;{TFreeCrosscurvesDialog.GridDrawCell}

procedure TFreeCrosscurvesDialog.PrintButtonClick(Sender: TObject);
var PrintText : TextFile;
    MaxWidth  : array of integer;
    I,J,L,N   : Integer;
    Str,Tmp   : Ansistring;
begin
   if PrintDialog.Execute then
   begin
      AssignPrn(PrintText);
      Rewrite(PrintText);
      Setlength(MaxWidth,Grid.ColCount);
      for I:=1 to Grid.ColCount do
      begin
         MaxWidth[I-1]:=0;
         for J:=1 to Grid.RowCount do
         begin
            L:=Length(Grid.Cells[I-1,J-1]);
            if L>MaxWidth[I-1] then MaxWidth[I-1]:=L;
         end;
      end;
      Writeln(PrintText);
      Writeln(PrintText);
      Printer.Canvas.Font.Size:=Printer.Canvas.Font.Size-1;
      for I:=1 to Grid.RowCount do
      begin
         Str:='';
         for J:=1 to Grid.ColCount do
         begin
            Tmp:=Grid.Cells[J-1,I-1];
            L:=Length(Tmp);
            for N:=L+1 to MaxWidth[J-1] do Tmp:=#32+Tmp;
            if J=1 then Str:=Tmp
                   else Str:=Str+#32+Tmp;
         end;
         Writeln(PrintText,#32#32,Str);
      end;
      CloseFile(PrintText);
      Chart.Print;
   end;
end;{TFreeCrosscurvesDialog.PrintButtonClick}

procedure TFreeCrosscurvesDialog.ToolButton2Click(Sender: TObject);
begin
   FAbortCalculation:=True;
end;{TFreeCrosscurvesDialog.ToolButton2Click}

procedure TFreeCrosscurvesDialog.SaveButtonClick(Sender: TObject);
var Strings    : TStringList;
    MaxWidth   : array of integer;
    I,J,L,N    : Integer;
    Str,Tmp    : Ansistring;
begin
   SaveDialog.InitialDir:=FFreeship.Preferences.ExportDirectory;
   SaveDialog.FileName:=ChangeFileExt(Userstring(293),'.txt');
   if SaveDialog.Execute then
   begin
      strings:=TStringList.Create;
      Setlength(MaxWidth,Grid.ColCount);
      for I:=1 to Grid.ColCount do
      begin
         MaxWidth[I-1]:=0;
         for J:=1 to Grid.RowCount do
         begin
            L:=Length(Grid.Cells[I-1,J-1]);
            if L>MaxWidth[I-1] then MaxWidth[I-1]:=L;
         end;
      end;
      Strings.Add('');
      Strings.Add('');
      for I:=1 to Grid.RowCount do
      begin
         Str:='';
         for J:=1 to Grid.ColCount do
         begin
            Tmp:=Grid.Cells[J-1,I-1];
            L:=Length(Tmp);
            for N:=L+1 to MaxWidth[J-1] do Tmp:=#32+Tmp;
            if J=1 then Str:=Tmp
                   else Str:=Str+#32+Tmp;
         end;
         Strings.Add(Str);
      end;
      // Skip translation
      Strings.SaveToFile(ChangeFileExt(SaveDialog.Filename,'.txt'));
      // End Skip translation
      Strings.Destroy;
   end;
end;{TFreeCrosscurvesDialog.SaveButtonClick}

end.
