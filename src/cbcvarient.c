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
 *  cbcvarient.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcvarient program
 * 
 */
#include <config.h>
#include <configmake.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbc_common.h"


enum {
	CVARIENT = 1,
	CPACKAGE = 2
};

enum {
	NONEV = 0,
	OSV = 1,
	VERSIONV = 2,
	ARCHV = 4
};

typedef struct cbcvari_comm_line_s {
	char *alias;
	char *arch;
	char *os;
	char *ver_alias;
	char *version;
	char *varient;
	char *valias;
	char *package;
	short int action;
	short int type;
} cbcvari_comm_line_s;

const struct ailsa_sql_query_s varient_queries[] = {
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? ORDER BY o.os, o.os_version, o.arch",
	1,
	{ AILSA_DB_LINT }
	},
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? AND (o.os = ? OR o.alias = ?) ORDER BY o.os, o.os_version, o.arch",
	3,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? AND (o.os_version = ? OR o.ver_alias = ?) ORDER BY o.os, o.os_version, o.arch",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? AND (o.os = ? OR o.alias = ?) AND (o.os_version = ? OR o.ver_alias = ?) ORDER BY o.os, o.os_version, o.arch",
	5,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? AND o.arch = ? ORDER BY o.os, o.os_version, o.arch",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? AND (o.os = ? OR o.alias = ?) AND o.arch = ? ORDER BY o.os, o.os_version, o.arch",
	4,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? AND (o.os_version = ? OR o.ver_alias = ?) AND o.arch = ? ORDER BY o.os, o.os_version, o.arch",
	4,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? AND (o.os = ? OR o.alias = ?) AND (o.os_version = ? OR o.ver_alias = ?) AND o.arch = ? ORDER BY o.os, o.os_version, o.arch",
	6,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	}
};

static void
clean_cbcvarient_comm_line(cbcvari_comm_line_s *cvl);

static void
varient_get_display_query(cbcvari_comm_line_s *cvl, AILLIST *list, ailsa_sql_query_s *query);

static int
parse_cbcvarient_comm_line(int argc, char *argv[], cbcvari_comm_line_s *cvl);

static int
list_cbc_build_varient(ailsa_cmdb_s *cmc);

static int
display_cbc_build_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl);

static int
add_cbc_build_varient(ailsa_cmdb_s *cbc, cbcvari_comm_line_s *cvl);

static int
add_cbc_package(ailsa_cmdb_s *cbc, cbcvari_comm_line_s *cvl);

static int
remove_cbc_package(ailsa_cmdb_s *cbc, cbcvari_comm_line_s *cvl);

static int
remove_cbc_build_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl);

static int
cbc_get_os(cbc_build_os_s *os, cbcvari_comm_line_s *cvl, unsigned long int **id);

static int
cbc_get_os_list(cbc_build_os_s *os, cbcvari_comm_line_s *cvl, unsigned long int *id);

static int
build_package_list(ailsa_cmdb_s *cbc, AILLIST *list, char *pack, char *vari, char *os, char *vers, char *arch);

static int
get_build_os_query(AILLIST *list, unsigned int *query, char *os, char *vers, char *arch);

static dbdata_s *
build_rem_pack_list(ailsa_cmdb_s *cbc, unsigned long int *ids, int noids, char *pack);

static void
copy_packages_from_base_varient(ailsa_cmdb_s *cbc, char *varient);

static int
build_copy_package_list(ailsa_cmdb_s *cbc, cbc_s *base, uli_t bid, uli_t id);

static void
add_package_to_list(cbc_s *base, dbdata_s *data, unsigned long int id);

static void
print_varient_details(AILLIST *list);

static int
compare_os_details(char *os, char *ver, char *arch, char *sos, char *sver, char *sarch);

int
main(int argc, char *argv[])
{
	char error[URL_S];
	int retval = NONE;
	ailsa_cmdb_s *cmc;
	cbcvari_comm_line_s *cvcl;

	cmc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmc in cbcvarient main");
	cvcl = ailsa_calloc(sizeof(cbcvari_comm_line_s), "cvcl in cbcvarient main");
	if ((retval = parse_cbcvarient_comm_line(argc, argv, cvcl)) != 0) {
		free(cmc);
		free(cvcl);
		display_command_line_error(retval, argv[0]);
	}
	if (!(cvcl->varient))
		snprintf(error, URL_S, "name %s", cvcl->varient);
	else if (!(cvcl->valias))
		snprintf(error, URL_S, "alias %s", cvcl->valias);
	parse_cmdb_config(cmc);
	if (cvcl->action == LIST_CONFIG)
		retval = list_cbc_build_varient(cmc);
	else if (cvcl->action == DISPLAY_CONFIG)
		retval = display_cbc_build_varient(cmc, cvcl);
	else if (cvcl->action == ADD_CONFIG && cvcl->type == CVARIENT)
		retval = add_cbc_build_varient(cmc, cvcl);
	else if (cvcl->action == ADD_CONFIG && cvcl->type == CPACKAGE)
		retval = add_cbc_package(cmc, cvcl);
	else if (cvcl->action == RM_CONFIG && cvcl->type == CVARIENT)
		retval = remove_cbc_build_varient(cmc, cvcl);
	else if (cvcl->action == RM_CONFIG && cvcl->type == CPACKAGE)
		retval = remove_cbc_package(cmc, cvcl);
	else if (cvcl->action == MOD_CONFIG)
		fprintf(stderr, "Cowardly refusal to modify varients\n");
	else
		printf("Unknown action type\n");
	if (retval == OS_NOT_FOUND) {
		if (cvcl->os)
			snprintf(error, HOST_S, "%s", cvcl->os);
		else if (cvcl->alias)
			snprintf(error, HOST_S, "alias %s", cvcl->alias);
	} else if (retval == NO_RECORDS) {
		goto cleanup;
	}
	cleanup:
		ailsa_clean_cmdb(cmc);
		clean_cbcvarient_comm_line(cvcl);
		if (retval > 0)
			report_error(retval, error);
		exit(retval);
}

static void
clean_cbcvarient_comm_line(cbcvari_comm_line_s *cvl)
{
	if (!(cvl))
		return;
	if (cvl->alias)
		my_free(cvl->alias);
	if (cvl->arch)
		my_free(cvl->arch);
	if (cvl->os)
		my_free(cvl->os);
	if (cvl->version)
		my_free(cvl->version);
	if (cvl->ver_alias)
		my_free(cvl->ver_alias);
	if (cvl->varient)
		my_free(cvl->varient);
	if (cvl->valias)
		my_free(cvl->valias);
	if (cvl->package)
		my_free(cvl->package);
	my_free(cvl);
}

static int
parse_cbcvarient_comm_line(int argc, char *argv[], cbcvari_comm_line_s *cvl)
{
	const char *optstr = "ade:ghjk:lmn:o:p:rs:t:vx:";
	int opt;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"display",		no_argument,		NULL,	'd'},
		{"version-alias",	required_argument,	NULL,	'e'},
		{"package",		no_argument,		NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"varient",		no_argument,		NULL,	'j'},
		{"varient-alias",	required_argument,	NULL,	'k'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"os-name",		required_argument,	NULL,	'n'},
		{"os-version",		required_argument,	NULL,	'o'},
		{"package-name",	required_argument,	NULL,	'p'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"os-alias",		required_argument,	NULL,	's'},
		{"architecture",	required_argument,	NULL,	't'},
		{"os-arch",		required_argument,	NULL,	't'},
		{"version",		no_argument,		NULL,	'v'},
		{"varient-name",	required_argument,	NULL,	'x'},
		{NULL,			0,			NULL,	0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a')
			cvl->action = ADD_CONFIG;
		else if (opt == 'd') {
			cvl->action = DISPLAY_CONFIG;
			cvl->type = CVARIENT;
		}else if (opt == 'l')
			cvl->action = LIST_CONFIG;
		else if (opt == 'r')
			cvl->action = RM_CONFIG;
		else if (opt == 'm')
			cvl->action = MOD_CONFIG;
		else if (opt == 'v')
			cvl->action = CVERSION;
		else if (opt == 'h')
			return DISPLAY_USAGE;
		else if (opt == 'g')
			cvl->type = CPACKAGE;
		else if (opt == 'j')
			cvl->type = CVARIENT;
		else if (opt == 'e')
			cvl->ver_alias = strndup(optarg, MAC_S);
		else if (opt == 'k')
			cvl->valias = strndup(optarg, MAC_S);
		else if (opt == 'n')
			cvl->os = strndup(optarg, MAC_S);
		else if (opt == 'o')
			cvl->version = strndup(optarg, MAC_S);
		else if (opt == 'p')
			cvl->package = strndup(optarg, HOST_S);
		else if (opt == 's')
			cvl->alias = strndup(optarg, MAC_S);
		else if (opt == 't')
			cvl->arch = strndup(optarg, RANGE_S);
		else if (opt == 'x')
			cvl->varient = strndup(optarg, HOST_S);
		else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cvl->action == CVERSION)
		return CVERSION;
	if (cvl->action == 0 && argc != 1)
		return NO_ACTION;
	if (cvl->type == 0 && cvl->action != LIST_CONFIG)
		return NO_TYPE;
	if (cvl->action != LIST_CONFIG && !(cvl->varient) && !(cvl->valias))
		return NO_VARIENT;
	if ((cvl->action == ADD_CONFIG) && (cvl->type == CVARIENT) && (!(cvl->varient) || !(cvl->valias))) {
		fprintf(stderr, "\
You need to supply both a varient name and valias when adding\n");
		return DISPLAY_USAGE;
	}
	if (cvl->type == CPACKAGE) {
		if (!(cvl->package))
			return NO_PACKAGE;
		if ((cvl->action != ADD_CONFIG) && (cvl->action != RM_CONFIG)) {
			fprintf(stderr, "Can only add or remove packages\n");
			return WRONG_ACTION;
		}
		if (!(cvl->os) && !(cvl->alias))
			return NO_OS_COMM;
	}
	return NONE;
}

static int
list_cbc_build_varient(ailsa_cmdb_s *cmc)
{
	if (!(cmc))
		return AILSA_NO_DATA;
	int retval;
	char *text;
	unsigned long int id;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *element;
	ailsa_data_s *data;

	if ((retval = ailsa_basic_query(cmc, BUILD_VARIENTS, list)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_VARIENTS query failed");
		goto cleanup;
	}
	if (list->total == 0) {
		ailsa_syslog(LOG_INFO, "No build varients found");
		goto cleanup;
	}
	element = list->head;
	printf("Alias\t\tName\t\t\tCreated By\tLast Modified\n");
	while (element) {
		if (element->next)
			element = element->next;
		else
			break;
		data = element->data;
		text = data->data->text;
		if (strlen(text) < 8)
			printf("%s\t\t", text);
		else if (strlen(text) < 16)
			printf("%s\t", text);
		else
			printf("%s\n\t\t\t\t", text);
		if (element->next)
			element = element->next;
		else
			break;
		data = element->data;
		text = data->data->text;
		if (strlen(text) < 8)
			printf("%s\t\t\t", text);
		else if (strlen(text) < 16)
			printf("%s\t\t", text);
		else if (strlen(text) < 24)
			printf("%s\t", text);
		else
			printf("%s\n\t\t\t\t\t", text);
		if (element->next)
			element = element->next;
		else
			break;
		data = element->data;
		id = data->data->number;
		if (strlen(get_uname(id)) < 8)
			printf("%s\t\t", get_uname(id));
		else if (strlen(get_uname(id)) < 16)
			printf("%s\t", get_uname(id));
		else
			printf("%s\n\t\t\t\t\t\t", get_uname(id));
		if (element->next)
			element = element->next;
		else
			break;
		if (element->next)
			element = element->next;
		else
			break;
		if (element->next)
			element = element->next;
		else
			break;
		data = element->data;
#ifdef HAVE_MYSQL
		if (data->type == AILSA_DB_TIME)
			printf("%04u-%02u-%02u %02u:%02u:%02u\n",
				data->data->time->year, data->data->time->month, data->data->time->day,
				data->data->time->hour, data->data->time->minute, data->data->time->second);
		else
#endif
			printf("%s\n", data->data->text);
		element = element->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}

static int
display_cbc_build_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl)
{
  /* If we are given an input here of an OS that does not exist, then
   * we will find no build packages and the error output will say as much.
   * Once we get defined OS types and use build_os to just store versions,
   * we can check if the OS exists and if not, tell the user then */
	if (!(cmc) || !(cvl))
		return AILSA_NO_DATA;
	int retval = NO_VARIENT;
	char *varient;
	ailsa_sql_query_s *query = ailsa_calloc(sizeof(ailsa_sql_query_s), "query in display_cbc_build_varient");
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *v = ailsa_db_data_list_init();

	if ((cvl->varient))
		varient = cvl->varient;
	else if ((cvl->valias))
		varient = cvl->valias;
	else
		goto cleanup;
	if ((retval = cmdb_add_varient_id_to_list(varient, cmc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add varient id to list");
		goto cleanup;
	} else if (list->total > 1) {	// FIXME
		ailsa_syslog(LOG_ERR, "Multiple varients returned");
		goto cleanup;
	}
	varient_get_display_query(cvl, list, query);
	if ((retval = ailsa_individual_query(cmc, query, list, v)) != 0) {
		ailsa_syslog(LOG_ERR, "PACKAGE_DETAILS_FOR_VARIENT query failed");
		goto cleanup;
	}
	if (v->total > 0)
		print_varient_details(v);
	else
		ailsa_syslog(LOG_INFO, "No packages found for build varient!");
	cleanup:
		ailsa_list_destroy(list);
		ailsa_list_destroy(v);
		my_free(list);
		my_free(v);
		my_free(query);
		return retval;
}

static void
varient_get_display_query(cbcvari_comm_line_s *cvl, AILLIST *list, ailsa_sql_query_s *query)
{
	int flag = 0;
	if (!(cvl) || !(list) || !(query))
		return;
	if ((cvl->os) || (cvl->alias)) {
		flag = flag | OSV;
		cmdb_add_os_name_or_alias_to_list(cvl->os, cvl->alias, list);
	}
	if ((cvl->version) || (cvl->ver_alias)) {
		flag = flag | VERSIONV;
		cmdb_add_os_version_or_alias_to_list(cvl->version, cvl->ver_alias, list);
	}
	if (cvl->arch) {
		flag = flag | ARCHV;
		if (cmdb_add_string_to_list(cvl->arch, list) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add arch to list");
	}
	memcpy(query, &varient_queries[flag], sizeof(ailsa_sql_query_s));
}

static int
add_cbc_build_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl)
{
	int retval = NONE;
	cbc_s *base;
	cbc_varient_s *vari, *dbvari;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add_cbc_build_varient");
	if (!(vari = malloc(sizeof(cbc_varient_s))))
		report_error(MALLOC_FAIL, "vari in add_cbc_build_varient");
	init_cbc_struct(base);
	init_varient(vari);
	if ((retval = cbc_run_query(cmc, base, VARIENT)) != 0) {
		if (retval == 6) {
			fprintf(stderr, "No build varients in DB\n");
		} else {
			clean_cbc_struct(base);
			clean_varient(vari);
			return retval;
		}
	}
	dbvari = base->varient;
	while (dbvari) {
		if ((strncmp(dbvari->varient, cvl->varient, MAC_S) == 0) ||
		    (strncmp(dbvari->valias, cvl->valias, MAC_S) == 0)) {
			clean_cbc_struct(base);
			clean_varient(vari);
			return VARIENT_EXISTS;
		}
		dbvari = dbvari->next;
	}
	clean_varient(base->varient);
	snprintf(vari->varient, HOST_S, "%s", cvl->varient);
	snprintf(vari->valias, MAC_S, "%s", cvl->valias);
	base->varient = vari;
	vari->cuser = vari->muser = (unsigned long int)getuid();
	if ((retval = cbc_run_insert(cmc, base, VARIENTS)) != 0) {
		printf("Unable to add varient %s to database\n", cvl->varient);
	} else {
		printf("Varient %s added to database\n", cvl->varient);
	}
	copy_packages_from_base_varient(cmc, cvl->varient);
	clean_cbc_struct(base);
	return retval;
}

static int
remove_cbc_build_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl)
{
	char varient[HOST_S];
	int retval = NONE, type = VARIENT_ID_ON_VALIAS;
	unsigned int max;
	dbdata_s *data = NULL;

	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	if (strncmp(cvl->varient, "NULL", COMM_S) != 0) {
		snprintf(data->args.text, HOST_S, "%s", cvl->varient);
		retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VARIENT);
		if (retval != 1) {
			clean_dbdata_struct(data);
			return VARIENT_NOT_FOUND;
		}
	} else if (strncmp(cvl->valias, "NULL", COMM_S) != 0) {
		snprintf(data->args.text, HOST_S, "%s", cvl->valias);
		retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VALIAS);
		if (retval != 1) {
			clean_dbdata_struct(data);
			return VARIENT_NOT_FOUND;
		}
	}
	snprintf(varient, HOST_S, "%s", data->args.text);
	data->args.number = data->fields.number;
	if ((retval = cbc_run_delete(cmc, data, VARI_DEL_VARI_ID)) != 1) {
		fprintf(stderr, "%d varients deleted for %s\n",
			retval, varient);
		retval = MULTIPLE_VARIENTS;
	} else {
		printf("Varient %s deleted\n", varient);
		retval = NONE;
	}
	free(data);
	return retval;
}

static void
copy_packages_from_base_varient(ailsa_cmdb_s *cbc, char *varient)
{
	char *bvar; 
	int retval, packs = 0;
	unsigned long int vid, bvid;
	cbc_s *base;
	cbc_package_s *pack;

	bvar = ailsa_calloc(COMM_S, "bvar in copy_packages_from_base_varient");
	snprintf(bvar, COMM_S, "base");
	if ((retval = get_varient_id(cbc, bvar, &bvid)) != 0) {
		fprintf(stderr, "Cannot find base varient\n");
		free(bvar);
		return;
	}
	free(bvar);
	if ((retval = get_varient_id(cbc, varient, &vid)) != 0) {
		fprintf(stderr, "Cannot get varient_id for %s\n", varient);
		return;
	}
	initialise_cbc_s(&base);
	if ((retval = build_copy_package_list(cbc, base, bvid, vid)) != 0) {
		fprintf(stderr, "Cannot build package copy list\n");
		clean_cbc_struct(base);
		return;
	}
	pack = base->package;
	while (base->package) {
		if ((retval = cbc_run_insert(cbc, base, BPACKAGES)) == 0)
			packs++;
		base->package = base->package->next;
	}
	printf("Inserted %d packages\n", packs);
	base->package = pack;
	clean_cbc_struct(base);
}

static int
add_cbc_package(ailsa_cmdb_s *cbc, cbcvari_comm_line_s *cvl)
{
	if (!(cbc) || !(cvl))
		return AILSA_NO_DATA;
	int retval = 0;
	char *varient, *os, *version, *arch, *pack;
	AILLIST *list = ailsa_db_data_list_init();

	if (cvl->valias)
		varient = cvl->valias;
	else if (cvl->varient)
		varient = cvl->varient;
	else
		return AILSA_NO_DATA;
	if (cvl->os)
		os = cvl->os;
	else if (cvl->alias)
		os = cvl->alias;
	else
		return AILSA_NO_DATA;
	if (cvl->version)
		version = cvl->version;
	else 
		version = cvl->ver_alias;
	arch = cvl->arch;
	pack = cvl->package;
	if ((retval = build_package_list(cbc, list, pack, varient, os, version, arch)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot build package list to insert");
		goto cleanup;
	}
	if ((retval = ailsa_multiple_insert_query(cbc, INSERT_BUILD_PACKAGE, list)) != 0 )
		ailsa_syslog(LOG_ERR, "Cannot add build package %s to varient %s for os %s\n", pack, varient, os);
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}

static int
remove_cbc_package(ailsa_cmdb_s *cbc, cbcvari_comm_line_s *cvl)
{
	char *varient;
	int retval = 0, os, packs = 0;
	unsigned long int *osid, vid, packid;
	cbc_s *base;
	dbdata_s *data, *list;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in remove_cbc_package");
	init_cbc_struct(base);
	if ((retval = cbc_run_multiple_query(cbc, base, BUILD_OS | VARIENT)) != 0) {
		fprintf(stderr, "Cannot run os and / or varient query\n");
		clean_cbc_struct(base);
		return retval;
	}
	check_for_alias(&varient, cvl->varient, cvl->valias);
	if ((retval = get_varient_id(cbc, varient, &vid)) != 0)
		return retval;
// This will setup the array of osid's in osid if we have more than one version / arch
	if ((os = cbc_get_os(base->bos, cvl, &osid)) == 0) {
		clean_cbc_struct(base);
		return OS_NOT_FOUND;
	}
// Set the last to the varient_id
	*(osid + (unsigned long int)os) = vid;
	if (!(data = build_rem_pack_list(cbc, osid, os, cvl->package))) {
		fprintf(stderr, "No packages to remove\n");
		free(osid);
		clean_cbc_struct(base);
		return NONE;
	}
	list = data;
	while (list) {
		packid = list->fields.number;
		list->args.number = packid;
		if ((retval = cbc_run_delete(cbc, list, PACK_DEL_PACK_ID)) > 1)
			fprintf(stderr, "Multiple packages deleted on pack id %lu\n", packid);
		else if (retval == 0)
			fprintf(stderr, "Cannot delete package id %lu\n", packid);
		packs += retval;
		list = list->next;
	}
	printf("%d packages deleted\n", packs);
	if (packs > 0)
		cbc_set_varient_updated(cbc, vid);
	free(osid);
	clean_cbc_struct(base);
	clean_dbdata_struct(data);
	return 0;
}

static int
cbc_get_os(cbc_build_os_s *os, cbcvari_comm_line_s *cvl, unsigned long int **id)
{
	int retval = NONE;
	unsigned long int *os_id = NULL;

	if ((retval = cbc_get_os_list(os, cvl, os_id)) == 0)
		return retval;
	if (!(os_id = calloc((size_t)retval + 1, sizeof(unsigned long int))))
		report_error(MALLOC_FAIL, "os_id in cbc_get_os");
	retval = cbc_get_os_list(os, cvl, os_id);
	*id = os_id;
	return retval;
}

static int
cbc_get_os_list(cbc_build_os_s *os, cbcvari_comm_line_s *cvl, unsigned long int *id)
{
	int retval = NONE;
	cbc_build_os_s *list;
	unsigned long int *os_id = id;

	list = os;
	while (list) {
		if (strncmp(cvl->version, "NULL", COMM_S) != 0) {
			if (strncmp(cvl->arch, "NULL", COMM_S) != 0) {
				if (((strncasecmp(cvl->alias, list->alias, MAC_S) == 0) ||
                                     (strncasecmp(cvl->os, list->os, MAC_S) == 0)) &&
				     (strncasecmp(cvl->arch, list->arch, RANGE_S) == 0) &&
				     (strncmp(cvl->version, list->version, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			} else {
				if (((strncasecmp(cvl->alias, list->alias, MAC_S) == 0) ||
				     (strncasecmp(cvl->os, list->os, MAC_S) == 0)) &&
				     (strncmp(cvl->version, list->version, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			}
		} else if (strncmp(cvl->ver_alias, "NULL", COMM_S) != 0) {
			if (strncmp(cvl->arch, "NULL", COMM_S) != 0) {
				if (((strncasecmp(cvl->alias, list->alias, MAC_S) == 0) ||
                                     (strncasecmp(cvl->os, list->os, MAC_S) == 0)) &&
				     (strncasecmp(cvl->arch, list->arch, RANGE_S) == 0) &&
				     (strncasecmp(cvl->ver_alias, list->ver_alias, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			} else {
				if (((strncasecmp(cvl->alias, list->alias, MAC_S) == 0) ||
				     (strncasecmp(cvl->os, list->os, MAC_S) == 0)) &&
				     (strncasecmp(cvl->ver_alias, list->ver_alias, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			}
		} else if (strncmp(cvl->arch, "NULL", COMM_S) != 0) {
			if (((strncasecmp(cvl->alias, list->alias, MAC_S) == 0) ||
			     (strncasecmp(cvl->os, list->os, MAC_S) == 0)) &&
			     (strncasecmp(cvl->arch, list->arch, RANGE_S) == 0)) {
				retval++;
				if (id) {
					*os_id = list->os_id;
					os_id++;
				}
			}
		} else {
			if ((strncasecmp(cvl->alias, list->alias, MAC_S) == 0) ||
			    (strncasecmp(cvl->os, list->os, MAC_S) == 0)) {
				retval++;
				if (id) {
					*os_id = list->os_id;
					os_id++;
				}
			}
		}
		list = list->next;
	}
	return retval;
}

static int
build_package_list(ailsa_cmdb_s *cbc, AILLIST *list, char *pack, char *vari, char *os, char *vers, char *arch)
{
	if (!(cbc) || !(list) || !(pack) || !(vari) || !(os))
		return AILSA_NO_DATA;
	AILLIST *build_os = ailsa_db_data_list_init();
	AILLIST *scratch = ailsa_db_data_list_init();
	AILELEM *e;
	unsigned long int vid;
	unsigned int query_no;
	size_t i;
	int retval = 0;

	if ((retval = cmdb_add_varient_id_to_list(vari, cbc, scratch)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get varient id");
		goto cleanup;
	}
	if (scratch->total > 1)
		ailsa_syslog(LOG_INFO, "Multiple varients returned. Using first one returned");
	vid = ((ailsa_data_s *)scratch->head->data)->data->number;
	ailsa_list_destroy(scratch);
	ailsa_list_init(scratch, ailsa_clean_data);
	if ((retval = get_build_os_query(scratch, &query_no, os, vers, arch)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get build_os query");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, query_no, scratch, build_os)) != 0) {
		ailsa_syslog(LOG_ERR, "Build OS query failed");
		goto cleanup;
	}
	e = build_os->head;
	for (i = 0; i < build_os->total; i++) {
		if ((retval = cmdb_add_string_to_list(pack, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert package name into list");
			goto cleanup;
		}
		if ((retval = cmdb_add_number_to_list(vid, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert varient id into list");
			goto cleanup;
		}
		if ((retval = cmdb_add_number_to_list(((ailsa_data_s *)e->data)->data->number, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert os id into list");
			goto cleanup;
		}
		if ((retval = cmdb_populate_cuser_muser(list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
			goto cleanup;
		}
		e = e->next;
	}
	cleanup:
		ailsa_list_destroy(build_os);
		ailsa_list_destroy(scratch);
		my_free(build_os);
		my_free(scratch);
		return retval;
}

static int
get_build_os_query(AILLIST *list, unsigned int *query, char *os, char *vers, char *arch)
{
	if (!(list) || !(query) || !(os))
		return AILSA_NO_DATA;
	unsigned int offset = 0;
	int retval;

	if (vers)
		offset = offset | 1;
	if (arch)
		offset = offset | 2;
	if ((retval = cmdb_add_string_to_list(os, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS name into list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(os, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS alias into list");
		return retval;
	}
	if (offset & 1) {
		if ((retval = cmdb_add_string_to_list(vers, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert OS version into list");
			return retval;
		}
		if ((retval = cmdb_add_string_to_list(vers, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert OS version alias into list");
			return retval;
		}
	}
	if (offset & 2) {
		if ((retval = cmdb_add_string_to_list(arch, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert OS arch into list");
			return retval;
		}
	}
	*query = offset + BUILD_OS_ON_NAME_OR_ALIAS;
	return retval;
}

static dbdata_s *
build_rem_pack_list(ailsa_cmdb_s *cbc, unsigned long int *ids, int noids, char *pack)
{
	int retval, i, query = PACK_ID_ON_DETAILS;
	unsigned int max;
	unsigned long int vid, *id_list;
	dbdata_s *data, *list = NULL, *dlist;
	if (!(ids))
		return list;
	if (!(pack))
		return list;
	id_list = ids;
	vid = *(ids + (unsigned long)noids);
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	for (i = 0; i < noids; i++) {
		init_multi_dbdata_struct(&data, max);
		snprintf(data->args.text, RBUFF_S, "%s", pack);
		data->next->args.number = vid;
		data->next->next->args.number = *id_list;
		if ((retval = cbc_run_search(cbc, data, query)) > 0) {
			dlist = list;
			if (dlist) {
				while (dlist->next)
					dlist = dlist->next;
				dlist->next = data;
			} else {
				list = data;
			}
			if (retval < 3) {
				if (retval > 1) {
					clean_dbdata_struct(data->next->next);
					data->next->next = NULL;
				} else {
					clean_dbdata_struct(data->next);
					data->next = NULL;
				}
			}
		} else {
			clean_dbdata_struct(data);
		}
		id_list++;
	}
	return list;
}

static int
build_copy_package_list(ailsa_cmdb_s *cbc, cbc_s *base, uli_t bid, uli_t id)
{
	int retval, query = PACKAGE_OS_ID_ON_VID;
	unsigned int max;
	dbdata_s *data, *list;

	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = bid;
	if ((retval = cbc_run_search(cbc, data, query)) > 0) {
		list = data;
		while (list) {
			add_package_to_list(base, list, id);
			list = move_down_list_data(list, max);
		}
	}
	clean_dbdata_struct(data);
	if (retval > 0)
		return 0;
	else
		return 1;
}

static void
add_package_to_list(cbc_s *base, dbdata_s *data, unsigned long int id)
{
	cbc_package_s *pack, *list;

	initialise_cbc_package_s(&pack);
	snprintf(pack->package, HOST_S, "%s", data->fields.text);
	pack->os_id = data->next->fields.number;
	pack->vari_id = id;
	pack->muser = pack->cuser = (unsigned long int)getuid();
	if (!(base->package)) {
		base->package = pack;
	} else {
		list = base->package;
		while (list->next)
			list = list->next;
		list->next = pack;
	}
}

static void
print_varient_details(AILLIST *list)
{
	if (!(list))
		return;
	if (list->total % 4 != 0)
		return;
	size_t i, n;
	char *pack, *os, *version, *arch;
	char *sav_os, *sav_ver, *sav_arch;

	pack = cmdb_get_string_from_data_list(list, 1);
	sav_os = os = cmdb_get_string_from_data_list(list, 2);
	sav_ver = version = cmdb_get_string_from_data_list(list, 3);
	sav_arch = arch = cmdb_get_string_from_data_list(list, 4);
	if (!(pack) || !(os) || !(version) || !(arch)) {
		ailsa_syslog(LOG_ERR, "Cannot get package details in print_varient_details");
		return;
	}
	for (i = 0; i < list->total / 4; i++) {
		if (i == 0) {
			printf("OS: %s, Version: %s, arch %s\n", os, version, arch);
			printf("  %s", pack);
		} else {
			if (compare_os_details(os, version, arch, sav_os, sav_ver, sav_arch) == 0) {
				printf(" %s", pack);
			} else {
				printf("\n\nOS: %s, Version: %s, arch %s\n", os, version, arch);
				printf("  %s", pack);
				sav_os = os;
				sav_ver = version;
				sav_arch = arch;
			}
		}
		n = (i + 1) * 4;
		pack = cmdb_get_string_from_data_list(list, 1 + n);
		os = cmdb_get_string_from_data_list(list, 2 + n);
		version = cmdb_get_string_from_data_list(list, 3 + n);
		arch = cmdb_get_string_from_data_list(list, 4 + n);
	}
	puts("\n");
}

static int
compare_os_details(char *os, char *ver, char *arch, char *sos, char *sver, char *sarch)
{
	int retval = -1;
	if (!(os) || !(ver) || !(arch) || !(sos) || !(sver) || !(sarch))
		return retval;
	retval = 0;
	if (strcmp(os, sos) != 0)
		retval = retval | 1;
	if (strcmp(ver, sver) != 0)
		retval = retval | 2;
	if (strcmp(arch, sarch) != 0)
		retval = retval | 4;
	return retval;
}

