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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbc_common.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */
#include "cbcvarient.h"

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	char error[URL_S];
	int retval = NONE;
	cbc_config_s *cmc;
	cbcvari_comm_line_s *cvcl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcvarient main");
	if (!(cvcl = malloc(sizeof(cbcvari_comm_line_s))))
		report_error(MALLOC_FAIL, "cvcl in cbcvarient main");
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
		printf("Cowardly refusal to modify varients\n");
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

	free(cmc);
	free(cvcl);
	exit (retval);
}

void
init_cbcvari_config(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	init_cbc_config_values(cmc);
	init_cbcvari_comm_line(cvl);
}

void
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

int
parse_cbcvarient_comm_line(int argc, char *argv[], cbcvari_comm_line_s *cvl)
{
	int opt;

	while ((opt = getopt(argc, argv, "ade:gjk:lmn:o:p:rs:t:vx:")) != -1) {
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

int
list_cbc_build_varient(cbc_config_s *cmc)
{
	int retval = NONE;
	cbc_s *base = '\0';
	cbc_varient_s *list = '\0';
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
	printf("Build Varients\n");
	printf("Name\t\tAlias\t\tUser\t\tTime\n");
	while (list) {
		create = (time_t)list->ctime;
		if (strlen(list->varient) < 8)
			printf("%s\t\t", list->varient);
		else if (strlen(list->varient) < 16)
			printf("%s\t", list->varient);
		else
			printf("%s\n\t\t", list->varient);
		if (strlen(list->valias) < 8)
			printf("%s\t\t", list->valias);
		else if (strlen(list->valias) < 16)
			printf("%s\t", list->valias);
		else
			printf("%s\n\t\t\t\t", list->valias);
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

int
display_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	int retval = NONE, type = VARIENT_ID_ON_VALIAS;
	char varient[HOST_S];
	unsigned int max;
	unsigned long int id;
	cbc_s *base;
	dbdata_s *data = '\0';

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in display_cbc_build_varient");
	init_cbc_struct(base);
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
	id = data->fields.number;
	if ((retval = cbc_run_multiple_query(cmc, base, BUILD_OS | BPACKAGE)) != 0) {
		clean_dbdata_struct(data);
		clean_cbc_struct(base);
		return MY_QUERY_FAIL;
	}
	if (!(base->package)) {
		clean_cbc_struct(base);
		clean_dbdata_struct(data);
		return NO_BUILD_PACKAGES;
	}
	if ((strncmp(cvl->os, "NULL", COMM_S) == 0) &&
	    (strncmp(cvl->alias, "NULL", COMM_S) == 0))
		retval = display_all_os_packages(base, id, cvl);
	else
		retval = display_one_os_packages(base, id, cvl);

	clean_cbc_struct(base);
	clean_dbdata_struct(data);
	return retval;
}

int
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
	clean_cbc_struct(base);
	return retval;
}

int
remove_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	char varient[HOST_S];
	int retval = NONE, type = VARIENT_ID_ON_VALIAS;
	unsigned int max;
	dbdata_s *data = '\0';

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

int
display_all_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl)
{
	cbc_build_os_s *bos = base->bos;
	if (!(bos))
		return OS_NOT_FOUND;
	printf("Displaying all OS build packages\n");
	while (bos) {
		snprintf(cvl->alias, MAC_S, "%s", bos->alias);
		display_one_os_packages(base, id, cvl);
		while ((bos) && strncmp(cvl->alias, bos->alias, MAC_S) == 0)
			bos = bos->next;
		snprintf(cvl->version, MAC_S, "NULL");
		snprintf(cvl->arch, RANGE_S, "NULL");
	}
	return NONE;
}

int
display_one_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl)
{
	int retval = NONE, i = NONE;
	unsigned long int osid;
	cbc_build_os_s *bos = base->bos;

	if (!(bos))
		return OS_NOT_FOUND;
	if ((strncmp(cvl->os, "NULL", COMM_S) != 0) &&
	    (strncmp(cvl->alias, "NULL", COMM_S) == 0))
		if ((retval = get_os_alias(base, cvl)) != 0)
			return retval;
	while (bos) {
		if ((strncmp(bos->alias, cvl->alias, MAC_S)) == 0)
			i++;
		bos = bos->next;
	}
	bos = base->bos;
	if (i == 0)
		return OS_NOT_FOUND;
	printf("\nDisplaying build packages for os %s\n", cvl->alias);
	if ((strncmp(cvl->version, "NULL", COMM_S) != 0) &&
	    (strncmp(cvl->arch, "NULL", COMM_S) != 0)) {
		printf("Version: %s\tArch: %s\n\t", cvl->version, cvl->arch);
		if ((osid = get_single_os_id(base, cvl)) == 0) {
			return OS_NOT_FOUND;
		}
		display_specific_os_packages(base, id, osid);
	} else if (strncmp(cvl->version, "NULL", COMM_S) != 0) {
		while (bos) {
			if ((strncmp(cvl->alias, bos->alias, MAC_S) == 0) &&
			    (strncmp(cvl->version, bos->version, MAC_S) == 0)) {
				snprintf(cvl->arch, RANGE_S, "%s", bos->arch);
				printf("\
Version: %s\tArch: %s\n\t", cvl->version, cvl->arch);
				osid = get_single_os_id(base, cvl);
				display_specific_os_packages(base, id, osid);
			}
			bos = bos->next;
		}
	} else {
		while (bos) {
			if (strncmp(cvl->alias, bos->alias, MAC_S) == 0) {
				snprintf(cvl->version, MAC_S, "%s", bos->version);
				snprintf(cvl->arch, RANGE_S, "%s", bos->arch);
				printf("\
Version: %s\tArch: %s\n\t", cvl->version, cvl->arch);
				osid = get_single_os_id(base, cvl);
				display_specific_os_packages(base, id, osid);
			}
			bos = bos->next;
		}
	}
	return retval;
}

int
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

int
get_os_alias(cbc_s *base, cbcvari_comm_line_s *cvl)
{
	char *os = cvl->os;
	cbc_build_os_s *bos = base->bos;
	if (!(bos))
		return OS_NOT_FOUND;
	while (bos) {
		if (strncmp(os, bos->os, MAC_S) == 0) {
			snprintf(cvl->alias, MAC_S, "%s", bos->alias);
			return NONE;
		}
		bos = bos->next;
	}
	return OS_NOT_FOUND;
}

unsigned long int
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

int
add_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl)
{
	int retval = 0, os, packs = 0;
	unsigned long int *osid, vid;
	cbc_s *base;
	cbc_package_s *pack;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add_cbc_package");
	init_cbc_struct(base);
	if ((retval = cbc_run_multiple_query(cbc, base, BUILD_OS | VARIENT)) != 0) {
		fprintf(stderr, "Cannot run os and / or varient query\n");
		clean_cbc_struct(base);
		return retval;
	}
	vid = search_for_vid(base->varient, cvl->varient, cvl->valias);
	if (vid == 0) 
		return VARIENT_NOT_FOUND;
	if ((os = cbc_get_os(base->bos, cvl->os, cvl->alias, cvl->arch, cvl->version, &osid)) == 0)
		return OS_NOT_FOUND;
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
	base->package = pack;
	free(osid);
	clean_cbc_struct(base);
	return retval;
}

int
remove_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl)
{
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
	vid = search_for_vid(base->varient, cvl->varient, cvl->valias);
	if (vid == 0) {
		clean_cbc_struct(base);
		return VARIENT_NOT_FOUND;
	}
	if ((os = cbc_get_os(base->bos, cvl->os, cvl->alias, cvl->arch, cvl->version, &osid)) == 0) {
		clean_cbc_struct(base);
		return OS_NOT_FOUND;
	}
	*(osid + (unsigned long int)os) = vid;
	if (!(data = build_rem_pack_list(cbc, osid, os, cvl->package))) {
		fprintf(stderr, "No packages to remove\n");
		free(osid);
		clean_cbc_struct(base);
		return NONE;
	}
	list = data;
	while (list) {
		packid = list->args.number;
		if ((retval = cbc_run_delete(cbc, list, PACK_DEL_PACK_ID)) > 1)
			fprintf(stderr, "Multiple packages deleted on pack id %lu\n", packid);
		else if (retval == 0)
			fprintf(stderr, "Cannot delete package id %lu\n", packid);
		packs += retval;
		list = list->next;
	}
	printf("%d packages deleted\n", packs);
	free(osid);
	clean_cbc_struct(base);
	clean_dbdata_struct(data);
	return 0;
}

int
cbc_get_os(cbc_build_os_s *os, char *name, char *alias, char *arch, char *ver, unsigned long int **id)
{
	int retval = NONE;
	unsigned long int *os_id = '\0';

	if ((retval = cbc_get_os_list(os, name, alias, arch, ver, os_id)) == 0)
		return retval;
	if (!(os_id = calloc((size_t)retval + 1, sizeof(unsigned long int))))
		report_error(MALLOC_FAIL, "os_id = cbc_get_os");
	retval = cbc_get_os_list(os, name, alias, arch, ver, os_id);
	*id = os_id;
	return retval;
}

int
cbc_get_os_list(cbc_build_os_s *os, char *name, char *alias, char *arch, char *ver, unsigned long int *id)
{
	int retval = NONE;
	cbc_build_os_s *list;
	unsigned long int *os_id = id;

	list = os;
	while (list) {
		if (strncmp(ver, "NULL", COMM_S) != 0) {
			if (strncmp(arch, "NULL", COMM_S) != 0) {
				if (((strncmp(alias, list->alias, MAC_S) == 0) ||
                                     (strncmp(name, list->os, MAC_S) == 0)) &&
				     (strncmp(arch, list->arch, RANGE_S) == 0) &&
				     (strncmp(ver, list->version, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			} else {
				if (((strncmp(alias, list->alias, MAC_S) == 0) ||
				     (strncmp(name, list->os, MAC_S) == 0)) &&
				     (strncmp(ver, list->version, MAC_S) == 0)) {
					retval++;
					if (id) {
						*os_id = list->os_id;
						os_id++;
					}
				}
			}
		} else if (strncmp(arch, "NULL", COMM_S) != 0) {
			if (((strncmp(alias, list->alias, MAC_S) == 0) ||
			     (strncmp(name, list->os, MAC_S) == 0)) &&
			     (strncmp(arch, list->arch, RANGE_S) == 0)) {
				retval++;
				if (id) {
					*os_id = list->os_id;
					os_id++;
				}
			}
		} else {
			if ((strncmp(alias, list->alias, MAC_S) == 0) ||
			    (strncmp(name, list->os, MAC_S) == 0)) {
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

cbc_package_s *
build_package_list(cbc_config_s *cbc, unsigned long int *os, int nos, char *pack)
{
	int i;
	unsigned long int *osid, vid;
	cbc_package_s *package, *list = '\0', *tmp;

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
		if (!(tmp = malloc(sizeof(cbc_package_s))))
			report_error(MALLOC_FAIL, "tmp in build_package_list");
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

dbdata_s *
build_rem_pack_list(cbc_config_s *cbc, unsigned long int *ids, int noids, char *pack)
{
	int retval, i;
	unsigned long int vid, *id_list;
	dbdata_s *data, *elem, *list = '\0', *dlist;
	if (!(ids))
		return list;
	if (!(pack))
		return list;
	id_list = ids;
	vid = *(ids + (unsigned long)noids);
	for (i = 0; i < noids; i++) {
		init_multi_dbdata_struct(&data, cbc_search_args[PACK_ID_ON_DETAILS]);
		snprintf(data->args.text, RBUFF_S, "%s", pack);
		data->next->args.number = vid;
		data->next->next->args.number = *id_list;
		if ((retval = cbc_run_search(cbc, data, PACK_ID_ON_DETAILS)) == 1) {
			if (!(elem = malloc(sizeof(dbdata_s))))
				report_error(MALLOC_FAIL, "elem in build_rem_pack_list");
			init_dbdata_struct(elem);
			elem->args.number = data->fields.number;
			if (!(list))
				list = elem;
			else {
				dlist = list;
				while (dlist->next)
					dlist = dlist->next;
				dlist->next = elem;
			}
		}
		id_list++;
		clean_dbdata_struct(data);
	}
	return list;
}

