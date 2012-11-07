#
#
# Makefile for DNSA c programs
#
#

CC = gcc
RM = rm -f
CFLAGS = -ansi -std=c99 -pedantic -W -Wall -Wconversion -Wshadow -Wcast-qual -Wwrite-strings
CPPFLAGS = -I/usr/include/mysql
LDFLAGS = -L/usr/lib/i386-linux-gnu -L/usr/lib/mysql
LIBS = -lmysqlclient

all:	dnsa

dnsa:	dnsa.o command.o wzf.o wrzf.o wrz.o wfz.o errors.o dzf.o drzf.o wcf.o wrcf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

dnsa.o:	dnsa.c dnsa.h forward.h reverse.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

dzf.o:	dzf.c forward.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

drzf.o:	drzf.c reverse.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wzf.o:	wzf.c forward.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wrzf.o:	wrzf.c reverse.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wfz.o: fwd_zone.c forward.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wrz.o:	rev_zone.c dnsa.h reverse.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wcf.o:	wcf.c forward.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $< 

wrcf.o:	wrcf.c reverse.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

command.o:	commline.c dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

errors.o:	errors.c dnsa.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

.PHONY :	clean
clean :
	$(RM) *.o dnsa

