; (C) Copyright 2010, Sergey Bronnikov
; (C) Copyright 2010-2012, Zdenko Podobný
; (C) Copyright 2015 Stefan Weil
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
; * Fix PreventMultipleInstances.
; * Add Tesseract icon and images for installer.
; * Add support for 64 bit Tesseract.

SetCompressor /FINAL /SOLID lzma
SetCompressorDictSize 32

; Settings which normally should be passed as command line arguments.
;define CROSSBUILD
;define SHARED
;define W64
!ifndef SRCDIR
!define SRCDIR .
!endif
!ifndef VERSION
!define VERSION 4.00-dev
!endif

!define PRODUCT_NAME "Tesseract-OCR"
!define PRODUCT_VERSION "${VERSION}"
!define PRODUCT_PUBLISHER "Tesseract-OCR community"
!ifndef PRODUCT_WEB_SITE
!define PRODUCT_WEB_SITE "https://github.com/tesseract-ocr/tesseract"
!endif
!define GITHUB_RAW_FILE_URL \
  "https://raw.githubusercontent.com/tesseract-ocr/tessdata/master"

!ifdef CROSSBUILD
!addincludedir ${SRCDIR}\nsis\include
!addplugindir ${SRCDIR}\nsis\plugins
!endif

!define PREFIX "../usr/i686-w64-mingw32"
!define TRAININGDIR "${PREFIX}/bin"

# General Definitions
Name "${PRODUCT_NAME}"
Caption "${PRODUCT_NAME} ${VERSION}"
!ifndef CROSSBUILD
BrandingText /TRIMCENTER "(c) 2010-2015 ${PRODUCT_NAME}"
!endif

!define REGKEY "SOFTWARE\${PRODUCT_NAME}"
; HKLM (all users) vs HKCU (current user) defines
!define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
!define env_hkcu 'HKCU "Environment"'

# MultiUser Symbol Definitions
!define MULTIUSER_EXECUTIONLEVEL Admin
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${REGKEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME MultiUserInstallMode
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_INSTDIR ${PRODUCT_NAME}
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${REGKEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUE "Path"

# MUI Symbol Definitions
!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue-full.ico"
!define MUI_FINISHPAGE_LINK "View Tesseract on GitHub"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/tesseract-ocr/tesseract"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_SHOWREADME "iexplore $INSTDIR\doc\README"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReadme
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show README"
!define MUI_LICENSEPAGE_CHECKBOX
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
!ifdef REGISTRY_SETTINGS
!include EnvVarUpdate.nsh
!endif ; REGISTRY_SETTINGS
!include LogicLib.nsh
!include winmessages.nsh # include for some of the windows messages defines

# Variables
Var StartMenuGroup
!ifdef REGISTRY_SETTINGS
Var PathKey
!endif ; REGISTRY_SETTINGS
; Define user variables
Var OLD_KEY

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${SRCDIR}\COPYING"
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!ifdef VERSION
  Page custom PageReinstall PageLeaveReinstall
!endif
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
!ifdef W64
InstallDir "$PROGRAMFILES64\Tesseract-OCR"
!else
InstallDir "$PROGRAMFILES\Tesseract-OCR"
!endif
# Name of program and file
!ifdef VERSION
OutFile tesseract-ocr-setup-${VERSION}.exe
!else
OutFile tesseract-ocr-setup.exe
!endif

!ifdef REGISTRY_SETTINGS
!macro AddToPath
  # TODO(zdenop): Check if $INSTDIR is in path. If yes, do not append it.
  # append bin path to user PATH environment variable
  StrCpy $PathKey "HKLM"
  StrCmp $MultiUser.InstallMode "AllUsers" +2
    StrCpy $PathKey "HKCU"
  DetailPrint "Setting PATH to $INSTDIR at $PathKey"
  ${EnvVarUpdate} $0 "PATH" "A" "$PathKey" "$INSTDIR"
  ; make sure windows knows about the change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
!macroend

!macro RemoveTessdataPrefix
  ReadRegStr $R2 ${env_hklm} 'TESSDATA_PREFIX'
  StrCmp $R2 "" Next1 0
    DetailPrint "Removing $R2 from HKLM Environment..."
    DeleteRegValue ${env_hklm} "TESSDATA_PREFIX"
  Next1:
  ReadRegStr $R2 ${env_hkcu} 'TESSDATA_PREFIX'
  StrCmp $R2 "" Next2 0
    DetailPrint "Removing $R2 from HKCU Environment..."
    DeleteRegValue ${env_hkcu} "TESSDATA_PREFIX"
  Next2:
  # make sure windows knows about the change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
!macroend

!macro SetTESSDATA
  !insertmacro RemoveTessdataPrefix
  StrCpy $PathKey "HKLM"
  StrCmp $MultiUser.InstallMode "AllUsers" +2
    StrCpy $PathKey "HKCU"
  DetailPrint "Setting TESSDATA_PREFIX at $PathKey"
  ${EnvVarUpdate} $0 "TESSDATA_PREFIX" "A" "$PathKey" "$INSTDIR\"
  # make sure windows knows about the change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
!macroend
!endif ; REGISTRY_SETTINGS

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
  File ${PREFIX}/bin/tesseract.exe
  File ${PREFIX}/bin/libtesseract-*.dll
!ifdef CROSSBUILD
  File ${SRCDIR}\dll\i686-w64-mingw32\*.dll
!endif
  CreateDirectory "$INSTDIR\tessdata"
  SetOutPath "$INSTDIR\tessdata"
  File ${PREFIX}/share/tessdata/pdf.ttf
  CreateDirectory "$INSTDIR\tessdata\configs"
  SetOutPath "$INSTDIR\tessdata\configs"
  File ${PREFIX}/share/tessdata/configs/*
  CreateDirectory "$INSTDIR\tessdata\tessconfigs"
  SetOutPath "$INSTDIR\tessdata\tessconfigs"
  File ${PREFIX}/share/tessdata/tessconfigs/*
  CreateDirectory "$INSTDIR\doc"
  SetOutPath "$INSTDIR\doc"
  File ${SRCDIR}\AUTHORS
  File ${SRCDIR}\COPYING
  File ${SRCDIR}\testing\eurotext.tif
  File ${SRCDIR}\testing\phototest.tif
  File ${SRCDIR}\testing\README
##  File ${SRCDIR}\ReleaseNotes
SectionEnd

Section "ScrollView" SecScrollView
  SectionIn 1
  CreateDirectory "$INSTDIR\java"
  SetOutPath "$INSTDIR\java"
  File ..\java\ScrollView.jar
  File ..\java\piccolo2d-core-3.0.jar
  File ..\java\piccolo2d-extras-3.0.jar
SectionEnd

Section "Training Tools" SecTr
  SectionIn 1
  SetOutPath "$INSTDIR"
  File ${TRAININGDIR}\*.exe
SectionEnd

!define UNINST_EXE "$INSTDIR\tesseract-uninstall.exe"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

Section -post SEC0001
  ;Store installation folder - we always use HKLM!
  WriteRegStr HKLM "${REGKEY}" "Path" "$INSTDIR"
  WriteRegStr HKLM "${REGKEY}" "Mode" $MultiUser.InstallMode
  WriteRegStr HKLM "${REGKEY}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "${REGKEY}" "CurrentVersion" "${VERSION}"
  WriteRegStr HKLM "${REGKEY}" "Uninstaller" "${UNINST_EXE}"
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
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Console.lnk" $WINDIR\system32\CMD.EXE
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Homepage.lnk" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\ReadMe.lnk" "${PRODUCT_WEB_SITE}/wiki/ReadMe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\FAQ.lnk" "${PRODUCT_WEB_SITE}/wiki/FAQ"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "${UNINST_EXE}" "" "${UNINST_EXE}" 0
  ;CreateShortCut "$DESKTOP\Tesseract-OCR.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
  ;CreateShortCut "$QUICKLAUNCH\.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
SectionEnd

!ifdef REGISTRY_SETTINGS ; disabled because of bad behaviour with long PATH
SectionGroup "Registry settings" SecRS
    Section /o "Add to Path" SecRS_path
        !insertmacro AddToPath
    SectionEnd
    Section /o "Set TESSDATA_PREFIX variable" SecRS_tessdata
        !insertmacro SetTESSDATA
    SectionEnd
SectionGroupEnd
!endif ; REGISTRY_SETTINGS

; Language files
SectionGroup "Language data" SecGrp_LD
    Section "English" SecLang_eng
    SectionIn RO
      SetOutPath "$INSTDIR\tessdata"
      File ${SRCDIR}\tessdata\eng.*
    SectionEnd

    Section "Orientation and script detection" SecLang_osd
    SectionIn 1
      SetOutPath "$INSTDIR\tessdata"
      File ${SRCDIR}\tessdata\osd.*
    SectionEnd
SectionGroupEnd

; Download language files
SectionGroup "Additional language data (download)" SecGrp_ALD
  Section /o "Math / equation detection module" SecLang_equ
    AddSize 2200
    !insertmacro Download_Lang_Data equ
  SectionEnd

  ; The language names are documented here:
  ; https://github.com/tesseract-ocr/tesseract/blob/master/doc/tesseract.1.asc#languages

  Section /o "Afrikaans" SecLang_afr
    AddSize 5080
    !insertmacro Download_Lang_Data afr
  SectionEnd

  Section /o "Albanian" SecLang_sqi
    AddSize 6436
    !insertmacro Download_Lang_Data sqi
  SectionEnd

  Section /o "Amharic" SecLang_amh
    AddSize 2888
    !insertmacro Download_Lang_Data amh
  SectionEnd

  Section /o "Arabic" SecLang_ara
    AddSize 27888
    !insertmacro Download_Lang_Data ara
  SectionEnd

  Section /o "Assamese" SecLang_asm
    AddSize 15460
    !insertmacro Download_Lang_Data asm
  SectionEnd

  Section /o "Azerbaijani" SecLang_aze
    AddSize 6464
    !insertmacro Download_Lang_Data aze
  SectionEnd

  Section /o "Azerbaijani (Cyrilic)" SecLang_aze_cyrl
    AddSize 2720
    !insertmacro Download_Lang_Data aze_cyrl
  SectionEnd

  Section /o "Basque" SecLang_eus
    AddSize 4856
    !insertmacro Download_Lang_Data eus
  SectionEnd

  Section /o "Belarusian" SecLang_bel
    AddSize 6664
    !insertmacro Download_Lang_Data bel
  SectionEnd

  Section /o "Bengali" SecLang_ben
    AddSize 15192
    !insertmacro Download_Lang_Data ben
  SectionEnd

  Section /o "Tibetan" SecLang_bod
    AddSize 24648
    !insertmacro Download_Lang_Data bod
  SectionEnd

  Section /o "Bosnian" SecLang_bos
    AddSize 5308
    !insertmacro Download_Lang_Data bos
  SectionEnd

  Section /o "Bulgarian" SecLang_bul
    AddSize 5888
    !insertmacro Download_Lang_Data bul
  SectionEnd

  Section /o "Catalan" SecLang_cat
    AddSize 5232
    !insertmacro Download_Lang_Data cat
  SectionEnd

  Section /o "Cebuano" SecLang_ceb
    AddSize 1648
    !insertmacro Download_Lang_Data ceb
  SectionEnd

  Section /o "Cherokee" SecLang_chr
    AddSize 1060
    !insertmacro Download_Lang_Data chr
  SectionEnd

  Section /o "Chinese (Traditional)" SecLang_chi_tra
    AddSize 55368
    !insertmacro Download_Lang_Data chi_tra
  SectionEnd

  Section /o "Chinese (Simplified)" SecLang_chi_sim
    AddSize 41108
    !insertmacro Download_Lang_Data chi_sim
  SectionEnd

  Section /o "Croatian" SecLang_hrv
    AddSize 8924
    !insertmacro Download_Lang_Data hrv
  SectionEnd

  Section /o "Czech" SecLang_ces
    AddSize 11620
    !insertmacro Download_Lang_Data ces
  SectionEnd

  Section /o "Welsh" SecLang_cym
    AddSize 3704
    !insertmacro Download_Lang_Data cym
  SectionEnd

  Section /o "Danish" SecLang_dan
    AddSize 7172
    !insertmacro Download_Lang_Data dan
  SectionEnd

  Section /o "Danish (Fraktur)" SecLang_dan_frak
    AddSize 1588
    !insertmacro Download_Lang_Data dan_frak
  SectionEnd

  Section /o "Dutch" SecLang_nld
    AddSize 16704
    !insertmacro Download_Lang_Data nld
  SectionEnd

  Section /o "English - Middle (1100-1500)" SecLang_enm
    AddSize 2060
    !insertmacro Download_Lang_Data enm
  SectionEnd

  Section /o "Esperanto" SecLang_epo
    AddSize 6448
    !insertmacro Download_Lang_Data epo
  SectionEnd

  Section /o "Estonian" SecLang_est
    AddSize 9424
    !insertmacro Download_Lang_Data est
  SectionEnd

  Section /o "German" SecLang_deu
    AddSize 13060
    !insertmacro Download_Lang_Data deu
  SectionEnd

  Section /o "German (Fraktur)" SecLang_deu_frak
    AddSize 1936
    !insertmacro Download_Lang_Data deu_frak
  SectionEnd

  Section /o "Dzongkha" SecLang_dzo
    AddSize 3236
    !insertmacro Download_Lang_Data dzo
  SectionEnd

  Section /o "Greek" SecLang_ell
    AddSize 5296
    !insertmacro Download_Lang_Data ell
  SectionEnd

  Section /o "Greek - Ancient" SecLang_grc
    AddSize 5064
    !insertmacro Download_Lang_Data grc
  SectionEnd

  Section /o "Persian" SecLang_fas
    AddSize 4692
    !insertmacro Download_Lang_Data fas
  SectionEnd

  Section /o "Finnish" SecLang_fin
    AddSize 12964
    !insertmacro Download_Lang_Data fin
  SectionEnd

  Section /o "Frankish" SecLang_frk
    AddSize 16072
    !insertmacro Download_Lang_Data frk
  SectionEnd

  Section /o "French" SecLang_fra
    AddSize 36504
    !insertmacro Download_Lang_Data fra
  SectionEnd

  Section /o "French - Middle (ca. 1400-1600)" SecLang_frm
    AddSize 15468
    !insertmacro Download_Lang_Data frm
  SectionEnd

  Section /o "Irish" SecLang_gle
    AddSize 3404
    !insertmacro Download_Lang_Data gle
  SectionEnd

  Section /o "Galician" SecLang_glg
    AddSize 5392
    !insertmacro Download_Lang_Data glg
  SectionEnd

  Section /o "Gujarati" SecLang_guj
    AddSize 10380
    !insertmacro Download_Lang_Data guj
  SectionEnd

  Section /o "Haitian" SecLang_hat
    AddSize 1320
    !insertmacro Download_Lang_Data hat
  SectionEnd

  Section /o "Hebrew" SecLang_heb
    AddSize 4240
    !insertmacro Download_Lang_Data heb
  SectionEnd

  Section /o "Hindi" SecLang_hin
    AddSize 22212
    !insertmacro Download_Lang_Data hin
  SectionEnd

  Section /o "Hungarian" SecLang_hun
    AddSize 11932
    !insertmacro Download_Lang_Data hun
  SectionEnd

  Section /o "Inuktitut" SecLang_iku
    AddSize 972
    !insertmacro Download_Lang_Data iku
  SectionEnd

  Section /o "Icelandic" SecLang_isl
    AddSize 5956
    !insertmacro Download_Lang_Data isl
  SectionEnd

  Section /o "Indonesian" SecLang_ind
    AddSize 6352
    !insertmacro Download_Lang_Data ind
  SectionEnd

  Section /o "Italian" SecLang_ita
    AddSize 31980
    !insertmacro Download_Lang_Data ita
  SectionEnd

  Section /o "Italian (Old)" SecLang_ita_old
    AddSize 13732
    !insertmacro Download_Lang_Data ita_old
  SectionEnd

  Section /o "Javanese" SecLang_jav
    AddSize 4304
    !insertmacro Download_Lang_Data jav
  SectionEnd

  Section /o "Japanese" SecLang_jpn
    AddSize 32304
    !insertmacro Download_Lang_Data jpn
  SectionEnd

  Section /o "Kannada" SecLang_kan
    AddSize 34828
    !insertmacro Download_Lang_Data kan
  SectionEnd

  Section /o "Georgian" SecLang_kat
    AddSize 6076
    !insertmacro Download_Lang_Data kat
  SectionEnd

  Section /o "Georgian (Old)" SecLang_kat_old
    AddSize 644
    !insertmacro Download_Lang_Data kat_old
  SectionEnd

  Section /o "Kazakh" SecLang_kaz
    AddSize 4424
    !insertmacro Download_Lang_Data kaz
  SectionEnd

  Section /o "Central Khmer" SecLang_khm
    AddSize 47712
    !insertmacro Download_Lang_Data khm
  SectionEnd

  Section /o "Kirghiz" SecLang_kir
    AddSize 5376
    !insertmacro Download_Lang_Data kir
  SectionEnd

  Section /o "Korean" SecLang_kor
    AddSize 13004
    !insertmacro Download_Lang_Data kor
  SectionEnd

  Section /o "Kurdish" SecLang_kur
    AddSize 1976
    !insertmacro Download_Lang_Data kur
  SectionEnd

  Section /o "Lao" SecLang_lao
    AddSize 20628
    !insertmacro Download_Lang_Data lao
  SectionEnd

  Section /o "Latin" SecLang_lat
    AddSize 5888
    !insertmacro Download_Lang_Data lat
  SectionEnd

  Section /o "Latvian" SecLang_lav
    AddSize 7620
    !insertmacro Download_Lang_Data lav
  SectionEnd

  Section /o "Lithuanian" SecLang_lit
    AddSize 8708
    !insertmacro Download_Lang_Data lit
  SectionEnd

  Section /o "Macedonian" SecLang_mkd
    AddSize 3748
    !insertmacro Download_Lang_Data mkd
  SectionEnd

  Section /o "Malay" SecLang_msa
    AddSize 6344
    !insertmacro Download_Lang_Data msa
  SectionEnd

  Section /o "Malayalam" SecLang_mal
    AddSize 8584
    !insertmacro Download_Lang_Data mal
  SectionEnd

  Section /o "Maltese" SecLang_mlt
    AddSize 5000
    !insertmacro Download_Lang_Data mlt
  SectionEnd

  Section /o "Marathi" SecLang_mar
    AddSize 13908
    !insertmacro Download_Lang_Data mar
  SectionEnd

  Section /o "Burmese" SecLang_mya
    AddSize 68140
    !insertmacro Download_Lang_Data mya
  SectionEnd

  Section /o "Nepali" SecLang_nep
    AddSize 15496
    !insertmacro Download_Lang_Data nep
  SectionEnd

  Section /o "Norwegian" SecLang_nor
    AddSize 8072
    !insertmacro Download_Lang_Data nor
  SectionEnd

  Section /o "Oriya" SecLang_ori
    AddSize 7716
    !insertmacro Download_Lang_Data ori
  SectionEnd

  Section /o "Panjabi / Punjabi" SecLang_pan
    AddSize 9976
    !insertmacro Download_Lang_Data pan
  SectionEnd

  Section /o "Polish" SecLang_pol
    AddSize 13592
    !insertmacro Download_Lang_Data pol
  SectionEnd

  Section /o "Portuguese" SecLang_por
    AddSize 12612
    !insertmacro Download_Lang_Data por
  SectionEnd

  Section /o "Pushto / Pashto" SecLang_pus
    AddSize 2436
    !insertmacro Download_Lang_Data pus
  SectionEnd

  Section /o "Romanian" SecLang_ron
    AddSize 7772
    !insertmacro Download_Lang_Data ron
  SectionEnd

  Section /o "Russian" SecLang_rus
    AddSize 38472
    !insertmacro Download_Lang_Data rus
  SectionEnd

  Section /o "Sanskrit" SecLang_san
    AddSize 22220
    !insertmacro Download_Lang_Data san
  SectionEnd

  Section /o "Sinhala / Sinhalese" SecLang_sin
    AddSize 6636
    !insertmacro Download_Lang_Data sin
  SectionEnd

  Section /o "Slovak" SecLang_slk
    AddSize 8916
    !insertmacro Download_Lang_Data slk
  SectionEnd

  Section /o "Slovak (Fraktur)" SecLang_slk_frak
    AddSize 828
    !insertmacro Download_Lang_Data slk_frak
  SectionEnd

  Section /o "Slovenian" SecLang_slv
    AddSize 6668
    !insertmacro Download_Lang_Data slv
  SectionEnd

  Section /o "Spanish" SecLang_spa
    AddSize 38276
    !insertmacro Download_Lang_Data spa
  SectionEnd

  Section /o "Spanish (Old)" SecLang_spa_old
    AddSize 16348
    !insertmacro Download_Lang_Data spa_old
  SectionEnd

  Section /o "Serbian" SecLang_srp
    AddSize 4504
    !insertmacro Download_Lang_Data srp
  SectionEnd

  Section /o "Serbian (Latin)" SecLang_srp_latn
    AddSize 5952
    !insertmacro Download_Lang_Data srp_latn
  SectionEnd

  Section /o "Swahili" SecLang_swa
    AddSize 3772
    !insertmacro Download_Lang_Data swa
  SectionEnd

  Section /o "Swedish" SecLang_swe
    AddSize 9240
    !insertmacro Download_Lang_Data swe
  SectionEnd

!ifdef OLD
  Section /o "Swedish (Fraktur)" SecLang_swe_frak
    AddSize 999
    !insertmacro Download_Lang_Data swe-frak
  SectionEnd
!endif ; OLD

  Section /o "Syriac" SecLang_syr
    AddSize 2672
    !insertmacro Download_Lang_Data syr
  SectionEnd

  Section /o "Tagalog" SecLang_tgl
    AddSize 4020
    !insertmacro Download_Lang_Data tgl
  SectionEnd

  Section /o "Tajik" SecLang_tgk
    AddSize 1096
    !insertmacro Download_Lang_Data tgk
  SectionEnd

  Section /o "Tamil" SecLang_tam
    AddSize 5000
    !insertmacro Download_Lang_Data tam
  SectionEnd

  Section /o "Telugu" SecLang_tel
    AddSize 38404
    !insertmacro Download_Lang_Data tel
  SectionEnd

  Section /o "Thai" SecLang_tha
    AddSize 13248
    !insertmacro Download_Lang_Data tha
  SectionEnd

  Section /o "Tigrinya" SecLang_tir
    AddSize 1764
    !insertmacro Download_Lang_Data tir
  SectionEnd

  Section /o "Turkish" SecLang_tur
    AddSize 13744
    !insertmacro Download_Lang_Data tur
  SectionEnd

  Section /o "Uighur" SecLang_uig
    AddSize 1972
    !insertmacro Download_Lang_Data uig
  SectionEnd

  Section /o "Ukrainian" SecLang_ukr
    AddSize 7856
    !insertmacro Download_Lang_Data ukr
  SectionEnd

  Section /o "Urdu" SecLang_urd
    AddSize 4716
    !insertmacro Download_Lang_Data urd
  SectionEnd

  Section /o "Uzbek" SecLang_uzb
    AddSize 4188
    !insertmacro Download_Lang_Data uzb
  SectionEnd

  Section /o "Uzbek (Cyrilic)" SecLang_uzb_cyrl
    AddSize 3264
    !insertmacro Download_Lang_Data uzb_cyrl
  SectionEnd

  Section /o "Vietnamese" SecLang_vie
    AddSize 5956
    !insertmacro Download_Lang_Data vie
  SectionEnd

  Section /o "Yiddish" SecLang_yid
    AddSize 4140
    !insertmacro Download_Lang_Data yid
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
  DetailPrint "Removing everything"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\*.*"
  RMDir  "$SMPROGRAMS\${PRODUCT_NAME}"
  DetailPrint "Removing registry info"
  DeleteRegKey HKLM "Software\Tesseract-OCR"
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
!ifdef REGISTRY_SETTINGS
  ${un.EnvVarUpdate} $0 "PATH" "R" HKLM $INSTDIR
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  !insertmacro RemoveTessdataPrefix
!endif ; REGISTRY_SETTINGS

  # remove the Add/Remove information
  DeleteRegKey HKLM "${UNINST_KEY}"
  Delete "${UNINST_EXE}"
  DeleteRegValue HKLM "${REGKEY}" Path
  DeleteRegKey /IfEmpty HKLM "${REGKEY}\Components"
  DeleteRegKey /IfEmpty HKLM "${REGKEY}"
  RMDir /r "$INSTDIR"
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
            !insertmacro SelectSection ${SecLang_dan_frak}
            Goto lang_end
    Dutch: !insertmacro SelectSection ${SecLang_nld}
            Goto lang_end
    Estonian: !insertmacro SelectSection ${SecLang_hrv}
            Goto lang_end
    German: !insertmacro SelectSection ${SecLang_deu}
            !insertmacro SelectSection ${SecLang_deu_frak}
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
            !insertmacro SelectSection ${SecLang_slk_frak}
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
!ifdef OLD
            !insertmacro SelectSection ${SecLang_swe_frak}
!endif
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

Function ShowReadme
  Exec "iexplore.exe $INSTDIR\doc\README"
  ;BringToFront
FunctionEnd

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
