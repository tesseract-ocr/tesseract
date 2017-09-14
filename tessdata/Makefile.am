datadir = @datadir@/tessdata

data_DATA = pdf.ttf
EXTRA_DIST = $(data_DATA)

SUBDIRS = configs tessconfigs

engosddata = eng.traineddata osd.traineddata

engosddata-install:
	wget  -nc https://github.com/tesseract-ocr/tessdata/raw/master/best/eng.traineddata
	wget  -nc https://github.com/tesseract-ocr/tessdata/raw/master/best/osd.traineddata
	cp   $(engosddata) $(DESTDIR)$(datadir)
  
engosddata-remove: 
	rm -f $(engosddata)

engosddata-uninstall:
	cd $(DESTDIR)$(datadir); \
	rm -f $(engosddata)
