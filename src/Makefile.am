bin_PROGRAMS = adplay

adplay_SOURCES = adplay.cc getopt.c getopt1.c getopt.h output.cc output.h \
	players.h

EXTRA_adplay_SOURCES = oss.cc oss.h null.h disk.cc disk.h esound.cc esound.h

#adplay_LDFLAGS = -all-static
adplay_LDADD = @drivers@ @ESD_LIBS@
adplay_DEPENDENCIES = @drivers@

AM_CPPFLAGS = @ESD_CFLAGS@
