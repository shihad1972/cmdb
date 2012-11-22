#
#
# Makefile for DNSA c programs
#
#

CC = gcc
RM = rm -f
CFLAGS = -ansi -std=c99 -pedantic -W -Wall -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -g
CPPFLAGS =
LDFLAGS = -L/usr/lib/i386-linux-gnu -L/usr/lib/mysql
LIBS = -lmysqlclient
DOBJECTS = dnsa.o dnsacom.o wzf.o wrzf.o rev_zone.o fwd_zone.o errors.o dzf.o drzf.o wcf.o wrcf.o dnsa_mysql.o
COBJECTS = cmdb.o cmdbcom.o cmdb_mysql.o errors.o servers.o

all:	dnsa cmdb

dnsa:	$(DOBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

cmdb:	$(COBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY :	clean
clean :
	$(RM) *.o dnsa cmdb

