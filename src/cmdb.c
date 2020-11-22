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
 *  cmdb.c
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
#include <ailsasql.h>
#include <cmdb.h>
#include <cmdb_cmdb.h>

static int
cmdb_server_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_customer_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_contact_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_service_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_service_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_hardware_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_hardware_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_vm_host_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

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
	case CONTACT:
		retval = cmdb_contact_actions(cm, cc);
		break;
	case SERVICE:
		retval = cmdb_service_actions(cm, cc);
		break;
	case SERVICE_TYPE:
		retval = cmdb_service_type_actions(cm, cc);
		break;
	case HARDWARE:
		retval = cmdb_hardware_actions(cm, cc);
		break;
	case HARDWARE_TYPE:
		retval = cmdb_hardware_type_actions(cm, cc);
		break;
	case VM_HOST:
		retval = cmdb_vm_host_actions(cm, cc);
		break;
	default:
		ailsa_syslog(LOG_ERR, "Unknown type %d", cm->type);
		goto cleanup;
	}

	cleanup:
		ailsa_clean_cmdb(cc);
		clean_cmdb_comm_line(cm);
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
		cmdb_list_servers(cc);
		break;
	case DISPLAY:
		cmdb_display_server(cm, cc);
		break;
	case RM_FROM_DB:
		retval = cmdb_remove_server_from_database(cm, cc);
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
	switch(cm->action) {
	case ADD_TO_DB:
		retval = cmdb_add_customer_to_database(cm, cc);
		break;
	case LIST_OBJ:
		cmdb_list_customers(cc);
		break;
	case DISPLAY:
		cmdb_display_customer(cm, cc);
		break;
	case CMDB_DEFAULT:
		retval = cmdb_set_default_customer(cm, cc);
		break;
	case VIEW_DEFAULT:
		cmdb_display_default_customer(cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_contact_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) | !(cc))
		return AILSA_NO_DATA;
	switch(cm->action) {
	case ADD_TO_DB:
		retval = cmdb_add_contacts_to_database(cm, cc);
		break;
	case LIST_OBJ:
		cmdb_list_contacts_for_customer(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_service_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) | !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case LIST_OBJ:
		cmdb_list_services_for_server(cm, cc);
		break;
	case ADD_TO_DB:
		cmdb_add_services_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_hardware_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case LIST_OBJ:
		cmdb_list_hardware_for_server(cm, cc);
		break;
	case ADD_TO_DB:
		cmdb_add_hardware_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_vm_host_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case LIST_OBJ:
		cmdb_list_vm_server_hosts(cc);
		break;
	case DISPLAY:
		cmdb_display_vm_server(cm, cc);
		break;
	case ADD_TO_DB:
		retval = cmdb_add_vm_host_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_service_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case LIST_OBJ:
		cmdb_list_service_types(cc);
		break;
	case ADD_TO_DB:
		retval = cmdb_add_service_type_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_hardware_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case LIST_OBJ:
		cmdb_list_hardware_types(cc);
		break;
	case ADD_TO_DB:
		retval = cmdb_add_hardware_type_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = WRONG_TYPE;
		break;
	}
	return retval;
}
