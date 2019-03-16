# doc/Makefile.am

if ASCIIDOC

man_MANS = \
  combine_lang_model.1 \
  combine_tessdata.1  \
  dawg2wordlist.1 \
  lstmeval.1 \
  lstmtraining.1 \
  merge_unicharsets.1 \
  set_unicharset_properties.1 \
  tesseract.1 \
  text2image.1 \
  unicharambigs.5 \
  unicharset_extractor.1 \
  wordlist2dawg.1

if !DISABLED_LEGACY_ENGINE
man_MANS += \
  ambiguous_words.1 \
  classifier_tester.1 \
  cntraining.1 \
  mftraining.1 \
  shapeclustering.1 \
  unicharset.5
endif

man_xslt = http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl

EXTRA_DIST = $(man_MANS) Doxyfile

.PHONY: html

html: ${man_MANS:%=%.html}
pdf: ${man_MANS:%=%.pdf}

SUFFIXES = .asc .html .pdf

.asc:
	-asciidoc -b docbook -d manpage -o - $< | \
	xsltproc --nonet $(man_xslt) -

.asc.html:
	asciidoc -b html5 -o $@ $<

.asc.pdf:
	asciidoc -b docbook -d manpage -o $*.dbk $<
	docbook2pdf $*.dbk

MAINTAINERCLEANFILES = $(man_MANS) Doxyfile

endif
