# doc/Makefile.am

if ASCIIDOC

asciidoc=asciidoc -d manpage

man_MANS = \
  ambiguous_words.1 \
  classifier_tester.1 \
  cntraining.1  \
  combine_lang_model.1 \
  combine_tessdata.1  \
  dawg2wordlist.1 \
  lstmeval.1 \
  lstmtraining.1 \
  merge_unicharsets.1 \
  mftraining.1  \
  set_unicharset_properties.1 \
  shapeclustering.1 \
  tesseract.1 \
  text2image.1 \
  unicharambigs.5 \
  unicharset.5 \
  unicharset_extractor.1  \
  wordlist2dawg.1 
  
  EXTRA_DIST = $(man_MANS) Doxyfile

%: %.asc
	$(asciidoc) -o $@ $<

MAINTAINERCLEANFILES = $(man_MANS) Doxyfile

endif
