sbin_PROGRAMS = cmdbd cmdbc

shar_SOURCES = client_info.c client_info.h conversion.h fileio.h

cmdbd_SOURCES = cmdbd.c $(shar_SOURCES)
cmdbd_LDADD = ../lib/libailsacmdb.la

cmdbc_SOURCES = cmdbc.c $(shar_SOURCES)
cmdbc_LDADD = ../lib/libailsacmdb.la

AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_CPPFLAGS = -I$(top_srcdir)/src/ -I$(top_srcdir)/include/
AM_LDFLAGS =

if HAVE_LIBGCRYPT
AM_CPPFLAGS += $(LIBGCRYPT_CFLAGS)
AM_LDFLAGS += $(LIBGCRYPT_LIBS)
endif

if HAVE_LIBUUID
AM_CPPFLAGS += $(UUID_CPPFLAGS)
AM_LDFLAGS += $(UUID_LDFLAGS)
LIBS += $(UUID_LIBS)
endif
