/* 
 *
 *  cmdb : Configuration Management Database
 *  Copyright (C) 2016  Iain M Conochie <iain-AT-thargoid.co.uk> 
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ailsacmdb.h>
#include <cmdb.h>
#include <cmdb_cmdb.h>
#include <cmdb_sql.h>

int
main(int argc, char *argv[])
{
	int retval = 0;
	cmdb_comm_line_s *cm = ailsa_calloc(sizeof(cmdb_comm_line_s), "cm in main");
	size_t len = sizeof(struct cmdbd_config);
	struct cmdbd_config *cc = ailsa_calloc(len, "cc in main");

	if ((retval = parse_cmdb_command_line(argc, argv, cm)) != 0)
		goto cleanup;
	cmdbd_parse_config(cm->config, cc, len);
//	cmdbd_print_config(cc);
	switch(cm->action) {
	case ADD_TO_DB:
		retval = cmdb_add_to_db(cm, cc);
		break;
	case LIST_OBJ:
		retval = cmdb_list_from_db(cm, cc);
		break;
	}

	cleanup:
		cmdbd_clean_config(cc);
		clean_cmdb_comm_line(cm);
		my_free(cc);
		if (retval != 0)
			display_command_line_error(retval, argv[0]);
		return retval;
}

