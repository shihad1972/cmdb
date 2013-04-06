bin_PROGRAMS = dnsa cmdb cbc cbcdomain cbcos cbcvarient cbcpack

dnsa_SOURCES =	dnsa.c dnsacom.c errors.c checks.c base_sql.c dnsa_base_sql.c\
		zones.c\
		cmdb_dnsa.h dnsa_base_sql.h cmdb.h checks.h\
		base_sql.h
cmdb_SOURCES =	cmdb.c cmdbcom.c errors.c customers.c checks.c servers.c\
		cmdb_base_sql.c base_sql.c\
		cmdb_cmdb.h cmdb_base_sql.h cmdb.h checks.h\
		base_sql.h
cbc_SOURCES =	cbc.c cbccom.c errors.c checks.c base_sql.c cbc_base_sql.c\
		cbc_data.c build.c\
		cmdb_cbc.h cbc_base_sql.h cmdb.h checks.h \
		cbc_data.h build.h base_sql.h
cbcdomain_SOURCES = \
		cbcdomain.c errors.c checks.c base_sql.c cbc_data.c\
		cbc_base_sql.c builddomain.c cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h builddomain.h\
		base_sql.h cbcdomain.h
cbcos_SOURCES = \
		cbcos.c errors.c checks.c base_sql.c cbc_data.c cbc_base_sql.c\
		cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h cbcos.h
cbcvarient_SOURCES = \
		cbcvarient.c errors.c checks.c base_sql.c cbc_data.c\
		cbc_base_sql.c cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h\
		cbcvarient.h
cbcpack_SOURCES = \
		cbcpack.c errors.c checks.c base_sql.c cbc_data.c cbc_base_sql.c\
		cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h cbcpack.h
cbcpart_SOURCES = \
		cbcpart.c errors.c checks.c base_sql.c cbc_data.c cbc_base_sql.c\
		cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h cbcpart.h

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
