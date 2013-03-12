bin_PROGRAMS = dnsa cmdb cbc

dnsa_SOURCES =	dnsa.c dnsacom.c errors.c checks.c base_sql.c dnsa_base_sql.c\
		zones.c\
		mysqlfunc.h cmdb_dnsa.h dnsa_base_sql.h cmdb.h checks.h\
		base_sql.h
cmdb_SOURCES =	cmdb.c cmdbcom.c errors.c customers.c checks.c servers.c\
		cmdb_base_sql.c base_sql.c\
		mysqlfunc.h cmdb_cmdb.h cmdb_base_sql.h cmdb.h checks.h\
		base_sql.h
cbc_SOURCES =	cbc.c cbccom.c errors.c checks.c base_sql.c cbc_base_sql.c\
		cbc_data.c build.c\
		mysqlfunc.h cmdb_cbc.h cbc_base_sql.h cmdb.h checks.h \
		cbc_data.h build.h base_sql.h

AM_CFLAGS = -W -Wall -Wconversion -Wshadow -Wcast-qual -Wwrite-strings
AM_LDFLAGS =

if HAVE_MYSQL
AM_CFLAGS += $(MYSQL_CFLAGS)
AM_LDFLAGS += $(MYSQL_LDFLAGS)
LIBS += -lmysqlclient
endif

if HAVE_PCRE
AM_CFLAGS += $(PCRE_CFLAGS)
endif

if HAVE_SQLITE3
AM_CFLAGS += $(SQLITE3_CFLAGS)
LIBS += -lsqlite3
endif
