bin_PROGRAMS = cmdb mkvm mksp
AM_CPPFLAGS = -I$(top_srcdir)/include/

mkvm_SOURCES = mkvm.c virtual.c virtual.h
mksp_SOURCES = mksp.c virtual.c virtual.h
cmdb_SOURCES = cmdb.c cmdbcom.c servers.c customers.c
dnsa_SOURCES = dnsa.c zones.c dnsacom.c
CBC_DNSA = dnsacom.c zones.c
cbc_SOURCES = cbc.c build.c createbuild.c cbccom.c
cbcdomain_SOURCES = cbcdomain.c
cbcos_SOURCES = cbcos.c
cbcpart_SOURCES = cbcpart.c
cbclocale_SOURCES = cbclocale.c
cbcvarient_SOURCES = cbcvarient.c
cbcsysp_SOURCES = cbcsysp.c
cbcscript_SOURCES = cbcscript.c

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

if HAVE_OPENSSL
AM_CFLAGS += $(OPENSSL_INCLUDES)
AM_LDFLAGS += $(OPENSSL_LDFLAGS)
LIBS += $(OPENSSL_LIBS)
endif
