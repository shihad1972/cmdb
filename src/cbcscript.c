/*
 *
 *  cbcscript: Create Build Configuration Scripts
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
 *  cbcscript.c
 *
 *  Functions to get configuration values and also parse command line arguments
 *
 *  Part of the cbcscript program
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

static int
cbc_script_add_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
cbc_script_add_script_arg(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
cbc_script_rm_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
cbc_script_rm_arg(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
cbc_script_display_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
cbc_script_list_script(ailsa_cmdb_s *cbc);

static int
cbc_script_args_display_all_domain(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
cbc_script_args_display_one_domain(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
cbc_script_args_display_one_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr);

static int
parse_cbc_script_comm_line(int argc, char *argv[], cbc_syss_s *cbcs);

static int
check_cbc_script_comm_line(cbc_syss_s *cbcs);

int
main(int argc, char *argv[])
{
	int retval = 0;
	cbc_syss_s *scr = ailsa_calloc(sizeof(cbc_syss_s), "scr in main");
	ailsa_cmdb_s *cbc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cbc in main");

	if ((retval = parse_cbc_script_comm_line(argc, argv, scr)) != 0) {
		clean_cbc_syss_s(scr);
		ailsa_clean_cmdb(cbc);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(cbc);
	switch (scr->action) {
	case CMDB_ADD:
		if (scr->what == CBCSCRIPT)
			retval = cbc_script_add_script(cbc, scr);
		else if (scr->what == CBCSCRARG)
			retval = cbc_script_add_script_arg(cbc, scr);
		else
			retval = AILSA_WRONG_TYPE;
		break;
	case CMDB_RM:
		if (scr->what == CBCSCRIPT)
			retval = cbc_script_rm_script(cbc, scr);
		else if (scr->what == CBCSCRARG)
			retval = cbc_script_rm_arg(cbc, scr);
		else
			retval = AILSA_WRONG_TYPE;
		break;
	case CMDB_LIST:
		retval = cbc_script_list_script(cbc);
		break;
	case CMDB_DISPLAY:
		retval = cbc_script_display_script(cbc, scr);
		break;
	default:
		retval = AILSA_WRONG_ACTION;
		break;
	}
	ailsa_clean_cmdb(cbc);
	clean_cbc_syss_s(scr);
	return retval;
}

// Helper functions

static int
parse_cbc_script_comm_line(int argc, char *argv[], cbc_syss_s *cbcs)
{
	const char *optstr = "ab:dfg:h:ln:o:rst:v";
	int retval, opt;
	retval = 0;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"domain",		required_argument,	NULL,	'b'},
		{"build-domain",	required_argument,	NULL,	'b'},
		{"display",		no_argument,		NULL,	'd'},
		{"script-arg",		no_argument,		NULL,	'f'},
		{"argument",		required_argument,	NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"list",		no_argument,		NULL,	'l'},
		{"name",		required_argument,	NULL,	'n'},
		{"number",		required_argument,	NULL,	'o'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"script",		no_argument,		NULL,	's'},
		{"type",		required_argument,	NULL,	't'},
		{"version",		no_argument,		NULL,	'v'},
		{NULL,			0,			NULL,	0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		switch (opt) {
		case 'a':
			cbcs->action = CMDB_ADD;
			break;
		case 'd':
			cbcs->action = CMDB_DISPLAY;
			break;
		case 'l':
			cbcs->action = CMDB_LIST;
			break;
		case 'r':
			cbcs->action = CMDB_RM;
			break;
		case 'h':
			return AILSA_DISPLAY_USAGE;
			break;
		case 'v':
			return AILSA_VERSION;
		case 's':
			cbcs->what = CBCSCRIPT;
			break;
		case 'f':
			cbcs->what = CBCSCRARG;
			break;
		case 'o':
			cbcs->no = strtoul(optarg, NULL, 10);
			break;
		case 'b':
			cbcs->domain = strndup(optarg, (CONFIG_LEN));
			break;
		case 'g':
			cbcs->arg = strndup(optarg, (CONFIG_LEN - 1));
			break;
		case 'n':
			cbcs->name = strndup(optarg, (CONFIG_LEN - 1));
			break;
		case 't':
			cbcs->type = strndup(optarg, (MAC_LEN - 1));
			break;
		default:
			return AILSA_DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		retval = AILSA_DISPLAY_USAGE;
	else
		retval = check_cbc_script_comm_line(cbcs);
	return retval;
}

static int
check_cbc_script_comm_line(cbc_syss_s *cbcs)
{
	int retval = 0;

	if (cbcs->action == 0) {
		retval = AILSA_NO_ACTION;
	} else if (cbcs->action == CMDB_DISPLAY) {
		if (!(cbcs->domain) && !(cbcs->name))
			retval = AILSA_NO_DOMAIN_OR_NAME;
	} else if (cbcs->action == CMDB_LIST) {
		cbcs->what = CBCSCRIPT;	// Listing just args makes no sense
	} else if (cbcs->action != CMDB_LIST) {
		if (!(cbcs->name))
			retval = AILSA_NO_NAME;
		else if (cbcs->what == 0)
			retval = AILSA_NO_TYPE;
		else if (cbcs->what == CBCSCRARG) {
			if (cbcs->no == 0)
				retval = AILSA_NO_NUMBER;
			else if (!(cbcs->type))
				retval = AILSA_NO_OS;
			else if (!(cbcs->domain))
				retval = AILSA_NO_BUILD_DOMAIN;
			else if (!(cbcs->arg))
				retval = AILSA_NO_ARG;
		}
	}
	return retval;
}

static int
cbc_script_add_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	if (!(cbc) || !(scr))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(scr->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add script name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPT_ID_ON_NAME, l, res)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_PACKAGE_ID query failed");
		goto cleanup;
	}
	if (res->total > 0) {
		ailsa_syslog(LOG_INFO, "Script %s already in the database");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_SYSTEM_SCRIPT, l)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_SYSTEM_SCRIPT query failed");
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(res);
		return retval;
}

static int
cbc_script_add_script_arg(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	if (!(cbc) || !(scr))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(scr->arg, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add system script arg to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(scr->no, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add number to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_system_script_id_to_list(scr->name, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add system script id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_build_domain_id_to_list(scr->domain, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_build_type_id_to_list(scr->type, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build type id to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPT_ARG_ID, l, res)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPT_ARG_ID query failed");
		goto cleanup;
	}
	if (res->total > 0) {
		ailsa_syslog(LOG_INFO, "System script argument is already in the database");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_SYSTEM_SCRIPT_ARGS, l)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_SYSTEM_SCRIPT_ARGS query failed");
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(res);
		return retval;
}

// Remove functions

static int
cbc_script_rm_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	if (!(cbc) || !(scr))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(scr->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add script name to database");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPT_ID_ON_NAME, l, res)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPT_ID_ON_NAME query failed");
		goto cleanup;
	}
	if (res->total == 0) {
		ailsa_syslog(LOG_INFO, "System script %s not found in the database");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_SYSTEM_SCRIPT], res)) != 0)
		ailsa_syslog(LOG_ERR, "DELETE_SYSTEM_SCRIPT query failed");

	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(res);
		return retval;
}

static int
cbc_script_rm_arg(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	if (!(cbc) || !(scr))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();
	int retval;

	if ((retval = cmdb_add_string_to_list(scr->arg, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add system script arg to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(scr->no, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add number to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_system_script_id_to_list(scr->name, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add system script id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_build_domain_id_to_list(scr->domain, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_build_type_id_to_list(scr->type, cbc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build type id to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPT_ARG_ID, l, res)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPT_ARG_ID query failed");
		goto cleanup;
	}
	if (res->total == 0) {
		ailsa_syslog(LOG_INFO, "System script arg %s for script %s not in database", scr->arg, scr->name);
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_SYSTEM_SCRIPT_ARG], res)) != 0)
		ailsa_syslog(LOG_INFO, "DELETE_SYSTEM_SCRIPT_ARG query failed");
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(res);
		return retval;
}

static int
cbc_script_list_script(ailsa_cmdb_s *cbc)
{
	if (!(cbc))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = ailsa_basic_query(cbc, SYSTEM_SCRIPT_NAMES, l)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPT_NAMES query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "No system scripts to display");
		goto cleanup;
	}
	e = l->head;
	while (e) {
		printf("%s\n", ((ailsa_data_s *)e->data)->data->text);
		e = e->next;
	}
	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static int
cbc_script_display_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;

	if (!(scr->domain) && (scr->name))
		retval = cbc_script_args_display_all_domain(cbc, scr);
	else if ((scr->domain) && !(scr->name))
		retval = cbc_script_args_display_one_domain(cbc, scr);
	else
		retval = cbc_script_args_display_one_script(cbc, scr);
	return retval;
}

static int
cbc_script_args_display_all_domain(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	if (!(cbc) || !(scr))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *s = ailsa_db_data_list_init();
	AILELEM *e;
	size_t total = 4;
	char *domain = NULL, *alias = NULL;
	int retval, flag = 0;

	if ((retval = cmdb_add_string_to_list(scr->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add script name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPTS_ON_NAME, l, s)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPTS_ON_NAME query failed");
		goto cleanup;
	}
	if (s->total == 0) {
		ailsa_syslog(LOG_INFO, "No system script in database with name %s", scr->name);
		goto cleanup;
	}
	if ((s->total % total) != 0) {
		ailsa_syslog(LOG_INFO, "Got wrong number of results. Got %zu, wanted multiple of %zu", s->total, total);
		goto cleanup;
	}
	e = s->head;
	while (e) {
		flag = 0;
		if (!(domain)) 
			printf("Domain: %s", ((ailsa_data_s *)e->data)->data->text);
		else if (strcmp(domain, ((ailsa_data_s *)e->data)->data->text) != 0)
			printf("\n\nDomain: %s", ((ailsa_data_s *)e->data)->data->text);
		domain = ((ailsa_data_s *)e->data)->data->text;
		if (!(alias)) {
			printf("\n %s build\n", ((ailsa_data_s *)e->next->data)->data->text);
			flag = 1;
		} else if (strcmp(alias, ((ailsa_data_s *)e->next->data)->data->text) != 0) {
			printf("\n %s build\n", ((ailsa_data_s *)e->next->data)->data->text);
			flag = 1;
		}
		alias = ((ailsa_data_s *)e->next->data)->data->text;
		if (flag == 1)
			printf("\t%s", scr->name);
		printf(" %s", ((ailsa_data_s *)e->next->next->next->data)->data->text);
		e = ailsa_move_down_list(e, total);
	}
	printf("\n");
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(s);
		return retval;
}

static int
cbc_script_args_display_one_domain(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	if (!(cbc) || !(scr))
		return AILSA_NO_DATA;
	int retval, flag;
	size_t total = 4;
	char *script = NULL, *build = NULL;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *s = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = cmdb_add_string_to_list(scr->domain, l)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPTS_ON_DOMAIN query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPTS_ON_DOMAIN, l, s)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPTS_ON_DOMAIN query failed");
		goto cleanup;
	}
	if (s->total == 0) {
		ailsa_syslog(LOG_INFO, "No system scripts configured for domain %s\n", scr->domain);
		goto cleanup;
	}
	if ((s->total % total) != 0) {
		ailsa_syslog(LOG_INFO, "Got wrong number of results. Got %zu, wanted mutiple of %zu", s->total, total);
		goto cleanup;
	}
	e = s->head;
	printf("Domain: %s\n", scr->domain);
	while (e) {
		flag = 0;
		if (!(build)) {
			printf(" %s build\n", (((ailsa_data_s *)e->next->data)->data->text));
			flag = 1;
		} else if (strcmp(build, ((ailsa_data_s *)e->next->data)->data->text) != 0) {
			printf("\n %s build\n", (((ailsa_data_s *)e->next->data)->data->text));
			flag = 1;
		}
		build = ((ailsa_data_s *)e->next->data)->data->text;
		if (flag == 1)
			printf("\t%s", ((ailsa_data_s *)e->data)->data->text);
		else if (!(script))
			printf("\t%s", ((ailsa_data_s *)e->data)->data->text);
		else if (strcmp(script, ((ailsa_data_s *)e->data)->data->text) != 0)
			printf("\n\t%s", ((ailsa_data_s *)e->data)->data->text);
		script = ((ailsa_data_s *)e->data)->data->text;
		printf(" %s", ((ailsa_data_s *)e->next->next->next->data)->data->text);
		e = ailsa_move_down_list(e, total);
	}
	printf("\n");
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(s);
		return retval;
}

static int
cbc_script_args_display_one_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	if (!(cbc) || !(scr))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *s = ailsa_db_data_list_init();
	AILELEM *e;
	char *build = NULL;
	int retval, flag;
	size_t total = 3;

	if ((retval = cmdb_add_string_to_list(scr->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add script name to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(scr->domain, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPTS_ON_NAME_DOMAIN, l, s)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPTS_ON_NAME_DOMAIN query failed");
		goto cleanup;
	}
	if (s->total == 0) {
		ailsa_syslog(LOG_INFO, "No script configuration found for %s in build domain %s\n", scr->name, scr->domain);
		goto cleanup;
	}
	if ((s->total % total) != 0) {
		ailsa_syslog(LOG_INFO, "Got wrong number of results. Got %zu, wanted mutiple of %zu", s->total, total);
		goto cleanup;
	}
	e = s->head;
	printf("Domain: %s\n", scr->domain);
	while (e) {
		flag = 0;
		if (!(build)) {
			flag = 1;
			printf(" %s build\n", (((ailsa_data_s *)e->data)->data->text));
		} else if (strcmp(build, (((ailsa_data_s *)e->data)->data->text)) != 0) {
			flag = 1;
			printf("\n %s build\n", (((ailsa_data_s *)e->data)->data->text));
		}
		build = ((ailsa_data_s *)e->data)->data->text;
		if (flag == 1)
			printf("\t%s", scr->name);
		printf(" %s", (((ailsa_data_s *)e->next->next->data)->data->text));
		e = ailsa_move_down_list(e, total);
	}
	printf("\n");
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(s);
		return retval;
}
