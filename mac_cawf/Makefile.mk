# 	Macintosh Makefile for cawf
#
#	by Matthias Ulrich Neeracher <neeri@iis.ee.ethz.ch>

BINDIR		=	"{{MPW}}LocalTools:"
LIBDIR		=	"{{MPW}}Interfaces:cawf"

COpt			=	-d UNIX -d USG -d STDLIB -d macintosh -w off -sym on
C68K			=	MWC68K ${COpt} -mc68020 -model far -mbg on
CPPC			=	MWCPPC ${COpt} -traceback
ROptions 	= 	-i :
Lib68K		=	MWLink68K -xm library -sym on
LibPPC		=	MWLinkPPC -xm library -sym on
LOpt			= 	-sym on -w -xm mpwtool
Link68K		=	MWLink68K ${LOpt} -model far
LinkPPC		=	MWLinkPPC ${LOpt} 

LibFiles68K	=			"{{MW68KLibraries}}GUSIDispatch.Lib.68K"				\
							"{{MW68KLibraries}}GUSIMPW.Lib.68K"						\
							"{{MW68KLibraries}}MPW_Runtime.o.lib"					\
							"{{MW68KLibraries}}new MPW ANSI (4i/8d) C.68K.Lib"	\
							"{{MW68KLibraries}}MathLib68K (4i/8d).Lib"			\
							"{{MW68KLibraries}}MacOS.Lib"								\
							"{{MW68KLibraries}}GUSI.Lib.68K"							\
							"{{MW68KLibraries}}CPlusPlus.lib"						\
							"{{MW68KLibraries}}ToolLibs.o"							\
							"{{MW68KLibraries}}PLStringFuncs.glue.lib"

LibFilesPPC	=			"{{MWPPCLibraries}}GUSIMPW.Lib.PPC"						\
							"{{MWPPCLibraries}}GUSI.Lib.PPC"							\
							"{{MWPPCLibraries}}MWStdCRuntime.Lib"					\
							"{{MWPPCLibraries}}InterfaceLib"							\
							"{{MWPPCLibraries}}ANSI (NL) C++.PPC.Lib"				\
							"{{MWPPCLibraries}}ANSI (NL) C.PPC.Lib"				\
							"{{MWPPCLibraries}}StdCLib"								\
							"{{MWPPCLibraries}}MathLib"								\
							"{{MWPPCLibraries}}PPCToolLibs.o"

HDR = ansi.h cawf.h cawflib.h proto.h regexp.h regmagic.h

SRC = cawf.c device.c error.c expand.c expr.c getopt.c macsup.c nreq.c \
      output.c pass2.c pass3.c  regerror.c regexp.c store.c string.c bsfilt.c	\
		strcasecmp.c macish.c

OBJ68K		=	$(SRC:s/.c/.c.68K.o/)
OBJPPC		=	$(SRC:s/.c/.c.PPC.o/)

.SOURCE.o	:	":Obj:"

%.c.68K.o		:	%.c
	Set Echo 0
	Set Src68K "{{Src68K}} $<"
%.c.PPC.o		:	%.c
	Set Echo 0
	Set SrcPPC "{{SrcPPC}} $<"

all:	bsfilt cawf

bsfilt: bsfilt.68K bsfilt.PPC
	Duplicate -y bsfilt.PPC bsfilt
	Echo 'Include "bsfilt.68K" '¶''CODE'¶'';Include "bsfilt.68K" '¶''DATA'¶'';' ¶
		| Rez -a -c 'MPS ' -t MPST -o bsfilt
bsfilt.68K:	Objects68K
	${Link68K} -o bsfilt.68K :Obj:bsfilt.c.68K.o :Obj:macish.c.68K.o ${LibFiles68K}
bsfilt.PPC:	ObjectsPPC
	${LinkPPC} -o bsfilt.PPC :Obj:bsfilt.c.PPC.o :Obj:macish.c.PPC.o ${LibFilesPPC}

cawf: cawf.68K cawf.PPC
	Duplicate -y cawf.PPC cawf
	Echo 'Include "cawf.68K" '¶''CODE'¶'';Include "cawf.68K" '¶''DATA'¶'';' ¶
		| Rez -a -c 'MPS ' -t MPST -o cawf
cawf.68K:	Objects68K cawf.r
	${Link68K} -o cawf.68K ${LibFiles68K} :Obj:{${OBJ68K}}
	Rez -a -t MPST -c 'MPS ' cawf.r -o cawf.68K
cawf.PPC:	ObjectsPPC cawf.r
	${LinkPPC} -o cawf.PPC :Obj:{${OBJPPC}} ${LibFilesPPC} 
	Rez -a -t MPST -c 'MPS ' cawf.r -o cawf.PPC

clean:
#	rm -f *.o a.out core *errs bsfilt cawf

install: all
	Begin
		Backup -a -to $(BINDIR) bsfilt cawf 
		Backup -a -c -to $(LIBDIR) Å.mac Å.dev Å.cf common
	End>"{{TempFolder}}"cawf.run
	"{{TempFolder}}"cawf.run
	

Objects68K	:	${OBJ68K}
	Set Echo 1
	If "{{Src68K}}" != "" 
		${C68K} -t -fatext {{Src68K}} -o :Obj:
	End
	echo > Objects68K
	
ObjectsPPC	:	${OBJPPC}	
	Set Echo 1
	If "{{SrcPPC}}" != "" 
		${CPPC} -t -fatext {{SrcPPC}} -o :Obj:
	End
	echo > ObjectsPPC
