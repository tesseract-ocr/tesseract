; (C) Copyright 2010, Sergey Bronnikov
; Contrib: Zdenko Podobný (C) 2010
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
; - insert sexy tesseract logos and artwork in installer instead default pictures ;)
; - add installation of language files
; - improve localization
; - add logging functions 
; - "is user admin" detection
; - replace hardcoded program name to variables (NAME and LONGNAME)
; - place shortcuts in program files for all users

  !define VERSION 3.00
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
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue-full.ico"
  !include "EnvVarUpdate.nsh"
  !include Sections.nsh

!macro Download_Lang_Data Lang
  StrCpy $1 ${Lang}
  StrCpy $2 "$INSTDIR\tessdata\$1"
  inetc::get /caption "Downloading $1" /popup "" "http://tesseract-ocr.googlecode.com/files/$1" $2 /end
    Pop $0 # return value = exit code, "OK" if OK
    StrCmp $0 "OK" dlok
    MessageBox MB_OK|MB_ICONEXCLAMATION "http download error. Download Status of $1: $0. Click OK to continue." /SD IDOK
	Goto error
  dlok:
	ExecWait  '"$INSTDIR\gzip.exe" -d "$2"'
  error:
!macroend

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
  ; mark as read only component
  SectionIn RO
  SetOutPath "$INSTDIR"
  ;files inclided in distribution
  File leptonlib.dll
  File tesseract.exe
  File gzip.exe  # for exctracting language data
  CreateDirectory "$INSTDIR\tessdata"
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
  ;WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Tesseract-OCR" "$INSTDIR\tesseract.exe"

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

; Download language files
SectionGroup "Language data" SecGrp_LD
	Section "English language data" SecLang_eng
	SectionIn RO	
  	SetOutPath "$INSTDIR\tessdata"  
	File tessdata\eng.traineddata
	SectionEnd
	
	Section /o "Download and install Bulgarian language data" SecLang_bul
	!insertmacro Download_Lang_Data bul.traineddata.gz
	SectionEnd

	Section /o "Download and install Catalan language data" SecLang_cat
	!insertmacro Download_Lang_Data cat.traineddata.gz
	SectionEnd

	Section /o "Download and install Czech language data" SecLang_ces
	!insertmacro Download_Lang_Data ces.traineddata.gz
	SectionEnd

	Section /o "Download and install Chinese (Traditional) language data" SecLang_chi_tra
	!insertmacro Download_Lang_Data chi_tra.traineddata.gz
	SectionEnd

	Section /o "Download and install Chinese (Simplified) language data" SecLang_chi_sim
	!insertmacro Download_Lang_Data chi_sim.traineddata.gz
	SectionEnd

	Section /o "Download and install Danish language data" SecLang_dan
	!insertmacro Download_Lang_Data dan.traineddata.gz
	SectionEnd

	Section /o "Download and install Danish (Fraktur) language data" SecLang_dan_frak
	!insertmacro Download_Lang_Data dan-frak.traineddata.gz
	SectionEnd

	Section /o "Download and install Dutch language data" SecLang_nld
	!insertmacro Download_Lang_Data nld.traineddata.gz
	SectionEnd

	Section /o "Download and install German language data" SecLang_deu
	!insertmacro Download_Lang_Data deu.traineddata.gz
	SectionEnd

	Section /o "Download and install Greek language data" SecLang_ell
	!insertmacro Download_Lang_Data ell.traineddata.gz
	SectionEnd

	Section /o "Download and install Finnish language data" SecLang_fin
	!insertmacro Download_Lang_Data fin.traineddata.gz
	SectionEnd

	Section /o "Download and install French language data" SecLang_fra
	!insertmacro Download_Lang_Data fra.traineddata.gz
	SectionEnd

	Section /o "Download and install Hungarian language data" SecLang_hun
	!insertmacro Download_Lang_Data hun.traineddata.gz
	SectionEnd

	Section /o "Download and install Indonesian language data" SecLang_ind
	!insertmacro Download_Lang_Data ind.traineddata.gz
	SectionEnd

	Section /o "Download and install Italian language data" SecLang_ita
	!insertmacro Download_Lang_Data ita.traineddata.gz
	SectionEnd

	Section /o "Download and install Japanese language data" SecLang_jpn
	!insertmacro Download_Lang_Data jpn.traineddata.gz
	SectionEnd

	Section /o "Download and install Korean language data" SecLang_kor
	!insertmacro Download_Lang_Data kor.traineddata.gz
	SectionEnd

	Section /o "Download and install Latvian language data" SecLang_lav
	!insertmacro Download_Lang_Data lav.traineddata.gz
	SectionEnd

	Section /o "Download and install Lithuanian language data" SecLang_lit
	!insertmacro Download_Lang_Data lit.traineddata.gz
	SectionEnd

	Section /o "Download and install Norwegian language data" SecLang_nor
	!insertmacro Download_Lang_Data nor.traineddata.gz
	SectionEnd

	Section /o "Download and install Polish language data" SecLang_pol
	!insertmacro Download_Lang_Data pol.traineddata.gz
	SectionEnd

	Section /o "Download and install Portuguese language data" SecLang_por
	!insertmacro Download_Lang_Data por.traineddata.gz
	SectionEnd

	Section /o "Download and install Romanian language data" SecLang_ron
	!insertmacro Download_Lang_Data ron.traineddata.gz
	SectionEnd

	Section /o "Download and install Russian language data" SecLang_rus
	!insertmacro Download_Lang_Data rus.traineddata.gz
	SectionEnd

	Section /o "Download and install Slovak language data" SecLang_slk
	!insertmacro Download_Lang_Data slk.traineddata.gz
	SectionEnd

	Section /o "Download and install Slovenian language data" SecLang_slv
	!insertmacro Download_Lang_Data slv.traineddata.gz
	SectionEnd

	Section /o "Download and install Spanish language data" SecLang_spa
	!insertmacro Download_Lang_Data spa.traineddata.gz
	SectionEnd

	Section /o "Download and install Serbian language data" SecLang_srp
	!insertmacro Download_Lang_Data srp.traineddata.gz
	SectionEnd

	Section /o "Download and install Swedish language data" SecLang_swe
	!insertmacro Download_Lang_Data swe.traineddata.gz
	SectionEnd

	Section /o "Download and install Tagalog language data" SecLang_tgl
	!insertmacro Download_Lang_Data tgl.traineddata.gz
	SectionEnd

	Section /o "Download and install Turkish language data" SecLang_tur
	!insertmacro Download_Lang_Data tur.traineddata.gz
	SectionEnd

	Section /o "Download and install Ukrainian language data" SecLang_ukr
	!insertmacro Download_Lang_Data ukr.traineddata.gz
	SectionEnd

	Section /o "Download and install Vietnamese language data" SecLang_vie
	!insertmacro Download_Lang_Data vie.traineddata.gz
	SectionEnd
SectionGroupEnd
;--------------------------------
;Descriptions
  ; At first we need to localize installer for languages which supports well in tesseract: Eng, Spa, Ger, Ita, Dutch + Russian (it is authors native language)
  ;Language strings
  LangString DESC_SecDummy ${LANG_RUSSIAN} "Установочные файлы."
  ;LangString DESC_SecHelp ${LANG_RUSSIAN} "Справочная информация."
  LangString DESC_SecCS    ${LANG_RUSSIAN} "Добавить ярлыки в меню Пуск"
 
  LangString DESC_SecDummy ${LANG_ENGLISH} "Installation files."
  ;LangString DESC_SecHelp ${LANG_ENGLISH} "Help information."
  LangString DESC_SecCS    ${LANG_ENGLISH} "Add shortcuts to Start menu."

  LangString DESC_SecDummy ${LANG_ITALIAN} "File di installazione."
  ;LangString DESC_SecHelp ${LANG_ITALIAN} "Guida di informazioni."
  LangString DESC_SecCS    ${LANG_ITALIAN} "Aggiungere collegamenti al menu Start."

  LangString DESC_SecDummy ${LANG_SLOVAK} "Súbory inštalácie."
  ;LangString DESC_SecHelp ${LANG_ENGLISH} "Pomocné informácie."
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
  ;DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Run\Tesseract-OCR"
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

Function .onInit
  ; is tesseract already installed?
  readRegStr $R0 HKCU "Software\Tesseract-OCR" "CurrentVersion"
  StrCmp $R0 "" SkipUnInstall

  MessageBox MB_YESNO|MB_ICONEXCLAMATION "Tesseract-ocr version $R0 is installed! Do you want to uninstall it first?$\nUninstall will delete all files in '$INSTDIR'!" \
	 /SD IDYES IDNO SkipUnInstall IDYES UnInstall
  UnInstall:
	readRegStr $R1 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString"
	ClearErrors
	ExecWait '$R1 _?=$INSTDIR'
  SkipUnInstall:
	  
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
    ; Make selection based on System language ID
    System::Call 'kernel32::GetSystemDefaultLangID() i .r0'
    ;http://msdn.microsoft.com/en-us/library/dd318693%28v=VS.85%29.aspx
    StrCmp $0 "1026" Bulgarian
    StrCmp $0 "1027" Catalan
    StrCmp $0 "1029" Czech
    StrCmp $0 "31748" Chinese_tra
    StrCmp $0 "4" Chinese_sim
    StrCmp $0 "1030" Danish
    StrCmp $0 "2067" Dutch
    StrCmp $0 "3079" German
    StrCmp $0 "1032" Greek
    StrCmp $0 "1035" Finnish
    StrCmp $0 "2060" French
    StrCmp $0 "1038" Hungarian
    StrCmp $0 "1057" Indonesian
    StrCmp $0 "1040" Italian
    StrCmp $0 "1041" Japanese
    StrCmp $0 "1042" Korean	
    StrCmp $0 "1062" Latvian
    StrCmp $0 "1063" Lithuanian
    StrCmp $0 "1044" Norwegian
    StrCmp $0 "1045" Polish
    StrCmp $0 "1046" Portuguese
    StrCmp $0 "1048" Romanian
    StrCmp $0 "1049" Russian     
    StrCmp $0 "1051" Slovak
    StrCmp $0 "1060" Slovenian
    StrCmp $0 "11274" Spanish
    StrCmp $0 "2074" Serbian
    StrCmp $0 "2077" Swedish
    ;StrCmp $0 "0000" Tagalog
    StrCmp $0 "1055" Turkish
    StrCmp $0 "1058" Ukrainian
    StrCmp $0 "1066" Vietnamese
    
    Goto lang_end
    
    Bulgarian: !insertmacro SelectSection ${SecLang_bul}
                Goto lang_end
    Catalan: !insertmacro SelectSection ${SecLang_cat}
                Goto lang_end
    Czech: !insertmacro SelectSection ${SecLang_ces}
                Goto lang_end
    Chinese_tra: !insertmacro SelectSection ${SecLang_chi_tra}
                Goto lang_end
    Chinese_sim: !insertmacro SelectSection ${SecLang_chi_sim}
                Goto lang_end
    Danish: !insertmacro SelectSection ${SecLang_dan}
            !insertmacro SelectSection ${SecLang_dan_frak}
            Goto lang_end
    Dutch: !insertmacro SelectSection ${SecLang_nld}
            Goto lang_end            
    German: !insertmacro SelectSection ${SecLang_deu}
            Goto lang_end
    Greek: !insertmacro SelectSection ${SecLang_ell}
            Goto lang_end            
    Finnish: !insertmacro SelectSection ${SecLang_fin}
            Goto lang_end            
    French: !insertmacro SelectSection ${SecLang_fra}
            Goto lang_end
    Hungarian: !insertmacro SelectSection ${SecLang_hun}
            Goto lang_end            
    Indonesian: !insertmacro SelectSection ${SecLang_ind}
            Goto lang_end            
    Italian: !insertmacro SelectSection ${SecLang_ita}
            Goto lang_end
    Japanese: !insertmacro SelectSection ${SecLang_jpn}
            Goto lang_end
    Korean: !insertmacro SelectSection ${SecLang_kor}
            Goto lang_end			
    Latvian: !insertmacro SelectSection ${SecLang_lav}
            Goto lang_end            
    Lithuanian: !insertmacro SelectSection ${SecLang_lit}
            Goto lang_end
    Norwegian: !insertmacro SelectSection ${SecLang_nor}
            Goto lang_end            
    Polish: !insertmacro SelectSection ${SecLang_pol}
            Goto lang_end            
    Portuguese: !insertmacro SelectSection ${SecLang_por}
            Goto lang_end
    Romanian: !insertmacro SelectSection ${SecLang_ron}
            Goto lang_end            
    Russian: !insertmacro SelectSection ${SecLang_rus}
            Goto lang_end            
    Slovak: !insertmacro SelectSection ${SecLang_slk}
            Goto lang_end
    Slovenian: !insertmacro SelectSection ${SecLang_slv}
            Goto lang_end            
    Spanish: !insertmacro SelectSection ${SecLang_spa}
            Goto lang_end            
    Serbian: !insertmacro SelectSection ${SecLang_srp}
            Goto lang_end
    Swedish: !insertmacro SelectSection ${SecLang_swe}
            Goto lang_end            
    ;Tagalog: !insertmacro SelectSection ${SecLang_tgl}
    ;        Goto lang_end            
    Turkish: !insertmacro SelectSection ${SecLang_tur}
            Goto lang_end
    Ukrainian: !insertmacro SelectSection ${SecLang_ukr}
            Goto lang_end            
    Vietnamese: !insertmacro SelectSection ${SecLang_vie}

    lang_end:
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