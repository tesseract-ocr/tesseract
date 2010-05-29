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

%build
export CFLAGS=
export CXXFLAGS=
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

%package bul
Group:          Applications/Multimedia
Summary:        Bulgarian language pack for tesseract
%description bul
The %{name}-%{version}.bul package contains the data files required to recognize Bulgarian

%files bul
%{_datadir}/tessdata/bul.traineddata

%package cat
Group:          Applications/Multimedia
Summary:        Catalan language pack for tesseract
%description cat
The %{name}-%{version}.cat package contains the data files required to recognize Catalan

%files cat
%{_datadir}/tessdata/cat.traineddata

%package ces
Group:          Applications/Multimedia
Summary:        Czech language pack for tesseract
%description ces
The %{name}-%{version}.ces package contains the data files required to recognize Czech

%files ces
%{_datadir}/tessdata/ces.traineddata

%package chi_sim
Group:          Applications/Multimedia
Summary:        Simplified Chinese language pack for tesseract
%description chi_sim
The %{name}-%{version}.chi_sim package contains the data files required to recognize Simplified Chinese

%files chi_sim
%{_datadir}/tessdata/chi_sim.traineddata

%package chi_tra
Group:          Applications/Multimedia
Summary:        Traditional Chinese language pack for tesseract
%description chi_tra
The %{name}-%{version}.chi_tra package contains the data files required to recognize Traditional Chinese

%files chi_tra
%{_datadir}/tessdata/chi_tra.traineddata

%package dan-frak
Group:          Applications/Multimedia
Summary:        Danish (Fraktur) language pack for tesseract
%description dan-frak
The %{name}-%{version}.dan-frak package contains the data files required to recognize Danish (Fraktur)

%files dan-frak
%{_datadir}/tessdata/dan-frak.traineddata

%package dan
Group:          Applications/Multimedia
Summary:        Danish language pack for tesseract
%description dan
The %{name}-%{version}.dan package contains the data files required to recognize Danish

%files dan
%{_datadir}/tessdata/dan.traineddata

%package deu
Group:          Applications/Multimedia
Summary:        German language pack for tesseract
%description deu
The %{name}-%{version}.deu package contains the data files required to recognize German

%files deu
%{_datadir}/tessdata/deu.traineddata

%package ell
Group:          Applications/Multimedia
Summary:        Greek language pack for tesseract
%description ell
The %{name}-%{version}.ell package contains the data files required to recognize Greek

%files ell
%{_datadir}/tessdata/ell.traineddata

%package eng
Group:          Applications/Multimedia
Summary:        English language pack for tesseract
%description eng
The %{name}-%{version}.eng package contains the data files required to recognize English

%files eng
%{_datadir}/tessdata/eng.traineddata

%package fin
Group:          Applications/Multimedia
Summary:        Finnish language pack for tesseract
%description fin
The %{name}-%{version}.fin package contains the data files required to recognize Finnish

%files fin
%{_datadir}/tessdata/fin.traineddata

%package fra
Group:          Applications/Multimedia
Summary:        French language pack for tesseract
%description fra
The %{name}-%{version}.fra package contains the data files required to recognize French

%files fra
%{_datadir}/tessdata/fra.traineddata

%package hun
Group:          Applications/Multimedia
Summary:        Hungarian language pack for tesseract
%description hun
The %{name}-%{version}.hun package contains the data files required to recognize Hungarian

%files hun
%{_datadir}/tessdata/hun.traineddata

%package ind
Group:          Applications/Multimedia
Summary:        Indonesian language pack for tesseract
%description ind
The %{name}-%{version}.ind package contains the data files required to recognize Indonesian

%files ind
%{_datadir}/tessdata/ind.traineddata

%package ita
Group:          Applications/Multimedia
Summary:        Italian language pack for tesseract
%description ita
The %{name}-%{version}.ita package contains the data files required to recognize Italian

%files ita
%{_datadir}/tessdata/ita.traineddata

%package jpn
Group:          Applications/Multimedia
Summary:        Japanese language pack for tesseract
%description jpn
The %{name}-%{version}.jpn package contains the data files required to recognize Japanese

%files jpn
%{_datadir}/tessdata/jpn.traineddata

%package kor
Group:          Applications/Multimedia
Summary:        Korean language pack for tesseract
%description kor
The %{name}-%{version}.kor package contains the data files required to recognize Korean

%files kor
%{_datadir}/tessdata/kor.traineddata

%package lav
Group:          Applications/Multimedia
Summary:        Latvian language pack for tesseract
%description lav
The %{name}-%{version}.lav package contains the data files required to recognize Latvian

%files lav
%{_datadir}/tessdata/lav.traineddata

%package lit
Group:          Applications/Multimedia
Summary:        Lithuanian language pack for tesseract
%description lit
The %{name}-%{version}.lit package contains the data files required to recognize Lithuanian

%files lit
%{_datadir}/tessdata/lit.traineddata

%package nld
Group:          Applications/Multimedia
Summary:        Dutch language pack for tesseract
%description nld
The %{name}-%{version}.nld package contains the data files required to recognize Dutch

%files nld
%{_datadir}/tessdata/nld.traineddata

%package nor
Group:          Applications/Multimedia
Summary:        Norwegian language pack for tesseract
%description nor
The %{name}-%{version}.nor package contains the data files required to recognize Norwegian

%files nor
%{_datadir}/tessdata/nor.traineddata

%package pol
Group:          Applications/Multimedia
Summary:        Polish language pack for tesseract
%description pol
The %{name}-%{version}.pol package contains the data files required to recognize Polish

%files pol
%{_datadir}/tessdata/pol.traineddata

%package por
Group:          Applications/Multimedia
Summary:        Portuguese language pack for tesseract
%description por
The %{name}-%{version}.por package contains the data files required to recognize Portuguese

%files por
%{_datadir}/tessdata/por.traineddata

%package ron
Group:          Applications/Multimedia
Summary:        Romanian language pack for tesseract
%description ron
The %{name}-%{version}.ron package contains the data files required to recognize Romanian

%files ron
%{_datadir}/tessdata/ron.traineddata

%package rus
Group:          Applications/Multimedia
Summary:        Russian language pack for tesseract
%description rus
The %{name}-%{version}.rus package contains the data files required to recognize Russian

%files rus
%{_datadir}/tessdata/rus.traineddata

%package slk
Group:          Applications/Multimedia
Summary:        Slovakian language pack for tesseract
%description slk
The %{name}-%{version}.slk package contains the data files required to recognize Slovakian

%files slk
%{_datadir}/tessdata/slk.traineddata

%package slv
Group:          Applications/Multimedia
Summary:        Slovenian language pack for tesseract
%description slv
The %{name}-%{version}.slv package contains the data files required to recognize Slovenian

%files slv
%{_datadir}/tessdata/slv.traineddata

%package spa
Group:          Applications/Multimedia
Summary:        Spanish language pack for tesseract
%description spa
The %{name}-%{version}.spa package contains the data files required to recognize Spanish

%files spa
%{_datadir}/tessdata/spa.traineddata

%package srp
Group:          Applications/Multimedia
Summary:        Serbian language pack for tesseract
%description srp
The %{name}-%{version}.srp package contains the data files required to recognize Serbian

%files srp
%{_datadir}/tessdata/srp.traineddata

%package swe
Group:          Applications/Multimedia
Summary:        Swedish language pack for tesseract
%description swe
The %{name}-%{version}.swe package contains the data files required to recognize Swedish

%files swe
%{_datadir}/tessdata/swe.traineddata

%package tgl
Group:          Applications/Multimedia
Summary:        Tagalog language pack for tesseract
%description tgl
The %{name}-%{version}.tgl package contains the data files required to recognize Tagalog

%files tgl
%{_datadir}/tessdata/tgl.traineddata

%package tha
Group:          Applications/Multimedia
Summary:        Thai language pack for tesseract
%description tha
The %{name}-%{version}.tha package contains the data files required to recognize Thai

%files tha
%{_datadir}/tessdata/tha.traineddata

%package tur
Group:          Applications/Multimedia
Summary:        Turkish language pack for tesseract
%description tur
The %{name}-%{version}.tur package contains the data files required to recognize Turkish

%files tur
%{_datadir}/tessdata/tur.traineddata

%package ukr
Group:          Applications/Multimedia
Summary:        Ukrainian language pack for tesseract
%description ukr
The %{name}-%{version}.ukr package contains the data files required to recognize Ukrainian

%files ukr
%{_datadir}/tessdata/ukr.traineddata

%package vie
Group:          Applications/Multimedia
Summary:        Vietnamese language pack for tesseract
%description vie
The %{name}-%{version}.vie package contains the data files required to recognize Vietnamese

%files vie
%{_datadir}/tessdata/vie.traineddata

%changelog
