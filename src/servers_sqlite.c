/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  servers_sqlite.c
 * 
 *  Functions relating to servers in the SQLite database for the cmdb program
 * 
 *  (C) Iain M Conochie 2012 - 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "cmdb_sqlite.h"
#include "checks.h"

int insert_server_into_sqlite(cmdb_config_t *config, cmdb_server_t *server)
{
	int retval;
	
	retval = 0;
	printf("Function add into sqlite DB\n");
	
	return retval;
}

void display_all_sqlite_servers(cmdb_config_t *config)
{
	int retval;
	
	retval = 0;
	printf("Function display servers from sqlite DB\n");
}

int insert_hardware_into_sqlite(cmdb_config_t *config, cmdb_hardware_t *hardware)
{
	int retval;
	
	retval = 0;
	printf("Function add hardware into sqlite DB\n");
	
	return retval;
}

int display_on_name_sqlite(char *name, cmdb_config_t *config)
{
	int retval;
	
	retval = 0;
	printf("Displaying server %s\n", name);
	
	return retval;
}

int display_on_uuid_sqlite(char *uuid, cmdb_config_t *config)
{
	int retval;
	
	retval = 0;
	printf("Displaying server UUID %s\n", uuid);
	
	return retval;
}
