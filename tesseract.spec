# This is a rough draft that may only work on Fedora Core 6.
# Andrew Ziem, 25 May 2007
# Hacked to add the new langeuages as separate language packs.
# Ray Smith, 16 July 2007


Name:           tesseract
Version:        2.00
Release:        1%{?dist}
Summary:        Open source OCR Engine developed by HP Labs - now improved by Google

Group:          Applications/Multimedia
License:        Apache License
URL:            http://code.google.com/p/tesseract-ocr/
Source0:        http://tesseract-ocr.googlecode.com/files/tesseract-%{version}.tar.gz
Source1:        http://tesseract-ocr.googlecode.com/files/tesseract-%{version}.eng.tar.gz
Source2:        http://tesseract-ocr.googlecode.com/files/tesseract-%{version}.fra.tar.gz
Source3:        http://tesseract-ocr.googlecode.com/files/tesseract-%{version}.ita.tar.gz
Source4:        http://tesseract-ocr.googlecode.com/files/tesseract-%{version}.deu.tar.gz
Source5:        http://tesseract-ocr.googlecode.com/files/tesseract-%{version}.spa.tar.gz
Source6:        http://tesseract-ocr.googlecode.com/files/tesseract-%{version}.nld.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

#BuildRequires:  compat-gcc-34-c++
BuildRequires:  libtiff-devel

%package devel
Summary: Development files for tesseract
Group: Development/Libraries
Requires: %name = %{version}


%description
The Tesseract OCR engine was one of the top 3 engines in the 1995 UNLV
Accuracy test. Since then it has had little work done on it, but it is
probably one of the most accurate open source OCR engines available. The
source code will read a binary, grey or color image and output text. A tiff
reader is built in that will read uncompressed TIFF images, or libtiff can
be added to read compressed images.

%description devel
tesseract libraries and includes

%prep
%setup -q

tar xzvf %{_sourcedir}/tesseract-%{version}.eng.tar.gz
tar xzvf %{_sourcedir}/tesseract-%{version}.fra.tar.gz
tar xzvf %{_sourcedir}/tesseract-%{version}.ita.tar.gz
tar xzvf %{_sourcedir}/tesseract-%{version}.deu.tar.gz
tar xzvf %{_sourcedir}/tesseract-%{version}.spa.tar.gz
tar xzvf %{_sourcedir}/tesseract-%{version}.nld.tar.gz


%build
export CFLAGS=
export CXXFLAGS=
# Should build with gcc4.1 now...
#export CC=gcc34
#export CXX=g++34
# % configure
./configure --bindir=%{_bindir} --datadir=%{_datadir} --libdir=%{_libdir}  --includedir=%{_includedir}
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog README
%{_bindir}/cntraining
%{_bindir}/mftraining
%{_bindir}/tesseract
%{_bindir}/unicharset_extractor
%{_bindir}/wordlist2dawg
%{_datadir}/tessdata/configs
%{_datadir}/tessdata/confsets
%{_datadir}/tessdata/tessconfigs

%files devel
%{_includedir}/tesseract/
%{_libdir}/libtesseract*

%package eng
Group:          Applications/Multimedia
Summary:        English language pack for tesseract
%description eng
The %{name}-%{version}.eng package contains the data files required to recognize English

%files eng
%{_datadir}/tessdata/eng.DangAmbigs
%{_datadir}/tessdata/eng.freq-dawg
%{_datadir}/tessdata/eng.inttemp
%{_datadir}/tessdata/eng.normproto
%{_datadir}/tessdata/eng.pffmtable
%{_datadir}/tessdata/eng.unicharset
%{_datadir}/tessdata/eng.user-words
%{_datadir}/tessdata/eng.word-dawg

%package fra
Group:          Applications/Multimedia
Summary:        French language pack for tesseract
%description fra
The %{name}-%{version}.fra package contains the data files required to recognize French

%files fra
%{_datadir}/tessdata/fra.DangAmbigs
%{_datadir}/tessdata/fra.freq-dawg
%{_datadir}/tessdata/fra.inttemp
%{_datadir}/tessdata/fra.normproto
%{_datadir}/tessdata/fra.pffmtable
%{_datadir}/tessdata/fra.unicharset
%{_datadir}/tessdata/fra.user-words
%{_datadir}/tessdata/fra.word-dawg

%package ita
Group:          Applications/Multimedia
Summary:        Italian language pack for tesseract
%description ita
The %{name}-%{version}.ita package contains the data files required to recognize Italian

%files ita
%{_datadir}/tessdata/ita.DangAmbigs
%{_datadir}/tessdata/ita.freq-dawg
%{_datadir}/tessdata/ita.inttemp
%{_datadir}/tessdata/ita.normproto
%{_datadir}/tessdata/ita.pffmtable
%{_datadir}/tessdata/ita.unicharset
%{_datadir}/tessdata/ita.user-words
%{_datadir}/tessdata/ita.word-dawg

%package deu
Group:          Applications/Multimedia
Summary:        German language pack for tesseract
%description deu
The %{name}-%{version}.deu package contains the data files required to recognize German

%files deu
%{_datadir}/tessdata/deu.DangAmbigs
%{_datadir}/tessdata/deu.freq-dawg
%{_datadir}/tessdata/deu.inttemp
%{_datadir}/tessdata/deu.normproto
%{_datadir}/tessdata/deu.pffmtable
%{_datadir}/tessdata/deu.unicharset
%{_datadir}/tessdata/deu.user-words
%{_datadir}/tessdata/deu.word-dawg

%package spa
Group:          Applications/Multimedia
Summary:        Spanish language pack for tesseract
%description spa
The %{name}-%{version}.spa package contains the data files required to recognize Spanish

%files spa
%{_datadir}/tessdata/spa.DangAmbigs
%{_datadir}/tessdata/spa.freq-dawg
%{_datadir}/tessdata/spa.inttemp
%{_datadir}/tessdata/spa.normproto
%{_datadir}/tessdata/spa.pffmtable
%{_datadir}/tessdata/spa.unicharset
%{_datadir}/tessdata/spa.user-words
%{_datadir}/tessdata/spa.word-dawg

%package nld
Group:          Applications/Multimedia
Summary:        Dutch language pack for tesseract
%description nld
The %{name}-%{version}.nld package contains the data files required to recognize Dutch

%files nld
%{_datadir}/tessdata/nld.DangAmbigs
%{_datadir}/tessdata/nld.freq-dawg
%{_datadir}/tessdata/nld.inttemp
%{_datadir}/tessdata/nld.normproto
%{_datadir}/tessdata/nld.pffmtable
%{_datadir}/tessdata/nld.unicharset
%{_datadir}/tessdata/nld.user-words
%{_datadir}/tessdata/nld.word-dawg


%changelog
