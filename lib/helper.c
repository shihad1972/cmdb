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
 *  Helper functions for libailsasql
 *
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
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

int
cmdb_add_hard_type_id_to_list(char *hclass, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(hclass) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *hard = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(hclass, hard)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add hardware class to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, HARDWARE_TYPE_ID_ON_CLASS, hard, list)) != 0)
		ailsa_syslog(LOG_ERR, "HARDWARE_TYPE_ID_ON_CLASS query failed");

	cleanup:
		ailsa_list_full_clean(hard);
		return retval;
}

int
cmdb_add_server_id_to_list(char *name, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(name) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *server = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(name, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVER_ID_ON_NAME, server, list)) != 0)
		ailsa_syslog(LOG_ERR, "SERVER_ID_ON_NAME query failed");

	cleanup:
		ailsa_list_full_clean(server);
		return retval;
}

int
cmdb_add_service_type_id_to_list(char *type, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(type) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *st = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(type, st)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add service type to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVICE_TYPE_ID_ON_SERVICE, st, list)) != 0)
		ailsa_syslog(LOG_ERR, "SERVICE_TYPE_ID_ON_SERVICE query failed");

	cleanup:
		ailsa_list_full_clean(st);
		return retval;
}

int
cmdb_add_build_domain_id_to_list(char *domain, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *dom = ailsa_db_data_list_init();

	if (!(domain)) {
		if ((retval = ailsa_basic_query(cc, DEFAULT_DOMAIN, list)) != 0) {
			ailsa_syslog(LOG_ERR, "DEFAULT_DOMAIN query failed");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list(domain, dom)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add domain to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cc, BUILD_DOMAIN_ID_ON_DOMAIN, dom, list)) != 0)
			ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_ID_ON_DOMAIN query failed");
	}

	cleanup:
		ailsa_list_full_clean(dom);
		return retval;
}

int
cmdb_add_cust_id_to_list(char *coid, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *customer = ailsa_db_data_list_init();

	if (!(coid)) {
		if ((retval = ailsa_basic_query(cc, DEFAULT_CUSTOMER, list)) != 0) {
			ailsa_syslog(LOG_ERR, "DEFAULT_CUSTOMER query failed");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list(coid, customer)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add COID to customer list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cc, CUST_ID_ON_COID, customer, list)) != 0) {
			ailsa_syslog(LOG_ERR, "CUST_ID_ON_COID query failed");
			goto cleanup;
		}
	}

	cleanup:
		ailsa_list_full_clean(customer);
		return retval;
}

int
cmdb_add_varient_id_to_list(char *varient, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *args = ailsa_db_data_list_init();

	if (!(varient)) {
		if ((retval = ailsa_basic_query(cc, DEFAULT_VARIENT, list)) != 0) {
			ailsa_syslog(LOG_ERR, "DEFAULT_VARIENT query failed");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list(varient, args)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add varient to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(varient, args)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add valias to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cc, VARIENT_ID_ON_VARIANT_OR_VALIAS, args, list)) != 0)
			ailsa_syslog(LOG_ERR, "VARIENT_ID_ON_VARIANT_OR_VALIAS, query failed");
	}

	cleanup:
		ailsa_list_full_clean(args);
		return retval;
}

int
cmdb_add_scheme_id_to_list(char *scheme, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *s = ailsa_db_data_list_init();
	int retval;

	if (!(scheme)) {
		if ((retval = ailsa_basic_query(cc, DEFAULT_SCHEME, list)) != 0) {
			ailsa_syslog(LOG_ERR, "DEFAULT_SCHEME query failed");
		}
	} else {
		if ((retval = cmdb_add_string_to_list(scheme, s)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add scheme name to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cc, SCHEME_ID_ON_NAME, s, list)) != 0)
			ailsa_syslog(LOG_ERR, "SCHEME_ID_ON_NAME query failed");
	}
	cleanup:
		ailsa_list_full_clean(s);
		return retval;
}

int
cmdb_add_default_part_id_to_list(char **args, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(args) || !(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *part = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(args[0], part)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add scheme name to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(args[1], part)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add partition name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, PARTITION_ID_ON_SEED_MOUNT, part, list)) != 0)
		ailsa_syslog(LOG_ERR, "PARTITION_ID_ON_SEED_MOUNT query failed");
	cleanup:
		ailsa_list_full_clean(part);
		return retval;
}

int
cmdb_add_disk_dev_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(server) || !(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(server, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, DISK_ID_ON_SERVER_NAME, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "DISK_ID_ON_SERVER_NAME query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_build_type_id_to_list(char *alias, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(alias) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *build = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(alias, build)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert build alias into list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, BT_ID_ON_ALIAS, build, list)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_TYPE_ID_ON_ALIAS query failed");
		goto cleanup;
	}

	cleanup:
		ailsa_list_full_clean(build);
		return retval;
}

int
cmdb_add_os_id_to_list(char **args, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	char *os = NULL;
	char *version = NULL;
	char *arch = NULL;
	int retval = 0;
	AILLIST *a = ailsa_db_data_list_init();

	if (!(args[0]) && !(args[1]) && !(args[2])) {
		if ((retval = ailsa_basic_query(cc, DEFAULT_OS, list)) != 0) {
			ailsa_syslog(LOG_ERR, "DEFAULT_OS query failed");
			goto cleanup;
		}
	} else {
		os = args[0];
		version = args[1];
		arch = args[2];
		if ((retval = cmdb_add_string_to_list(os, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add OS name to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(os, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add OS alias to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(version, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add version to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(version, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add version alias to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(arch, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add OS arch to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cc, BUILD_OS_ON_ALL, a, list)) != 0)
			ailsa_syslog(LOG_ERR, "CHECK_BUILD_OS query failed");
	}
	cleanup:
		ailsa_list_full_clean(a);
		return retval;
}

int
cmdb_add_os_alias_to_list(char *os, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(os) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	size_t total = list->total;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(os, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add OS name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, OS_ALIAS_ON_OS_NAME, l, list)) != 0) {
		ailsa_syslog(LOG_ERR, "OS_ALIAS_ON_OS_NAME query failed");
		goto cleanup;
	}
	if (list->total == total)
		retval = AILSA_NO_OS;

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}
int
cmdb_add_zone_id_to_list(char *zone, int type, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(zone) || !(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(zone, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone name %s to list", zone);
		goto cleanup;
	}
	switch (type) {
	case FORWARD_ZONE:
		if ((retval = ailsa_argument_query(cc, FWD_ZONE_ID_ON_ZONE_NAME, l, list)) != 0)
			ailsa_syslog(LOG_ERR, "FWD_ZONE_ID_ON_ZONE_NAME query failed");
		break;
	case REVERSE_ZONE:
		if ((retval = ailsa_argument_query(cc, REV_ZONE_ID_ON_RANGE, l, list)) != 0)
			ailsa_syslog(LOG_ERR, "REV_ZONE_ID_ON_RANGE query failed");
		break;
	default:
		retval = UNKNOWN_ZONE_TYPE;
		break;
	}
	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_sys_pack_id_to_list(char *pack, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(pack) || !(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(pack, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add package to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SYSTEM_PACKAGE_ID, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "SYSTEM_PACKAGE_ARG_ID query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_sys_pack_arg_id_to_list(char **args, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	char *name = args[0];
	char *field = args[1];
	int retval = 0;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add package name into list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(field, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add package field into list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SYSTEM_PACKAGE_ARG_ID, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "SYSTEM_PACKAGE_ARG_ID query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_system_script_id_to_list(char *name, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add script name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SYSTEM_SCRIPT_ID_ON_NAME, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPT_ID_ON_NAME query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_vm_server_id_to_list(char *name, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(name) || !(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add vm server name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, VM_SERVER_ID_ON_NAME, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "VM_SERVER_ID_ON_NAME query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_build_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(server) || !(cc) || !(list))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(server, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, BUILD_ID_ON_SERVER_NAME, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "BUILD_ID_ON_SERVER_NAME query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_locale_id_to_list(char *locale, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if (!(locale)) {
		if ((retval = ailsa_basic_query(cc, DEFAULT_LOCALE, list)) != 0) {
			ailsa_syslog(LOG_ERR, "DEFAULT_LOCALE query failed");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list(locale, l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add locale name to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cc, LOCALE_ID_FROM_NAME, l, list)) != 0) {
			ailsa_syslog(LOG_ERR, "LOCALE_ID_ON_NAME query failed");
			goto cleanup;
		}
	}

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_ip_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(server) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(server, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, IP_ID_ON_SERVER_NAME, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "IP_ID_ON_SERVER_NAME query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_add_disk_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(server) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(server, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, DISK_ID_ON_SERVER_NAME, l, list)) != 0)
		ailsa_syslog(LOG_ERR, "DISK_ID_ON_SERVER_NAME query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_check_for_fwd_zone(ailsa_cmdb_s *cc, char *zone)
{
	if (!(cc) || !(zone))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_zone_id_to_list(zone, FORWARD_ZONE, cc, l)) != 0) {
		retval = -1;
		goto cleanup;
	}
	retval = (int)l->total;
	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_check_for_fwd_record(ailsa_cmdb_s *cc, AILLIST *rec)
{
	if (!(cc) || !(rec))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	switch(rec->total) {
	case 3:
		if ((retval = ailsa_argument_query(cc, RECORD_ID_ON_HOST_ZONE_TYPE, rec, l)) != 0) {
			ailsa_syslog(LOG_ERR, "RECORD_ID_ON_HOST_ZONE_TYPE query failed");
			retval = -1;
			goto cleanup;
		}
		break;
	case 4:
		if ((retval = ailsa_argument_query(cc, RECORD_ID_BASE, rec, l)) != 0) {
			ailsa_syslog(LOG_ERR, "RECORD_ID_BASE query failed");
			retval = -1;
			goto cleanup;
		}
		break;
	case 5:
		if ((retval = ailsa_argument_query(cc, RECORD_ID_MX, rec, l)) != 0) {
			ailsa_syslog(LOG_ERR, "RECORD_ID_MX query failed");
			retval = -1;
			goto cleanup;
		}
		break;
	case 7:
		if ((retval = ailsa_argument_query(cc, RECORD_ID_SRV, rec, l)) != 0) {
			ailsa_syslog(LOG_ERR, "RECORD_ID_SRV query failed");
			retval = -1;
			goto cleanup;
		}
		break;
	default:
		ailsa_syslog(LOG_ERR, "Got %zu list total in cmdb_check_for_fwd_record");
		retval = -1;
		goto cleanup;
		break;
	}
	retval = (int)l->total;
	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_check_for_os(ailsa_cmdb_s *cc, char *os, char *arch, char *version)
{
	int retval = 0;
	if (!(cc) || !(os) || !(arch) || !(version ))
		return retval;
	AILLIST *list = ailsa_db_data_list_init();
	char **args = ailsa_calloc((sizeof(char *) * 3), "args in cmdb_check_for_os");

	args[0] = os;
	args[1] = version;
	args[2] = arch;
	if ((retval = cmdb_add_os_id_to_list(args, cc, list)) != 0) {
		retval = -1;
		goto cleanup;
	}
	retval = (int)list->total;

	cleanup:
		my_free(args);
		ailsa_list_full_clean(list);
		return retval;
}

int
check_builds_for_os_id(ailsa_cmdb_s *cc, unsigned long int id, AILLIST *list)
{
	if (!(cc) || !(list) || (id == 0))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_lint_data_init();

	data->data->number = id;
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert os_id into list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVERS_WITH_BUILDS_ON_OS_ID, args, list)) != 0) {
		ailsa_syslog(LOG_ERR, "SERVERS_WITH_BUILDS_ON_OS_ID query failed");
		goto cleanup;
	}
	cleanup:
		ailsa_list_full_clean(args);
		return retval;
}

int
cmdb_check_add_server_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(server) || !(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	size_t total = list->total;

	if ((retval = cmdb_add_server_id_to_list(server, cc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server id to list for %s", server);
		return retval;
	}
	if (list->total != (total + 1)) {
		ailsa_syslog(LOG_ERR, "Cannot find server %s", server);
		retval = 1;
	}
	return retval;
}

int
cmdb_check_add_cust_id_to_list(char *coid, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(cc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	size_t total = list->total;

	if ((retval = cmdb_add_cust_id_to_list(coid, cc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add customer id to list");
		return retval;
	}
	if (list->total != (total + 1)) {
		ailsa_syslog(LOG_ERR, "Cannot find customer with coid %s", coid);
		retval = 1;
	}
	return retval;
}

int
cmdb_check_add_zone_id_to_list(char *zone, int type, ailsa_cmdb_s *cc, AILLIST *list)
{
	if (!(zone) || !(cc) || !(list) || (type == 0))
		return AILSA_NO_DATA;
	int retval;
	size_t total = list->total;

	if ((retval = cmdb_add_zone_id_to_list(zone, type, cc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone id to list");
		return retval;
	}
	if (list->total != (total + 1)) {
		ailsa_syslog(LOG_ERR, "Cannot find zone %s", zone);
		retval = 1;
	}
	return retval;
}

int
set_db_row_updated(ailsa_cmdb_s *cc, unsigned int query, char *name, unsigned long int number)
{
	if (!(cc) || (query == 0) || (!(name) && number == 0))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *a = ailsa_db_data_list_init();
	unsigned long int user = (unsigned long int)getuid();

	if ((retval = cmdb_add_number_to_list(user, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add muser to list");
		goto cleanup;
	}
	if (name) {
		if ((retval = cmdb_add_string_to_list(name, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add name to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_number_to_list(number, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add number to list");
			goto cleanup;
		}
	}
	if ((retval = ailsa_update_query(cc, update_queries[query], a)) != 0)
		ailsa_syslog(LOG_ERR, "Update query for %s failed", name);

	cleanup:
		ailsa_list_full_clean(a);
		return retval;
}

int
cmdb_replace_data_element(AILLIST *list, AILELEM *element, size_t number)
{
	if (!(list) || !(element))
		return AILSA_NO_DATA;
	size_t i;
	size_t esize = sizeof(AILELEM);
	size_t dsize = sizeof(ailsa_data_s);
	size_t usize = sizeof(ailsa_data_u);
	AILELEM *e = list->head;
	AILELEM *r = ailsa_calloc(esize, "r in cmdb_replace_data_element");
	ailsa_data_s *d = ailsa_calloc(dsize, "d in cmdb_replace_data_element");
	ailsa_data_u *u = ailsa_calloc(usize, "u in cmdb_replace_data_element");
	ailsa_data_s *s = element->data;

	memcpy(r, element, esize);
	memcpy(d, s, dsize);
	memcpy(u, s->data, usize);
	r->data = d;
	d->data = u;
	if (d->type == AILSA_DB_TEXT)
		u->text = strndup(((ailsa_data_s *)element->data)->data->text, DOMAIN_LEN);
	if (number == 0) {
		list->head = r;
		r->next = e->next;
		r->next->prev = r;
	} else if (number == (list->total - 1)) {
		e = list->tail;
		list->tail = r;
		r->prev = e->prev;
		r->prev->next = r;
	} else {
		for (i = 0; i < number; i++) {
			if (e)
				e = e->next;
		}
		e->next->prev = r;
		e->prev->next = r;
		r->next = e->next;
		r->prev = e->prev;
	}
	if (list->destroy)
		list->destroy(e->data);
	my_free(e);
	return 0;
}
