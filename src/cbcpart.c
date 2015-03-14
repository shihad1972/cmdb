/* 
 *
 *  cbcpart: Create Build Configuration Partition
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
 *  cbcpart.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  Part of the cbcpart program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
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
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif // HAVE_LIBPCRE
#include "cbcpart.h"

int
main (int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	int retval = NONE;
	cbc_config_s *cmc;
	cbcpart_comm_line_s *cpcl;
	
	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcpart main");
	if (!(cpcl = malloc(sizeof(cbcpart_comm_line_s))))
		report_error(MALLOC_FAIL, "cpcl in cbcpart main");
	init_cbcpart_config(cmc, cpcl);
	if ((retval = parse_cbcpart_comm_line(argc, argv, cpcl)) != 0) {
		free(cmc);
		free(cpcl);
		display_command_line_error(retval, argv[0]);
	}

	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free (cpcl);
		free (cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (cpcl->action == ADD_CONFIG)
		retval = add_scheme_part(cmc, cpcl);
	else if (cpcl->action == DISPLAY_CONFIG)
		retval = display_full_seed_scheme(cmc, cpcl);
	else if (cpcl->action == MOD_CONFIG)
		retval = mod_scheme_part(cmc, cpcl);
	else if (cpcl->action == LIST_CONFIG)
		retval = list_seed_schemes(cmc);
	else if (cpcl->action == RM_CONFIG)
		retval = remove_scheme_part(cmc, cpcl);
	if (retval == WRONG_TYPE)
		fprintf(stderr, "Wrong type specified. Neither partition or scheme?\n");
	free(cmc);
	clean_cbcpart_comm_line(cpcl);
	exit (retval);
}

void
init_cbcpart_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	init_cbc_config_values(cbc);
	init_cbcpart_comm_line(cpl);
}

void
init_cbcpart_comm_line(cbcpart_comm_line_s *cpl)
{
	memset(cpl, 0, sizeof(cbcpart_comm_line_s));
}

void
clean_cbcpart_comm_line(cbcpart_comm_line_s *cpl)
{
	if (cpl->fs)
		free(cpl->fs);
	if (cpl->scheme)
		free(cpl->scheme);
	if (cpl->option)
		free(cpl->option);
	if (cpl->log_vol)
		free(cpl->log_vol);
	if (cpl->partition)
		free(cpl->partition);
	free(cpl);
}

int
parse_cbcpart_comm_line(int argc, char *argv[], cbcpart_comm_line_s *cpl)
{
	int opt, retval = 0;
	const char *errmsg = "parse_cbcpart_comm_line";

	while ((opt = getopt(argc, argv, "adf:g:i:lmn:o:prst:uvx:y:")) != -1) {
		if (opt == 'a') {
			cpl->action = ADD_CONFIG;
		} else if (opt == 'd') {
			cpl->action = DISPLAY_CONFIG;
		} else if (opt == 'l') {
			cpl->action = LIST_CONFIG;
		} else if (opt == 'm') {
			cpl->action = MOD_CONFIG;
		} else if (opt == 'r') {
			cpl->action = RM_CONFIG;
		} else if (opt == 'u') {
			cpl->lvm = TRUE;
		} else if (opt == 'v') {
			cpl->action = CVERSION;
		} else if (opt == 'p') {
			cpl->type = PARTITION;
		} else if (opt == 's') {
			cpl->type = SCHEME;
		} else if (opt == 'f') {
			cpl->fs = cmdb_malloc(RANGE_S, errmsg);
			snprintf(cpl->fs, RANGE_S, "%s", optarg);
		}
		else if (opt == 'g') {
			if (cpl->lvm < 1) {
				fprintf(stderr, "LVM not set before logvol\n");
				return DISPLAY_USAGE;
			}
			cpl->log_vol = cmdb_malloc(MAC_S, errmsg);
			snprintf(cpl->log_vol, MAC_S, "%s", optarg);
		} else if (opt == 'n') {
			cpl->scheme = cmdb_malloc(CONF_S, errmsg);
			snprintf(cpl->scheme, CONF_S, "%s", optarg);
		} else if (opt == 'o') {
			cpl->option = cmdb_malloc(CONF_S, errmsg);
			snprintf(cpl->option, CONF_S, "%s", optarg);
		} else if (opt == 't') {
			cpl->partition = cmdb_malloc(RBUFF_S, errmsg);
			snprintf(cpl->partition, RBUFF_S, "%s", optarg);
#ifdef HAVE_LIBPCRE
		} else if (opt == 'i') {
			if ((validate_user_input(optarg, ID_REGEX)) < 0) {
				fprintf(stderr, "minimum not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->min = strtoul(optarg, NULL, 10);
		} else if (opt == 'x') {
			if ((validate_user_input(optarg, ID_REGEX)) < 0) {
				fprintf(stderr, "maximum not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->max = strtoul(optarg, NULL, 10);
		} else if (opt == 'y') {
			if ((validate_user_input(optarg, ID_REGEX)) < 0) {
				fprintf(stderr, "priority not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->pri = strtoul(optarg, NULL, 10);
#else
		} else if (opt == 'i') {
			cpl->min = strtoul(optarg, NULL, 10);
		} else if (opt == 'x') {
			cpl->max = strtoul(optarg, NULL, 10);
		} else if (opt == 'y') {
			cpl->pri = strtoul(optarg, NULL, 10);
#endif
		} else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
#ifdef HAVE_LIBPCRE
	if ((validate_cbcpart_comm_line(cpl)) != 0)
		return USER_INPUT_INVALID;
#endif // HAVE_LIBPCRE
	if (argc == 1)
		return DISPLAY_USAGE;
	retval = validate_cbcpart_user_input(cpl, argc);
	return retval;
}

#ifdef HAVE_LIBPCRE
int
validate_cbcpart_comm_line(cbcpart_comm_line_s *cpl)
{
	int retval = 0;

	if (cpl->fs) {
		if ((validate_user_input(cpl->fs, FS_REGEX)) < 0) {
			fprintf(stderr, "filesystem (-f) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->log_vol) {
		if ((validate_user_input(cpl->log_vol, LOGVOL_REGEX)) < 0) {
			fprintf(stderr, "logical volume name (-g) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->partition) {
		if ((validate_user_input(cpl->partition, PATH_REGEX)) < 0) {
			fprintf(stderr, "path (-t) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->option) {
		if ((validate_user_input(cpl->option, FS_REGEX)) < 0) {
			fprintf(stderr, "partition option (-o) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->scheme) {
		if ((validate_user_input(cpl->scheme, NAME_REGEX)) < 0) {
			fprintf(stderr, "partition scheme name (-n) invalid!\n");
			return USER_INPUT_INVALID;
		}
	}
	return retval;
}
#endif // HAVE_LIBPCRE

int
validate_cbcpart_user_input(cbcpart_comm_line_s *cpl, int argc)
{
	int retval = 0;

	if (cpl->action == CVERSION)
		return CVERSION;
	if (cpl->action == NONE && argc != 1)
		return NO_ACTION;
	if (cpl->action == ADD_CONFIG || cpl->action == RM_CONFIG ||
	    cpl->action == MOD_CONFIG) {
		if (cpl->type == NONE)
			return NO_TYPE;
		if (!(cpl->partition) && (cpl->type == PARTITION))
			return NO_PARTITION_INFO;
	}
	if (cpl->action == ADD_CONFIG) {
		if ((cpl->min == 0) && (cpl->max > 0))
			cpl->min = cpl->max;
		if (cpl->pri == 0)
			cpl->pri = 100;
		if (!(cpl->fs))
			return NO_FILE_SYSTEM;
		if ((cpl->lvm > 0) && !(cpl->log_vol))
			return NO_LOG_VOL;
	}
	if ((cpl->action != LIST_CONFIG) && 
	    (!(cpl->scheme)))
		return NO_SCHEME_INFO;
	return retval;
}

int
list_seed_schemes(cbc_config_s *cbc)
{
	int retval = NONE;
	cbc_s *base;
	cbc_seed_scheme_s *seeds;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in list_seed_schemes");
	init_cbc_struct(base);
	if ((retval = cbc_run_query(cbc, base, SSCHEME)) != 0) {
		if (retval == 6)
			fprintf(stderr, "No Partition schemes in DB\n");
		else
			fprintf(stderr, "Seed scheme query failed\n");
		free(base);
		return retval;
	}
	seeds = base->sscheme;
	printf("Partition Schemes\n");
	while (seeds) {
		printf("\t%s\n", seeds->name);
		seeds = seeds->next;
	}
	clean_cbc_struct(base);
	return retval;
}

int
display_full_seed_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE, i = 0;
	unsigned long int def_id = 0;
	cbc_s *base;
	cbc_seed_scheme_s *seed;
	cbc_pre_part_s *part;
	time_t create;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in display_full_seed_scheme\n");
	init_cbc_struct(base);
	if ((retval = cbc_run_multiple_query(cbc, base, SSCHEME | DPART)) != 0) {
		fprintf(stderr, "Seed scheme and default part query failed\n");
		free(base);
		return retval;
	}
	seed = base->sscheme;
	while (seed) {
		part = base->dpart;
		if (strncmp(seed->name, cpl->scheme, CONF_S) == 0) {
			i++;
			def_id = seed->def_scheme_id;
			printf("Scheme %s partitions; ", cpl->scheme);
			if (seed->lvm > 0)
				printf("with LVM\n");
			else
				printf("No LVM\n");
			create = (time_t)seed->ctime;
			printf("Created by %s on %s", get_uname(seed->cuser), ctime(&create));
			create = (time_t)seed->mtime;
			printf("Modified by %s on %s", get_uname(seed->muser), ctime(&create));
			printf("Mount\t\tFS\tMin\tMax");
			if (seed->lvm > 0)
				printf("\tVolume\n");
			else
				printf("\n");
		} else {
			seed = seed->next;
			continue;
		}
		while (part) {
			if (def_id == part->link_id.def_scheme_id) {
				if (seed->lvm > 0) {
					if (strlen(part->mount) >= 16)
						printf("%s\n\t\t%s\t%lu\t%lu\t%s\n",
part->mount, part->fs, part->min, part->max, part->log_vol);
					else if (strlen(part->mount) >= 8)
						printf("%s\t%s\t%lu\t%lu\t%s\n",
part->mount, part->fs, part->min, part->max, part->log_vol);
					else
						printf("%s\t\t%s\t%lu\t%lu\t%s\n",
part->mount, part->fs, part->min, part->max, part->log_vol);
				} else {
					if (strlen(part->mount) >= 16)
						printf("%s\n\t\t%s\t%lu\t%lu\n",
part->mount, part->fs, part->min, part->max);
					if (strlen(part->mount) >= 8)
						printf("%s\t%s\t%lu\t%lu\n",
part->mount, part->fs, part->min, part->max);
					else
						printf("%s\t\t%s\t%lu\t%lu\n",
part->mount, part->fs, part->min, part->max);
				}
			}
			part = part->next;
		}
		seed = seed->next;
	}
	if (i == 0) {
		retval = SCHEME_NOT_FOUND;
		printf("No scheme with name %s found\n", cpl->scheme);
	}
	clean_cbc_struct(base);
	return retval;
}

int
add_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;

	if (cpl->type == PARTITION)
		retval = add_partition_to_scheme(cbc, cpl);
	else if (cpl->type == SCHEME)
		retval = add_new_scheme(cbc, cpl);
	else
		retval = WRONG_TYPE;
	return retval;
}

int
remove_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;

	if (cpl->type == PARTITION)
		retval = remove_partition_from_scheme(cbc, cpl);
	else if (cpl->type == SCHEME)
		retval = remove_scheme(cbc, cpl);
	else
		retval = WRONG_TYPE;
	return retval;
}

int
mod_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;

	if (cpl->type == PARTITION)
		retval = modify_partition_config(cbc, cpl);
	else if (cpl->type == SCHEME)
		retval = modify_scheme_config(cbc, cpl);
	else
		retval = WRONG_TYPE;
	return retval;
}

int
add_partition_to_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;
	short int lvm = 0;
	unsigned long int scheme_id = 0;
	cbc_pre_part_s *part, *dpart;
	cbc_s *base;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_scheme_id_on_name(cbc, cpl->scheme, data)) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	scheme_id = data->fields.number;

	if ((retval = cbc_run_search(cbc, data, LVM_ON_DEF_SCHEME_ID)) > 1)
		fprintf(stderr, "More than one scheme_id %lu in DB>??\n", scheme_id);
	else if (retval != 1)
		fprintf(stderr, "Cannot find scheme_id %lu in DB??\n", scheme_id);
	lvm = data->fields.small;
	clean_dbdata_struct(data);

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add_part_to_scheme");
	if (!(part = malloc(sizeof(cbc_pre_part_s))))
		report_error(MALLOC_FAIL, "part in add_part_to_scheme");
	init_cbc_struct(base);
	init_pre_part(part);

	if ((retval = check_cbcpart_lvm(cpl, lvm)) != 0)
		goto cleanup;
	if ((retval = cbc_run_query(cbc, base, DPART)) != 0) {
		if (retval != 6) {
			fprintf(stderr, "Unable to get partitions from DB\n");
			goto cleanup;
		}
	}
	dpart = base->dpart;
	part->link_id.def_scheme_id = scheme_id;
	if (cpl->log_vol)
		snprintf(part->log_vol, MAC_S, "%s", cpl->log_vol);
	else
		snprintf(part->log_vol, MAC_S, "none");

	if ((retval = add_part_info(cpl, part)) != 0) {
		fprintf(stderr, "Unable to add part info for DB insert\n");
		goto cleanup;
	}
	cpl->scheme_id = scheme_id;
	if ((retval = check_cbcpart_names(dpart, part, cpl)) != 0)
		goto cleanup;
	clean_pre_part(base->dpart);
	base->dpart = part;
	if ((retval = cbc_run_insert(cbc, base, DPARTS)) != 0)
		printf("Unable to add partition to DB\n");
	else
		printf("Partition added to DB\n");
	if (cpl->option)
		cbcpart_add_part_option(cbc, base, cpl);
	if (retval == 0) 
		retval = set_scheme_updated(cbc, cpl->scheme);
	base->dpart = NULL;
	goto cleanup;
	cleanup:
		free(part);
		clean_cbc_struct(base);
		return retval;
}

int
check_cbcpart_lvm(cbcpart_comm_line_s *cpl, short int lvm)
{
	if ((lvm > 0) && (cpl->lvm < 1)) {
		printf("Logical volume defined for scheme %s\n", cpl->scheme);
		printf("Please supply logical volume details\n");
		return NO_LOG_VOL;
	}
	if ((lvm < 1) && (cpl->lvm > 0)) {
		printf("You have defined a logical volume, but this ");
		printf("scheme %s does not use it\n", cpl->scheme);
		return EXTRA_LOG_VOL;
	}
	return 0;
}

int
check_cbcpart_names(cbc_pre_part_s *dpart, cbc_pre_part_s *part, cbcpart_comm_line_s *cpl)
{
	while (dpart) {
		if (cpl->scheme_id == dpart->link_id.def_scheme_id) {
			if (strncmp(part->mount, dpart->mount, RANGE_S) == 0) {
				fprintf(stderr, "Partition %s already defined in %s\n",
part->mount, cpl->scheme);
			 	return PARTITION_EXISTS;
			}
			if ((strncmp(part->log_vol, dpart->log_vol, MAC_S) == 0) &&
				cpl->lvm > 0) {
				fprintf(stderr, "Logical volume %s already used in %s\n",
part->log_vol, cpl->scheme);
				return LOG_VOL_EXISTS;
			}
		}
		dpart = dpart->next;
	}
	return NONE;
}

int
add_new_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;
	cbc_seed_scheme_s *scheme;
	cbc_s *base;
	dbdata_s *data;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add_new_scheme");
	if (!(scheme = malloc(sizeof(cbc_seed_scheme_s))))
		report_error(MALLOC_FAIL, "scheme in add_new_scheme");
	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in add_new_scheme");
	init_dbdata_struct(data);
	init_cbc_struct(base);
	init_seed_scheme(scheme);
	base->sscheme = scheme;
	snprintf(data->args.text, RBUFF_S, "%s", cpl->scheme);
	retval = cbc_run_search(cbc, data, DEF_SCHEME_ID_ON_SCH_NAME);
	if (retval > 0) {
		fprintf(stderr, "Scheme %s already in database\n", cpl->scheme);
		clean_dbdata_struct(data);
		clean_cbc_struct(base);
		return SCHEME_EXISTS;
	}
	scheme->lvm = cpl->lvm;
	scheme->cuser = scheme->muser = (unsigned long int)getuid();
	strncpy(scheme->name, cpl->scheme, CONF_S);
	if ((retval = cbc_run_insert(cbc, base, SSCHEMES)) != 0) 
		printf("Unable to add seed scheme to the database\n");
	else
		printf("Added seed scheme %s to database\n", scheme->name);
	clean_dbdata_struct(data);
	clean_cbc_struct(base);
	return retval;
}

int
add_part_info(cbcpart_comm_line_s *cpl, cbc_pre_part_s *part)
{
	int retval = NONE;

	part->min = cpl->min;
	part->max = cpl->max;
	part->pri = cpl->pri;
	if (cpl->partition) {
		snprintf(part->mount, HOST_S, "%s", cpl->partition);
	} else {
		fprintf(stderr, "No Partition??\n");
		return NO_PARTITION_INFO;
	}
	if (cpl->fs) {
		snprintf(part->fs, RANGE_S, "%s", cpl->fs);
	} else {
		fprintf(stderr, "No Filesystem??\n");
		return NO_FILE_SYSTEM;
	}
	return retval;
}

void
cbcpart_add_part_option(cbc_config_s *cbc, cbc_s *base, cbcpart_comm_line_s *cpl)
{
	cbc_part_opt_s *opt;

	initialise_cbc_part_opt(&opt);
	base->part_opt = opt;
	if (get_partition_id(cbc, cpl->scheme, cpl->partition, &(opt->def_part_id)) != 0)
		return;
	opt->def_scheme_id = cpl->scheme_id;
	opt->option = cmdb_malloc(MAC_S, "cbcpart_add_part_option");
	snprintf(opt->option, MAC_S, "%s", cpl->option);
	opt->cuser = opt->muser = (unsigned long int)getuid();
	if (cbc_run_insert(cbc, base, PARTOPTS) != 0)
		fprintf(stderr, "Cannot add partition option to database\n");
	else
		printf("Partition option added to database\n");
}

/*
int
add_part_info(cbcpart_comm_line_s *cpl, cbc_pre_part_s *part)
{
	int retval = NONE, i = NONE;
	char *sbuck, *pbuck;
	sbuck = cpl->partition;
	if (!(pbuck = strchr(cpl->partition, ','))) {
		printf("Invalid input for partition\n");
		printf("You need 5 strings separated by 4 commas\n");
		printf("You have %d commas\n", i);
		return USER_INPUT_INVALID;
	} else {
		i++;
	}
	*pbuck = '\0';
	pbuck++;
	part->min = strtoul(sbuck, NULL, 10);
	sbuck = pbuck;
	if (!(pbuck = strchr(sbuck, ','))) {
		printf("Invalid input for partition\n");
		printf("You need 5 strings separated by 4 commas\n");
		printf("You have %d commas\n", i);
		return USER_INPUT_INVALID;
	} else {
		i++;
	}
	*pbuck = '\0';
	pbuck++;
	part->max = strtoul(sbuck, NULL, 10);
	sbuck = pbuck;
	if (!(pbuck = strchr(sbuck, ','))) {
		printf("Invalid input for partition\n");
		printf("You need 5 strings separated by 4 commas\n");
		printf("You have %d commas\n", i);
		return USER_INPUT_INVALID;
	} else {
		i++;
	}
	*pbuck = '\0';
	pbuck++;
	part->pri = strtoul(sbuck, NULL, 10);
	sbuck = pbuck;
	if (!(pbuck = strchr(sbuck, ','))) {
		printf("Invalid input for partition\n");
		printf("You need 5 strings separated by 4 commas\n");
		printf("You have %d commas\n", i);
		return USER_INPUT_INVALID;
	} else {
		i++;
	}
	*pbuck = '\0';
	pbuck++;
	snprintf(part->mount, HOST_S, "%s", sbuck);
	sbuck = pbuck;
	snprintf(part->fs, RANGE_S, "%s", sbuck);
	part->cuser = part->muser = (unsigned long int)getuid();
	return retval;
} */

int
remove_partition_from_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0, type;
	unsigned int max;
	dbdata_s *data;

	type = DEFP_ID_ON_SCHEME_PART;
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, CONF_S, "%s", cpl->scheme);
	snprintf(data->next->args.text, RBUFF_S, "%s", cpl->partition);
	if ((retval = cbc_run_search(cbc, data, type)) == 0) {
		fprintf(stderr, "Cannot find partition %s in scheme %s\n",
		 data->next->args.text, data->args.text);
		clean_dbdata_struct(data);
		return PARTITON_NOT_FOUND;
	} else if (retval > 1)
		fprintf(stderr, "Multiple partitions found\n");
	clean_dbdata_struct(data->next);
	data->next = NULL;
	memset(data->args.text, 0, RBUFF_S);
	data->args.number = data->fields.number;
	if ((retval = cbc_run_delete(cbc, data, DEF_PART_ON_PART_ID)) == 0) {
		fprintf(stderr, "Partition %s in scheme %s not deleted\n",
		 cpl->partition, cpl->scheme);
		return DB_DELETE_FAILED;
	} else if (retval > 1 ) {
		fprintf(stderr, "Multiple partitions deleted??\n");
	} else {
		printf("Partition %s deleted from scheme %s\n", cpl->partition,
		 cpl->scheme);
	}
	retval = set_scheme_updated(cbc, cpl->scheme);
	clean_dbdata_struct(data);
	return retval;
}

int
remove_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;
	dbdata_s *data;

	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in remove_scheme");
	init_dbdata_struct(data);
	if ((retval = get_scheme_id_on_name(cbc, cpl->scheme, data)) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	retval = cbc_run_delete(cbc, data, DEF_PART_ON_DEF_ID);
	printf("Removed %d partition(s)\n", retval);
	if ((retval = cbc_run_delete(cbc, data, SEED_SCHEME_ON_DEF_ID)) == 0) {
		fprintf(stderr, "No scheme removed\n");
		retval = DB_DELETE_FAILED;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple schemes removed\n");
		retval = 0;
	} else {
		printf("Scheme %s removed\n", cpl->scheme);
		retval = 0;
	}
	clean_dbdata_struct(data);
	return retval;
}

int
get_scheme_id_on_name(cbc_config_s *cbc, char *scheme, dbdata_s *data)
{
	int retval;
	snprintf(data->args.text, RBUFF_S, "%s", scheme);
	if ((retval = cbc_run_search(cbc, data, DEF_SCHEME_ID_ON_SCH_NAME)) == 0) {
		clean_dbdata_struct(data);
		printf("Cannot find scheme %s\n", scheme);
		return SCHEME_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple scheme's found for %s\n", scheme);
		fprintf(stderr, "Using first one found\n");
	}
	memset(data->args.text, 0, RBUFF_S);
	data->args.number = data->fields.number;
	return 0;
}

int
set_scheme_updated(cbc_config_s *cbc, char *scheme)
{
	int retval;
	unsigned long int scheme_id;
	dbdata_s *user;

	if (scheme) {
		init_multi_dbdata_struct(&user, 1);
		if ((retval = get_scheme_id_on_name(cbc, scheme, user)) != 0) {
			clean_dbdata_struct(user);
			return retval;
		}
		scheme_id = user->args.number;
		clean_dbdata_struct(user);
	} else {
		return SCHEME_NOT_FOUND;
	}
	init_multi_dbdata_struct(&user, cbc_update_args[UP_SEEDSCHEME]);
	user->args.number = (unsigned long int)getuid();
	user->next->args.number = scheme_id;
	if ((retval = cbc_run_update(cbc, user, UP_SEEDSCHEME)) == 1) {
		printf("Scheme marked as updated\n");
		retval = 0;
	} else if (retval == 0)
		printf("Scheme not updated\n");
	clean_dbdata_struct(user);
	return retval;
}

int
modify_partition_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;
	unsigned long int scheme_id;
	short int lvm;
	dbdata_s *data;

	if (!(cpl->scheme))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_scheme_id_on_name(cbc, cpl->scheme, data)) != 0)
		goto cleanup;
	scheme_id = data->fields.number;
	if ((retval = cbc_run_search(cbc, data, LVM_ON_DEF_SCHEME_ID)) != 0)
		goto cleanup;
	lvm = data->fields.small;
	cleanup:
		clean_dbdata_struct(data);
		return retval;
}

int
modify_scheme_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;

	return retval;
}

