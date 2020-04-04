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
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbcnet.h"

typedef struct cbcos_comm_line_s {
	char alias[MAC_S];
	char arch[RANGE_S];
	char os[MAC_S];
	char ver_alias[MAC_S];
	char version[MAC_S];
	short int action;
} cbcos_comm_line_s;

static void
init_cbcos_comm_line(cbcos_comm_line_s *col);

static int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col);

static int
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
cbcos_check_for_os(cbcos_comm_line_s *col, cbc_build_os_s *os, int *test);

static void
cbcos_get_os_string(char *error, cbcos_comm_line_s *col);

int
main (int argc, char *argv[])
{
	char error[URL_S], *config;
	int retval = NONE;
	ailsa_cmdb_s *cmc;
	cbcos_comm_line_s *cocl;

	cmc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmc in main");
	cocl = ailsa_calloc(sizeof(cbcos_comm_line_s), "cocl in main");
	config = ailsa_calloc(CONF_S, "config in main");
	init_cbcos_comm_line(cocl);
	memset(error, 0, URL_S);
	if ((retval = parse_cbcos_comm_line(argc, argv, cocl)) != 0) {
		free(config);
		free(cocl);
		free(cmc);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(cmc);
	if (cocl->action == LIST_CONFIG)
		retval = list_cbc_build_os(cmc);
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
		free(cocl);
		report_error(retval, error);
	}
	cmdbd_clean_config(cmc);
	my_free(cmc);
	my_free(cocl);
	my_free(config);
	exit(retval);
}

static void
cbcos_get_os_string(char *error, cbcos_comm_line_s *cocl)
{
	if (strncasecmp(cocl->version, "NULL", COMM_S) != 0) {
		if (strncasecmp (cocl->arch, "NULL", COMM_S))
			snprintf(error, URL_S, "%s %s %s",
			 cocl->os, cocl->version, cocl->arch);
		else
			snprintf(error, URL_S, "%s %s", cocl->os, cocl->version);
	} else {
		snprintf(error, URL_S, "%s", cocl->os);
	}
}

static void
init_cbcos_comm_line(cbcos_comm_line_s *col)
{
	col->action = 0;
	snprintf(col->alias, MAC_S, "NULL");
	snprintf(col->arch, RANGE_S, "NULL");
	snprintf(col->os, MAC_S, "NULL");
	snprintf(col->ver_alias, MAC_S, "NULL");
	snprintf(col->version, MAC_S, "NULL");
}

static int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col)
{
	const char *optstr = "ade:ghlmn:o:rs:t:v";
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
		{"modify",		no_argument,		NULL,	'm'},
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
		else if (opt == 'm')
			col->action = MOD_CONFIG;
		else if (opt == 'v')
			col->action = CVERSION;
		else if (opt == 'g')
			col->action = DOWNLOAD;
		else if (opt == 'h')
			return DISPLAY_USAGE;
		else if (opt == 'e')
			snprintf(col->ver_alias, MAC_S, "%s", optarg);
		else if (opt == 'n')
			snprintf(col->os, MAC_S, "%s", optarg);
		else if (opt == 'o')
			snprintf(col->version, MAC_S, "%s", optarg);
		else if (opt == 's')
			snprintf(col->alias, MAC_S, "%s", optarg);
		else if (opt == 't')
			snprintf(col->arch, RANGE_S, "%s", optarg);
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
		strncasecmp(col->version, "NULL", COMM_S) == 0 ||
		strncasecmp(col->os, "NULL", COMM_S) == 0 ||
		strncasecmp(col->arch, "NULL", COMM_S) == 0)) {
			printf("Some details were not provided\n");
			return DISPLAY_USAGE;
	}
	if ((col->action != LIST_CONFIG && col->action != DOWNLOAD) && 
		(strncasecmp(col->os, "NULL", COMM_S) == 0)) {
		printf("No OS name was provided\n");
		return DISPLAY_USAGE;
	}
	return NONE;
}

static int
list_cbc_build_os(ailsa_cmdb_s *cmc)
{
	char *oalias, *talias;
	int retval = 0;
	cbc_s *base;
	cbc_build_os_s *os;
	cbc_build_type_s *type;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in list_cbc_build_os");
	init_cbc_struct(base);
	if ((retval = cbc_run_multiple_query(cmc, base, BUILD_OS | BUILD_TYPE)) != 0) {
		clean_cbc_struct(base);
		return retval;
	}
	type = base->btype;
	printf("Operating Systems\n");
	while (type) {
		os = base->bos;
		talias = type->alias;
		oalias = os->alias;
		while (os) {
			if (strncasecmp(talias, oalias, MAC_S) != 0) {
				os = os->next;
				oalias = os->alias;
			} else {
				printf("%s\n", os->os);
				os = os->next;
				break;
			}
		}	
		type = type->next;
	}
	clean_cbc_struct(base);
	return retval;
}

static int
display_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	char *name = col->os;
	int retval = 0, i = 0;
	time_t create;
	cbc_s *base;
	cbc_build_os_s *os;

	initialise_cbc_s(&base);
	if ((retval = cbc_run_query(cmc, base, BUILD_OS)) != 0) {
		if (retval == 6) {
			fprintf(stderr, "No build OS's\n");
			clean_cbc_struct(base);
			return retval;
		}
		clean_cbc_struct(base);
		return MY_QUERY_FAIL;
	}
	os = base->bos;
	while (os) {
		if (strncasecmp(os->os, name, MAC_S) == 0) {
			if (i == 0) {
				printf("Operating System %s\n", name);
				printf("Version\tVersion alias\tArchitecture\tCreated by\tCreation time\n");
			}
			i++;
			create = (time_t)os->ctime;
			if (strncasecmp(os->ver_alias, "none", COMM_S) == 0) {
				printf("%s\tnone\t\t%s\t\t",
				     os->version, os->arch);
			} else {
				if (strlen(os->ver_alias) < 8)
					printf("%s\t%s\t\t%s\t\t",
					     os->version, os->ver_alias, os->arch);
				else
					printf("%s\t%s\t%s\t\t",
					     os->version, os->ver_alias, os->arch);
			}
			if (strlen(get_uname(os->cuser < 8)))
				printf("%s\t\t%s", get_uname(os->cuser), ctime(&create));
			else
				printf("%s\t%s", get_uname(os->cuser), ctime(&create));
		}
		os = os->next;
	}
	if (i == 0)
		retval =  OS_NOT_FOUND;
	clean_cbc_struct(base);
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
	cbc_s *cbc = NULL;
	cbc_build_os_s *bos;

	cbcos_check_for_null_in_comm_line(col, &test);
	initialise_cbc_s(&cbc);
	if ((retval = cbc_run_query(cmc, cbc, BUILD_OS)) != 0) {
		fprintf(stderr, "Build os query failed\n");
		goto cleanup;
	}
	bos = cbc->bos;
	cbcos_check_for_null_in_comm_line(col, &test);
	while (bos) {
		if ((test & 7) == 7) {
			printf("Will download OS %s, version %s, arch %s\n",
			 bos->os, bos->version, bos->arch);
			count++;
			if ((retval = cbc_get_boot_files(cmc, bos->alias, bos->version, bos->arch, bos->ver_alias)) != 0)
				fprintf(stderr, "Error downloading OS\n");
		} else {
			cbcos_check_for_os(col, bos, &test);
			if ((test & 56) == 56) {
				printf("Will download OS %s, version %s, arch %s\n",
				 bos->os, bos->version, bos->arch);
				count ++;
				if ((retval = cbc_get_boot_files(cmc, bos->alias, bos->version, bos->arch, bos->ver_alias)) != 0)
					fprintf(stderr, "Error downloading OS\n");
			}
		}
		bos = bos->next;
		test = test & 7;
	}
	if (count == 0)
		fprintf(stderr, "No OS found to download\n");
	cleanup:
		if (cbc)
			clean_cbc_struct(cbc);
		return retval;
}

static void
cbcos_check_for_null_in_comm_line(cbcos_comm_line_s *col, int *test)
{
	*test = 0;	// Sanity
/* Set bit fields from command line input */
	if (strncasecmp(col->arch, "NULL", RANGE_S) == 0)
		*test = *test | 1;
	if ((strncasecmp(col->version, "NULL", MAC_S) == 0) &&
	    (strncasecmp(col->ver_alias, "NULL", MAC_S) == 0))
		*test = *test | 2;
	if ((strncasecmp(col->os, "NULL", MAC_S) == 0) &&
	    (strncasecmp(col->alias, "NULL", MAC_S) == 0))
		*test = *test | 4;
}

static void
cbcos_check_for_os(cbcos_comm_line_s *col, cbc_build_os_s *cbos, int *test)
{
	if (((*test & 1) == 1 ) || (strncasecmp(col->arch, cbos->arch, RANGE_S) == 0))
		*test = *test | 8;
	if (((*test & 2) == 2) ||
	    (strncasecmp(col->version, cbos->version, MAC_S) == 0) ||
	    (strncasecmp(col->ver_alias, cbos->ver_alias, MAC_S) == 0))
		*test = *test | 16;
	if (((*test & 4) == 4) ||
	    (strncasecmp(col->os, cbos->os, MAC_S) == 0) ||
	    (strncasecmp(col->alias, cbos->alias, MAC_S) == 0))
		*test = *test | 32;
}

