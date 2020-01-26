AM_CFLAGS = -ansi -std=c99 -D_POSIX_C_SOURCE=200809L
AM_CPPFLAGS = -I$(top_srcdir)/include/ -I$(top_srcdir)/src/
AM_LDFLAGS =
lib_LTLIBRARIES = libailsacmdb.la
libailsacmdb_la_SOURCES = ailsacmdb.c parse_config.c logging.c networking.c \
			fileio.c data.c dnsa_data.c cmdb_data.c cbc_data.c \
			errors.c list.c
include_HEADERS = $(top_srcdir)/include/ailsacmdb.h

if HAVE_MYSQL
AM_CFLAGS += $(MYSQL_CFLAGS)
AM_LDFLAGS += $(MYSQL_LDFLAGS)
endif

if HAVE_LIBODBC
lib_LTLIBRARIES += libailsasql.la
libailsasql_la_SOURCES = base_sql.c sql.c
endif

if HAVE_LIBUUID
AM_CPPFLAGS += $(UUID_CPPFLAGS)
AM_LDFLAGS += $(UUID_LDFLAGS)
LIBS += $(UUID_LIBS)
endif

if HAVE_PCRE
AM_CFLAGS += $(PCRE_CFLAGS)
AM_LDFLAGS += $(PCRE_LIBS)
endif

if HAVE_SQLITE3
AM_CFLAGS += $(SQLITE3_CFLAGS)
AM_LDFLAGS += $(SQLITE3_LDFLAGS)
endif
