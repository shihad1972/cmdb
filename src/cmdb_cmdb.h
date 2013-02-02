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

typedef struct cmdb_comm_line_t { /* Hold parsed command line args */
	short int action;
	short int type;
	char config[CONF_S];
	char name[CONF_S];
	char id[CONF_S];
} cmdb_comm_line_t;

typedef struct cmdb_config_t { /* Hold CMDB configuration values */
	char dbtype[RANGE_S];
	char file[CONF_S];
	char db[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char socket[CONF_S];
	unsigned int port;
	unsigned long int cliflag;
} cmdb_config_t;

typedef struct cmdb_vm_host_t {
	char name[CONF_S];
	char type[CONF_S];
	unsigned long int id;
	struct cmdb_vm_host_t *next;
} cmdb_vm_host_t;

typedef struct cmdb_server_t {
	char vendor[CONF_S];
	char make[CONF_S];
	char model[CONF_S];
	char uuid[CONF_S];
	char name[MAC_S];
	unsigned long int cust_id;
	unsigned long int server_id;
	unsigned long int vm_server_id;
	struct cmdb_server_t *next;
} cmdb_server_t;

typedef struct cmdb_customer_t {
	char name[HOST_S];
	char address[NAME_S];
	char city[HOST_S];
	char county[MAC_S];
	char postcode[RANGE_S];
	char coid[RANGE_S];
	unsigned long int cust_id;
	struct cmdb_customer_t *next;
} cmdb_customer_t;

typedef struct cmdb_contact_t {
	char name[HOST_S];
	char phone[MAC_S];
	char email[HOST_S];
	unsigned long int cont_id;
	unsigned long int cust_id;
	struct cmdb_config_t *next;
} cmdb_contact_t;

typedef struct cmdb_hard_type_t {
	char type[HOST_S];
	char hclass[HOST_S];
	unsigned long int ht_id;
	struct cmdb_hard_type_t *next;
} cmdb_hard_type_t;

typedef struct cmdb_hardware_t {
	char detail[HOST_S];
	char device[MAC_S];
	unsigned long int hard_id;
	unsigned long int server_id;
	unsigned long int ht_id;
	struct cmdb_hardware_t *next;
} cmdb_hardware_t;

typedef struct cmdb_service_t {
	char detail[HOST_S];
	char url[NAME_S];
	unsigned long int service_id;
	unsigned long int server_id;
	unsigned long int cust_id;
	struct cmdb_service_t *next;
} cmdb_service_t;

typedef struct cmdb_service_type_t {
	char service[MAC_S];
	char detail[HOST_S];
	unsigned long int service_id;
} cmdb_service_type_t;

typedef struct cmdb_t {
	struct cmdb_server_t *server;
	struct cmdb_vm_host_t *vmhost;
	struct cmdb_hardware_t *hardware;
	struct cmdb_hard_type_t *hardtype;
	struct cmdb_customer_t *customer;
	struct cmdb_contact_t *contact;
	struct cmdb_service_t *service;
	struct cmdb_service_type_t *servicetype;
} cmdb_t;

void
cmdb_main_free(cmdb_comm_line_t *cm, cmdb_config_t *cmc, char *cmdb_config);
int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_t *comm);
int
parse_cmdb_config_file(cmdb_config_t *dc, char *config);
void
init_cmdb_config_values(cmdb_config_t *dc);
void
cmdb_init_struct(cmdb_t *cmdb);
void
cmdb_clean_list(cmdb_t *cmdb);
/*
int
cmdb_use_mysql(cmdb_config_t *cmc, cmdb_comm_line_t *cm, int retval);
int
cmdb_use_sqlite(cmdb_config_t *cmc, cmdb_comm_line_t *cm);
*/
/*
int
display_customer_info_on_coid(char *coid, cmdb_config_t *config);
int
display_customer_info_on_name(char *name, cmdb_config_t *config); */
/* Routines to display server data from database.
void
display_server_from_name(char **server_info);
void
display_server_from_uuid(char **server_info);
Server functions */
/*
int
get_server_hardware(cmdb_config_t *config, cmdb_hardware_t *head, unsigned long int id); */

/*  Routines to display customer data from database.
void
display_customer_from_name(char **cust_info);
void
display_customer_from_coid(char **cust_info);
void
display_all_customers(cmdb_config_t *config); 
int
add_server_to_database(cmdb_config_t *config); 
int
add_hardware_to_db(cmdb_config_t *config, cmdb_hardware_t *hw); */
void
get_full_server_config(cmdb_server_t *server);
void
print_hardware_details(cmdb_hardware_t *hard);
/* Linked list functions for virtual machine hosts */
/*
cmdb_vm_host_t *
get_vm_host(cmdb_config_t *config); */
/* Linked list fucntions for customers 
cmdb_customer_t
*create_customer_node(void);

unsigned long int
get_customer_for_server(cmdb_config_t *config); */
/* linked list functions for hardware and hardware types
void
add_hardware_types(cmdb_config_t *config, cmdb_hard_type_t *hthead); */
cmdb_vm_host_t *
vm_host_create(void);
cmdb_hard_type_t *
hard_type_node_create(void);
cmdb_hardware_t *
hard_node_create(void);
cmdb_hard_type_t *
get_network_device_id(cmdb_hard_type_t *head);
cmdb_hard_type_t *
get_disk_device_id(cmdb_hard_type_t *head);
int
get_network_device(cmdb_hardware_t *head);
int
get_disk_device(cmdb_hardware_t *head);


void
display_server_info (char *name, char *uuid, cmdb_config_t *config);
void
display_all_servers(cmdb_config_t *config);
void
print_all_servers(cmdb_t *cmdb);
void
display_customer_info(char *server, char *uuid, cmdb_config_t *config);
void
display_all_customers(cmdb_config_t *config);

/* New server functions for linked list */

void
clean_server_list(cmdb_server_t *list);
void
print_server_details(cmdb_server_t *server, cmdb_customer_t *customer);

/* New customer functions for linked list */

void
clean_customer_list(cmdb_customer_t *list);
void
print_customer_details(cmdb_customer_t *list);
#endif

