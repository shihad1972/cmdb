/* 
 *
 *  cbc: Create Build Configuration
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
 *  cbcdomain.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcdomain program
 * 
 */
#include <config.h>
#include <configmake.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb_cbc.h"
#ifdef HAVE_DNSA
# include "cmdb_dnsa.h"
#endif // HAVE_DNSA 

static int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl);

static int
validate_cbcdomain_comm_line(cbcdomain_comm_line_s *cdl);

static int
display_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

static void
display_build_domain_details(AILLIST *domain);

static void
display_build_domain_servers(AILLIST *domain);

static int
list_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

static int
add_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

static int
remove_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

static int
modify_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

static void
cbcdomain_clean_comm_line(cbcdomain_comm_line_s *cdc);

static int
split_network_args(cbcdomain_comm_line_s *cdl, char *netinfo);

static int
write_dhcp_net_config(ailsa_cmdb_s *cbs);

static void
write_dhcp_config_file(ailsa_cmdb_s *cbs, AILLIST *ice, AILLIST *dom);

static int
cbc_populate_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl, AILLIST *dom);

static int
set_default_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

int
main(int argc, char *argv[])
{
	int retval = NONE;
	ailsa_cmdb_s *cmc;
	cbcdomain_comm_line_s *cdcl;

	cmc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmc in main");
	cdcl = ailsa_calloc(sizeof(cbcdomain_comm_line_s), "cdcl in main");
	if ((retval = parse_cbcdomain_comm_line(argc, argv, cdcl)) != 0) {
		cbcdomain_clean_comm_line(cdcl);
		ailsa_clean_cmdb(cmc);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(cmc);
	if (cdcl->action == CMDB_LIST)
		retval = list_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == CMDB_DISPLAY)
		retval = display_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == CMDB_ADD)
		retval = add_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == CMDB_RM)
		retval = remove_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == CMDB_MOD)
		retval = modify_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == CMDB_WRITE)
		retval = write_dhcp_net_config(cmc);
	else if (cdcl->action == CMDB_DEFAULT)
		retval = set_default_cbc_build_domain(cmc, cdcl);
	else
		printf("Unknown Action type\n");

	cbcdomain_clean_comm_line(cdcl);
	ailsa_clean_cmdb(cmc);
	exit(retval);
}

static void
cbcdomain_clean_comm_line(cbcdomain_comm_line_s *cdc)
{
	if (!(cdc))
		return;
	if (cdc->domain)
		my_free(cdc->domain);
	if (cdc->ntpserver)
		my_free(cdc->ntpserver);
	if (cdc->config)
		my_free(cdc->config);
	my_free(cdc);
}

static int
validate_cbcdomain_comm_line(cbcdomain_comm_line_s *cdl)
{
	if (cdl->ntpserver)
		if (ailsa_validate_input(cdl->ntpserver, DOMAIN_REGEX) < 0)
			if (ailsa_validate_input(cdl->ntpserver, IP_REGEX) < 0)
				return NTP_SERVER_INVALID;
	if (cdl->domain)
		if (ailsa_validate_input(cdl->domain, DOMAIN_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
	return 0;
}

static int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl)
{
	const char *optstr = "abdk:lmn:rt:vwz";
	int opt, retval;

	retval = NONE;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"display",		no_argument,		NULL,	'd'},
		{"help",		no_argument,		NULL,	'h'},
		{"network-info",	required_argument,	NULL,	'k'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"build-domain",	required_argument,	NULL,	'n'},
		{"domain",		required_argument,	NULL,	'n'},
		{"name",		required_argument,	NULL,	'n'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"ntp-server",		required_argument,	NULL,	't'},
		{"version",		no_argument,		NULL,	'v'},
		{"write",		no_argument,		NULL,	'w'},
		{"commit",		no_argument,		NULL,	'w'},
		{"set-default",		no_argument,		NULL,	'z'},
		{NULL,			0,			NULL,	0}
	};
	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a') {
			cdl->action = CMDB_ADD;
		} else if (opt == 'd') {
			cdl->action = CMDB_DISPLAY;
		} else if (opt == 'l') {
			cdl->action = CMDB_LIST;
		} else if (opt == 'm') {
			cdl->action = CMDB_MOD;
		} else if (opt == 'r') {
			cdl->action = CMDB_RM;
		} else if (opt == 'w') {
			cdl->action = CMDB_WRITE;
		} else if (opt == 'z') {
			cdl->action = CMDB_DEFAULT;
		} else if (opt == 'k') {
			retval = split_network_args(cdl, optarg);
		} else if (opt == 'n') {
			cdl->domain = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 't') {
			cdl->ntpserver = strndup(optarg, DOMAIN_LEN);
			cdl->confntp = 1;
		} else if (opt == 'v') {
			cdl->action = AILSA_VERSION;
		} else if (opt == 'h') {
			return AILSA_DISPLAY_USAGE;
		} else {
			printf("Unknown option: %c\n", opt);
			return AILSA_DISPLAY_USAGE;
		}
	}
	if ((retval = validate_cbcdomain_comm_line(cdl)) != 0)
		return retval;
	if (argc == 1)
		return AILSA_DISPLAY_USAGE;
	if (cdl->action == AILSA_VERSION)
		return AILSA_VERSION;
	if (cdl->action == NONE)
		return AILSA_NO_ACTION;
	if (cdl->action != CMDB_LIST && cdl->action != CMDB_WRITE &&
	     !(cdl->domain))
		return AILSA_NO_BUILD_DOMAIN;
	if ((cdl->action == CMDB_MOD) && ((cdl->start_ip != 0) ||
		                            (cdl->end_ip != 0) ||
		                            (cdl->netmask != 0) ||
		                            (cdl->gateway != 0) ||
		                            (cdl->ns != 0)))
		return AILSA_NO_MOD_BUILD_DOM_NET;
	return retval;
}

static int
split_network_args(cbcdomain_comm_line_s *cdl, char *netinfo)
{
	unsigned long int ips[4];
	char *ip, *tmp;
	int retval = NONE, delim = ',', i;
	uint32_t ip_addr;

	ip = netinfo;
	for (i = 0; i < 4; i++) {
		if ((tmp = strchr(ip, delim))) {
			*tmp = '\0';
			tmp++;
		} else {
			return BUILD_DOMAIN_NETWORK_INVALID;
		}
		if (ailsa_validate_input(ip, IP_REGEX) < 0)
			return BUILD_DOMAIN_NETWORK_INVALID;
		if (inet_pton(AF_INET, ip, &ip_addr))
			ips[i] = (unsigned long int) htonl(ip_addr);
		else
			return BUILD_DOMAIN_NETWORK_INVALID;
		ip = tmp;
	}
	cdl->start_ip = ips[0];
	cdl->end_ip = ips[1];
	cdl->gateway = ips[2];
	cdl->netmask = ips[3];
	if (ailsa_validate_input(ip, IP_REGEX) < 0)
		return BUILD_DOMAIN_NETWORK_INVALID;
	if (inet_pton(AF_INET, ip, &ip_addr)) {
		cdl->ns = (unsigned long int) htonl(ip_addr);
	} else {
		retval = BUILD_DOMAIN_NETWORK_INVALID;
	}
	return retval;
}

static int
list_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *elem;

	if ((retval = ailsa_basic_query(cbs, BUILD_DOMAIN_NAMES, list)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_NAMES query failed");
		goto cleanup;
	}
	elem = list->head;
	if (list->total == 0) {
		ailsa_syslog(LOG_INFO, "No build domains found");
		goto cleanup;
	}
	while (elem) {
		printf("%s\n", ((ailsa_data_s *)elem->data)->data->text);
		elem = elem->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
		return retval;
}

static int
display_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *dom = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();
	AILLIST *bld = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cdl->domain, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbs, BUILD_DOMAIN_DETAILS_ON_NAME, dom, res)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_DETAILS_ON_NAME query failed");
		goto cleanup;
	}
	if (res->total == 0) {
		ailsa_syslog(LOG_INFO, "Build domain %s does not exist", cdl->domain);
		goto cleanup;
	}
	printf("Details for build domain: %s\n", cdl->domain);
	display_build_domain_details(res);
	if ((retval = ailsa_argument_query(cbs, BUILD_DETAILS_ON_DOMAIN, dom, bld)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DETAILS_ON_DOMAIN query failed");
		goto cleanup;
	}
	printf("\nServers build in domain: %s\n", cdl->domain);
	printf("Server Name\tIP Address\tBuild Varient\n");
	display_build_domain_servers(bld);
	cleanup:
		ailsa_list_full_clean(dom);
		ailsa_list_full_clean(res);
		ailsa_list_full_clean(bld);
		return retval;
}

static void
display_build_domain_details(AILLIST *d)
{
	if (!(d))
		return;
	if (d->total != 11)
		return;
	char ip[SERVICE_LEN];
	char *uname, *cname;
	uint32_t ip_addr;
	AILELEM *e = d->head;

	printf("\nNetwork Configuration:\n");
	memset(ip, 0, SERVICE_LEN);
	ip_addr = htonl((uint32_t)((ailsa_data_s *)e->data)->data->number);
	inet_ntop(AF_INET, &ip_addr, ip, SERVICE_LEN);
	printf("\tStart IP: %s\n", ip);
	e = e->next;
	memset(ip, 0, SERVICE_LEN);
	ip_addr = htonl((uint32_t)((ailsa_data_s *)e->data)->data->number);
	inet_ntop(AF_INET, &ip_addr, ip, SERVICE_LEN);
	printf("\tEnd IP: %s\n", ip);
	e = e->next;
	memset(ip, 0, SERVICE_LEN);
	ip_addr = htonl((uint32_t)((ailsa_data_s *)e->data)->data->number);
	inet_ntop(AF_INET, &ip_addr, ip, SERVICE_LEN);
	printf("\tNetmask: %s\n", ip);
	e = e->next;
	memset(ip, 0, SERVICE_LEN);
	ip_addr = htonl((uint32_t)((ailsa_data_s *)e->data)->data->number);
	inet_ntop(AF_INET, &ip_addr, ip, SERVICE_LEN);
	printf("\tGateway: %s\n", ip);
	e = e->next;
	memset(ip, 0, SERVICE_LEN);
	ip_addr = htonl((uint32_t)((ailsa_data_s *)e->data)->data->number);
	inet_ntop(AF_INET, &ip_addr, ip, SERVICE_LEN);
	printf("\tName Server: %s\n", ip);
	e = e->next;
	if (((ailsa_data_s *)e->data)->data->small > 0) {
		e = e->next;
		printf("NTP Server: %s\n", ((ailsa_data_s *)e->data)->data->text);
	} else {
		e = e->next;
		printf("No NTP configuration\n");
	}
	e = e->next;
	uname = cmdb_get_uname(((ailsa_data_s *)e->data)->data->number);
	printf("Created by:\t%s", uname);
	e = e->next;
#ifdef HAVE_MYSQL
	if (((ailsa_data_s *)e->data)->type == AILSA_DB_TIME)
		printf(" @ %s\n", ailsa_convert_mysql_time(((ailsa_data_s *)e->data)->data->time));
	else
#endif
		printf(" @ %s\n", ((ailsa_data_s *)e->data)->data->text);
	e = e->next;
	cname = cmdb_get_uname(((ailsa_data_s *)e->data)->data->number);
	printf("Modified by:\t%s", cname);
	e = e->next;
#ifdef HAVE_MYSQL
	if (((ailsa_data_s *)e->data)->type == AILSA_DB_TIME)
		printf(" @ %s\n", ailsa_convert_mysql_time(((ailsa_data_s *)e->data)->data->time));
	else
#endif
		printf(" @ %s\n", ((ailsa_data_s *)e->data)->data->text);
	if (uname)
		my_free(uname);
	if (cname)
		my_free(cname);
}

static void
display_build_domain_servers(AILLIST *b)
{
	if (!(b))
		return;
	size_t members = 3;
	if ((b->total % members) != 0)
		return;
	size_t len;
	char *ptr;
	char ip[SERVICE_LEN];
	uint32_t ip_addr;
	AILELEM *e = b->head;
	while (e) {
		memset(ip, 0, SERVICE_LEN);
		ptr = ((ailsa_data_s *)e->data)->data->text;
		len = strlen(ptr);
		if (len < 8)
			printf("%s\t\t", ptr);
		else
			printf("%s\t", ptr);
		e = e->next;
		ip_addr = htonl((uint32_t)((ailsa_data_s *)e->data)->data->number);
		inet_ntop(AF_INET, &ip_addr, ip, SERVICE_LEN);
		printf("%s\t", ip);
		e = e->next;
		ptr = ((ailsa_data_s *)e->data)->data->text;
		printf("%s\n", ptr);
		e = e->next;
	}
}

static int
add_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	AILLIST *d = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	char *domain = cdl->domain;
	int retval = 0;
	unsigned int query;
	unsigned long int ips[] = {cdl->start_ip, cdl->end_ip, cdl->start_ip, cdl->start_ip, cdl->end_ip, cdl->end_ip, 0};

	if (!(cdl->ntpserver))
		cdl->ntpserver = strdup("none");
	if ((retval = cmdb_add_string_to_list(domain, r)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbs, BUILD_DOMAIN_ID_ON_DOMAIN, r, d)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_ID_ON_DOMAIN query failed");
		goto cleanup;
	}
	if (d->total > 0) {
		ailsa_syslog(LOG_ERR, "Build domain %s already exists in database", domain);
		goto cleanup;
	}
	if ((retval = check_for_build_domain_overlap(cbs, ips)) != 0)
		goto cleanup;
	if ((retval = cbc_populate_domain(cbs, cdl, d)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot populate build domain values");
		goto cleanup;
	}
	if ((cdl->confntp))
		query = INSERT_BUILD_DOMAIN;
	else
		query = INSERT_BUILD_DOMAIN_NO_NTP;
	if ((retval = ailsa_insert_query(cbs, INSERT_BUILD_DOMAIN, d)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_BUILD_DOMAIN query failed");
		goto cleanup;
	}
	if ((retval = write_dhcp_net_config(cbs)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot write new dhcpd.networks file\n");
#ifdef HAVE_DNSA
	if ((retval = add_forward_zone(cbs, cdl->domain, "master", NULL)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add DNS domain %s to database", cdl->domain);
		goto cleanup;
	}
#endif // HAVE_DNSA
	cleanup:
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(d);
		return retval;
}

static int
remove_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *dom = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cdl->domain, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain %s to list", cdl->domain);
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbs, delete_queries[DELETE_BUILD_DOMAIN_ON_NAME], dom)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_BUILD_DOMAIN_ON_NAME query failed");
		goto cleanup;
	}
	if ((retval = write_dhcp_net_config(cbs)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot write new dhcpd.networks file\n");

	cleanup:
		ailsa_list_full_clean(dom);
		return retval;
}

static int
modify_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	int retval;
	unsigned long int uid = (unsigned long int)getuid();
	AILLIST *dom = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cdl->ntpserver, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add NTP server to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(uid, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add muser to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cdl->domain, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_update_query(cbs, update_queries[UPDATE_BUILD_DOMAIN], dom)) != 0)
		ailsa_syslog(LOG_ERR, "UPDATE_BUILD_DOMAIN query failed");
	cleanup:
		ailsa_list_full_clean(dom);
		return 0;
}

static int
write_dhcp_net_config(ailsa_cmdb_s *cbs)
{
	if (!(cbs))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *ice = ailsa_iface_list_init();
	AILLIST *dom = ailsa_dhcp_list_init();

	if ((retval = ailsa_get_iface_list(ice)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get interface list");
		goto cleanup;
	}
	if ((retval = ailsa_get_bdom_list(cbs, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get domain list");
		goto cleanup;
	}
	write_dhcp_config_file(cbs, ice, dom);
	cleanup:
		ailsa_list_full_clean(ice);
		ailsa_list_full_clean(dom);
		return retval;
}

static void
write_dhcp_config_file(ailsa_cmdb_s *cbs, AILLIST *ice, AILLIST *dom)
{
	if (!(cbs) || !(ice) || !(dom))
		return;
	int flags, fd;
	char *name = ailsa_calloc(DOMAIN_LEN, "name in write_dhcp_config_file");
	ailsa_dhcp_s *dhcp;
	ailsa_iface_s *iface;
	mode_t um, mask;
	AILELEM *d, *i;

	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((snprintf(name, DOMAIN_LEN, "%s/dhcpd.networks", cbs->dhcpconf)) >= DOMAIN_LEN)
		ailsa_syslog(LOG_ERR, "path truncated in write_dhcp_config_file");
	if ((fd = open(name, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open file dhcpd.networks: %s\n", strerror(errno));
		goto cleanup;
	}
	d = dom->head;
	while (d) {
		i = ice->head;
		dhcp = d->data;
		while (i) {
			iface = i->data;
			if ((iface->nw == dhcp->nw) && (iface->flag == 0)) {
				iface->flag = 1;
				dprintf(fd, "\
  shared-network %s {\n\
	option domain-name-servers %s;\n\
	option domain-search \"%s\";\n\
	option routers %s;\n\
	subnet %s netmask %s {\n\
		authoratative;\n\
		next-server %s;\n\
		filename \"pxelinux.0\";\n\
	}\n\
}\n\n", dhcp->dname, dhcp->nameserver, dhcp->dname, dhcp->gateway, dhcp->network, dhcp->netmask, dhcp->gateway);
			}
			i = i->next;
		}
		d = d->next;
	}
	close(fd);
	mask = umask(um);
	cleanup:
		my_free(name);
}

static int
cbc_populate_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl, AILLIST *dom)
{
	if (!(cbs) || !(cdl) || !(dom))
		return AILSA_NO_DATA;
	int retval;

	if ((retval = cmdb_add_number_to_list(cdl->start_ip, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add start IP to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cdl->end_ip, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add end IP to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cdl->ns, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot nameserver to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cdl->gateway, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add gateway to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cdl->netmask, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add netmask to list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(cdl->domain, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain name to list");
		return retval;
	}
	if ((cdl->confntp)) {
		if ((retval = cmdb_add_short_to_list(cdl->confntp, dom)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add config ntp to list");
			return retval;
		}
		if ((retval = cmdb_add_string_to_list(cdl->ntpserver, dom)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add ntp server name to list");
			return retval;
		}
	}
	if ((retval = cmdb_populate_cuser_muser(dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		return retval;
	}
	return retval;
}

static int
set_default_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	if (!(cdl->domain))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *domain = ailsa_db_data_list_init();
	AILLIST *def = ailsa_db_data_list_init();

	if ((retval = cmdb_add_build_domain_id_to_list(cdl->domain, cbs, domain)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain ID to list");
		goto cleanup;
	}
	if (domain->total == 0) {
		ailsa_syslog(LOG_ERR, "Build domain %s does not exist");
		goto cleanup;
	}
	if ((retval = ailsa_basic_query(cbs, DEFAULT_DOMAIN, def)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_DOMAIN query failed");
		goto cleanup;
	}
	if (def->total == 0) {
		if ((retval = cmdb_populate_cuser_muser(domain)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to domain list");
			goto cleanup;
		}
		if ((retval = ailsa_insert_query(cbs, INSERT_DEFAULT_DOMAIN, domain)) != 0) {
			ailsa_syslog(LOG_ERR, "INSERT_DEFAULT_DOMAIN query failed");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_number_to_list((unsigned long int)getuid(), domain)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add muser to domain list");
			goto cleanup;
		}
		if ((retval = ailsa_update_query(cbs, update_queries[UPDATE_DEFAULT_DOMAIN], domain)) != 0) {
			ailsa_syslog(LOG_ERR, "UPDATE_DEFAULT_DOMAIN query failed");
			goto cleanup;
		}
	}
	cleanup:
		ailsa_list_full_clean(domain);
		ailsa_list_full_clean(def);
		return retval;
}
