bin_PROGRAMS = adplay

adplay_SOURCES = adplay.cc output.cc output.h players.h defines.h

if NEED_GETOPT
adplay_SOURCES += getopt.c getopt1.c getopt_compat.h
endif

EXTRA_adplay_SOURCES = oss.cc oss.h null.h disk.cc disk.h esound.cc esound.h \
	qsa.cc qsa.h sdl.cc sdl_driver.h alsa.cc alsa.h ao.cc ao.h getopt.c \
	getopt1.c getopt_compat.h diskraw.h

adplay_LDADD = $(drivers) $(adplug_LIBS) @ESD_LIBS@ @QSA_LIBS@ @SDL_LIBS@ \
	@ALSA_LIBS@ @AO_LIBS@
adplay_DEPENDENCIES = $(drivers)

adplug_data_dir = $(sharedstatedir)/adplug

AM_CPPFLAGS = $(adplug_CFLAGS) @ESD_CFLAGS@ @SDL_CFLAGS@ @ALSA_CFLAGS@ \
	-DADPLUG_DATA_DIR=\"$(adplug_data_dir)\"
