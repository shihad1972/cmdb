AM_CFLAGS = -ansi -std=c99 -D_POSIX_C_SOURCE=200809L
AM_CPPFLAGS = -I$(top_srcdir)/include/ -I$(top_srcdir)/src/
AM_LDFLAGS =
lib_LTLIBRARIES = libailsacmdb.la libailsasql.la
libailsacmdb_la_SOURCES = ailsacmdb.c parse_config.c logging.c networking.c fileio.c data.c
libailsasql_la_SOURCES = base_sql.c cmdb_base_sql.c dnsa_base_sql.c cbc_base_sql.c
include_HEADERS = $(top_srcdir)/include/ailsacmdb.h

if HAVE_MYSQL
AM_CFLAGS += $(MYSQL_CFLAGS)
AM_LDFLAGS += $(MYSQL_LDFLAGS)
endif

if HAVE_PCRE
AM_CFLAGS += $(PCRE_CFLAGS)
AM_LDFLAGS += $(PCRE_LIBS)
endif

if HAVE_SQLITE3
AM_CFLAGS += $(SQLITE3_CFLAGS)
AM_LDFLAGS += $(SQLITE3_LDFLAGS)
endif

