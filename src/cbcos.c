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
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbcnet.h"

typedef struct cbcos_comm_line_s {
	char *alias;
	char *arch;
	char *os;
	char *ver_alias;
	char *version;
	short int action;
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

static int
check_for_build_os_in_use(ailsa_cmdb_s *cbc, unsigned long int os_id);

static void
copy_new_os_profile(ailsa_cmdb_s *cmc, char *oss[]);

static int
cbc_choose_os_to_copy(ailsa_cmdb_s *cbc, uli_t *id, char *oss[]);

static void
copy_new_build_os(ailsa_cmdb_s *cbc, uli_t *id);
/*
static void
copy_locale_for_os(ailsa_cmdb_s *cbc, uli_t *id); */

static void
copy_packages_for_os(ailsa_cmdb_s *cbc, uli_t *id);

static void
cbcos_check_for_null_in_comm_line(cbcos_comm_line_s *col, int *test);

static void
cbcos_check_for_os(cbcos_comm_line_s *col, AILELEM *head, int *test);

static void
cbcos_get_os_string(char *error, cbcos_comm_line_s *col);

static void
cbcos_clean_comm_line(cbcos_comm_line_s *cl);

int
main (int argc, char *argv[])
{
	char error[URL_S];
	int retval = NONE;
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
	else if (cocl->action == MOD_CONFIG)
		printf("Cowardly refusal to modify Operating Systems\n");
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
	const char *optstr = "ade:ghln:o:rs:t:v";
	int opt;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"display",		no_argument,		NULL,	'd'},
		{"version-alias",	required_argument,	NULL,	'e'},
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
			if (strlen(get_uname(five->data->number)) < 8)
				printf("%s\t\t%s\n", get_uname(five->data->number), six->data->text);
			else
				printf("%s\t%s\n", get_uname(five->data->number), six->data->text);
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
	char *oss[3];
	int retval = NONE;
	unsigned long int id;
	cbc_s *cbc;
	cbc_build_os_s *os;
	dbdata_s *data = NULL;

	initialise_cbc_s(&cbc);
	initialise_cbc_os_s(&os);
	cbc->bos = os;
	if (strncasecmp(col->ver_alias, "NULL", COMM_S) == 0)
		snprintf(col->ver_alias, COMM_S, "none");
	if (strncasecmp(col->alias, "NULL", MAC_S) == 0) {
		if ((retval = get_os_alias(cmc, col->os, col->alias)) != 0) {
			clean_cbc_struct(cbc);
			return OS_NOT_FOUND;
		}
	}
	if ((retval = get_build_type_id(cmc, col->alias, &(os->bt_id))) != 0) {
		clean_cbc_struct(cbc);
		return BUILD_TYPE_NOT_FOUND;
	}
/* Check to make sure this OS not already in DB */
	oss[0] = col->arch;
	oss[1] = col->version;
	oss[2] = col->os;
	if ((retval = get_os_id(cmc, oss, &id)) != OS_NOT_FOUND) {
		fprintf(stderr, "OS %s already in database\n", col->os);
		clean_cbc_struct(cbc);
		return BUILD_OS_EXISTS;
	}
	snprintf(os->alias, MAC_S, "%s", col->alias);
	snprintf(os->os, MAC_S, "%s", col->os);
	snprintf(os->version, MAC_S, "%s", col->version);
	snprintf(os->ver_alias, MAC_S, "%s", col->ver_alias);
	snprintf(os->arch, RANGE_S, "%s", col->arch);
	os->cuser = os->muser = (unsigned long int)getuid();
	if ((retval = cbc_run_insert(cmc, cbc, BUILD_OSS)) != 0)
		printf("Unable to add build os to database\n");
	else
		printf("Build os added to database\n");
	copy_new_os_profile(cmc, oss);
	if ((retval = cbc_get_boot_files(cmc, col->alias, col->version, col->arch, col->ver_alias)) != 0)
		fprintf(stderr, "Unable to download boot files\n");
	clean_dbdata_struct(data);
	clean_cbc_struct(cbc);
	return retval;
}

static int
remove_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	if (!(cmc) || !(col))
		return CBC_NO_DATA;
	char *name = col->os;
	char *version = col->version, *arch = col->arch;
	char *oss[3];
	int retval = NONE;
	unsigned long int id;
	dbdata_s *data = NULL;

	oss[0] = col->arch;
	oss[1] = col->version;
	oss[2] = col->os;
	if ((retval = get_os_id(cmc, oss, &id)) != 0)
		return OS_NOT_FOUND;
	if ((retval = check_for_build_os_in_use(cmc, id)) != 0)
		return BUILD_OS_IN_USE;
	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in remove_cbc_build_os");
	data->next = NULL;
	data->args.number = id;
	if ((retval = cbc_run_delete(cmc, data, BOS_DEL_BOS_ID)) != 1) {
		fprintf(stderr, "%d oses deleted for %s %s %s\n",
			retval, name, version, arch);
		retval = MULTIPLE_OS;
	} else {
		printf("OS %s %s %s deleted from db\n", name, version, arch);
		retval = NONE;
	}
	clean_dbdata_struct(data);
	return retval;
}

static int
check_for_build_os_in_use(ailsa_cmdb_s *cbc, unsigned long int os_id)
{
	int retval, query = BUILD_ID_ON_OS_ID, i;
	char *name;
	unsigned int max;
	dbdata_s *data, *list;

	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = os_id;
	if ((retval = cbc_run_search(cbc, data, query)) != 0) {
		clean_dbdata_struct(data->next);
		data->next = NULL;
		memset(data->fields.text, 0, RBUFF_S);
		retval = cbc_run_search(cbc, data, SERVERS_USING_BUILD_OS);
		printf("%d server(s) ", retval);
		list = data;
		for (i = 0; i < retval; i++) {
			name = list->fields.text;
			if (i + 1 == retval)
				printf("%s ", name);
			else
				printf("%s, ", name);
			if (list->next)
				list = list->next;
			else
				break;
		}
		printf("are using build os.\n");
	}
	clean_dbdata_struct(data);
	return retval;
}

static void
copy_new_os_profile(ailsa_cmdb_s *cmc, char *oss[])
{
	char alias[RANGE_S];
	int retval;
	unsigned long int id[3]; //os_id = 0, bt_id = 1, id to copy from = 2

	memset(id, 0, sizeof(id));
	if ((retval = get_os_id(cmc, oss, &id[0])) != 0) {
		fprintf(stderr, "Cannot find OS? %s, %s, %s\n", oss[0], oss[1], oss[2]);
		return;
	}
	if ((retval = get_os_alias(cmc, oss[2], alias)) != 0) {
		fprintf(stderr, "Cannot find alias for os %s\n", oss[2]);
		return;
	}
	if ((retval = get_build_type_id(cmc, alias, &id[1])) != 0) {
		fprintf(stderr, "Cannot find build type for alias %s\n", alias);
		return;
	}
	if ((retval = cbc_choose_os_to_copy(cmc, id, oss)) != 0) {
		fprintf(stderr, "Cannot choose an os to copy??\n");
		return;
	}
	copy_new_build_os(cmc, id);
	return;
}

static int
cbc_choose_os_to_copy(ailsa_cmdb_s *cbc, uli_t *id, char *oss[])
{
	if (!(cbc) || !(oss) || !(id))
		return CBC_NO_DATA;
	char *arch = oss[0];
	int retval = 0, query = OS_DETAIL_ON_BT_ID;
	unsigned int max;
	unsigned long int time = 0, ctime;
	dbdata_s *data, *list;

	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = *(id + 1);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find any previous os for %s\n", oss[2]);
		clean_dbdata_struct(data);
		return OS_NOT_FOUND;
	}
	if (check_data_length(data, max) != 0) {
		clean_dbdata_struct(data);
		return CBC_DATA_WRONG_COUNT;
	}
	list = data;
	while (data) {
		if ((strncasecmp(arch, data->next->next->fields.text, RBUFF_S) == 0) &&
		    (data->fields.number != id[0])) {
			convert_time(data->next->fields.text, &ctime);
			if (ctime > time) {
				id[2] = data->fields.number;
				time = ctime; 
			}
		}
		data = move_down_list_data(data, max);
	}
	clean_dbdata_struct(list);
	if (id[2] == 0)
		return 1;
	else
		return 0;
}

static void
copy_new_build_os(ailsa_cmdb_s *cbc, uli_t *id)
{
	if (!(cbc) || !(id)) {
		fprintf(stderr, "No data passed to copy_new_build_os\nNo Copy performed\n");
		return;
	}
	copy_packages_for_os(cbc, id);
}

static void
copy_packages_for_os(ailsa_cmdb_s *cbc, uli_t *id)
{
	int retval = 0, query = PACKAGE_VID_ON_OS_ID, i = 0;
	unsigned int max;
	dbdata_s *data, *dlist;
	cbc_s *base;
	cbc_package_s *pack, *plist = NULL, *next;

	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = id[2];
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "No packages for OS?\n");
		clean_dbdata_struct(data);
		return;
	}
	dlist = data;
	if (check_data_length(dlist, max) != 0) {
		fprintf(stderr, "dbdata count wrong in copy_packages_for_os");
		clean_dbdata_struct(data);
		return;
	}
	initialise_cbc_s(&base);
	data = dlist;
	while (data) {
		initialise_cbc_package_s(&pack);
		snprintf(pack->package, HOST_S, "%s", data->fields.text);
		pack->vari_id = data->next->fields.number;
		pack->os_id = id[0];
		pack->cuser = pack->muser = (unsigned long int)getuid();
		if (!(plist)) {
			plist = pack;
		} else {
			next = plist;
			while (next->next)
				next = next->next;
			next->next = pack;
		}
		data = move_down_list_data(data, max);
	}
	pack = plist;
	clean_dbdata_struct(dlist);
	while (pack) {
		base->package = pack;
		if ((retval = cbc_run_insert(cbc, base, BPACKAGES)) != 0) {
			fprintf(stderr, "Cannot insert package %s\n", pack->package);
		} else {
			i++;
		}
		pack = pack->next;
	}
	printf("%d package(s) inserted\n", i);
	base->package = plist;
	clean_cbc_struct(base);
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
				fprintf(stderr, "Error downloading OS\n");
		} else {
			cbcos_check_for_os(col, head, &test);
			if ((test & 56) == 56) {
				printf("Will download OS %s, version %s, arch %s\n", os, version, arch);
				count ++;
				if ((retval = cbc_get_boot_files(cmc, alias, version, arch, ver_alias)) != 0)
					fprintf(stderr, "Error downloading OS\n");
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
		fprintf(stderr, "No OS found to download\n");
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
