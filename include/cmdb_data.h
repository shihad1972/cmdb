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
	struct cmdb_server_s *next;
	char vendor[CONF_S];
	char make[CONF_S];
	char model[CONF_S];
	char uuid[CONF_S];
	char name[HOST_S];
	char server_name[RBUFF_S];
	unsigned long int server_id;
	unsigned long int cust_id;
	unsigned long int vm_server_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_server_s;

typedef struct cmdb_customer_s {
	struct cmdb_customer_s *next;
	char name[HOST_S];
	char address[NAME_S];
	char city[HOST_S];
	char county[MAC_S];
	char postcode[RANGE_S];
	char coid[RANGE_S];
	unsigned long int cust_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_customer_s;

typedef struct cmdb_contact_s {
	struct cmdb_contact_s *next;
	char name[HOST_S];
	char phone[MAC_S];
	char email[HOST_S];
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
	char detail[HOST_S];
	char url[RBUFF_S];
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
	char service[RANGE_S];
	char detail[MAC_S];
	unsigned long int service_id;
} cmdb_service_type_s;

typedef struct cmdb_hardware_s {
	struct cmdb_hardware_s *next;
	struct cmdb_hard_type_s *hardtype;
	char detail[HOST_S];
	char device[MAC_S];
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
	char type[MAC_S];
	char hclass[MAC_S];
	unsigned long int ht_id;
} cmdb_hard_type_s;

typedef struct cmdb_vm_host_s {
	struct cmdb_vm_host_s *next;
	char name[RBUFF_S];
	char type[MAC_S];
	unsigned long int id;
	unsigned long int server_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} cmdb_vm_host_s;

typedef struct cmdb_s {
	struct cmdb_server_s *server;
	struct cmdb_customer_s *customer;
	struct cmdb_contact_s *contact;
	struct cmdb_service_s *service;
	struct cmdb_service_type_s *servicetype;
	struct cmdb_hardware_s *hardware;
	struct cmdb_hard_type_s *hardtype;
	struct cmdb_vm_host_s *vmhost;
} cmdb_s;

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

#endif // __CMDB_DATA_H__
