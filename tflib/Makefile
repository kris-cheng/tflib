#
## Makefile for TFLib
# 1.0.0
# ===============
#     Basic share library,most for OO project
#     2013-08-28
#     by Kris
# #
SHELL=/bin/sh

CC=gcc
MV=mv
AR=ar

LIBS = -lbz2 -lz -lrt -lpthread -lm -lc -ldl
INCLS=-I.
ARFLAGS=-cvq

TF_OS_BITS := $(shell getconf LONG_BIT)
ifeq ($(TF_OS_BITS), 32)
    TF_OS_BITS=32
else
    TF_OS_BITS=64
endif

HOSTARCH := $(shell uname -m | sed -e s/i86pc/X86/ -e s/x86_64/X86/ -e s/sun4v/SPARC/ -e s/sun4u/SPARC/)
ifeq ($(HOSTARCH),SPARC)
    TF_BIG_ENDIAN=1
endif

ifeq ($(TF_BIG_ENDIAN),1)
    FLAGS=-Wall -fPIC -std=c99 -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D__EXTENSIONS__ -DTF_OS_BITS=$(TF_OS_BITS) -DTF_BIG_ENDIAN 
else
    FLAGS=-Wall -fPIC -std=c99 -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D__EXTENSIONS__ -DTF_OS_BITS=$(TF_OS_BITS)
endif

SYSTEM := $(shell uname -s)

ifeq ($(SYSTEM),Linux)
    CFLAGS=$(INCLS) $(FLAGS) -O2 -DOS_LINUX
    LDFLAGS=-shared -L..
else
    ifeq ($(SYSTEM),SunOS)
        CFLAGS=$(INCLS) $(FLAGS) -s -O2 -DOS_SUNOS
        LDFLAGS=-G  -lxnet -lnsl -lsocket -L..
    else
        CFLAGS=$(INCLS) $(FLAGS) -s -O2 -DOS_ELSE
        LDFLAGS=-G -L..
    endif
endif

.SUFFIXES: $(SUFFIXES) .c .o

srcdir = .
bindir = $(HOME)/lib
incdir = $(HOME)/inc

ae_incs = ae/ae.h
ae_dir  = $(incdir)/ae

expert_incs = expert/d3des.h expert/md5.h expert/base64.h
expert_dir  = $(incdir)/expert

srcs = tf_ipc.c tf_log.c tf_conf_file.c tf_string.c tf_malloc.c tf_file.c tf_socket.c tf_thread.c tf_system.c tf_util.c tf_list.c
objs = tf_ipc.o tf_log.o tf_conf_file.o tf_string.o tf_malloc.o tf_file.o tf_socket.o tf_thread.o tf_system.o tf_util.o tf_list.o ae.o d3des.o base64.o md5.o
incs = tflib.h tf_config.h tf_config_linux.h tf_ipc.h tf_log.h tf_conf_file.h tf_string.h tf_malloc.h tf_file.h tf_socket.h tf_thread.h tf_system.h tf_util.h tf_list.h

libstatic = libtf.a
libshared = libtf.so

.c.o:
	@echo "Compiling $< ..."
	@$(CC) $(CFLAGS) -o $@ -c $<

ifeq ($(SYSTEM),SunOS)
all:$(libshared) move clean
else
all:$(libstatic) $(libshared) move clean
endif

ae.o:ae/ae.c
	@$(CC) $(CFLAGS) -c -o $@ $?

d3des.o:expert/d3des.c
	@$(CC) $(CFLAGS) -c -o $@ $?

md5.o:expert/md5.c
	@$(CC) $(CFLAGS) -c -o $@ $?

base64.o:expert/base64.c
	@$(CC) $(CFLAGS) -c -o $@ $?

clean:
	@rm -f $(objs) $(libstatic) $(libshared)
move:
ifneq ($(SYSTEM),SunOS)
	@$(MV) $(libstatic) $(bindir)
endif
	@$(MV) $(libshared) $(bindir)

rebuild: clean all

$(libstatic) : $(objs)
	@echo "Linking $(libstatic) on $(SYSTEM)..."
	@$(AR) $(ARFLAGS) $@ $(objs)

$(libshared): $(objs)
	@echo "Linking $(libshared) on $(SYSTEM)..."
	@$(CC) $(LDFLAGS) -o $@ $(objs) $(LIBS)

install:
	@mkdir -p $(incdir)
	@mkdir -p $(ae_dir)
	@mkdir -p $(expert_dir)
	@cp -Rf $(incs) $(incdir)
	@cp -Rf $(ae_incs) $(ae_dir)
	@cp -Rf $(expert_incs) $(expert_dir)
