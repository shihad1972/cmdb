/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  Contains main() function for cmdb program
 *
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "base_sql.h"

int
cmdb_server_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb);

int
cmdb_customer_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb);

int
cmdb_contact_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb);

int
cmdb_service_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb);

int
cmdb_hardware_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb);

int
cmdb_vmhost_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb);

int main(int argc, char *argv[])
{
	cmdb_comm_line_s *cm;
	cmdb_config_s *cmc;
	cmdb_s *base;
	char *cmdb_config;
	int retval, cl;

	if (!(cmdb_config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "cmdb_config in cmdb.c");
	cmdb_setup_config(&cmc, &cm, &base);
	cl = parse_cmdb_command_line(argc, argv, cm, base);
	if ((retval = check_for_comm_line_errors(cl, cm)) != 0) {
		cmdb_main_free(cm, cmc, cmdb_config);
		cmdb_clean_list(base);
		display_command_line_error(retval, argv[0]);
	}
	sprintf(cmdb_config, "%s", cm->config);
	if ((retval = parse_cmdb_config_file(cmc, cmdb_config)) != 0) {
		cmdb_main_free(cm, cmc, cmdb_config);
		cmdb_clean_list(base);
		report_error(retval, "config");
	}
	if (cm->type == SERVER)
		retval = cmdb_server_action(cmc, cm, base);
	else if (cm->type == CUSTOMER)
		retval = cmdb_customer_action(cmc, cm, base);
	else if (cm->type == CONTACT)
		retval = cmdb_contact_action(cmc, cm, base);
	else if (cm->type == SERVICE)
		retval = cmdb_service_action(cmc, cm, base);
	else if (cm->type == HARDWARE)
		retval = cmdb_hardware_action(cmc, cm, base);
	else if (cm->type == VM_HOST)
		retval = cmdb_vmhost_action(cmc, cm, base);
	else
		display_type_error(cm->type);
	cmdb_clean_list(base);
	cmdb_main_free(cm, cmc, cmdb_config);
	if (retval > 0)
		report_error(retval, " from main ");
	exit(retval);
}

int
cmdb_server_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = 0;
	if (cm->action == DISPLAY) {
		display_server_info(cm->name, cm->id, ccs);
	} else if (cm->action == LIST_OBJ) {
		display_all_servers(ccs);
	} else if (cm->action == ADD_TO_DB) {
		if ((retval = add_server_to_database(ccs, cm, cmdb)) != 0) {
			printf("Error %d adding server %s to database\n",
			 retval, cm->name);
			retval = DB_INSERT_FAILED;
		} else {
			printf("Added into database\n");
		}
	} else if (cm->action == RM_FROM_DB) {
		retval = remove_server_from_database(ccs, cm);
	} else if (cm->action == MODIFY) {
		retval = update_server_in_database(ccs, cm);
	} else {
		display_action_error(cm->action);
	}
	return retval;
}

int
cmdb_customer_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = 0;
	if (cm->action == DISPLAY) {
		display_customer_info(cm->name, cm->id, ccs);
	} else if (cm->action == LIST_OBJ) {
		display_all_customers(ccs);
	} else if (cm->action == ADD_TO_DB) {
		retval = add_customer_to_database(ccs, cmdb);
		if (retval != 0) {
			printf("Error %d adding customer to DB\n", retval);
			retval = DB_INSERT_FAILED;
		} else {
			printf("Added %s to database\n", cmdb->customer->name);
		}
	} else if (cm->action == RM_FROM_DB) {
		retval = remove_customer_from_database(ccs, cm);
	} else {
		display_action_error(cm->action);
	}
	return retval;
}

int
cmdb_contact_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = 0;
	if ((cm->action == DISPLAY) || (cm->action == LIST_OBJ)) {
		if (strncmp(cm->id, "NULL", CONF_S) != 0) {
			display_customer_contacts(ccs, cm->id);
		}
	} else if (cm->action == ADD_TO_DB) {
		if ((retval = add_contact_to_database(ccs, cmdb)) != 0) {
			printf("Error %d adding contact to DB\n", retval);
			retval = DB_INSERT_FAILED;
		} else {
			printf("Added %s to database\n", cmdb->contact->name);
		}
	} else if (cm->action == RM_FROM_DB) {
		retval = remove_contact_from_database(ccs, cm);
	} else {
		display_action_error(cm->action);
	}
	return retval;
}

int
cmdb_service_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = 0;
	if (cm->action == LIST_OBJ) {
		display_service_types(ccs);
	} else if (cm->action == DISPLAY) {
		if (cm->name)
			display_server_services(ccs, cm->name);
		else if (cm->id)
			display_customer_services(ccs, cm->id);
	} else if (cm->action == ADD_TO_DB) {
		if ((retval = add_service_to_database(ccs, cmdb)) != 0) {
			printf("Error %d adding service %s to DB\n",
				 retval, cmdb->service->detail);
			retval = DB_INSERT_FAILED;
		} else {
			printf("Service %s added to database\n", cmdb->service->detail);
		}
	} else if (cm->action == RM_FROM_DB) {
		retval = remove_service_from_database(ccs, cm);
	} else {
		display_action_error(cm->action);
	}
	return retval;
}

int
cmdb_hardware_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = 0;
	if (cm->action == LIST_OBJ) {
		display_hardware_types(ccs);
	} else if (cm->action == DISPLAY) {
		display_server_hardware(ccs, cm->name);
	} else if (cm->action == ADD_TO_DB) {
		if ((retval = add_hardware_to_database(ccs, cmdb)) != 0) {
			printf("Error %d adding hardware %s to DB\n",
				 retval, cmdb->hardtype->hclass);
			retval = DB_INSERT_FAILED;
		} else {
			printf("Hardware for server %s added to database\n",
			  cmdb->server->name);
		}
	} else {
		display_action_error(cm->action);
	}
	return retval;
}

int
cmdb_vmhost_action(cmdb_config_s *ccs, cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = 0;
	if (cm->action == LIST_OBJ) {
		display_vm_hosts(ccs);
	} else if (cm->action == ADD_TO_DB) {
		if ((retval = add_vm_host_to_db(ccs, cm, cmdb)) == 0)
			printf("Added vm host server %s to db\n", cm->name);
		else
			printf("Error adding vm host server %s to db\n", cm->name);
	} else {
		printf("Action not supported (yet)\n");
		display_action_error(cm->action);
	}
	return retval;
}
