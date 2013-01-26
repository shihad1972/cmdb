bin_PROGRAMS = dnsa cmdb cbc
dnsa_SOURCES = dnsa.c dnsacom.c rev_zone.c fwd_zone.c errors.c dzf.c drzf.c dnsa_mysql.c checks.c base_mysql.c
cmdb_SOURCES = cmdb.c cmdbcom.c cmdb_mysql.c errors.c servers.c customers.c checks.c base_mysql.c
cbc_SOURCES = cbc.c cbc_mysql.c errors.c checks.c cbccom.c build_info.c build_add.c build_display.c base_mysql.c build_data_struct.c
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
