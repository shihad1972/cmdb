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
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "cbc_common.h"
#include "cbcnet.h"
#include "cbc_base_sql.h"
#ifdef HAVE_DNSA
# include "cmdb_dnsa.h"
# include "dnsa_base_sql.h"
# include "cbc_dnsa.h"
#endif // HAVE_DNSA 

static int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl);

static void
validate_cbcdomain_comm_line(cbcdomain_comm_line_s *cdl);

static int
display_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

static void
display_bdom_servers(ailsa_cmdb_s *cbs, char *domain);

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
check_for_bdom_overlap(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl);

static int
split_network_args(cbcdomain_comm_line_s *cdl, char *netinfo);

static int
add_ips_to_list(unsigned long i, unsigned long int j, AILLIST *l);

static int
cbc_populate_zone(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl, AILLIST *zone);

static int
cbc_populate_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl, AILLIST *domain);

static int
write_dhcp_net_config(ailsa_cmdb_s *cbs);

static int
fill_dhcp_net_config(string_len_s *conf, cbc_dhcp_s *dh);

static void
fill_dhcp_val(cbc_dhcp_s *src, cbc_dhcp_string_s *dst);

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
	if (cdcl->action == LIST_CONFIG)
		retval = list_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == DISPLAY_CONFIG)
		retval = display_cbc_build_domain(cmc, cdcl);
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

	cbcdomain_clean_comm_line(cdcl);
	ailsa_clean_cmdb(cmc);
	if (retval > 0)
		report_error(retval, argv[0]);
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

static void
validate_cbcdomain_comm_line(cbcdomain_comm_line_s *cdl)
{
	if (cdl->ntpserver)
		if (ailsa_validate_input(cdl->ntpserver, DOMAIN_REGEX) < 0)
			if (ailsa_validate_input(cdl->ntpserver, IP_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "ntp server");
	if (cdl->domain)
		if (ailsa_validate_input(cdl->domain, DOMAIN_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "domain");
}

static int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl)
{
	const char *optstr = "abd:k:lmn:rt:vw";
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
		{"build-domain",	required_argument,	NULL,	'n'},
		{"domain",		required_argument,	NULL,	'n'},
		{"name",		required_argument,	NULL,	'n'},
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
		} else if (opt == 'd') {
			cdl->action = DISPLAY_CONFIG;
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
			cdl->domain = strndup(optarg, RBUFF_S);
		} else if (opt == 't') {
			cdl->ntpserver = strndup(optarg, RBUFF_S);
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
	validate_cbcdomain_comm_line(cdl);
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cdl->action == CVERSION)
		return CVERSION;
	if (cdl->action == NONE)
		return NO_ACTION;
	if (cdl->action != LIST_CONFIG && cdl->action != WRITE_CONFIG &&
	     !(cdl->domain))
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
			return USER_INPUT_INVALID;
		}
		if (ailsa_validate_input(ip, IP_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "network");
		if (inet_pton(AF_INET, ip, &ip_addr))
			ips[i] = (unsigned long int) htonl(ip_addr);
		else
			report_error(USER_INPUT_INVALID, "network");
		ip = tmp;
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

static int
list_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *elem;

	if ((retval = ailsa_basic_query(cbs, ALL_BUILD_DOMAINS, list)) != 0) {
		ailsa_syslog(LOG_ERR, "ALL_BUILD_DOMAINS query failed");
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

	cleanup:
		return retval;
}

static int
add_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	AILLIST *d = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
#ifdef HAVE_DNSA
	AILLIST *z = ailsa_db_data_list_init();
#endif // HAVE_DNSA
	char *domain = cdl->domain;
	int retval = 0;

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
	if ((retval = check_for_bdom_overlap(cbs, cdl)) != 0) {
		ailsa_syslog(LOG_ERR, "IP's in this range overlap with existing build domain");
		goto cleanup;
	}
	if ((retval = cbc_populate_domain(cbs, cdl, d)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot populate build domain values");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbs, INSERT_BUILD_DOMAIN, d)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_BUILD_DOMAIN query failed");
		goto cleanup;
	}
#ifdef HAVE_DNSA
	if ((retval = cmdb_check_for_fwd_zone(cbs, domain)) > 0) {
		ailsa_syslog(LOG_INFO, "Zone %s already in dnsa database", domain);
	} else if (retval == -1) {
		goto cleanup;
	} else {
		if ((retval = cbc_populate_zone(cbs, cdl, z)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot populate zone list");
			goto cleanup;
		}
		if ((retval = ailsa_insert_query(cbs, INSERT_BUILD_DOMAIN_ZONE, z)) != 0) {
			ailsa_syslog(LOG_ERR, "INSERT_BUILD_DOMAIN_ZONE query failed");
			goto cleanup;
		}
		if ((retval = cmdb_validate_zone(cbs, FORWARD_ZONE, domain)) != 0)
			ailsa_syslog(LOG_ERR, "Cannot validate zone %s", domain);
	}
#endif // HAVE_DNSA
	cleanup:
#ifdef HAVE_DNSA
		ailsa_list_full_clean(z);
#endif // HAVE_DNSA
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(d);
		return retval;
}

static int
remove_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
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

static int
modify_cbc_build_domain(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
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

static int
write_dhcp_net_config(ailsa_cmdb_s *cbs)
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

static void
display_bdom_servers(ailsa_cmdb_s *cbs, char *domain)
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

static int
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
        val.nm, val.ns);
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

static void
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

static int
check_for_bdom_overlap(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl)
{
	if (!(cbs) || !(cdl))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();

	if ((retval = add_ips_to_list(cdl->start_ip, cdl->end_ip, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add IP's to list");
		goto cleanup;
	}
	if ((retval = add_ips_to_list(cdl->start_ip, cdl->start_ip, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add start IP's to list");
		goto cleanup;
	}
	if ((retval = add_ips_to_list(cdl->end_ip, cdl->end_ip, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add end IP's to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbs, BUILD_DOMAIN_OVERLAP, l, r)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_OVERLAP query failed");
		goto cleanup;
	}
	if (r->total > 0)
		retval = BDOM_OVERLAP;
	cleanup:
		ailsa_list_destroy(l);
		ailsa_list_destroy(r);
		my_free(l);
		my_free(r);
		return retval;
}

static int
add_ips_to_list(unsigned long i, unsigned long int j, AILLIST *l)
{
	int retval;

	if ((retval = cmdb_add_number_to_list(i, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert first IP address into list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(j, l)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert second IP address into list");
	return retval;
}

static int
cbc_populate_zone(ailsa_cmdb_s *cbs, cbcdomain_comm_line_s *cdl, AILLIST *zone)
{
	if (!(cbs) || !(cdl) || !(zone))
		return AILSA_NO_DATA;
	int retval;

	if ((retval = cmdb_add_string_to_list(cdl->domain, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone name to list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(cbs->prins, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add primary DNS server to list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(cbs->secns, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add secondary DNS server to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cbs->refresh, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add refresh value to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cbs->retry, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add retry value to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cbs->expire, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add expire value to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(cbs->ttl, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add ttl value to list");
		return retval;
	}
	if ((retval = cmdb_add_number_to_list(generate_zone_serial(), zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone serial to list");
		return retval;
	}
	if ((retval = cmdb_populate_cuser_muser(zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		return retval;
	}
	return retval;
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
	if ((retval = cmdb_add_short_to_list(cdl->confntp, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add config ntp to list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(cdl->ntpserver, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add ntp server name to list");
		return retval;
	}
	if ((retval = cmdb_populate_cuser_muser(dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		return retval;
	}
	return retval;
}
