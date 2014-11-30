bin_PROGRAMS = cmdb cbc cbcdomain cbcos cbcvarient cbcpart cpc

combo_SOURCES = errors.c checks.c

cbc_combo_SOURCES = cbc_base_sql.c cbc_data.c cbc_common.c

cpc_SOURCES = cpc.c $(combo_SOURCES) cmdb_cpc.h

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
		cmdb_cbc.h cbc_base_sql.h cmdb.h checks.h cbc_common.h\
		cbc_data.h build.h base_sql.h dnsa_net.h
cbcdomain_SOURCES = \
		cbcdomain.c $(combo_SOURCES) base_sql.c \
		$(cbc_combo_SOURCES) builddomain.c cbccom.c zones.c cbc_dnsa.c\
		dnsa_base_sql.c dnsacom.c cbcnet.c dnsa_net.c cbcnet.h cbc_common.h\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h builddomain.h\
		base_sql.h cbcdomain.h cbc_dnsa.h cmdb_dnsa.h dnsa_base_sql.h
else
cbc_SOURCES =	cbc.c cbccom.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		build.c createbuild.c\
		cmdb_cbc.h cbc_base_sql.h cmdb.h checks.h cbc_common.h \
		cbc_data.h build.h base_sql.h
cbcdomain_SOURCES = \
		cbcdomain.c $(combo_SOURCES) base_sql.c \
		$(cbc_combo_SOURCES) builddomain.c cbccom.c cbcnet.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h builddomain.h\
		base_sql.h cbcdomain.h cbc_common.h cbcnet.h
endif
cbcos_SOURCES = \
		cbcos.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES) cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h cbcos.h cbc_common.h
cbcvarient_SOURCES = \
		cbcvarient.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES) cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h\
		cbcvarient.h cbc_common.h
cbcpart_SOURCES = \
		cbcpart.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		cbccom.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h cbcpart.h cbc_common.h

AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_LDFLAGS =
AM_LIBS = 

if HAVE_MYSQL
AM_CFLAGS += $(MYSQL_CFLAGS)
AM_LDFLAGS += $(MYSQL_LDFLAGS)
AM_LIBS += -lmysqlclient
endif

if HAVE_PCRE
AM_CFLAGS += $(PCRE_CFLAGS)
AM_LIBS += $(PCRE_LIBS)
endif

if HAVE_SQLITE3
AM_CFLAGS += $(SQLITE3_CFLAGS)
AM_LDFLAGS += $(SQLITE3_LDFLAGS)
AM_LIBS += -lsqlite3
endif
