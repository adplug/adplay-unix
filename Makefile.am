bin_PROGRAMS = adplay

adplay_SOURCES = adplay.cc system.h aclocal.m4 getopt.c getopt1.c \
		getopt.h
adplay_LDFLAGS = -L$(libdir) -ladplug

INCLUDES = -I$(includedir)
#CFLAGS = -Wall -g
#CXXFLAGS = -Wall -g

man_MANS = adplay.1

EXTRA_DIST = $(man_MANS)
