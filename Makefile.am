bin_PROGRAMS = adplay

adplay_SOURCES = adplay.cc getopt.c getopt1.c getopt.h oss.cc oss.h output.cc \
	output.h null.h disk.cc disk.h

adplay_LDADD = @libdir@/libadplug.la

AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall

man_MANS = adplay.1

EXTRA_DIST = $(man_MANS)
