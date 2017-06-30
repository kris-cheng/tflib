#
# Makefile for nac
# 2014/07/09 by kris
# ============================
#    nac for unionpay pos
# #

SHELL=/bin/sh

CC=gcc
MV=mv

INCLS=-I$(HOME)/inc -I$(HOME)/include -I./
LDFLAGS=-L$(HOME)/lib

SYSTEM := $(shell uname -s)

ifeq ($(SYSTEM),Linux)
    CFLAGS=$(INCLS) -s -O2 -DUNIX_SVR4 -DUNIX -DUNIXMODE -DMULTITHREAD -D_REENTRANT -DOS_LINUX -std=c99 
    LIBS=-ltf -lstdc++ -lpthread -lz -lm
else
    ifeq ($(SYSTEM),SunOS)
        CFLAGS=$(INCLS) -s -O2 -DUNIX_SVR4 -DUNIX -DUNIXMODE -DMULTITHREAD -D_REENTRANT -DOS_SUNOS -std=c99 
        LIBS=-ltf -lstdc++ -lpthread -lz -lsocket -lnsl -lresolv -lm
    else
        CFLAGS=$(INCLS) -s -O2 -DUNIX_SVR4 -DUNIX -DUNIXMODE -DMULTITHREAD -D_REENTRANT -DOS_ELSE -std=c99 
        LIBS=-ltf -lstdc++ -lpthread -lz -lm
    endif
endif

srcdir = .
bindir = $(HOME)/bin
objdir = .

objs =  main.o config.o console.o
progs = nac-service

all:$(progs) move clean

$(progs): $(objs)
	@echo "Linking $(progs) on $(SYSTEM)..."
	@${CC} $(LDFLAGS) ${CFLAGS} -o $@ $? ${LIBS}

$(objdir)/main.o: ${srcdir}/main.c
	@echo "Compiling $@ ...... "
	@${CC} ${CFLAGS} -c -o $@ $?

$(objdir)/config.o: ${srcdir}/config.c
	@echo "Compiling $@ ...... "
	@${CC} ${CFLAGS} -c -o $@ $?

$(objdir)/console.o: ${srcdir}/console.c
	@echo "Compiling $@ ...... "
	@${CC} ${CFLAGS} -c -o $@ $?

clean:
	@rm -f $(objs) $(progs)
move:
	@$(MV) $(progs) $(bindir)

rebuild: clean all
