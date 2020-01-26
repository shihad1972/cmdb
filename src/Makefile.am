bin_PROGRAMS = cmdb2 cpc ckc mkvm mksp
AM_CPPFLAGS = -I$(top_srcdir)/include/

mkvm_SOURCES = mkvm.c virtual.c virtual.h
mksp_SOURCES = mksp.c virtual.c virtual.h
cmdb2_SOURCES = cmdb2.c cmdb2com.c
dnsa_SOURCES = dnsa.c zones.c dnsacom.c dnsa_net.c
CBC_COMBO = cbc_common.c cbccom.c
CBC_DNSA = cbc_dnsa.c dnsacom.c dnsa_net.c zones.c
cbc_SOURCES = cbc.c $(CBC_COMBO) build.c createbuild.c
cbcdomain_SOURCES = cbcdomain.c $(CBC_COMBO) cbcnet.c cbcnet.h cbcdomain.h

if HAVE_CBC
bin_PROGRAMS += cbc cbcdomain cbcos cbcpart cbcvarient cbclocale cbcsysp cbcscript
endif

if HAVE_DNSA
bin_PROGRAMS += dnsa
if HAVE_CBC
cbc_SOURCES += $(CBC_DNSA)
cbcdomain_SOURCES += $(CBC_DNSA)
endif
endif

AM_CFLAGS = -W -Wall -Wshadow -Wcast-qual -Wwrite-strings -D_XOPEN_SOURCE=700
AM_LDFLAGS = -L$(top_builddir)/lib
LIBS += -lailsasql -lailsacmdb

if HAVE_MYSQL
AM_CPPFLAGS += $(MYSQL_CFLAGS)
AM_LDFLAGS += $(MYSQL_LDFLAGS)
endif

if HAVE_PCRE
AM_CPPFLAGS += $(PCRE_CFLAGS)
AM_LDFLAGS += $(PCRE_LIBS)
endif

if HAVE_SQLITE3
AM_CPPFLAGS += $(SQLITE3_CFLAGS)
AM_LDFLAGS += $(SQLITE3_LDFLAGS)
endif

if HAVE_LIBXML
AM_CPPFLAGS += $(XML_CPPFLAGS)
LIBS += $(XML_LIBS)
endif

if HAVE_LIBVIRT
AM_CPPFLAGS += $(LIBVIRT_CPPFLAGS)
AM_CFLAGS += $(LIBVIRT_CFLAGS)
AM_LDFLAGS += $(LIBVIRT_LDFLAGS)
LIBS += $(LIBVIRT_LIBS)
endif
