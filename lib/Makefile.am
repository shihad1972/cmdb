AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_CPPFLAGS = -I$(top_srcdir)/include/ -I$(top_srcdir)/src/
AM_LDFLAGS =
lib_LTLIBRARIES = libailsacmdb.la libailsasql.la
libailsacmdb_la_SOURCES = ailsacmdb.c parse_config.c logging.c \
			regexp.c fileio.c data.c dnsa_data.c cmdb_data.c cbc_data.c \
			errors.c list.c hash.c config.c uuid.c dnsa_net.c cbcnet.c
libailsasql_la_SOURCES = sql.c base_sql.c cmdb_base_sql.c dnsa_base_sql.c \
			cbc_base_sql.c
include_HEADERS = $(top_srcdir)/include/ailsacmdb.h

if HAVE_MYSQL
AM_CFLAGS += $(MYSQL_CFLAGS)
AM_LDFLAGS += $(MYSQL_LDFLAGS)
endif

if HAVE_LIBUUID
AM_CPPFLAGS += $(UUID_CPPFLAGS)
AM_LDFLAGS += $(UUID_LDFLAGS)
LIBS += $(UUID_LIBS)
endif

if HAVE_SQLITE3
AM_CFLAGS += $(SQLITE3_CFLAGS)
AM_LDFLAGS += $(SQLITE3_LDFLAGS)
endif
