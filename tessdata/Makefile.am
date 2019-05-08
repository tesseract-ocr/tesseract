datadir = @datadir@/tessdata

data_DATA = pdf.ttf
EXTRA_DIST = $(data_DATA)

SUBDIRS = configs tessconfigs

langdata =

uninstall-local:
	cd $(DESTDIR)$(datadir); \
	rm -f $(langdata)
