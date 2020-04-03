/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcdomain.h
 * 
 *  Header file for data and functions for cbcdomain program
 * 
 *  part of the cbcdomain program
 * 
 */

#ifndef __CBCDOMAIN_H__
# define __CBCDOMAIN_H__
enum {			/* cbcdomain modify types */
	BASEDN = 1,
	BINDDN = 2,
	LDAPSERV = 4,
	LDAPSSL = 8,
	NFSDOM = 16,
	NTPSERV = 32,
	SMTPSERV = 64,
	XYMONSERV = 128,
	LOGSERV = 256
};

typedef struct cbcdomain_comm_line_s {
	char domain[RBUFF_S];
	char ntpserver[RBUFF_S];
/*	char basedn[NAME_S];
	char binddn[NAME_S];
	char ldapserver[RBUFF_S];
	char logserver[RBUFF_S];
	char nfsdomain[RBUFF_S];
	char smtpserver[RBUFF_S];
	char xymonserver[RBUFF_S]; */
	char config[RBUFF_S];
	short int action;
/*	short int confldap;
	short int ldapssl;
	short int conflog;
	short int confsmtp;
	short int confxymon; */
	short int confntp;
	unsigned long int start_ip;
	unsigned long int end_ip;
	unsigned long int netmask;
	unsigned long int gateway;
	unsigned long int ns;
} cbcdomain_comm_line_s;

void
init_cbcdomain_comm_line(cbcdomain_comm_line_s *cdcl);

void
print_cbcdomain_comm_line(cbcdomain_comm_line_s *cdcl);

void
init_cbcdomain_config(ailsa_cmdb_s *cmc, cbcdomain_comm_line_s *cdcl);

int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl);

void
validate_cbcdomain_comm_line(cbcdomain_comm_line_s *cdl);

int
split_network_args(cbcdomain_comm_line_s *cdl, char *optarg);

int
list_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

int
add_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

int
remove_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

int
modify_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

int
write_dhcp_net_config(ailsa_cmdb_s *cbs);

void
display_bdom_servers(ailsa_cmdb_s *cbs, char *domain);

int
fill_dhcp_net_config(string_len_s *conf, cbc_dhcp_s *dh);

void
fill_dhcp_val(cbc_dhcp_s *src, cbc_dhcp_string_s *dst);

void
check_bdom_overlap(ailsa_cmdb_s *cbs, cbc_build_domain_s *bdom);

int
build_dom_overlap(cbc_build_domain_s *list, cbc_build_domain_s *new);

void
fill_bdom_values(cbc_build_domain_s *bdom, cbcdomain_comm_line_s *cdl);

#endif /* __CBCDOMAIN_H__ */
