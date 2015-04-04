sbin_PROGRAMS = cmdbd

cmdbd_SOURCES = cmdbd.c cmdbd.h
cmdbd_LDADD = ../lib/libalisacmdb.la

AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_CPPFLAGS = -I../src/ -I../include/
AM_LDFLAGS =

if HAVE_LIBGCRYPT
AM_CPPFLAGS += $(LIBGCRYPT_CFLAGS)
AM_LDFLAGS += $(LIBGCRYPT_LIBS)
endif

