bin_PROGRAMS = adplay

adplay_SOURCES = adplay.cc getopt.c getopt1.c getopt.h output.cc output.h \
	players.h

EXTRA_adplay_SOURCES = oss.cc oss.h null.h disk.cc disk.h esound.cc esound.h

adplay_LDADD = @drivers@ @ESD_LIBS@
adplay_DEPENDENCIES = @drivers@

adplug_data_dir = $(datadir)/adplug

AM_CPPFLAGS = @ESD_CFLAGS@ -DADPLUG_DATA_DIR=\"$(adplug_data_dir)\"
