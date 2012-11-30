#
#
# Makefile for DNSA c programs
#
#

CC = gcc
RM = rm -f
CFLAGS = -ansi -std=c99 -pedantic -W -Wall -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -g
CPPFLAGS =
LDFLAGS = -L/usr/lib/mysql
LIBS = -lmysqlclient -lpcre
DOBJECTS = dnsa.o dnsacom.o rev_zone.o fwd_zone.o errors.o dzf.o drzf.o dnsa_mysql.o checks.o
COBJECTS = cmdb.o cmdbcom.o cmdb_mysql.o errors.o servers.o customers.o checks.o
BOBJECTS = cbc.o cbc_mysql.o errors.o checks.o cbccom.o build_info.o

all:	dnsa cmdb cbc

dnsa:	$(DOBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

cmdb:	$(COBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

cbc:	$(BOBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY :	clean
clean :
	$(RM) *.o dnsa cmdb cbc

