/* 
 *
 *  cmdb : Configuration Management Database
 *  Copyright (C) 2016 - 2020 Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  cmdb2.c
 *
 *  Contains main functions for cmdb2 program
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <syslog.h>
#include <ailsacmdb.h>
#include <cmdb.h>
#include <cmdb_cmdb.h>
#include <cmdb_sql.h>

static int
cmdb_server_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_customer_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

int
main(int argc, char *argv[])
{
	int retval = 0;
	ailsa_start_syslog(basename(argv[0]));
	cmdb_comm_line_s *cm = ailsa_calloc(sizeof(cmdb_comm_line_s), "cm in main");
	size_t len = sizeof(ailsa_cmdb_s);
	ailsa_cmdb_s *cc = ailsa_calloc(len, "cc in main");

	if ((retval = parse_cmdb_command_line(argc, argv, cm)) != 0)
		goto cleanup;
	parse_cmdb_config(cc);
	switch(cm->type) {
	case SERVER:
		retval = cmdb_server_actions(cm, cc);
		break;
	case CUSTOMER:
		retval = cmdb_customer_actions(cm, cc);
		break;
	default:
		ailsa_syslog(LOG_ERR, "Unknown type %d", cm->type);
		goto cleanup;
	}

	cleanup:
		cmdbd_clean_config(cc);
		clean_cmdb_comm_line(cm);
		my_free(cc);
		if (retval != 0)
			display_command_line_error(retval, argv[0]);
		return retval;
}

static int
cmdb_server_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	switch(cm->action) {
	case ADD_TO_DB:
		retval = cmdb_add_server_to_database(cm, cc);
		break;
	case LIST_OBJ:
		retval = cmdb_list_servers(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_customer_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;

	return retval;
}

