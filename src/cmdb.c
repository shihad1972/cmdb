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

int main(int argc, char *argv[])
{
	cmdb_comm_line_s *cm;
	cmdb_config_s *cmc;
	cmdb_s *base;
	char *cmdb_config;
	int retval, cl;

	if (!(cmc = malloc(sizeof(cmdb_config_s))))
		report_error(MALLOC_FAIL, "cmc in cmdb.c");
	if (!(cm = malloc(sizeof(cmdb_comm_line_s))))
		report_error(MALLOC_FAIL, "cm in cmdb.c");
	if (!(base = malloc(sizeof(cmdb_s))))
		report_error(MALLOC_FAIL, "base in cmdb.c");
	if (!(cmdb_config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "cmdb_config in cmdb.c");
	cmdb_init_struct(base);
	init_cmdb_comm_line_values(cm);
	init_cmdb_config_values(cmc);
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
	/* Need to decide what to do if we have an error here */
	if (cm->type == SERVER) {
		if (cm->action == DISPLAY) {
			display_server_info(cm->name, cm->id, cmc);
		} else if (cm->action == LIST_OBJ) {
			display_all_servers(cmc);
		} else if (cm->action == ADD_TO_DB) {
			retval = add_server_to_database(cmc, cm, base, cl);
			if (retval > 0) {
				cmdb_main_free(cm, cmc, cmdb_config);
				cmdb_clean_list(base);
				printf("Error adding server to database\n");
				exit(DB_INSERT_FAILED);
			} else {
				printf("Added into database\n");
			}
		} else if (cm->action == RM_FROM_DB) {
			retval = remove_server_from_database(cmc, cm);
		} else {
			display_action_error(cm->action);
		}
	} else if (cm->type == CUSTOMER) {
		if (cm->action == DISPLAY) {
			display_customer_info(cm->name, cm->id, cmc);
		} else if (cm->action == LIST_OBJ) {
			display_all_customers(cmc);
		} else if (cm->action == ADD_TO_DB) {
			retval = add_customer_to_database(cmc, base);
			if (retval != 0) {
				cmdb_main_free(cm, cmc, cmdb_config);
				cmdb_clean_list(base);
				printf("Error %d adding customer to DB\n", retval);
				exit(DB_INSERT_FAILED);
			} else {
				printf("Added %s to database\n", base->customer->name);
			}
		} else if (cm->action == RM_FROM_DB) {
			retval = remove_customer_from_database(cmc, cm);
		} else {
			display_action_error(cm->action);
		}
	} else if (cm->type == CONTACT) {
		if ((cm->action == DISPLAY) || (cm->action == LIST_OBJ)) {
			if (strncmp(cm->id, "NULL", CONF_S) != 0) {
				display_customer_contacts(cmc, cm->id);
			}
		} else if (cm->action == ADD_TO_DB) {
			if ((retval = add_contact_to_database(cmc, base)) != 0) {
				cmdb_main_free(cm, cmc, cmdb_config);
				printf("Error %d adding contact to DB\n", retval);
				cmdb_clean_list(base);
				exit(DB_INSERT_FAILED);
			} else {
				printf("Added %s to database\n", base->contact->name);
			}
		} else if (cm->action == RM_FROM_DB) {
			retval = remove_contact_from_database(cmc, cm);
		} else {
			display_action_error(cm->action);
		}
	} else if (cm->type == SERVICE) {
		if (cm->action == LIST_OBJ) {
			display_service_types(cmc);
		} else if (cm->action == DISPLAY) {
			if (cm->name)
				display_server_services(cmc, cm->name);
			else if (cm->id)
				display_customer_services(cmc, cm->id);
		} else if (cm->action == ADD_TO_DB) {
			if ((retval = add_service_to_database(cmc, base)) != 0) {
				cmdb_main_free(cm, cmc, cmdb_config);
				printf("Error %d adding service %s to DB\n",
					 retval, base->service->detail);
				cmdb_clean_list(base);
				exit(DB_INSERT_FAILED);
			} else {
				printf("Service %s added to database\n", base->service->detail);
			}
		} else if (cm->action == RM_FROM_DB) {
			retval = remove_service_from_database(cmc, cm);
		} else {
			display_action_error(cm->action);
		}
	} else if (cm->type == HARDWARE) {
		if (cm->action == LIST_OBJ) {
			display_hardware_types(cmc);
		} else if (cm->action == DISPLAY) {
			display_server_hardware(cmc, cm->name);
		} else if (cm->action == ADD_TO_DB) {
			if ((retval = add_hardware_to_database(cmc, base)) != 0) {
				cmdb_main_free(cm, cmc, cmdb_config);
				printf("Error %d adding hardware %s to DB\n",
					 retval, base->hardtype->hclass);
				cmdb_clean_list(base);
				exit(DB_INSERT_FAILED);
			} else {
				printf("\
Hardware for server %s added to database\n",base->server->name);
			}
		} else {
			display_action_error(cm->action);
		}
	} else if (cm->type == VM_HOST) {
		if (cm->action == LIST_OBJ) {
			display_vm_hosts(cmc);
		} else if (cm->action == ADD_TO_DB) {
			if ((retval = add_vm_host_to_db(cmc, cm, base)) == 0)
				printf("Added vm host server %s to db\n", cm->name);
			else
				printf("Error adding vm host server %s to db\n", cm->name);
		} else {
			printf("Action not supported (yet)\n");
			display_action_error(cm->action);
		}
	} else {
		display_type_error(cm->type);
	}
	cmdb_clean_list(base);
	cmdb_main_free(cm, cmc, cmdb_config);
	if (retval > 0)
		report_error(retval, " from main ");
	exit (retval);
}

