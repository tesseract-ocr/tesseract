datadir = @datadir@/tessdata
data_DATA = confsets \
            fra.DangAmbigs fra.freq-dawg fra.inttemp fra.normproto \
            fra.pffmtable fra.user-words fra.word-dawg fra.unicharset \
            ita.DangAmbigs ita.freq-dawg ita.inttemp ita.normproto \
            ita.pffmtable ita.user-words ita.word-dawg ita.unicharset \
            deu.DangAmbigs deu.freq-dawg deu.inttemp deu.normproto \
            deu.pffmtable deu.user-words deu.word-dawg deu.unicharset \
            spa.DangAmbigs spa.freq-dawg spa.inttemp spa.normproto \
            spa.pffmtable spa.user-words spa.word-dawg spa.unicharset \
            nld.DangAmbigs nld.freq-dawg nld.inttemp nld.normproto \
            nld.pffmtable nld.user-words nld.word-dawg nld.unicharset \
            eng.DangAmbigs eng.freq-dawg eng.inttemp eng.normproto \
            eng.pffmtable eng.user-words eng.word-dawg eng.unicharset

SUBDIRS = configs tessconfigs

EXTRA_DIST = confsets makedummies

eng.DangAmbigs eng.freq-dawg eng.inttemp eng.normproto eng.pffmtable eng.user-words eng.word-dawg eng.unicharset : makedummies
	./makedummies eng
fra.DangAmbigs fra.freq-dawg fra.inttemp fra.normproto fra.pffmtable fra.user-words fra.word-dawg fra.unicharset : makedummies
	./makedummies fra
ita.DangAmbigs ita.freq-dawg ita.inttemp ita.normproto ita.pffmtable ita.user-words ita.word-dawg ita.unicharset : makedummies
	./makedummies ita
deu.DangAmbigs deu.freq-dawg deu.inttemp deu.normproto deu.pffmtable deu.user-words deu.word-dawg deu.unicharset : makedummies
	./makedummies deu
spa.DangAmbigs spa.freq-dawg spa.inttemp spa.normproto spa.pffmtable spa.user-words spa.word-dawg spa.unicharset : makedummies
	./makedummies spa
nld.DangAmbigs nld.freq-dawg nld.inttemp nld.normproto nld.pffmtable nld.user-words nld.word-dawg nld.unicharset : makedummies
	./makedummies nld
