Name:           tesseract
Version:        3.00
Release:        1%{?dist}
Summary:        Raw Open source OCR Engine 

Group:          Applications/Multimedia
License:        Apache License
URL:            http://code.google.com/p/tesseract-ocr/
Source0:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.tar.gz
Source1:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.bul.tar.gz
Source2:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.cat.tar.gz
Source3:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.ces.tar.gz
Source4:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.dan-frak.tar.gz
Source5:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.dan.tar.gz
Source6:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.deu.tar.gz
Source7:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.ell.tar.gz
Source8:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.eng.tar.gz
Source9:        http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.fin.tar.gz
Source10:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.fra.tar.gz
Source11:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.hun.tar.gz
Source12:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.chi_sim.tar.gz
Source13:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.chi_tra.tar.gz
Source14:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.ind.tar.gz
Source15:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.ita.tar.gz
Source16:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.jpn.tar.gz
Source17:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.kor.tar.gz
Source18:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.lav.tar.gz
Source19:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.lit.tar.gz
Source20:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.nld.tar.gz
Source21:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.nor.tar.gz
Source22:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.pol.tar.gz
Source23:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.por.tar.gz
Source24:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.ron.tar.gz
Source25:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.rus.tar.gz
Source26:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.slk.tar.gz
Source27:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.slv.tar.gz
Source28:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.spa.tar.gz
Source29:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.srp.tar.gz
Source30:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.swe.tar.gz
Source31:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.tgl.tar.gz
Source32:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.tha.tar.gz
Source33:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.tur.tar.gz
Source34:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.ukr.tar.gz
Source35:       http://tesseract-ocr.googlecode.com/files/%{name}-%{version}.vie.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{_id_u} -n)

BuildRequires:  libtiff-devel
BuildRequires:  leptonlib-devel >= 1.60

Requires:       leptonlib >= 1.60
Requires:       tesseract-language >= %version
Provides:       %name = %version-%release
Obsoletes:      %name < 3.00

%description
A commercial quality OCR engine originally developed at HP between 1985 and
1995. In 1995, this engine was among the top 3 evaluated by UNLV. It was
open-sourced by HP and UNLV in 2005. From 2007 it is developed by Google.

%package devel
Summary: Development files for tesseract
Group: Development/Libraries
Requires:       %name = %{version}
Provides:       %name-devel = %version-%release
Obsoletes:      %name-devel < 3.00

%description devel
The %{name}-devel package contains header file for
developing applications that use %{name}.

%prep
%setup -q -a 1

%build
./configure --bindir=%{_bindir} --datadir=%{_datadir} --libdir=%{_libdir} --includedir=%{_includedir}
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog README
%{_bindir}/*
%{_datadir}/tessdata/configs
%{_datadir}/tessdata/tessconfigs
%{_libdir}/lib%{name}*.so*

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}
%{_libdir}/
%exclude %_libdir/lib%{name}*.so*

%package bul
Group:          Applications/Multimedia
Summary:        Bulgarian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description bul
The %{name}-%{version}.bul package contains the data files required to recognize Bulgarian

%files bul
%defattr(-,root,root,-)
%{_datadir}/tessdata/bul.traineddata

%package cat
Group:          Applications/Multimedia
Summary:        Catalan language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description cat
The %{name}-%{version}.cat package contains the data files required to recognize Catalan

%files cat
%defattr(-,root,root,-)
%{_datadir}/tessdata/cat.traineddata

%package ces
Group:          Applications/Multimedia
Summary:        Czech language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description ces
The %{name}-%{version}.ces package contains the data files required to recognize Czech

%files ces
%defattr(-,root,root,-)
%{_datadir}/tessdata/ces.traineddata

%package chi_sim
Group:          Applications/Multimedia
Summary:        Simplified Chinese language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description chi_sim
The %{name}-%{version}.chi_sim package contains the data files required to recognize Simplified Chinese

%files chi_sim
%defattr(-,root,root,-)
%{_datadir}/tessdata/chi_sim.traineddata

%package chi_tra
Group:          Applications/Multimedia
Summary:        Traditional Chinese language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description chi_tra
The %{name}-%{version}.chi_tra package contains the data files required to recognize Traditional Chinese

%files chi_tra
%defattr(-,root,root,-)
%{_datadir}/tessdata/chi_tra.traineddata

%package dan-frak
Group:          Applications/Multimedia
Summary:        Danish (Fraktur) language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description dan-frak
The %{name}-%{version}.dan-frak package contains the data files required to recognize Danish (Fraktur)

%files dan-frak
%defattr(-,root,root,-)
%{_datadir}/tessdata/dan-frak.traineddata

%package dan
Group:          Applications/Multimedia
Summary:        Danish language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description dan
The %{name}-%{version}.dan package contains the data files required to recognize Danish

%files dan
%defattr(-,root,root,-)
%{_datadir}/tessdata/dan.traineddata

%package deu
Group:          Applications/Multimedia
Summary:        German language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description deu
The %{name}-%{version}.deu package contains the data files required to recognize German

%files deu
%defattr(-,root,root,-)
%{_datadir}/tessdata/deu.traineddata

%package ell
Group:          Applications/Multimedia
Summary:        Greek language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description ell
The %{name}-%{version}.ell package contains the data files required to recognize Greek

%files ell
%defattr(-,root,root,-)
%{_datadir}/tessdata/ell.traineddata

%package eng
Group:          Applications/Multimedia
Summary:        English language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description eng
The %{name}-%{version}.eng package contains the data files required to recognize English

%files eng
%defattr(-,root,root,-)
%{_datadir}/tessdata/eng.traineddata

%package fin
Group:          Applications/Multimedia
Summary:        Finnish language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description fin
The %{name}-%{version}.fin package contains the data files required to recognize Finnish

%files fin
%defattr(-,root,root,-)
%{_datadir}/tessdata/fin.traineddata

%package fra
Group:          Applications/Multimedia
Summary:        French language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description fra
The %{name}-%{version}.fra package contains the data files required to recognize French

%files fra
%defattr(-,root,root,-)
%{_datadir}/tessdata/fra.traineddata

%package hun
Group:          Applications/Multimedia
Summary:        Hungarian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description hun
The %{name}-%{version}.hun package contains the data files required to recognize Hungarian

%files hun
%defattr(-,root,root,-)
%{_datadir}/tessdata/hun.traineddata

%package ind
Group:          Applications/Multimedia
Summary:        Indonesian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description ind
The %{name}-%{version}.ind package contains the data files required to recognize Indonesian

%files ind
%defattr(-,root,root,-)
%{_datadir}/tessdata/ind.traineddata

%package ita
Group:          Applications/Multimedia
Summary:        Italian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description ita
The %{name}-%{version}.ita package contains the data files required to recognize Italian

%files ita
%defattr(-,root,root,-)
%{_datadir}/tessdata/ita.traineddata

%package jpn
Group:          Applications/Multimedia
Summary:        Japanese language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description jpn
The %{name}-%{version}.jpn package contains the data files required to recognize Japanese

%files jpn
%defattr(-,root,root,-)
%{_datadir}/tessdata/jpn.traineddata

%package kor
Group:          Applications/Multimedia
Summary:        Korean language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description kor
The %{name}-%{version}.kor package contains the data files required to recognize Korean

%files kor
%defattr(-,root,root,-)
%{_datadir}/tessdata/kor.traineddata

%package lav
Group:          Applications/Multimedia
Summary:        Latvian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description lav
The %{name}-%{version}.lav package contains the data files required to recognize Latvian

%files lav
%defattr(-,root,root,-)
%{_datadir}/tessdata/lav.traineddata

%package lit
Group:          Applications/Multimedia
Summary:        Lithuanian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description lit
The %{name}-%{version}.lit package contains the data files required to recognize Lithuanian

%files lit
%defattr(-,root,root,-)
%{_datadir}/tessdata/lit.traineddata

%package nld
Group:          Applications/Multimedia
Summary:        Dutch language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description nld
The %{name}-%{version}.nld package contains the data files required to recognize Dutch

%files nld
%defattr(-,root,root,-)
%{_datadir}/tessdata/nld.traineddata

%package nor
Group:          Applications/Multimedia
Summary:        Norwegian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description nor
The %{name}-%{version}.nor package contains the data files required to recognize Norwegian

%files nor
%defattr(-,root,root,-)
%{_datadir}/tessdata/nor.traineddata

%package pol
Group:          Applications/Multimedia
Summary:        Polish language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description pol
The %{name}-%{version}.pol package contains the data files required to recognize Polish

%files pol
%defattr(-,root,root,-)
%{_datadir}/tessdata/pol.traineddata

%package por
Group:          Applications/Multimedia
Summary:        Portuguese language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description por
The %{name}-%{version}.por package contains the data files required to recognize Portuguese

%files por
%defattr(-,root,root,-)
%{_datadir}/tessdata/por.traineddata

%package ron
Group:          Applications/Multimedia
Summary:        Romanian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description ron
The %{name}-%{version}.ron package contains the data files required to recognize Romanian

%files ron
%defattr(-,root,root,-)
%{_datadir}/tessdata/ron.traineddata

%package rus
Group:          Applications/Multimedia
Summary:        Russian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description rus
The %{name}-%{version}.rus package contains the data files required to recognize Russian

%files rus
%defattr(-,root,root,-)
%{_datadir}/tessdata/rus.traineddata

%package slk
Group:          Applications/Multimedia
Summary:        Slovakian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description slk
The %{name}-%{version}.slk package contains the data files required to recognize Slovakian

%files slk
%defattr(-,root,root,-)
%{_datadir}/tessdata/slk.traineddata

%package slv
Group:          Applications/Multimedia
Summary:        Slovenian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description slv
The %{name}-%{version}.slv package contains the data files required to recognize Slovenian

%files slv
%defattr(-,root,root,-)
%{_datadir}/tessdata/slv.traineddata

%package spa
Group:          Applications/Multimedia
Summary:        Spanish language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description spa
The %{name}-%{version}.spa package contains the data files required to recognize Spanish

%files spa
%defattr(-,root,root,-)
%{_datadir}/tessdata/spa.traineddata

%package srp
Group:          Applications/Multimedia
Summary:        Serbian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description srp
The %{name}-%{version}.srp package contains the data files required to recognize Serbian

%files srp
%defattr(-,root,root,-)
%{_datadir}/tessdata/srp.traineddata

%package swe
Group:          Applications/Multimedia
Summary:        Swedish language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description swe
The %{name}-%{version}.swe package contains the data files required to recognize Swedish

%files swe
%defattr(-,root,root,-)
%{_datadir}/tessdata/swe.traineddata

%package tgl
Group:          Applications/Multimedia
Summary:        Tagalog language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description tgl
The %{name}-%{version}.tgl package contains the data files required to recognize Tagalog

%files tgl
%defattr(-,root,root,-)
%{_datadir}/tessdata/tgl.traineddata

%package tha
Group:          Applications/Multimedia
Summary:        Thai language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description tha
The %{name}-%{version}.tha package contains the data files required to recognize Thai

%files tha
%defattr(-,root,root,-)
%{_datadir}/tessdata/tha.traineddata

%package tur
Group:          Applications/Multimedia
Summary:        Turkish language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description tur
The %{name}-%{version}.tur package contains the data files required to recognize Turkish

%files tur
%defattr(-,root,root,-)
%{_datadir}/tessdata/tur.traineddata

%package ukr
Group:          Applications/Multimedia
Summary:        Ukrainian language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description ukr
The %{name}-%{version}.ukr package contains the data files required to recognize Ukrainian

%files ukr
%defattr(-,root,root,-)
%{_datadir}/tessdata/ukr.traineddata

%package vie
Group:          Applications/Multimedia
Summary:        Vietnamese language pack for tesseract
Requires:       %name >= %{version}
Provides:       tesseract-language = %{version}
%description vie
The %{name}-%{version}.vie package contains the data files required to recognize Vietnamese

%files vie
%defattr(-,root,root,-)
%{_datadir}/tessdata/vie.traineddata

%changelog
* Sun Sep 26 2010 Zdenko Podobny <zdenop@gmail.com>
- Addaption for release 3.00

* Mon Jul 16 2007 Ray Smith
- Hacked to add the new langeuages as separate language packs.

* Fri May 25 2007 Andrew Ziem
- This is a rough draft that may only work on Fedora Core 6.
