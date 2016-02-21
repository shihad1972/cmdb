/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  ailsa.c
 *
 *  Contains main() function for ailsa program - this reads the contents
 *  of the database and dumps it out into local files. It will also read
 *  these files it has created to ensure they are correct. This is a
 *  testing program to ensure the file formats are correct
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ailsacmdb.h>
#include "cmdb.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cmdb_base_sql.h"
#include "dnsa_base_sql.h"
#include "cmdb_data.h"
#include "cmdb_dnsa.h"
#include "cmdb_cbc.h"
#include "conversion.h"
#include "fileio.h"

struct comm_line {
	int action;
	int type;
	int option;
};

enum {
	AIL_SERVER = 1,
	AIL_ZONE = 2,
	AIL_BUILD = 3,
	AIL_ALL = 4
};

enum {
	AIL_INPUT = 1,
	AIL_OUTPUT = 2
};

enum {
	AIL_HELP = 1,
	AIL_VERSION = 2
};

static void
display_usage(const char *prog);

static int
parse_command_line(struct comm_line *cm, int argc, char *argv[]);

int
main(int argc, char *argv[])
{
	const char *config_file = "/etc/dnsa/dnsa.conf";
	int retval = 0;
	struct cmdbd_config data;
	struct comm_line cl = { .action = 0, .type = 0, .option = 0};
	size_t len = sizeof data;

	if (argc > 1 && argc < 5) {
		retval = parse_command_line(&cl, argc, argv);
		if (retval != 0)
			exit(retval);
	} else {
		display_usage(argv[0]);
	}

	cmdb_config_s *cmdb = ailsa_calloc(sizeof(cmdb_config_s), "cmdb in main");
	dnsa_config_s *dnsa = ailsa_calloc(sizeof(dnsa_config_s), "dnsa in main");
	cbc_config_s *cbc = ailsa_calloc(sizeof(cbc_config_s), "cbc in main");
	struct all_config all =  { .cmdb = cmdb, .dnsa = dnsa, .cbc = cbc, .cmdbd = &data };
	memset(&data, 0, len);
	cmdbd_parse_config(config_file, &data, len);
	retval = convert_all_config(&all);

	switch (cl.type) {
	case AIL_SERVER:
		if (cl.action == AIL_INPUT)
			retval = read_cmdb(cmdb, data.toplevelos);
		else if (cl.action == AIL_OUTPUT)
			retval = write_cmdb(cmdb, data.toplevelos);
		else
			retval = 1;
		break;
	case AIL_ZONE:
		if (cl.action == AIL_INPUT)
			retval = read_dnsa(dnsa, data.toplevelos);
		else if (cl.action == AIL_OUTPUT)
			retval = write_dnsa(dnsa, data.toplevelos);
		else
			retval = 1;
		break;
	case AIL_BUILD:
		if (cl.action == AIL_INPUT)
			retval = read_cbc(cbc, data.toplevelos);
		else if (cl.action == AIL_OUTPUT)
			retval = write_cbc(cbc, data.toplevelos);
		else
			retval = 1;
		break;
	default:
		retval = 1;
	}
	my_free(cmdb);
	my_free(dnsa);
	my_free(cbc);
	cmdbd_clean_config(&data);
	return retval;
}

static int
parse_command_line(struct comm_line *cm, int argc, char *argv[])
{
	int c, err = 0;
	static const struct option longopts[] = {
		{ "all",	no_argument,	0,	'a' },
		{ "builds",	no_argument,	0,	'b' },
		{ "cmdb",	no_argument,	0,	'c' },
		{ "dnsa",	no_argument,	0,	'z' },
		{ "help",	no_argument,	0,	'h' },
		{ "input",	no_argument,	0,	'i' },
		{ "output",	no_argument,	0,	'o' },
		{ "version",	no_argument,	0,	'v' },
		{ NULL,		0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, "abcdhiov", longopts, NULL)) != -1) {
		switch(c) {
		case 'a':
			if (cm->type == 0)
				cm->type = AIL_ALL;
			else
				err = 1;
			break;
		case 'b':
			if (cm->type == 0)
				cm->type = AIL_BUILD;
			else
				err = 1;
			break;
		case 'c':
			if (cm->type == 0)
				cm->type = AIL_SERVER;
			else
				err = 1;
			break;
		case 'd':
			if (cm->type == 0)
				cm->type = AIL_ZONE;
			else
				err = 1;
			break;
		case 'h':
			if (cm->option == 0)
				cm->option = AIL_HELP;
			else
				err = 1;
			break;
		case 'i':
			if (cm->action == 0)
				cm->action = AIL_INPUT;
			else
				err = 1;
			break;
		case 'o':
			if (cm->action == 0)
				cm->action = AIL_OUTPUT;
			else
				err = 1;
			break;
		case 'v':
			if (cm->option == 0)
				cm->option = AIL_VERSION;
			else
				err = 1;
			break;
		}
		if (err > 0)
			break;
	}
	if (cm->type == 0 && cm->option == 0) {
		fprintf(stderr, "No type specified\n");
		err = 1;
	}
	if (cm->action == 0 && cm->option == 0) {
		fprintf(stderr, "No action specified\n");
		err = 1;
	}
	if (cm->option == AIL_HELP)
		display_usage(argv[0]);
	else if (cm->option == AIL_VERSION)
		display_version(argv[0]);
	return err;
}

static void
display_usage(const char *prog)
{
	fprintf(stderr, "%s -a (--all)|-b (--builds)|-c (--cmdb)|-d (--dnsa) -i (--input) |-o (--output) [-h|-v]\n", prog);
	exit(0);
}

