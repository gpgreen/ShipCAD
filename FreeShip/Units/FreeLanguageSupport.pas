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

// Based upon IniLang v 0.9
// Freeware unit for Delphi 4 projects
// by Frédéric Sigonneau <aFas member> 24/04/1999
// e-mail : frederic.sigonneau@wanadoo.fr
// Modified to suit FREEship and adapted to new components

unit FreeLanguageSupport;

interface

uses Windows,
     ActnList,
     SysUtils,
     Classes,
     Forms,
     stdCtrls,
     typInfo,
	   extCtrls,
     iniFiles,
     Dialogs,
     Menus,
     ComCtrls,
     FreeStringsUnit,
     Controls;

// Skip translation


type TCompInfo = record
                    CompName: String;
                    Action  : string;
                    Caption : string;
                    Hint    : string;
                 end;

var CurrentLanguage : TMemIniFile; 	//Global variable for current language

//user procs
function LoadLanguage(Name:string):TMemIniFile;overload;
procedure CreateLanguageFile;
procedure ShowTranslatedValues(Component:TComponent);
function UserString(Index:Integer):String;

//utilities
function GetCompInfo(Component:TComponent):TCompInfo;
function GetProperty(comp:TComponent;prop:string):string;
procedure setProp(comp:TComponent;{const }prop,value:string);
function HasProperty(comp:TComponent;prop:string):boolean;

implementation

function LoadLanguage(Name:string):TMemIniFile;
var Filename:string;    
begin
	Filename:=ChangeFileExt(extractFileDir(application.exeName)+'\Languages\'+Name,'.ini');
   if FileExists(Filename) then
   begin
      if CurrentLanguage<>nil then CurrentLanguage.Free;
      CurrentLanguage:=TMemIniFile.create(Filename);
   end else if uppercase(Name)<>'ENGLISH' then MessageDlg('Could not load language file '+Filename,mtError,[mbOk],0);
   Result:=CurrentLanguage;
end;{LoadLanguage}

// Takse a string value and detrmines if it is a stringvalue that should
// be translated into another langage
// Values not to be translated are: numbers, strings containing no valid characters
function ValidString(Input:String):Boolean;
var Value   : single;
    IsNumber: Boolean;
    Flag,I,N: Integer;
    Up      : string;
begin
   Result:=True;
   Up:=Uppercase(trim(Input));
   if Input='' then Result:=False;
   if Result then
   begin
      if (Up='.FBM') or
         (Up='[MM]') or
         (Up='[M]') or
         (Up='[M2]') or
         (Up='[M3]') or
         (Up='[M4]') or
         (Up='[FT]') or
         (Up='[FT2]') or
         (Up='[FT3]') or
         (Up='[FT4]') or
         (Up='[T/M3]') or
         (Up='[LBS/FT3]') or
         (Up='[DEGR]') or
         (Up='[DEGR.]') or
         (Up='[%]') or
         (Up='[KN]') or
         (Up='[INCH]') then result:=False;
   end;
   if Result then
   begin
      Val(Input,Value,flag);
      IsNumber:=Flag=0;
      if IsNumber then Result:=False;
   end;
   if result then
   begin
      // check for valid characters
      N:=0;
      for I:=1 to length(Up) do if Up[I] in ['A'..'Z']then
      begin
         inc(N);
      end;
      Result:=N>1;
   end;
end;{ValidString}

{Creates the original iniFile 'English.ini' and fills it first
 with string properties and their values (captions, hints,
 items -TRadioGroup and TComboBox-
 and lines -TMemo and TRichEdit-  values).
 One section for each form in the project.
 Then fills a 'Misc' section with strings declared with 'const' or
 'resourcestring' keywords (customized messages to inform your users or
 properties dynamically renamed in your code).
 Call 'CreateLanguageFile' from the onCreate event of the last created form
 in your project.
 At this time, ALL FORMS MUST PHYSICALLY EXIST}

procedure CreateLanguageFile;
var Ini       : TMemIniFile;
    I,j,k     : integer;
    Component : TComponent;
    cmpt      : TComponent;
    Info      : TCompInfo;
    val,comp  : string;
    Filename  : string;
    F         : File;

   procedure AddUserstrings(var ini:TMemIniFile);
   var I,J,N   : Integer;
       Tmp     : array of TUserstring;
       TmpVal  : TUserstring;
       Dest    : TStringList;
       Str     : String;
   begin
      N:=Length(Userstrings);
      Setlength(tmp,N);
      for I:=1 to N do Tmp[I-1]:=Userstrings[I-1];
      Dest:=TStringList.Create;
      for I:=1 to N do
      begin
         for J:=I+1 to N do if Uppercase(Tmp[J-1].Value)<Uppercase(Tmp[I-1].Value) then
         begin
            TmpVal:=Tmp[I-1];
            Tmp[I-1]:=Tmp[J-1];
            Tmp[J-1]:=TmpVal;
         end;
         str:=IntToStr(Tmp[I-1].ID);
         while length(Str)<4 do Str:='0'+Str;
         Dest.Add('   (ID:'+Str+'; Value:'+''''+Tmp[I-1].Value+'''),');
      end;
      dest.SaveToFile('c:\test.txt');
      Dest.Destroy;
      for I:=1 to N do
      begin
         str:=IntToStr(Tmp[I-1].ID);
         while length(Str)<4 do Str:='0'+Str;
         ini.writeString('User','User'+Str,Tmp[I-1].Value);
      end;
   end;{AddUserstrings}

begin
   Filename:=extractFileDir(application.exeName)+'\'+'English.ini';
   if FileExists(Filename) then
   begin
      AssignFile(f,Filename);
      Erase(F);
   end;
	ini:=TMemIniFile.create(Filename);
   Ini.WriteString('Translation','Author','Translation: <Your name>');
   {First search the properties that will be translated}
   for i := 0 to application.ComponentCount - 1 do
   begin
   	Component:=application.Components[i];
      Info:=GetCompInfo(Component);
      if Info.Action<>'' then
      begin
      end else if Info.Caption<>'' then
      begin
         ini.writeString(Component.ClassName,Component.ClassName+'.Caption',Info.Caption);
      end;

      for J:=1 to Component.componentCount do
      begin
      	Cmpt:=Component.Components[J-1];
         Info:=GetCompInfo(Cmpt);
         //use a underscore (_) prefixed name if you want the component won't appear
         //in the list (ie : _Label6:TLabel) so as the list won't be too big
         if Info.CompName[1]='_' then continue;

         if Info.Action<>'' then
         begin
         end else
         begin
            if Info.Caption<>'' then
            begin
               if ((Info.Caption<>Info.CompName) and (Info.Caption<>'-')) or
                  (Cmpt is TAction) or (Cmpt is TMenuItem) or (Cmpt is TTabsheet) then
                  if (ValidString(Info.Caption)) then Ini.WriteString(Component.ClassName,Component.ClassName+'.'+Info.CompName+'.Caption',Info.Caption);
            end;
            if Info.Hint<>'' then Ini.WriteString(component.classname,component.classname+'.'+Info.CompName+'.Hint',Info.Hint);
         end;

      	if (Cmpt is TCustomMemo) then
         begin
            for K:=1 to TCustomMemo(cmpt).Lines.count do
            begin
            	val:=TCustomMemo(cmpt).Lines[K-1];
               if Val=Info.CompName then break;
               comp:=Info.CompName+'.Lines['+intToStr(K-1)+']';
               ini.writeString(component.classname,component.classname+'.'+comp,val);
            end;
         end else if (cmpt is TRadioGroup) then
         begin
            for K:=1 to TRadioGroup(cmpt).Items.Count do
            begin
               val:=TRadioGroup(cmpt).Items[K-1];
               comp:=Info.CompName+'.Items['+IntToStr(K-1)+']';
               ini.writeString(component.classname,component.classname+'.'+comp,val);
            end;
         end else if (cmpt is TComboBox) then
         begin
            for K:=1 to TComboBox(cmpt).Items.Count do
            begin
               val:=TComboBox(cmpt).Items[K-1];
               comp:=Info.CompName+'.Items['+IntToStr(K-1)+']';
               ini.writeString(component.classname,Component.ClassName+'.'+comp,val);
            end;
         end;
      end;
   end;
   // search for error or information messages stored as const or resourcestring - see below}
   AddUserstrings(ini);
   Ini.UpdateFile;
   ini.free;
end;{CreateLanguageFile}

function UserString(Index:Integer):String;
var Str     : string;
    Section : String;
    val     : string;
    I,N     : Integer;
begin
   Result:='';
   if CurrentLanguage<>nil then
   begin
      Section:='User';
      Val:=IntToStr(Index);
      while length(val)<4 do val:='0'+val;
      Val:='User'+Val;
      Str:=CurrentLanguage.readString(Section,Val,'');
      if Str<>'' then Result:=Str;
   end;
   if Result='' then
   begin
      N:=length(UserStrings);
      for I:=1 to N do if UserStrings[I-1].ID=index then
      begin
         Result:=UserStrings[I-1].Value;
         break;
      end;
   end;
end;{UserString}

{Translates the forms you choose in the language called in ini.
Only created forms are translated with ShowTranslatedValues. Call it in the onShow
event of your main form whith names of all automatically created forms
at the start-up of your application in the TC parameter.
In runtime, call it when you create dynamically a form.
See demo for a sample}
procedure ShowTranslatedValues(Component:TComponent);
var I,J     : integer;
    Index   : Integer;
    Comp    : TComponent;
    Str,Tmp : string;
begin
   if (CurrentLanguage=nil) or (Component=nil) then exit;
   with CurrentLanguage do
   begin
      Str:=readString(Component.Classname,Component.Classname+'.Caption','');
      if Str<>'' then TForm(Component).caption:=Str;
      for i:=0 to Component.componentCount-1 do
      begin
   		comp:=Component.Components[i];

         Str:=readString(Component.Classname,Component.Classname+'.'+comp.name+'.Caption','');
         if Str<>'' then setProp(comp,'Caption',Str);

         Str:=readString(Component.ClassName,Component.Classname+'.'+comp.name+'.Hint','');
         if Str<>'' then setProp(comp,'Hint',Str);

         if comp is TCustomMemo then
         begin
         	for J:=0 to TCustomMemo(comp).lines.count-1 do
            begin
            	Str:=readString(Component.classname,
                  Component.classname+'.'+comp.name+'.lines['+intToStr(J)+']','fsdef');
               //in TMemo or TRichEdit, you may have to leave some lines empty
               if Str<>'fsdef' then TCustomMemo(comp).lines[J]:=Str;
            end;
         end else if comp is TRadioGroup then
         begin
         	for J:=0 to TRadioGroup(comp).items.count-1 do
            begin
            	Str:=readString(Component.classname,
                  Component.classname+'.'+comp.name+'.items['+intToStr(J)+']','');
               if Str<>'' then TRadioGroup(comp).items[J]:=Str;
            end;
         end else if comp is TComboBox then
         begin
            Index:=TComboBox(comp).ItemIndex;
            TComboBox(comp).Items.BeginUpdate;
            try
            	for J:=0 to TComboBox(comp).items.count-1 do
               begin
                  Tmp:=Component.classname+'.'+comp.name+'.items['+IntToStr(J)+']';
               	Str:=readString(Component.classname,Tmp,'');
               	if Str<>'' then TComboBox(comp).items[J]:=Str;
               end;
            finally
               TComboBox(comp).Items.EndUpdate;
               TComboBox(comp).ItemIndex:=Index;
            end;
         end;
   	end;
   end;
end;{ShowTranslatedValues}

//Backs up the prop property value of the comp component
function GetProperty(comp:TComponent;prop:string):string;
var ppi:PPropInfo;
    P:TPropInfo;
begin
	ppi:=getPropInfo(comp.classInfo,prop);
   if ppi<>nil then P:=ppi^;
   if ppi<>nil then result:=getStrProp(comp,ppi)
               else result:='';
end;

//Assign the value value to prop property of comp component
procedure setProp(comp:TComponent;{const }prop,value:string);
var ppi:PPropInfo;
begin
	if value<>'' then
   begin
   	ppi:=getPropInfo(comp.classInfo,prop);
      if ppi<>nil then setStrProp(comp,ppi,value);
   end;
end;

//True if prop property exists for comp component
function HasProperty(comp:TComponent;prop:string):boolean;
begin
	result:=(getPropInfo(comp.classInfo,prop)<>nil) and (comp.name<>'');
end;

function GetCompInfo(Component:TComponent):TCompInfo;
var ActionComp : TAction;
    Obj        : TObject;
    Index      : Integer;
begin
   Result.CompName:=Component.Name;
   if HasProperty(Component,'Action') then
   begin
      Obj:=GetObjectProp(Component,'Action');
      if Obj<>nil then
      begin
         ActionComp:=Obj as TAction;
         Result.Action:=ActionComp.Name;
      end else Result.Action:='';

   end else Result.Action:='';
   if HasProperty(Component,'Caption') then
   begin
      Result.Caption:=GetProperty(Component,'Caption');
      Index:=pos('&',Result.Caption);
      if Index<>0 then Delete(result.Caption,Index,1);
   end else Result.Caption:='';
   if HasProperty(Component,'Hint') then Result.Hint:=GetProperty(Component,'Hint')
                                    else Result.Hint:='';
end;{GetCompInfo}

initialization
   CurrentLanguage:=nil;

finalization
   if CurrentLanguage<>nil then CurrentLanguage.Free;

end.
