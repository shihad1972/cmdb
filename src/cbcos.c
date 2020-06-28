/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcos.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcos program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include <config.h>
#include <configmake.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "cbc_common.h"
#include "cbcnet.h"

typedef struct cbcos_comm_line_s {
	char *alias;
	char *arch;
	char *os;
	char *ver_alias;
	char *version;
	short int action;
	short int force;
} cbcos_comm_line_s;

static int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col);

static void
list_cbc_build_os(ailsa_cmdb_s *cmc);

static int
display_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static int
add_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static int
remove_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static int
cbcos_grab_boot_files(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static void
cbcos_check_for_null_in_comm_line(cbcos_comm_line_s *col, int *test);

static void
cbcos_check_for_os(cbcos_comm_line_s *col, AILELEM *head, int *test);

static void
cbcos_get_os_string(char *error, cbcos_comm_line_s *col);

static void
cbcos_clean_comm_line(cbcos_comm_line_s *cl);

static int
cmdb_fill_os_details(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col, AILLIST *os);

static int
cbcos_create_os_profile(ailsa_cmdb_s *cmc, AILLIST *os);

static int
cbcos_insert_os_id_user_into_list(AILLIST *pack, unsigned long int id);

static int
ailsa_insert_cuser_muser(AILLIST *list, AILELEM *elem);

static void
display_server_name_for_build_os_id(AILLIST *list);

int
main (int argc, char *argv[])
{
	char error[URL_S];
	int retval = 0;
	ailsa_cmdb_s *cmc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmc in main");
	cbcos_comm_line_s *cocl = ailsa_calloc(sizeof(cbcos_comm_line_s), "cocl in main");

	memset(error, 0, URL_S);
	if ((retval = parse_cbcos_comm_line(argc, argv, cocl)) != 0) {
		cbcos_clean_comm_line(cocl);
		ailsa_clean_cmdb(cmc);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(cmc);
	if (cocl->action == LIST_CONFIG)
		list_cbc_build_os(cmc);
	else if (cocl->action == DISPLAY_CONFIG)
		retval = display_cbc_build_os(cmc, cocl);
	else if (cocl->action == ADD_CONFIG)
		retval = add_cbc_build_os(cmc, cocl);
	else if (cocl->action == RM_CONFIG)
		retval = remove_cbc_build_os(cmc, cocl);
	else if (cocl->action == DOWNLOAD)
		retval = cbcos_grab_boot_files(cmc, cocl);
	else
		printf("Unknown action type\n");
	if (retval != 0) {
		cbcos_get_os_string(error, cocl);
		cbcos_clean_comm_line(cocl);
		ailsa_clean_cmdb(cmc);
		report_error(retval, error);
	}
	ailsa_clean_cmdb(cmc);
	cbcos_clean_comm_line(cocl);
	exit(retval);
}

static void
cbcos_get_os_string(char *error, cbcos_comm_line_s *cocl)
{
	if (cocl->version) {
		if (cocl->arch)
			snprintf(error, URL_S, "%s %s %s",
			 cocl->os, cocl->version, cocl->arch);
		else
			snprintf(error, URL_S, "%s %s", cocl->os, cocl->version);
	} else {
		snprintf(error, URL_S, "%s", cocl->os);
	}
}

static int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col)
{
	const char *optstr = "ade:fghln:o:rs:t:v";
	int opt;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"display",		no_argument,		NULL,	'd'},
		{"version-alias",	required_argument,	NULL,	'e'},
		{"force",		no_argument,		NULL,	'f'},
		{"download",		no_argument,		NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"list",		no_argument,		NULL,	'l'},
		{"name",		required_argument,	NULL,	'n'},
		{"os",			required_argument,	NULL,	'n'},
		{"os-version",		required_argument,	NULL,	'o'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"alias",		required_argument,	NULL,	's'},
		{"os-alias",		required_argument,	NULL,	's'},
		{"architecture",	required_argument,	NULL,	't'},
		{"os-arch",		required_argument,	NULL,	't'},
		{"version",		no_argument,		NULL,	'v'},
		{NULL,			0,			NULL,	0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a')
			col->action = ADD_CONFIG;
		else if (opt == 'd')
			col->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			col->action = LIST_CONFIG;
		else if (opt == 'r')
			col->action = RM_CONFIG;
		else if (opt == 'v')
			col->action = CVERSION;
		else if (opt == 'g')
			col->action = DOWNLOAD;
		else if (opt == 'h')
			return DISPLAY_USAGE;
		else if (opt == 'e')
			col->ver_alias = strndup(optarg, MAC_S);
		else if (opt == 'f')
			col->force = 1;
		else if (opt == 'n')
			col->os = strndup(optarg, MAC_S);
		else if (opt == 'o')
			col->version = strndup(optarg, MAC_S);
		else if (opt == 's')
			col->alias = strndup(optarg, MAC_S);
		else if (opt == 't')
			col->arch = strndup(optarg, RANGE_S);
		else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		return DISPLAY_USAGE;
	if (col->action == CVERSION)
		return CVERSION;
	if (col->action == 0 && argc != 1) {
		printf("No action provided\n");
		return NO_ACTION;
	}
	if (col->action == ADD_CONFIG && (
		(!(col->version)) || (!(col->os)) || (!(col->arch)))) {
			printf("Some details were not provided\n");
			return DISPLAY_USAGE;
	}
	if ((col->action != LIST_CONFIG && col->action != DOWNLOAD) && 
		!((col->os))) {
		printf("No OS name was provided\n");
		return DISPLAY_USAGE;
	}
	return NONE;
}

static void
list_cbc_build_os(ailsa_cmdb_s *cmc)
{
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *name;
	ailsa_data_s *one;

	if (!(cmc))
		goto cleanup;
	if ((retval = ailsa_basic_query(cmc, BUILD_OS_NAME_TYPE, list)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	name = list->head;
	if (list->total > 0) {
		printf("Operating Systems\n");
	} else {
		ailsa_syslog(LOG_INFO, "No build operating systems were found");
		goto cleanup;
	}
	while (name) {
		one = (ailsa_data_s *)name->data;
		printf("%s\n", one->data->text);
		name = name->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
}

static int
display_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *elem;
	ailsa_data_s *one, *two, *three, *four, *five, *six;
	if (!(cmc) || !(col))
		goto cleanup;
	char *name = col->os;
	char *uname;

	int i = 0;

	if ((retval = ailsa_basic_query(cmc, BUILD_OSES, list)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL query returned %d", retval );
		goto cleanup;
	}
	elem = list->head;
	while (elem) {
		one = (ailsa_data_s *)elem->data;
		if (elem->next)
			elem = elem->next;
		two = (ailsa_data_s *)elem->data;
		if (elem->next)
			elem = elem->next;
		three = (ailsa_data_s *)elem->data;
		if (elem->next)
			elem = elem->next;
		four = (ailsa_data_s *)elem->data;
		if (elem->next)
			elem = elem->next;
		if (elem->next)
			elem = elem->next;
		five = (ailsa_data_s *)elem->data;
		if (elem->next)
			elem = elem->next;
		six = (ailsa_data_s *)elem->data;
		elem = elem->next;
		if (five == six)
			break;
		if (strncasecmp(one->data->text, name, MAC_S) == 0) {
			if (i == 0) {
				printf("Operating System %s\n", name);
				printf("Version\tVersion alias\tArchitecture\tCreated by\tCreation time\n");
			}
			i++;
			if (strncasecmp(three->data->text, "none", COMM_S) == 0) {
				printf("%s\tnone\t\t%s\t\t",
				     two->data->text, four->data->text);
			} else {
				if (strlen(two->data->text) < 8)
					printf("%s\t%s\t\t%s\t\t",
					     two->data->text, three->data->text, four->data->text);
				else
					printf("%s\t%s\t%s\t\t",
					     two->data->text, three->data->text, four->data->text);
			}
			uname = cmdb_get_uname(five->data->number);
			if (strlen(uname) < 8)
				printf("%s\t\t%s\n", uname, six->data->text);
			else
				printf("%s\t%s\n", uname, six->data->text);
			my_free(uname);
		}
	}
	if (i == 0)
		retval =  OS_NOT_FOUND;
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}

static int
add_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	int retval;
	AILLIST *os = ailsa_db_data_list_init();

	if (!(col->ver_alias))
		col->ver_alias = strdup("none");
	if (!(col->alias)) {
		col->alias = ailsa_calloc(SERVICE_LEN, "col->alias in add_cbc_build_os");
		if ((retval = get_os_alias(cmc, col->os, col->alias)) != 0) {
			goto cleanup;
		}
	}
	if ((retval = cmdb_check_for_os(cmc, col->os, col->arch, col->version)) > 0) {
		retval = 0;
		ailsa_syslog(LOG_ERR, "Build OS already in database");
		goto cleanup;
	} else if (retval < 0) {
		goto cleanup;
	}

	if ((retval = cmdb_fill_os_details(cmc, col, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to fill in OS details");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cmc, INSERT_BUILD_OS, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert os %s into database", col->os);
		goto cleanup;
	}
	if ((retval = cbcos_create_os_profile(cmc, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create new OS build profile");
		goto cleanup;
	}
	if ((retval = cbc_get_boot_files(cmc, col->alias, col->version, col->arch, col->ver_alias)) != 0)
		ailsa_syslog(LOG_ERR, "Unable to download boot files\n");

	cleanup:
		ailsa_list_destroy(os);
		my_free(os);
		return retval;
}

static int
remove_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	if (!(cmc) || !(col))
		return AILSA_NO_DATA;
	char **args = ailsa_calloc((sizeof(char *) * 3), "args in remove_cbc_build_os");
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *build = ailsa_db_data_list_init();
	int retval;
	unsigned long int id;
	AILELEM *element;
	ailsa_data_s *data;

	args[0] = col->os;
	args[1] = col->version;
	args[2] = col->arch;
	if ((retval = cmdb_add_os_id_to_list(args, cmc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get OS id");
		retval = OS_NOT_FOUND;
		goto cleanup;
	}
	if (list->total == 0) {
		retval = OS_NOT_FOUND;
		goto cleanup;
	}
	element = list->head;
	data = element->data;
	id = data->data->number;
	if ((retval = check_builds_for_os_id(cmc, id, build)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot check for server builds with this OS");
		goto cleanup;
	}
	if (build->total > 0) {
		display_server_name_for_build_os_id(build);
		if (col->force == 0) {
			ailsa_syslog(LOG_ERR, "Build OS in use. If you want to delete, use -f");
			retval = BUILD_OS_IN_USE;
			goto cleanup;
		}
	}
	if ((retval = ailsa_delete_query(cmc, delete_queries[DELETE_BUILD_OS], list)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_BUILD_OS query failed");
		goto cleanup;
	}
	cleanup:
		ailsa_list_destroy(list);
		ailsa_list_destroy(build);
		my_free(args);
		my_free(list);
		my_free(build);
		return retval;
}

static void
display_server_name_for_build_os_id(AILLIST *list)
{
	if (!(list))
		return;
	size_t i;
	AILELEM *element;
	ailsa_data_s *data;

	element = list->head;

	printf("The following servers are using the build os:\n\t");
	for (i = 0; i < list->total; i++) {
		if (!(element))
			break;
		data = element->data;
		printf(" %s", data->data->text);
		element = element->next;
	}
	printf("\n");
}

static int
cbcos_grab_boot_files(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	int retval = 0;
	int test = 0;
	int count = 0;
	char *os, *alias, *version, *arch, *ver_alias;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *elem = NULL, *head;
	ailsa_data_s *data;

	cbcos_check_for_null_in_comm_line(col, &test);
	if ((retval = ailsa_basic_query(cmc, BUILD_OSES, list)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	elem = list->head;
	if (list->total < 1)
		goto cleanup;
	while (elem) {
		head = elem;
		data = elem->data;
		os = data->data->text;
		if (elem->next)
			elem = elem->next;
		data = elem->data;
		version = data->data->text;
		if (elem->next)
			elem = elem->next;
		data = elem->data;
		alias = data->data->text;
		if (elem->next)
			elem = elem->next;
		data = elem->data;
		arch = data->data->text;
		if (elem->next)
			elem = elem->next;
		data = elem->data;
		ver_alias = data->data->text;
		if (ver_alias == arch)
			break;
		if ((test & 7) == 7) {
			printf("Will download OS %s, version %s, arch %s\n", os, version, arch);
			count++;
			if ((retval = cbc_get_boot_files(cmc, alias, version, arch, ver_alias)) != 0)
				ailsa_syslog(LOG_ERR, "Error downloading OS\n");
		} else {
			cbcos_check_for_os(col, head, &test);
			if ((test & 56) == 56) {
				printf("Will download OS %s, version %s, arch %s\n", os, version, arch);
				count ++;
				if ((retval = cbc_get_boot_files(cmc, alias, version, arch, ver_alias)) != 0)
					ailsa_syslog(LOG_ERR, "Error downloading OS\n");
			}
		}
		if (elem->next)
			elem = elem->next;
		if (elem->next)
			elem = elem->next;
		if (elem->next)
			elem = elem->next;
		test = test & 7;
	}
	if (count == 0)
		ailsa_syslog(LOG_ERR, "No OS found to download\n");
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}

static void
cbcos_check_for_null_in_comm_line(cbcos_comm_line_s *col, int *test)
{
	*test = 0;	// Sanity
/* Set bit fields from command line input */
	if (!(col->arch))
		*test = *test | 1;
	if (!(col->version) && !(col->ver_alias))
		*test = *test | 2;
	if (!(col->os) && !(col->alias))
		*test = *test | 4;
}

static void
cbcos_check_for_os(cbcos_comm_line_s *col, AILELEM *head, int *test)
{
	if (!(col) || !(head) || !(test))
		return;
	AILELEM *elem = head;
	ailsa_data_s *data = elem->data;
	char *os, *alias, *arch, *version, *ver_alias;

	os = data->data->text;
	if (elem->next)
		elem = elem->next;
	data = elem->data;
	version = data->data->text;
	if (elem->next)
		elem = elem->next;
	data = elem->data;
	alias = data->data->text;
	if (elem->next)
		elem = elem->next;
	data = elem->data;
	arch = data->data->text;
	if (elem->next)
		elem = elem->next;
	data = elem->data;
	ver_alias = data->data->text;
	if (ver_alias == arch)
		return;
	if ((*test & 4) == 4) {
		*test = *test | 32;
	} else if (col->os) {
		if (strncasecmp(col->os, os, MAC_S) == 0)
			*test = *test | 32;
	} else if (col->alias) {
		if (strncasecmp(col->alias, alias, MAC_S) == 0)
			*test = *test | 32;
	}
	if ((*test & 1) == 1)
		*test = *test | 8;
	else if (strncasecmp(col->arch, arch, RANGE_S) == 0)
		*test = *test | 8;
	if ((*test & 2) == 2) {
		*test = *test | 16;
	} else if (col->version) {
		if (strncasecmp(col->version, version, MAC_S) == 0)
			*test = *test | 16;
	} else if (col->ver_alias) {
		if (strncasecmp(col->ver_alias, ver_alias, MAC_S) == 0)
			*test = *test | 16;
	}
}

static void
cbcos_clean_comm_line(cbcos_comm_line_s *cl)
{
	if (!(cl))
		return;
	if (cl->alias)
		my_free(cl->alias);
	if (cl->arch)
		my_free(cl->arch);
	if (cl->os)
		my_free(cl->os);
	if (cl->ver_alias)
		my_free(cl->ver_alias);
	if (cl->version)
		my_free(cl->version);
	my_free(cl);
}

static int
cmdb_fill_os_details(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col, AILLIST *os)
{
	int retval;
	if (!(cmc) || !(col) || !(os))
		return AILSA_NO_DATA;

	if ((retval = cmdb_add_string_to_list(col->os, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS name into list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(col->version, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS version into list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(col->alias, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS alias into list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(col->ver_alias, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS version alias into list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(col->arch, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS architecture into list");
		return retval;
	}
	if ((retval = cmdb_add_build_type_id_to_list(col->alias, cmc, os)) != 0)
		return BUILD_TYPE_NOT_FOUND;
	retval = cmdb_populate_cuser_muser(os);
	return retval;
}

static int
cbcos_create_os_profile(ailsa_cmdb_s *cmc, AILLIST *os)
{
	if (!(cmc) || !(os))
		return AILSA_NO_DATA;
	int retval = 0;
	unsigned long int new_os_id;
	void *ptr;
	if (os->total != 8)
		return WRONG_LENGTH_LIST;
	AILLIST *pack = ailsa_db_data_list_init();
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	AILELEM *text = os->head;
	text = text->next->next->next->next;
	AILELEM *id = text->next;
	ailsa_data_s *tmp = id->data;

	if ((retval = cmdb_add_number_to_list(tmp->data->number, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert bt_id into query list in cbcos_create_os_profile");
		goto cleanup;
	}
	tmp = text->data;
	if ((retval = cmdb_add_string_to_list(tmp->data->text, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert arch into query list in cbcos_create_os_profile");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, OS_FROM_BUILD_TYPE_AND_ARCH, list, results)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL query for os_id failed with %d", retval);
		goto cleanup;
	}
	id = results->head;
	tmp = id->data;
	new_os_id = tmp->data->number;
	if ((retval = ailsa_list_remove(results, id, &ptr)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot remove member from os_id list");
		goto cleanup;
	}
	ailsa_clean_data((ailsa_data_s *)ptr);
	if ((retval = ailsa_argument_query(cmc, PACKAGE_DETAIL_ON_OS_ID, results, pack)) != 0) {
		ailsa_syslog(LOG_ERR, "PACKAGE_DETAIL_ON_OS_ID query failed: %d", retval);
		goto cleanup;
	}
	if (pack->total == 0) {
		ailsa_syslog(LOG_ERR, "Unable to get package info for previous varients");
		ailsa_syslog(LOG_ERR, "You will have to manually create the varients for this OS");
		goto cleanup;
	}
	if ((retval = cbcos_insert_os_id_user_into_list(pack, new_os_id)) != 0) {
		ailsa_syslog(LOG_ERR, "Inserting new os_id into list failed");
		goto cleanup;
	}
	if ((retval = ailsa_multiple_query(cmc, insert_queries[INSERT_BUILD_PACKAGE], pack)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert build packages into database: %d", retval);
		goto cleanup;
	}
	cleanup:
		ailsa_list_destroy(pack);
		ailsa_list_destroy(list);
		ailsa_list_destroy(results);
		my_free(pack);
		my_free(list);
		my_free(results);
		return retval;
}

static int
cbcos_insert_os_id_user_into_list(AILLIST *pack, unsigned long int id)
{
	if (!(pack) || (id == 0))
		return AILSA_NO_DATA;
	int retval = 0;
	size_t i, total;
	AILELEM *p = pack->head;
	ailsa_data_s *data;

	total = pack->total / 2;
	p = p->next;
	for (i = 0; i < total; i++) {
		data = ailsa_db_lint_data_init();
		data->data->number = id;
		if ((retval = ailsa_list_ins_next(pack, p, (void *)data)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert os id into list");
			goto cleanup;
		}
		p = p->next;
		if ((retval = ailsa_insert_cuser_muser(pack, p)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert cuser and muser into list");
			goto cleanup;
		}
		if (p->next->next->next)
			p = p->next->next->next->next;
	}
	cleanup:
		return retval;
}

static int
ailsa_insert_cuser_muser(AILLIST *list, AILELEM *elem)
{
	if (!(list) || !(elem))
		return AILSA_NO_DATA;
	int i, retval;
	uid_t uid = getuid();
	ailsa_data_s *data;
	AILELEM *p = elem;
	for (i = 0; i < 2; i++) {
		data = ailsa_db_lint_data_init();
		data->data->number = (unsigned long int)uid;
		if ((retval = ailsa_list_ins_next(list, p, (void *)data)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert user id into list round %d", i);
			return retval;
		}
		p = p->next;
	}
	return retval;
}

