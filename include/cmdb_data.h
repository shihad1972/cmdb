/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_data.h
 */

#ifndef __CMDB_DATA_H__
# define __CMDB_DATA_H__
# include <ailsacmdb.h>

typedef struct cmdb_config_s { /* Hold CMDB configuration values */
	char dbtype[SERVICE_LEN];
	char file[CONFIG_LEN];
	char db[CONFIG_LEN];
	char user[CONFIG_LEN];
	char pass[CONFIG_LEN];
	char host[CONFIG_LEN];
	char socket[CONFIG_LEN];
	unsigned int port;
	unsigned long int cliflag;
} cmdb_config_s;

typedef struct cmdb_server_s {
	unsigned short int type;
	char name[HOST_LEN];
	char server_name[CONFIG_LEN];
	unsigned long int server_id;
	unsigned long int server_type_id;
	unsigned long int cust_id;
	unsigned long int vm_server_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
	struct cmdb_server_s *next;
} cmdb_server_s;

typedef struct cmdb_server_type_s {
	unsigned short int type;
	char vendor[HOST_LEN];
	char make[HOST_LEN];
	char model[MAC_LEN];
	char alias[MAC_LEN];
	unsigned long int server_type_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
	struct cmdb_server_type_s *next;
} cmdb_server_type_s;

typedef struct cmdb_customer_s {
	struct cmdb_customer_s *next;
	char name[HOST_LEN];
	char address[CONFIG_LEN];
	char city[HOST_LEN];
	char county[MAC_LEN];
	char postcode[SERVICE_LEN];
	char coid[SERVICE_LEN];
	unsigned long int cust_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_customer_s;

typedef struct cmdb_contact_s {
	struct cmdb_contact_s *next;
	char name[HOST_LEN];
	char phone[MAC_LEN];
	char email[HOST_LEN];
	unsigned long int cont_id;
	unsigned long int cust_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_contact_s;

typedef struct cmdb_service_s {
	struct cmdb_service_s *next;
	struct cmdb_service_type_s *servicetype;
	char detail[HOST_LEN];
	char url[CONFIG_LEN];
	unsigned long int service_id;
	unsigned long int server_id;
	unsigned long int cust_id;
	unsigned long int service_type_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_service_s;

typedef struct cmdb_service_type_s {
	struct cmdb_service_type_s *next;
	char service[SERVICE_LEN];
	char detail[MAC_LEN];
	unsigned long int service_id;
} cmdb_service_type_s;

typedef struct cmdb_hardware_s {
	struct cmdb_hardware_s *next;
	struct cmdb_hard_type_s *hardtype;
	char detail[HOST_LEN];
	char device[MAC_LEN];
	unsigned long int hard_id;
	unsigned long int server_id;
	unsigned long int ht_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_hardware_s;

typedef struct cmdb_hard_type_s {
	struct cmdb_hard_type_s *next;
	char type[MAC_LEN];
	char hclass[MAC_LEN];
	unsigned long int ht_id;
} cmdb_hard_type_s;

typedef struct cmdb_vm_HOST_LEN {
	struct cmdb_vm_HOST_LEN *next;
	char name[CONFIG_LEN];
	char type[MAC_LEN];
	unsigned long int id;
	unsigned long int server_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_vm_HOST_LEN;

typedef struct cmdb_s {
	struct cmdb_server_s *server;
	struct cmdb_server_type_s *servertype;
	struct cmdb_customer_s *customer;
	struct cmdb_contact_s *contact;
	struct cmdb_service_s *service;
	struct cmdb_service_type_s *servicetype;
	struct cmdb_hardware_s *hardware;
	struct cmdb_hard_type_s *hardtype;
	struct cmdb_vm_HOST_LEN *vmhost;
} cmdb_s;

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
	char *shtype;
	char *fullname;
	short int action;
	short int type;
	short int force;
	unsigned long int sid;
} cmdb_comm_line_s;

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
cmdb_init_vmhost_t(cmdb_vm_HOST_LEN *type);
/* New clean functions for linked list */

void
cmdb_clean_list(cmdb_s *cmdb);
void
clean_server_list(cmdb_server_s *list);
void
clean_server_type_list(cmdb_server_type_s *list);
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
clean_vmhost_list(cmdb_vm_HOST_LEN *list);
void
clean_cmdb_comm_line(cmdb_comm_line_s *list);

/* Fill struct functions. These use the regex to check input */
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

// pre database access functions
int
cmdb_add_to_db(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);
int
cmdb_list_from_db(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

#endif // __CMDB_DATA_H__
