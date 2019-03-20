AM_CPPFLAGS = -I$(top_srcdir)/include/
AM_LDFLAGS =

lib_LTLIBRARIES = libailsacmdb.la
libailsacmdb_la_SOURCES = ailsacmdb.c logging.c data.c uuid.c config.c
include_HEADERS = $(top_srcdir)/include/ailsacmdb.h

if HAVE_LIBUUID
AM_CPPFLAGS += $(UUID_CPPFLAGS)
AM_LDFLAGS += $(UUID_LDFLAGS)
LIBS += $(UUID_LIBS)
endif
