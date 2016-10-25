bin_PROGRAMS = cmdb cpc ckc
AM_CPPFLAGS = -I$(top_srcdir)/include/

combo_SOURCES = errors.c checks.c cmdb_data.c

cbc_combo_SOURCES = cbc_data.c cbc_common.c cbccom.c

cpc_SOURCES = cpc.c $(combo_SOURCES)

ckc_SOURCES = ckc.c $(combo_SOURCES)

cmdb_SOURCES =	cmdb.c cmdbcom.c customers.c servers.c $(combo_SOURCES)
cmdb_LDADD = $(top_builddir)/lib/libailsasql.la

dnsa_SOURCES =	dnsa.c dnsacom.c $(combo_SOURCES) zones.c dnsa_net.c dnsa_data.c
dnsa_LDADD = $(top_builddir)/lib/libailsasql.la

cbcos_SOURCES = cbcos.c $(combo_SOURCES) $(cbc_combo_SOURCES) cbcnet.c
cbcos_LDADD = $(top_builddir)/lib/libailsasql.la

cbcvarient_SOURCES = cbcvarient.c $(combo_SOURCES) $(cbc_combo_SOURCES)
cbcvarient_LDADD = $(top_builddir)/lib/libailsasql.la

cbcpart_SOURCES = cbcpart.c $(combo_SOURCES) $(cbc_combo_SOURCES)
cbcpart_LDADD = $(top_builddir)/lib/libailsasql.la

cbc_SOURCES =	cbc.c $(combo_SOURCES) $(cbc_combo_SOURCES) build.c createbuild.c
cbc_LDADD = $(top_builddir)/lib/libailsasql.la

cbcdomain_SOURCES = \
		cbcdomain.c $(combo_SOURCES) $(cbc_combo_SOURCES) cbcnet.c\
		cbcdomain.h cbcnet.h
cbcdomain_LDADD = $(top_builddir)/lib/libailsasql.la

cbclocale_SOURCES = \
		cbclocale.c $(combo_SOURCES) $(cbc_combo_SOURCES)
cbclocale_LDADD = $(top_builddir)/lib/libailsasql.la

cbcsysp_SOURCES = \
		cbcsysp.c $(combo_SOURCES) $(cbc_combo_SOURCES)\
		cbcsysp.h
cbcsysp_LDADD = $(top_builddir)/lib/libailsasql.la

cbcscript_SOURCES = \
		cbcscript.c $(combo_SOURCES) $(cbc_combo_SOURCES) \
		cbcscript.h
cbcscript_LDADD = $(top_builddir)/lib/libailsasql.la
if HAVE_CBC
bin_PROGRAMS += cbc cbcdomain cbcos cbcvarient cbcpart cbcsysp cbcscript cbclocale
endif

if HAVE_DNSA
bin_PROGRAMS += dnsa
if HAVE_CBC
cbc_SOURCES +=	cbc_dnsa.c dnsacom.c zones.c dnsa_net.c dnsa_data.c
cbcdomain_SOURCES += zones.c cbc_dnsa.c dnsacom.c dnsa_net.c dnsa_data.c
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
