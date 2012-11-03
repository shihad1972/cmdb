#
#
# Makefile for DNSA c programs
#
#

CC = gcc
RM = rm -f
CFLAGS = -ansi -std=c99 -pedantic -W -Wall -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -g
CPPFLAGS = -I/usr/include/mysql
LDFLAGS = -L/usr/lib/i386-linux-gnu -L/usr/lib/mysql
LIBS = -lmysqlclient

all:	wzf wcf wrzf wrcf dnsa

dnsa:	dnsa.o command.o wzf.o wrzf.o write_rev_zone.o write_zone.o errors.o dzf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

dnsa.o:	dnsa.c dnsa.h write_zone.h rev_zone.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

write_zone.o: write_zone.c write_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

dzf.o:	dzf.c write_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wzf.o:	wzf.c write_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wcf.o:	wcf.c write_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $< 

wrzf.o:	wrzf.c rev_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

write_rev_zone.o:	write_rev_zone.c dnsa.h rev_zone.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wrcf.o:	wrcf.c rev_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

command.o:	commline.c dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

errors.o:	errors.c dnsa.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

.PHONY :	clean
clean :
	$(RM) *.o wzf wcf wrzf wrcf dnsa

wrcf:	wrcf.o write_rev_zone.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

wrzf:	write_rev_zone.o wrzf.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

wzf:	write_zone.o wzf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

wcf:	wcf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
