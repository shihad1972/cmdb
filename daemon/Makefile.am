sbin_PROGRAMS = cmdbd
AM_CPPFLAGS = -I$(top_srcdir)/include/

cmdbd_SOURCES = cmdbd.c cmdbd.h
cmdbd_LDADD = ../lib/libalisacmdb.la

AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_CPPFLAGS = -I$(top_srcdir)/src/ -I$(top_srcdir)/include/
AM_LDFLAGS =

if HAVE_LIBGCRYPT
AM_CPPFLAGS += $(LIBGCRYPT_CFLAGS)
AM_LDFLAGS += $(LIBGCRYPT_LIBS)
endif

