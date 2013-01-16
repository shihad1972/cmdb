#
#
# Makefile for DNSA c programs
#
#

CC = gcc
RM = rm -f
CFLAGS = -ansi -std=c99 -pedantic -W -Wall -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -g -D_POSIX_C_SOURCE=200809L
CPPFLAGS =
LDFLAGS = -L/usr/lib/mysql
LIBS = -lmysqlclient -lpcre
DOBJECTS = dnsa.o dnsacom.o rev_zone.o fwd_zone.o errors.o dzf.o drzf.o dnsa_mysql.o checks.o base_mysql.o
COBJECTS = cmdb.o cmdbcom.o cmdb_mysql.o errors.o servers.o customers.o checks.o base_mysql.o
BOBJECTS = cbc.o cbc_mysql.o errors.o checks.o cbccom.o build_info.o build_add.o build_display.o base_mysql.o
DHEADERS = mysqlfunc.h cmdb_dnsa.h dnsa_mysql.h forward.h reverse.h cmdb.h checks.h
CHEADERS = mysqlfunc.h cmdb_cmdb.h cmdb_mysql.h cmdb.h checks.h
BHEADERS = build.h cbc_mysql.h cmdb_cbc.h cmdb.h mysqlfunc.h

all:	dnsa cmdb cbc

dnsa:	$(DOBJECTS) $(DHEADERS)
	$(CC) $(CFLAGS) -o $@ $(DOBJECTS) $(LDFLAGS) $(LIBS)

cmdb:	$(COBJECTS) $(CHEADERS)
	$(CC) $(CFLAGS) -o $@ $(COBJECTS) $(LDFLAGS) $(LIBS)

cbc:	$(BOBJECTS) $(BHEADERS)
	$(CC) $(CFLAGS) -o $@ $(BOBJECTS) $(LDFLAGS) $(LIBS)

.PHONY :	clean
clean :
	$(RM) *.o dnsa cmdb cbc

