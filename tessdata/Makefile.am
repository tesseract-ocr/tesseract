datadir = @datadir@/tessdata

data_DATA = pdf.ttf pdf.ttx
EXTRA_DIST = $(data_DATA)

SUBDIRS = configs tessconfigs

langdata = bul.traineddata mlt.traineddata chr.traineddata \
	slk.traineddata dan-frak.traineddata eng.traineddata \
	ces.traineddata afr.traineddata swa.traineddata \
	kan.traineddata bel.traineddata ind.traineddata \
	lit.traineddata nld.traineddata osd.traineddata \
	mkd.traineddata est.traineddata fra.traineddata \
	hin.traineddata lat_lid.traineddata nor.traineddata \
	por.traineddata ron.traineddata swe.traineddata \
	pol.traineddata ara.traineddata tel.traineddata \
	ell.traineddata mal.traineddata vie.traineddata \
	heb.traineddata deu.traineddata eus.traineddata \
	ita_old.traineddata rus.traineddata sqi.traineddata \
	spa.traineddata glg.traineddata slk-frak.traineddata \
	equ.traineddata hrv.traineddata frk.traineddata \
	cat.traineddata lav.traineddata ukr.traineddata \
	enm.traineddata dan.traineddata fin.traineddata \
	ben.traineddata srp.traineddata tha.traineddata \
	hun.traineddata tgl.traineddata frm.traineddata \
	slv.traineddata chi_sim.traineddata tam.traineddata \
	tur.traineddata epo.traineddata msa.traineddata \
	kor.traineddata isl.traineddata jpn.traineddata \
	chi_tra.traineddata ita.traineddata spa_old.traineddata \
	deu-frak.traineddata aze.traineddata fra.cube.lm \
	ita.tesseract_cube.nn eng.cube.word-freq rus.cube.lm \
	spa.cube.size fra.cube.nn fra.cube.params rus.cube.size \
	fra.cube.fold eng.cube.size ita.cube.bigrams \
	eng.tesseract_cube.nn rus.cube.params hin.cube.nn \
	spa.cube.params hin.cube.lm fra.cube.word-freq \
	spa.cube.word-freq ara.cube.nn ara.cube.word-freq \
	spa.cube.fold eng.cube.nn eng.cube.params eng.cube.lm \
	ita.cube.size hin.tesseract_cube.nn ita.cube.lm \
	fra.cube.bigrams ara.cube.fold spa.cube.bigrams \
	hin.cube.word-freq rus.cube.word-freq ita.cube.word-freq \
	fra.tesseract_cube.nn rus.cube.fold ara.cube.size \
	eng.cube.fold ita.cube.params ara.cube.params ita.cube.fold \
	ara.cube.bigrams hin.cube.params hin.cube.fold spa.cube.lm \
	ita.cube.nn fra.cube.size eng.cube.bigrams ara.cube.lm \
	rus.cube.nn spa.cube.nn hin.cube.bigrams

.PHONY: install-langs
install-langs:
	@if [ ! -d  $(DESTDIR)$(datadir) ]; then mkdir -p $(DESTDIR)$(datadir); fi;
	@if test "${LANGS}" != ""; then \
		for lang_code in ${LANGS}; do \
			echo "installing data for $$lang_code"; \
			$(INSTALL) -m 644 $(srcdir)/$$lang_code.* $(DESTDIR)$(datadir); \
		done; \
	else \
		for l in ./*.traineddata; do \
			filename=`basename $$l`; \
			lang_code=$${filename%.*}; \
			if test "$$lang_code" == "*"; then \
				echo "No lang present."; \
				break; \
			fi; \
			echo "installing data for $$lang_code"; \
			$(INSTALL) -m 644 $(srcdir)/$$lang_code.* $(DESTDIR)$(datadir); \
		done; \
	fi;

uninstall-local:
	cd $(DESTDIR)$(datadir); \
	rm -f $(langdata)
