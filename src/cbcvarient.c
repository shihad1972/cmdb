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
 *  Static code for the cbcvarient program.
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

const struct ailsa_sql_query_s package_deletes[] = {
	{ // RM_PACKAGE_ON_OS
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE alias = ? OR os = ?)",
	3,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RM_PACKAGE_ON_OS_VERSION
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE (alias = ? OR os = ?) AND (os_version = ? OR ver_alias = ?))",
	5,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RM_PACKAGE_ON_OS_ARCH
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE (alias = ? OR os = ?) AND arch = ?",
	4,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RM_PACKAGE_ON_OS_VERSION_ARCH
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE (alias = ? OR os = ?) AND (os_version = ? OR ver_alias = ?) AND arch = ?)",
	6,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RM_PACKAGE_ON_OS_VARIENT
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE alias = ? OR os = ?) AND varient_id IN (SELECT varient_id FROM varient WHERE varient = ? OR valias = ?)",
	5,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RM_PACKAGE_ON_OS_VERSION_VARIENT
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE (alias = ? OR os = ?) AND (os_version = ? OR ver_alias = ?)) AND varient_id IN (SELECT varient_id FROM varient WHERE varient = ? OR valias = ?)",
	7,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RM_PACKAGE_ON_OS_ARCH_VARIENT
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE (alias = ? OR os = ?) AND arch = ? AND varient_id IN (SELECT varient_id FROM varient WHERE varient = ? OR valias = ?)",
	6,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RM_PACKAGE_ON_OS_VERSION_ARCH_VARIENT
"DELETE FROM packages WHERE package = ? AND os_id IN (SELECT os_id FROM build_os WHERE (alias = ? OR os = ?) AND (os_version = ? OR ver_alias = ?) AND arch = ?) AND varient_id IN (SELECT varient_id FROM varient WHERE varient = ? OR valias = ?)",
	8,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
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
build_package_list(ailsa_cmdb_s *cbc, AILLIST *list, char *pack, char *vari, char *os, char *vers, char *arch);

static int
check_package_list(ailsa_cmdb_s *cbc, AILLIST *list);

static int
get_build_os_query(AILLIST *list, unsigned int *query, char *os, char *vers, char *arch);

static int
get_delete_package_query(AILLIST *list, unsigned int *query, char *pack, char *os, char *vers, char *arch, char *vari);

static void
print_varient_details(AILLIST *list);

static int
compare_os_details(char *os, char *ver, char *arch, char *sos, char *sver, char *sarch);

static int
set_default_cbc_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl);

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
		clean_cbcvarient_comm_line(cvcl);
		display_command_line_error(retval, argv[0]);
	}
	if (!(cvcl->varient))
		snprintf(error, URL_S, "alias %s", cvcl->valias);
	else if (!(cvcl->valias))
		snprintf(error, URL_S, "name %s", cvcl->varient);
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
		ailsa_syslog(LOG_ERR, "Cowardly refusal to modify varients\n");
	else if (cvcl->action == SET_DEFAULT)
		retval = set_default_cbc_varient(cmc, cvcl);
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
	const char *optstr = "ade:ghjk:lmn:o:p:rs:t:vx:z";
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
		{"set-default",		no_argument,		NULL,	'z'},
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
		else if (opt == 'z')
			cvl->action = SET_DEFAULT;
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
	if (cvl->action == SET_DEFAULT)
		cvl->type = CVARIENT;
	if (cvl->type == 0 && cvl->action != LIST_CONFIG)
		return NO_TYPE;
	if (cvl->action != LIST_CONFIG && !(cvl->varient) && !(cvl->valias))
		return NO_VARIENT;
	if ((cvl->action == ADD_CONFIG) && (cvl->type == CVARIENT) && (!(cvl->varient) || !(cvl->valias))) {
		ailsa_syslog(LOG_ERR, "You need to supply both a varient name and valias when adding\n");
		return DISPLAY_USAGE;
	}
	if (cvl->type == CPACKAGE) {
		if (!(cvl->package))
			return NO_PACKAGE;
		if ((cvl->action != ADD_CONFIG) && (cvl->action != RM_CONFIG)) {
			ailsa_syslog(LOG_ERR, "Can only add or remove packages\n");
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
	char *text = NULL, *uname = NULL;
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
		uname = cmdb_get_uname(data->data->number);
		if (strlen(uname) < 8)
			printf("%s\t\t", uname);
		else if (strlen(uname) < 16)
			printf("%s\t", uname);
		else
			printf("%s\n\t\t\t\t\t\t", uname);
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
			printf("%s\n", ailsa_convert_mysql_time(data->data->time));
		else
#endif
			printf("%s\n", data->data->text);
		element = element->next;
		if (uname)
			my_free(uname);
	}
	cleanup:
		ailsa_list_full_clean(list);
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
	if ((retval = cmdb_check_add_varient_id_to_list(varient, cmc, list)) != 0)
		goto cleanup;
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
	if (!(cmc) || !(cvl))
		return AILSA_NO_DATA;
	int retval;
	unsigned long int vid;
	size_t i;
	AILELEM *e;
	AILLIST *v = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cvl->varient, v)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add varient to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cvl->valias, v)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add valias to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, VARIENT_ID_ON_VARIANT_OR_VALIAS, v, r)) != 0) {
		ailsa_syslog(LOG_ERR, "VARIENT_ID_ON_VARIANT_OR_VALIAS query failed");
		goto cleanup;
	}
	if (r->total > 0) {
		ailsa_syslog(LOG_INFO, "Varient %s already in the database", cvl->varient);
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(v)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to new varient list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cmc, INSERT_VARIENT, v)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert varient into database");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, VARIENT_ID_ON_VARIANT_OR_VALIAS, v, r)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot find newly inserted varient?");
		goto cleanup;
	}
	vid = ((ailsa_data_s *)r->head->data)->data->number;
	ailsa_list_destroy(r);
	ailsa_list_init(r, ailsa_clean_data);
	ailsa_list_destroy(v);
	ailsa_list_init(v, ailsa_clean_data);
	if ((retval = ailsa_basic_query(cmc, BASE_VARIENT_PACKAGES, r)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to get list of packages for base varient");
		goto cleanup;
	}
	if (r->total == 0) {
		ailsa_syslog(LOG_ERR, "Cannot find any packages for base varient");
		ailsa_syslog(LOG_ERR, "You will have to create a package list from scratch");
		goto cleanup;
	}
	e = r->head;
	for (i = 0; i < (r->total / 2); i++) {
		if (!(e))
			break;
		if ((retval = cmdb_add_string_to_list(((ailsa_data_s *)e->data)->data->text, v)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add package name to list");
			goto cleanup;
		}
		if (e->next)
			e = e->next;
		else
			break;
		if ((retval = cmdb_add_number_to_list(vid, v)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add varient id to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_number_to_list(((ailsa_data_s *)e->data)->data->number, v)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add os id to list");
			goto cleanup;
		}
		if ((retval = cmdb_populate_cuser_muser(v)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
			goto cleanup;
		}
		e = e->next;
	}
	if ((retval = ailsa_multiple_query(cmc, insert_queries[INSERT_BUILD_PACKAGE], v)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert build packages for new varient");
	cleanup:
		ailsa_list_destroy(v);
		ailsa_list_destroy(r);
		my_free(v);
		my_free(r);
		return retval;
}

static int
remove_cbc_build_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl)
{
	int retval = AILSA_NO_DATA;
	if (!(cmc) || !(cvl))
		return retval;
	AILLIST *list = ailsa_db_data_list_init();
	char *varient;

	if (cvl->varient)
		varient = cvl->varient;
	else if (cvl->valias)
		varient = cvl->valias;
	else
		goto cleanup;
	if ((retval = cmdb_add_string_to_list(varient, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add varient to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(varient, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add valias to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cmc, delete_queries[DELETE_VARIENT], list)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot remove varient %s from database", varient);
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
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
	if ((retval = check_package_list(cbc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot check package list");
		goto cleanup;
	}
	if (list->total > 0) {
		if ((retval = ailsa_multiple_query(cbc, insert_queries[INSERT_BUILD_PACKAGE], list)) != 0 )
			ailsa_syslog(LOG_ERR, "Cannot add build package %s to varient %s for os %s\n", pack, varient, os);
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}
static int
remove_cbc_package(ailsa_cmdb_s *cbc, cbcvari_comm_line_s *cvl)
{
	if (!(cbc) || !(cvl))
		return AILSA_NO_DATA;
	int retval = 0;
	char *os;
	char *os_ver;
	char *varient;
	unsigned int query = 0;
	AILLIST *list = ailsa_db_data_list_init();

	if (!(cvl->os))
		os = cvl->alias;
	else
		os = cvl->os;
	if (!(cvl->version))
		os_ver = cvl->ver_alias;
	else
		os_ver = cvl->version;
	if (!(cvl->varient))
		varient = cvl->valias;
	else
		varient = cvl->varient;
	if ((retval = get_delete_package_query(list, &query, cvl->package, os, os_ver, cvl->arch, varient)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get delete package query");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, package_deletes[query], list)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot delete build packages");
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
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

	if ((retval = cmdb_check_add_varient_id_to_list(vari, cbc, scratch)) != 0)
		goto cleanup;
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
check_package_list(ailsa_cmdb_s *cbc, AILLIST *list)
{
	if (!(cbc) || !(list))
		return AILSA_NO_DATA;
	int retval = 0;
	AILLIST *results = ailsa_db_data_list_init();
	AILLIST *pack = ailsa_db_data_list_init();
	AILELEM *elem = list->head;
	AILELEM *head = NULL;
	ailsa_data_s *data = NULL;
	size_t len = 5;
	while (elem) {
		head = elem;
		data = elem->data;
		if ((retval = cmdb_add_string_to_list(data->data->text, pack)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add package name to package list");
			goto cleanup;
		}
		elem = elem->next;
		data = elem->data;
		if ((retval = cmdb_add_number_to_list(data->data->number, pack)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add varient_id to package list");
			goto cleanup;
		}
		elem = elem->next;
		data = elem->data;
		if ((retval = cmdb_add_number_to_list(data->data->number, pack)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add os_id to package list");
			goto cleanup;
		}
		elem = elem->next->next->next;
		if ((retval = ailsa_argument_query(cbc, PACKAGE_FULL, pack, results)) != 0) {
			ailsa_syslog(LOG_ERR, "PACKAGE_FULL query failed");
			goto cleanup;
		}
		if (results->total > 0) {
			if ((retval = ailsa_list_remove_elements(list, head, len)) != 0)
				goto cleanup;
		}
		ailsa_list_destroy(pack);
		ailsa_list_destroy(results);
		ailsa_list_init(pack, ailsa_clean_data);
		ailsa_list_init(results, ailsa_clean_data);
	}
	cleanup:
		ailsa_list_full_clean(results);
		ailsa_list_full_clean(pack);
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

static int
get_delete_package_query(AILLIST *list, unsigned int *query, char *pack, char *os, char *vers, char *arch, char *vari)
{
	if (!(list) || !(query) || !(os))
		return AILSA_NO_DATA;

	unsigned int offset = 0;
	int retval;
	if (vers)
		offset = offset | 1;
	if (arch)
		offset = offset | 2;
	if (vari)
		offset = offset | 4;
	if ((retval = cmdb_add_string_to_list(pack, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert package name into list");
		return retval;
	}
	if ((retval = get_build_os_query(list, query, os, vers, arch)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert os details into list");
		return retval;
	}
	if (offset & 4) {
		if ((retval = cmdb_add_string_to_list(vari, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert varient into list");
			return retval;
		}
		if ((retval = cmdb_add_string_to_list(vari, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert valias into list");
			return retval;
		}
	}
	*query = offset;
	return retval;
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

static int
set_default_cbc_varient(ailsa_cmdb_s *cmc, cbcvari_comm_line_s *cvl)
{
	if (!(cmc) || !(cvl))
		return AILSA_NO_DATA;
	if (!(cvl->varient) && !(cvl->valias))
		return AILSA_NO_DATA;
	AILLIST *varient = ailsa_db_data_list_init();
	AILLIST *def = ailsa_db_data_list_init();
	char *v;
	int retval;

	if (cvl->varient)
		v = cvl->varient;
	else
		v = cvl->valias;
	if ((retval = cmdb_check_add_varient_id_to_list(v, cmc, varient)) != 0)
		goto cleanup;
	if ((retval = ailsa_basic_query(cmc, DEFAULT_VARIENT, def)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_VARIENT query failed");
		goto cleanup;
	}
	if (def->total == 0) {
		if ((retval = cmdb_populate_cuser_muser(varient)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot populate cuser and muser in list");
			goto cleanup;
		}
		if ((retval = ailsa_insert_query(cmc, INSERT_DEFAULT_VARIENT, varient)) != 0) {
			ailsa_syslog(LOG_ERR, "INSERT_DEFAULT_VARIENT query failed");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_number_to_list((unsigned long int)getuid(), varient)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add muser to varient list");
			goto cleanup;
		}
		if ((retval = ailsa_update_query(cmc, update_queries[UPDATE_DEFAULT_VARIENT], varient)) != 0) {
			ailsa_syslog(LOG_ERR, "UPDATE_DEFAULT_VARIENT query failed");
			goto cleanup;
		}
	}
	cleanup:
		ailsa_list_full_clean(varient);
		ailsa_list_full_clean(def);
		return retval;
}
