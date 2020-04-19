/* 
 *
 *  cbclocale: Create build config locale
 *  Copyright (C) 2016 - 2020 Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  cbclocale.c
 *
 *  Main source file for the cbclocale program
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
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

typedef struct locale_comm_line_s {
	char *country;
	char *keymap;
	char *language;
	char *locale;
	char *name;
	char *timezone;
	short int action;
} locale_comm_line_s;

static int
get_default_locale(ailsa_cmdb_s *ccs, unsigned long int *lid);

static int
parse_locale_comm_line(int argc, char *argv[], locale_comm_line_s *cl);

static void
validate_locale_comm_line(locale_comm_line_s *cl);

static int
list_locales(ailsa_cmdb_s *ccs);

static int
display_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static int
add_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static int
remove_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static int
set_default_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static void
fill_locale(cbc_locale_s *loc, locale_comm_line_s *cl);

int
main(int argc, char *argv[])
{
	int retval = 0;
	locale_comm_line_s *cl = ailsa_calloc(sizeof(locale_comm_line_s), "cl in main");
	ailsa_cmdb_s *ccs = ailsa_calloc(sizeof(ailsa_cmdb_s), "ccs in main");
	if ((retval = parse_locale_comm_line(argc, argv, cl)) != 0) {
		free(cl);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(ccs);
	if (cl->action == LIST_CONFIG)
		retval = list_locales(ccs);
	else if (cl->action == DISPLAY_CONFIG)
		retval = display_locale(ccs, cl);
	else if (cl->action == ADD_CONFIG)
		retval = add_locale(ccs, cl);
	else if (cl->action == RM_CONFIG)
		retval = remove_locale(ccs, cl);
	else if (cl->action == SET_DEFAULT)
		retval = set_default_locale(ccs, cl);
	else {
		fprintf(stderr, "Action not yet implemented\n");
		retval = DISPLAY_USAGE;
	}
	ailsa_clean_cmdb(ccs);
	my_free(cl);
	return retval;
}

static int
parse_locale_comm_line(int argc, char *argv[], locale_comm_line_s *cl)
{
	int retval = 0;
	const char *optstr = "adg:hk:lo:n:rt:u:vx";
	int opt;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"display",		no_argument,		NULL,	'd'},
		{"language",		required_argument,	NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"keymap",		required_argument,	NULL,	'k'},
		{"list",		no_argument,		NULL,	'l'},
		{"locale",		required_argument,	NULL,	'o'},
		{"name",		required_argument,	NULL,	'n'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"timezone",		required_argument,	NULL,	't'},
		{"country",		required_argument,	NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"default",		no_argument,		NULL,	'x'}
	};

	if (argc == 1)
		return DISPLAY_USAGE;
	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optsttr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a')
			cl->action = ADD_CONFIG;
		else if (opt == 'd')
			cl->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			cl->action = LIST_CONFIG;
		else if (opt == 'r')
			cl->action = RM_CONFIG;
		else if (opt == 'v')
			return CVERSION;
		else if (opt == 'x')
			cl->action = SET_DEFAULT;
		else if (opt == 'g')
			cl->language = strndup(optarg, RANGE_S);
		else if (opt == 'k')
			cl->keymap = strndup(optarg, RANGE_S);
		else if (opt == 'o')
			cl->locale = strndup(optarg, MAC_S);
		else if (opt == 'n')
			cl->name = strndup(optarg, HOST_S);
		else if (opt == 't')
			cl->timezone = strndup(optarg, HOST_S);
		else if (opt == 'u')
			cl->country = strndup(optarg, HOST_S);
		else
			return DISPLAY_USAGE;
	}
	if ((cl->action == CVERSION) || (cl->action == LIST_CONFIG))
		return retval;
	if ((cl->action != LIST_CONFIG) && (!(cl->name)))
		return NO_NAME;
	if (cl->action == 0)
		return NO_ACTION;
	if (cl->action == ADD_CONFIG) {
		if (!(cl->language)) {
			fprintf(stderr, "No language specified\n\n");
			return DISPLAY_USAGE;
		} else if (!(cl->keymap)) {
			fprintf(stderr, "No keymap specified\n\n");
			return DISPLAY_USAGE;
		} else if (!(cl->locale)) {
			fprintf(stderr, "No locale specified\n\n");
			return DISPLAY_USAGE;
		} else if (!(cl->timezone)) {
			fprintf(stderr, "No timezone specified\n\n");
			return DISPLAY_USAGE;
		} else if (!(cl->country)) {
			fprintf(stderr, "No country specified\n\n");
			return DISPLAY_USAGE;
		}
	}
	validate_locale_comm_line(cl);
	return retval;
}

static void
validate_locale_comm_line(locale_comm_line_s *cl)
{
	if (cl->language)
		if (ailsa_validate_input(cl->language, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "language");
	if (cl->keymap)
		if (ailsa_validate_input(cl->keymap, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "keymap");
	if (cl->locale)
		if (ailsa_validate_input(cl->locale, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "locale");
	if (cl->name)
		if (ailsa_validate_input(cl->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "name");
	if (cl->timezone)
		if (ailsa_validate_input(cl->timezone, TIMEZONE_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "timezone");
	if (cl->country)
		if (ailsa_validate_input(cl->country, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "country");
}

static int
get_default_locale(ailsa_cmdb_s *ccs, unsigned long int *lid)
{
	if (!(ccs) || !(lid))
		return AILSA_NO_DATA;
	int retval = 0;
	AILLIST *d = ailsa_db_data_list_init();

	if ((retval = ailsa_basic_query(ccs, DEFAULT_LOCALE, d)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_LOCALE query failed");
		goto cleanup;
	}
	if (d->total == 0)
		*lid = 0;
	else
		*lid = ((ailsa_data_s *)d->head->data)->data->number;
	cleanup:
		ailsa_list_destroy(d);
		my_free(d);
		return retval;
}

static int
list_locales(ailsa_cmdb_s *ccs)
{
	int retval = 0;
	unsigned long int isdefault = 0;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = get_default_locale(ccs, &isdefault)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get default locale");
		goto cleanup;
	}
	if ((retval = ailsa_basic_query(ccs, LOCALE_NAMES, l)) != 0) {
		ailsa_syslog(LOG_ERR, "ALL_LOCALES query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "No locales in database?");
		goto cleanup;
	}
	e = l->head;
	while (e) {
		if (e->next)
			if (((ailsa_data_s *)e->next->data)->type == AILSA_DB_TEXT)
				printf("%s", ((ailsa_data_s *)e->next->data)->data->text);
		if (((ailsa_data_s *)e->data)->type == AILSA_DB_LINT)
			if (((ailsa_data_s *)e->data)->data->number == isdefault)
				printf(" *");
		printf("\n");
		e = e->next->next;
	}
	cleanup:
		ailsa_list_destroy(l);
		my_free(l);
		return retval;
}

static int
display_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	if (!(ccs) || !(cl))
		return AILSA_NO_DATA;
	int retval = 0;
	unsigned long int isdefault = 0;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = get_default_locale(ccs, &isdefault)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get default locale");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cl->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot name into local list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(ccs, LOCALE_ON_NAME, l, r)) != 0) {
		ailsa_syslog(LOG_ERR, "LOCALE_ON_NAME query failed");
		goto cleanup;
	}
	if (r->total != 8) {
		ailsa_syslog(LOG_ERR, "Should have 8 in list; got %zu", r->total);
		goto cleanup;
	}
	e = r->head;
	printf("Locale: %s", cl->name);
	if (isdefault == ((ailsa_data_s *)e->data)->data->number)
		printf("  * Default");
	printf("\n");
	e = e->next;
	printf("  Locale:\t%s\n", ((ailsa_data_s *)e->data)->data->text);
	e = e->next;
	printf("  Country:\t%s\n", ((ailsa_data_s *)e->data)->data->text);
	e = e->next;
	printf("  Language:\t%s\n", ((ailsa_data_s *)e->data)->data->text);
	e = e->next;
	printf("  Keymap:\t%s\n", ((ailsa_data_s *)e->data)->data->text);
	e = e->next;
	printf("  Timezone:\t%s\n", ((ailsa_data_s *)e->data)->data->text);

	cleanup:
		ailsa_list_destroy(l);
		ailsa_list_destroy(r);
		my_free(l);
		my_free(r);
		return retval;
}

static int
add_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	int retval;
	cbc_s *cbc = ailsa_calloc(sizeof(cbc_s), "cbc in add_locale");
	cbc_locale_s *loc = ailsa_calloc(sizeof(cbc_locale_s), "loc in add_locale");

	cbc->locale = loc;
	fill_locale(loc, cl);
	if ((retval = cbc_run_insert(ccs, cbc, LOCALES)) != 0)
		fprintf(stderr, "Unable to insert locale %s into database", loc->name);
	else
		printf("Locale %s added to DB\n", loc->name);
	clean_cbc_struct(cbc);
	return retval;
}

static void
fill_locale(cbc_locale_s *loc, locale_comm_line_s *cl)
{
	if (!(loc) || !(cl))
		report_error(CBC_NO_DATA, "fill_locale");
	snprintf(loc->country, RANGE_S, "%s", cl->country);
	snprintf(loc->language, RANGE_S, "%s", cl->language);
	snprintf(loc->keymap, RANGE_S, "%s", cl->keymap);
	snprintf(loc->locale, MAC_S, "%s", cl->locale);
	snprintf(loc->timezone, HOST_S, "%s", cl->timezone);
	snprintf(loc->name, HOST_S, "%s", cl->name);
	loc->cuser = (unsigned long int)getuid();
	loc->muser = (unsigned long int)getuid();
}

static int
remove_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	if (!(ccs) || !(cl))
		report_error(CBC_NO_DATA, "remove_locale");
	char *name = cl->name;
	unsigned long int id;
	int retval;
	dbdata_s *data;

	if ((retval = get_locale_id(ccs, name, &id)) != 0)
		return retval;
	init_multi_dbdata_struct(&data, 1);
	data->args.number = id;
	if ((retval = cbc_run_delete(ccs, data, LOCALE_ON_ID)) == 0)
		fprintf(stderr, "Unable to delete locale %s\n", name);
	else if (retval == 1)
		printf("Locale %s deleted\n", name);
	else
		printf("%d locales deleted for %s\n", retval, name);
	return 0;
}

static int
set_default_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	int retval = 0;
	int query = UP_DEFAULT_LOCALE;
	unsigned long int locale_id;
	dbdata_s *data;

	if ((retval = get_default_id(ccs, LOCALE_ID_ON_NAME, cl->name, &locale_id)) != 0) {
		fprintf(stderr, "Cannot find locale %s\n", cl->name);
		return retval;
	}
	init_multi_dbdata_struct(&data, 1);
	data->args.number = locale_id;
	if ((retval = cbc_run_update(ccs, data, query)) == 0)
		fprintf(stderr, "Unable to set default locale\n");
	else if (retval > 1)
		fprintf(stderr, "Multiple default locales updated\n");
	else
		printf("Default locale set to %s\n", cl->name);
	clean_dbdata_struct(data);
	return retval;
}

