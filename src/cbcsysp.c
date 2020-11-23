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

static int
validate_cbcsysp_comm_line(cbc_sysp_s *cbs);

static void
clean_cbcsysp_s(cbc_sysp_s *cbcs);

static int
parse_cbc_sysp_comm_line(int argc, char *argv[], cbc_sysp_s *cbcs);

static int
check_sysp_comm_line_for_errors(cbc_sysp_s *cbcs);

static int
list_cbc_syspackage(ailsa_cmdb_s *cbc);

static int
list_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *css);

static int
list_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *css);

static int
add_cbc_syspackage(ailsa_cmdb_s *cbc, cbc_sysp_s *cbs);

static int
add_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *cbs);

static int
add_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs);

static int
rem_cbc_syspackage(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs);

static int
rem_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs);

static int
rem_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs);

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
		if (cbs->action == CMDB_LIST)
			retval = list_cbc_syspackage(cbc);
		else if (cbs->action == CMDB_ADD)
			retval = add_cbc_syspackage(cbc, cbs);
		else if (cbs->action == CMDB_RM)
			retval = rem_cbc_syspackage(cbc, cbs);
		else
			retval = WRONG_ACTION;
	} else if (cbs->what == SPACKARG) {
		if (cbs->action == CMDB_LIST)
			retval = list_cbc_syspackage_arg(cbc, cbs);
		else if (cbs->action == CMDB_ADD)
			retval = add_cbc_syspackage_arg(cbc, cbs);
		else if (cbs->action == CMDB_RM)
			retval = rem_cbc_syspackage_arg(cbc, cbs);
		else
			retval = WRONG_ACTION;
	} else if (cbs->what == SPACKCNF) {
		if (cbs->action == CMDB_LIST)
			retval = list_cbc_syspackage_conf(cbc, cbs);
		else if (cbs->action == CMDB_ADD)
			retval = add_cbc_syspackage_conf(cbc, cbs);
		else if  (cbs->action == CMDB_RM)
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
			cbcs->action = CMDB_ADD;
		else if (opt == 'l')
			cbcs->action = CMDB_LIST;
		else if (opt == 'm')
			cbcs->action = CMDB_MOD;
		else if (opt == 'r')
			cbcs->action = CMDB_RM;
		else if (opt == 'o')
			cbcs->what = SPACKCNF;
		else if (opt == 'p')
			cbcs->what = SPACKAGE;
		else if (opt == 'y')
			cbcs->what = SPACKARG;
		else if (opt == 'h')
			return DISPLAY_USAGE;
		else if (opt == 'v') {
			cbcs->action = AILSA_VERSION;
			retval = AILSA_VERSION;
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
	if ((retval != DISPLAY_USAGE) && (retval != AILSA_VERSION))
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
		if (ailsa_validate_input(cbs->arg, SYSTEM_PACKAGE_ARG_REGEX)) {
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
		if (!(cbcs->name) && (cbcs->action != CMDB_LIST)) {
			fprintf(stderr, "You need a package name!\n\n");
			retval = DISPLAY_USAGE;
		}
	} else if (cbcs->what == SPACKARG) {
		if (cbcs->action != CMDB_LIST) {
			if ((cbcs->action != CMDB_RM) && (!(cbcs->type) || !(cbcs->field))) {
				fprintf(stderr, "You need both package field and type\n");
				retval = DISPLAY_USAGE;
			} else if (cbcs->action == CMDB_RM && !(cbcs->field)) {
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
			ailsa_syslog(LOG_ERR, "No build domain supplied");
			return DISPLAY_USAGE;
		}
		if ((cbcs->action != CMDB_LIST) && (!(cbcs->name) || !(cbcs->field))) {
			ailsa_syslog(LOG_ERR, "Need both package name and field to add or remove config");
			return DISPLAY_USAGE;
		}
		if ((cbcs->action != CMDB_LIST) && !(cbcs->arg)) {
			ailsa_syslog(LOG_ERR, "Need a package argument to add or delete a config");
			return DISPLAY_USAGE;
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
	if (!(cbc) || !(cbcs))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *pack = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cbcs->name, pack)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add package name to list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(pack)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_SYSTEM_PACKAGE, pack)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_SYSTEM_PACKAGE query failed");
		
	cleanup:
		ailsa_list_full_clean(pack);
		return retval;
}

int
add_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *cbcs)
{
	if (!(cbc) || !(cbcs))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cbcs->field, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add field to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cbcs->type, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add type to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_sys_pack_id_to_list(cbcs->name, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add package id to list");
		goto cleanup;
	}
	if (l->total != 3) {
		ailsa_syslog(LOG_ERR, "List has wrong number? Should be 3, got %u", l->total);
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_SYSTEM_PACKAGE_ARGS, l)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_SYSTEM_PACKAGE_ARGS query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
add_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *cbs)
{
	if (!(cbc) || !(cbs))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	char **args = ailsa_calloc((sizeof(char *) * 2), "args in add_cbc_syspackage_conf");

	args[0] = cbs->name;
	args[1] = cbs->field;
	if ((retval = cmdb_add_string_to_list(cbs->arg, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add arg to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_sys_pack_id_to_list(cbs->name, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add system package id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_sys_pack_arg_id_to_list(args, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add system package arg id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_build_domain_id_to_list(cbs->domain, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain id to list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_SYSTEM_PACKAGE_CONF, l)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_SYSTEM_PACKAGE_CONF query failed");

	cleanup:
		my_free(args);
		ailsa_list_full_clean(l);
		return retval;
}

// Remove functions

int
rem_cbc_syspackage(ailsa_cmdb_s *cbc, cbc_sysp_s *cbs)
{
	if (!(cbc) || !(cbs))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cbs->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add package name to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_SYSTEM_PACKAGE], l)) != 0)
		ailsa_syslog(LOG_ERR, "DELETE_SYSTEM_PACKAGE query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
rem_cbc_syspackage_arg(ailsa_cmdb_s *cbc, cbc_sysp_s *cbs)
{
	if (!(cbc) || !(cbs))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cbs->field, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add field to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_sys_pack_id_to_list(cbs->name, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add syspack id to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_SYS_PACK_ARG], l)) != 0)
		ailsa_syslog(LOG_ERR, "DELETE_SYS_PACK_ARG query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
rem_cbc_syspackage_conf(ailsa_cmdb_s *cbc, cbc_sysp_s *cbs)
{
	if (!(cbc) || !(cbs))
		return AILSA_NO_DATA;
	int retval;
	char **args = ailsa_calloc((sizeof(char *) * 2), "args in rem_cbc_syspackage_conf");
	AILLIST *l = ailsa_db_data_list_init();

	args[0] = cbs->name;
	args[1] = cbs->field;
	if ((retval = cmdb_add_string_to_list(cbs->arg, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add arg to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_sys_pack_id_to_list(cbs->name, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add syspack id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_sys_pack_arg_id_to_list(args, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add syspack arg id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_build_domain_id_to_list(cbs->domain, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain id to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_SYS_PACK_CONF], l)) != 0)
		ailsa_syslog(LOG_ERR, "DELETE_SYS_PACK_CONF query failed");

	cleanup:
		my_free(args);
		ailsa_list_full_clean(l);
		return retval;
}
