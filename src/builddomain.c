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
# include "cbc_dnsa.h"

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
		if (retval == 6)
			fprintf(stderr, "No build domains\n");
		else
			fprintf(stderr, "build query failed\n");
		clean_cbc_struct(base);
		return retval;
	}
	if ((retval = get_build_domain(cdl, base)) != 0) {
		fprintf(stderr, "build domain %s not found\n", cdl->domain);
		clean_cbc_struct(base);
		return retval;
	}
	if (cdl->action != LIST_SERVERS)
		display_build_domain(base->bdom);
	list_build_dom_servers(cbc, base->bdom->bd_id, cdl->domain);
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
	char configfile[CONF_S] = "/etc/dnsa/dnsa.conf";
	if ((retval = parse_dnsa_config_file(dc, configfile)) != 0) {
		fprintf(stderr, "Error in config file %s\n", configfile);
		free(dc);
		free(dnsa);
		free(zone);
		free(data);
		clean_cbc_struct(base);
		return retval;
	}
	fill_cbc_fwd_zone(zone, bdom->domain, dc);
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
	if ((retval = validate_fwd_zone(dc, zone, dnsa)) != 0) {
		dnsa_clean_list(dnsa);
		free(dc);
		free(data);
		clean_cbc_struct(base);
		return retval;
	}
	data->args.number = zone->id;
	if ((retval = dnsa_run_update(dc, data, ZONE_VALID_YES)) != 0)
		printf("Unable to mark zone as valid in database\n");
	else
		printf("Zone marked as valid in the database\n");

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
modify_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl)
{
	int retval = NONE, type = NONE, i = NONE;
	dbdata_s *data = '\0';

	if ((strncmp(cdl->basedn, "NULL", COMM_S) != 0) ||
	    (strncmp(cdl->binddn, "NULL", COMM_S) != 0) ||
	    (strncmp(cdl->ldapserver, "NULL", COMM_S) != 0) ||
	    (cdl->ldapssl != 0)) {
		printf("Modifying ldap authentication config for domain %s\n",
		       cdl->domain);
		printf("Ensure you have a full LDAP config \
(server, basedn and binddn)\n\n");
		if ((retval = get_mod_ldap_bld_dom(cdl, &type)) != 0) {
			free (data);
			return retval;
		}
		cbc_init_update_dbdata(&data, (unsigned) type);
		if ((retval = cbc_get_build_dom_id(cbc, cdl, data)) == 0) {
			fprintf(stderr, "No build domain for %s\n", data->args.text);
			clean_dbdata_struct(data);
			return NO_DOMAIN;
		} else if (retval > 1) {
			clean_dbdata_struct(data);
			return MULTI_DOMAIN;
		}
		cbc_fill_ldap_update_data(cdl, data, type);
		if ((retval = cbc_run_update(cbc, data, type)) == 0)
			printf("Build domain %s not modified\n", cdl->domain);
		else if (retval == 1)
			printf("Build domain %s modified\n", cdl->domain);
		else
			printf("Multiple build domains for %s modified\n", cdl->domain);
		clean_dbdata_struct(data);
		data = '\0';
		i++;
	}
	if ((strncmp(cdl->logserver, "NULL", COMM_S) != 0) ||
	    (strncmp(cdl->nfsdomain, "NULL", COMM_S) != 0) ||
	    (strncmp(cdl->ntpserver, "NULL", COMM_S) != 0) ||
	    (strncmp(cdl->smtpserver, "NULL", COMM_S) != 0) ||
	    (strncmp(cdl->xymonserver, "NULL", COMM_S) != 0)) {
		printf("Modifying application configuration for domain %s\n",
		       cdl->domain);
		if ((retval = get_mod_app_bld_dom(cdl, &type)) != 0) {
			free(data);
			return retval;
		}
		cbc_init_update_dbdata(&data, (unsigned) type);
		if ((retval = cbc_get_build_dom_id(cbc, cdl, data)) == 0) {
			fprintf(stderr, "No build domain for %s\n", data->args.text);
			clean_dbdata_struct(data);
			return NO_DOMAIN;
		} else if (retval > 1) {
			clean_dbdata_struct(data);
			return MULTI_DOMAIN;
		}
		cbc_fill_app_update_data(cdl, data, type);
		if ((retval = cbc_run_update(cbc, data, type)) == 0)
			printf("Build domain %s not modified\n", cdl->domain);
		else if (retval == 1)
			printf("Build domain %s modified\n", cdl->domain);
		else
			printf("Multiple build domains for %s modified\n", cdl->domain);
		clean_dbdata_struct(data);
		i++;
	}
	if (i > 0)
		return NONE;
	else
		return DID_NOT_MOD_BUILD_DOMAIN;
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
		if (retval == NO_RECORDS) {
			fprintf(stderr, "No build domains in DB\n");
			retval = NO_BUILD_DOMAIN;
		} else
			fprintf(stderr, "build query failed\n");
		free(base);
		return retval;
	}
	bdom = base->bdom;
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

int
get_mod_ldap_bld_dom(cbcdomain_comm_line_s *cdl, int *query)
{
	int retval = NONE, type = NONE;

	if (strncmp(cdl->basedn, "NULL", NAME_S) != 0)
		type = type | BASEDN;
	if (strncmp(cdl->binddn, "NULL", NAME_S) != 0)
		type = type | BINDDN;
	if (strncmp(cdl->ldapserver, "NULL", HOST_S) != 0)
		type = type | LDAPSERV;
	if (cdl->ldapssl != 0)
		type = type | LDAPSSL;

	if (!(type & LDAPSSL) && !(type & LDAPSERV) && !(type & BINDDN) && 
	     (type & BASEDN))
		*query = UP_DOM_BASEDN;
	else if (!(type & LDAPSSL) && !(type & LDAPSERV) && (type & BINDDN) &&
	    !(type & BASEDN))
		*query = UP_DOM_BINDDN;
	else if (!(type & LDAPSSL) && (type & LDAPSERV) && !(type & BINDDN) &&
	    !(type & BASEDN))
		*query = UP_DOM_LDAPSERV;
	else if ((type & LDAPSSL) && !(type & LDAPSERV) && !(type & BINDDN) &&
	    !(type & BASEDN))
		*query = UP_DOM_LDAPSSL;
	else if (!(type & LDAPSSL) && !(type & LDAPSERV) && (type & BINDDN) &&
	     (type & BASEDN))
		*query = UP_DOM_BASEBINDDN;
	else if (!(type & LDAPSSL) && (type & LDAPSERV) && !(type & BINDDN) &&
	     (type & BASEDN))
		*query = UP_DOM_BASEDNSERV;
	else if ((type & LDAPSSL) && !(type & LDAPSERV) && !(type & BINDDN) &&
	     (type & BASEDN))
		*query = UP_DOM_BASEDNSSL;
	else if (!(type & LDAPSSL) && (type & LDAPSERV) && (type & BINDDN) &&
	    !(type & BASEDN))
		*query = UP_DOM_BINDDNSERV;
	else if ((type & LDAPSSL) && !(type & LDAPSERV) && (type & BINDDN) &&
	    !(type & BASEDN))
		*query = UP_DOM_BINDDNSSL;
	else if ((type & LDAPSSL) && (type & LDAPSERV) && !(type & BINDDN) &&
	    !(type & BASEDN))
		*query = UP_DOM_LDAPSERVSSL;
	else if (!(type & LDAPSSL) && (type & LDAPSERV) && (type & BINDDN) && 
	     (type & BASEDN))
		*query = UP_DOM_BASEBINDDNSERV;
	else if ((type & LDAPSSL) && (type & LDAPSERV) && !(type & BINDDN) &&
	     (type & BASEDN))
		*query = UP_DOM_BASEDNSERVSSL;
	else if ((type & LDAPSSL) && !(type & LDAPSERV) && (type & BINDDN) &&
	     (type & BASEDN))
		*query = UP_DOM_BASEBINDDNSSL;
	else if ((type & LDAPSSL) && (type & LDAPSERV) && (type & BINDDN) &&
	    !(type & BASEDN))
		*query = UP_DOM_BINDDNSERVSSL;
	else if ((type & LDAPSSL) && (type & LDAPSERV) && (type & BINDDN) &&
	     (type & BASEDN))
		*query = UP_DOM_LDAPALL;
	return retval;
}

int
get_mod_app_bld_dom(cbcdomain_comm_line_s *cdl, int *query)
{
	int retval = NONE, type = NONE;

	if (strncmp(cdl->nfsdomain, "NULL", COMM_S) != 0)
		type = type | NFSDOM;
	if (strncmp(cdl->ntpserver, "NULL", COMM_S) != 0)
		type = type | NTPSERV;
	if (strncmp(cdl->smtpserver, "NULL", COMM_S) != 0)
		type = type | SMTPSERV;
	if (strncmp(cdl->logserver, "NULL", COMM_S) != 0)
		type = type | LOGSERV;
	if (strncmp(cdl->xymonserver, "NULL", COMM_S) != 0)
		type = type | XYMONSERV;
	if ((type & NFSDOM) && !(type & NTPSERV) && !(type & SMTPSERV) &&
	   !(type & XYMONSERV) && !(type & LOGSERV))
		*query = UP_DOM_NFS;
	else if (!(type & NFSDOM) && (type & NTPSERV) && !(type & SMTPSERV) &&
	   !(type & XYMONSERV) && !(type & LOGSERV))
		*query = UP_DOM_NTP;
	else if (!(type & NFSDOM) && !(type & NTPSERV) && (type & SMTPSERV) &&
	   !(type & XYMONSERV) && !(type & LOGSERV))
		*query = UP_DOM_SMTP;
	else if (!(type & NFSDOM) && !(type & NTPSERV) && !(type & SMTPSERV) &&
	   !(type & XYMONSERV) && (type & LOGSERV))
		*query = UP_DOM_LOG;
	else if (!(type & NFSDOM) && !(type & NTPSERV) && !(type & SMTPSERV) &&
	   (type & XYMONSERV) && !(type & LOGSERV))
		*query = UP_DOM_XYMON;
	return retval;
}

int
cbc_get_build_dom_id(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl, dbdata_s *data)
{
	int retval;
	dbdata_s *dom = '\0';

	if (!(dom = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "dom in cbc_get_build_dom_id");
	init_dbdata_struct(dom);
	snprintf(dom->args.text, RBUFF_S, "%s", cdl->domain);
	if ((retval = cbc_run_search(cbc, dom, BD_ID_ON_DOMAIN)) == 0) {
		fprintf(stderr, "Domain %s not found\n", cdl->domain);
		free(dom);
		return retval;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple build domains %s\n", cdl->domain);
		free(dom);
		return retval;
	}
	data->fields.number = dom->fields.number;
	free(dom);
	return retval;
}

void
cbc_fill_ldap_update_data(cbcdomain_comm_line_s *cdl, dbdata_s *data, int query)
{
	unsigned long int bd_id = data->fields.number;
	if (query == UP_DOM_BASEDN)  {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_BINDDN) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_LDAPSERV) {
		if (data)
			snprintf(data->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_LDAPSSL) {
		if (data)
			data->args.small = 1;
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_BASEBINDDN) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			snprintf(data->next->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next->next)
			data->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BASEDNSERV) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			snprintf(data->next->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next->next)
			data->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BASEDNSSL) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			data->next->args.small = 1;
		if (data->next->next)
			data->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BINDDNSERV) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next)
			snprintf(data->next->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next->next)
			data->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BINDDNSSL) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next)
			data->next->args.small = 1;
		if (data->next->next)
			data->next->next->args.number = bd_id;
	} else if (query == UP_DOM_LDAPSERVSSL) {
		if (data)
			snprintf(data->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next)
			data->next->args.small = 1;
		if (data->next->next)
			data->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BASEBINDDNSERV) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			snprintf(data->next->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next->next)
			snprintf(data->next->next->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next->next->next)
			data->next->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BASEDNSERVSSL) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			snprintf(data->next->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next->next)
			data->next->next->args.small = 1;
		if (data->next->next->next)
			data->next->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BASEBINDDNSSL) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			snprintf(data->next->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next->next)
			data->next->next->args.small = 1;
		if (data->next->next->next)
			data->next->next->next->args.number = bd_id;
	} else if (query == UP_DOM_BINDDNSERVSSL) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next)
			snprintf(data->next->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next->next)
			data->next->next->args.small = 1;
		if (data->next->next->next)
			data->next->next->next->args.number = bd_id;
	} else if (query == UP_DOM_LDAPALL) {
		if (data)
			snprintf(data->args.text, NAME_S, "%s", cdl->basedn);
		if (data->next)
			snprintf(data->next->args.text, NAME_S, "%s", cdl->binddn);
		if (data->next->next)
			snprintf(data->next->next->args.text, HOST_S, "%s", cdl->ldapserver);
		if (data->next->next->next)
			data->next->next->next->args.small = 1;
		if (data->next->next->next->next)
			data->next->next->next->next->args.number = bd_id;
	}
}

void
cbc_fill_app_update_data(cbcdomain_comm_line_s *cdl, dbdata_s *data, int query)
{
	unsigned long int bd_id = data->fields.number;
	if (query == UP_DOM_NFS) {
		if (data)
			snprintf(data->args.text, CONF_S, "%s", cdl->nfsdomain);
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_NTP) {
		if (data)
			snprintf(data->args.text, CONF_S, "%s", cdl->ntpserver);
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_SMTP) {
		if (data)
			snprintf(data->args.text, CONF_S, "%s", cdl->smtpserver);
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_LOG) {
		if (data)
			snprintf(data->args.text, CONF_S, "%s", cdl->logserver);
		if (data->next)
			data->next->args.number = bd_id;
	} else if (query == UP_DOM_XYMON) {
		if (data)
			snprintf(data->args.text, CONF_S, "%s", cdl->xymonserver);
		if (data->next)
			data->next->args.number = bd_id;
	}
}

void
list_build_dom_servers(cbc_config_s *cbc, unsigned long int id, char *name)
{
	int retval;
	dbdata_s *data = '\0';

	cbc_init_initial_dbdata(&data, BUILD_DOM_SERVERS);
	data->args.number = id;
	if ((retval = cbc_run_search(cbc, data, BUILD_DOM_SERVERS)) == 0)
		printf("No severs built in domain %s\n", name);
	else
		print_build_dom_servers(data, name);
	clean_dbdata_struct(data);
}

void
print_build_dom_servers(dbdata_s *data, char *name)
{
	char *ip;
	uint32_t ip_addr;
	dbdata_s *list = data;
	size_t len;

	if (!(ip = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in print_build_dom_servers");
	printf("Servers built in domain %s\n", name);
	while (list) {
		ip_addr = htonl((uint32_t)list->next->fields.number);
		inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
		len = strlen(list->fields.text);
		if (len >= 8)
			printf("%s\t%s\n", list->fields.text, ip);
		else
			printf("%s\t\t%s\n", list->fields.text, ip);
		list = list->next->next;
	}
	free(ip);
}

#ifdef HAVE_DNSA

#endif