AUTOMAKE_OPTIONS = subdir-objects

all:

if MINGW

gitrev="$(shell git --git-dir=${abs_top_srcdir}/.git --work-tree=${abs_top_srcdir} describe --always --tags)"

.PHONY: winsetup

winsetup:
	makensis -DCROSSBUILD -DSHARED -DSRCDIR=$(top_srcdir) -DVERSION=${gitrev} $(shell test "$(host_cpu)" = x86_64 && echo "-DW64") -NOCD $(top_srcdir)/nsis/tesseract.nsi

endif
