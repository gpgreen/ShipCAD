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

program FreeShip;

//{$DEFINE CREATE_TRANSLATION}

uses
  Forms,
  SysUtils,
  FreeLanguageSupport in 'Units\FreeLanguageSupport.pas',
  Main in 'Forms\Main.pas' {MainForm},
  FreeHullformWindow in 'Forms\FreeHullformWindow.pas' {FreeHullWindow},
  FreeLayerDlg in 'Forms\FreeLayerDlg.pas' {FreeLayerDialog},
  FreeNewModelDlg in 'Forms\FreeNewModelDlg.pas' {FreeNewModelDialog},
  FreeSplashWndw in 'Forms\FreeSplashWndw.pas' {FreeSplashWindow},
  FreeVersionUnit in 'Units\FreeVersionUnit.pas',
  FreeIntersectionDlg in 'Forms\FreeIntersectionDlg.pas' {FreeIntersectionDialog},
  FreeExtrudeDlg in 'Forms\FreeExtrudeDlg.pas' {FreeExtrudeDialog},
  FreeControlPointFrm in 'Forms\FreeControlPointFrm.pas' {FreeControlPointForm},
  FreeProjectSettingsDlg in 'Forms\FreeProjectSettingsDlg.pas' {FREEProjectSettingsDialog},
  FreeHydrostaticsdlg in 'Forms\FreeHydrostaticsdlg.pas' {FreeHydrostaticsDialog},
  FreeRotateDlg in 'Forms\FreeRotateDlg.pas' {FreeRotateDialog},
  FreeHydrostaticsFrm in 'Forms\FreeHydrostaticsFrm.pas' {FreeHydrostaticsForm},
  FreePreferencesDlg in 'Forms\FreePreferencesDlg.pas' {FreePreferencesDialog},
  FreeExpanedPlatesDlg in 'Forms\FreeExpanedPlatesDlg.pas' {FreeExpanedplatesDialog},
  FreeLinesplanFrme in 'Forms\FreeLinesplanFrme.pas' {FreeLinesplanFrame: TFrame},
  FreeLinesplanFrm in 'Forms\FreeLinesplanFrm.pas' {FreeLinesplanForm},
  FreeSaveImageDlg in 'Forms\FreeSaveImageDlg.pas' {SaveImageDialog},
  FreeInsertPlaneDlg in 'Forms\FreeInsertPlaneDlg.pas' {FreeInsertPlaneDialog},
  FreeMichletOutputDlg in 'Forms\FreeMichletOutputDlg.pas' {FreeMichletOutputDialog},
  FreeResistance_KaperDlg in 'Forms\FreeResistance_KaperDlg.pas' {FreeResistance_Kaper},
  FreeResistance_DelftDlg in 'Forms\FreeResistance_DelftDlg.pas' {FreeResistance_Delft},
  FreeMirrorPlaneDlg in 'Forms\FreeMirrorPlaneDlg.pas' {FreeMirrorPlaneDialog},
  FreeSelectLayersDlg in 'Forms\FreeSelectLayersDlg.pas' {FreeSelectLayersDialog},
  FreeKeelWizardDlg in 'Forms\FreeKeelWizardDlg.pas' {FreeKeelWizardDialog},
  VRMLUnit in 'Units\VRMLUnit.pas',
  FreeLackenbyDlg in 'Forms\FreeLackenbyDlg.pas' {FreeLackenbyDialog},
  FreeIGESUnit in 'Units\FreeIGESUnit.pas',
  FreeIntersectLayerDlg in 'Forms\FreeIntersectLayerDlg.pas' {FreeIntersectLayerDialog},
  FreeUndoHistoryDlg in 'Forms\FreeUndoHistoryDlg.pas' {FreeUndoHistoryDialog},
  FreeHydrostaticsResultsDlg in 'Forms\FreeHydrostaticsResultsDlg.pas' {FreeHydrostaticsResultsDialog},
  FreeBackgroundBlendingDlg in 'Forms\FreeBackgroundBlendingDlg.pas' {FreeBackgroundBlendDialog},
  FreeCylinderDlg in 'Forms\FreeCylinderDlg.pas' {FreeCylinderDialog},
  FreeStringsUnit in 'Units\FreeStringsUnit.pas',
  Free2DDXFExportDlg in 'Forms\Free2DDXFExportDlg.pas' {DXFExport2DDialog},
  FreeCrosscurvesDlg in 'Forms\FreeCrosscurvesDlg.pas' {FreeCrosscurvesDialog};

{$R *.res}
begin
   DecimalSeparator:='.';
   Application.Initialize;
   Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TFreeCrosscurvesDialog, FreeCrosscurvesDialog);
  {$IFNDEF CREATE_TRANSLATION}
   LoadLanguage(Mainform.Freeship.Preferences.LanguageFile);
   {$ENDIF}
   ShowTranslatedValues(Mainform);
   Application.CreateForm(TFreeKeelWizardDialog, FreeKeelWizardDialog);
   FreeSplashWindow:=TFreeSplashWindow.Create(Application);

   {$IFDEF CREATE_TRANSLATION}
      // Create a translation of all the forms and stringvalues in the project
      FreeExpanedplatesDialog:=TFreeExpanedplatesDialog.Create(Application);
      FreeExtrudeDialog:=TFreeExtrudeDialog.Create(Application);
      FreeHullWindow:=TFreeHullWindow.Create(Application);
      FreeHydrostaticsDialog:=TFreeHydrostaticsDialog.Create(Application);
      FreeHydrostaticsForm:=TFreeHydrostaticsForm.Create(Application);
      FreeControlPointForm:=TFreeControlPointForm.Create(Application);
      FreeHydrostaticsResultsDialog:=TFreeHydrostaticsResultsDialog.Create(Application);
      FreeInsertPlaneDialog:=TFreeInsertPlaneDialog.Create(Application);
      FreeIntersectionDialog:=TFreeIntersectionDialog.Create(Application);
      FreeIntersectLayerDialog:=TFreeIntersectLayerDialog.Create(Application);
      FreeLackenbyDialog:=TFreeLackenbyDialog.Create(Application);
      FreeLayerDialog:=TFreeLayerDialog.Create(Application);
      FreeLinesplanFrame:=TFreeLinesplanFrame.Create(Application);
      FreeMichletOutputDialog:=TFreeMichletOutputDialog.Create(Application);
      FreeMirrorPlaneDialog:=TFreeMirrorPlaneDialog.Create(Application);
      FreeNewModelDialog:=TFreeNewModelDialog.Create(Application);
      FreePreferencesDialog:=TFreePreferencesDialog.Create(Application);
      FREEProjectSettingsDialog:=TFREEProjectSettingsDialog.Create(Application);
      FreeResistance_Delft:=TFreeResistance_Delft.Create(Application);
      FreeResistance_Kaper:=TFreeResistance_Kaper.Create(Application);
      FreeRotateDialog:=TFreeRotateDialog.Create(Application);
      FreeSelectLayersDialog:=TFreeSelectLayersDialog.Create(Application);
      FreeUndoHistoryDialog:=TFreeUndoHistoryDialog.Create(Application);
      SaveImageDialog:=TSaveImageDialog.Create(Application);
      FreeBackgroundBlendDialog:=TFreeBackgroundBlendDialog.Create(Application);
      FreeCylinderDialog:=TFreeCylinderDialog.Create(Application);
      DXFExport2DDialog:=TDXFExport2DDialog.Create(Application);
      FreeLinesplanForm:=TFreeLinesplanForm.Create(Application);
      FreeCrosscurvesDialog:=TFreeCrosscurvesDialog.create(Application);
      CreateLanguageFile;
      FreeControlPointForm.Free;
      FreeExpanedplatesDialog.Free;
      FreeExtrudeDialog.Free;
      FreeHullWindow.Free;
      FreeHydrostaticsDialog.Free;
      FreeHydrostaticsForm.Free;
      FreeHydrostaticsResultsDialog.Free;
      FreeInsertPlaneDialog.Free;
      FreeIntersectionDialog.Free;
      FreeIntersectLayerDialog.Free;
      FreeLackenbyDialog.Free;
      FreeLayerDialog.Free;
      FreeLinesplanFrame.Free;
      FreeMichletOutputDialog.Free;
      FreeMirrorPlaneDialog.Free;
      FreeNewModelDialog.Free;
      FreePreferencesDialog.Free;
      FREEProjectSettingsDialog.Free;
      FreeResistance_Kaper.Free;
      FreeResistance_Delft.Free;
      FreeRotateDialog.Free;
      FreeSelectLayersDialog.Free;
      FreeUndoHistoryDialog.Free;
      SaveImageDialog.Free;
      FreeBackgroundBlendDialog.Free;
      FreeCylinderDialog.Free;
      DXFExport2DDialog.Free;
      FreeLinesplanForm.Free;
      FreeCrosscurvesDialog.Free;
   {$ENDIF}

   ShowTranslatedValues(FreeSplashWindow);
   FreeSplashWindow.Show;
   FreeSplashWindow.Refresh;
   sleep(1500);
   Application.Run;
end.
