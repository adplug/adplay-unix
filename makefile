# Makefile for AdPlay/Linux, by Simon Peter (dn.tlp@gmx.net)

.PHONY: clean distclean

ADPLUG		= ../adplug
OPTIONS		= -Wall
DEFINES		= -Dstricmp=strcasecmp
CPP		= g++
CC		= gcc

adplay: adplay.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG) -I$(ADPLUG)/players $(ADPLUG)/adplug.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG) $(ADPLUG)/emuopl.cpp
	$(CC) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG) $(ADPLUG)/fmopl.c
	$(CPP) -o adplay $(OPTIONS) $(DEFINES) -I$(ADPLUG) adplay.cpp adplug.o emuopl.o fmopl.o protrack.o a2m.o amd.o d00.o dfm.o hsc.o hsp.o imf.o ksm.o mid.o mkj.o mtk.o rad.o raw.o s3m.o sa2.o sng.o u6m.o lds.o

players:
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/protrack.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/a2m.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/amd.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/d00.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/dfm.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/hsc.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/hsp.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/imf.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/ksm.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/mid.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/mkj.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/mtk.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/rad.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/raw.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/s3m.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/sa2.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/sng.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/u6m.cpp
	$(CPP) -c $(OPTIONS) $(DEFINES) -I$(ADPLUG)/players $(ADPLUG)/players/lds.cpp

clean:
	rm -f *.o

distclean: clean
	rm -f adplay
