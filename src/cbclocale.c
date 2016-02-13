/* 
 *
 *  cbclocale: Create build config locale
 *  Copyright (C) 2016 Iain M Conochie <iain-AT-thargoid.co.uk> 
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
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif // HAVE_LIBPCRE

typedef struct locale_comm_line_s {
	char country[RANGE_S];
	char keymap[RANGE_S];
	char language[RANGE_S];
	char locale[MAC_S];
	char name[HOST_S];
	char timezone[HOST_S];
	short int action;
} locale_comm_line_s;

static void
init_locale_comm_line(locale_comm_line_s *cl);

static int
parse_locale_comm_line(int argc, char *argv[], locale_comm_line_s *cl);

#ifdef HAVE_LIBPCRE
static void
validate_locale_comm_line(locale_comm_line_s *cl);
#endif // HAVE_LIBPCRE

static int
list_locales(cbc_config_s *ccs);

static int
display_locales(cbc_config_s *ccs, locale_comm_line_s *cl);

static int
add_locale(cbc_config_s *ccs, locale_comm_line_s *cl);

static void
fill_locale(cbc_locale_s *loc, locale_comm_line_s *cl);

static void
print_locale(cbc_locale_s *loc);

int
main(int argc, char *argv[])
{
	char conf[CONF_S];
	const char *config;
	int retval = 0;
	locale_comm_line_s *cl = cmdb_malloc(sizeof(locale_comm_line_s), "cl in main");
	cbc_config_s *ccs = cmdb_malloc(sizeof(cbc_config_s), "ccs in main");
	init_locale_comm_line(cl);
	init_cbc_config_values(ccs);
	if ((retval = parse_locale_comm_line(argc, argv, cl)) != 0) {
		free(cl);
		display_command_line_error(retval, argv[0]);
	}
	get_config_file_location(conf);
	config = conf;
	if ((retval = parse_cbc_config_file(ccs, config)) != 0) {
		parse_cbc_config_error(retval);
		goto cleanup;
	}
	if (cl->action == LIST_CONFIG)
		retval = list_locales(ccs);
	else if (cl->action == DISPLAY_CONFIG)
		retval = display_locales(ccs, cl);
	else if (cl->action == ADD_CONFIG)
		retval = add_locale(ccs, cl);
	else {
		fprintf(stderr, "Action not yet implemented\n");
		retval = DISPLAY_USAGE;
	}

	cleanup:
		free(ccs);
		free(cl);
		return retval;
}

static int
parse_locale_comm_line(int argc, char *argv[], locale_comm_line_s *cl)
{
	int retval = 0;
	const char *optstr = "adg:hk:lo:n:rt:u:v";
	int opt;
	size_t glen, klen, olen, nlen, tlen, ulen;
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
		{"version",		no_argument,		NULL,	'v'}
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
		else if (opt == 'g')
			snprintf(cl->language, RANGE_S, "%s", optarg);
		else if (opt == 'k')
			snprintf(cl->keymap, RANGE_S, "%s", optarg);
		else if (opt == 'l')
			cl->action = LIST_CONFIG;
		else if (opt == 'o')
			snprintf(cl->locale, MAC_S, "%s", optarg);
		else if (opt == 'n')
			snprintf(cl->name, HOST_S, "%s", optarg);
		else if (opt == 'r')
			cl->action = RM_CONFIG;
		else if (opt == 't')
			snprintf(cl->timezone, HOST_S, "%s", optarg);
		else if (opt == 'u')
			snprintf(cl->country, HOST_S, "%s", optarg);
		else if (opt == 'v')
			return CVERSION;
		else
			return DISPLAY_USAGE;
	}
	glen = strlen(cl->language);
	klen = strlen(cl->keymap);
	olen = strlen(cl->locale);
	nlen = strlen(cl->name);
	tlen = strlen(cl->timezone);
	ulen = strlen(cl->country);
	if ((cl->action == CVERSION) || (cl->action == LIST_CONFIG))
		return retval;
	if (((cl->action == RM_CONFIG) || (cl->action == DISPLAY_CONFIG) ||
	    (cl->action == ADD_CONFIG)) && nlen == 0)
		return NO_NAME;
	if (cl->action == 0)
		return NO_ACTION;
	if (cl->action == ADD_CONFIG) {
		if (glen == 0) {
			fprintf(stderr, "No language specified\n\n");
			return DISPLAY_USAGE;
		} else if (klen == 0) {
			fprintf(stderr, "No keymap specified\n\n");
			return DISPLAY_USAGE;
		} else if (olen == 0) {
			fprintf(stderr, "No locale specified\n\n");
			return DISPLAY_USAGE;
		} else if (tlen == 0) {
			fprintf(stderr, "No timezone specified\n\n");
			return DISPLAY_USAGE;
		} else if (ulen == 0) {
			fprintf(stderr, "No country specified\n\n");
			return DISPLAY_USAGE;
		}
	}
#ifdef HAVE_LIBPCRE
	validate_locale_comm_line(cl);
#endif // HAVE_LIBPCRE
	return retval;
}

#ifdef HAVE_LIBPCRE
static void
validate_locale_comm_line(locale_comm_line_s *cl)
{
	if (strncmp(cl->language, "NULL", COMM_S) != 0)
		if (validate_user_input(cl->language, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "language");
	if (strncmp(cl->keymap, "NULL", COMM_S) != 0)
		if (validate_user_input(cl->keymap, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "keymap");
	if (strncmp(cl->locale, "NULL", COMM_S) != 0)
		if (validate_user_input(cl->locale, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "locale");
	if (strncmp(cl->name, "NULL", COMM_S) != 0)
		if (validate_user_input(cl->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "name");
	if (strncmp(cl->timezone, "NULL", COMM_S) != 0)
		if (validate_user_input(cl->timezone, TIMEZONE_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "timezone");
	if (strncmp(cl->country, "NULL", COMM_S) != 0)
		if (validate_user_input(cl->country, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "country");
}
#endif // HAVE_LIBPCRE

static void
init_locale_comm_line(locale_comm_line_s *cl)
{
	if (!(cl))
		return;
	snprintf(cl->language, COMM_S, "NULL");
	snprintf(cl->keymap, COMM_S, "NULL");
	snprintf(cl->locale, COMM_S, "NULL");
	snprintf(cl->name, COMM_S, "NULL");
	snprintf(cl->timezone, COMM_S, "NULL");
	snprintf(cl->country, COMM_S, "NULL");
}

static int
list_locales(cbc_config_s *ccs)
{
	int retval = 0;
	cbc_s *cbc = cmdb_malloc(sizeof(cbc_s), "cbc in list_locales");
	cbc_locale_s *loc;

	if ((retval = cbc_run_query(ccs, cbc, LOCALE)) != 0) {
		fprintf(stderr, "DB query error %d\n", retval);
		goto cleanup;
	}
	loc = cbc->locale;
	while (loc) {
		printf("%s\n", loc->name);
		loc = loc->next;
	}
	cleanup:
		clean_cbc_struct(cbc);
		return retval;
}

static void
print_locale(cbc_locale_s *locale)
{
	cbc_locale_s *loc = locale;

	if (!(loc))
		return;
	printf("Locale %s\n\n", loc->name);
	printf("Keymap:\t\t%s\n", loc->keymap);
	printf("Language:\t%s\n", loc->language);
	printf("Country:\t%s\n", loc->country);
	printf("Locale:\t\t%s\n", loc->locale);
	printf("Timezone:\t%s\n", loc->timezone);
	printf("\n");
}

static int
display_locales(cbc_config_s *ccs, locale_comm_line_s *cl)
{
	int retval = 0;
	cbc_s *cbc = cmdb_malloc(sizeof(cbc_s), "cbc in display_locales");
	cbc_locale_s *loc;

	if ((retval = cbc_run_query(ccs, cbc, LOCALE)) != 0) {
		fprintf(stderr, "DB query error %d\n", retval);
		goto cleanup;
	}
	loc = cbc->locale;
	while (loc) {
		if (strncmp(loc->name, cl->name, HOST_S) == 0)
			print_locale(loc);
		loc = loc->next;
	}
	cleanup:
		clean_cbc_struct(cbc);
		return retval;
}

static int
add_locale(cbc_config_s *ccs, locale_comm_line_s *cl)
{
	int retval;
	cbc_s *cbc = cmdb_malloc(sizeof(cbc_s), "cbc in add_locale");
	cbc_locale_s *loc = cmdb_malloc(sizeof(cbc_locale_s), "loc in add_locale");

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

