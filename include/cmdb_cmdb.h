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
 *  cmdb_cmdb.h
 */

#ifndef __CMDB_CMDB_H__
# define __CMDB_CMDB_H__
# include "cmdb_data.h"

void
cmdb_main_free(cmdb_comm_line_s *cm, cmdb_config_s *cmc, char *cmdb_config);
int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comm);
int
check_cmdb_comm_options(cmdb_comm_line_s *conf);
int
check_for_comm_line_errors(int cl, cmdb_comm_line_s *cm);
void
clean_cmdb_comm_line(cmdb_comm_line_s *list);

// Various DB functions

int
get_table_id(cmdb_config_s *cbc, int query, char *name, unsigned long int *id);


// New CMDB functions for re-worked program

int
cmdb_add_server_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_list_servers(ailsa_cmdb_s *cc);

void
cmdb_display_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_display_server_details(AILLIST *server);

int
cmdb_add_customer_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_list_customers(ailsa_cmdb_s *cc);

void
cmdb_display_customer(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

int
cmdb_set_default_customer(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_list_contacts_for_customer(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_display_contacts(AILLIST *list);

void
cmdb_display_customer_details(AILLIST *list);

void
cmdb_list_services_for_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_display_services(AILLIST *list);

void
cmdb_list_hardware_for_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_display_hardware(AILLIST *list);

int
cmdb_add_hardware_type_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

int
cmdb_add_hardware_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_list_vm_server_hosts(ailsa_cmdb_s *cc);

void
cmdb_display_vm_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

int
cmdb_add_vm_host_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_display_built_vms(AILLIST *list);

void
cmdb_list_service_types(ailsa_cmdb_s *cc);

int
cmdb_add_service_type_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

int
cmdb_add_services_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

void
cmdb_list_hardware_types(ailsa_cmdb_s *cc);

int
cmdb_add_contacts_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

#endif
