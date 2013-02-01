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
 *  cmdb_sqlite.h: Header file for cmdb sqlite functions 
 */

#include <sqlite3.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"

void
display_all_sqlite_servers(cmdb_config_t *config);
int
insert_server_into_sqlite(cmdb_config_t *config, cmdb_server_t *server);
int
insert_hardware_into_sqlite(cmdb_config_t *config, cmdb_hardware_t *hardware);
int
display_on_name_sqlite(char *name, cmdb_config_t *config);
int
display_on_uuid_sqlite(char *uuid, cmdb_config_t *config);