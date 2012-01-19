#-----------------------------------------------------------------------------
#	Makefile
#	EECS 337 Compilers Fall 2010
#	by caseid
#
# REVISION HISTORY
#
#-----------------------------------------------------------------------------
.KEEP_STATE:
SHELL=/bin/bash

#
#	define version of c compiler, linker and lex
#
CC=		g++
LINK=	g++
LEX=	flex
#
#	define yacc lex and compiler flags
#
YFLAGS	= -dv
LFLAGS	=
CFLAGS	= -g

SRC	= ciy.y ci.l main.c strmap.c variable.cpp evaltree.cpp type.cpp
OBJ	= ciy.o ci.o main.o strmap.o variable.o evaltree.o type.o

all :	$(OBJ)
	$(LINK) $(CFLAGS) $(OBJ) -o cinterp

ci.o	: y.tab.h

clean	:
	rm -f ciy.c ci.c y.tab.h y.output *.o
