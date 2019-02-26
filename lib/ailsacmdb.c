/*
 *
 *  alisacmdb: Alisatech Configuration Management Database library
 *  Copyright (C) 2015 Iain M Conochie <iain-AT-thargoid.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *  
 *  ailsacmdb.c
 *
 *  Contains generic functions for the ailsacmdb library
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_REGEX_H
# include <regex.h>
#endif // HAVE_REGEX_H
#include <ailsacmdb.h>

void
show_ailsacmdb_version()
{
	printf("libailsacmdb: Version %s\n", VERSION);
}

void
ailsa_chomp(char *line)
{
	char *p;

	p = strchr(line, '\n');
	if (p)
		*p = '\0';
}

void
ailsa_munch(char *line)
{
	char *p;

	p = strstr(line, "\r\n");
	if (p)
		*p = '\0';
}

void *
ailsa_calloc(size_t len, const char *msg)
{
	void *p;

	if (!(p = calloc(len, sizeof(char)))) {
		perror(msg);
		exit(errno);
	}
	return p;
}

void *
ailsa_realloc(void *r, size_t len, const char *msg)
{
	void *p;

	if (!(p = realloc(r, len * sizeof(char)))) {
		perror(msg);
		exit(errno);
	}
	return p;
}


int
ailsa_add_trailing_slash(char *member)
{
	size_t len;
	int retval;
	
	retval = 0;
	len = strlen(member);
	if ((member[len - 1] != '/') && len < 127) {
		member[len] = '/';
		member[len + 1] = '\0';
	} else if (member[len - 1] == '/') {
		retval = 0;
	} else {
		retval = -1;
	}
	
	return retval;
}

int
ailsa_add_trailing_dot(char *member)
{
// Maximum string size is 255 bytes 
	size_t len;
	int retval;
	
	retval = 0;
	if ((len = strlen(member)) > 254)
		return -1;
	if (member[len - 1] != '.') {
		member[len] = '.';
		member[len +1] = '\0';
	} else if (member[len - 1] == '.') {
		retval = 0;
	} else {
		retval = -1;
	}
	return retval;
}

int
ailsa_gen_mac(char *mac, int type)
{
	int retval = 0;
	char buf[MAC_LEN];
	long int r, a, m;
	if (!(mac))
		return AILSA_NO_DATA;
	r = random();
	if (type == AILSA_ESX) {
		snprintf(buf, MAC_LEN, "00:50:56:%x:%x:%x",
			(r >> 24), (r >> 16) & 0xff, (r >> 8) & 0xff);
	} else if (type == AILSA_KVM) {
		snprintf(buf, MAC_LEN, "52:54:00:%x:%x:%x",
			(r >> 24), (r >> 16) & 0xff, (r >> 8) & 0xff);
	} else {
		snprintf(buf, MAC_LEN, "26:20:31:%x:%x:%x",
			(r >> 24), (r >> 16) & 0xff, (r >> 8) & 0xff);
	}
	snprintf(mac, MAC_LEN, "%s", buf);
	return retval;
}

#ifdef HAVE_REGEXEC


#endif
