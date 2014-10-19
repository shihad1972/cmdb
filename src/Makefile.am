bin_PROGRAMS = cmdb cbc cbcdomain cbcos cbcvarient cbcpart

combo_SOURCES = errors.c checks.c

cbc_combo_SOURCES = cbc_base_sql.c cbc_data.c cbc_common.c

cmdb_SOURCES =	cmdb.c cmdbcom.c customers.c servers.c $(combo_SOURCES) \
		cmdb_base_sql.c base_sql.c\
		cmdb_cmdb.h cmdb_base_sql.h cmdb.h checks.h\
		base_sql.h
if HAVE_DNSA
bin_PROGRAMS += dnsa

dnsa_SOURCES =	dnsa.c dnsacom.c $(combo_SOURCES) base_sql.c dnsa_base_sql.c\
		zones.c dnsa_net.c\
		cmdb_dnsa.h dnsa_base_sql.h cmdb.h checks.h\
		base_sql.h dnsa_net.h
cbc_SOURCES =	cbc.c cbccom.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		build.c createbuild.c cbc_dnsa.c dnsa_base_sql.c\
		dnsacom.c zones.c dnsa_net.c\
		cmdb_cbc.h cbc_base_sql.h cmdb.h checks.h \
		cbc_data.h build.h base_sql.h dnsa_net.h
cbcdomain_SOURCES = \
		cbcdomain.c $(combo_SOURCES) base_sql.c \
		$(cbc_combo_SOURCES) builddomain.c cbccom.c zones.c cbc_dnsa.c\
		dnsa_base_sql.c dnsacom.c cbcnet.c dnsa_net.c cbcnet.h\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h builddomain.h\
		base_sql.h cbcdomain.h cbc_dnsa.h cmdb_dnsa.h dnsa_base_sql.h
else
cbc_SOURCES =	cbc.c cbccom.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		build.c createbuild.c\
		cmdb_cbc.h cbc_base_sql.h cmdb.h checks.h \
		cbc_data.h build.h base_sql.h
cbcdomain_SOURCES = \
		cbcdomain.c $(combo_SOURCES) base_sql.c \
		$(cbc_combo_SOURCES) builddomain.c cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h builddomain.h\
		base_sql.h cbcdomain.h
endif
cbcos_SOURCES = \
		cbcos.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES) cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h cbcos.h
cbcvarient_SOURCES = \
		cbcvarient.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES) cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h\
		cbcvarient.h
cbcpart_SOURCES = \
		cbcpart.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h cbcpart.h

AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
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
AM_LDFLAGS += $(SQLITE3_LDFLAGS)
LIBS += -lsqlite3
endif
