; (C) Copyright 2010, Sergey Bronnikov
; impoved by Zdenko Podobný (C) 2010
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
; http://www.apache.org/licenses/LICENSE-2.0
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

; TODO:
; - check that tesseract was already installed
; - insert sexy tesseract logos and artwork in installer instead default pictures ;)
; - add installation of language files
; - improve localization


  !define VERSION 3.00.1
  !define PRODUCT_NAME "Tesseract-OCR"
  !define PRODUCT_VERSION "${VERSION}"
  !define PRODUCT_PUBLISHER ""
  !define PRODUCT_WEB_SITE "http://code.google.com/p/tesseract-ocr"
  ;!define PRODUCT_DIR_REGKEY "Software/Microsoft"
  ;!define PRODUCT_UNINST_KEY "Software/Microsoft"
  ;!define PRODUCT_UNINST_ROOT_KEY "Software/Microsoft"
  SetCompressor lzma
  Name "Tesseract-OCR ${VERSION}"
  Caption "Tesseract-OCR ${VERSION}"
  ;Icon "icon_1.ico"
  ;UninstallIcon "install_icon.ico"
  BrandingText /TRIMCENTER "(c) 2010 Tesseract-OCR "
  InstallDir "$PROGRAMFILES\Tesseract-OCR"
  InstallDirRegKey HKCU "Software\Tesseract-OCR" ""
  ShowInstDetails show
  XPStyle on
  SpaceTexts
  CRCCheck on 
  InstProgressFlags smooth colored
  ;Name of program and file
  !ifdef VERSION
    OutFile tesseract-ocr-setup-${VERSION}.exe
  !else
    OutFile tesseract-ocr-setup.exe
  !endif
  !include "MUI.nsh"
  #!include MUI2.nsh
  !include "LogicLib.nsh"
  !include "EnvVarUpdate.nsh"
  
Function .onInit
  MessageBox MB_YESNO|MB_ICONQUESTION "Do you want to install ${PRODUCT_NAME} ${VERSION}?" \
    /SD IDYES IDNO no IDYES yes
  no:
    SetSilent silent
    Goto done
  yes:
    SetSilent normal 
    ;InitPluginsDir
    ;File /oname=$PLUGINSDIR\splash.bmp "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
    ;File /oname=$PLUGINSDIR\splash.bmp "new.bmp"
    ;advsplash::show 1000 600 400 -1 $PLUGINSDIR\splash
    ;Pop $0          ; $0 has '1' if the user closed the splash screen early,
                    ; '0' if everything closed normal, and '-1' if some error occured.
    ;IfFileExists $INSTDIR\loadmain.exe PathGood
  done:
      # check if tesseract is not installed
      readRegStr $R0 HKCU "Software\Tesseract-OCR" "CurrentVersion"
	  StrCmp $R0 "" done2

	  MessageBox MB_YESNO|MB_ICONEXCLAMATION "Tesseract-ocr version $R0 is installed! Do you want to uninstall it first?" \
	     /SD IDYES IDNO done2 IDYES yes2
	  yes2:
		readRegStr $R1 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString"
		ClearErrors
		ExecWait '$R1 _?=$INSTDIR'
	  done2:
FunctionEnd

Function un.onInit
   ;!insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Function .onInstFailed
  MessageBox MB_OK "Installation failed."
FunctionEnd

Function ShowReadme
  Exec "explorer.exe $INSTDIR\doc\README"
  BringToFront
FunctionEnd

;MUI Settings
  !define MUI_ABORTWARNING
  ;!define MUI_ICON "install_icon.ico"
  ;!define MUI_UNICON "uninstall_icon.ico"
;Welcome page
  !define MUI_HEADERIMAGE
  ;
  ;!define MUI_HEADERIMAGE_BITMAP  "452.bmp"
  ;!define MUI_HEADERIMAGE_UNBITMAP  "452.bmp"
  !define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
  ;;!define MUI_HEADERIMAGE_RIGHT
  ;!define MUI_WELCOMEFINISHPAGE_BITMAP  "logo.bmp"
  !define MUI_WELCOMEPAGE_TITLE_3LINES
  !define MUI_COMPONENTSPAGE_SMALLDESC

;--------------------------------
;Pages
;License page
  !define MUI_LICENSEPAGE_CHECKBOX
  ;!define MUI_LICENSEPAGE_TEXT "$(License)"
  ;!insertmacro MUI_PAGE_LICENSE "${MUI_LICENSEPAGE_TEXT}"  
  !insertmacro MUI_PAGE_LICENSE "doc/COPYING"
!ifdef VERSION
  Page custom PageReinstall PageLeaveReinstall
!endif
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !define MUI_FINISHPAGE_LINK "http://code.google.com/p/tesseract-ocr/"
  !define MUI_FINISHPAGE_LINK_LOCATION "http://code.google.com/p/tesseract-ocr/"
  ;!define MUI_FINISHPAGE_RUN "$INSTDIR\tesseract.exe"
  ;!define MUI_FINISHPAGE_NOREBOOTSUPPORT
  !define MUI_FINISHPAGE_SHOWREADME "notepad $INSTDIR\doc\README"
  !define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReadme
  !insertmacro MUI_PAGE_FINISH
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  ;!insertmacro MUI_UNPAGE_FINISH
 
;--------------------------------
;Languages
  !insertmacro MUI_LANGUAGE "English"
  ;!insertmacro MUI_LANGUAGE "Indonesian"
  ;!insertmacro MUI_LANGUAGE "Irish"
  !insertmacro MUI_LANGUAGE "Italian"
  ;!insertmacro MUI_LANGUAGE "Japanese"
  ;!insertmacro MUI_LANGUAGE "Korean"
  ;!insertmacro MUI_LANGUAGE "Kurdish"
  ;!insertmacro MUI_LANGUAGE "Latvian""
  ;!insertmacro MUI_LANGUAGE "Lithuanian"
  ;!insertmacro MUI_LANGUAGE "Luxembourgish"
  ;!insertmacro MUI_LANGUAGE "Macedonian"
  ;!insertmacro MUI_LANGUAGE "Malay"
  ;!insertmacro MUI_LANGUAGE "Mongolian"
  ;!insertmacro MUI_LANGUAGE "Norwegian"
  ;!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
  ;!insertmacro MUI_LANGUAGE "Polish"
  ;!insertmacro MUI_LANGUAGE "Portuguese"
  ;!insertmacro MUI_LANGUAGE "PortugueseBR"
  ;!insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Russian"
  ;!insertmacro MUI_LANGUAGE "Serbian"
  ;!insertmacro MUI_LANGUAGE "SerbianLatin"
  ;!insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "Slovak"
  ;!insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SpanishInternational"
  ;!insertmacro MUI_LANGUAGE "Swedish"
  ;!insertmacro MUI_LANGUAGE "Thai"
  ;!insertmacro MUI_LANGUAGE "TradChinese"
  ;!insertmacro MUI_LANGUAGE "Turkish"
  ;!insertmacro MUI_LANGUAGE "Ukrainian"
  ;!insertmacro MUI_LANGUAGE "Uzbek"
  ;!insertmacro MUI_LANGUAGE "Welsh"

;--------------------------------

;Installer Sections
ShowInstDetails show
InstProgressFlags smooth colored

Section "Tesseract-OCR" SecDummy
  SetOutPath "$INSTDIR"
  ;files inclided in distribution
  File leptonlib.dll
  File tesseract.exe
  CreateDirectory "$INSTDIR\tessdata"
  SetOutPath "$INSTDIR\tessdata"  
  File tessdata\eng.traineddata  
  CreateDirectory "$INSTDIR\tessdata\configs"
  SetOutPath "$INSTDIR\tessdata\configs"
  File tessdata\configs\ambigs.train
  File tessdata\configs\api_config
  File tessdata\configs\box.train
  File tessdata\configs\box.train.stderr
  File tessdata\configs\digits
  File tessdata\configs\inter
  File tessdata\configs\kannada
  File tessdata\configs\logfile
  File tessdata\configs\makebox
  File tessdata\configs\unlv
  CreateDirectory "$INSTDIR\tessdata\tessconfigs"
  SetOutPath "$INSTDIR\tessdata\tessconfigs"
  File tessdata\tessconfigs\batch
  File tessdata\tessconfigs\batch.nochop
  File tessdata\tessconfigs\matdemo
  File tessdata\tessconfigs\msdemo
  File tessdata\tessconfigs\nobatch
  File tessdata\tessconfigs\segdemo
  CreateDirectory "$INSTDIR\training"
  SetOutPath "$INSTDIR\training"
  File training\cntraining.exe
  File training\combine_tessdata.exe
  File training\mftraining.exe
  File training\unicharset_extractor.exe
  File training\wordlist2dawg.exe
  CreateDirectory "$INSTDIR\doc"
  SetOutPath "$INSTDIR\doc"
  File doc\AUTHORS
  File doc\COPYING
  File doc\eurotext.tif
  File doc\phototest.tif
  File doc\README
  File doc\ReleaseNotes
  ;Store installation folder
  WriteRegStr HKCU "Software\Tesseract-OCR" "InstallDir" $INSTDIR
  WriteRegStr HKCU "Software\Tesseract-OCR" "CurrentVersion" "${VERSION}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Tesseract-OCR" "C:\Program Files\Tesseract-OCR\tesseract.exe"

  ; include for some of the windows messages defines
  !include "winmessages.nsh"
  ; HKLM (all users) vs HKCU (current user) defines
  !define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
  !define env_hkcu 'HKCU "Environment"'

   ; set variable
   ; append bin path to user PATH environment variable
   ReadRegStr $0 HKCU "Environment" "PATH"
   WriteRegExpandStr HKCU "Environment" "PATH" "$INSTDIR;$INSTDIR\training;$0"
   #${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$0;$INSTDIR" # this command destroys long variables like path...
   ${EnvVarUpdate} $0 "TESSDATA_PREFIX" "A" "HKCU" "$INSTDIR\" 
   ; make sure windows knows about the change
   SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
   
  ; Would be nice to download language files during installation (as Google Chrome do)
  ; Download language files
  ;StrCpy $2 "$INSTDIR\$1"
  ;StrCpy $1 "tesseract-2.00.deu.tar.gz"
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/tesseract-2.00.deu.tar.gz" $INSTDIR 
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/$1" $2 
  ;StrCpy $1 "tesseract-2.00.eng.tar.gz"
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/$1" $2 
  ;StrCpy $1 "tesseract-2.00.fra.tar.gz"
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/$1" $2 
  ;StrCpy $1 "tesseract-2.00.ita.tar.gz"
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/$1" $2 
  ;StrCpy $1 "tesseract-2.00.nld.tar.gz"
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/$1" $2 
  ;StrCpy $1 "tesseract-2.01.por.tar.gz"
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/$1" $2 
  ;StrCpy $1 "tesseract-2.00.spa.tar.gz"
  ;NSISdl::download "http://tesseract-ocr.googlecode.com/files/$1" $2 
  ;Quit 
   
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  ;ExecShell "open" "http://code.google.com/p/tesseract-ocr/"
  ;ExecShell "open" '"$INSTDIR"' 
  ;BringToFront
  ; Register to Add/Remove program in control panel
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME} ${VERSION} - open source OCR engine"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
  
SectionEnd

;Section "Help" SecHelp
  ;CreateDirectory "$INSTDIR\Help"
  ;SetOutPath "$INSTDIR\Help"
  ;File "README"
  ;File "COPYING"
;SectionEnd

Section "Shortcuts creation" SecCS
  CreateDirectory "$SMPROGRAMS\Tesseract-OCR"
  CreateShortCut  "$SMPROGRAMS\Tesseract-OCR\Tesseract-OCR.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
  CreateShortCut  "$SMPROGRAMS\Tesseract-OCR\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ;CreateShortCut "$DESKTOP\Tesseract-OCR.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
  ;CreateShortCut "$QUICKLAUNCH\.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
SectionEnd

;Section "Languages support" SecLanguage
;SectionEnd

;--------------------------------
;Descriptions
  ; At first we need to localize installer for languages which supports well in tesseract: Eng, Spa, Ger, Ita, Dutch + Russian (it is authors native language)
  ;Language strings
  LangString DESC_SecDummy ${LANG_RUSSIAN} "???????????? ?????."
  ;LangString DESC_SecHelp ${LANG_RUSSIAN} "?????????? ??????????."
  LangString DESC_SecCS    ${LANG_RUSSIAN} "???????? ?????? ???? ????"

  LangString DESC_SecDummy ${LANG_ENGLISH} "Installation files."
  ;LangString DESC_SecHelp ${LANG_ENGLISH} "Help information."
  LangString DESC_SecCS    ${LANG_ENGLISH} "Add shortcuts to Start menu."

  LangString DESC_SecDummy ${LANG_ITALIAN} "File di installazione."
  ;LangString DESC_SecHelp ${LANG_ITALIAN} "Guida di informazioni."
  LangString DESC_SecCS    ${LANG_ITALIAN} "Aggiungere collegamenti al menu Start."

  LangString DESC_SecDummy ${LANG_SLOVAK} "Súbory inštalácie."
  ;LangString DESC_SecHelp ${LANG_ENGLISH} "Help information."
  LangString DESC_SecCS    ${LANG_SLOVAK} "Pridať odkaz do Start menu."
  
  LangString DESC_SecDummy ${LANG_SPANISH} "Los archivos de instalación."
  ;LangString DESC_SecHelp ${LANG_SPANISH} "Información de ayuda."
  LangString DESC_SecCS    ${LANG_SPANISH} "Ańadir accesos directos al menú Inicio."

  LangString DESC_SecDummy ${LANG_SPANISHINTERNATIONAL} "Los archivos de instalación."
  ;LangString DESC_SecHelp ${LANG_SPANISHINTERNATIONAL} "Información de ayuda."
  LangString DESC_SecCS    ${LANG_SPANISHINTERNATIONAL} "Ańadir accesos directos al menú Inicio."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCS} $(DESC_SecCS)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  !define MUI_FINISHPAGE_SHOWREADME
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "Create desktop shortcut"
  !define MUI_FINISHPAGE_SHOWREADME_FUNCTION CreateDeskShortcut
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Run\Tesseract-OCR"
  DeleteRegKey /ifempty HKCU "Software\Tesseract-OCR"
  ; delete variable
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" $INSTDIR
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" "$INSTDIR\training"
  DeleteRegValue ${env_hklm} "TESSDATA_PREFIX"
  ; make sure windows knows about the change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

  Delete "$INSTDIR\*.*"  
  Delete "$SMPROGRAMS\Tesseract-OCR\*.*"
  Delete "$INSTDIR\training\*.*"
  Delete "$INSTDIR\doc\*.*"
  Delete "$INSTDIR\tessdata\*.*"
  Delete "$INSTDIR\tessdata\configs\*.*"
  Delete "$INSTDIR\tessdata\tessconfigs\*.*"
  RMDir  "$INSTDIR\training"
  RMDir  "$INSTDIR\doc"
  RMDir  "$INSTDIR\tessdata\configs"
  RMDir  "$INSTDIR\tessdata\tessconfigs"
  RMDir  "$INSTDIR\tessdata"
  ;Delete "$DESKTOP\Tesseract-OCR.lnk"
  ;Delete "$QUICKLAUNCH\Tesseract-OCR.lnk" 
  RMDir "$INSTDIR"
  RMDir "$SMPROGRAMS\Tesseract-OCR"

  ; remove the Add/Remove information
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
  
SectionEnd

Function PageReinstall

FunctionEnd

Function PageLeaveReinstall

FunctionEnd