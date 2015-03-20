/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_data.c: data functions for the cmdb suite of programs
 * 
 *  Part of the CMDB program
 * 
 * 
 */
#include "../config.h"
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include "cmdb.h"

void *
cmdb_malloc(size_t len, const char *msg)
{
	void *data;
	if (!(data = malloc(len))) {
		perror(msg);
		exit(MALLOC_FAIL);
	}
	return data;
}

void
initialise_string_array(char *list[], size_t quantity, size_t quality[])
{
	size_t i;

	for (i = 0; i < quantity; i++)
		list[i] = cmdb_malloc(quality[i], "initialise_string_array");
}

void
cmdb_prep_db_query(dbdata_s **data, const unsigned int *values[], int query)
{
	unsigned int max = 0;
	max = cmdb_get_max(values[0][query], values[1][query]);
	init_multi_dbdata_struct(data, max);
}

unsigned int
cmdb_get_max_val(const unsigned int *search[], int query)
{
	unsigned int max;
	max = cmdb_get_max(search[0][query], search[1][query]);
	return max;
}

unsigned int
cmdb_get_max(const unsigned int args, const unsigned int fields)
{
	unsigned int max;

	max = (fields >= args) ? fields :  args ;
	return max;
}

int
check_data_length(dbdata_s *data, unsigned int len)
{
	unsigned int i = 0;
	if (!(data))
		report_error(CBC_NO_DATA, "data in check_data_length");
	dbdata_s *list = data;
	while (list) {
		i++;
		list = list->next;
	}
	if ((i % len) != 0)
		return 1;
	else
		return 0;
}

dbdata_s *
move_down_list_data(dbdata_s *data, unsigned int len)
{
	dbdata_s *list;
	unsigned int i;
	if (!(data))
		report_error(CBC_NO_DATA, "data in move_down_list_data");
	if (check_data_length(data, len) != 0)
		report_error(CBC_DATA_WRONG_COUNT, "move_down_list_data");
	list = data;
	for (i = len; i != 0; i--) {
		list = list->next;
	}
	return list;
}

