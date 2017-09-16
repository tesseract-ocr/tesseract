datadir = @datadir@/tesseract-ocr/4.00/tessdata

data_DATA = pdf.ttf
EXTRA_DIST = $(data_DATA)

SUBDIRS = configs tessconfigs

engosddata = eng.traineddata osd.traineddata

engosddata-best-install:
	wget  -nc https://github.com/tesseract-ocr/tessdata_best/raw/master/eng.traineddata
	wget  -nc https://github.com/tesseract-ocr/tessdata_best/raw/master/osd.traineddata
	cp   $(engosddata) $(DESTDIR)$(datadir)
	
engosddata-fast-install:
	wget  -nc https://github.com/tesseract-ocr/tessdata_fast/raw/master/eng.traineddata
	wget  -nc https://github.com/tesseract-ocr/tessdata_fast/raw/master/osd.traineddata
	cp   $(engosddata) $(DESTDIR)$(datadir)

engosddata-legacy-install:
	wget  -nc https://github.com/tesseract-ocr/tessdata/raw/master/eng.traineddata
	wget  -nc https://github.com/tesseract-ocr/tessdata/raw/master/osd.traineddata
	cp   $(engosddata) $(DESTDIR)$(datadir)
  
engosddata-remove: 
	rm -f $(engosddata)

engosddata-uninstall:
	cd $(DESTDIR)$(datadir); \
	rm -f $(engosddata)
