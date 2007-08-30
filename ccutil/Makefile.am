SUBDIRS =
AM_CXXFLAGS = -DTESSDATA_PREFIX=@datadir@/

include_HEADERS = \
    basedir.h bits16.h clst.h debugwin.h elst2.h elst.h errcode.h \
    fileerr.h tessopt.h globaloc.h hashfn.h host.h hosthplb.h lsterr.h \
    mainblk.h memblk.h memryerr.h memry.h mfcpch.h ndminx.h notdll.h \
    nwmain.h ocrclass.h ocrshell.h platform.h secname.h serialis.h \
    stderr.h strngs.h tessclas.h tprintf.h varable.h \
    mfcpch.cpp scanutils.cpp scanutils.h unichar.h \
    unicharmap.h unicharset.h boxread.h

lib_LIBRARIES = libtesseract_ccutil.a
libtesseract_ccutil_a_SOURCES = \
    basedir.cpp bits16.cpp clst.cpp debugwin.cpp elst.cpp \
    elst2.cpp errcode.cpp globaloc.cpp hashfn.cpp mainblk.cpp \
    memblk.cpp memry.cpp ocrshell.cpp serialis.cpp strngs.cpp \
    tprintf.cpp varable.cpp unichar.cpp tessopt.cpp \
    unicharmap.cpp unicharset.cpp boxread.cpp
