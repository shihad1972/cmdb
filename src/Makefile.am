bin_PROGRAMS = cpc ckc mkvm
AM_CPPFLAGS = -I$(top_srcdir)/include/

combo_SOURCES = checks.c checks.h

cpc_SOURCES = cpc.c $(combo_SOURCES)

ckc_SOURCES = ckc.c $(combo_SOURCES)

mkvm_SOURCES = mkvm.c virtual.c

AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_LDFLAGS = -L$(top_builddir)/lib -lailsasql -lailsacmdb

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
