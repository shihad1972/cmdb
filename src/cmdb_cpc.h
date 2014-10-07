/*
 *
 *  cpc: Create preseed config
 *  Copyright (C) 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_cpc.h
 *
 *  Main header file for the cpc program
 *
 */

#ifndef __CMDB_CPC_H__
# define __CMDB_CPC_H__
# include "../config.h"

typedef struct cpc_config_s {
	char *disk;
	char *domain;
	char *interface;
	char *file;
	char *kbd;
	char *locale;
	char *mirror;
	char *name;
	char *ntp_server;
	char *packages;
	char *pinstall;
	char *proxy;
	char *rpass;
	char *suite;
	char *tzone;
	char *ugroups;
	char *uid;
	char *uname;
	char *upass;
	char *url;
	char *user;
	short int add_root;
	short int add_user;
	short int encrypt_rpass;
	short int encrypt_upass;
	short int post;
	short int ntp;
	short int recommends;
	short int utc;
} cpc_config_s;

int
parse_cpc_comm_line(int argc, char *argv[], cpc_config_s *cl);

int
parse_cpc_config_file(cpc_config_s *cpc);

int
parse_cpc_environment(cpc_config_s *cpc);

void
fill_default_cpc_config_values(cpc_config_s *cpc);

void
add_header(string_len_s *preseed);

void
add_locale(string_len_s *pre, cpc_config_s *cpc);

void
add_network(string_len_s *pre, cpc_config_s *cpc);

void
add_mirror(string_len_s *pre, cpc_config_s *cpc);

void
add_account(string_len_s *pre, cpc_config_s *cpc);

void
add_root_account(string_len_s *pre, cpc_config_s *cpc);

void
add_no_root_account(string_len_s *pre);

void
add_user_account(string_len_s *pre, cpc_config_s *cpc);

void
add_clock_and_ntp(string_len_s *pre, cpc_config_s *cpc);

void
add_partitions(string_len_s *pre, cpc_config_s *cpc);

void
add_no_recommends(string_len_s *pre, cpc_config_s *cpc);

void
add_apt(string_len_s *pre, cpc_config_s *cpc);

void
add_final(string_len_s *pre, cpc_config_s *cpc);

void
build_preseed(cpc_config_s *cpc);

void
init_cpc_config(cpc_config_s *cpc);

void
fill_default_cpc_config_values(cpc_config_s *cpc);

void
clean_cpc_config(cpc_config_s *cpc);

void
replace_space(char *packages);

#endif /* __CMDB_CPC_H__ */

