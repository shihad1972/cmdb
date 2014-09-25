/* 
 *
 *  cpc: Create Preseed Configuration
 *  Copyright (C) 2014 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cpc.c
 * 
 *  Main source file for cpc
 * 
 *  Part of the cpc program
 * 
 * 
 */
#define _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"
#include "cmdb_cpc.h"
#include "cbc_data.h"
#include "checks.h"

int
main (int argc, char *argv[])
{
	int retval = NONE;
	cpc_config_s *cpc = '\0';
	cpc_comm_line_s *cl = '\0';

	if (!(cpc = malloc(sizeof(cpc_config_s))))
		report_error(MALLOC_FAIL, "cpc in main");
	if (!(cl = malloc(sizeof(cpc_comm_line_s))))
		report_error(MALLOC_FAIL, "cl in main");
	init_cpc_config(cpc);
	init_cpc_comm_line(cl);
	if ((retval = parse_cpc_comm_line(argc, argv, cl)) != 0) {
		clean_cpc_comm_line(cl);
		report_error(retval, "cpc command line");
	}
	cp_cl_to_conf(cl, cpc);
	clean_cpc_comm_line(cl);
	build_preseed(cpc);
	clean_cpc_config(cpc);
	return retval;
}

int
parse_cpc_comm_line(int argc, char *argv[], cpc_comm_line_s *cl)
{
	int opt, retval = NONE;

	while ((opt = getopt(argc, argv, "d:f:n:")) != -1) {
		if (opt == 'd') {
			snprintf(cl->domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'f') {
			snprintf(cl->file, RBUFF_S, "%s", optarg);
		} else if (opt == 'n') {
			snprintf(cl->name, RBUFF_S, "%s", optarg);
		} else {
			fprintf(stderr, "Unknown option %c\n", opt);
			retval = DISPLAY_USAGE;
		}
	}
	return retval;
}

void
cp_cl_to_conf(cpc_comm_line_s *cl, cpc_config_s *cpc)
{
	if (strlen(cl->domain) >0)
		snprintf(cpc->domain, RBUFF_S, "%s", cl->domain);
	else
		snprintf(cpc->domain, RBUFF_S, "domain.lan");
	if (strlen(cl->file) > 0)
		snprintf(cpc->file, RBUFF_S, "%s", cl->file);
	if (strlen(cl->name) > 0)
		snprintf(cpc->name, RBUFF_S, "%s", cl->name);
	else
		snprintf(cpc->name, RBUFF_S, "debian");
}

void
build_preseed(cpc_config_s *cpc)
{
	string_len_s *output;

	if (!(output = malloc(sizeof(string_len_s))))
		report_error(MALLOC_FAIL, "output in output_preseed");
	init_string_len(output);
	add_header(output);
	add_locale(output, cpc);
	add_network(output, cpc);
	printf("%s\n", output->string);
	clean_string_len(output);
}

void
add_header(string_len_s *pre)
{
	snprintf(pre->string, BUFF_S, "\
### Preseed config\n\
## Created by cpc\n\
## Inspired by https://www.debian.org/releases/wheezy/example-preseed.txt\n\
\n");
	pre->size = strlen(pre->string);
}

void
add_locale(string_len_s *pre, cpc_config_s *cpc)
{
	snprintf(pre->string + pre->size, RBUFF_S, "\
d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string en_GB\n\
d-i keyboard-configuration/xkb-keymap select uk\n\
\n");
	pre->size = strlen(pre->string);
}

void
add_network(string_len_s *pre, cpc_config_s *cpc)
{
	size_t size;
	char *buffer;

	if ((asprintf(&buffer, "\
d-i netcfg/enable boolean true\n\
d-i netcfg/choose_interface select %s\n\
d-i netcfg/disable_dhcp boolean false\n\
d-i netcfg/get_hostname string %s\n\
d-i netcfg/get_domain string %s\n\
d-i hw-detect/load_firmware boolean true\n\
\n", cpc->interface, cpc->name, cpc->domain)) == -1) {
		report_error(MALLOC_FAIL, "buffer in add_network");
	} else {
		size = strlen(buffer);
		if (size + pre->size >= pre->len) {
			resize_string_buff(pre);
		}
		snprintf(pre->string + pre->size, size + 1, "%s", buffer);
	}
	free(buffer);
}


void
init_cpc_config(cpc_config_s *cpc)
{
	memset(cpc, 0, sizeof(cpc_config_s));
	if (!(cpc->domain = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->domain init");
	if (!(cpc->interface = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->interface init");
	if (!(cpc->file = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->file init");
	if (!(cpc->name = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->name init");
}

void
clean_cpc_config(cpc_config_s *cpc)
{
	if (cpc) {
		if (cpc->domain)
			free(cpc->domain);
		if (cpc->interface)
			free(cpc->interface);
		if (cpc->file)
			free(cpc->file);
		if (cpc->name)
			free(cpc->name);
	} else {
		return;
	}
	free(cpc);
}

void
init_cpc_comm_line(cpc_comm_line_s *cl)
{
	memset(cl, 0, sizeof(cpc_comm_line_s));
	if (!(cl->domain = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cl->domain init");
	if (!(cl->file = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cl->file init");
	if (!(cl->name = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cl->name init");
}

void
clean_cpc_comm_line(cpc_comm_line_s *cl)
{
	if (cl) {
		if (cl->domain)
			free(cl->domain);
		if (cl->file)
			free(cl->file);
		if (cl->name)
			free(cl->name);
	} else {
		return;
	}
	free(cl);
}

