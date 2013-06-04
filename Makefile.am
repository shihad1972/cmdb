#
#
# Makefile for DNSA c programs
#
#
RM = rm -f
ECHO = echo
DIR = src
SUBDIRS = src
bin_PROGRAMS = dnsa cmdb cbc cbcdomain cbcos cbcvarient cbcpack cbcpart

dnsa_SOURCES =	src/dnsa.c src/dnsacom.c src/errors.c src/zones.c src/checks.c\
		src/dnsa_base_sql.c src/base_sql.c\
		src/cmdb_dnsa.h src/cmdb.h src/checks.h\
		src/dnsa_base_sql.h src/base_sql.h

cbc_SOURCES =	src/cbc.c src/errors.c src/checks.c src/cbccom.c createbuild.c\
		src/base_sql.c src/cbc_data.c src/build.c src/cbc_base_sql.c\
		src/cmdb_cbc.h src/cmdb.h src/cbc_data.h\
		src/build.h src/cbc_base_sql.h

cbcdomain_SOURCES = \
		src/cbcdomain.c src/errors.c src/checks.c src/base_sql.c\
		src/cbc_data.c src/cbc_base_sql.c src/builddomain.c\
		src/cbccom.c\
		src/cmdb.h src/cmdb_cbc.h src/cbc_data.h\
		src/cbc_base_sql.h src/builddomain.h src/cbcdomain.h

cbcos_SOURCES = src/cbcos.c src/errors.c src/checks.c src/base_sql.c\
		src/cbc_data.c src/cbc_base_sql.c src/cbccom.c\
		src/cmdb.h src/cmdb_cbc.h src/cbc_data.h\
		src/cbc_base_sql.h src/cbcos.h

cbcvarient_SOURCES = \
		src/cbcvarient.c src/errors.c src/checks.c src/base_sql.c\
		src/cbc_data.c src/cbc_base_sql.c src/cbccom.c\
		src/cmdb.h src/cmdb_cbc.h src/cbc_data.h\
		src/cbc_base_sql.h src/cbcvarient.h

cmdb_SOURCES =	src/cmdb.c src/cmdbcom.c src/errors.c src/servers.c\
		src/customers.c src/checks.c src/base_sql.c\
		src/cmdb_base_sql.c\
		src/cmdb_cmdb.h src/cmdb_base_sql.h src/cmdb.h\
		src/checks.h src/base_sql.h

cbcpack_SOURCES = \
		src/cbcpack.c src/errors.c src/checks.c src/base_sql.c\
		src/cbc_data.c src/cbc_base_sql.c src/cbccom.c\
		src/cmdb.h src/cmdb_cbc.h src/cbc_data.h\
		src/cbc_base_sql.h src/cbcpack.h

cbcpart_SOURCES = \
		src/cbcpart.c src/errors.c src/checks.c src/base_sql.c\
		src/cbc_data.c src/cbc_base_sql.c src/cbccom.c\
		src/cmdb.h src/cmdb_cbc.h src/cbc_data.h\
		src/cbc_base_sql.h src/cbcpart.h

EXTRA_DIST = \
		sql/mysql/all-tables-mysql.sql\
		sql/sqlite/all-tables-sqlite.sql\
		scripts/cmdb.sh\
		scripts/create_ldif_for_domain.sh

all:	$(bin_PROGRAMS)

dnsa:
	$(ECHO) Building dnsa : $(MAKE) $(MFLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) dnsa

cmdb:
	$(ECHO) Building cmdb : $(MAKE) $(MFLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) cmdb

cbc:
	$(ECHO) Building cbc : $(MAKE) $(MFLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) cbc

cbcdomain:
	$(ECHO) Building cbcdomain: $(MAKE) $(FLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) cbcdomain

cbcos:
	$(ECHO) Building cbcos: $(MAKE) $(FLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) cbcos

cbcvarient:
	$(ECHO) Building cbcvarient: $(MAKE) $(FLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) cbcvarient

cbcpack:
	$(ECHO) Building cbcpack: $(MAKE) $(FLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) cbcpack

cbcpart:
	$(ECHO) Building cbcpart: $(MAKE) $(MFLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS) cbcpart
