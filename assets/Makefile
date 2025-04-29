
MAKEFLAGS+= -j 1
include ../header.mk

SVGS=$(wildcard *.svg)
PNGS=$(SVGS:.svg=.png)
ICOS=$(SVGS:.svg=.ico)

all : ${PNGS} ${ICOS}

clean :
	rm -f ${PNGS} ${ICOS}

include ../rules.mk

