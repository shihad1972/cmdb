#
#
# Makefile for DNSA c programs
#
#
CC = gcc
RM = rm -f
ECHO = echo
DIR = src
SUBDIRS = src
bin_PROGRAMS = dnsa cmdb cbc

dnsa_SOURCES =	src/dnsa.c src/dnsacom.c src/rev_zone.c src/fwd_zone.c src/errors.c\
		src/dzf.c src/drzf.c src/dnsa_mysql.c src/checks.c src/base_mysql.c\
		src/mysqlfunc.h src/cmdb_dnsa.h src/dnsa_mysql.h src/forward.h\
		src/reverse.h src/cmdb.h src/checks.h

cbc_SOURCES =	src/cbc.c src/cbc_mysql.c src/errors.c src/checks.c src/cbccom.c\
		src/build_info.c src/build_add.c src/build_display.c src/base_mysql.c\
		src/build_data_struct.c\
		src/build.h src/cbc_mysql.h src/cmdb_cbc.h src/cmdb.h src/mysqlfunc.h

cmdb_SOURCES =	src/cmdb.c src/cmdbcom.c src/cmdb_mysql.c src/errors.c src/servers.c\
		src/customers.c src/checks.c src/base_mysql.c\
		src/mysqlfunc.h src/cmdb_cmdb.h src/cmdb_mysql.h src/cmdb.h src/checks.h

all:	$(bin_PROGRAMS)

dnsa:
	$(ECHO) Building dnsa : $(MAKE) $(MFLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS)

cmdb:
	$(ECHO) Building cmdb : $(MAKE) $(MFLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS)

cbc:
	$(ECHO) Building cbc : $(MAKE) $(MFLAGS)
	cd $(DIR); $(MAKE) $(MFLAGS)


