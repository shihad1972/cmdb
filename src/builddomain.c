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
 *  builddomain.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

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
#include "../config.h"
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "checks.h"
#include "cbcdomain.h"
#include "builddomain.h"

#ifdef HAVE_DNSA

# include "cmdb_dnsa.h"
# include "dnsa_base_sql.h"

#endif /* HAVE_DNSA */

int
display_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl)
{
	int retval = NONE;
	cbc_s *base;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in display_cbc_build_domain");
	init_cbc_struct(base);
	if ((retval = cbc_run_query(cbc, base, BUILD_DOMAIN)) != 0) {
		fprintf(stderr, "build query failed\n");
		clean_cbc_struct(base);
		return retval;
	}
	if ((retval = get_build_domain(cdl, base)) != 0) {
		fprintf(stderr, "build domain %s not found\n", cdl->domain);
		clean_cbc_struct(base);
		return retval;
	}
	display_build_domain(base->bdom);
	clean_cbc_struct(base);
	return retval;
}

int
add_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl)
{
	int retval = NONE;
	cbc_s *base;
	cbc_build_domain_s *bdom;
	dbdata_s *data;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add_cbc_build_domain");
	if (!(bdom = malloc(sizeof(cbc_build_domain_s))))
		report_error(MALLOC_FAIL, "bdom in add_cbc_build_domain");
	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "bdom in add_cbc_build_domain");
	init_cbc_struct(base);
	init_build_domain(bdom);
	base->bdom = bdom;
	copy_build_domain_values(cdl, bdom);
	snprintf(data->args.text, RBUFF_S, "%s", bdom->domain);
	retval = cbc_run_search(cbc, data, BUILD_DOMAIN_COUNT);
	if (data->fields.number > 0) {
		printf("Domain %s already in database\n", bdom->domain);
		free(data);
		clean_cbc_struct(base);
		return BUILD_DOMAIN_EXISTS;
	}
	display_build_domain(bdom);
#ifdef HAVE_DNSA

	dnsa_config_s *dc;
	dnsa_s *dnsa;
	zone_info_s *zone;
	if (!(dc = malloc(sizeof(dnsa_config_s))))
		report_error(MALLOC_FAIL,"dc in add_cbc_build_domain");
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in add_cbc_build_domain");
	if (!(zone = malloc(sizeof(zone_info_s))))
		report_error(MALLOC_FAIL, "zone in add_fwd_zone");
	
	init_dnsa_struct(dnsa);
/*
 * This really needs to get put into the config struct 
 */
	const char configfile = "/etc/dnsa/dnsa.conf";
	if ((retval = parse_dnsa_config_file(dc, configfile)) != 0) {
		fprintf(stderr, "Error in config file %s\n", configfile);
		free(dc);
		free(dnsa);
		free(zone);
		free(data);
		clean_cbc_struct(base);
		return retval;
	}
	fill_fwd_zone_info(zone, bdom->domain, dc);
	dnsa->zones = zone;
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

#endif
	if ((retval = cbc_run_insert(cbc, base, BUILD_DOMAINS)) != 0)
		printf("\
\nUnable to add build domain %s to database\n", bdom->domain);
	else
		printf("\
\nAdded build domain %s to database\n", bdom->domain);

	clean_cbc_struct(base);
	free(data);
	return retval;
}

int
remove_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl)
{
	int retval = NONE;
	dbdata_s *data;

	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in remove_cbc_build_domain");
	if (strncmp(cdl->domain, "NULL", COMM_S) != 0)
		snprintf(data->args.text, RBUFF_S, "%s", cdl->domain);
	else {
		free(data);
		return NO_DOMAIN_NAME;
	}
	if ((retval = cbc_run_delete(cbc, data, BDOM_DEL_DOMAIN)) != 1) {
		printf("%d domain(s) deleted\n", retval);
		free(data);
		return retval;
	} else {
		printf("Build domain %s deleted\n", data->args.text);
		retval = NONE;
	}

	free(data);
	return retval;
}

int
list_cbc_build_domain(cbc_config_s *cbc)
{
	int retval = NONE;
	cbc_s *base;
	cbc_build_domain_s *bdom;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in list_cbc_build_domain");
	init_cbc_struct(base);
	if ((retval = cbc_run_query(cbc, base, BUILD_DOMAIN)) != 0) {
		fprintf(stderr, "build query failed\n");
		free(base);
		return retval;
	}
	bdom = base->bdom;
	printf("Build Domains\n\n");
	while (bdom) {
		printf("%s\n", bdom->domain);
		bdom = bdom->next;
	}
	clean_cbc_struct(base);
	return retval;
}

int
get_build_domain(cbcdomain_comm_line_s *cdl, cbc_s *base)
{
	int retval = NONE;
	char *domain = cdl->domain;
	cbc_build_domain_s *bdom = base->bdom, *next;
	base->bdom = '\0';

	if (bdom)
		next = bdom->next;
	else
		return BUILD_DOMAIN_NOT_FOUND;
	while (bdom) {
		bdom->next = '\0';
		if (strncmp(bdom->domain, domain, RBUFF_S) == 0)
			base->bdom = bdom;
		else
			free(bdom);
		bdom = next;
		if (next)
			next = next->next;
	}
	if (!base->bdom)
		retval = BUILD_DOMAIN_NOT_FOUND;
	return retval;
}

void
copy_build_domain_values(cbcdomain_comm_line_s *cdl, cbc_build_domain_s *bdom)
{
	if (cdl->confntp > 0) {
		bdom->config_ntp = 1;
		snprintf(bdom->ntp_server, HOST_S, "%s", cdl->ntpserver);
	}
	if (cdl->conflog > 0) {
		bdom->config_log = 1;
		snprintf(bdom->log_server, CONF_S, "%s", cdl->logserver);
	}
	if (cdl->confsmtp > 0) {
		bdom->config_email = 1;
		snprintf(bdom->smtp_server, CONF_S, "%s", cdl->smtpserver);
	}
	if (cdl->confxymon > 0) {
		bdom->config_xymon = 1;
		snprintf(bdom->xymon_server, CONF_S, "%s", cdl->xymonserver);
	}
	if (cdl->confldap > 0) {
		bdom->config_ldap = 1;
		snprintf(bdom->ldap_server, URL_S, "%s", cdl->ldapserver);
		snprintf(bdom->ldap_dn, URL_S, "%s", cdl->basedn);
		snprintf(bdom->ldap_bind, URL_S, "%s", cdl->binddn);
		bdom->ldap_ssl = cdl->ldapssl;
	}
	bdom->start_ip = cdl->start_ip;
	bdom->end_ip = cdl->end_ip;
	bdom->netmask = cdl->netmask;
	bdom->gateway = cdl->gateway;
	bdom->ns = cdl->ns;
	snprintf(bdom->domain, RBUFF_S, "%s", cdl->domain);
	snprintf(bdom->nfs_domain, CONF_S, "%s", cdl->nfsdomain);
}

#ifdef HAVE_DNSA

#endif