#
#
# Makefile for DNSA c programs
#
#

CC = gcc
RM = rm -f
CFLAGS = -ansi -std=c99 -pedantic -W -Wall -Wconversion -Wshadow -Wcast-qual -Wwrite-strings
CPPFLAGS = -I/usr/include/mysql
LDFLAGS = -L/usr/lib/i386-linux-gnu
LIBS = -lmysqlclient

all:	wzf wcf wrzf wrcf

wzf:	write_zone.o wzf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

write_zone.o: write_zone.c write_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $< 

wzf.o:	wzf.c write_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wcf:	wcf.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

wcf.o:	wcf.c write_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $< 

wrzf:	write_rev_zone.o wrzf.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

wrzf.o:	wrzf.c rev_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

write_rev_zone.o:	write_rev_zone.c dnsa.h rev_zone.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

wrcf:	wrcf.o write_rev_zone.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

wrcf.o:	wrcf.c rev_zone.h dnsa.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

.PHONY :	clean
clean :
	$(RM) *.o wzf wcf wrzf