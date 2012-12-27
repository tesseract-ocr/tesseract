; (C) Copyright 2010, Sergey Bronnikov
; (C) Copyright 2010-2012, Zdenko Podobný
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

SetCompressor /FINAL /SOLID lzma
;SetCompressor lzma
SetCompressorDictSize 32

!define VERSION 3.02.02
!define PRODUCT_NAME "Tesseract-OCR"
!define PRODUCT_VERSION "${VERSION}"
!define PRODUCT_PUBLISHER "Tesseract-OCR community"
!define PRODUCT_WEB_SITE "http://code.google.com/p/tesseract-ocr"
!define FILE_URL "http://tesseract-ocr.googlecode.com/files/"

# General Definitions
Name "${PRODUCT_NAME} ${VERSION} for Windows"
Caption "Tesseract-OCR ${VERSION}"
BrandingText /TRIMCENTER "(c) 2010-2012 Tesseract-OCR "
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
!define MUI_FINISHPAGE_LINK "http://code.google.com/p/tesseract-ocr/"
!define MUI_FINISHPAGE_LINK_LOCATION "http://code.google.com/p/tesseract-ocr/"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_SHOWREADME "notepad $INSTDIR\doc\README"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReadme
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
!include EnvVarUpdate.nsh
!include LogicLib.nsh
!include winmessages.nsh # include for some of the windows messages defines

# Variables
Var StartMenuGroup
Var PathKey
; Define user variables
Var OLD_KEY

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!ifdef VERSION
  Page custom PageReinstall PageLeaveReinstall
!endif
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"

# Installer attributes
ShowInstDetails show
InstProgressFlags smooth colored
XPStyle on
SpaceTexts
CRCCheck on
InstProgressFlags smooth colored
CRCCheck On  # Do a CRC check before installing
InstallDir "$PROGRAMFILES\Tesseract-OCR"
# Name of program and file
!ifdef VERSION
OutFile tesseract-ocr-setup-${VERSION}.exe
!else
OutFile tesseract-ocr-setup.exe
!endif

!macro AddToPath
  # TODO(zdenop): Check if $INSTDIR is in path. If yes, that do not append it
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
  ReadRegStr $R3 ${env_hkcu} 'TESSDATA_PREFIX'
  StrCmp $R2 "" Next1 0
    DetailPrint "Removing $R2 from HKLM Environment..."
    DeleteRegValue ${env_hklm} TESSDATA_PREFIX  # This only empty variable, but do not remove it!
    ${EnvVarUpdate} $0 "TESSDATA_PREFIX"  "R" "HKLM" $R1
  Next1:
    StrCmp $R3 "" Next2 0
      DetailPrint "Removing $R3 from HKCU Environment..."
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

!macro Download_Lang_Data Lang
  IfFileExists $TEMP/${Lang} dlok
  ;StrCpy $1 ${Lang}
  ;StrCpy $2 "$INSTDIR\tessdata\$1"
  ;inetc::get /caption "Downloading $1" /popup "" "${FILE_URL}/$1" $2 /end
  inetc::get /caption "Downloading ${Lang}" /popup "" "${FILE_URL}/${Lang}" $TEMP/${Lang} /end
    Pop $0 # return value = exit code, "OK" if OK
    StrCmp $0 "OK" dlok
    MessageBox MB_OK|MB_ICONEXCLAMATION "http download error. Download Status of ${Lang}: $0. Click OK to continue." /SD IDOK
    Goto error
  dlok:
    DetailPrint "Extracting ${Lang}"
    untgz::extract "-j" "-d" "$INSTDIR\tessdata\" "$TEMP/${Lang}" 
    # tarbal has to be created with option --old-archive otherwise there will be error
    # untgz::extract failed because of checksum
  error:
    Delete "$TEMP\${Lang}"
!macroend

!macro Download_Leptonica DataUrl
  IfFileExists $TEMP/leptonica.zip dlok
  inetc::get /caption "Downloading $1" /popup "" ${DataUrl} $TEMP/leptonica.zip /end
    Pop $R0 # return value = exit code, "OK" if OK
    StrCmp $R0 "OK" dlok
    MessageBox MB_OK|MB_ICONEXCLAMATION "http download error. Download Status of $1: $R0. Click OK to continue." /SD IDOK
    Goto error
  dlok:
    nsisunz::UnzipToLog "$TEMP/leptonica.zip" "$INSTDIR"
    Pop $R0
    StrCmp $R0 "success" +2
        MessageBox MB_OK "Decompression of leptonica failed: $R0"
        Goto error
  error:
    Delete "$TEMP\leptonica.zip"
!macroend

!macro Download_Lang_Data_gz Lang
  ;IfFileExists $TEMP/${Lang} dlok
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

!macro Download_Data2 Filename Komp
  IfFileExists $TEMP/${Filename} dlok
  inetc::get /caption "Downloading $1" /popup "" "${FILE_URL}/${Filename}" $TEMP/${Filename} /end
    Pop $R0 # return value = exit code, "OK" if OK
    StrCmp $R0 "OK" dlok
    MessageBox MB_OK|MB_ICONEXCLAMATION "http download error. Download Status of $1: $R0. Click OK to continue." /SD IDOK
    Goto error
  dlok:
    ${If} ${Komp} == "tgz"
        DetailPrint "Extracting ${Filename}"
        untgz::extract "-d" "$INSTDIR\.." "$TEMP\${Filename}"
        Goto install
    ${EndIf}
    ${If} ${Komp} == "zip"
        DetailPrint "Extracting ${Filename}"
        nsisunz::UnzipToLog "$TEMP\${Filename}" "$INSTDIR\"
        Goto install
    ${EndIf}
     MessageBox MB_OK|MB_ICONEXCLAMATION "Unsupported compression!"
  install:
        Pop $R0
        StrCmp $R0 "success" +3
            MessageBox MB_OK "Decompression of ${Filename} failed: $R0"
            Goto error
    Delete "$TEMP\${Filename}"
  error:
!macroend

!macro Download_Data Filename Komp
  IfFileExists $TEMP/${Filename} dlok
  inetc::get /caption "Downloading $1" /popup "" "${FILE_URL}/${Filename}" $TEMP/${Filename} /end
    Pop $R0 # return value = exit code, "OK" if OK
    StrCmp $R0 "OK" dlok
    MessageBox MB_OK|MB_ICONEXCLAMATION "http download error. Download Status of $1: $R0. Click OK to continue." /SD IDOK
    Goto end
  dlok:
    ${If} ${Komp} == "tgz"
        untgz::extract "-d" "$INSTDIR" "$TEMP\${Filename}"
        Goto install
    ${EndIf}
    ${If} ${Komp} == "zip"
        nsisunz::UnzipToLog "$TEMP\${Filename}" "$INSTDIR"
        Goto install
    ${EndIf}
     MessageBox MB_OK|MB_ICONEXCLAMATION "Unsupported compression!"
  install:
        Pop $R0
        StrCmp $R0 "success" +3
            MessageBox MB_OK "Decompression of ${Filename} failed: $R0"
            Goto end
    Delete "$TEMP\${Filename}"
    ${If} ${Komp} == "zip"
        Goto end
    ${EndIf}
    CopyFiles "$TEMP\Tesseract-OCR\*" "$INSTDIR"
    RMDir /r "$TEMP\Tesseract-OCR"
  end:
!macroend

Section -Main SEC0000
  ; mark as read only component
  SectionIn RO
  SetOutPath "$INSTDIR"
  # files included in distribution
  File LIB_Release\tesseract.exe
  File gzip.exe
  File tar.exe
  CreateDirectory "$INSTDIR\java"
  SetOutPath "$INSTDIR\java"
  File ..\java\ScrollView.jar
  CreateDirectory "$INSTDIR\tessdata"
  CreateDirectory "$INSTDIR\tessdata\configs"
  SetOutPath "$INSTDIR\tessdata\configs"
  File ..\tessdata\configs\ambigs.train
  File ..\tessdata\configs\api_config
  File ..\tessdata\configs\bigram
  File ..\tessdata\configs\box.train
  File ..\tessdata\configs\box.train.stderr
  File ..\tessdata\configs\digits
  File ..\tessdata\configs\hocr
  File ..\tessdata\configs\inter
  File ..\tessdata\configs\kannada
  File ..\tessdata\configs\linebox
  File ..\tessdata\configs\logfile
  File ..\tessdata\configs\makebox
  File ..\tessdata\configs\quiet
  File ..\tessdata\configs\rebox
  File ..\tessdata\configs\strokewidth
  File ..\tessdata\configs\unlv
  CreateDirectory "$INSTDIR\tessdata\tessconfigs"
  SetOutPath "$INSTDIR\tessdata\tessconfigs"
  File ..\tessdata\tessconfigs\batch
  File ..\tessdata\tessconfigs\batch.nochop
  File ..\tessdata\tessconfigs\matdemo
  File ..\tessdata\tessconfigs\msdemo
  File ..\tessdata\tessconfigs\nobatch
  File ..\tessdata\tessconfigs\segdemo
  CreateDirectory "$INSTDIR\doc"
  SetOutPath "$INSTDIR\doc"
  File ..\AUTHORS
  File ..\COPYING
  File ..\eurotext.tif
  File ..\phototest.tif
  File ..\README
  File ..\ReleaseNotes
SectionEnd

Section "Traning Tools" SecTr
  SectionIn 1
  SetOutPath "$INSTDIR"
  File LIB_Release\cntraining.exe
  File LIB_Release\combine_tessdata.exe
  File LIB_Release\mftraining.exe
  File LIB_Release\unicharset_extractor.exe
  File LIB_Release\wordlist2dawg.exe
  File LIB_Release\classifier_tester.exe
  File LIB_Release\dawg2wordlist.exe
  File LIB_Release\ambiguous_words.exe
  File LIB_Release\shapeclustering.exe
SectionEnd

Section -post SEC0001
  ;Store installation folder - we use allways HKLM!
  WriteRegStr HKLM "${REGKEY}" "Path" "$INSTDIR"
  WriteRegStr HKLM "${REGKEY}" "Mode" $MultiUser.InstallMode
  WriteRegStr HKLM "${REGKEY}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "${REGKEY}" "CurrentVersion" "${VERSION}"
  WriteRegStr HKLM "${REGKEY}" "Uninstaller" "$INSTDIR\uninstall.exe"
  ;WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "Tesseract-OCR" "$INSTDIR\tesseract.exe"
  ; Register to Add/Remove program in control panel
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME} - open source OCR engine"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" DisplayVersion "${VERSION}"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" Publisher "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" URLInfoAbout "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayIcon" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" NoModify 1
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" NoRepair 1
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  ;ExecShell "open" "http://code.google.com/p/tesseract-ocr/"
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
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ;CreateShortCut "$DESKTOP\Tesseract-OCR.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
  ;CreateShortCut "$QUICKLAUNCH\.lnk" "$INSTDIR\tesseract.exe" "" "$INSTDIR\tesseract.exe" 0
SectionEnd

SectionGroup "Registry setttings" SecRS
    Section "Add to Path" SecRS_path
        !insertmacro AddToPath
    SectionEnd
    Section "Set TESSDATA_PREFIX variable" SecRS_tessdata
        !insertmacro SetTESSDATA
    SectionEnd
SectionGroupEnd

SectionGroup "Tesseract development files" SecGrp_dev
    Section /o "Download and install tesseract libraries including header files" SecLang_tlib
    !insertmacro Download_Data2 tesseract-ocr-3.02.02-win32-lib-include-dirs.zip zip
    CopyFiles $INSTDIR\lib\libtesseract*.dll $INSTDIR\  ; $INSTDIR is in the path!
    Delete $INSTDIR\lib\libtesseract*.dll
    SectionEnd
    Section /o "Download and install leptonica 1.68 libraries including header files" SecLang_llib
    !insertmacro Download_Leptonica http://leptonica.org/source/leptonica-1.68-win32-lib-include-dirs.zip
    CopyFiles $INSTDIR\lib\liblept*.dll $INSTDIR\  ; move to path
    Delete $INSTDIR\lib\liblept*.dll
    SectionEnd
    Section /o "Download and install VC++ 2008 tesseract API example solution" SecLang_example
    !insertmacro Download_Data2 tesseract-ocr-API-Example-vs2008.zip zip
    SectionEnd
    Section /o "Download and install tesseract source code" SecLang_source
    !insertmacro Download_Data tesseract-ocr-3.02.02.tar.gz tgz
    SectionEnd
    Section /o "Download and install VS C++ 2008 solution for tesseract" SecLang_vs2008
    !insertmacro Download_Data tesseract-ocr-3.02-vs2008.zip zip
    SectionEnd
    Section /o "Download and install doxygen documentation for tesseract" SecLang_doxygen
    !insertmacro Download_Data tesseract-ocr-3.02.02-doc-html.tar.gz tgz
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\DoxygenDoc.lnk" "$INSTDIR\tesseract-ocr\doc\html\index.html"
    SectionEnd
SectionGroupEnd

; Download language files
SectionGroup "Language data" SecGrp_LD
    Section "English language data" SecLang_eng
    SectionIn RO
      SetOutPath "$INSTDIR\tessdata"
      File ..\tessdata\eng.*
    SectionEnd

    Section "Orientation and script detection data" SecLang_osd
    SectionIn 1
      SetOutPath "$INSTDIR\tessdata"
     File ..\tessdata\osd.*
    SectionEnd

    Section /o "Download and install Math / equation detection module" SecLang_equ
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.equ.tar.gz
    SectionEnd

    Section /o "Download and install Afrikaans language data" SecLang_afr
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.afr.tar.gz
    SectionEnd

    Section /o "Download and install Albanian language data" SecLang_sqi
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.sqi.tar.gz
    SectionEnd

    Section /o "Download and install Arabic language data" SecLang_ara
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ara.tar.gz
    SectionEnd

    Section /o "Download and install Azerbaijani language data" SecLang_aze
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.aze.tar.gz
    SectionEnd

    Section /o "Download and install Basque language data" SecLang_eus
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.eus.tar.gz
    SectionEnd

    Section /o "Download and install Belarusian language data" SecLang_bel
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.bel.tar.gz
    SectionEnd

    Section /o "Download and install Bengali language data" SecLang_ben
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ben.tar.gz
    SectionEnd

    Section /o "Download and install Bulgarian language data" SecLang_bul
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.bul.tar.gz
    SectionEnd

    Section /o "Download and install Catalan language data" SecLang_cat
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.cat.tar.gz
    SectionEnd

    Section /o "Download and install Cherokee language data" SecLang_chr
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.chr.tar.gz
    SectionEnd

    Section /o "Download and install Chinese (Traditional) language data" SecLang_chi_tra
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.chi_tra.tar.gz
    SectionEnd

    Section /o "Download and install Chinese (Simplified) language data" SecLang_chi_sim
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.chi_sim.tar.gz
    SectionEnd

    Section /o "Download and install Croatian language data" SecLang_hrv
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.hrv.tar.gz
    SectionEnd

    Section /o "Download and install Czech language data" SecLang_ces
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ces.tar.gz
    SectionEnd

    Section /o "Download and install Danish language data" SecLang_dan
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.dan.tar.gz
    SectionEnd

    Section /o "Download and install Danish (Fraktur) language data" SecLang_dan_frak
    !insertmacro Download_Lang_Data_gz dan-frak.traineddata.gz
    SectionEnd

    Section /o "Download and install Dutch language data" SecLang_nld
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.nld.tar.gz
    SectionEnd

    Section /o "Download and install English - Middle (1100-1500) language data" SecLang_enm
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.enm.tar.gz
    SectionEnd

    Section /o "Download and install Esperanto language data" SecLang_epo
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.epo.tar.gz
    SectionEnd

    Section /o "Download and install Estonian language data" SecLang_est
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.est.tar.gz
    SectionEnd

    Section /o "Download and install German language data" SecLang_deu
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.deu.tar.gz
    SectionEnd

    Section /o "Download and install German (Fraktur) language data" SecLang_deu_frak
    !insertmacro Download_Lang_Data_gz deu-frak.traineddata.gz
    SectionEnd

    Section /o "Download and install Greek language data" SecLang_ell
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ell.tar.gz
    SectionEnd

    Section /o "Download and install Greek - Ancient language data" SecLang_grc
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.grc.tar.gz
    SectionEnd

    Section /o "Download and install Finnish language data" SecLang_fin
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.fin.tar.gz
    SectionEnd

    Section /o "Download and install Frankish language data" SecLang_frk
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.frk.tar.gz
    SectionEnd

    Section /o "Download and install French language data" SecLang_fra
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.fra.tar.gz
    SectionEnd

    Section /o "Download and install French - Middle(ca. 1400-1600) language data" SecLang_frm
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.frm.tar.gz
    SectionEnd

    Section /o "Download and install Hebrew language data" SecLang_heb
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.heb.tar.gz
    SectionEnd

    Section /o "Download and install Hebrew (community traning) language data" SecLang_heb_com
    !insertmacro Download_Lang_Data tesseract-ocr-3.01.heb-com.tar.gz
    SectionEnd

    Section /o "Download and install Hindi language data" SecLang_hin
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.hin.tar.gz
    SectionEnd

    Section /o "Download and install Hungarian language data" SecLang_hun
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.hun.tar.gz
    SectionEnd

    Section /o "Download and install Icelandic language data" SecLang_isl
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.isl.tar.gz
    SectionEnd

    Section /o "Download and install Indonesian language data" SecLang_ind
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ind.tar.gz
    SectionEnd

    Section /o "Download and install Italian language data" SecLang_ita
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ita.tar.gz
    SectionEnd

    Section /o "Download and install Italian (Old) language data" SecLang_ita_old
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ita_old.tar.gz
    SectionEnd

    Section /o "Download and install Japanese language data" SecLang_jpn
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.jpn.tar.gz
    SectionEnd

    Section /o "Download and install Kannada language data" SecLang_kan
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.kan.tar.gz
    SectionEnd

    Section /o "Download and install Korean language data" SecLang_kor
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.kor.tar.gz
    SectionEnd

    Section /o "Download and install Latvian language data" SecLang_lav
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.lav.tar.gz
    SectionEnd

    Section /o "Download and install Lithuanian language data" SecLang_lit
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.lit.tar.gz
    SectionEnd

    Section /o "Download and install Macedonian language data" SecLang_mkd
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.mkd.tar.gz
    SectionEnd

    Section /o "Download and install Malay language data" SecLang_msa
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.msa.tar.gz
    SectionEnd

    Section /o "Download and install Malayalam language data" SecLang_mal
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.mal.tar.gz
    SectionEnd

    Section /o "Download and install Maltese language data" SecLang_mlt
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.mlt.tar.gz
    SectionEnd

    Section /o "Download and install Norwegian language data" SecLang_nor
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.nor.tar.gz
    SectionEnd

    Section /o "Download and install Polish language data" SecLang_pol
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.pol.tar.gz
    SectionEnd

    Section /o "Download and install Portuguese language data" SecLang_por
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.por.tar.gz
    SectionEnd

    Section /o "Download and install Romanian language data" SecLang_ron
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ron.tar.gz
    SectionEnd

    Section /o "Download and install Russian language data" SecLang_rus
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.rus.tar.gz
    SectionEnd

    Section /o "Download and install Slovak language data" SecLang_slk
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.slk.tar.gz
    SectionEnd

    Section /o "Download and install Slovak (Fraktur) language data" SecLang_slk_frak
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.slk_frak.tar.gz
    SectionEnd

    Section /o "Download and install Slovenian language data" SecLang_slv
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.slv.tar.gz
    SectionEnd

    Section /o "Download and install Spanish language data" SecLang_spa
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.spa.tar.gz
    SectionEnd

    Section /o "Download and install Spanish (Old) language data" SecLang_spa_old
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.spa_old.tar.gz
    SectionEnd

    Section /o "Download and install Serbian language data" SecLang_srp
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.srp.tar.gz
    SectionEnd

    Section /o "Download and install Swahili language data" SecLang_swa
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.swa.tar.gz
    SectionEnd

    Section /o "Download and install Swedish language data" SecLang_swe
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.swe.tar.gz
    SectionEnd

    Section /o "Download and install Swedish (Fraktur) language data" SecLang_swe_frak
    !insertmacro Download_Lang_Data_gz swe-frak.traineddata.gz
    SectionEnd

    Section /o "Download and install Tagalog language data" SecLang_tgl
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.tgl.tar.gz
    SectionEnd

    Section /o "Download and install Tamil language data" SecLang_tam
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.tam.tar.gz
    SectionEnd

    Section /o "Download and install Telugu language data" SecLang_tel
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.tel.tar.gz
    SectionEnd

    Section /o "Download and install Thai language data" SecLang_tha
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.tha.tar.gz
    SectionEnd

    Section /o "Download and install Turkish language data" SecLang_tur
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.tur.tar.gz
    SectionEnd

    Section /o "Download and install Ukrainian language data" SecLang_ukr
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.ukr.tar.gz
    SectionEnd

    Section /o "Download and install Vietnamese language data" SecLang_vie
    !insertmacro Download_Lang_Data tesseract-ocr-3.02.vie.tar.gz
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
  ${un.EnvVarUpdate} $0 "PATH" "R" HKLM $INSTDIR
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  DeleteRegValue HKLM "Environment" "TESSDATA_PREFIX"
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

  # remove the Add/Remove information
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
  Delete "$INSTDIR\Uninstall.exe"
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
  ;RequestExecutionLevel admin
  !insertmacro MULTIUSER_INIT

  ; is tesseract already installed?
  ReadRegStr $R0 HKCU "${REGKEY}" "CurrentVersion"
  StrCpy $OLD_KEY HKCU
  StrCmp $R0 "" test1 test2
  test1:
    ReadRegStr $R0 HKLM "${REGKEY}" "CurrentVersion"
    StrCpy $OLD_KEY HKLM
    StrCmp $R0 "" SkipUnInstall
  test2:
    MessageBox MB_YESNO|MB_ICONEXCLAMATION "Tesseract-ocr version $R0 is installed (in $OLD_KEY)! Do you want to uninstall it first?$\nUninstall will delete all files in '$INSTDIR'!" \
       /SD IDYES IDNO SkipUnInstall IDYES UnInstall
  UnInstall:
    StrCmp $OLD_KEY "HKLM" UnInst_hklm
       DetailPrint "CurrentUser:"
       readRegStr $R1 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString"
       Goto try_uninstall
    UnInst_hklm:
       DetailPrint "UnInst_hklm"
       readRegStr $R1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString"
    try_uninstall:
      ClearErrors
      ExecWait '$R1 _?=$INSTDIR'$0
      StrCmp $0 0 0 +3   ; Check if unstaller finished ok. If yes, than try to remove it from installer
        !insertmacro REMOVE_REGKEY ${OLD_KEY}
        Goto SkipUnInstall
      messagebox mb_ok "Uninstaller failed:\n$0\n\nYou need to remove program manually."
  SkipUnInstall:
  MessageBox MB_YESNO|MB_ICONQUESTION "Do you want to install ${PRODUCT_NAME} ${VERSION}?" \
    /SD IDYES IDNO no IDYES yes
  no:
    SetSilent silent
    Goto done
  yes:
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
            !insertmacro SelectSection ${SecLang_heb_com}
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
            !insertmacro SelectSection ${SecLang_swe_frak}
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
   !insertmacro MULTIUSER_UNINIT
   ;!insertmacro SELECT_UNSECTION Main ${UNSEC0000}
   ;!insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Function .onInstFailed
  MessageBox MB_OK "Installation failed."
FunctionEnd

Function ShowReadme
  Exec "explorer.exe $INSTDIR\doc\README"
  ;BringToFront
FunctionEnd

; Prevent running multiple instances of the installer
Function PreventMultipleInstances
  Push $R0
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t ${PRODUCT_NAME}) ?e'
  Pop $R0
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running." /SD IDOK
    Abort
  Pop $R0
FunctionEnd

