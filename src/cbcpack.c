/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcpack.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcpack program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "checks.h"
#include "cbcpack.h"

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	char error[URL_S];
	int retval = NONE;
	cbc_config_s *cmc;
	cbcpack_comm_line_s *cpcl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcpack main");
	if (!(cpcl = malloc(sizeof(cbcpack_comm_line_s))))
		report_error(MALLOC_FAIL, "cpcl in cbcpack main");
	init_cbcpack_config(cmc, cpcl);
	if ((retval = parse_cbcpack_comm_line(argc, argv, cpcl)) != 0) {
		free(cmc);
		free(cpcl);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	if (strncmp(cpcl->package, "NULL", COMM_S) != 0)
		snprintf(error, HOST_S, "%s", cpcl->package);
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cpcl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (cpcl->action == LIST_CONFIG)
		printf("Please use cbcvarient to list packages\n");
	else if (cpcl->action == DISPLAY_CONFIG)
		printf("Please use cbcvarient to display packages\n");
	else if (cpcl->action == MOD_CONFIG)
		printf("Cowardly refusal to modify packages\n");
	else if (cpcl->action == ADD_CONFIG)
		add_package(cmc, cpcl);
	else if (cpcl->action == RM_CONFIG)
		remove_package(cmc, cpcl);
	else
		printf("Unknown action type\n");
	if (retval != NONE) {
		free(cmc);
		free(cpcl);
		report_error(retval, error);
	}

	free(cmc);
	free(cpcl);
	exit(retval);
}

void
init_cbcpack_config(cbc_config_s *cmc, cbcpack_comm_line_s *cpl)
{
	init_cbc_config_values(cmc);
	init_cbcpack_comm_line(cpl);
}

void
init_cbcpack_comm_line(cbcpack_comm_line_s *cpl)
{
	cpl->action = 0;
	snprintf(cpl->alias, MAC_S, "NULL");
	snprintf(cpl->arch, RANGE_S, "NULL");
	snprintf(cpl->os, MAC_S, "NULL");
	snprintf(cpl->ver_alias, MAC_S, "NULL");
	snprintf(cpl->version, MAC_S, "NULL");
	snprintf(cpl->varient, HOST_S, "NULL");
	snprintf(cpl->valias, MAC_S, "NULL");
	snprintf(cpl->package, HOST_S, "NULL");
}

int
parse_cbcpack_comm_line(int argc, char *argv[], cbcpack_comm_line_s *cpl)
{
	int opt;

	while ((opt = getopt(argc, argv, "ade:k:lmn:o:p:rs:t:x:")) != -1) {
		if (opt == 'a')
			cpl->action = ADD_CONFIG;
		else if (opt == 'd')
			cpl->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			cpl->action = LIST_CONFIG;
		else if (opt == 'm')
			cpl->action = MOD_CONFIG;
		else if (opt == 'r')
			cpl->action = RM_CONFIG;
		else if (opt == 'e')
			snprintf(cpl->ver_alias, MAC_S, "%s", optarg);
		else if (opt == 'k')
			snprintf(cpl->valias, MAC_S, "%s", optarg);
		else if (opt == 'n')
			snprintf(cpl->os, MAC_S, "%s", optarg);
		else if (opt == 'o')
			snprintf(cpl->version, MAC_S, "%s", optarg);
		else if (opt == 'p')
			snprintf(cpl->package, HOST_S, "%s", optarg);
		else if (opt == 's')
			snprintf(cpl->alias, MAC_S, "%s", optarg);
		else if (opt == 't')
			snprintf(cpl->arch, RANGE_S, "%s", optarg);
		else if (opt == 'x')
			snprintf(cpl->varient, HOST_S, "%s", optarg);
		else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cpl->action == 0 && argc != 1)
		return NO_ACTION;
	if (strncmp(cpl->package, "NULL", COMM_S) == 0)
		return NO_PACKAGE;
	if ((strncmp(cpl->os, "NULL", COMM_S) == 0) &&
	    (strncmp(cpl->alias, "NULL", COMM_S) == 0))
		return NO_OS_COMM;
	return NONE;
}

int
add_package(cbc_config_s *cmc, cbcpack_comm_line_s *cpl)
{
	int retval = NONE, osnum = NONE, varinum = NONE, packnum = NONE;
	unsigned long int *osid, *variid;
	size_t len;
	cbc_s *base;
	cbc_package_s *pack, *links;
	dbdata_s *data;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add package");
	init_cbc_struct(base);
	cbc_init_initial_dbdata(&data, OS_VARIENT_ID_ON_PACKAGE);
	if ((retval = cbc_run_multiple_query(cmc, base, BUILD_OS | VARIENT)) != 0) {
		printf("Unable to run os and varient query\n");
		free(base);
		return retval;
	}
	if ((osnum = get_os_list_count(cpl, base)) == 0)
		return OS_NOT_FOUND;
	if ((varinum = get_vari_list_count(cpl, base)) == 0)
		return NO_VARIENT;
	
	len = (size_t)osnum;
	if (!(osid = malloc(len * sizeof(unsigned long int))))
		report_error(MALLOC_FAIL, "osid in add_package");
	len = (size_t)varinum;
	if (!(variid = malloc(len * sizeof(unsigned long int))))
		report_error(MALLOC_FAIL, "variid in add_package");
	if ((retval = get_os_list(cpl, base, osid, osnum)) != 0) {
		fprintf(stderr, "Unable to get OS list!\n");
		clean_cbc_struct(base);
		free(osid);
		free(variid);
		return retval;
	}
	if ((retval = get_vari_list(cpl, base, variid, varinum)) != 0) {
		fprintf(stderr, "Unable to get varient list!\n");
		clean_cbc_struct(base);
		free(osid);
		free(variid);
		return retval;
	}
	snprintf(data->args.text, RBUFF_S, "%s", cpl->package);
	if ((packnum = cbc_run_search(cmc, data, OS_VARIENT_ID_ON_PACKAGE)) > 0) {
		retval = check_for_package(osid, osnum, variid, varinum, data);
		if (retval > 0) {
			if (strncmp(cpl->os, "NULL", COMM_S) == 0) 
				printf("Package %s already in a build for alias %s\n",
			    cpl->package, cpl->alias);
			else
				printf("Package %s already in a build for os %s\n",
			    cpl->package, cpl->os);
			free(osid);
			free(variid);
			clean_cbc_struct(base);
			clean_dbdata_struct(data);
			return PACKAGE_EXISTS;
		}
	}
	build_package_list(osid, osnum, variid, varinum, cpl->package, base);
	pack = links = base->package;
	while (pack) {
		base->package = pack;
		printf("OS ID: %lu\tVarient ID: %lu\n", pack->os_id, pack->vari_id);
		if ((retval = cbc_run_insert(cmc, base, BPACKAGES)) != 0) {
			printf("Unable to insert package %s\n", pack->package);
			free(osid);
			free(variid);
			base->package = links;
			clean_cbc_struct(base);
			clean_dbdata_struct(data);
			return DB_INSERT_FAILED;
		}
		pack = pack->next;
	}
	base->package = links;
	free(osid);
	free(variid);
	clean_dbdata_struct(data);
	clean_cbc_struct(base);
	return retval;
}

int
get_os_list(cbcpack_comm_line_s *cpl, cbc_s *cbc, unsigned long int *id, int num)
{
	int retval = NONE, counter = NONE, type = NONE;
	cbc_build_os_s *bos = cbc->bos;

	type = get_comm_line_os_details(cpl);
	if (type == NONE) {
		while (bos) {
			if ((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			    (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) {
				if (counter < num) {
					*id = bos->os_id;
					id++;
					counter++;
				} else {
					retval = TOO_MANY_OS;
				}
			}
			bos = bos->next;
		}
	} else if (type == ARCH) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    (strncmp(cpl->arch, bos->arch, RANGE_S) == 0)) {
				if (counter < num) {
					*id = bos->os_id;
					id++;
					counter++;
				} else {
					retval = TOO_MANY_OS;
				}
			}
			bos = bos->next;
		}
	} else if (type == VER) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    ((strncmp(cpl->version, bos->version, MAC_S) == 0) ||
			     (strncmp(cpl->ver_alias, bos->ver_alias, MAC_S) == 0))) {
				if (counter < num) {
					*id = bos->os_id;
					id++;
					counter++;
				} else {
					retval = TOO_MANY_OS;
				}
			}
			bos = bos->next;
		}
	} else if (type == BOTH) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    ((strncmp(cpl->version, bos->version, MAC_S) == 0) ||
			     (strncmp(cpl->ver_alias, bos->ver_alias, MAC_S) == 0)) &&
			    (strncmp(cpl->arch, bos->arch, RANGE_S) == 0)) {
				if (counter < num) {
					*id = bos->os_id;
					id++;
					counter++;
				} else {
					retval = TOO_MANY_OS;
				}
			}
			bos = bos->next;
		}
	}
	return retval;
}

int
get_comm_line_os_details(cbcpack_comm_line_s *cpl)
{
	int retval = NONE;

	/* See what we are looking for */
	if ((strncmp(cpl->version, "NULL", COMM_S) != 0) ||
	    (strncmp(cpl->ver_alias, "NULL", COMM_S) != 0)) {
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			retval = BOTH;
		else
			retval = VER;
	} else {
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			retval = ARCH;
		else
			retval = NONE;
	}
	return retval;
}

int
get_vari_list(cbcpack_comm_line_s *cpl, cbc_s *cbc, unsigned long int *id, int num)
{
	int retval = NONE, counter = NONE;
	cbc_varient_s *vari = cbc->varient;

	if ((strncmp(cpl->varient, "NULL", COMM_S) == 0) &&
	    (strncmp(cpl->valias, "NULL", COMM_S) == 0)) {
		while (vari) {
			if (counter < num) {
				*id = vari->varient_id;
				id++;
				counter++;
			} else {
				return TOO_MANY_VARIENT;
			}
			vari = vari->next;
		}
	} else {
		while (vari) {
			if ((strncmp(cpl->varient, vari->varient, MAC_S) == 0) ||
			    (strncmp(cpl->valias, vari->valias, MAC_S) == 0)) {
				if (counter < num) {
					*id = vari->varient_id;
					counter++;
				} else {
					return TOO_MANY_VARIENT;
				}
			}
			vari = vari->next;
		}
	}
	return retval;
}

int
get_os_list_count(cbcpack_comm_line_s *cpl, cbc_s *cbc)
{
	int retval = NONE, type;
	cbc_build_os_s *bos = cbc->bos;

	/* See what we are looking for */
	if ((strncmp(cpl->version, "NULL", COMM_S) != 0) ||
	    (strncmp(cpl->ver_alias, "NULL", COMM_S) != 0)) {
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			type = BOTH;
		else
			type = VER;
	} else {
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			type = ARCH;
		else
			type = NONE;
	}
	/* 
	 * Count number of build operating systems we will be inserting into db
	 * Then we can create an array that size.
	 */
	if (type == NONE) {
		while (bos) {
			if ((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			    (strncmp(cpl->alias, bos->alias, MAC_S) == 0))
				retval++;
			bos = bos->next;
		}
	} else if (type == ARCH) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    (strncmp(cpl->arch, bos->arch, RANGE_S) == 0))
				retval++;
			bos = bos->next;
		}
	} else if (type == VER) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    ((strncmp(cpl->version, bos->version, MAC_S) == 0) ||
			     (strncmp(cpl->ver_alias, bos->ver_alias, MAC_S) == 0)))
				retval++;
			bos = bos->next;
		}
	} else if (type == BOTH) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    ((strncmp(cpl->version, bos->version, MAC_S) == 0) ||
			     (strncmp(cpl->ver_alias, bos->ver_alias, MAC_S) == 0)) &&
			    (strncmp(cpl->arch, bos->arch, RANGE_S) == 0))
				retval++;
			bos = bos->next;
		}
	}
	if (retval == 0) {
		printf("Unknown OS: ");
		if (strncmp(cpl->os, "NULL", COMM_S) != 0)
			printf("%s ", cpl->os);
		if (strncmp(cpl->alias, "NULL", COMM_S) != 0)
			printf("%s ", cpl->alias);
		if (strncmp(cpl->version, "NULL", COMM_S) != 0) 
			printf("%s ", cpl->version);
		if (strncmp(cpl->ver_alias, "NULL", COMM_S) != 0)
			printf("%s ", cpl->ver_alias);
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			printf("%s ", cpl->arch);
		printf("\n");
	}
	return retval;
}

int
get_vari_list_count(cbcpack_comm_line_s *cpl, cbc_s *cbc)
{
	int retval = NONE;
	cbc_varient_s *vari = cbc->varient;

	if ((strncmp(cpl->varient, "NULL", COMM_S) != 0) ||
	    (strncmp(cpl->valias, "NULL", COMM_S) != 0)) {
		while (vari) {
			if ((strncmp(vari->varient, cpl->varient, HOST_S) == 0) ||
			    (strncmp(vari->valias, cpl->valias, MAC_S) == 0))
				retval++;
			vari = vari->next;
		}
	} else {
		while (vari) {
			retval++;
			vari = vari->next;
		}
	}
	if (retval == 0) {
		printf("No varients found ");
		if (strncmp(cpl->varient, "NULL", COMM_S) != 0)
			printf("for varient %s\n", cpl->varient);
		else if (strncmp(cpl->valias, "NULL", COMM_S) != 0)
			printf("for valias %s\n", cpl->valias);
		else
			printf("in DB. Please check with cbcvarient -l\n");
	}
	return retval;
}

int
check_for_package(unsigned long int *osid, int osnum, unsigned long int *variid, int varinum, dbdata_s *data)
{
	int retval = NONE, count = NONE, i, j, k;
	unsigned long int *os = osid, *vari = variid;
	dbdata_s *temp = data;
	while (temp) {
		count++;
		if (temp->next) 
			temp = temp->next->next;
	}
	temp = data;
	for (i = 0; i < count; i++) {
		vari = variid;
		for (j = 0; j < varinum; j++) {
			os = osid;
			for (k = 0; k < osnum; k++) {
				if ((*os == temp->fields.number) &&
				    (*vari == temp->next->fields.number))
					retval++;
				os++;
			}
			vari++;
		}
		if (temp->next)
			temp = temp->next->next;
	}

	return retval;
}

void
build_package_list(unsigned long int *os, int osnum, unsigned long int *vari, int varinum, char *package, cbc_s *base)
{
	int i, j;
	unsigned long int *oid, *vid;
	cbc_package_s *pack, *list, *tmp;

	list = '\0';
	pack = '\0';
	vid = vari;
	for (i = 0; i < varinum; i++) {
		oid = os;
		for (j = 0; j < osnum; j++) {
			if (!(tmp = malloc(sizeof(cbc_package_s))))
				report_error(MALLOC_FAIL, "tmp in build_package_list");
			init_package(tmp);
			snprintf(tmp->package, HOST_S, "%s", package);
			tmp->vari_id = *vid;
			tmp->os_id = *oid;
			if (list) {
				pack = list;
				while (pack->next) {
					pack = pack->next;
				}
				pack->next = tmp;
			} else {
				list = tmp;
			}	
			oid++;
		}
		vid++;
	}
	base->package = list;
}

int
remove_package(cbc_config_s *cmc, cbcpack_comm_line_s *cpl)
{
	int retval = NONE, osnum = NONE, varinum = NONE;
	unsigned long int *osid, *variid;
	size_t len;
	cbc_s *base;
	dbdata_s *data = '\0', *list;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add package");
	init_cbc_struct(base);
	if ((retval = 
cbc_run_multiple_query(cmc, base, BUILD_OS | VARIENT | BPACKAGE)) != 0) {
		printf("Unable to run os and varient query\n");
		free(base);
		return retval;
	}
	if ((osnum = get_os_list_count(cpl, base)) == 0)
		return OS_NOT_FOUND;
	if ((varinum = get_vari_list_count(cpl, base)) == 0)
		return NO_VARIENT;
	len = (size_t)osnum;
	if (!(osid = malloc(len * sizeof(unsigned long int))))
		report_error(MALLOC_FAIL, "osid in add_package");
	len = (size_t)varinum;
	if (!(variid = malloc(len * sizeof(unsigned long int))))
		report_error(MALLOC_FAIL, "variid in add_package");
	if ((retval = get_os_list(cpl, base, osid, osnum)) != 0) {
		fprintf(stderr, "Unable to get OS list!\n");
		clean_cbc_struct(base);
		free(osid);
		free(variid);
		return retval;
	}
	if ((retval = get_vari_list(cpl, base, variid, varinum)) != 0) {
		fprintf(stderr, "Unable to get varient list!\n");
		clean_cbc_struct(base);
		free(osid);
		free(variid);
		return retval;
	}
	if ((retval = get_package_rm_list(
cpl->package, base, variid, varinum, osid, osnum, &data)) == 0) {
		fprintf(stderr, "Package %s not in database for os and / \
or varient.\n", cpl->package);
		clean_cbc_struct(base);
		free(osid);
		free(variid);
		return retval;
	}
	list = data;
	while (list) {
		retval = cbc_run_delete(cmc, list, PACK_DEL_PACK_ID);
		if (retval == 1) {
			printf("Removed package id %lu\n", list->args.number);
		} else if (retval == 0) {
			printf("Unable to remove package id %lu\n", 
			       list->args.number);
			retval = CANNOT_DELETE_PACKAGE;
			break;
		} else if (retval > 1) {
			printf("Multiple packages removed for id %lu\n",
			       list->args.number);
		}
		list = list->next;
	}
	free(osid);
	free(variid);
	clean_dbdata_struct(data);
	clean_cbc_struct(base);

	return NONE;
}

int
get_package_rm_list(char *package, cbc_s *base, unsigned long int *vari, int varnum, unsigned long int *os, int osnum, dbdata_s **data)
{
	int retval = NONE, i, j;
	unsigned long int *vid = vari, *oid = os;
	cbc_package_s *pack = base->package;
	dbdata_s *tmp, *list;

	for (i = 0; i < osnum; i++) {
		vid = vari;
		for (j = 0; j < varnum; j++) {
			pack = base->package;
			while (pack) {
				if (
(*oid == pack->os_id) && (*vid == pack->vari_id) && 
(strncmp(pack->package, package, HOST_S) == 0)) {
					if (!(tmp = malloc(sizeof(dbdata_s))))
						report_error(MALLOC_FAIL, "tmp in get rm pack list");
					init_dbdata_struct(tmp);
					tmp->args.number = pack->pack_id;
					list = *data;
					if (list) {
						while (list->next)
							list = list->next;
						list->next = tmp;
					} else 
						*data = tmp;
					retval++;
				}
				pack = pack->next;
			}
			vid++;
		}
		oid++;
	}
	return retval;
}
