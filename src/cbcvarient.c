/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
#include <time.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbc_common.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */


enum {
	CVARIENT = 1,
	CPACKAGE = 2
};
typedef struct cbcvari_comm_line_s {
	char alias[MAC_S];
	char arch[RANGE_S];
	char os[MAC_S];
	char ver_alias[MAC_S];
	char version[MAC_S];
	char varient[HOST_S];
	char valias[MAC_S];
	char package[HOST_S];
	short int action;
	short int type;
} cbcvari_comm_line_s;

static void
init_cbcvari_config(cbc_config_s *cmc, cbcvari_comm_line_s *cvl);

static void
init_cbcvari_comm_line(cbcvari_comm_line_s *cvl);

static int
parse_cbcvarient_comm_line(int argc, char *argv[], cbcvari_comm_line_s *cvl);

static int
list_cbc_build_varient(cbc_config_s *cmc);

static int
display_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl);

static int
add_cbc_build_varient(cbc_config_s *cbc, cbcvari_comm_line_s *cvl);

static int
add_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl);

static int
remove_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl);

static int
remove_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl);

static int
check_build_os(cbc_config_s *cbc, cbc_s *base, cbcvari_comm_line_s *cvl);

static int
display_all_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl);

static int
display_one_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl);

static int
display_specific_os_packages(cbc_s *base, unsigned long int id, unsigned long int osid);

static unsigned long int
get_single_os_id(cbc_s *base, cbcvari_comm_line_s *cvl);

static int
cbc_get_os(cbc_build_os_s *os, cbcvari_comm_line_s *cvl, unsigned long int **id);

static int
cbc_get_os_list(cbc_build_os_s *os, cbcvari_comm_line_s *cvl, unsigned long int *id);

static cbc_package_s *
build_package_list(cbc_config_s *cbc, unsigned long int *os, int nos, char *pack);

static dbdata_s *
build_rem_pack_list(cbc_config_s *cbc, unsigned long int *ids, int noids, char *pack);

static void
copy_packages_from_base_varient(cbc_config_s *cbc, char *varient);

static int
build_copy_package_list(cbc_config_s *cbc, cbc_s *base, uli_t bid, uli_t id);

static void
add_package_to_list(cbc_s *base, dbdata_s *data, unsigned long int id);

int
main(int argc, char *argv[])
{
	char error[URL_S], *config;
	int retval = NONE;
	cbc_config_s *cmc;
	cbcvari_comm_line_s *cvcl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcvarient main");
	if (!(cvcl = malloc(sizeof(cbcvari_comm_line_s))))
		report_error(MALLOC_FAIL, "cvcl in cbcvarient main");
	config = cmdb_malloc(CONF_S, "config in main");
	get_config_file_location(config);
	init_cbcvari_config(cmc, cvcl);
	if ((retval = parse_cbcvarient_comm_line(argc, argv, cvcl)) != 0) {
		free(cmc);
		free(cvcl);
		display_command_line_error(retval, argv[0]);
	}
	if (strncmp(cvcl->varient, "NULL", COMM_S) != 0)
		snprintf(error, URL_S, "name %s", cvcl->varient);
	else if (strncmp(cvcl->valias, "NULL", COMM_S) != 0)
		snprintf(error, URL_S, "alias %s", cvcl->valias);
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cvcl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
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
	if (retval != 0) {
		if (retval == OS_NOT_FOUND) {
			if (strncmp(cvcl->os, "NULL", COMM_S) != 0)
				snprintf(error, HOST_S, "%s", cvcl->os);
			else if (strncmp(cvcl->alias, "NULL", COMM_S) != 0)
				snprintf(error, HOST_S, "alias %s", cvcl->alias);
		} else if (retval == NO_RECORDS) {
			free(cmc);
			free(cvcl);
			exit (retval);
		}
		free(cmc);
		free(cvcl);
		report_error(retval, error);
	}
	free(config);
	free(cmc);
	free(cvcl);
	exit (retval);
}

static void
init_cbcvari_config(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	init_cbc_config_values(cmc);
	init_cbcvari_comm_line(cvl);
}

static void
init_cbcvari_comm_line(cbcvari_comm_line_s *cvl)
{
	memset(cvl, 0, sizeof(cbcvari_comm_line_s));
	snprintf(cvl->alias, MAC_S, "NULL");
	snprintf(cvl->arch, RANGE_S, "NULL");
	snprintf(cvl->os, MAC_S, "NULL");
	snprintf(cvl->ver_alias, MAC_S, "NULL");
	snprintf(cvl->version, MAC_S, "NULL");
	snprintf(cvl->varient, HOST_S, "NULL");
	snprintf(cvl->valias, MAC_S, "NULL");
	snprintf(cvl->package, HOST_S, "NULL");
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
			snprintf(cvl->ver_alias, MAC_S, "%s", optarg);
		else if (opt == 'k')
			snprintf(cvl->valias, MAC_S, "%s", optarg);
		else if (opt == 'n')
			snprintf(cvl->os, MAC_S, "%s", optarg);
		else if (opt == 'o')
			snprintf(cvl->version, MAC_S, "%s", optarg);
		else if (opt == 'p')
			snprintf(cvl->package, HOST_S, "%s", optarg);
		else if (opt == 's')
			snprintf(cvl->alias, MAC_S, "%s", optarg);
		else if (opt == 't')
			snprintf(cvl->arch, RANGE_S, "%s", optarg);
		else if (opt == 'x')
			snprintf(cvl->varient, HOST_S, "%s", optarg);
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
	if (cvl->action != LIST_CONFIG &&
		(strncmp(cvl->varient, "NULL", COMM_S) == 0) &&
		(strncmp(cvl->valias, "NULL", COMM_S) == 0))
		return NO_VARIENT;
	if (cvl->action == ADD_CONFIG && (cvl->type == CVARIENT) &&
		((strncmp(cvl->varient, "NULL", COMM_S) == 0) ||
		 (strncmp(cvl->valias, "NULL", COMM_S) == 0))) {
		fprintf(stderr, "\
You need to supply both a varient name and valias when adding\n");
		return DISPLAY_USAGE;
	}
	if (cvl->type == CPACKAGE) {
		if (strncmp(cvl->package, "NULL", COMM_S) == 0)
			return NO_PACKAGE;
		if ((cvl->action != ADD_CONFIG) && (cvl->action != RM_CONFIG)) {
			fprintf(stderr, "Can only add or remove packages\n");
			return WRONG_ACTION;
		}
		if ((strncmp(cvl->os, "NULL", MAC_S) == 0) &&
		    (strncmp(cvl->alias, "NULL", MAC_S) == 0))
			return NO_OS_COMM;
	}
	return NONE;
}

static int
list_cbc_build_varient(cbc_config_s *cmc)
{
	int retval = NONE;
	cbc_s *base = NULL;
	cbc_varient_s *list = NULL;
	time_t create;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in list_cbc_build_varient");
	init_cbc_struct(base);
	if ((retval = cbc_run_query(cmc, base, VARIENT)) != 0) {
		if (retval == 6)
			fprintf(stderr, "No build varients in DB\n");
		clean_cbc_struct(base);
		return retval;
	}
	if (base->varient) {
		list = base->varient;
	} else {
		printf("No build varients??\n");
		clean_cbc_struct(base);
		return VARIENT_NOT_FOUND;
	}
	printf("Alias\t\tName\t\t\tUser\t\tLast Modified\n");
	while (list) {
		create = (time_t)list->mtime;
		if (strlen(list->valias) < 8)
			printf("%s\t\t", list->valias);
		else if (strlen(list->valias) < 16)
			printf("%s\t", list->valias);
		else
			printf("%s\n\t\t\t\t", list->valias);
		if (strlen(list->varient) < 8)
			printf("%s\t\t\t", list->varient);
		else if (strlen(list->varient) < 16)
			printf("%s\t\t", list->varient);
		else if (strlen(list->varient) < 24)
			printf("%s\t", list->varient);
		else
			printf("%s\n\t\t\t\t\t", list->varient);
		if (strlen(get_uname(list->cuser)) < 8)
			printf("%s\t\t", get_uname(list->cuser));
		else if (strlen(get_uname(list->cuser)) < 16)
			printf("%s\t", get_uname(list->cuser));
		else
			printf("%s\n\t\t\t\t\t\t", get_uname(list->cuser));
		printf("%s", ctime(&create));
		list = list->next;
	}
	clean_cbc_struct(base);
	return retval;
}

static int
display_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	int retval = NONE;
	unsigned long int id;
	cbc_s *base;

	if ((strncmp(cvl->varient, "NULL", COMM_S)) != 0) {
		if ((retval = get_varient_id(cmc, cvl->varient, &id)) != 0)
			return retval;
	} else if ((strncmp(cvl->valias, "NULL", COMM_S)) != 0) {
		if ((retval = get_varient_id(cmc, cvl->valias, &id)) != 0)
			return retval;
	}
	initialise_cbc_s(&base);
	if ((retval = cbc_run_multiple_query(cmc, base, BUILD_OS | BPACKAGE | BUILD_TYPE)) != 0) {
		clean_cbc_struct(base);
		return MY_QUERY_FAIL;
	}
	if ((retval = check_build_os(cmc, base, cvl)) != 0)
		return retval;
	if (!(base->package)) {
		clean_cbc_struct(base);
		return NO_BUILD_PACKAGES;
	}
	if ((strncmp(cvl->os, "NULL", COMM_S) == 0) &&
	    (strncmp(cvl->alias, "NULL", COMM_S) == 0))
		retval = display_all_os_packages(base, id, cvl);
	else
		retval = display_one_os_packages(base, id, cvl);

	clean_cbc_struct(base);
	return retval;
}

static int
add_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
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
remove_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
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

static int
check_build_os(cbc_config_s *cbc, cbc_s *base, cbcvari_comm_line_s *cvl)
{
	int retval = 0;
	int flag = 0;
	cbc_build_type_s *bt;

	if (strncmp(cvl->alias, "NULL", COMM_S) == 0) {
		if (strncmp(cvl->os, "NULL", COMM_S) == 0)
			return retval;		// No OS specified
		if ((retval = get_os_alias(cbc, cvl->os, cvl->alias)) != 0)
			return retval;
	}
	if (base->btype) {
		bt = base->btype;
	} else {
		fprintf(stderr, "No build types in database?\n");
		exit(BUILD_TYPE_NOT_FOUND);
	}
	while (bt) {
		if (strncmp(bt->alias, cvl->alias, MAC_S) == 0) {
			flag = 1;
			break;
		}
		bt = bt->next;
	}
	if (flag == 0)
		retval = BUILD_TYPE_NOT_FOUND;
	return retval;
}


static int
display_all_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl)
{
	int retval = 0;
	cbc_build_type_s *bt = base->btype;

	if (strncmp(cvl->version, "NULL", COMM_S) != 0)
		return OS_NO_VERSION;
	printf("Displaying all OS build packages\n");
	while (bt) {
		snprintf(cvl->alias, MAC_S, "%s", bt->alias);
		retval = display_one_os_packages(base, id, cvl);
		if ((retval != 0) && (retval != SERVER_PACKAGES_NOT_FOUND))
			return retval;
		bt = bt->next;
	}
	if (retval == SERVER_PACKAGES_NOT_FOUND)
		retval = 0;
	return retval;
}

static int
display_one_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl)
{
	int retval = 0;
	int flag = 0;
	unsigned long int osid;
	cbc_build_os_s *bos = base->bos;

	if (!(bos))
		return OS_NOT_FOUND;
	printf("\nDisplaying build packages for os %s\n", cvl->alias);
	if (strncmp(cvl->version, "NULL", COMM_S) != 0) {	// version set
		if (strncmp(cvl->arch, "NULL", COMM_S) != 0) {	// arch set
			printf("Version: %s\tArch: %s\n\t", cvl->version, cvl->arch);
			if ((osid = get_single_os_id(base, cvl)) == 0) {
				return OS_NOT_FOUND;
			}
			if ((retval = display_specific_os_packages(base, id, osid)) == SERVER_PACKAGES_NOT_FOUND)
				fprintf(stderr, "Os has no packages\n");
		} else {					// arch not set
			while (bos) {
				if ((strncmp(cvl->alias, bos->alias, MAC_S) == 0) &&
				    (strncmp(cvl->version, bos->version, MAC_S) == 0)) {
					flag = 1;
					printf("\
Version: %s\tArch: %s\n\t", bos->version, bos->arch);
					if ((retval = display_specific_os_packages(base, id, bos->os_id)) == SERVER_PACKAGES_NOT_FOUND)
						fprintf(stderr, "Os has no packages\n");
				}
				bos = bos->next;
			}
		}
	} else {
		if (strncmp(cvl->arch, "NULL", COMM_S) != 0) {	// arch set
			while (bos) {
				if ((strncmp(cvl->alias, bos->alias, MAC_S) == 0) &&
				    (strncmp(cvl->arch, bos->arch, MAC_S) == 0)) {
					flag = 1;
					printf("\
Version: %s\tArch: %s\n\t", bos->version, bos->arch);
					if ((retval = display_specific_os_packages(base, id, bos->os_id)) == SERVER_PACKAGES_NOT_FOUND)
						fprintf(stderr, "Os has no packages\n");
				}
				bos = bos->next;
			}
		} else {					// arch not set
			while (bos) {
				if (strncmp(cvl->alias, bos->alias, MAC_S) == 0) {
					flag = 1;
					printf("\
Version: %s\tArch: %s\n\t", bos->version, bos->arch);
					if ((retval = display_specific_os_packages(base, id, bos->os_id)) == SERVER_PACKAGES_NOT_FOUND)
						fprintf(stderr, "Os has no packages\n");
				}
				bos = bos->next;
			}
		}
	}
	if (flag == 0)
		printf("\tNo build varient for os %s\n", cvl->alias);
	else
		flag = 0;
	if (retval == SERVER_PACKAGES_NOT_FOUND)
		retval = 0;
	return retval;
}

static int
display_specific_os_packages(cbc_s *base, unsigned long int id, unsigned long int osid)
{
	int i = 0;
	cbc_package_s *pack = base->package;

	while (pack) {
		if (id == pack->vari_id && osid == pack->os_id) {
			i++;
			printf("%s ", pack->package);
		}
		pack = pack->next;
	}
	printf("\n");
	if (i == 0)
		return SERVER_PACKAGES_NOT_FOUND;
	return NONE;
}

static unsigned long int
get_single_os_id(cbc_s *base, cbcvari_comm_line_s *cvl)
{
	cbc_build_os_s *bos = base->bos;
	if (!(bos))
		return NONE;
	while (bos) {
		if ((strncmp(bos->alias, cvl->alias, MAC_S) == 0) &&
		    (strncmp(bos->version, cvl->version, MAC_S) == 0) &&
		    (strncmp(bos->arch, cvl->arch, RANGE_S) == 0))
			return bos->os_id;
		bos = bos->next;
	}
	return NONE;
}

static void
copy_packages_from_base_varient(cbc_config_s *cbc, char *varient)
{
	char *bvar; 
	int retval, packs = 0;
	unsigned long int vid, bvid;
	cbc_s *base;
	cbc_package_s *pack;

	bvar = cmdb_malloc(COMM_S, "bvar in copy_packages_from_base_varient");
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
add_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl)
{
	char *varient;
	int retval = 0, os, packs = 0;
	unsigned long int *osid, vid;
	cbc_s *base;
	cbc_package_s *pack;

	initialise_cbc_s(&base);
	if ((retval = cbc_run_multiple_query(cbc, base, BUILD_OS | VARIENT)) != 0) {
		fprintf(stderr, "Cannot run os and / or varient query\n");
		clean_cbc_struct(base);
		return retval;
	}
	check_for_alias(&varient, cvl->varient, cvl->valias);
	if ((retval = get_varient_id(cbc, varient, &vid)) != 0)
		return retval;
// This will setup the array of osid's in osid if we have more than one version / arch
	if ((os = cbc_get_os(base->bos, cvl, &osid)) == 0)
		return OS_NOT_FOUND;
// Set the last to the varient_id
	*(osid + (unsigned long int)os) = vid;
	if (!(pack = build_package_list(cbc, osid, os, cvl->package))) {
		fprintf(stderr, "No packages to add\n");
		free(osid);
		clean_cbc_struct(base);
		return NONE;
	}
	base->package = pack;
	while (base->package) {
		if ((retval = cbc_run_insert(cbc, base, BPACKAGES)) == 0)
			packs++;
		base->package = base->package->next;
	}
	printf("Inserted %d packages\n", packs);
	if (packs > 0)
		cbc_set_varient_updated(cbc, vid);
	base->package = pack;
	free(osid);
	clean_cbc_struct(base);
	return retval;
}

static int
remove_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl)
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
				if (((strncmp(cvl->alias, list->alias, MAC_S) == 0) ||
                                     (strncmp(cvl->os, list->os, MAC_S) == 0)) &&
				     (strncmp(cvl->arch, list->arch, RANGE_S) == 0) &&
				     (strncmp(cvl->version, list->version, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			} else {
				if (((strncmp(cvl->alias, list->alias, MAC_S) == 0) ||
				     (strncmp(cvl->os, list->os, MAC_S) == 0)) &&
				     (strncmp(cvl->version, list->version, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			}
		} else if (strncmp(cvl->arch, "NULL", COMM_S) != 0) {
			if (((strncmp(cvl->alias, list->alias, MAC_S) == 0) ||
			     (strncmp(cvl->os, list->os, MAC_S) == 0)) &&
			     (strncmp(cvl->arch, list->arch, RANGE_S) == 0)) {
				retval++;
				if (id) {
					*os_id = list->os_id;
					os_id++;
				}
			}
		} else {
			if ((strncmp(cvl->alias, list->alias, MAC_S) == 0) ||
			    (strncmp(cvl->os, list->os, MAC_S) == 0)) {
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

static cbc_package_s *
build_package_list(cbc_config_s *cbc, unsigned long int *os, int nos, char *pack)
{
	int i;
	unsigned long int *osid, vid;
	cbc_package_s *package, *list = NULL, *tmp;

	if (!(os))
		return list;
	else
		osid = os;
	vid = *(osid + (unsigned long int)nos);
	package = list;
	for (i = 0; i < nos; i++) {
		if (check_for_package(cbc, *osid, vid, pack) > 0) {
			osid++;
			continue;
		}
		tmp = cmdb_malloc(sizeof(cbc_package_s), "tmp in build_package_list");
		init_package(tmp);
		if (package) {
			while (list->next)
				list = list->next;
			list->next = tmp;
		} else {
			package = list = tmp;
		}
		snprintf(tmp->package, HOST_S, "%s", pack);
		tmp->vari_id = vid;
		tmp->os_id = *osid;
		tmp->cuser = tmp->muser = (unsigned long int)getuid();
		osid++;
		list = package;
	}
	return list;
}

static dbdata_s *
build_rem_pack_list(cbc_config_s *cbc, unsigned long int *ids, int noids, char *pack)
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
build_copy_package_list(cbc_config_s *cbc, cbc_s *base, uli_t bid, uli_t id)
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
