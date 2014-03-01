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
 *  cmdb_cmdb.h
 */

#ifndef __CMDB_CMDB_H__
#define __CMDB_CMDB_H__

typedef struct cmdb_comm_line_s { /* Hold parsed command line args */
	char *vmhost;
	char *config;
	char *vendor;
	char *make;
	char *model;
	char *id;
	char *uuid;
	char *stype;
	char *name;
	char *address;
	char *city;
	char *email;
	char *detail;
	char *hclass;
	char *url;
	char *device;
	char *phone;
	char *postcode;
	char *county;
	char *coid;
	char *service;
	short int action;
	short int type;
	short int force;
	unsigned long int sid;
} cmdb_comm_line_s;

typedef struct cmdb_config_s { /* Hold CMDB configuration values */
	char dbtype[RANGE_S];
	char file[CONF_S];
	char db[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char socket[CONF_S];
	unsigned int port;
	unsigned long int cliflag;
} cmdb_config_s;

typedef struct cmdb_server_s {
	char vendor[CONF_S];
	char make[CONF_S];
	char model[CONF_S];
	char uuid[CONF_S];
	char name[HOST_S];
	unsigned long int server_id;
	unsigned long int cust_id;
	unsigned long int vm_server_id;
	struct cmdb_server_s *next;
} cmdb_server_s;

typedef struct cmdb_customer_s {
	char name[HOST_S];
	char address[NAME_S];
	char city[HOST_S];
	char county[MAC_S];
	char postcode[RANGE_S];
	char coid[RANGE_S];
	unsigned long int cust_id;
	struct cmdb_customer_s *next;
} cmdb_customer_s;

typedef struct cmdb_contact_s {
	char name[HOST_S];
	char phone[MAC_S];
	char email[HOST_S];
	unsigned long int cont_id;
	unsigned long int cust_id;
	struct cmdb_contact_s *next;
} cmdb_contact_s;

typedef struct cmdb_service_s {
	char detail[HOST_S];
	char url[URL_S];
	unsigned long int service_id;
	unsigned long int server_id;
	unsigned long int cust_id;
	unsigned long int service_type_id;
	struct cmdb_service_s *next;
	struct cmdb_service_type_s *servicetype;
} cmdb_service_s;

typedef struct cmdb_service_type_s {
	char service[RANGE_S];
	char detail[MAC_S];
	unsigned long int service_id;
	struct cmdb_service_type_s *next;
} cmdb_service_type_s;

typedef struct cmdb_hardware_s {
	char detail[HOST_S];
	char device[MAC_S];
	unsigned long int hard_id;
	unsigned long int server_id;
	unsigned long int ht_id;
	struct cmdb_hardware_s *next;
	struct cmdb_hard_type_s *hardtype;
} cmdb_hardware_s;

typedef struct cmdb_hard_type_s {
	char type[MAC_S];
	char hclass[MAC_S];
	unsigned long int ht_id;
	struct cmdb_hard_type_s *next;
} cmdb_hard_type_s;

typedef struct cmdb_vm_host_s {
	char name[RBUFF_S];
	char type[MAC_S];
	unsigned long int id;
	unsigned long int server_id;
	struct cmdb_vm_host_s *next;
} cmdb_vm_host_s;

typedef struct cmdb_s {
	struct cmdb_server_s *server;
	struct cmdb_vm_host_s *vmhost;
	struct cmdb_hardware_s *hardware;
	struct cmdb_hard_type_s *hardtype;
	struct cmdb_customer_s *customer;
	struct cmdb_contact_s *contact;
	struct cmdb_service_s *service;
	struct cmdb_service_type_s *servicetype;
} cmdb_s;

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
void
init_cmdb_comm_line_values(cmdb_comm_line_s *cm);
void
init_cmdb_config_values(cmdb_config_s *dc);
void
cmdb_init_struct(cmdb_s *cmdb);
void
cmdb_init_server_t(cmdb_server_s *server);
void
cmdb_init_customer_t(cmdb_customer_s *cust);
void
cmdb_init_service_t(cmdb_service_s *service);
void
cmdb_init_hardware_t(cmdb_hardware_s *hard);
void
cmdb_init_contact_t(cmdb_contact_s *cont);
void
cmdb_init_hardtype_t(cmdb_hard_type_s *type);
void
cmdb_init_servicetype_t(cmdb_service_type_s *type);
void
cmdb_init_vmhost_t(cmdb_vm_host_s *type);

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

/* Fill struct functions. These use the pcre regex to check input */
int
fill_server_values(cmdb_comm_line_s *cm, cmdb_s *cmdb);
int
fill_customer_values(cmdb_comm_line_s *cm, cmdb_s *cmdb);
int
fill_service_values(cmdb_comm_line_s *cm, cmdb_s *cmdb);
int
fill_contact_values(cmdb_comm_line_s *cm, cmdb_s *cmdb);
int
fill_hardware_values(cmdb_comm_line_s *cm, cmdb_s *cmdb);
int
fill_vmhost_values(cmdb_comm_line_s *cm, cmdb_s *cmdb);

/* Complete the structs if comm line options were missing */
void
complete_server_values(cmdb_s *cmdb, int cl);

/* New user input functions */
int
add_server_to_database(cmdb_config_s *config, cmdb_comm_line_s *cm, cmdb_s *cmdb, int cl);
int
remove_server_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
add_customer_to_database(cmdb_config_s *config, cmdb_s *cmdb);
int
remove_customer_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
add_contact_to_database(cmdb_config_s *config, cmdb_s *base);
int
remove_contact_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
remove_service_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm);
int
add_service_to_database(cmdb_config_s *config, cmdb_s *cmdb);
int
add_hardware_to_database(cmdb_config_s *config, cmdb_s *cmdb);
int
check_for_vm_host(cmdb_config_s *config, cmdb_s *cmdb, char *vmhost);
int
check_for_coid(cmdb_config_s *config, cmdb_s *cmdb, char *coid);
int
get_customer(cmdb_config_s *config, cmdb_s *cmdb, char *coid);
void
print_vm_hosts(cmdb_vm_host_s *vmhost);

/* New clean functions for linked list */

void
cmdb_clean_list(cmdb_s *cmdb);
void
clean_server_list(cmdb_server_s *list);
void
clean_customer_list(cmdb_customer_s *list);
void
clean_contact_list(cmdb_contact_s *list);
void
clean_service_list(cmdb_service_s *list);
void
clean_service_type_list(cmdb_service_type_s *list);
void
clean_hardware_list(cmdb_hardware_s *list);
void
clean_hardware_type_list(cmdb_hard_type_s *list);
void
clean_vmhost_list(cmdb_vm_host_s *list);
void
clean_cmdb_comm_line(cmdb_comm_line_s *list);

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
#endif

