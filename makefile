# Makefile for AdPlay/Linux, (c) 2001 Simon Peter <dn.tlp@gmx.net>

CXX = gcc-3.0
INSTALL = install

CXXFLAGS = -Wall
LDFLAGS = -ladplug

SRCS = adplay.cpp makefile
AUX = README COPYING

distname = adplay-1.0
bindir = /usr/local/bin

all: adplay

clean:
	rm -f adplay

distclean: clean

install: all
	$(INSTALL) adplay $(bindir)

uninstall:
	rm $(bindir)/adplay

dist:
	-rm -rf $(distname)
	mkdir $(distname)
	cp $(SRCS) $(AUX) $(distname)
	tar cfj $(distname).tar.bz2 $(distname)
	rm -rf $(distname)

adplay: adplay.cpp
