AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_CPPFLAGS = -I$(top_srcdir)/include/ -I$(top_srcdir)/src/
AM_LDFLAGS =
LDADD =
lib_LTLIBRARIES = libailsacmdb.la libailsasql.la
libailsacmdb_la_SOURCES = ailsacmdb.c logging.c regexp.c data.c \
			errors.c list.c hash.c config.c uuid.c
libailsasql_la_SOURCES = queries.c sql.c helper.c sql_data.c dnsa_net.c
include_HEADERS = $(top_srcdir)/include/ailsacmdb.h $(top_srcdir)/include/ailsasql.h

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

if HAVE_OPENSSL
AM_CPPFLAGS += $(OPENSSL_INCLUDES)
AM_LDFLAGS += $(OPENSSL_LDFLAGS)
LIBS += $(OPENSSL_LIBS)
endif

if HAVE_LIBGCRYPT
AM_CPPFLAGS += $(LIBGCRYPT_CFLAGS)
LDADD += $(LIBGCRYPT_LIBS)
endif
