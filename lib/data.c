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
 *  data.c
 *
 *  Contains the functions to initialise and destroy various data types
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ailsacmdb.h>

void
ailsa_clean_mkvm(void *vm)
{
	ailsa_mkvm_s *i;

	i = vm;
	if (i->name)
		my_free(i->name);
	if (i->pool)
		my_free(i->pool);
	if (i->uri)
		my_free(i->uri);
	if (i->storxml)
		my_free(i->storxml);
	if (i->path)
		my_free(i->path);
	if (i->network)
		my_free(i->network);
	if (i->vt)
		my_free(i->vt);
	free(i);
}

void
ailsa_init_string(ailsa_string_s *str)
{
	str->size = FILE_LEN;
	str->len = 0;
	str->string = ailsa_calloc(FILE_LEN, "string->string in ailsa_init_string");
}

void
ailsa_clean_string(ailsa_string_s *str)
{
	if (!(str))
		return;
	if (str->string)
		my_free(str->string);
	free(str);
}

void
ailsa_resize_string(ailsa_string_s *str)
{
	char *tmp;

	str->size *= 2;
	tmp = ailsa_realloc(str->string, str->size * sizeof(char), "In ailsa_resize_string");
	str->string = tmp;
}

void
ailsa_fill_string(ailsa_string_s *str, const char *s)
{
	size_t len;

	len = strlen(s);
	if (len + str->len >= str->size)
		ailsa_resize_string(str);
	len++;
	snprintf(str->string + str->len, len, "%s", s);
	str->len = strlen(str->string);
}

