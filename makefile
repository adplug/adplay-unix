# Makefile for AdPlay/Linux, (c) 2001 Simon Peter <dn.tlp@gmx.net>

CXX		= c++
CXXFLAGS	= -Wall
CPPFLAGS	= 
LDFLAGS		= -lpthread -ladplug

SRCS		= adplay.cpp makefile
AUX		= README COPYING

distname	= adplay-1.0

all: adplay

clean:
	rm -f adplay

distclean: clean

dist:
	-rm -rf $(distname)
	mkdir $(distname)
	cp $(SRCS) $(AUX) $(distname)
	tar cfz $(distname).tar.gz $(distname)
	rm -rf $(distname)

adplay: adplay.cpp
