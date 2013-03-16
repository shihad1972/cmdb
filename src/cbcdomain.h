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
 *  cbcdomain.h
 * 
 *  Header file for data and functions for cbcdomain program
 * 
 *  part of the cbcdomain program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBCDOMAIN_H__
# define __CBCDOMAIN_H__

typedef struct cbcdomain_comm_line_s {
	char domain[RBUFF_S];
	char basedn[NAME_S];
	char binddn[NAME_S];
	char ldapserver[HOST_S];
	char logserver[HOST_S];
	char nfsdomain[CONF_S];
	char ntpserver[HOST_S];
	char smtpserver[HOST_S];
	char xymonserver[HOST_S];
	short int action;
	short int confldap;
	short int ldapssl;
	short int conflog;
	short int confsmtp;
	short int confxymon;
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
init_cbcdomain_config(cbc_config_s *cmc, cbcdomain_comm_line_s *cdcl);

int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl);

int
split_network_args(cbcdomain_comm_line_s *cdl, char *optarg);

#endif /* __CBCDOMAIN_H__ */
