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
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"

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

static int
pack_script_arg(ailsa_cmdb_s *cbc, cbc_script_arg_s *arg, cbc_syss_s *scr);

static void
pack_script_arg_data(dbdata_s *data, cbc_script_arg_s *arg);

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
	if (scr->action == CMDB_ADD) {
		if (scr->what == CBCSCRIPT)
			retval = cbc_script_add_script(cbc, scr);
		else if (scr->what == CBCSCRARG)
			retval = cbc_script_add_script_arg(cbc, scr);
		else
			retval = WRONG_TYPE;
	} else if (scr->action == CMDB_RM) {
		if (scr->what == CBCSCRIPT)
			retval = cbc_script_rm_script(cbc, scr);
		else if (scr->what == CBCSCRARG)
			retval = cbc_script_rm_arg(cbc, scr);
		else
			retval = WRONG_TYPE;
	} else if (scr->action == CMDB_LIST) {
		retval = cbc_script_list_script(cbc);
	} else if (scr->action == CMDB_DISPLAY) {
		retval = cbc_script_display_script(cbc, scr);
	} else {
		retval = WRONG_ACTION;
	}
	ailsa_clean_cmdb(cbc);
	clean_cbc_syss_s(scr);
	if ((retval != 0) && (retval != NO_RECORDS))
		report_error(retval, "");
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
		if (opt == 'a')
			cbcs->action = CMDB_ADD;
		else if (opt == 'd')
			cbcs->action = CMDB_DISPLAY;
		else if (opt == 'l')
			cbcs->action = CMDB_LIST;
		else if (opt == 'r')
			cbcs->action = CMDB_RM;
		else if (opt == 'h')
			return DISPLAY_USAGE;
		else if (opt == 'v')
			return CVERSION;
		else if (opt == 's')
			cbcs->what = CBCSCRIPT;
		else if (opt == 'f')
			cbcs->what = CBCSCRARG;
		else if (opt == 'o')
			cbcs->no = strtoul(optarg, NULL, 10);
		else if (opt == 'b')
			cbcs->domain = strndup(optarg, (CONFIG_LEN));
		else if (opt == 'g')
			cbcs->arg = strndup(optarg, (CONF_S - 1));
		else if (opt == 'n')
			cbcs->name = strndup(optarg, (CONF_S - 1));
		else if (opt == 't')
			cbcs->type = strndup(optarg, (MAC_S - 1));
		else
			retval = DISPLAY_USAGE;
	}
	if (argc == 1)
		retval = DISPLAY_USAGE;
	else
		retval = check_cbc_script_comm_line(cbcs);
	return retval;
}

static int
check_cbc_script_comm_line(cbc_syss_s *cbcs)
{
	int retval = 0;

	if (cbcs->action == 0) {
		retval = NO_ACTION;
	} else if (cbcs->action == CMDB_DISPLAY) {
		if (!(cbcs->domain) && !(cbcs->name))
			retval = NO_DOMAIN_OR_NAME;
	} else if (cbcs->action == CMDB_LIST) {
		cbcs->what = CBCSCRIPT;	// Listing just args makes no sense
	} else if (cbcs->action != CMDB_LIST) {
		if (!(cbcs->name))
			retval = NO_NAME;
		else if (cbcs->what == 0)
			retval = NO_TYPE;
		else if (cbcs->what == CBCSCRARG) {
			if (cbcs->no == 0)
				retval = NO_NUMBER;
			else if (!(cbcs->type))
				retval = DISPLAY_USAGE;
			else if (!(cbcs->domain))
				retval = DISPLAY_USAGE;
			if (cbcs->action == CMDB_ADD) {
				if  (!(cbcs->arg))
					retval = NO_ARG;
			}
		}
	}
	return retval;
}

static int
pack_script_arg(ailsa_cmdb_s *cbc, cbc_script_arg_s *arg, cbc_syss_s *scr)
{
	int retval = 0;
	if (scr->name) {
		if ((retval = get_system_script_id(cbc, scr->name, &(arg->systscr_id))) != 0)
			return retval;
	} else {
		return NO_NAME;
	}
	if (scr->domain) {
		if ((retval = get_build_domain_id(cbc, scr->domain, &(arg->bd_id))) != 0)
			return retval;
	} else {
		return NO_DOMAIN;
	}
	if (scr->type) {
		if ((retval = get_build_type_id(cbc, scr->type, &(arg->bt_id))) != 0)
			return retval;
	} else {
		return CBC_NO_DATA;
	}
	if (scr->arg)
		snprintf(arg->arg, CONF_S, "%s", scr->arg);
	else if (scr->action == ADD_CONFIG)
		return CBC_NO_DATA;
	if (scr->no > 0)
		arg->no = scr->no;
	else
		return CBC_NO_DATA;
	arg->cuser = arg->muser = (unsigned long int)getuid();
	return 0;
}

static void
pack_script_arg_data(dbdata_s *data, cbc_script_arg_s *arg)
{
	data->args.number = arg->bd_id;
	data->next->args.number = arg->bt_id;
	data->next->next->args.number = arg->systscr_id;
	data->next->next->next->args.number = arg->no;
}

// Add functions

static int
cbc_script_add_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_script_s *cbcscr;

	initialise_cbc_s(&cbs);
	initialise_cbc_scripts(&cbcscr);
	cbs->scripts = cbcscr;
	cbcscr->cuser = cbcscr->muser = (unsigned long int)getuid();
	snprintf(cbcscr->name, CONF_S, "%s", scr->name);
	if ((retval = cbc_run_insert(cbc, cbs, SCRIPTS)) != 0)
		fprintf(stderr, "Unable to add script to database\n");
	else
		printf("Script %s added to database\n", scr->name);
	clean_cbc_struct(cbs);
	return retval;
}

static int
cbc_script_add_script_arg(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_script_arg_s *arg;

	initialise_cbc_s(&cbs);
	initialise_cbc_script_args(&arg);
	cbs->script_arg = arg;
	if ((retval = pack_script_arg(cbc, arg, scr)) != 0)
		goto cleanup;
	if ((retval = cbc_run_insert(cbc, cbs, SCRIPTAS)) != 0)
		fprintf(stderr, "Unable to add args for script to db\n");
	else
		printf("Script args added into db\n");
	goto cleanup;

	cleanup:
		clean_cbc_struct(cbs);
		return retval;
}

// Remove functions

static int
cbc_script_rm_script(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_system_script_id(cbc, scr->name, &(data->args.number))) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	if ((retval = cbc_run_delete(cbc, data, CBCSCR_ON_ID)) == 0) {
		fprintf(stderr, "Unable to remove script %s\n", scr->name);
		retval = DB_DELETE_FAILED;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple scripts removed for %s\n", scr->name);
		retval = 0;
	} else {
		printf("Script %s removed from database\n", scr->name);
		retval = 0;
	}
	clean_dbdata_struct(data);
	return retval;
}

static int
cbc_script_rm_arg(ailsa_cmdb_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;
	dbdata_s *data;
	cbc_script_arg_s *arg;

	if (!(arg = malloc(sizeof(cbc_script_arg_s))))
		report_error(MALLOC_FAIL, "arg in cbc_script_rm_arg");
	init_cbc_script_args(arg);
	if ((retval = pack_script_arg(cbc, arg, scr)) != 0) {
		clean_cbc_script_args(arg);
		return retval;
	}
	init_multi_dbdata_struct(&data, 4);
	pack_script_arg_data(data, arg);
	if ((retval = cbc_run_search(cbc, data, SCR_ARG_ID)) == 0) {
		clean_dbdata_struct(data);
		clean_cbc_script_args(arg);
		return NO_ARG;
	} else if (retval > 1)
		fprintf(stderr, "More than one cbcscript was returned. Using 1st\n");
	clean_dbdata_struct(data->next);
	data->next = 0;
	data->args.number = data->fields.number;
	retval = cbc_run_delete(cbc, data, CBCSCRARG_ON_ID);
	printf("%d args deleted\n", retval);
	return 0;
}

// List functions

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
