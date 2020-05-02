/*
 *
 *  cbcsysp: Create Build Configuration Partition
 *  Copyright (C) 2014 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcsysp.c
 *
 *  Functions to get configuration values and also parse command line arguments
 *
 *  Part of the cbcsysp program
 *
 */
#define _GNU_SOURCE
#include <config.h>
#include <configmake.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbcsysp.h"

static int
validate_cbcsysp_comm_line(cbc_sysp_s *cbs);

int
main(int argc, char *argv[])
{
	int retval;
	char *config = ailsa_calloc(CONF_S, "config in main");
	ailsa_cmdb_s *cbc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cbc in main");
	cbc_sysp_s *cbs = ailsa_calloc(sizeof(cbc_sysp_s), "cbs in main");

	if ((retval = parse_cbc_sysp_comm_line(argc, argv, cbs)) != 0) {
		clean_cbcsysp_s(cbs);
		free(cbc);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(cbc);
	if (cbs->what == SPACKAGE) {
		if (cbs->action == LIST_CONFIG)
			retval = list_cbc_syspackage(cbc);
		else if (cbs->action == ADD_CONFIG)
			retval = add_cbc_syspackage(cbc, cbs);
		else if (cbs->action == RM_CONFIG)
			retval = rem_cbc_syspackage(cbc, cbs);
		else
			retval = WRONG_ACTION;
	} else if (cbs->what == SPACKARG) {
		if (cbs->action == LIST_CONFIG)
			retval = list_cbc_syspackage_arg(cbc, cbs);
		else if (cbs->action == ADD_CONFIG)
			retval = add_cbc_syspackage_arg(cbc, cbs);
		else if (cbs->action == RM_CONFIG)
			retval = rem_cbc_syspackage_arg(cbc, cbs);
		else
			retval = WRONG_ACTION;
	} else if (cbs->what == SPACKCNF) {
		if (cbs->action == LIST_CONFIG)
			retval = list_cbc_syspackage_conf(cbc, cbs);
		else if (cbs->action == ADD_CONFIG)
			retval = add_cbc_syspackage_conf(cbc, cbs);
		else if  (cbs->action == RM_CONFIG)
			retval = rem_cbc_syspackage_conf(cbc, cbs);
		else
			retval = WRONG_ACTION;
	}
	if (retval == WRONG_ACTION)
		fprintf(stderr, "Action not supported for type\n");
	clean_cbcsysp_s(cbs);
	ailsa_clean_cmdb(cbc);
	free(config);
	if ((retval != 0) && (retval != NO_RECORDS))
		report_error(retval, "");
	return retval;
}

int
parse_cbc_sysp_comm_line(int argc, char *argv[], cbc_sysp_s *cbcs)
{
	const char *optstr = "ab:df:g:hlmn:oprt:vy";
	int retval, opt;
	retval = 0;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"domain",		required_argument,	NULL,	'b'},
		{"build-domain",	required_argument,	NULL,	'b'},
		{"field-no",		required_argument,	NULL,	'f'},
		{"argument",		required_argument,	NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"name",		required_argument,	NULL,	'n'},
		{"package-config",	no_argument,		NULL,	'o'},
		{"package",		no_argument,		NULL,	'p'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"type",		required_argument,	NULL,	't'},
		{"version",		no_argument,		NULL,	'v'},
		{"package-arg",		no_argument,		NULL,	'y'},
		{NULL,			0,			NULL,	0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a')
			cbcs->action = ADD_CONFIG;
		else if (opt == 'l')
			cbcs->action = LIST_CONFIG;
		else if (opt == 'm')
			cbcs->action = MOD_CONFIG;
		else if (opt == 'r')
			cbcs->action = RM_CONFIG;
		else if (opt == 'o')
			cbcs->what = SPACKCNF;
		else if (opt == 'p')
			cbcs->what = SPACKAGE;
		else if (opt == 'y')
			cbcs->what = SPACKARG;
		else if (opt == 'h')
			return DISPLAY_USAGE;
		else if (opt == 'v') {
			cbcs->action = CVERSION;
			retval = CVERSION;
		} else if (opt == 'b') {
			cbcs->domain = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 'f') {
			cbcs->field = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 'g') {
			cbcs->arg = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 'n') {
			cbcs->name = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 't') {
			cbcs->type = strndup(optarg, DOMAIN_LEN);
		} else
			retval = DISPLAY_USAGE;
	}
	if (argc == 1)
		retval = DISPLAY_USAGE;
	if ((retval != DISPLAY_USAGE) && (retval != CVERSION))
		if ((retval = validate_cbcsysp_comm_line(cbcs)) != 0)
			return retval;
	return check_sysp_comm_line_for_errors(cbcs);
}

static int
validate_cbcsysp_comm_line(cbc_sysp_s *cbs)
{
	if (!(cbs))
		return AILSA_NO_DATA;
	int retval = 0;

	if (cbs->domain) {
		if (ailsa_validate_input(cbs->domain, DOMAIN_REGEX)) {
			ailsa_syslog(LOG_ERR, "Domain invalid");
			return USER_INPUT_INVALID;
		}
	}
	if (cbs->field) {
		if (ailsa_validate_input(cbs->field, PACKAGE_FIELD_REGEX)) {
			ailsa_syslog(LOG_ERR, "Field invalid");
			return USER_INPUT_INVALID;
		}
	}
	if (cbs->arg) {
		if (ailsa_validate_input(cbs->arg, DOMAIN_REGEX)) {
			ailsa_syslog(LOG_ERR, "Args invalid");
			return USER_INPUT_INVALID;
		}
	}
	if (cbs->name) {
		if (ailsa_validate_input(cbs->name, DOMAIN_REGEX)) {
			ailsa_syslog(LOG_ERR, "Name invalid");
			return USER_INPUT_INVALID;
		}
	}
	if (cbs->type) {
		if (ailsa_validate_input(cbs->type, DOMAIN_REGEX)) {
			ailsa_syslog(LOG_ERR, "Type invalid");
			return USER_INPUT_INVALID;
		}
	}
	return retval;
}

int
check_sysp_comm_line_for_errors(cbc_sysp_s *cbcs)
{
	int retval = 0;

	if (cbcs->what == 0) {
		fprintf(stderr, "No type specified\n\n");
		retval = DISPLAY_USAGE;
	} else if (cbcs->action == 0) {
		fprintf(stderr, "No action spcified\n\n");
		retval = DISPLAY_USAGE;
	} else if (cbcs->what == SPACKAGE) {
		if (!(cbcs->name) && (cbcs->action != LIST_CONFIG)) {
			fprintf(stderr, "You need a package name!\n\n");
			retval = DISPLAY_USAGE;
		}
	} else if (cbcs->what == SPACKARG) {
		if (cbcs->action != LIST_CONFIG) {
			if ((cbcs->action != RM_CONFIG) && (!(cbcs->type) || !(cbcs->field))) {
				fprintf(stderr, "You need both package field and type\n");
				retval = DISPLAY_USAGE;
			} else if (cbcs->action == RM_CONFIG && !(cbcs->field)) {
				fprintf(stderr,
"You need to provide the field you want to remove\n");
				retval = DISPLAY_USAGE;
			}
		} else if (!(cbcs->name)) {
			fprintf(stderr, "You need a package name!\n\n");
			retval = DISPLAY_USAGE;
		}
	} else if (cbcs->what == SPACKCNF) {
		if (!(cbcs->domain))  {
			fprintf(stderr, "No build domain supplied\n\n");
			retval = DISPLAY_USAGE;
		}
		if ((cbcs->action != LIST_CONFIG) && (!(cbcs->name) || !(cbcs->field))) {
			fprintf(stderr, "Need both package name and field to add or remove config\n\n");
			retval = DISPLAY_USAGE;
		}
	}
	return retval;
}

void
clean_cbcsysp_s(cbc_sysp_s *cbcs)
{
	if (!(cbcs))
		return;
	if (cbcs->arg)
		free(cbcs->arg);
	if (cbcs->domain)
		free(cbcs->domain);
	if (cbcs->field)
		free(cbcs->field);
	if (cbcs->name)
		free(cbcs->name);
	if (cbcs->type)
		free(cbcs->type);
	free(cbcs);
}

// List functions

int
list_cbc_syspackage(ailsa_cmdb_s *cbc)
{
	if (!(cbc))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *sysp = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = ailsa_basic_query(cbc, SYSTEM_PACKAGE_NAMES, sysp)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_PACKAGE_NAMES query failed");
		goto cleanup;
	}
	if (sysp->total == 0) {
		ailsa_syslog(LOG_ERR, "No packages returned");
		goto cleanup;
	}
	e = sysp->head;
	while (e) {
		printf("%s\n", ((ailsa_data_s *)e->data)->data->text);
		e = e->next;
	}

	cleanup:
		ailsa_list_full_clean(sysp);
		return retval;
}

int
list_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *css)
{
	char *package = NULL, *tmp = NULL;
	int retval;
	unsigned int query;
	size_t total = 4;
	AILLIST *pack = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = cmdb_add_string_to_list(css->domain, pack)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain name to list");
		goto cleanup;
	}
	query = SYS_PACK_DETAILS_ON_DOMAIN;
	if (css->name) {
		if ((retval = cmdb_add_string_to_list(css->name, pack)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add package name to list");
			goto cleanup;
		}
		query = SYS_PACK_DETAILS_ON_NAME_DOMAIN;
	}
	if (css->field) {
		if ((retval = cmdb_add_string_to_list(css->field, pack)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add package field to list");
			goto cleanup;
		}
		query = SYS_PACK_DETAILS_MIN;
	}
	if ((retval = ailsa_argument_query(cbc, query, pack, res)) != 0) {
		ailsa_syslog(LOG_ERR, "System package query %u failed", query);
		goto cleanup;
	}
	if (res->total == 0) {
		ailsa_syslog(LOG_INFO, "Build domain %s has no configured packages\n", css->domain);
		retval = NO_RECORDS;
		goto cleanup;
	} else if ((res->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Query returned wrong number of elements?");
		goto cleanup;
	}
	printf("System package config for build domain %s\n", css->domain);
	e = res->head;
	while (e) {
		if (!(package)) {
			package = ((ailsa_data_s *)e->data)->data->text;
			printf("\n%s\n", package);
		} else if (strncmp(package, ((ailsa_data_s *)e->data)->data->text, DOMAIN_LEN) != 0) {
			package = ((ailsa_data_s *)e->data)->data->text;
			printf("\n%s\n", package);
		}
		tmp = ((ailsa_data_s *)e->next->data)->data->text;
		if (strlen(tmp) > 23)
			printf("\t%s\t%s\t%s\n", tmp, ((ailsa_data_s *)e->next->next->data)->data->text,
			  ((ailsa_data_s *)e->next->next->next->data)->data->text);
		else if (strlen(tmp) > 15)
			printf("\t%s\t\t%s\t%s\n", tmp, ((ailsa_data_s *)e->next->next->data)->data->text,
			  ((ailsa_data_s *)e->next->next->next->data)->data->text);
		else
			printf("\t%s\t\t\t%s\t%s\n", tmp, ((ailsa_data_s *)e->next->next->data)->data->text,
			  ((ailsa_data_s *)e->next->next->next->data)->data->text);
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		ailsa_list_full_clean(pack);
		ailsa_list_full_clean(res);
		return retval;
}

int
list_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *css)
{
	if (!(cbc) || !(css))
		return AILSA_NO_DATA;
	int retval;
	size_t total = 2;
	size_t len;
	char *package = css->name;
	char *type, *field;
	AILLIST *pack = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = cmdb_add_string_to_list(package, pack)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add package name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYS_PACK_ARGS_ON_NAME, pack, res)) != 0) {
		ailsa_syslog(LOG_ERR, "SYS_PACK_ARGS_ON_NAME query failed");
		goto cleanup;
	}
	if (res->total == 0) {
		ailsa_syslog(LOG_INFO, "No arguments configured for package %s", package);
		goto cleanup;
	}
	if ((res->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Query returned wrong number of elements?");
		goto cleanup;
	}
	e = res->head;
	printf("Arguments available for package %s\n", package);
	printf("Argument\t\t\t\tType\n");
	while (e) {
		field = ((ailsa_data_s *)e->data)->data->text;
		type = ((ailsa_data_s *)e->next->data)->data->text;
		len = strlen(field);
		if (len > 39)
			printf("%s\n\t\t\t\t%s\n", field, type);
		else if (len > 31)
			printf("%s\t%s\n", field, type);
		else if (len > 23)
			printf("%s\t\t%s\n", field, type);
		else if (len > 15)
			printf("%s\t\t\t%s\n", field, type);
		else if (len > 7)
			printf("%s\t\t\t\t%s\n", field, type);
		else
			printf("%s\t\t\t\t\t%s\n", field, type);
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		ailsa_list_full_clean(pack);
		ailsa_list_full_clean(res);
		return retval;
}

// Add functions

int
add_cbc_syspackage(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_syspack_s *spack;

	initialise_cbc_s(&cbs);
	initialise_cbc_syspack(&spack);
	cbs->syspack = spack;
	pack_syspack(spack, cbcs);
	if ((retval = cbc_run_insert(cbc, cbs, SYSPACKS)) != 0)
		fprintf(stderr, "Cannot insert system package into DB\n");
	else
		printf("Package %s inserted into db\n", spack->name);
	clean_cbc_struct(cbs);
	return retval;
}

int
add_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0;
	unsigned long int spack_id;
	cbc_s *cbs;
	cbc_syspack_arg_s *cpsa;

	if (!(cbc) || !(cbcs))
		return CBC_NO_DATA;
	if ((retval = get_system_package_id(cbc, cbcs->name, &spack_id)) != 0)
		return retval;
	initialise_cbc_s(&cbs);
	initialise_cbc_syspack_arg(&cpsa);
	cbs->sysarg = cpsa;
	cpsa->syspack_id = spack_id;
	pack_sysarg(cpsa, cbcs);
	if ((retval = cbc_run_insert(cbc, cbs, SYSARGS)) != 0)
		fprintf(stderr, "Cannot insert system package into DB\n");
	else
		printf("Package args for package %s inserted into DB\n", cbcs->name);
	clean_cbc_struct(cbs);
	return retval;
}

int
add_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0;
	unsigned long int spack_id, sconf_id, bd_id;
	cbc_s *cbs;
	cbc_syspack_conf_s *cpsc;

	if (!(cbc) || !(cbcs))
		return CBC_NO_DATA;
	if ((retval = get_system_package_id(cbc, cbcs->name, &spack_id)) != 0)
		return retval;
	if ((retval = get_build_domain_id(cbc, cbcs->domain, &bd_id)) != 0)
		return retval;
	if ((retval = get_syspack_arg_id(cbc, cbcs->field, spack_id, &sconf_id)) != 0)
		return retval;
	initialise_cbc_s(&cbs);
	initialise_cbc_syspack_conf(&cpsc);
	cbs->sysconf = cpsc;
	cpsc->syspack_id = spack_id;
	cpsc->syspack_arg_id = sconf_id;
	cpsc->bd_id = bd_id;
	pack_sysconf(cpsc, cbcs);
	if ((retval = cbc_run_insert(cbc, cbs, SYSCONFS)) != 0)
		fprintf(stderr, "Cannot insert syspack config into DB\n");
	else
		printf("Package config for package %s, domain %s inserted into DB\n",
		 cbcs->name, cbcs->domain);
	clean_cbc_struct(cbs);
	return retval;
}

// Remove functions

int
rem_cbc_syspackage(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0, query = SYSP_PACKAGE;
	unsigned int args = cbc_delete_args[query];
	dbdata_s *data = 0;
	
	if (!(cbc) || !(cbcs))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, args);
	if ((retval = get_system_package_id(cbc, cbcs->name, &(data->args.number))) != 0)
		return retval;
	if ((retval = cbc_run_delete(cbc, data, query)) == 0)
		fprintf(stderr, "Unable to delete system package\n");
	else
		printf("Package %s deleted\n", cbcs->name);
	clean_dbdata_struct(data);
	return 0;
}

int
rem_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0, query = SYSP_ARG;
	unsigned int args = cbc_delete_args[query];
	dbdata_s *data = 0;
	unsigned long int spack_id;

	if (!(cbc) || !(cbcs))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, args);
	if ((retval = get_system_package_id(cbc, cbcs->name, &spack_id)) != 0)
		return retval;
	if ((retval = get_syspack_arg_id(cbc, cbcs->field, spack_id, &(data->args.number))) != 0)
		return retval;
	if ((retval = cbc_run_delete(cbc, data, query)) == 0)
		fprintf(stderr, "Unable to delete system package argument\n");
	else
		printf("Package %s, field %s deleted\n", cbcs->name, cbcs->field);
	clean_dbdata_struct(data);
	return 0;
}

int
rem_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0, query = SYS_PACK_CONF_ID;
	unsigned int args = cbc_search_args[query];
	unsigned int fields = cbc_search_fields[query];
	unsigned int max = cmdb_get_max(args, fields);
	dbdata_s *data = 0;

	if (!(cbc) || !(cbcs))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, RBUFF_S, "%s", cbcs->domain);
	snprintf(data->next->args.text, RBUFF_S, "%s", cbcs->name);
	snprintf(data->next->next->args.text, RBUFF_S, "%s", cbcs->field);
	if ((retval = cbc_run_search(cbc, data, SYS_PACK_CONF_ID)) == 0) {
		fprintf(stderr, "Cannot find system package conf\n");
		clean_dbdata_struct(data);
		return NO_SYSPACK_CONF;
	} else if (retval > 1)
		fprintf(stderr, "Multiple system packages? Using 1st\n");
	data->args.number = data->fields.number;
	if ((retval = cbc_run_delete(cbc, data, SYSP_CONF)) == 0) {
		fprintf(stderr, "Cannot remove syspack conf from DB\n");
		retval = DB_DELETE_FAILED;
	} else if (retval == 1) {
		printf("Package config for package %s, domain %s removed from DB\n",
		 cbcs->name, cbcs->domain);
		retval = 0;
	}
	clean_dbdata_struct(data);
	return retval;
}

// Helper functions

void
pack_syspack(cbc_syspack_s *spack, cbc_sysp_s *cbs)
{
	snprintf(spack->name, URL_S, "%s", cbs->name);
	spack->cuser = spack->muser = (unsigned long int)getuid();
}

void
pack_sysarg(cbc_syspack_arg_s *cpsa, cbc_sysp_s *cbs)
{
	snprintf(cpsa->type, MAC_S, "%s", cbs->type);
	snprintf(cpsa->field, URL_S, "%s", cbs->field);
	cpsa->cuser = cpsa->muser = (unsigned long int)getuid();
}

void
pack_sysconf(cbc_syspack_conf_s *cbcs, cbc_sysp_s *cbs)
{
	snprintf(cbcs->arg, RBUFF_S, "%s", cbs->arg);
	cbcs->cuser = cbcs->muser = (unsigned long int)getuid();
}

int
get_syspack_ids(ailsa_cmdb_s *cbc, cbc_sysp_s *css, dbdata_s *data, int query)
{
	int retval = 0;
	if ((retval = get_build_domain_id(cbc, css->domain, &(data->args.number))) != 0)
		return NO_BD_CONFIG;
	if (query != SYSP_INFO_ON_BD_ID) {
		if ((retval = get_system_package_id(cbc, css->name, &(data->next->args.number))) != 0) {
			fprintf(stderr, "Cannot find package %s\n", css->name);
			return NO_BUILD_PACKAGES;
		}
	}
	if (query == SYSP_INFO_SYS_AND_BD_ID) {
		if ((retval = get_syspack_arg_id(cbc, css->field, data->next->args.number,
					&(data->next->next->args.number))) != 0) {
			fprintf(stderr, "Cannot find package field %s\n", css->field);
			return NO_PACKAGE_CONFIG;
		}
	}
	return retval;
}
