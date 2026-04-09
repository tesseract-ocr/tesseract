AUTOMAKE_OPTIONS = subdir-objects

all:

if MINGW

gitrev="$(shell git --git-dir=${abs_top_srcdir}/.git --work-tree=${abs_top_srcdir} describe --always --tags | sed s/^v//)"

.PHONY: winsetup

Plugins/x86-unicode/INetC.dll:
	curl -OsS https://nsis.sourceforge.io/mediawiki/images/c/c9/Inetc.zip
	unzip Inetc.zip $@

winpath.exe: winpath.cpp
	x86_64-w64-mingw32-g++ -Os -o $@ $<
	x86_64-w64-mingw32-strip --strip-unneeded $@

winsetup: Plugins/x86-unicode/INetC.dll winpath.exe
	makensis -DCROSSBUILD -DSHARED -DSIGNCODE=$(SIGNCODE) -DSRCDIR=$(top_srcdir) -DVERSION=${gitrev} $(shell test "$(host_cpu)" = x86_64 && echo "-DW64") -NOCD $(top_srcdir)/nsis/tesseract.nsi

endif
