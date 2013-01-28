bin_PROGRAMS = dnsa cmdb cbc

dnsa_SOURCES =	dnsa.c dnsacom.c rev_zone.c fwd_zone.c errors.c dzf.c drzf.c dnsa_mysql.c checks.c base_mysql.c\
		mysqlfunc.h cmdb_dnsa.h dnsa_mysql.h forward.h reverse.h cmdb.h checks.h
cmdb_SOURCES =	cmdb.c cmdbcom.c cmdb_mysql.c errors.c servers.c customers.c checks.c base_mysql.c\
		mysqlfunc.h cmdb_cmdb.h cmdb_mysql.h cmdb.h checks.h
cbc_SOURCES =	cbc.c cbc_mysql.c errors.c checks.c cbccom.c build_info.c build_add.c build_display.c base_mysql.c build_data_struct.c\
		build.h cbc_mysql.h cmdb_cbc.h cmdb.h mysqlfunc.h

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
