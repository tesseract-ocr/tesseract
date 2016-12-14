datadir = @datadir@/tessdata

data_DATA = pdf.ttf
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
	deu-frak.traineddata aze.traineddata

uninstall-local:
	cd $(DESTDIR)$(datadir); \
	rm -f $(langdata)
