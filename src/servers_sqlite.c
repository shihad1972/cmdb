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
	const char *file, *cquery;
	char *query;
	int retval, cretval;
	sqlite3 *cmdb;
	sqlite3_stmt *state;
	
	retval = 0;
	file = config->file;
	
	if (!(query = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_all_sqlite_servers");
	cquery = query;
	snprintf(query, TBUFF_S, "\
SELECT server_id, name, uuid FROM server");
	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READONLY, NULL)) > 0) {
		free(query);
		report_error(CANNOT_OPEN_FILE, file);
	}
	if ((retval = sqlite3_prepare_v2(cmdb, cquery, CONF_S, &state, NULL)) > 0) {
		cretval = sqlite3_close(cmdb);
		free(query);
		report_error(SQLITE_STATEMENT_FAILED, "display_all_sqlite_servers");
	}
	while((retval = sqlite3_step(state)) == SQLITE_ROW) {
		printf("%d\t%s\t%s\n",
		       sqlite3_column_int(state, 0),
		       sqlite3_column_text(state, 1),
		       sqlite3_column_text(state, 2));
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	free(query);
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
	const char *file, *cquery;
	char *query;
	int retval, cretval;
	sqlite3 *cmdb;
	sqlite3_stmt *state;
	
	if (!(query = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_on_name_sqlite");
	retval = 0;
	file = config->file;
	cquery = query;
	printf("Server %s info\n", name);
	snprintf(query, CONF_S, "\
SELECT vendor, make, model, uuid, name FROM server WHERE name = '%s'", name);
	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READONLY, NULL)) > 0) {
		free(query);
		report_error(CANNOT_OPEN_FILE, file);
	}
	if ((retval = sqlite3_prepare_v2(cmdb, cquery, CONF_S, &state, NULL)) > 0) {
		cretval = sqlite3_close(cmdb);
		free(query);
		report_error(SQLITE_STATEMENT_FAILED, "display_all_sqlite_servers");
	}
	while((retval = sqlite3_step(state)) == SQLITE_ROW) {
		printf("\
Vendor: %s\n\
Make: %s\n\
Model: %s\n\
UUID: %s\n", 		sqlite3_column_text(state, 0),
			sqlite3_column_text(state, 1),
			sqlite3_column_text(state, 2),
		        sqlite3_column_text(state, 3));
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	free(query);
	return retval;
}

int display_on_uuid_sqlite(char *uuid, cmdb_config_t *config)
{
	int retval;
	
	retval = 0;
	printf("Displaying server UUID %s\n", uuid);
	
	return retval;
}
