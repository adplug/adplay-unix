bin_PROGRAMS = adplay

adplay_SOURCES = adplay.cc output.cc output.h players.h defines.h

EXTRA_adplay_SOURCES = oss.cc oss.h null.h disk.cc disk.h esound.cc esound.h \
	qsa.cc qsa.h sdl.cc sdl.h alsa.cc alsa.h ao.cc ao.h getopt.c \
	getopt1.c getopt.h

adplay_LDADD = $(drivers) $(adplug_LIBS) @ESD_LIBS@ @QSA_LIBS@ @SDL_LIBS@ \
	@ALSA_LIBS@ @AO_LIBS@ $(GETOPT_SOURCES)
adplay_DEPENDENCIES = $(drivers) $(GETOPT_SOURCES)

adplug_data_dir = $(sharedstatedir)/adplug

AM_CPPFLAGS = $(adplug_CFLAGS) @ESD_CFLAGS@ @SDL_CFLAGS@ @ALSA_CFLAGS@ \
	-DADPLUG_DATA_DIR=\"$(adplug_data_dir)\"
