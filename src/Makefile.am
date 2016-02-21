bin_PROGRAMS = cmdb cpc ckc
AM_CPPFLAGS = -I$(top_srcdir)/include/

combo_SOURCES = errors.c checks.c cmdb_data.c

cbc_combo_SOURCES = cbc_base_sql.c cbc_data.c cbc_common.c cbccom.c

cpc_SOURCES = cpc.c $(combo_SOURCES)

ckc_SOURCES = ckc.c $(combo_SOURCES)

cmdb_SOURCES =	cmdb.c cmdbcom.c customers.c servers.c $(combo_SOURCES) \
		cmdb_base_sql.c base_sql.c\
		cmdb_cmdb.h cmdb_base_sql.h cmdb.h checks.h\
		base_sql.h

dnsa_SOURCES =	dnsa.c dnsacom.c $(combo_SOURCES) base_sql.c dnsa_base_sql.c\
		zones.c dnsa_net.c\
		cmdb_dnsa.h dnsa_base_sql.h cmdb.h checks.h\
		base_sql.h dnsa_net.h

cbcos_SOURCES = \
		cbcos.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		cbcnet.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h \
		cbc_common.h
cbcvarient_SOURCES = \
		cbcvarient.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h\
		cbc_common.h
cbcpart_SOURCES = \
		cbcpart.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h \
		cbc_common.h
cbc_SOURCES =	cbc.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES)\
		build.c createbuild.c\
		cmdb_cbc.h cbc_base_sql.h cmdb.h checks.h cbc_common.h \
		cbc_data.h build.h base_sql.h
cbcdomain_SOURCES = \
		cbcdomain.c $(combo_SOURCES) base_sql.c \
		$(cbc_combo_SOURCES) cbcnet.c\
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h\
		base_sql.h cbcdomain.h cbc_common.h cbcnet.h
cbclocale_SOURCES = \
		cbclocal.c $(combo_SOURCES) base_sql.c $(cbc_combo_SOURCES) \
		cmdb.h cmdb_cbc.h cbc_data.h cbc_base_sql.h base_sql.h \
		cbc_common.h
cbcsysp_SOURCES = \
		cbcsysp.c $(combo_SOURCES) $(cbc_combo_SOURCES) base_sql.c \
		cbcsysp.h cmdb.h cmdb_cbc.h cbc_data.h cbc_common.h base_sql.h \
		cbc_base_sql.h checks.h
cbcscript_SOURCES = \
		cbcscript.c $(combo_SOURCES) $(cbc_combo_SOURCES) base_sql.c \
		cbcscript.h cmdb.h cmdb_cbc.h cbc_data.h cbc_common.h base_sql.h \
		cbc_base_sql.h checks.h
if HAVE_CBC
bin_PROGRAMS += cbc cbcdomain cbcos cbcvarient cbcpart cbcsysp cbcscript cbclocale
endif

if HAVE_DNSA
bin_PROGRAMS += dnsa
if HAVE_CBC
cbc_SOURCES +=	cbc_dnsa.c dnsa_base_sql.c\
		dnsacom.c zones.c dnsa_net.c\
		dnsa_net.h
cbcdomain_SOURCES += zones.c cbc_dnsa.c\
		dnsa_base_sql.c dnsacom.c dnsa_net.c cbc_common.h\
		cbc_dnsa.h cmdb_dnsa.h dnsa_base_sql.h
endif
endif
AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_LDFLAGS =

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
