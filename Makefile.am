bin_PROGRAMS = adplay

adplay_SOURCES = adplay.cc getopt.c getopt1.c getopt.h output.cc output.h \
	config.h players.h

EXTRA_adplay_SOURCES = oss.cc oss.h null.h disk.cc disk.h

adplay_LDADD = @drivers@
adplay_DEPENDENCIES = @drivers@

man_MANS = adplay.1

EXTRA_DIST = $(man_MANS)

AUTOMAKE_OPTIONS = dist-bzip2
