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
#include <ailsacmdb.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"

enum {
	PARTITION = 1,
	SCHEME = 2,
	OPTION = 3
};

typedef struct cbcpart_comm_line_s {
	unsigned long int min;
	unsigned long int max;
	unsigned long int pri;
	unsigned long int scheme_id;
	char *fs;
	char *log_vol;
	char *option;
	char *partition;
	char *scheme;
	short int action;
	short int lvm;
	short int type;
} cbcpart_comm_line_s;

static void
init_cbcpart_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static void
init_cbcpart_comm_line(cbcpart_comm_line_s *cpl);

static void
clean_cbcpart_comm_line(cbcpart_comm_line_s *cpl);

static int
parse_cbcpart_comm_line(int argc, char *argv[], cbcpart_comm_line_s *cpl);

static int
validate_cbcpart_comm_line(cbcpart_comm_line_s *cpl);

static int
validate_cbcpart_user_input(cbcpart_comm_line_s *cpl, int argc);

static int
list_seed_schemes(cbc_config_s *cbc);

static int
display_full_seed_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
add_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
add_partition_to_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
check_cbcpart_lvm(cbcpart_comm_line_s *cpl, short int lvm);

static int
check_cbcpart_names(cbc_pre_part_s *dpart, cbc_pre_part_s *part, cbcpart_comm_line_s *cpl);

static int
add_new_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
add_part_info(cbcpart_comm_line_s *cpl, cbc_pre_part_s *part);

static void
cbcpart_add_part_option(cbc_config_s *cbc, cbc_s *base, cbcpart_comm_line_s *cpl);

static int
add_new_partition_option(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_partition_from_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_part_option(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);
/*
static int
mod_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
modify_partition_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

static int
modify_scheme_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl); */

static void
cbcp_setup_parts(char *p[], cbc_pre_part_s *part, char *opt);

static void
get_opts_for_part(cbc_config_s *cbc, cbc_pre_part_s *part, char *opt);

int
main (int argc, char *argv[])
{
	char *config;
	int retval = NONE;
	cbc_config_s *cmc;
	cbcpart_comm_line_s *cpl;
	
	cmc = cmdb_malloc(sizeof(cbc_config_s), "main");
	cpl = cmdb_malloc(sizeof(cbcpart_comm_line_s), "main");
	config = cmdb_malloc(CONF_S, "config in main");
	get_config_file_location(config);
	init_cbcpart_config(cmc, cpl);
	if ((retval = parse_cbcpart_comm_line(argc, argv, cpl)) != 0) {
		free(config);
		free(cmc);
		free(cpl);
		display_command_line_error(retval, argv[0]);
	}

	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(config);
		free (cpl);
		free (cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (cpl->action == ADD_CONFIG)
		retval = add_scheme_part(cmc, cpl);
	else if (cpl->action == DISPLAY_CONFIG)
		retval = display_full_seed_scheme(cmc, cpl);
/*	else if (cpl->action == MOD_CONFIG)
		retval = mod_scheme_part(cmc, cpl); */
	else if (cpl->action == LIST_CONFIG)
		retval = list_seed_schemes(cmc);
	else if (cpl->action == RM_CONFIG)
		retval = remove_scheme_part(cmc, cpl);
	if (retval == WRONG_TYPE)
		fprintf(stderr, "Wrong type specified. Neither partition or scheme?\n");
	free(cmc);
	clean_cbcpart_comm_line(cpl);
	free(config);
	exit (retval);
}

static void
init_cbcpart_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	init_cbc_config_values(cbc);
	init_cbcpart_comm_line(cpl);
}

static void
init_cbcpart_comm_line(cbcpart_comm_line_s *cpl)
{
	memset(cpl, 0, sizeof(cbcpart_comm_line_s));
}

static void
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

static int
parse_cbcpart_comm_line(int argc, char *argv[], cbcpart_comm_line_s *cpl)
{
	const char *errmsg = "parse_cbcpart_comm_line";
	const char *optstr = "ab:df:g:hi:lmn:oprst:uvx:y:";
	int opt, retval;
	retval = 0;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"partition-option",	required_argument,	NULL,	'b'},
		{"display",		no_argument,		NULL,	'd'},
		{"file-system",		required_argument,	NULL,	'f'},
		{"logical-volume",	required_argument,	NULL,	'g'},
		{"logvol",		required_argument,	NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"min-size",		required_argument,	NULL,	'i'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"scheme-name",		required_argument,	NULL,	'n'},
		{"option",		no_argument,		NULL,	'o'},
		{"partition",		no_argument,		NULL,	'p'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"scheme",		no_argument,		NULL,	's'},
		{"mount-point",		required_argument,	NULL,	't'},
		{"lvm",			no_argument,		NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"max-size",		required_argument,	NULL,	'x'},
		{"priority",		required_argument,	NULL,	'y'},
		{NULL,			0,			NULL,	0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
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
		} else if (opt == 'h') {
			return DISPLAY_USAGE;
		} else if (opt == 'p') {
			cpl->type = PARTITION;
		} else if (opt == 's') {
			cpl->type = SCHEME;
		} else if (opt == 'o') {
			cpl->type = OPTION;
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
		} else if (opt == 'b') {
			cpl->option = cmdb_malloc(CONF_S, errmsg);
			snprintf(cpl->option, CONF_S, "%s", optarg);
		} else if (opt == 't') {
			cpl->partition = cmdb_malloc(RBUFF_S, errmsg);
			snprintf(cpl->partition, RBUFF_S, "%s", optarg);
		} else if (opt == 'i') {
			if ((ailsa_validate_input(optarg, ID_REGEX)) < 0) {
				fprintf(stderr, "minimum not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->min = strtoul(optarg, NULL, 10);
		} else if (opt == 'x') {
			if ((ailsa_validate_input(optarg, ID_REGEX)) < 0) {
				fprintf(stderr, "maximum not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->max = strtoul(optarg, NULL, 10);
		} else if (opt == 'y') {
			if ((ailsa_validate_input(optarg, ID_REGEX)) < 0) {
				fprintf(stderr, "priority not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->pri = strtoul(optarg, NULL, 10);
		} else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
	if ((validate_cbcpart_comm_line(cpl)) != 0)
		return USER_INPUT_INVALID;
	if (argc == 1)
		return DISPLAY_USAGE;
	retval = validate_cbcpart_user_input(cpl, argc);
	return retval;
}

static int
validate_cbcpart_comm_line(cbcpart_comm_line_s *cpl)
{
	int retval = 0;

	if (cpl->fs) {
		if ((ailsa_validate_input(cpl->fs, FS_REGEX)) < 0) {
			fprintf(stderr, "filesystem (-f) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->log_vol) {
		if ((ailsa_validate_input(cpl->log_vol, LOGVOL_REGEX)) < 0) {
			fprintf(stderr, "logical volume name (-g) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->partition) {
		if ((ailsa_validate_input(cpl->partition, PATH_REGEX)) < 0) {
			fprintf(stderr, "path (-t) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->option) {
		if ((ailsa_validate_input(cpl->option, FS_REGEX)) < 0) {
			fprintf(stderr, "partition option (-o) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->scheme) {
		if ((ailsa_validate_input(cpl->scheme, NAME_REGEX)) < 0) {
			fprintf(stderr, "partition scheme name (-n) invalid!\n");
			return USER_INPUT_INVALID;
		}
	}
	return retval;
}

static int
validate_cbcpart_user_input(cbcpart_comm_line_s *cpl, int argc)
{
	int retval = 0;

	if (cpl->action == CVERSION)
		return CVERSION;
	if (cpl->action == NONE && argc != 1)
		return NO_ACTION;
	if ((cpl->action != LIST_CONFIG) && (!(cpl->scheme)))
		return NO_SCHEME_INFO;
	if (cpl->action == ADD_CONFIG || cpl->action == RM_CONFIG ||
	    cpl->action == MOD_CONFIG) {
		if (cpl->type == NONE)
			return NO_TYPE;
		if (!(cpl->partition) && (cpl->type == PARTITION))
			return NO_PARTITION_INFO;
	}
	if (cpl->action == ADD_CONFIG) {
		if (cpl->type == PARTITION) {
			if ((cpl->min == 0) && (cpl->max > 0))
				cpl->min = cpl->max;
			if (cpl->pri == 0)
				cpl->pri = 100;
			if (!(cpl->fs))
				return NO_FILE_SYSTEM;
			if ((cpl->lvm > 0) && !(cpl->log_vol))
				return NO_LOG_VOL;
		}
		if ((cpl->type == OPTION) && !(cpl->option))
			return NO_OPTION;
	}
	return retval;
}

static int
list_seed_schemes(cbc_config_s *cbc)
{
	int retval = NONE;
	cbc_s *base;
	cbc_seed_scheme_s *seeds;

	initialise_cbc_s(&base);
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

static int
display_full_seed_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	char *opt;
	int retval = NONE, i = 0;
	unsigned long int def_id = 0;
	size_t num = 4, j;
	char *p[num];
	size_t len[num];
	time_t create;
	cbc_s *base;
	cbc_seed_scheme_s *seed;
	cbc_pre_part_s *part;

	initialise_cbc_s(&base);
	for (j = 0; j < num; j++)
		len[j] = RBUFF_S;
	initialise_string_array(p, num, len);
	if ((retval = cbc_run_multiple_query(cbc, base, SSCHEME | DPART)) != 0) {
		fprintf(stderr, "Seed scheme and default part query failed\n");
		free(base);
		return retval;
	}
	opt = cmdb_malloc(RBUFF_S, "display_full_seed_scheme");
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
			printf("Mount\t\tFS\tMin\tMax\tOptions\t\t");
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
				get_opts_for_part(cbc, part, opt);
				cbcp_setup_parts(p, part, opt);
				if (seed->lvm > 0)
printf("%s\t%s\t%lu\t%lu\t%s\t%s\n", p[0], p[1], part->min, part->max, p[2], p[3]);
				else
printf("%s\t%s\t%lu\t%lu\t%s\n", p[0], p[1], part->min, part->max, p[2]);
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
	free(opt);
	return retval;
}

static int
add_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;

	if (cpl->type == PARTITION)
		retval = add_partition_to_scheme(cbc, cpl);
	else if (cpl->type == SCHEME)
		retval = add_new_scheme(cbc, cpl);
	else if (cpl->type == OPTION)
		retval = add_new_partition_option(cbc, cpl);
	else
		retval = WRONG_TYPE;
	return retval;
}

static int
remove_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;

	if (cpl->type == PARTITION)
		retval = remove_partition_from_scheme(cbc, cpl);
	else if (cpl->type == SCHEME)
		retval = remove_scheme(cbc, cpl);
	else if (cpl->type == OPTION)
		retval = remove_part_option(cbc, cpl);
	else
		retval = WRONG_TYPE;
	return retval;
}
/*
static int
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
*/
static int
add_partition_to_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = NONE;
	short int lvm = 0;
	unsigned long int scheme_id = 0;
	cbc_pre_part_s *part, *dpart;
	cbc_s *base;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_scheme_id(cbc, cpl->scheme, &(scheme_id))) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	data->args.number = scheme_id;

	if ((retval = cbc_run_search(cbc, data, LVM_ON_DEF_SCHEME_ID)) > 1)
		fprintf(stderr, "More than one scheme_id %lu in DB>??\n", scheme_id);
	else if (retval != 1)
		fprintf(stderr, "Cannot find scheme_id %lu in DB??\n", scheme_id);
	lvm = data->fields.small;
	clean_dbdata_struct(data);

	initialise_cbc_s(&base);
	if (!(part = malloc(sizeof(cbc_pre_part_s))))
		report_error(MALLOC_FAIL, "part in add_part_to_scheme");
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

static int
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

static int
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

static int
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

static int
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

static void
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

static int
add_new_partition_option(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	cbc_s *base;
	int retval = 0;

	if ((retval = get_scheme_id(cbc, cpl->scheme, &(cpl->scheme_id))) != 0)
		return retval;
	initialise_cbc_s(&base);
	cbcpart_add_part_option(cbc, base, cpl);
	clean_cbc_struct(base);
	return 0;
}

static int
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

static int
remove_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_scheme_id(cbc, cpl->scheme, &(data->args.number))) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
/*	retval = cbc_run_delete(cbc, data, DEF_PART_ON_DEF_ID);
	printf("Removed %d partition(s)\n", retval); */
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

static int
remove_part_option(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;
	dbdata_s *data;
	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_part_opt_id(cbc, cpl->scheme, cpl->partition, cpl->option, &(data->args.number))) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	if ((retval = cbc_run_delete(cbc, data, PART_OPT_ON_ID)) > 1)
		printf("Removed %d options\n", retval);
	else if (retval == 1)
		printf("Remove 1 option\n");
	else
		fprintf(stderr, "No options removed from DB\n");
	clean_dbdata_struct(data);
	return 0;
}
/*
static int
modify_partition_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;
	short int lvm;
	dbdata_s *data;

	if (!(cpl->scheme))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_scheme_id(cbc, cpl->scheme, &(data->args.number))) != 0)
		goto cleanup;
	if ((retval = cbc_run_search(cbc, data, LVM_ON_DEF_SCHEME_ID)) != 0)
		goto cleanup;
	lvm = data->fields.small;
	cleanup:
		clean_dbdata_struct(data);
		return retval;
}

static int
modify_scheme_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;

	return retval;
}
*/
static void
get_opts_for_part(cbc_config_s *cbc, cbc_pre_part_s *part, char *opt)
{
	char *newopt, *optpos;
	int retval = 0, query = PART_OPT_ON_SCHEME_ID;
	unsigned int max = 0;
	size_t len, size;
	dbdata_s *data, *list;

	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = part->id.def_part_id;
	data->next->args.number = part->link_id.def_scheme_id;
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		snprintf(opt, MAC_S, "none");
		clean_dbdata_struct(data);
		return;
	} else {
		retval = 0;
		list = data;
		while (list) {
			if (retval == 0) {
				snprintf(opt, MAC_S, "%s", list->fields.text);
	//			list = list->next;
			} else {
				newopt = list->fields.text;
				size = strlen(opt);
				len = strlen(newopt);
				optpos = opt + size;
				if (len > 0) {
					if  ((len + size) < RBUFF_S)
						snprintf(optpos, len + 2, ",%s", newopt);
					else
						fprintf(stderr, "Too many options!\n");
				}
			}
			retval++;
			list = list->next;
		}
	}
}

static void
cbcp_setup_parts(char *p[], cbc_pre_part_s *part, char *opt)
{
	if (strlen(part->mount) >= 16)
		snprintf(p[0], RBUFF_S, "%s\n\t", part->mount);
	else if (strlen(part->mount) >= 8)
		snprintf(p[0], RBUFF_S, "%s", part->mount);
	else
		snprintf(p[0], RBUFF_S, "%s\t", part->mount);
	snprintf(p[1], RBUFF_S, "%s", part->fs);
	if (strlen(opt) >= 16)
		snprintf(p[2], RBUFF_S, "%s", opt);
	else if (strlen(opt) >= 8)
		snprintf(p[2], RBUFF_S, "%s\t", opt);
	else
		snprintf(p[2], RBUFF_S, "%s\t\t", opt);
	snprintf(p[3], RBUFF_S, "%s", part->log_vol);
}
