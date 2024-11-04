; (C) Copyright 2010, Sergey Bronnikov
; (C) Copyright 2010-2012, Zdenko Podobný
; (C) Copyright 2015-2024 Stefan Weil
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

; Links to NSIS documentation:
; https://nsis.sourceforge.io/Docs/Modern%20UI%202/Readme.html

; TODO:
; * Fix PreventMultipleInstances.
; * Add Tesseract icon and images for installer.

SetCompressor /FINAL /SOLID lzma
SetCompressorDictSize 32

Unicode true

; Settings which normally should be passed as command line arguments.
;define CROSSBUILD
;define SHARED
;define W64
!ifndef COMMENTS
!define COMMENTS "GitHub CI build"
!endif
!ifndef COMPANYNAME
!define COMPANYNAME "Open Source Community"
!endif
!ifndef SRCDIR
!define SRCDIR .
!endif
!ifndef VERSION
!define VERSION undefined
!endif

!define PRODUCT_NAME "Tesseract-OCR"
!define PRODUCT_VERSION "${VERSION}"
!define PRODUCT_PUBLISHER "Tesseract-OCR community"
!ifndef PRODUCT_WEB_SITE
!define PRODUCT_WEB_SITE "https://github.com/tesseract-ocr/tesseract"
!endif
!define GITHUB_RAW_FILE_URL \
  "https://raw.githubusercontent.com/tesseract-ocr/tessdata_fast/main"

!ifdef CROSSBUILD
!addincludedir ${SRCDIR}\nsis\include
!addplugindir Plugins/x86-unicode
!endif

!ifdef W64
!define ARCH "x86_64"
!define SETUP "tesseract-ocr-w64-setup"
!else
!define ARCH "i686"
!define SETUP "tesseract-ocr-w32-setup"
!endif

# Name of program and file
!define OUTFILE "${SETUP}-${VERSION}.exe"
OutFile ${OUTFILE}

!ifdef SIGNCODE
!finalize "${SIGNCODE} %1"
!uninstfinalize "${SIGNCODE} %1"
!endif

!ifndef PREFIX
!define PREFIX "../mingw64"
!endif
!define BINDIR "${PREFIX}/bin"

# General Definitions
Name "${PRODUCT_NAME}"
Caption "${PRODUCT_NAME} ${VERSION}"
!ifndef CROSSBUILD
BrandingText /TRIMCENTER "(c) 2010-2019 ${PRODUCT_NAME}"
!endif

; File properties.
!define /date DATEVERSION "%Y%m%d%H%M%S"
VIProductVersion "${VERSION}"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "Comments" "${COMMENTS}"
VIAddVersionKey "CompanyName" "${COMPANYNAME}"
VIAddVersionKey "FileDescription" "Tesseract OCR"
!define /date DATETIME "%Y-%m-%d-%H-%M-%S"
VIAddVersionKey "FileVersion" "${DATETIME}"
VIAddVersionKey "InternalName" "Tesseract"
VIAddVersionKey "LegalCopyright" "Apache-2.0"
#VIAddVersionKey "LegalTrademarks" ""
VIAddVersionKey "OriginalFilename" "${OUTFILE}"
VIAddVersionKey "ProductVersion" "${VERSION}"

!define REGKEY "SOFTWARE\${PRODUCT_NAME}"
; HKLM (all users) vs HKCU (current user) defines
!define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
!define env_hkcu 'HKCU "Environment"'

# MultiUser Symbol Definitions
# https://nsis.sourceforge.io/Docs/MultiUser/Readme.html
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${REGKEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME MultiUserInstallMode
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_INSTDIR ${PRODUCT_NAME}
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${REGKEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUE "Path"
!ifdef W64
!define MULTIUSER_USE_PROGRAMFILES64
!endif

# MUI Symbol Definitions
!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue-full.ico"
!define MUI_FINISHPAGE_LINK "View Tesseract on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/tesseract-ocr/tesseract"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!ifdef SHOW_README
; Showing the README does not work.
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\doc\README.md"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReadme
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show README"
!endif
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY ${REGKEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER ${PRODUCT_NAME}
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_WELCOMEPAGE_TITLE_3LINES

# Included files
!include MultiUser.nsh
!include Sections.nsh
!include MUI2.nsh
!include LogicLib.nsh
!include winmessages.nsh # include for some of the windows messages defines

# Variables
Var StartMenuGroup
; Define user variables
Var OLD_KEY

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${SRCDIR}\LICENSE"
!insertmacro MULTIUSER_PAGE_INSTALLMODE
  Page custom PageReinstall PageLeaveReinstall
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"

# Installer attributes
ShowInstDetails hide
InstProgressFlags smooth colored
XPStyle on
SpaceTexts
CRCCheck on
InstProgressFlags smooth colored
CRCCheck On  # Do a CRC check before installing

!macro Download_Lang_Data Lang
  ; Download traineddata file.
  DetailPrint "Download: ${Lang} language file"
  inetc::get /caption "Downloading ${Lang} language file" \
      "${GITHUB_RAW_FILE_URL}/${Lang}.traineddata" $INSTDIR/tessdata/${Lang}.traineddata \
      /END
    Pop $0 # return value = exit code, "OK" if OK
    StrCmp $0 "OK" +2
    MessageBox MB_OK|MB_ICONEXCLAMATION \
      "Download error. Status of ${Lang}: $0. Click OK to continue." /SD IDOK
!macroend

Section -Main SEC0000
  ; mark as read only component
  SectionIn RO
  SetOutPath "$INSTDIR"
  # files included in distribution
  File ${BINDIR}/tesseract.exe
  File ${BINDIR}/libtesseract-*.dll
!ifdef CROSSBUILD
  File ../dll/*.dll
!endif
  File winpath.exe
  File ../doc/*.html
  CreateDirectory "$INSTDIR\tessdata"
  SetOutPath "$INSTDIR\tessdata"
  File ${PREFIX}/share/tessdata/pdf.ttf
  CreateDirectory "$INSTDIR\tessdata\configs"
  SetOutPath "$INSTDIR\tessdata\configs"
  File ${PREFIX}/share/tessdata/configs/*
  CreateDirectory "$INSTDIR\tessdata\script"
  CreateDirectory "$INSTDIR\tessdata\tessconfigs"
  SetOutPath "$INSTDIR\tessdata\tessconfigs"
  File ${PREFIX}/share/tessdata/tessconfigs/*
  CreateDirectory "$INSTDIR\doc"
  SetOutPath "$INSTDIR\doc"
  File ${SRCDIR}\AUTHORS
  File ${SRCDIR}\LICENSE
  File ${SRCDIR}\README.md
##  File ${SRCDIR}\ReleaseNotes
SectionEnd

Section "ScrollView" SecScrollView
  SectionIn 1
  SetOutPath "$INSTDIR\tessdata"
  File ${PREFIX}/share/tessdata/*.jar
SectionEnd

Section "Training Tools" SecTr
  SectionIn 1
  SetOutPath "$INSTDIR"
  File /x tesseract.exe ${BINDIR}/*.exe
SectionEnd

!define UNINST_EXE "$INSTDIR\tesseract-uninstall.exe"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

Section -post SEC0001
!ifdef W64
  SetRegView 64
!endif
  ;Store installation folder - we always use HKLM!
  WriteRegStr HKLM "${REGKEY}" "Path" "$INSTDIR"
  WriteRegStr HKLM "${REGKEY}" "Mode" $MultiUser.InstallMode
  WriteRegStr HKLM "${REGKEY}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "${REGKEY}" "CurrentVersion" "${VERSION}"
  WriteRegStr HKLM "${REGKEY}" "Uninstaller" "${UNINST_EXE}"
  ;WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\tesseract.exe" "$INSTDIR\tesseract.exe"
  ;WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "Tesseract-OCR" "$INSTDIR\tesseract.exe"
  ; Register to Add/Remove program in control panel
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} - open source OCR engine"
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "${UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "${UNINST_KEY}" "DisplayIcon" "${UNINST_EXE}"
  WriteRegStr HKLM "${UNINST_KEY}" "UninstallString" "${UNINST_EXE}"
  WriteRegStr HKLM "${UNINST_KEY}" "QuietUninstallString" '"${UNINST_EXE}" /S'
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair" 1
  ;Create uninstaller
  WriteUninstaller "${UNINST_EXE}"
  ;ExecShell "open" "https://github.com/tesseract-ocr/tesseract"
  ;ExecShell "open" '"$INSTDIR"'
  ;BringToFront
SectionEnd

Section "Shortcuts creation" SecCS
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Console.lnk" "$INSTDIR\winpath.exe" "cmd"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Dokumentation.lnk" "$INSTDIR\tesseract.1.html"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Homepage.lnk" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\ReadMe.lnk" "${PRODUCT_WEB_SITE}/wiki/ReadMe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\FAQ.lnk" "${PRODUCT_WEB_SITE}/wiki/FAQ"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "${UNINST_EXE}" "" "${UNINST_EXE}" 0
  ;CreateShortCut "$DESKTOP\Tesseract-OCR.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
  ;CreateShortCut "$QUICKLAUNCH\.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
SectionEnd

; Language files
SectionGroup "Language data" SecGrp_LD
    Section "English" SecLang_eng
    SectionIn RO
      !insertmacro Download_Lang_Data eng
    SectionEnd

    Section "Orientation and script detection" SecLang_osd
    SectionIn 1
      !insertmacro Download_Lang_Data osd
    SectionEnd
SectionGroupEnd

; Download script files
SectionGroup "Additional script data (download)" SecGrp_ASD
  Section /o "Arabic script" SecLang_Arabic
    AddSize 8880
    !insertmacro Download_Lang_Data script/Arabic
  SectionEnd

  Section /o "Armenian script" SecLang_Armenian
    AddSize 7510
    !insertmacro Download_Lang_Data script/Armenian
  SectionEnd

  Section /o "Bengali script" SecLang_Bengali
    AddSize 5450
    !insertmacro Download_Lang_Data script/Bengali
  SectionEnd

  Section /o "Canadian Aboriginal script" SecLang_Canadian_Aboriginal
    AddSize 6850
    !insertmacro Download_Lang_Data script/Canadian_Aboriginal
  SectionEnd

  Section /o "Cherokee script" SecLang_Cherokee
    AddSize 4040
    !insertmacro Download_Lang_Data script/Cherokee
  SectionEnd

  Section /o "Cyrillic script" SecLang_Cyrillic
    AddSize 27900
    !insertmacro Download_Lang_Data script/Cyrillic
  SectionEnd

  Section /o "Devanagari script" SecLang_Devanagari
    AddSize 17100
    !insertmacro Download_Lang_Data script/Devanagari
  SectionEnd

  Section /o "Ethiopic script" SecLang_Ethiopic
    AddSize 8650
    !insertmacro Download_Lang_Data script/Ethiopic
  SectionEnd

  Section /o "Fraktur script" SecLang_Fraktur
    AddSize 10400
    !insertmacro Download_Lang_Data script/Fraktur
  SectionEnd

  Section /o "Georgian script" SecLang_Georgian
    AddSize 6630
    !insertmacro Download_Lang_Data script/Georgian
  SectionEnd

  Section /o "Greek script" SecLang_Greek
    AddSize 2900
    !insertmacro Download_Lang_Data script/Greek
  SectionEnd

  Section /o "Gujarati script" SecLang_Gujarati
    AddSize 4780
    !insertmacro Download_Lang_Data script/Gujarati
  SectionEnd

  Section /o "Gurmukhi script" SecLang_Gurmukhi
    AddSize 4020
    !insertmacro Download_Lang_Data script/Gurmukhi
  SectionEnd

  Section /o "Han Simplified script" SecLang_HanS
    AddSize 5700
    !insertmacro Download_Lang_Data script/HanS
  SectionEnd

  Section /o "Han Simplified vertical script" SecLang_HanS_vert
    AddSize 5304
    !insertmacro Download_Lang_Data script/HanS_vert
  SectionEnd

  Section /o "Han Traditional script" SecLang_HanT
    AddSize 5200
    !insertmacro Download_Lang_Data script/HanT
  SectionEnd

  Section /o "Han Traditional vertical script" SecLang_HanT_vert
    AddSize 5200
    !insertmacro Download_Lang_Data script/HanT_vert
  SectionEnd

  Section /o "Hangul script" SecLang_Hangul
    AddSize 4620
    !insertmacro Download_Lang_Data script/Hangul
  SectionEnd

  Section /o "Hangul vertical script" SecLang_Hangul_vert
    AddSize 4510
    !insertmacro Download_Lang_Data script/Hangul_vert
  SectionEnd

  Section /o "Hebrew script" SecLang_Hebrew
    AddSize 4640
    !insertmacro Download_Lang_Data script/Hebrew
  SectionEnd

  Section /o "Japanese script" SecLang_Japanese
    AddSize 5610
    !insertmacro Download_Lang_Data script/Japanese
  SectionEnd

  Section /o "Japanese vertical script" SecLang_Japanese_vert
    AddSize 6150
    !insertmacro Download_Lang_Data script/Japanese_vert
  SectionEnd

  Section /o "Kannada script" SecLang_Kannada
    AddSize 6460
    !insertmacro Download_Lang_Data script/Kannada
  SectionEnd

  Section /o "Khmer script" SecLang_Khmer
    AddSize 4270
    !insertmacro Download_Lang_Data script/Khmer
  SectionEnd

  Section /o "Lao script" SecLang_Script_Lao
    AddSize 9640
    !insertmacro Download_Lang_Data script/Lao
  SectionEnd

  Section /o "Latin script" SecLang_Latin
    AddSize 85200
    !insertmacro Download_Lang_Data script/Latin
  SectionEnd

  Section /o "Malayalam script" SecLang_Malayalam
    AddSize 8590
    !insertmacro Download_Lang_Data script/Malayalam
  SectionEnd

  Section /o "Myanmar script" SecLang_Myanmar
    AddSize 7480
    !insertmacro Download_Lang_Data script/Myanmar
  SectionEnd

  Section /o "Oriya script" SecLang_Oriya
    AddSize 5480
    !insertmacro Download_Lang_Data script/Oriya
  SectionEnd

  Section /o "Sinhala script" SecLang_Sinhala
    AddSize 4560
    !insertmacro Download_Lang_Data script/Sinhala
  SectionEnd

  Section /o "Syriac script" SecLang_Syriac
    AddSize 5530
    !insertmacro Download_Lang_Data script/Syriac
  SectionEnd

  Section /o "Tamil script" SecLang_Tamil
    AddSize 6760
    !insertmacro Download_Lang_Data script/Tamil
  SectionEnd

  Section /o "Telugu script" SecLang_Telugu
    AddSize 6180
    !insertmacro Download_Lang_Data script/Telugu
  SectionEnd

  Section /o "Thaana script" SecLang_Thaana
    AddSize 5770
    !insertmacro Download_Lang_Data script/Thaana
  SectionEnd

  Section /o "Thai script" SecLang_Thai
    AddSize 4050
    !insertmacro Download_Lang_Data script/Thai
  SectionEnd

  Section /o "Tibetan script" SecLang_Tibetan
    AddSize 5440
    !insertmacro Download_Lang_Data script/Tibetan
  SectionEnd

  Section /o "Vietnamese script" SecLang_Vietnamese
    AddSize 1590
    !insertmacro Download_Lang_Data script/Vietnamese
  SectionEnd

SectionGroupEnd

; Download language files
SectionGroup "Additional language data (download)" SecGrp_ALD
  Section /o "Math / equation detection module" SecLang_equ
    AddSize 2200
    !insertmacro Download_Lang_Data equ
  SectionEnd

  ; The language names are documented here:
  ; https://github.com/tesseract-ocr/tesseract/blob/main/doc/tesseract.1.asc#languages

  Section /o "Afrikaans" SecLang_afr
    AddSize 2530
    !insertmacro Download_Lang_Data afr
  SectionEnd

  Section /o "Amharic" SecLang_amh
    AddSize 5220
    !insertmacro Download_Lang_Data amh
  SectionEnd

  Section /o "Arabic" SecLang_ara
    AddSize 1370
    !insertmacro Download_Lang_Data ara
  SectionEnd

  Section /o "Assamese" SecLang_asm
    AddSize 1950
    !insertmacro Download_Lang_Data asm
  SectionEnd

  Section /o "Azerbaijani" SecLang_aze
    AddSize 3360
    !insertmacro Download_Lang_Data aze
  SectionEnd

  Section /o "Azerbaijani (Cyrillic)" SecLang_aze_cyrl
    AddSize 1850
    !insertmacro Download_Lang_Data aze_cyrl
  SectionEnd

  Section /o "Belarusian" SecLang_bel
    AddSize 3520
    !insertmacro Download_Lang_Data bel
  SectionEnd

  Section /o "Bengali" SecLang_ben
    AddSize 836
    !insertmacro Download_Lang_Data ben
  SectionEnd

  Section /o "Tibetan" SecLang_bod
    AddSize 1880
    !insertmacro Download_Lang_Data bod
  SectionEnd

  Section /o "Bosnian" SecLang_bos
    AddSize 2380
    !insertmacro Download_Lang_Data bos
  SectionEnd

  Section /o "Breton" SecLang_bre
    AddSize 6188
    !insertmacro Download_Lang_Data bre
  SectionEnd

  Section /o "Bulgarian" SecLang_bul
    AddSize 1600
    !insertmacro Download_Lang_Data bul
  SectionEnd

  Section /o "Catalan" SecLang_cat
    AddSize 1090
    !insertmacro Download_Lang_Data cat
  SectionEnd

  Section /o "Cebuano" SecLang_ceb
    AddSize 699
    !insertmacro Download_Lang_Data ceb
  SectionEnd

  Section /o "Czech" SecLang_ces
    AddSize 3620
    !insertmacro Download_Lang_Data ces
  SectionEnd

  Section /o "Chinese (Simplified)" SecLang_chi_sim
    AddSize 2350
    !insertmacro Download_Lang_Data chi_sim
  SectionEnd

  Section /o "Chinese (Simplified vertical)" SecLang_chi_sim_vert
    AddSize 1840
    !insertmacro Download_Lang_Data chi_sim_vert
  SectionEnd

  Section /o "Chinese (Traditional)" SecLang_chi_tra
    AddSize 2260
    !insertmacro Download_Lang_Data chi_tra
  SectionEnd

  Section /o "Chinese (Traditional vertical)" SecLang_chi_tra_vert
    AddSize 1740
    !insertmacro Download_Lang_Data chi_tra_vert
  SectionEnd

  Section /o "Cherokee" SecLang_chr
    AddSize 366
    !insertmacro Download_Lang_Data chr
  SectionEnd

  Section /o "Corsican" SecLang_cos
    AddSize 2190
    !insertmacro Download_Lang_Data cos
  SectionEnd

  Section /o "Welsh" SecLang_cym
    AddSize 2110
    !insertmacro Download_Lang_Data cym
  SectionEnd

  Section /o "Danish" SecLang_dan
    AddSize 2460
    !insertmacro Download_Lang_Data dan
  SectionEnd

  Section /o "German" SecLang_deu
    AddSize 1450
    !insertmacro Download_Lang_Data deu
  SectionEnd

 Section /o "German Fraktur" SecLang_deu_latf
    AddSize 6130
    !insertmacro Download_Lang_Data deu_latf
  SectionEnd

  Section /o "Divehi" SecLang_div
    AddSize 1690
    !insertmacro Download_Lang_Data div
  SectionEnd

  Section /o "Dzongkha" SecLang_dzo
    AddSize 439
    !insertmacro Download_Lang_Data dzo
  SectionEnd

  Section /o "Greek" SecLang_ell
    AddSize 1350
    !insertmacro Download_Lang_Data ell
  SectionEnd

  Section /o "English - Middle (1100-1500)" SecLang_enm
    AddSize 2960
    !insertmacro Download_Lang_Data enm
  SectionEnd

  Section /o "Esperanto" SecLang_epo
    AddSize 4510
    !insertmacro Download_Lang_Data epo
  SectionEnd

  Section /o "Estonian" SecLang_est
    AddSize 4250
    !insertmacro Download_Lang_Data est
  SectionEnd

  Section /o "Basque" SecLang_eus
    AddSize 4940
    !insertmacro Download_Lang_Data eus
  SectionEnd

  Section /o "Faroese" SecLang_fao
    AddSize 3280
    !insertmacro Download_Lang_Data fao
  SectionEnd

  Section /o "Persian" SecLang_fas
    AddSize 421
    !insertmacro Download_Lang_Data fas
  SectionEnd

  Section /o "Filipino" SecLang_fil
    AddSize 1760
    !insertmacro Download_Lang_Data fil
  SectionEnd

 Section /o "Finnish" SecLang_fin
    AddSize 7500
    !insertmacro Download_Lang_Data fin
  SectionEnd

  Section /o "French" SecLang_fra
    AddSize 1080
    !insertmacro Download_Lang_Data fra
  SectionEnd

 Section /o "French - Middle (ca. 1400-1600)" SecLang_frm
    AddSize 1930
    !insertmacro Download_Lang_Data frm
  SectionEnd

  Section /o "Frisian (Western)" SecLang_fry
    AddSize 1820
    !insertmacro Download_Lang_Data fry
  SectionEnd

  Section /o "Gaelic (Scots)" SecLang_gla
    AddSize 2930
    !insertmacro Download_Lang_Data gla
  SectionEnd

  Section /o "Irish" SecLang_gle
    AddSize 1130
    !insertmacro Download_Lang_Data gle
  SectionEnd

  Section /o "Galician" SecLang_glg
    AddSize 2440
    !insertmacro Download_Lang_Data glg
  SectionEnd

  Section /o "Greek, Ancient (-1453)" SecLang_grc
    AddSize 2140
    !insertmacro Download_Lang_Data grc
  SectionEnd

  Section /o "Gujarati" SecLang_guj
    AddSize 1350
    !insertmacro Download_Lang_Data guj
  SectionEnd

  Section /o "Haitian" SecLang_hat
    AddSize 1890
    !insertmacro Download_Lang_Data hat
  SectionEnd

  Section /o "Hebrew" SecLang_heb
    AddSize 939
    !insertmacro Download_Lang_Data heb
  SectionEnd

  Section /o "Hindi" SecLang_hin
    AddSize 1070
    !insertmacro Download_Lang_Data hin
  SectionEnd

  Section /o "Croatian" SecLang_hrv
    AddSize 3910
    !insertmacro Download_Lang_Data hrv
  SectionEnd

  Section /o "Hungarian" SecLang_hun
    AddSize 5050
    !insertmacro Download_Lang_Data hun
  SectionEnd

  Section /o "Armenian" SecLang_hye
    AddSize 3300
    !insertmacro Download_Lang_Data hye
  SectionEnd

  Section /o "Inuktitut" SecLang_iku
    AddSize 2670
    !insertmacro Download_Lang_Data iku
  SectionEnd

  Section /o "Indonesian" SecLang_ind
    AddSize 1070
    !insertmacro Download_Lang_Data ind
  SectionEnd

  Section /o "Icelandic" SecLang_isl
    AddSize 2170
    !insertmacro Download_Lang_Data isl
  SectionEnd

  Section /o "Italian" SecLang_ita
    AddSize 2580
    !insertmacro Download_Lang_Data ita
  SectionEnd

  Section /o "Italian (Old)" SecLang_ita_old
    AddSize 3130
    !insertmacro Download_Lang_Data ita_old
  SectionEnd

  Section /o "Javanese" SecLang_jav
    AddSize 2840
    !insertmacro Download_Lang_Data jav
  SectionEnd

  Section /o "Japanese" SecLang_jpn
    AddSize 2360
    !insertmacro Download_Lang_Data jpn
  SectionEnd

  Section /o "Japanese (vertical)" SecLang_jpn_vert
    AddSize 2900
    !insertmacro Download_Lang_Data jpn_vert
  SectionEnd

  Section /o "Kannada" SecLang_kan
    AddSize 3440
    !insertmacro Download_Lang_Data kan
  SectionEnd

  Section /o "Georgian" SecLang_kat
    AddSize 2410
    !insertmacro Download_Lang_Data kat
  SectionEnd

  Section /o "Georgian (Old)" SecLang_kat_old
    AddSize 413
    !insertmacro Download_Lang_Data kat_old
  SectionEnd

  Section /o "Kazakh" SecLang_kaz
    AddSize 4520
    !insertmacro Download_Lang_Data kaz
  SectionEnd

  Section /o "Central Khmer" SecLang_khm
    AddSize 1380
    !insertmacro Download_Lang_Data khm
  SectionEnd

  Section /o "Kirghiz" SecLang_kir
    AddSize 9470
    !insertmacro Download_Lang_Data kir
  SectionEnd

  Section /o "Korean" SecLang_kor
    AddSize 1600
    !insertmacro Download_Lang_Data kor
  SectionEnd

  Section /o "Kurdish (Kurmanji)" SecLang_kmr
    AddSize 3400
    !insertmacro Download_Lang_Data kmr
  SectionEnd

  Section /o "Lao" SecLang_lao
    AddSize 6090
    !insertmacro Download_Lang_Data lao
  SectionEnd

  Section /o "Latin" SecLang_lat
    AddSize 3040
    !insertmacro Download_Lang_Data lat
  SectionEnd

  Section /o "Latvian" SecLang_lav
    AddSize 2590
    !insertmacro Download_Lang_Data lav
  SectionEnd

  Section /o "Lithuanian" SecLang_lit
    AddSize 3010
    !insertmacro Download_Lang_Data lit
  SectionEnd

  Section /o "Luxembourgish" SecLang_ltz
    AddSize 2490
    !insertmacro Download_Lang_Data ltz
  SectionEnd

  Section /o "Malayalam" SecLang_mal
    AddSize 5030
    !insertmacro Download_Lang_Data mal
  SectionEnd

  Section /o "Marathi" SecLang_mar
    AddSize 2020
    !insertmacro Download_Lang_Data mar
  SectionEnd

  Section /o "Macedonian" SecLang_mkd
    AddSize 1530
    !insertmacro Download_Lang_Data mkd
  SectionEnd

  Section /o "Maltese" SecLang_mlt
    AddSize 2200
    !insertmacro Download_Lang_Data mlt
  SectionEnd

  Section /o "Mongolian" SecLang_mon
    AddSize 2040
    !insertmacro Download_Lang_Data mon
  SectionEnd

  Section /o "Maori" SecLang_mri
    AddSize 843
    !insertmacro Download_Lang_Data mri
  SectionEnd

  Section /o "Malay" SecLang_msa
    AddSize 1670
    !insertmacro Download_Lang_Data msa
  SectionEnd

  Section /o "Burmese" SecLang_mya
    AddSize 4430
    !insertmacro Download_Lang_Data mya
  SectionEnd

  Section /o "Nepali" SecLang_nep
    AddSize 979
    !insertmacro Download_Lang_Data nep
  SectionEnd

  Section /o "Dutch; Flemish" SecLang_nld
    AddSize 5770
    !insertmacro Download_Lang_Data nld
  SectionEnd

  Section /o "Norwegian" SecLang_nor
    AddSize 3440
    !insertmacro Download_Lang_Data nor
  SectionEnd

  Section /o "Occitan (post 1500)" SecLang_oci
    AddSize 6030
    !insertmacro Download_Lang_Data oci
  SectionEnd

  Section /o "Oriya" SecLang_ori
    AddSize 1410
    !insertmacro Download_Lang_Data ori
  SectionEnd

  Section /o "Panjabi / Punjabi" SecLang_pan
    AddSize 4860
    !insertmacro Download_Lang_Data pan
  SectionEnd

  Section /o "Polish" SecLang_pol
    AddSize 4540
    !insertmacro Download_Lang_Data pol
  SectionEnd

  Section /o "Portuguese" SecLang_por
    AddSize 1890
    !insertmacro Download_Lang_Data por
  SectionEnd

  Section /o "Pushto / Pashto" SecLang_pus
    AddSize 1690
    !insertmacro Download_Lang_Data pus
  SectionEnd

  Section /o "Quechua" SecLang_que
    AddSize 4790
    !insertmacro Download_Lang_Data que
  SectionEnd

  Section /o "Romanian" SecLang_ron
    AddSize 2270
    !insertmacro Download_Lang_Data ron
  SectionEnd

  Section /o "Russian" SecLang_rus
    AddSize 3680
    !insertmacro Download_Lang_Data rus
  SectionEnd

  Section /o "Sanskrit" SecLang_san
    AddSize 1180
    !insertmacro Download_Lang_Data san
  SectionEnd

  Section /o "Sinhala / Sinhalese" SecLang_sin
    AddSize 1650
    !insertmacro Download_Lang_Data sin
  SectionEnd

  Section /o "Slovak" SecLang_slk
    AddSize 4220
    !insertmacro Download_Lang_Data slk
  SectionEnd

  Section /o "Slovenian" SecLang_slv
    AddSize 2860
    !insertmacro Download_Lang_Data slv
  SectionEnd

  Section /o "Sindhi" SecLang_snd
    AddSize 1620
    !insertmacro Download_Lang_Data snd
  SectionEnd

  Section /o "Spanish" SecLang_spa
    AddSize 2190
    !insertmacro Download_Lang_Data spa
  SectionEnd

  Section /o "Spanish (Old)" SecLang_spa_old
    AddSize 2760
    !insertmacro Download_Lang_Data spa_old
  SectionEnd

  Section /o "Albanian" SecLang_sqi
    AddSize 1790
    !insertmacro Download_Lang_Data sqi
  SectionEnd

  Section /o "Serbian" SecLang_srp
    AddSize 2050
    !insertmacro Download_Lang_Data srp
  SectionEnd

  Section /o "Serbian (Latin)" SecLang_srp_latn
    AddSize 3130
    !insertmacro Download_Lang_Data srp_latn
  SectionEnd

  Section /o "Sundanese" SecLang_sun
    AddSize 1310
    !insertmacro Download_Lang_Data sun
  SectionEnd

  Section /o "Swahili" SecLang_swa
    AddSize 2070
    !insertmacro Download_Lang_Data swa
  SectionEnd

  Section /o "Swedish" SecLang_swe
    AddSize 3970
    !insertmacro Download_Lang_Data swe
  SectionEnd

  Section /o "Syriac" SecLang_syr
    AddSize 2100
    !insertmacro Download_Lang_Data syr
  SectionEnd

 Section /o "Tamil" SecLang_tam
    AddSize 3090
    !insertmacro Download_Lang_Data tam
  SectionEnd

  Section /o "Tatar" SecLang_tat
    AddSize 1020
    !insertmacro Download_Lang_Data tat
  SectionEnd

  Section /o "Telugu" SecLang_tel
    AddSize 2640
    !insertmacro Download_Lang_Data tel
  SectionEnd

  Section /o "Tajik" SecLang_tgk
    AddSize 2480
    !insertmacro Download_Lang_Data tgk
  SectionEnd

  Section /o "Thai" SecLang_tha
    AddSize 1020
    !insertmacro Download_Lang_Data tha
  SectionEnd

  Section /o "Tigrinya" SecLang_tir
    AddSize 370
    !insertmacro Download_Lang_Data tir
  SectionEnd

 Section /o "Tonga" SecLang_ton
    AddSize 925
    !insertmacro Download_Lang_Data ton
  SectionEnd

  Section /o "Turkish" SecLang_tur
    AddSize 4240
    !insertmacro Download_Lang_Data tur
  SectionEnd

  Section /o "Uighur" SecLang_uig
    AddSize 2660
    !insertmacro Download_Lang_Data uig
  SectionEnd

  Section /o "Ukrainian" SecLang_ukr
    AddSize 3650
    !insertmacro Download_Lang_Data ukr
  SectionEnd

  Section /o "Urdu" SecLang_urd
    AddSize 1330
    !insertmacro Download_Lang_Data urd
  SectionEnd

  Section /o "Uzbek" SecLang_uzb
    AddSize 6170
    !insertmacro Download_Lang_Data uzb
  SectionEnd

  Section /o "Uzbek (Cyrillic)" SecLang_uzb_cyrl
    AddSize 1490
    !insertmacro Download_Lang_Data uzb_cyrl
  SectionEnd

  Section /o "Vietnamese" SecLang_vie
    AddSize 519
    !insertmacro Download_Lang_Data vie
  SectionEnd

  Section /o "Yiddish" SecLang_yid
    AddSize 533
    !insertmacro Download_Lang_Data yid
  SectionEnd

  Section /o "Yoruba" SecLang_yor
    AddSize 941
    !insertmacro Download_Lang_Data yor
  SectionEnd

SectionGroupEnd

;--------------------------------
;Descriptions
  ; At first we need to localize installer for languages which supports well in tesseract: Eng, Spa, Ger, Ita, Dutch + Russian (it is authors native language)
  ;Language strings
  LangString DESC_SEC0001 ${LANG_RUSSIAN} "Установочные файлы."
  ;LangString DESC_SecHelp ${LANG_RUSSIAN} "Справочная информация."
  LangString DESC_SecCS    ${LANG_RUSSIAN} "Добавить ярлыки в меню Пуск"

  LangString DESC_SEC0001 ${LANG_ENGLISH} "Installation files."
  ;LangString DESC_SecHelp ${LANG_ENGLISH} "Help information."
  LangString DESC_SecCS    ${LANG_ENGLISH} "Add shortcuts to Start menu."

  LangString DESC_SEC0001 ${LANG_FRENCH} "Fichier d'installation."
  ;LangString DESC_SecHelp ${LANG_FRENCH} "Aide."
  LangString DESC_SecCS   ${LANG_FRENCH} "Ajouter des raccourcis vers le menu démarrer."

  LangString DESC_SEC0001 ${LANG_GERMAN} "Dateien für die Installation."
 ;LangString DESC_SecHelp ${LANG_GERMAN} "Hilfe."
  LangString DESC_SecCS   ${LANG_GERMAN} "Einträge im Startmenü hinzufügen."

  LangString DESC_SEC0001 ${LANG_ITALIAN} "File di installazione."
  ;LangString DESC_SecHelp ${LANG_ITALIAN} "Guida di informazioni."
  LangString DESC_SecCS    ${LANG_ITALIAN} "Aggiungere collegamenti al menu Start."

  LangString DESC_SEC0001 ${LANG_SLOVAK} "Súbory inštalácie."
  ;LangString DESC_SecHelp ${LANG_ENGLISH} "Pomocné informácie."
  LangString DESC_SecCS    ${LANG_SLOVAK} "Pridať odkaz do Start menu."

  LangString DESC_SEC0001 ${LANG_SPANISH} "Los archivos de instalación."
  ;LangString DESC_SecHelp ${LANG_SPANISH} "Información de ayuda."
  LangString DESC_SecCS    ${LANG_SPANISH} "Ańadir accesos directos al menú Inicio."

  LangString DESC_SEC0001 ${LANG_SPANISHINTERNATIONAL} "Los archivos de instalación."
  ;LangString DESC_SecHelp ${LANG_SPANISHINTERNATIONAL} "Información de ayuda."
  LangString DESC_SecCS    ${LANG_SPANISHINTERNATIONAL} "Ańadir accesos directos al menú Inicio."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SEC0001} $(DESC_SEC0001)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCS} $(DESC_SecCS)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

;Section /o -un.Main UNSEC0000
Section -un.Main UNSEC0000
!ifdef W64
  SetRegView 64
!endif
  DetailPrint "Removing everything"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\*.*"
  RMDir  "$SMPROGRAMS\${PRODUCT_NAME}"
  DetailPrint "Removing registry info"
  DeleteRegKey HKLM "Software\Tesseract-OCR"
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=1000

  # remove the Add/Remove information
  DeleteRegKey HKLM "${UNINST_KEY}"
  Delete "${UNINST_EXE}"
  DeleteRegValue HKLM "${REGKEY}" Path
  DeleteRegKey /IfEmpty HKLM "${REGKEY}\Components"
  DeleteRegKey /IfEmpty HKLM "${REGKEY}"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\*.html"
  Delete "$INSTDIR\doc\AUTHORS"
  Delete "$INSTDIR\doc\LICENSE"
  Delete "$INSTDIR\doc\README.md"
  RMDir "$INSTDIR\doc"
  RMDir /r "$INSTDIR\tessdata"
  RMDir "$INSTDIR"
SectionEnd

Function PageReinstall

FunctionEnd

Function PageLeaveReinstall

FunctionEnd

!macro REMOVE_REGKEY OLD_KEY
  StrCmp ${OLD_KEY} HKLM 0 +3
    DeleteRegKey HKLM "${REGKEY}"
    Goto End
  DeleteRegKey HKCU "${REGKEY}"
  End:
!macroend

Function .onInit
!ifdef W64
  SetRegView 64
!endif
  Call PreventMultipleInstances
  !insertmacro MUI_LANGDLL_DISPLAY
  ;RequestExecutionLevel admin
  !insertmacro MULTIUSER_INIT

  ; is tesseract already installed?
  ReadRegStr $R0 HKCU "${REGKEY}" "CurrentVersion"
  StrCpy $OLD_KEY HKCU
  StrCmp $R0 "" TestHKLM AskUninstall
  TestHKLM:
    ReadRegStr $R0 HKLM "${REGKEY}" "CurrentVersion"
    StrCpy $OLD_KEY HKLM
    StrCmp $R0 "" SkipUnInstall
  AskUninstall:
    MessageBox MB_YESNO|MB_ICONEXCLAMATION \
      "Tesseract-ocr version $R0 is installed (in $OLD_KEY)! Do you want to uninstall it first?$\nUninstall will delete all files in '$INSTDIR'!" \
       /SD IDYES IDNO SkipUnInstall IDYES UnInstall
  UnInstall:
    StrCmp $OLD_KEY "HKLM" UnInst_hklm
       DetailPrint "Uninstall: current user"
       readRegStr $R1 HKCU "${UNINST_KEY}" "UninstallString"
       Goto try_uninstall
    UnInst_hklm:
       DetailPrint "UnInstall: all users"
       readRegStr $R1 HKLM "${UNINST_KEY}" "UninstallString"
    try_uninstall:
      ClearErrors
      ExecWait '$R1 _?=$INSTDIR'$0
      ; Check if unstaller finished ok. If yes, then try to remove it from installer.
      StrCmp $0 0 0 +3
        !insertmacro REMOVE_REGKEY ${OLD_KEY}
        Goto SkipUnInstall
      messagebox mb_ok "Uninstaller failed:\n$0\n\nYou need to remove program manually."
  SkipUnInstall:
    ;InitPluginsDir
    ;File /oname=$PLUGINSDIR\splash.bmp "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
    ;File /oname=$PLUGINSDIR\splash.bmp "new.bmp"
    ;advsplash::show 1000 600 400 -1 $PLUGINSDIR\splash
    ;Pop $0          ; $0 has '1' if the user closed the splash screen early,
                    ; '0' if everything closed normal, and '-1' if some error occurred.
    ;IfFileExists $INSTDIR\loadmain.exe PathGood
  ;done:
    ; Make selection based on System language ID
    System::Call 'kernel32::GetSystemDefaultLangID() i .r0'
    ;http://msdn.microsoft.com/en-us/library/dd318693%28v=VS.85%29.aspx
    StrCmp $0 "1078" Afrikaans
    StrCmp $0 "1052" Albanian
    StrCmp $0 "5121" Arabic
    StrCmp $0 "1068" Azerbaijani
    StrCmp $0 "1069" Basque
    StrCmp $0 "1059" Belarusian
    StrCmp $0 "1093" Bengali
    StrCmp $0 "1026" Bulgarian
    StrCmp $0 "1027" Catalan
    StrCmp $0 "1116" Cherokee
    StrCmp $0 "31748" Chinese_tra
    StrCmp $0 "4" Chinese_sim
    StrCmp $0 "26" Croatian
    StrCmp $0 "1029" Czech
    StrCmp $0 "1030" Danish
    StrCmp $0 "2067" Dutch
    StrCmp $0 "1061" Estonian
    StrCmp $0 "3079" German
    StrCmp $0 "1032" Greek
    StrCmp $0 "1035" Finnish
    StrCmp $0 "2060" French
    StrCmp $0 "1037" Hebrew
    StrCmp $0 "1081" Hindi
    StrCmp $0 "1038" Hungarian
    StrCmp $0 "1039" Icelandic
    StrCmp $0 "1057" Indonesian
    StrCmp $0 "1040" Italian
    StrCmp $0 "1041" Japanese
    StrCmp $0 "1099" Kannada
    StrCmp $0 "1042" Korean
    StrCmp $0 "1062" Latvian
    StrCmp $0 "1063" Lithuanian
    StrCmp $0 "1071" Macedonian
    StrCmp $0 "1100" Malayalam
    StrCmp $0 "2110" Malay
    StrCmp $0 "1082" Maltese
    StrCmp $0 "1044" Norwegian
    StrCmp $0 "1045" Polish
    StrCmp $0 "1046" Portuguese
    StrCmp $0 "1048" Romanian
    StrCmp $0 "1049" Russian
    StrCmp $0 "1051" Slovak
    StrCmp $0 "1060" Slovenian
    StrCmp $0 "11274" Spanish
    StrCmp $0 "2074" Serbian
    StrCmp $0 "1089" Swahili
    StrCmp $0 "2077" Swedish
    StrCmp $0 "1097" Tamil
    StrCmp $0 "1098" Telugu
    StrCmp $0 "1054" Thai
    StrCmp $0 "1055" Turkish
    StrCmp $0 "1058" Ukrainian
    StrCmp $0 "1066" Vietnamese

    Goto lang_end

    Afrikaans: !insertmacro SelectSection ${SecLang_afr}
            Goto lang_end
    Albanian: !insertmacro SelectSection ${SecLang_sqi}
            Goto lang_end
    Arabic: !insertmacro SelectSection ${SecLang_ara}
            Goto lang_end
    ;Assamese: !insertmacro SelectSection ${SecLang_asm}
    ;        Goto lang_end
    Azerbaijani: !insertmacro SelectSection ${SecLang_aze}
            Goto lang_end
    Basque: !insertmacro SelectSection ${SecLang_eus}
            Goto lang_end
    Belarusian: !insertmacro SelectSection ${SecLang_bel}
            Goto lang_end
    Bengali: !insertmacro SelectSection ${SecLang_ben}
            Goto lang_end
    Bulgarian: !insertmacro SelectSection ${SecLang_bul}
            Goto lang_end
    Catalan: !insertmacro SelectSection ${SecLang_cat}
            Goto lang_end
    Cherokee: !insertmacro SelectSection ${SecLang_chr}
            Goto lang_end
    Chinese_tra: !insertmacro SelectSection ${SecLang_chi_tra}
            Goto lang_end
    Chinese_sim: !insertmacro SelectSection ${SecLang_chi_sim}
            Goto lang_end
    Croatian: !insertmacro SelectSection ${SecLang_hrv}
            Goto lang_end
    Czech: !insertmacro SelectSection ${SecLang_ces}
            Goto lang_end
    Danish: !insertmacro SelectSection ${SecLang_dan}
            Goto lang_end
    Dutch: !insertmacro SelectSection ${SecLang_nld}
            Goto lang_end
    Estonian: !insertmacro SelectSection ${SecLang_hrv}
            Goto lang_end
    German: !insertmacro SelectSection ${SecLang_deu}
            Goto lang_end
    Greek: !insertmacro SelectSection ${SecLang_ell}
            !insertmacro SelectSection ${SecLang_grc}
            Goto lang_end
    Finnish: !insertmacro SelectSection ${SecLang_fin}
            !insertmacro SelectSection ${SecLang_frm}
            Goto lang_end
    French: !insertmacro SelectSection ${SecLang_fra}
            Goto lang_end
    Hebrew: !insertmacro SelectSection ${SecLang_heb}
            ;!insertmacro SelectSection ${SecLang_heb_com}
            Goto lang_end
    Hungarian: !insertmacro SelectSection ${SecLang_hin}
            Goto lang_end
    Hindi: !insertmacro SelectSection ${SecLang_hun}
            Goto lang_end
    Icelandic: !insertmacro SelectSection ${SecLang_isl}
            Goto lang_end
    Indonesian: !insertmacro SelectSection ${SecLang_ind}
            Goto lang_end
    Italian: !insertmacro SelectSection ${SecLang_ita}
            !insertmacro SelectSection ${SecLang_ita_old}
            Goto lang_end
    Japanese: !insertmacro SelectSection ${SecLang_jpn}
            Goto lang_end
    Kannada: !insertmacro SelectSection ${SecLang_kan}
            Goto lang_end
    Korean: !insertmacro SelectSection ${SecLang_kor}
            Goto lang_end
    Latvian: !insertmacro SelectSection ${SecLang_lav}
            Goto lang_end
    Lithuanian: !insertmacro SelectSection ${SecLang_lit}
            Goto lang_end
    Macedonian: !insertmacro SelectSection ${SecLang_mkd}
            Goto lang_end
    Malayalam: !insertmacro SelectSection ${SecLang_msa}
            Goto lang_end
    Malay: !insertmacro SelectSection ${SecLang_mal}
            Goto lang_end
    Maltese: !insertmacro SelectSection ${SecLang_mlt}
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
            !insertmacro SelectSection ${SecLang_spa_old}
            Goto lang_end
    Serbian: !insertmacro SelectSection ${SecLang_srp}
            Goto lang_end
    Swahili: !insertmacro SelectSection ${SecLang_swa}
            Goto lang_end
    Swedish: !insertmacro SelectSection ${SecLang_swe}
            Goto lang_end
    Tamil: !insertmacro SelectSection ${SecLang_tam}
            Goto lang_end
    Telugu: !insertmacro SelectSection ${SecLang_tel}
            Goto lang_end
    Thai: !insertmacro SelectSection ${SecLang_tha}
            Goto lang_end
    Turkish: !insertmacro SelectSection ${SecLang_tur}
            Goto lang_end
    Ukrainian: !insertmacro SelectSection ${SecLang_ukr}
            Goto lang_end
    Vietnamese: !insertmacro SelectSection ${SecLang_vie}

    lang_end:
FunctionEnd

Function un.onInit
  !insertmacro MUI_LANGDLL_DISPLAY
  !insertmacro MULTIUSER_UNINIT
  ;!insertmacro SELECT_UNSECTION Main ${UNSEC0000}
  ;!insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Function .onInstFailed
  MessageBox MB_OK "Installation failed."
FunctionEnd

!ifdef SHOW_README
Function ShowReadme
  Exec '"wordpad" "doc\README.md"'
  ;BringToFront
FunctionEnd
!endif

; Prevent running multiple instances of the installer
Function PreventMultipleInstances
  ; TODO: Does not work.
  Push $R0
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t ${PRODUCT_NAME}) ?e'
  Pop $R0
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running." /SD IDOK
    Abort
  Pop $R0
FunctionEnd
