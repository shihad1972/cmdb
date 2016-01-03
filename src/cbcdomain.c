/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 201r54  Iain M Conochie <iain-AT-thargoid.co.uk>
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
#include "../config.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "cbc_common.h"
#include "cbcnet.h"
#include "cbc_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif // HAVE_LIBPCRE
#include "cbcdomain.h"
#ifdef HAVE_DNSA
# include "cmdb_dnsa.h"
# include "dnsa_base_sql.h"
# include "cbc_dnsa.h"
#endif // HAVE_DNSA 

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	int retval = NONE;
	cbc_config_s *cmc;
	cbcdomain_comm_line_s *cdcl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcdomain main");
	if (!(cdcl = malloc(sizeof(cbcdomain_comm_line_s))))
		report_error(MALLOC_FAIL, "cdcl in cbcdomain main");
	init_cbcdomain_config(cmc, cdcl);
	if ((retval = parse_cbcdomain_comm_line(argc, argv, cdcl)) != 0) {
		free(cdcl);
		free(cmc);
		display_command_line_error(retval, argv[0]);
	}
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cdcl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	else if (cdcl->action == LIST_CONFIG)
		retval = list_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == ADD_CONFIG)
		retval = add_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == RM_CONFIG)
		retval = remove_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == MOD_CONFIG)
		retval = modify_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == WRITE_CONFIG)
		retval = write_dhcp_net_config(cmc);
	else
		printf("Unknown Action type\n");

	free(cdcl);
	free(cmc);
	if (retval > 0)
		report_error(retval, argv[0]);
	exit(retval);
}

void
init_cbcdomain_comm_line(cbcdomain_comm_line_s *cdcl)
{
	memset(cdcl, 0, sizeof (cbcdomain_comm_line_s));
	get_config_file_location(cdcl->config);
	snprintf(cdcl->domain, COMM_S, "NULL");
	snprintf(cdcl->ntpserver, COMM_S, "NULL");
}

void
init_cbcdomain_config(cbc_config_s *cmc, cbcdomain_comm_line_s *cdcl)
{
	init_cbc_config_values(cmc);
	init_cbcdomain_comm_line(cdcl);
}

#ifdef HAVE_LIBPCRE
void
validate_cbcdomain_comm_line(cbcdomain_comm_line_s *cdl)
{
	if (strncmp(cdl->ntpserver, "NULL", COMM_S) != 0)
		if (validate_user_input(cdl->ntpserver, DOMAIN_REGEX) < 0)
			if (validate_user_input(cdl->ntpserver, IP_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "ntp server");
}
#endif // HAVE_LIBPCRE
int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl)
{
	const char *optstr = "ak:lmn:rt:vw";
	int opt, retval;

	retval = NONE;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"help",		no_argument,		NULL,	'h'},
		{"network-info",	required_argument,	NULL,	'k'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"domain-name",		required_argument,	NULL,	'n'},
		{"domain",		required_argument,	NULL,	'n'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"ntp-server",		required_argument,	NULL,	't'},
		{"version",		no_argument,		NULL,	'v'},
		{"write",		no_argument,		NULL,	'w'},
		{"commit",		no_argument,		NULL,	'w'},
		{NULL,			0,			NULL,	0}
	};
	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a') {
			cdl->action = ADD_CONFIG;
		} else if (opt == 'l') {
			cdl->action = LIST_CONFIG;
		} else if (opt == 'm') {
			cdl->action = MOD_CONFIG;
		} else if (opt == 'r') {
			cdl->action = RM_CONFIG;
		} else if (opt == 'w') {
			cdl->action = WRITE_CONFIG;
		} else if (opt == 'k') {
			retval = split_network_args(cdl, optarg);
		} else if (opt == 'n') {
			snprintf(cdl->domain, RBUFF_S, "%s", optarg);
		} else if (opt == 't') {
			snprintf(cdl->ntpserver, RBUFF_S, "%s", optarg);
			cdl->confntp = 1;
		} else if (opt == 'v') {
			cdl->action = CVERSION;
		} else if (opt == 'h') {
			return DISPLAY_USAGE;
		} else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
#ifdef HAVE_LIBPCRE
	validate_cbcdomain_comm_line(cdl);
#endif /* HAVE_LIBPCRE */
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cdl->action == CVERSION)
		return CVERSION;
	if (cdl->action == NONE)
		return NO_ACTION;
	if (cdl->action != LIST_CONFIG && cdl->action != WRITE_CONFIG &&
	     strncmp(cdl->domain, "NULL", COMM_S) == 0)
		return NO_DOMAIN_NAME;
	if ((cdl->action == MOD_CONFIG) && ((cdl->start_ip != 0) ||
		                            (cdl->end_ip != 0) ||
		                            (cdl->netmask != 0) ||
		                            (cdl->gateway != 0) ||
		                            (cdl->ns != 0)))
		return NO_MOD_BUILD_DOM_NET;
	if ((cdl->action == MOD_CONFIG) && (cdl->confntp == 0))
		return NO_NTP_SERVER;
	return retval;
}

int
split_network_args(cbcdomain_comm_line_s *cdl, char *netinfo)
{
	unsigned long int ips[4];
	char *ip, *tmp;
	const char *network = netinfo;
	int retval = NONE, delim = ',', i;
	uint32_t ip_addr;

/* This is taken from the calling function */
	ip = optarg;
	for (i = 0; i < 4; i++) {
		if ((tmp = strchr(network, delim))) {
			*tmp = '\0';
			tmp++;
		} else {
			return USER_INPUT_INVALID;
		}
#ifdef HAVE_LIBPCRE
		if (validate_user_input(ip, IP_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "network");
#endif /* HAVE_LIBPCRE */
		if (inet_pton(AF_INET, ip, &ip_addr))
			ips[i] = (unsigned long int) htonl(ip_addr);
		else
			report_error(USER_INPUT_INVALID, "network");
		network = ip = tmp;
	}
	cdl->start_ip = ips[0];
	cdl->end_ip = ips[1];
	cdl->gateway = ips[2];
	cdl->netmask = ips[3];
	if (inet_pton(AF_INET, ip, &ip_addr)) {
		cdl->ns = (unsigned long int) htonl(ip_addr);
	} else {
		retval = USER_INPUT_INVALID;
	}
	return retval;
}

int
list_cbc_build_domain(cbc_config_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return CBC_NO_DATA;
	char *domain = cdl->domain;
	int retval = 0, i = 0;
	cbc_s *cbc;
	cbc_build_domain_s *bdom;

	initialise_cbc_s(&cbc);
	if ((retval = cbc_run_query(cbs, cbc, BUILD_DOMAIN)) != 0)
		goto cleanup;
	bdom = cbc->bdom;
	if (strncmp(cdl->domain, "NULL", COMM_S) != 0) {
		while (bdom) {
			if (strncmp(bdom->domain, domain, RBUFF_S) == 0) {
				display_build_domain(bdom);
				i++;
				display_bdom_servers(cbs, domain);
			}
			bdom = bdom->next;
		}
		if (i == 0)
			printf("Build domain %s not found\n", cdl->domain);
	} else {
		while (bdom) {
			printf("%s\n", bdom->domain);
			bdom = bdom->next;
		}
	}
	goto cleanup;

	cleanup:
		clean_cbc_struct(cbc);
		return retval;
}

int
add_cbc_build_domain(cbc_config_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return CBC_NO_DATA;
	char *domain = cdl->domain;
	int retval = 0;
	cbc_s *base;
	cbc_build_domain_s *bdom;
	dbdata_s *data;

	initialise_cbc_s(&base);
	if (!(bdom = malloc(sizeof(cbc_build_domain_s))))
		report_error(MALLOC_FAIL, "bdom in add_cbc_build_domain");
	init_build_domain(bdom);
	base->bdom = bdom;
	if ((retval = get_build_domain_id(cbs, domain, &(bdom->bd_id))) == 0) {
		clean_cbc_struct(base);
		report_error(BUILD_DOMAIN_EXISTS, domain);
	}
	init_multi_dbdata_struct(&data, 1);
	fill_bdom_values(bdom, cdl);
	check_bdom_overlap(cbs, bdom);
#ifdef HAVE_DNSA
	size_t dclen = sizeof(dnsa_config_s);
	dnsa_config_s *dc;
	dnsa_s *dnsa;
	dbdata_s *user;
	zone_info_s *zone;

	dc = cmdb_malloc(dclen, "dc in add_cbc_build_domain");
	dnsa = cmdb_malloc(sizeof(dnsa_s), "dnsa in add_cbc_build_domain");
	zone = cmdb_malloc(sizeof(zone_info_s), "zone in add_cbc_build_domain");
	init_multi_dbdata_struct(&user, 1);
	dnsa_init_config_values(dc);
	init_zone_struct(zone);
	dnsa->zones = zone;
	if ((retval = parse_dnsa_config_file(dc, cdl->config)) != 0) {
		fprintf(stderr, "Error in config file %s\n", cdl->config);
		goto dnsa_cleanup;
	}
	fill_cbc_fwd_zone(zone, bdom->domain, dc);
	if ((retval = check_for_zone_in_db(dc, dnsa, FORWARD_ZONE)) != 0) {
		printf("Zone %s already in DNS\n", bdom->domain);
		retval = NONE;
	} else {
		if ((retval = dnsa_run_insert(dc, dnsa, ZONES)) != 0) {
			fprintf(stderr, "Unable to add zone %s to dns\n", zone->name);
			retval = 0;
		} else {
			fprintf(stderr, "Added zone %s\n", zone->name);
		}
	}
	if ((retval = validate_fwd_zone(dc, zone, dnsa)) != 0)
		goto dnsa_cleanup;
	data->args.number = zone->id;
	user->args.number = (unsigned long int)getuid();
	user->next = data;
	if ((retval = dnsa_run_update(dc, user, ZONE_VALID_YES)) != 0)
		printf("Unable to mark zone as valid in database\n");
	else
		printf("Zone marked as valid in the database\n");
	user->next = NULL;
	clean_dbdata_struct(user);
	dnsa_clean_list(dnsa);
	cmdb_free(dc, dclen);
#endif // HAVE_DNSA
	if ((retval = cbc_run_insert(cbs, base, BUILD_DOMAINS)) != 0) {
		fprintf(stderr, "Unable to add build domain %s\n", domain);
	} else {
		printf("Build domain %s added\n", domain);
		if ((retval = write_dhcp_net_config(cbs)) != 0)
			fprintf(stderr, "Cannot write dhcpd.networks!!\n");
	}
	clean_cbc_struct(base);
	clean_dbdata_struct(data);
	return retval;
#ifdef HAVE_DNSA
	dnsa_cleanup:
		dnsa_clean_list(dnsa);
		cmdb_free(dc, dclen);
		clean_dbdata_struct(data);
		clean_dbdata_struct(user);
		clean_cbc_struct(base);
		return retval;
#endif // HAVE_DNSA
}

int
remove_cbc_build_domain(cbc_config_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return CBC_NO_DATA;
	char *domain = cdl->domain;
	int retval = 0;
	dbdata_s *data;

	if (!(data = calloc(sizeof(dbdata_s), 1)))
		report_error(MALLOC_FAIL, "data in remove_cbc_build_domain");
	unsigned long int *bd_id = &(data->args.number);
	if ((retval = get_build_domain_id(cbs, domain, bd_id)) != 0)
		goto cleanup;
	retval = cbc_run_delete(cbs, data, BDOM_DEL_DOM_ID);
	printf ("%d build domain(s) deleted for %s\n", retval, domain);
	if ((retval = write_dhcp_net_config(cbs)) != 0)
		fprintf(stderr, "Cannot write new dhcpd.networks file\n");
	goto cleanup;

	cleanup:
		free(data);
		return retval;
}

int
modify_cbc_build_domain(cbc_config_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return CBC_NO_DATA;
	char *domain = cdl->domain;
	int retval = 0, query = UP_DOM_NTP;
	unsigned long int *bd_id;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, cbc_update_args[query]);
	snprintf(data->args.text, RBUFF_S, "%s", cdl->ntpserver);
	data->next->args.number = (unsigned long int)getuid();
	bd_id = &(data->next->next->args.number);
	if ((retval = get_build_domain_id(cbs, domain, bd_id)) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	retval = cbc_run_update(cbs, data, query);
	printf("%d domains modified\n", retval);
	return 0;
}

int
write_dhcp_net_config(cbc_config_s *cbs)
{
	if (!(cbs))
		return CBC_NO_DATA;
	int retval = 0;
	char filename[CONF_S];
	cbc_s *cbc;
	cbc_dhcp_s *dhcp = 0;
// Why am I bothering to use string_len_s here?
	string_len_s *conf = 0;

	if (!(conf = malloc(sizeof(string_len_s))))
		report_error(MALLOC_FAIL, "conf in write_dhcp_net_config");
	initialise_cbc_s(&cbc);
	init_string_len(conf);
	if (snprintf(filename, CONF_S, "%s/dhcpd.networks", cbs->dhcpconf) >= CONF_S)
		fprintf(stderr, "filename for dhcpd.networks truncated!\n");
	if ((retval = cbc_run_query(cbs, cbc, BUILD_DOMAIN)) != 0) {
		if (retval == NO_RECORDS)
			printf("No build domains configured\n");
		else
			printf("Build domain query failed\n");
		goto cleanup;
	}
	if ((retval = get_net_list_for_dhcp(cbc->bdom, &dhcp)) != 0)
		goto cleanup;
	if ((retval = fill_dhcp_net_config(conf, dhcp)) != 0)
		goto cleanup;
	retval = write_file(filename, conf->string);
	goto cleanup;

	cleanup:
		if (dhcp)
			clean_cbc_dhcp(dhcp);
		clean_cbc_struct(cbc);
		clean_string_len(conf);
		return retval;
}

void
display_bdom_servers(cbc_config_s *cbs, char *domain)
{
	if (!(cbs) || !(domain))
		return;
	dbdata_s *data, *list;
	char *ip;
	int query = BUILD_DOM_SERVERS, retval;
	uint32_t ip_addr;
	unsigned int max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);

	if (!(ip = malloc(RANGE_S)))
		report_error(MALLOC_FAIL, "ip in display_bdom_servers");
	init_multi_dbdata_struct(&data, max);
	list = data;
	if ((retval = get_build_domain_id(cbs, domain, &(data->args.number))) != 0)
		goto cleanup;
	if ((retval = cbc_run_search(cbs, data, query)) == 0) {
		printf("Build domain %s has no servers\n", domain);
	} else {
		printf("Built Servers\tIP\n");
		while (list) {
			memset(ip, 0, RANGE_S);
			ip_addr = htonl((uint32_t)list->next->fields.number);
			inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
			if (strlen(list->fields.text) > 7)
				printf("%s\t%s\n", list->fields.text, ip);
			else
				printf("%s\t\t%s\n", list->fields.text, ip);
			list = list->next->next;
		}
	}
	goto cleanup;

	cleanup:
		free(ip);
		clean_dbdata_struct(data);
		return;
}

int
fill_dhcp_net_config(string_len_s *conf, cbc_dhcp_s *dh)
{
	char *buff, *pos;
	int retval = 0;
	size_t len;
	cbc_dhcp_s *list;
	cbc_dhcp_string_s val;

	if (!(conf) || !(dh))
		return NULL_POINTER_PASSED;
	if (!(buff = malloc(BUFF_S * 2)))
		report_error(MALLOC_FAIL, "buff in fill_dhcp_net_config");
	list = dh;
	while(list) {
		fill_dhcp_val(list, &val);
		snprintf(buff, (BUFF_S * 2), "\
  shared-network %s {\n\
        option domain-name-servers %s;\n\
        option domain-search \"%s\";\n\
        option routers %s;\n\
        subnet %s netmask %s {\n\
                authoratative;\n\
                next-server %s;\n\
                filename \"pxelinux.0\";\n\
        }\n\
}\n\n", list->dname, val.ns, list->dom_search->string, val.gw, val.sn,
        val.nm, val.gw);
		len = strlen(buff);
		if ((len + conf->size) > conf->len)
			resize_string_buff(conf);
		pos = conf->string + conf->size;
		snprintf(pos, len + 1, "%s", buff);
		conf->size += len;
		list = list->next;
	}
	free(buff);
	return retval;
}

void
fill_dhcp_val(cbc_dhcp_s *src, cbc_dhcp_string_s *dst)
{
	uint32_t ip_addr;

	ip_addr = htonl((uint32_t)src->ns);
	inet_ntop(AF_INET, &ip_addr, dst->ns, INET6_ADDRSTRLEN);
	ip_addr = htonl((uint32_t)src->gw);
	inet_ntop(AF_INET, &ip_addr, dst->gw, INET6_ADDRSTRLEN);
	ip_addr = htonl((uint32_t)src->nw);
	inet_ntop(AF_INET, &ip_addr, dst->sn, INET6_ADDRSTRLEN);
	ip_addr = htonl((uint32_t)src->nm);
	inet_ntop(AF_INET, &ip_addr, dst->nm, INET6_ADDRSTRLEN);
	ip_addr = htonl((uint32_t)src->ip);
	inet_ntop(AF_INET, &ip_addr, dst->ip, INET6_ADDRSTRLEN);
}

void
check_bdom_overlap(cbc_config_s *cbs, cbc_build_domain_s *bdom)
{
	int retval;
	cbc_s *cbc;
	cbc_build_domain_s *list = NULL;

	initialise_cbc_s(&cbc);
	if ((retval = cbc_run_query(cbs, cbc, BUILD_DOMAIN)) != 0) {
		if (retval != NO_RECORDS)
			fprintf(stderr, "Build domain query failed\n");
		goto cleanup;
	}
	list = cbc->bdom;
	while (list) {
		if ((retval = build_dom_overlap(list, bdom)) != 0) {
			clean_cbc_struct(cbc);
			report_error(retval, bdom->domain);
		} else
			list = list->next;
	}
	goto cleanup;

	cleanup:
		clean_cbc_struct(cbc);
		return;
}

int
build_dom_overlap(cbc_build_domain_s *list, cbc_build_domain_s *new)
{
	int retval = 0;

	if (((new->start_ip >= list->start_ip) && (new->start_ip <= list->end_ip)) ||
	   ((new->end_ip >= list->start_ip) && (new->end_ip <= list->end_ip )))
		retval = BDOM_OVERLAP;
	return retval;
}

void
fill_bdom_values(cbc_build_domain_s *bdom, cbcdomain_comm_line_s *cdl)
{
	if (cdl->confntp > 0) {
		bdom->config_ntp = 1;
		snprintf(bdom->ntp_server, RBUFF_S, "%s", cdl->ntpserver);
	}
	bdom->start_ip = cdl->start_ip;
	bdom->end_ip = cdl->end_ip;
	bdom->netmask = cdl->netmask;
	bdom->gateway = cdl->gateway;
	bdom->ns = cdl->ns;
	snprintf(bdom->domain, RBUFF_S, "%s", cdl->domain);
	bdom->cuser = bdom->muser = (unsigned long int)getuid();
}

