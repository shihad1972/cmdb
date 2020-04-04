/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  helper.c
 *
 *
 *  Helper functions for libailsasql
 *
 *
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif // HAVE_STDBOOL_H
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /*HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /*HAVE_SQLITE3 */
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "base_sql.h"


int
cmdb_add_hard_type_id_to_list(char *hclass, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(hclass) || !(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *hard, *results;
	AILELEM *res;
	ailsa_data_s *data = NULL, *tmp;
	int retval = 0;

	hard = ailsa_db_data_list_init();
	results = ailsa_db_data_list_init();
	data = ailsa_db_text_data_init();
	data->data->text = strndup(hclass, MAC_LEN);
	if ((retval = ailsa_list_insert(hard, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert class name into list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, HARDWARE_TYPE_ID_ON_CLASS, hard, results)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot run SQL query: %d", retval);
		goto cleanup;
	}
	if (results->total > 1) {
		ailsa_syslog(LOG_INFO, "More than 1 hard type id returned. Using first one");
	} else if (results->total < 1) {
		ailsa_syslog(LOG_INFO, "No hard type's returned.");
		retval = NO_HARDWARE_TYPES;
		goto cleanup;
	}
	res = results->head;
	tmp = res->data;
	data = ailsa_db_lint_data_init();
	data->data->number = tmp->data->number;
	if ((retval = ailsa_list_insert(list, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert hard type id into list");
		goto cleanup;
	}
	cleanup:
		ailsa_list_destroy(hard);
		ailsa_list_destroy(results);
		my_free(hard);
		my_free(results);
		return retval;
}

int
cmdb_add_server_id_to_list(char *name, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(name) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();
	AILELEM *element;
	ailsa_data_s *tmp;

	data->data->text = strndup(name, HOST_LEN);
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert name into server list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVER_ID_ON_NAME, server, results)) != 0) {
		ailsa_syslog(LOG_ERR, "SERVER_ID_ON_NAME query failed");
		goto cleanup;
	}
	if (results->total == 0) {
		ailsa_syslog(LOG_INFO, "No server %s found in database", name);
		retval = SERVER_NOT_FOUND;
		goto cleanup;
	} else if (results->total > 1) {
		ailsa_syslog(LOG_INFO, "More than 1 server found searching on %s: using first one", name);
	}
	element = results->head;
	tmp = element->data;
	data = ailsa_db_lint_data_init();
	data->data->number = tmp->data->number;
	if ((retval = ailsa_list_insert(list, data)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert server id into list");

	cleanup:
		ailsa_list_destroy(server);
		ailsa_list_destroy(results);
		my_free(server);
		my_free(results);
		return retval;
}

int
cmdb_add_service_type_id_to_list(char *type, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(type) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *st = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();
	AILELEM *element;
	ailsa_data_s *tmp;

	data->data->text = strndup(type, SERVICE_LEN);
	if ((retval = ailsa_list_insert(st, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert service type name into list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVICE_TYPE_ID_ON_SERVICE, st, results)) != 0) {
		ailsa_syslog(LOG_ERR, "SERVICE_TYPE_ID_ON_SERVICE query failed");
		goto cleanup;
	}
	if (results->total == 0) {
		ailsa_syslog(LOG_INFO, "No service with type %s was found in the database", type);
		goto cleanup;
	} else if (results->total > 1) {
		ailsa_syslog(LOG_INFO, "Multiple services types found for %s. Using first one", type);
	}
	element = results->head;
	tmp = element->data;
	data = ailsa_db_lint_data_init();
	data->data->number = tmp->data->number;
	if ((retval = ailsa_list_insert(list, data)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert service type id into list");

	cleanup:
		ailsa_list_destroy(st);
		ailsa_list_destroy(results);
		my_free(st);
		my_free(results);
		return retval;
}

int
cmdb_add_cust_id_to_list(char *coid, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(coid) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *customer = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();
	AILELEM *element;
	ailsa_data_s *tmp;

	data->data->text = strndup(coid, MAC_LEN);
	if ((retval = ailsa_list_insert(customer, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert customer name into list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CUST_ID_ON_COID, customer, results)) != 0) {
		ailsa_syslog(LOG_ERR, "CUST_ID_ON_COID query failed");
		goto cleanup;
	}
	if (results->total == 0) {
		ailsa_syslog(LOG_ERR, "Cannot find customer with coid %s", coid);
		goto cleanup;
	} else if (results->total > 1) {
		ailsa_syslog(LOG_INFO, "More than one customer with coid %s. Using first one");
	}
	element = results->head;
	tmp = element->data;
	data = ailsa_db_lint_data_init();
	data->data->number = tmp->data->number;
	if ((retval = ailsa_list_insert(list, data)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert cust id into list");

	cleanup:
		ailsa_list_destroy(customer);
		ailsa_list_destroy(results);
		my_free(customer);
		my_free(results);
		return retval;
}