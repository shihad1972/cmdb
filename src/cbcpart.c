/* 
 *
 *  cbcpart: Create Build Configuration Partition
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 */
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
clean_cbcpart_comm_line(cbcpart_comm_line_s *cpl);

static int
parse_cbcpart_comm_line(int argc, char *argv[], cbcpart_comm_line_s *cpl);

static int
validate_cbcpart_comm_line(cbcpart_comm_line_s *cpl);

static int
validate_cbcpart_user_input(cbcpart_comm_line_s *cpl, int argc);

static int
list_seed_schemes(ailsa_cmdb_s *cbc);

static int
display_full_seed_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
add_scheme_part(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
add_partition_to_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
add_new_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
add_new_partition_option(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_scheme_part(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_partition_from_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

static int
remove_part_option(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl);

int
main (int argc, char *argv[])
{
	int retval = NONE;
	ailsa_cmdb_s *cmc;
	cbcpart_comm_line_s *cpl;
	
	cmc = ailsa_calloc(sizeof(ailsa_cmdb_s), "main");
	cpl = ailsa_calloc(sizeof(cbcpart_comm_line_s), "main");
	if ((retval = parse_cbcpart_comm_line(argc, argv, cpl)) != 0) {
		ailsa_clean_cmdb(cmc);
		clean_cbcpart_comm_line(cpl);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(cmc);
	if (cpl->action == ADD_CONFIG)
		retval = add_scheme_part(cmc, cpl);
	else if (cpl->action == DISPLAY_CONFIG)
		retval = display_full_seed_scheme(cmc, cpl);
	else if (cpl->action == LIST_CONFIG)
		retval = list_seed_schemes(cmc);
	else if (cpl->action == RM_CONFIG)
		retval = remove_scheme_part(cmc, cpl);
	if (retval == WRONG_TYPE)
		ailsa_syslog(LOG_ERR, "Wrong type specified. Neither partition or scheme?\n");
	ailsa_clean_cmdb(cmc);
	clean_cbcpart_comm_line(cpl);
	exit (retval);
}

static void
clean_cbcpart_comm_line(cbcpart_comm_line_s *cpl)
{
	if (cpl->fs)
		my_free(cpl->fs);
	if (cpl->scheme)
		my_free(cpl->scheme);
	if (cpl->option)
		my_free(cpl->option);
	if (cpl->log_vol)
		my_free(cpl->log_vol);
	if (cpl->partition)
		my_free(cpl->partition);
	my_free(cpl);
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
			cpl->fs = ailsa_calloc(RANGE_S, errmsg);
			snprintf(cpl->fs, RANGE_S, "%s", optarg);
		}
		else if (opt == 'g') {
			if (cpl->lvm < 1) {
				ailsa_syslog(LOG_ERR, "LVM not set before logvol\n");
				return DISPLAY_USAGE;
			}
			cpl->log_vol = ailsa_calloc(MAC_S, errmsg);
			snprintf(cpl->log_vol, MAC_S, "%s", optarg);
		} else if (opt == 'n') {
			cpl->scheme = ailsa_calloc(CONF_S, errmsg);
			snprintf(cpl->scheme, CONF_S, "%s", optarg);
		} else if (opt == 'b') {
			cpl->option = ailsa_calloc(CONF_S, errmsg);
			snprintf(cpl->option, CONF_S, "%s", optarg);
		} else if (opt == 't') {
			cpl->partition = ailsa_calloc(RBUFF_S, errmsg);
			snprintf(cpl->partition, RBUFF_S, "%s", optarg);
		} else if (opt == 'i') {
			if ((ailsa_validate_input(optarg, ID_REGEX)) < 0) {
				ailsa_syslog(LOG_ERR, "minimum not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->min = strtoul(optarg, NULL, 10);
		} else if (opt == 'x') {
			if ((ailsa_validate_input(optarg, ID_REGEX)) < 0) {
				ailsa_syslog(LOG_ERR, "maximum not a number?\n");
				return USER_INPUT_INVALID;
			}
			cpl->max = strtoul(optarg, NULL, 10);
		} else if (opt == 'y') {
			if ((ailsa_validate_input(optarg, ID_REGEX)) < 0) {
				ailsa_syslog(LOG_ERR, "priority not a number?\n");
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
			ailsa_syslog(LOG_ERR, "filesystem (-f) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->log_vol) {
		if ((ailsa_validate_input(cpl->log_vol, LOGVOL_REGEX)) < 0) {
			ailsa_syslog(LOG_ERR, "logical volume name (-g) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->partition) {
		if ((ailsa_validate_input(cpl->partition, PATH_REGEX)) < 0) {
			ailsa_syslog(LOG_ERR, "path (-t) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->option) {
		if ((ailsa_validate_input(cpl->option, FS_REGEX)) < 0) {
			ailsa_syslog(LOG_ERR, "partition option (-o) invalid!\n");
			return USER_INPUT_INVALID;
		}
	} else if (cpl->scheme) {
		if ((ailsa_validate_input(cpl->scheme, NAME_REGEX)) < 0) {
			ailsa_syslog(LOG_ERR, "partition scheme name (-n) invalid!\n");
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
			if ((cpl->max == 0) && (cpl->min > 0))
				cpl->max = cpl->min;
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
list_seed_schemes(ailsa_cmdb_s *cbc)
{
	if (!(cbc))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = ailsa_basic_query(cbc, PARTITION_SCHEME_NAMES, l)) != 0) {
		ailsa_syslog(LOG_ERR, "PARTITION_SCHEME_NAMES query failed");
		goto cleanup;
	}
	if (l->total > 0) {
		printf("Partition Schemes\n");
		printf(" LVM\tName\n");
	} else {
		ailsa_syslog(LOG_INFO, "No partition schemes are defined");
		goto cleanup;
	}
	e = l->head;
	while (e) {
		if (!(e->next))
			break;
		if (((ailsa_data_s *)e->next->data)->data->small > 0)
			printf ("  *");
		printf("\t%s\n", ((ailsa_data_s *)e->data)->data->text);
		e = e->next->next;
	}
	cleanup:
		ailsa_list_destroy(l);
		my_free(l);
		return retval;
}

static int
display_full_seed_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
{
	if (!(cbc) || !(cpl))
		return AILSA_NO_DATA;
	if (!(cpl->scheme))
		return AILSA_NO_DATA;
	AILLIST *p = ailsa_db_data_list_init();
	AILLIST *s = ailsa_db_data_list_init();
	AILLIST *a = ailsa_db_data_list_init();
	AILLIST *o = ailsa_db_data_list_init();
	AILELEM *e, *g;
	size_t parts, i, len;
	int retval;
	short int lvm;
	char *str = NULL, *uname = NULL;
	void *data;
	if ((retval = cmdb_add_string_to_list(cpl->scheme, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add scheme name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SEED_SCHEME_ON_NAME, a, s)) != 0) {
		ailsa_syslog(LOG_ERR, "SEED_SCHEME_ON_NAME query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, PARTITIONS_ON_SCHEME_NAME, a, p)) != 0) {
		ailsa_syslog(LOG_ERR, "PARTITIONS_ON_SCHEME_NAME query failed");
		goto cleanup;
	}
	if ((s->total != 4) || ((p->total % 6) != 0)) {
		retval = AILSA_NO_DATA;
		goto cleanup;
	}
	parts = p->total / 6;
	printf("Partitioning scheme: %s, ", cpl->scheme);
	lvm = ((ailsa_data_s *)s->head->next->data)->data->small;
	if (lvm > 0)
		printf("with LVM,");
	else
		printf("no LVM,");
	uname = cmdb_get_uname(((ailsa_data_s *)s->head->next->next->data)->data->number);
#ifdef HAVE_MYSQL
	if (((ailsa_data_s *)s->head->next->next->next->data)->type == AILSA_DB_TIME)
		printf("  Created by: %s @ %s\n\n", uname, ailsa_convert_mysql_time(((ailsa_data_s *)s->head->next->next->next->data)->data->time));
	else
#endif
		printf("  Created by: %s @ %s\n\n", uname, ((ailsa_data_s *)s->head->next->next->next->data)->data->text);
	printf("Mount\t\tFS\tMin\tMax\tOptions\t\t");
	if (lvm > 0)
		printf("\tVolume\n");
	else
		printf("\n");
	e = p->head;
	for (i = 0; i< parts; i++) {
		str = ((ailsa_data_s *)e->next->next->next->next->next->data)->data->text;
		while (a->total > 1) {
			g = a->tail;
			ailsa_list_remove(a, g, &data);
			ailsa_clean_data(data);
		}
		if ((retval = cmdb_add_string_to_list(str, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add partition name into list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cbc, PART_OPTIONS_ON_SCHEME_NAME_AND_PARTITION, a, o)) != 0) {
			ailsa_syslog(LOG_ERR, "PART_OPTIONS_ON_SCHEME_NAME_AND_PARTITION query failed");
			goto cleanup;
		}
		printf("%s\t", str);
		if (strlen(str) < 7)
			printf("\t");
		else if (strlen(str) > 15)
			printf("\n\t\t");
		str = ((ailsa_data_s *)e->next->next->next->data)->data->text;
		printf("%s\t", str);
		printf("%lu\t%lu\t", ((ailsa_data_s *)e->next->data)->data->number,
		  ((ailsa_data_s *)e->next->next->data)->data->number);
		g = o->head;
		len = 0;
		if (o->total == 0) {
			printf("none\t\t\t");
		} else {
			while (g) {
				if (len > 0)
					printf(",");
				len += strlen(((ailsa_data_s *)g->data)->data->text);
				printf("%s", ((ailsa_data_s *)g->data)->data->text);
				g = g->next;
			}
			if (len > 16)
				printf("\t");
			else if (len > 8 )
				printf("\t\t");
			else
				printf("\t\t\t");
			ailsa_list_destroy(o);
			ailsa_list_init(o, ailsa_clean_data);
		}
		if (lvm > 0) {
			str = ((ailsa_data_s *)e->next->next->next->next->data)->data->text;
			printf("%s", str);
		}
		printf("\n");
		e = e->next->next->next->next->next->next;
	}
	cleanup:
		ailsa_list_destroy(p);
		ailsa_list_destroy(s);
		ailsa_list_destroy(a);
		ailsa_list_destroy(o);
		my_free(p);
		my_free(s);
		my_free(a);
		my_free(o);
		if (uname)
			my_free(uname);
		return retval;
}

static int
add_scheme_part(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
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
remove_scheme_part(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
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

static int
add_partition_to_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
{
	if (!(cbc) || !(cpl))
		return AILSA_NO_DATA;
	AILLIST *a = ailsa_db_data_list_init();
	AILLIST *l = ailsa_db_data_list_init();
	int retval;
	size_t total;
	unsigned long int scheme_id;

	if (!(cpl->log_vol))
		cpl->log_vol = strdup("none");
	if ((retval = cmdb_add_string_to_list(cpl->scheme, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add scheme name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SCHEME_LVM_INFO, a, l)) != 0) {
		ailsa_syslog(LOG_ERR, "SCHEME_LVM_INFO query failed");
		goto cleanup;
	}
	if (((ailsa_data_s *)l->head->data)->data->small > 0 && !(cpl->log_vol)) {
		ailsa_syslog(LOG_ERR, "Scheme %s is an LVM scheme. Need a logical volume name!", cpl->scheme);
		goto cleanup;
	}
	total = l->total;
	if ((retval = ailsa_argument_query(cbc, SEED_SCHEME_ON_NAME, a, l)) != 0) {
		ailsa_syslog(LOG_ERR, "SEED_SCHEME_ON_NAME query failed");
		goto cleanup;
	}
	if (l->total == total) {
		ailsa_syslog(LOG_ERR, "Cannot find partition scheme %s in database", cpl->scheme);
		goto cleanup;
	} else {
		scheme_id = ((ailsa_data_s *)l->head->next->data)->data->number;
	}
	if ((retval = cmdb_add_string_to_list(cpl->partition, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add mount point to list");
		goto cleanup;
	}
	total = l->total;
	if ((retval = ailsa_argument_query(cbc, IDENTIFY_PARTITION, a, l)) != 0) {
		ailsa_syslog(LOG_ERR, "IDENTIFY_PARTITION query failed");
		goto cleanup;
	}
	if (l->total > total) {
		ailsa_syslog(LOG_INFO, "Partition %s is already in the scheme %s\n", cpl->partition, cpl->scheme);
		goto cleanup;
	}
	ailsa_list_destroy(l);
	ailsa_list_init(l, ailsa_clean_data);
	if ((retval = cmdb_add_number_to_list(cpl->min, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add minimum to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(cpl->max, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add maximum to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(cpl->pri, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add priority to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cpl->partition, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add partition to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cpl->fs, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add filesystem to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(scheme_id, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add priority to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cpl->log_vol, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add logical volume to list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_PARTITION, l)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_PARTITION query failed");
		goto cleanup;
	}
	if ((retval = set_db_row_updated(cbc, SET_PART_SCHEME_UPDATED, cpl->scheme, 0)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot update the partition scheme in the database");
	cleanup:
		ailsa_list_destroy(a);
		ailsa_list_destroy(l);
		my_free(a);
		my_free(l);
		return retval;
}

static int
add_new_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
{
	if (!(cbc) || !(cpl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *a = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cpl->scheme, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add scheme name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SEED_SCHEME_ON_NAME, a, r)) != 0) {
		ailsa_syslog(LOG_ERR, "SEED_SCHEME_ON_NAME failed");
		goto cleanup;
	}
	if (r->total > 0) {
		ailsa_syslog(LOG_INFO, "Scheme %s already in database", cpl->scheme);
		goto cleanup;
	}
	if ((retval = cmdb_add_short_to_list(cpl->lvm, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add lvm flag to list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot populate cuser and muser");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_SEED_SCHEME, a)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_SEED_SCHEME query failed");
		goto cleanup;
	}
	cleanup:
		ailsa_list_destroy(a);
		ailsa_list_destroy(r);
		my_free(a);
		my_free(r);
		return retval;
}

static int
add_new_partition_option(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
{
	if (!(cbc) || !(cpl))
		return AILSA_NO_DATA;
	int retval;
	char **args = ailsa_calloc((sizeof(char *) * 2), "args in add_new_partition_option");
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();

	args[0] = cpl->scheme;
	args[1] = cpl->partition;
	if ((retval = cmdb_add_scheme_id_to_list(cpl->scheme, cbc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add scheme id to list");
		goto cleanup;
	}
	if (list->total != 1) {
		ailsa_syslog(LOG_ERR, "Scheme list total is not 1?");
		goto cleanup;
	}
	if ((retval = cmdb_add_default_part_id_to_list(args, cbc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add partition id to list");
		goto cleanup;
	}
	if (list->total != 2) {
		ailsa_syslog(LOG_ERR, "Partition list total is not 2?");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cpl->option, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add partition option to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, IDENTIFY_PART_OPTION, list, results)) != 0) {
		ailsa_syslog(LOG_ERR, "IDENTIFY_PART_OPTION query failed");
		goto cleanup;
	}
	if (results->total > 0) {
		ailsa_syslog(LOG_INFO, "Partition option %s already in database for scheme %s, partition %s",
		  cpl->option, cpl->scheme, cpl->partition);
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add muser and cuser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbc, INSERT_PART_OPTION, list)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_PART_OPTION query failed");
		goto cleanup;
	}
	if ((retval = set_db_row_updated(cbc, SET_PART_SCHEME_UPDATED, cpl->scheme, 0)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot update the partition scheme in the database");
	cleanup:
		my_free(args);
		ailsa_list_destroy(list);
		ailsa_list_destroy(results);
		my_free(list);
		my_free(results);
		return retval;
}

static int
remove_partition_from_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
{
	if (!(cbc) || !(cpl))
		return AILSA_NO_DATA;
	int retval;
	char **args = ailsa_calloc((sizeof(char *) * 2), "args in remove_partition_from_scheme");
	AILLIST *list = ailsa_db_data_list_init();

	args[0] = cpl->scheme;
	args[1] = cpl->partition;
	if ((retval = cmdb_add_default_part_id_to_list(args, cbc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get def_part_id for partition");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_PARTITION], list)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot delete partition %s in scheme %s", cpl->partition, cpl->scheme);
	cleanup:
		my_free(args);
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}

static int
remove_scheme(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
{
	int retval = 0;
	AILLIST *list = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cpl->scheme, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to add scheme name to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_SCHEME_ON_NAME], list)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot delete scheme %s from database", cpl->scheme);
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}

static int
remove_part_option(ailsa_cmdb_s *cbc, cbcpart_comm_line_s *cpl)
{
	if (!(cbc) || !(cpl))
		return AILSA_NO_DATA;
	int retval;
	char **args = ailsa_calloc((sizeof(char *) * 2), "args in remove_part_option");
	AILLIST *list = ailsa_db_data_list_init();

	args[0] = cpl->scheme;
	args[1] = cpl->partition;
	if ((retval = cmdb_add_scheme_id_to_list(cpl->scheme, cbc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add scheme id to list");
		goto cleanup;
	}
	if (list->total != 1) {
		ailsa_syslog(LOG_ERR, "Scheme list total is not 1?");
		goto cleanup;
	}
	if ((retval = cmdb_add_default_part_id_to_list(args, cbc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add partition id to list");
		goto cleanup;
	}
	if (list->total != 2) {
		ailsa_syslog(LOG_ERR, "Partition list total is not 2?");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cpl->option, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add partition option to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbc, delete_queries[DELETE_PART_OPTION], list)) != 0)
		ailsa_syslog(LOG_ERR, "DELETE_PART_OPTION query failed");
	cleanup:
		my_free(args);
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}
