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

static int
validate_locale_comm_line(locale_comm_line_s *cl);

static int
list_locales(ailsa_cmdb_s *ccs);

static int
display_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static int
display_servers_in_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static int
add_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static int
remove_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static int
set_default_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl);

static void
clean_cbc_local_comm_line(locale_comm_line_s *cl);

int
main(int argc, char *argv[])
{
	int retval = 0;
	locale_comm_line_s *cl = ailsa_calloc(sizeof(locale_comm_line_s), "cl in main");
	ailsa_cmdb_s *ccs = ailsa_calloc(sizeof(ailsa_cmdb_s), "ccs in main");
	if ((retval = parse_locale_comm_line(argc, argv, cl)) != 0) {
		ailsa_clean_cmdb(ccs);
		clean_cbc_local_comm_line(cl);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(ccs);
	if (cl->action == CMDB_LIST)
		retval = list_locales(ccs);
	else if (cl->action == CMDB_DISPLAY)
		retval = display_locale(ccs, cl);
	else if (cl->action == CMDB_ADD)
		retval = add_locale(ccs, cl);
	else if (cl->action == CBC_SERVER)
		retval = display_servers_in_locale(ccs, cl);
	else if (cl->action == CMDB_RM)
		retval = remove_locale(ccs, cl);
	else if (cl->action == CMDB_DEFAULT)
		retval = set_default_locale(ccs, cl);
	else {
		ailsa_syslog(LOG_ERR, "Action not yet implemented\n");
		retval = AILSA_DISPLAY_USAGE;
	}
	ailsa_clean_cmdb(ccs);
	clean_cbc_local_comm_line(cl);
	return retval;
}

static int
parse_locale_comm_line(int argc, char *argv[], locale_comm_line_s *cl)
{
	int retval = 0;
	const char *optstr = "adg:hk:lo:n:rqt:u:vz";
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
		{"query",		no_argument,		NULL, 	'q'},
		{"server",		no_argument,		NULL,	'q'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"timezone",		required_argument,	NULL,	't'},
		{"country",		required_argument,	NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"default",		no_argument,		NULL,	'z'}
	};

	if (argc == 1)
		return AILSA_DISPLAY_USAGE;
	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optsttr)) != -1)
#endif // HAVE_GETOPT_H
	{
		switch (opt) {
		case 'a':
			cl->action = CMDB_ADD;
			break;
		case 'd':
			cl->action = CMDB_DISPLAY;
			break;
		case 'l':
			cl->action = CMDB_LIST;
			break;
		case 'q':
			cl->action = CBC_SERVER;
			break;
		case 'r':
			cl->action = CMDB_RM;
			break;
		case 'v':
			return AILSA_VERSION;
		case 'z':
			cl->action = CMDB_DEFAULT;
			break;
		case 'g':
			cl->language = strndup(optarg, SERVICE_LEN);
			break;
		case 'k':
			cl->keymap = strndup(optarg, SERVICE_LEN);
			break;
		case 'o':
			cl->locale = strndup(optarg, MAC_LEN);
			break;
		case 'n':
			cl->name = strndup(optarg, HOST_LEN);
			break;
		case 't':
			cl->timezone = strndup(optarg, HOST_LEN);
			break;
		case 'u':
			cl->country = strndup(optarg, HOST_LEN);
			break;
		default:
			return AILSA_DISPLAY_USAGE;
		}
	}
	if ((cl->action == AILSA_VERSION) || (cl->action == CMDB_LIST))
		return retval;
	if ((cl->action != CMDB_LIST) && (!(cl->name)))
		return AILSA_NO_NAME;
	if (cl->action == 0)
		return AILSA_NO_ACTION;
	if (cl->action == CMDB_ADD) {
		if (!(cl->language)) {
			ailsa_syslog(LOG_ERR, "No language specified\n\n");
			return AILSA_DISPLAY_USAGE;
		} else if (!(cl->keymap)) {
			ailsa_syslog(LOG_ERR, "No keymap specified\n\n");
			return AILSA_DISPLAY_USAGE;
		} else if (!(cl->locale)) {
			ailsa_syslog(LOG_ERR, "No locale specified\n\n");
			return AILSA_DISPLAY_USAGE;
		} else if (!(cl->timezone)) {
			ailsa_syslog(LOG_ERR, "No timezone specified\n\n");
			return AILSA_DISPLAY_USAGE;
		} else if (!(cl->country)) {
			ailsa_syslog(LOG_ERR, "No country specified\n\n");
			return AILSA_DISPLAY_USAGE;
		}
	}
	validate_locale_comm_line(cl);
	return retval;
}

static int
validate_locale_comm_line(locale_comm_line_s *cl)
{
	if (cl->language)
		if (ailsa_validate_input(cl->language, NAME_REGEX) < 0)
			return LANGUAGE_INVALID;
	if (cl->keymap)
		if (ailsa_validate_input(cl->keymap, NAME_REGEX) < 0)
			return KEYMAP_INVALID;
	if (cl->locale)
		if (ailsa_validate_input(cl->locale, LOGVOL_REGEX) < 0)
			return LOCALE_INVALID;
	if (cl->name)
		if (ailsa_validate_input(cl->name, NAME_REGEX) < 0)
			return LOCAL_NAME_INVALID;
	if (cl->timezone)
		if (ailsa_validate_input(cl->timezone, TIMEZONE_REGEX) < 0)
			return TIMEZONE_INVALID;
	if (cl->country)
		if (ailsa_validate_input(cl->country, NAME_REGEX) < 0)
			return COUNTRY_INVALID;
	return 0;
}

static void
clean_cbc_local_comm_line(locale_comm_line_s *cl)
{
	if (!(cl))
		return;
	if (cl->country)
		my_free(cl->country);
	if (cl->keymap)
		my_free(cl->keymap);
	if (cl->language)
		my_free(cl->language);
	if (cl->locale)
		my_free(cl->locale);
	if (cl->name)
		my_free(cl->name);
	if (cl->timezone)
		my_free(cl->timezone);
	my_free(cl);
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
		ailsa_list_full_clean(d);
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
		ailsa_list_full_clean(l);
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
	if (r->total == 0) {
		ailsa_syslog(LOG_INFO, "No locale named %s", cl->name);
		goto cleanup;
	}
	if (r->total != 8) {
		ailsa_syslog(LOG_ERR, "Should have 8 in list; got %zu", r->total);
		retval = AILSA_WRONG_LIST_LENGHT;
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
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		return retval;
}

static int
display_servers_in_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	if (!(ccs) || !(cl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *locale = ailsa_db_data_list_init();
	AILLIST *server = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = cmdb_add_string_to_list(cl->name, locale)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(ccs, SERVERS_IN_LOCALE, locale, server)) != 0)
		goto cleanup;
	if (server->total == 0) {
		ailsa_syslog(LOG_INFO, "No servers built with locale %s", cl->name);
		goto cleanup;
	}
	printf("Servers build with locale %s\n", cl->name);
	e = server->head;
	while (e) {
		printf(" %s\n", ((ailsa_data_s *)e->data)->data->text);
		e = e->next;
	}

	cleanup:
		ailsa_list_full_clean(locale);
		ailsa_list_full_clean(server);
		return retval;
}

static int
add_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILLIST *a = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cl->name, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add locale name into list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(ccs, LOCALE_ON_NAME, a, r)) != 0) {
		ailsa_syslog(LOG_ERR, "LOCALE_ON_NAME query failed");
		goto cleanup;
	}
	if (r->total > 0) {
		ailsa_syslog(LOG_ERR, "Locale %s already in database", cl->locale);
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cl->locale, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add locale into list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cl->country, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add country into list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cl->language, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add language into list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cl->keymap, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add keymap into list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cl->timezone, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add keymap into list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cl->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add locale name into list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(ccs, INSERT_LOCALE, l)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert locale into database");
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(a);
		return retval;
}

static int
remove_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	if (!(ccs) || !(cl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cl->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add locale name to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(ccs, delete_queries[DELETE_LOCALE], l)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot delete locale %s\n", cl->name);
	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static int
set_default_locale(ailsa_cmdb_s *ccs, locale_comm_line_s *cl)
{
	if (!(ccs) || !(cl))
		return AILSA_NO_DATA;
	int retval = 0;
	AILLIST *u = ailsa_db_data_list_init();
	unsigned long int user_id = (unsigned long int)getuid();

	if ((retval = cmdb_add_string_to_list(cl->name, u)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add locale name into list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(user_id, u)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add user id into list");
		goto cleanup;
	}
	if ((retval = ailsa_update_query(ccs, update_queries[SET_DEFAULT_LOCALE], u)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot set default locale");
	cleanup:
		ailsa_list_full_clean(u);
		return retval;
}

