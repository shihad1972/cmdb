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
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comm, cmdb_s *base);
int
check_cmdb_comm_options(cmdb_comm_line_s *conf, cmdb_s *cmdb);
int
check_for_comm_line_errors(int cl, cmdb_comm_line_s *cm);
int
parse_cmdb_config_file(cmdb_config_s *dc, char *config);
int
read_cmdb_config_values(cmdb_config_s *dc, FILE *cnf);
void
init_cmdb_comm_line_values(cmdb_comm_line_s *cm);
void
cmdb_setup_config(cmdb_config_s **cf, cmdb_comm_line_s **com, cmdb_s **cmdb);
void
init_cmdb_config_values(cmdb_config_s *dc);
void
clean_cmdb_comm_line(cmdb_comm_line_s *list);

void
display_server_info (char *name, char *uuid, cmdb_config_s *config);
void
display_all_servers(cmdb_config_s *config);
void
print_all_servers(cmdb_s *cmdb);
void
display_customer_info(char *server, char *uuid, cmdb_config_s *config);
void
display_all_customers(cmdb_config_s *config);
void
display_service_types(cmdb_config_s *config);
void
display_hardware_types(cmdb_config_s *config);
void
display_server_hardware(cmdb_config_s *config, char *server);
void
display_server_services(cmdb_config_s *config, char *name);
void
display_customer_services(cmdb_config_s *config, char *coid);
void
display_customer_contacts(cmdb_config_s *config, char *coid);
int
print_customer_servers(cmdb_server_s *server, unsigned long int cust_id);
void
display_vm_hosts(cmdb_config_s *config);
int
add_vm_host_to_db(cmdb_config_s *cmc, cmdb_comm_line_s *cm, cmdb_s *base);

/* New user input functions */
int
add_server_to_database(cmdb_config_s *config, cmdb_comm_line_s *cm, cmdb_s *cmdb);
int
remove_server_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
update_server_in_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
add_customer_to_database(cmdb_config_s *config, cmdb_s *cmdb);
int
remove_customer_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
add_contact_to_database(cmdb_config_s *config, cmdb_s *base);
int
remove_contact_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
add_service_to_database(cmdb_config_s *config, cmdb_s *cmdb);
int
remove_service_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
add_hardware_to_database(cmdb_config_s *config, cmdb_s *cmdb);
int
remove_hardware_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
check_for_vm_host(cmdb_config_s *config, cmdb_s *cmdb, char *vmhost);
int
check_for_coid(cmdb_config_s *config, cmdb_s *cmdb, char *coid);
int
get_customer(cmdb_config_s *config, cmdb_s *cmdb, char *coid);
void
print_vm_hosts(cmdb_vm_host_s *vmhost);
void
set_server_updated(cmdb_config_s *config, unsigned long int *ids);
void
set_customer_updated(cmdb_config_s *config, cmdb_s *cmdb);
unsigned long int
cmdb_get_customer_id(cmdb_config_s *config, char *coid);
unsigned long int
cmdb_get_server_id(cmdb_config_s *config, char *server);
int
update_member_on_id(cmdb_config_s *config, char *member, unsigned long int id, int type);
int
update_member_id_on_id(cmdb_config_s *config, unsigned long int *id, int type);

/* New server functions for linked list */

void
print_server_details(cmdb_server_s *server, cmdb_s *base);
int
print_services(cmdb_service_s *service, unsigned long int id, int type);
int
print_hardware(cmdb_hardware_s *hard, unsigned long int id);

/* New customer functions for linked list */

void
print_customer_details(cmdb_customer_s *list, cmdb_s *base);
int
print_customer_contacts(cmdb_contact_s *contacts, unsigned long int cust_id);

// Various DB functions

int
get_table_id(cmdb_config_s *cbc, int query, char *name, unsigned long int *id);

#endif

