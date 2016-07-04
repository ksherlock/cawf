# 	Makefile for cawf

#	Define UNIX for vanilla Unix systems -- e.g., older DYNIX.
#
#	Define UNIX and USG for System V, BSD 4.3 and for SunOS.
#
#	USG may also be needed if the required string function prototypes --
#	e.g., for strrchr() -- are in <string.h> rather than <strings.h>.
#
#DEFS = -DUNIX -DUSG
#
#	Define STDLIB for systems that have <stdlib.h> -- e.g., AIX and
#	SunOS.
#
#	Redefine CAWFLIB by adding -DCAWFLIB=\"...\" to DEFS.
#
#DEFS = -DUNIX -DUSG -DCAWFLIB=\"/usr/local/lib/cawf\"
#
#	Customize the install rule.
#
#	-ansi and -pedantic are ANSI compliance options for the gcc compiler.
#	Remove them if your compiler objects.
#
#	If you're using xlc 2.1 on AIX 3.2 for the RISC/SYSTEM 6000, you
#	must delete the definition of __STR__ (two leading and two trailing
#	underscore characters), because the xlc 2.1 compiler incorrectly
#	inlines string functions when compiling pass3.c.
#
#DEFS = -DUNIX -DSTDLIB -U__STR__
#
#	Unix systems that have a <malloc.h> need MALLOCH defined, unless
#	they also have a <stdlib.h> that provides a function prototype for
#	malloc() and its relatives (most do).
#
#DEFS = -DUNIX -DMALLOCH

DEFS = -DUNIX -DSTDLIB -D_NEXT_SOURCE -ansi -pedantic

CFLAGS = -O ${DEFS}

HDR = ansi.h cawf.h cawflib.h proto.h regexp.h regmagic.h

SRC = cawf.c device.c error.c expand.c expr.c getopt.c macsup.c nreq.c \
      output.c pass2.c pass3.c  regerror.c regexp.c store.c string.c

OBJ = cawf.o device.o error.o expand.o expr.o getopt.o macsup.o nreq.o \
      output.o pass2.o pass3.o  regerror.o regexp.o store.o string.o

all:	bsfilt cawf

bsfilt: bsfilt.c
	${CC} ${CFLAGS} bsfilt.c -o bsfilt

cawf:	${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o cawf

clean:
	rm -f *.o a.out core *errs bsfilt cawf

${OBJ}:	${HDR}

install: bsfilt cawf
