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
 *  cbc_dnsa.c
 * 
 *  Functions to likn into dnsa functions for cbc
 * 
 *  part of the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "../config.h"
#include "base_sql.h"
#include "cbc_data.h"
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_base_sql.h"

#ifdef HAVE_LIBPCRE

# include "checks.h"

#endif /* HAVE_LIBPCRE */

#ifdef HAVE_DNSA

# include "cmdb_dnsa.h"
# include "cbc_dnsa.h"
# include "dnsa_base_sql.h"


void
fill_cbc_fwd_zone(zone_info_s *zone, char *domain, dnsa_config_s *dc)
{
	snprintf(zone->name, RBUFF_S, "%s", domain);
	snprintf(zone->pri_dns, RBUFF_S, "%s", dc->prins);
	snprintf(zone->sec_dns, RBUFF_S, "%s", dc->secns);
	snprintf(zone->type, COMM_S, "master");
	zone->serial = get_zone_serial();
	zone->refresh = dc->refresh;
	zone->retry = dc->retry;
	zone->expire = dc->expire;
	zone->ttl = dc->ttl;
	zone->cuser = zone->muser = (unsigned long int)getuid();
}

void
copy_cbc_into_dnsa(dnsa_config_s *dc, cbc_config_s *cbc)
{
	snprintf(dc->dbtype, RANGE_S, "%s", cbc->dbtype);
	snprintf(dc->file, CONF_S, "%s", cbc->file);
	snprintf(dc->db, CONF_S, "%s", cbc->db);
	snprintf(dc->user, CONF_S, "%s", cbc->user);
	snprintf(dc->host, CONF_S, "%s", cbc->host);
	snprintf(dc->pass, CONF_S, "%s", cbc->pass);
	snprintf(dc->socket, CONF_S, "%s", cbc->socket);
	dc->port = cbc->port;
	dc->cliflag = cbc->cliflag;
}

int
get_dns_ip_list(cbc_config_s *cbt, uli_t *ip, dbdata_s *data)
{
	int retval = NONE;
	size_t dclen = sizeof(dnsa_config_s);
	dnsa_s *dnsa;
	dnsa_config_s *dc = NULL;

	dc = cmdb_malloc(dclen, "dc in get_dns_ip_list");
	dnsa = cmdb_malloc(sizeof(dnsa_s), "dnsa in get_dns_ip_list");
	copy_cbc_into_dnsa(dc, cbt);
	if ((retval = dnsa_run_query(dc, dnsa, ALL_A_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		free(dc);
		return retval;
	}
	prep_dnsa_ip_list(data, dnsa, ip);
	dnsa_clean_list(dnsa);
	cmdb_free(dc, dclen);
	return retval;
}

void
prep_dnsa_ip_list(dbdata_s *data, dnsa_s *dnsa, uli_t *ip)
{
	int i;
	dbdata_s *pos = data, *list;
	record_row_s *rec = dnsa->records;
	while (pos) {
		if ((pos->fields.number > 0) && (pos->next))
			pos = pos->next;
		else
			break;
	}
	while (rec) {
		add_int_ip_to_fwd_records(rec);
		if ((rec->ip_addr >= *(ip + 1)) && 
		    (rec->ip_addr <= *(ip + 2))) {
			list = data;
			i = FALSE;
			while (list) {
				if (list->fields.number == rec->ip_addr)
					i = TRUE;
				list = list->next;
			}
			if (i == FALSE) {
				if (!(list = calloc(sizeof(dbdata_s), sizeof(char))))
					report_error(MALLOC_FAIL, "list in prep_dnsa_ip_list");
				init_dbdata_struct(list);
				list->fields.number = rec->ip_addr;
				pos->next = list;
				pos = list;
			}
		}
		rec = rec->next;
	}
}

int
check_for_build_ip_in_dns(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_s *cbc)
{
	int retval = NONE;
	unsigned int a = dnsa_extended_search_args[RECORDS_ON_ZONE];
	unsigned int f = dnsa_extended_search_fields[RECORDS_ON_ZONE];
	unsigned int max = cmdb_get_max(a, f);
	size_t dclen = sizeof(dnsa_config_s);
	dbdata_s *data;
	dnsa_config_s *dc;
	dnsa_s *dnsa;
	zone_info_s *zone;
	record_row_s *rec;

	dc = cmdb_malloc(dclen, "dc in check_for_build_ip_in_dns");
	dnsa = cmdb_malloc(sizeof(dnsa_s), "dnsa in check_for_build_ip_in_dns");
	zone = cmdb_malloc(sizeof(zone_info_s), "zone in check_for_build_ip_in_dns");
	if (!(rec = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "rec in check_for_build_ip_in_dns");
	setup_dnsa_build_ip_structs(zone, dnsa, dc, cbt, rec);
	init_multi_dbdata_struct(&data, max);
	snprintf(zone->name, RBUFF_S, "%s", cml->build_domain);
	if ((retval = dnsa_run_search(dc, dnsa, ZONE_ID_ON_NAME)) != 0)
		goto cleanup;
	data->args.number = zone->id;
	fill_rec_with_build_info(rec, zone, cml, cbc);
	retval = dnsa_run_extended_search(dc, data, RECORDS_ON_ZONE);
	if (retval == 0) /* No hosts in zone so just add */ {
		retval = add_build_host_to_dns(dc, dnsa);
	} else {
		retval = do_build_ip_dns_check(cbc->bip, data);
		if (retval == 0)
			retval = add_build_host_to_dns(dc, dnsa);
	}
	cleanup:
		clean_dbdata_struct(data);
		dnsa_clean_list(dnsa);
		cmdb_free(dc, dclen);
		return retval;
}

void
setup_dnsa_build_ip_structs(zone_info_s *zone, dnsa_s *dnsa, dnsa_config_s *dc, cbc_config_s *cbt, record_row_s *rec)
{
	init_zone_struct(zone);
	init_record_struct(rec);
	dnsa->zones = zone;
	dnsa->records = rec;
	dnsa_init_config_values(dc);
	copy_cbc_into_dnsa(dc, cbt);
}

int
do_build_ip_dns_check(cbc_build_ip_s *bip, dbdata_s *data)
{
	char ipaddr[RANGE_S], *ip;
	int retval = NONE;
	uint32_t ip_addr;
	dbdata_s *list = data;

	ip = ipaddr;
	if (!(list))
		return NO_BUILD_IP;
	while (list) {
		if (list->next->next->next->next)
			list = list->next->next->next->next;
		else
			break;
/* This only checks if the IP is in DNS. Will have to add full check for name as well */
		ip_addr = htonl((uint32_t)bip->ip);
		inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
		if (strncmp(list->fields.text, ip, RANGE_S) ==0) {
			retval = 2;
			break;
		} else {
			list = list->next;
		}
	}
	return retval;
}

void
fill_rec_with_build_info(record_row_s *rec, zone_info_s *zone, cbc_comm_line_s *cml, cbc_s *cbc)
{
	char *dest = rec->dest;
	uint32_t ip_addr;

	rec->pri = 0;
	snprintf(rec->type, COMM_S, "A");
	rec->zone = zone->id;
	snprintf(rec->host, HOST_S, "%s", cml->name);
	ip_addr = htonl((uint32_t)cbc->bip->ip);
	inet_ntop(AF_INET, &ip_addr, dest, RANGE_S);
	rec->cuser = rec->muser = (unsigned long int)getuid();
}

int
add_build_host_to_dns(dnsa_config_s *dc, dnsa_s *dnsa)
{
	int retval = NONE;
	char *host = dnsa->records->host, *zone = dnsa->zones->name;
	dbdata_s data, next;

	memset(&data, 0, sizeof(data));
	memset(&next, 0, sizeof(data));
	data.args.number = dnsa->zones->id;
	if ((retval = dnsa_run_insert(dc, dnsa, RECORDS)) != 0) {
		fprintf(stderr, "Cannot insert host %s into zone %s\n",
		  host, zone);
	} else {
		printf("Host %s added to zone %s\n", host, zone);
		data.next = &(next);
		data.args.number = (unsigned long int)getuid();
		next.args.number = dnsa->zones->id;
		if ((retval = dnsa_run_update(dc, &data, ZONE_UPDATED_YES)) != 0)
			fprintf(stderr, "Unable to mark zone as updated!\n");
		else
			printf("Zone marked as updated in database\n");
	}
	return retval;
}

void
write_zone_and_reload_nameserver(cbc_comm_line_s *cml)
{
	char config[NAME_S] = "/etc/dnsa/dnsa.conf", *buff;
	int retval = NONE;
	size_t dclen = sizeof(dnsa_config_s);
	dnsa_config_s *dc;
	dnsa_s *dnsa;
	zone_info_s *zone;

	buff = config;
	dnsa = cmdb_malloc(sizeof(dnsa_s), "dnsa in write_zone_and_reload_nameserver");
	dc = cmdb_malloc(dclen, "dc in write_zone_and_reload_nameserver");
	if ((retval = parse_dnsa_config_file(dc, config)) != 0) {
		fprintf(stderr, "Unable to parse config file??\n");
		goto cleanup;
	}
	if ((retval = dnsa_run_multiple_query(dc, dnsa, ZONE | RECORD)) != 0) {
		fprintf(stderr, "Query for zones and records failed!\n");
		goto cleanup;
	}
	zone = dnsa->zones;
	while (zone) {
		if (strncmp(zone->name, cml->build_domain, RBUFF_S) == 0) {
			check_for_updated_fwd_zone(dc, zone);
			/*create_and_write_fwd_zone(dnsa, dc, zone); */
			if ((retval = validate_fwd_zone(dc, zone, dnsa)) != 0)
				printf("Zone %s not valid\n", zone->name);
		}
		zone = zone->next;
	}
	snprintf(buff, NAME_S, "%s reload", dc->rndc);
	if ((retval = system(buff)) != 0)
		fprintf(stderr, "%s failed with %d\n", buff, retval);
/* Here we need to be able to work out what the reverse zone is, build it in
 * the database, and commit it on the name server */
	goto cleanup;

	cleanup:
		dnsa_clean_list(dnsa);
		cmdb_free(dc, dclen);
		return;
}

void
remove_ip_from_dns(cbc_config_s *cbc, cbc_comm_line_s *cml, dbdata_s *data)
{
	int retval, type;
	unsigned long int bip, server_id;
	unsigned int max;
	uint32_t ip_addr;
	dnsa_config_s *dnsa = NULL;
	dbdata_s *dns = NULL;

	server_id = data->args.number;	// save server_id so we can use this data struct.
	dnsa = cmdb_malloc(sizeof(dnsa_config_s), "dnsa in remove_ip_from_dns");
	type = RECORD_ID_ON_IP_DEST_DOM;
	max = cmdb_get_max(dnsa_extended_search_fields[type], dnsa_extended_search_args[type]);
	init_multi_dbdata_struct(&dns, max);
	copy_cbc_into_dnsa(dnsa, cbc);
	if ((retval = cbc_run_search(cbc, data, BUILD_IP_ON_SERVER_ID)) <= 0) {
		fprintf(stderr, "Cannot find build ip for server %s\n", cml->name);
	} else {
		if (retval > 1)
			fprintf(stderr, "Multiple build IP's found. Using first one\n");
		bip = data->fields.number;
		ip_addr = htonl((uint32_t)bip);
		if (!(inet_ntop(AF_INET, &ip_addr, dns->args.text, INET6_ADDRSTRLEN))) {
			fprintf(stderr, "Cannot convert IP into dotted quad\n");
			goto cleanup;
		}
		data->args.number = server_id;
		memset(data->fields.text, 0, RBUFF_S);
		if ((retval = dnsa_run_extended_search(dnsa, data, BUILD_DOM_ON_SERVER_ID)) <= 0) {
			fprintf(stderr, "Cannot find build domain from server in build_ip table\n");
			goto cleanup;
		} else if (retval > 1) {
			fprintf(stderr, "Multiple IP's. Using first one!\n");
		}
		snprintf(data->args.text, RBUFF_S, "%s", data->fields.text);
		if ((retval = dnsa_run_extended_search(dnsa, data, FWD_ZONE_ID_ON_NAME)) <= 0) {
			fprintf(stderr, "Cannot find build domain %s\n", data->args.text);
			goto cleanup;
		} else if (retval > 1) {
			fprintf(stderr, "More than one build domain for %s. Using first one\n", data->args.text);
		}
		dns->next->args.number = data->fields.number;
		snprintf(dns->next->next->args.text, HOST_S, "%s", cml->name);
		if ((retval = dnsa_run_extended_search(dnsa, dns, type)) <= 0) {
			fprintf(stderr, "Cannot find any dns entry for %s.%s @ %s\n", cml->name, cml->build_domain, dns->args.text);
			goto cleanup;
		}
		data->args.number = dns->fields.number;
		if ((retval = dnsa_run_delete(dnsa, data, RECORDS)) <= 0)
			fprintf(stderr, "No records deleted\n");
		else
			printf("%d records deleted\n", retval);
	}
	cleanup:
		free(dnsa);
		clean_dbdata_struct(dns);
		data->args.number = server_id;
}
#endif /* HAVE_DNSA */

