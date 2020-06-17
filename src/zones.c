/* 
 * 
 *  dnsa: DNS Administration
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
 *  zones.c: functions that deal with zones and records for the dnsa program
 *
 * 
 */

#include <config.h>
#include <configmake.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "dnsa_data.h"
#include "cmdb_dnsa.h"
#include "base_sql.h"
#include "dnsa_base_sql.h"

static void
print_fwd_zone_records(AILLIST *r);

static void
print_fwd_ns_mx_srv_records(char *zone, AILLIST *m);

static void
print_glue_records(char *zone, AILLIST *g);

static void
print_rev_zone_info(char *domain, AILLIST *z);

static void
print_rev_zone_records(char *domain, AILLIST *r);

static int
dnsa_populate_rev_zone(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl, AILLIST *list);

static int
dnsa_populate_record(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl, AILLIST *list);

static int
check_for_zones_and_hosts(ailsa_cmdb_s *dc, char *domain, char *top, char *host);

void
list_zones(ailsa_cmdb_s *dc)
{
	int retval;
	size_t total = 5;
	size_t len;
	AILLIST *z = ailsa_db_data_list_init();
	AILELEM *e;
	char *zone, *valid, *type, *master;
	unsigned long int serial;

	if ((retval = ailsa_basic_query(dc, ZONE_INFORMATION, z)) != 0) {
		ailsa_syslog(LOG_ERR, "ZONE_INFORMATION query failed");
		goto cleanup;
	}
	if (z->total == 0) {
		ailsa_syslog(LOG_INFO, "No forward zones found in the database");
		goto cleanup;
	}
	if ((z->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wanted %zu list factor; list contains %zu", total, z->total);
		goto cleanup;
	}
	e = z->head;
	printf("Listing zones from database %s on %s\n", dc->db, dc->dbtype);
	printf("Name\t\t\t\tValid\tSerial\t\tType\tMaster\n");
	while (e) {
		zone = ((ailsa_data_s *)e->data)->data->text;
		valid = ((ailsa_data_s *)e->next->data)->data->text;
		serial = ((ailsa_data_s *)e->next->next->data)->data->number;
		type = ((ailsa_data_s *)e->next->next->next->data)->data->text;
		master = ((ailsa_data_s *)e->next->next->next->next->data)->data->text;
		len = strlen(zone);
/*		if ((strncmp(zone->master, "(null)", COMM_S)) == 0)
			snprintf(zone->master, RANGE_S, "N/A"); */
		if (len < 8)
			printf("%s\t\t\t\t", zone);
		else if (len < 16)
			printf("%s\t\t\t", zone);
		else if (len < 24)
			printf("%s\t\t", zone);
		else if (len < 32)
			printf("%s\t", zone);
		else
			printf("%s\n\t\t\t\t", zone);
		printf("%s\t%lu\t%s\t", valid, serial, type);
		if (master)
			printf("%s", master);
		else
			printf("N/A");
		printf("\n");
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		ailsa_list_full_clean(z);
		return;
}

void
list_rev_zones(ailsa_cmdb_s *dc)
{
	if (!(dc))
		return;
	int retval;
	size_t total = 6;
	size_t len;
	char *range, *prefix, *valid, *type, *master;
	unsigned long int serial;
	AILLIST *r = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = ailsa_basic_query(dc, REV_ZONE_INFORMATION, r)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_ZONE_INFORMATION query failed");
		goto cleanup;
	}
	if (r->total == 0) {
		ailsa_syslog(LOG_INFO, "No reverse zones in DB");
		goto cleanup;
	}
	if ((r->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong factor in query. wanted %zu, got %zu", total, r->total);
		goto cleanup;
	}
	e = r->head;
	printf("Listing reverse zones from database %s on %s\n", dc->db, dc->dbtype);
// asuming IPv4 address length for range
	printf("Range\t\t\tvalid\tSerial\t\tType\tMaster\n");
	while (e) {
		range = ((ailsa_data_s *)e->data)->data->text;
		prefix = ((ailsa_data_s *)e->next->data)->data->text;
		valid = ((ailsa_data_s *)e->next->next->data)->data->text;
		serial = ((ailsa_data_s *)e->next->next->next->data)->data->number;
		type = ((ailsa_data_s *)e->next->next->next->next->data)->data->text;
		master = ((ailsa_data_s *)e->next->next->next->next->next->data)->data->text;
		len = strlen(range) + strlen(prefix) + 1;
		printf("%s/%s", range, prefix);
		if (len < 8)
			printf("\t\t\t");
		else if (len < 16)
			printf("\t\t");
		else
			printf("\t");
		printf("%s\t%lu\t%s\t", valid, serial, type);
		if (master)
			printf("%s", master);
		else
			printf("N/A");
		printf("\n");
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		ailsa_list_full_clean(r);
		return;
}

void
display_zone(char *zone, ailsa_cmdb_s *dc)
{
	int retval;
	if (!(zone) || !(dc))
		return;
	AILLIST *g = ailsa_db_data_list_init();
	AILLIST *m = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILLIST *s = ailsa_db_data_list_init();
	AILLIST *z = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(zone, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, ZONE_SERIAL_ON_NAME, z, s)) != 0) {
		ailsa_syslog(LOG_ERR, "ZONE_SERIAL_ON_NAME query failed");
		goto cleanup;
	}
	if (s->total == 0) {
		ailsa_syslog(LOG_INFO, "zone %s not found in DB", zone);
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, ZONE_RECORDS_ON_NAME, z, r)) != 0) {
		ailsa_syslog(LOG_ERR, "ZONE_RECORDS_ON_NAME query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, NS_MX_SRV_RECORDS, z, m)) != 0) {
		ailsa_syslog(LOG_ERR, "NS_MX_SRV_RECORDS query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, GLUE_ZONE_ON_ZONE_NAME, z, g)) != 0) {
		ailsa_syslog(LOG_ERR, "GLUE_ZONE_ON_ZONE_NAME query failed");
		goto cleanup;
	}
	printf("%s.\t%s.\t%s\t%lu\n", zone, dc->prins, dc->hostmaster, ((ailsa_data_s *)s->head->data)->data->number);
	print_fwd_zone_records(r);
	print_fwd_ns_mx_srv_records(zone, m);
	print_glue_records(zone, g);
	cleanup:
		ailsa_list_full_clean(g);
		ailsa_list_full_clean(m);
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(s);
		ailsa_list_full_clean(z);
		return;
}

static void
print_glue_records(char *zone, AILLIST *g)
{
	if (!(g))
		return;
	char *name, *pri, *sec;
	size_t total = 3;
	size_t len;
	AILELEM *e = g->head;

	if (g->total == 0) {
		ailsa_syslog(LOG_INFO, "No glue records for zone %s", zone);
		return;
	}
	if ((g->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong factor. Wanted %zu got %zu", total, g->total);
		return;
	}
	while (e) {
		name = ((ailsa_data_s *)e->data)->data->text;
		pri = ((ailsa_data_s *)e->next->data)->data->text;
		sec = ((ailsa_data_s *)e->next->next->data)->data->text;
		len = strlen(name);
		if (len >= 15)
			printf("%s.\tIN\tNS\t%s\n", name, pri);
		else if (len >= 7)
			printf("%s.\t\tIN\tNS\t%s\n", name, pri);
		else
			printf("%s.\t\t\tIN\tNS\t%s\n", name, pri);
		if (sec) {
			if (strcmp(sec, "none") != 0) {
				if (len >= 15)
					printf("%s.\tIN\tNS\t%s\n", name, sec);
				else if (len >= 7)
					printf("%s.\t\tIN\tNS\t%s\n", name, sec);
				else
					printf("%s.\t\t\tIN\tNS\t%s\n", name, sec);
			}
		}
		e = ailsa_move_down_list(e, total);
	}
}

static void
print_fwd_ns_mx_srv_records(char *zone, AILLIST *m)
{
	if (!(m))
		return;
	char *type, *dest, *proto, *service;
	int retval;
	size_t total = 6;
	unsigned long int pri;
	unsigned int port;
	AILELEM *e = m->head;

	if (m->total == 0) {
		ailsa_syslog(LOG_INFO, "No MX NS or SRV for zone");
		return;
	}
	if ((m->total % total) != 0) {
		ailsa_syslog(LOG_INFO, "Wrong factor in query. Wanted %zu; got %zu", total, m->total);
		return;
	}
	while (e) {
		type = ((ailsa_data_s *)e->data)->data->text;
		proto = ((ailsa_data_s *)e->next->next->data)->data->text;
		service = ((ailsa_data_s *)e->next->next->next->data)->data->text;
		pri = ((ailsa_data_s *)e->next->next->next->next->data)->data->number;
		dest = ((ailsa_data_s *)e->next->next->next->next->next->data)->data->text;
		if (strcmp(type, "SRV") == 0) {
			if ((retval = cmdb_get_port_number(proto, service, &port)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot get port number for service %s", service);
				return;
			}
			printf("_%s._%s.%s.\tIN\tSRV\t%lu 0 %u %s\n", service, proto, zone, pri, port, dest);
		} else if (strcmp(type, "MX") == 0) {
			printf("\t\t\tIN\tMX\t%lu  %s\n", pri, dest);
		} else if (strcmp(type, "NS") == 0) {
			printf("\t\t\tIN\tNS\t%s\n", dest);
		}
		e = ailsa_move_down_list(e, total);
	}
}

static void
print_fwd_zone_records(AILLIST *r)
{
	if (!(r))
		return;
	size_t total = 3;
	size_t len;
	AILELEM *e = r->head;
	char *type, *host, *dest;

	if (r->total == 0) {
		ailsa_syslog(LOG_INFO, "No forward records for this zone");
		return;
	}
	if ((r->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong factor in list. wanted %zu, got %zu", total, r->total);
		return;
	}
	while (e) {
		type = ((ailsa_data_s *)e->data)->data->text;
		host = ((ailsa_data_s *)e->next->data)->data->text;
		dest = ((ailsa_data_s *)e->next->next->data)->data->text;
		len = strlen(host);
		if (len >= 24)
			printf("%s\n\t\t\tIN\t%s\t%s\n", host, type, dest);
		else if (len >= 16)
			printf("%s\tIN\t%s\t%s\n", host, type, dest);
		else if (len >= 8)
			printf("%s\t\tIN\t%s\t%s\n", host, type, dest);
		else
			printf("%s\t\t\tIN\t%s\t%s\n", host, type, dest);
		e = ailsa_move_down_list(e, total);
	}
}

void
display_rev_zone(char *domain, ailsa_cmdb_s *dc)
{
	if (!(domain) || !(dc))
		return;
	int retval;
	char in_addr[MAC_LEN];
	unsigned long int prefix;
	AILLIST *i = ailsa_db_data_list_init();
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILLIST *z = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d;

	memset(in_addr, 0, MAC_LEN);
	if ((retval = cmdb_add_string_to_list(domain, l)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(dc, REV_ZONE_INFO_ON_RANGE, l, z)) != 0)
		goto cleanup;
	if (z->total == 0) {
		ailsa_syslog(LOG_INFO, "Zone %s does not exist", domain);
		goto cleanup;
	} else {
		e = z->head->next->next->next;
		d = e->data;
		if (strcmp("master", d->data->text) != 0) {
			ailsa_syslog(LOG_INFO, "Zone %s not a master zone");
			goto cleanup;
		}
		e = e->prev;
		d = e->data;
	}
	prefix = strtoul(d->data->text, NULL, 10);
	get_in_addr_string(in_addr, domain, prefix);
	if ((retval = ailsa_argument_query(dc, REV_ZONE_ID_ON_RANGE, l, i)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(dc, REV_RECORDS_ON_ZONE_ID, i, r)) != 0)
		goto cleanup;
	print_rev_zone_info(domain, z);
	print_rev_zone_records(in_addr, r);
	cleanup:
		ailsa_list_full_clean(i);
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(z);
}

void
print_rev_zone_info(char *domain, AILLIST *z)
{
	if (!(domain) || !(z))
		return;
	AILELEM *e = z->head;

	printf("%s\t", domain);
	printf("%s\t%lu\n", ((ailsa_data_s *)e->next->data)->data->text, ((ailsa_data_s *)e->data)->data->number);
}

static void
print_rev_zone_records(char *domain, AILLIST *r)
{
	if (!(domain) || !(r))
		return;
	AILELEM *e = r->head;
	ailsa_data_s *d;

	while (e) {
		d = e->data;
		printf("%s.%s\t", d->data->text, domain);
		e = e->next;
		d = e->data;
		printf("%s\n", d->data->text);
		e = e->next;
	}
}

int
check_zone(char *domain, ailsa_cmdb_s *dc)
{
	char *command, syscom[RBUFF_S];
	int retval;
	
	command = &syscom[0];
	
	snprintf(command, RBUFF_S, "%s %s %s%s", dc->chkz, domain, dc->dir, domain);
	retval = system(syscom);
	if (retval != 0)
		retval = CHKZONE_FAIL;
	return retval;
}

int
commit_fwd_zones(ailsa_cmdb_s *dc, char *name)
{
	if (!(dc))
		return AILSA_NO_DATA;
	int retval;
	size_t len = 2;
	char *zone, *type;
	char *command = ailsa_calloc(CONFIG_LEN, "command in commit_fwd_zones");
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILELEM *e;

	if (name) {
		zone = name;
		if ((retval = cmdb_add_string_to_list(zone, l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone name to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(dc, ZONE_TYPE_ON_NAME, l, r)) != 0) {
			ailsa_syslog(LOG_ERR, "ZONE_TYPE_ON_NAME query failed");
			goto cleanup;
		}
		if (r->total == 0) {
			ailsa_syslog(LOG_INFO, "zone %s not found in database");
			goto cleanup;
		}
		e = r->head;
		type = ((ailsa_data_s *)e->data)->data->text;
		if (strcmp(type, "master") == 0) {
			if ((retval = cmdb_validate_zone(dc, FORWARD_ZONE, zone)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot validate zone %s", zone);
				goto cleanup;
			}
		} else {
			ailsa_syslog(LOG_INFO, "zone %s not master. %s", zone, type);
			goto cleanup; // maybe go on to write the zone file anyway??
		}
	} else {
		if ((retval = ailsa_basic_query(dc, ZONE_NAME_TYPES, r)) != 0) {
			ailsa_syslog(LOG_ERR, "ZONE_NAME_TYPES query failed");
			goto cleanup;
		}
		if (r->total == 0) {
			ailsa_syslog(LOG_INFO, "No zones found in database");
			goto cleanup;
		}
		if ((r->total % len) != 0) {
			ailsa_syslog(LOG_ERR, "ZONE_NAME_TYPES wrong factor. wanted %zu got total %zu", len, r->total);
			goto cleanup;
		}
		e = r->head;
		while (e) {
			zone = ((ailsa_data_s *)e->data)->data->text;
			type = ((ailsa_data_s *)e->next->data)->data->text;
			if (strcmp(type, "master") == 0) {
				if ((retval = cmdb_validate_zone(dc, FORWARD_ZONE, zone)) != 0) {
					ailsa_syslog(LOG_ERR, "Cannot validate zone %s", zone);
				}
			}
			e = ailsa_move_down_list(e, len);
		}
	}
	if ((retval = cmdb_write_fwd_zone_config(dc)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write out forward zone configuration");
		goto cleanup;
	}
	snprintf(command, CONFIG_LEN, "%s reload", dc->rndc);
	if ((retval = system(command)) != 0)
		ailsa_syslog(LOG_ERR, "Reload of nameserver failed");

	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		my_free(command);
		return retval;
}

int
commit_rev_zones(ailsa_cmdb_s *dc, char *name)
{
	if (!(dc))
		return AILSA_NO_DATA;
	int retval;
	char *comm = ailsa_calloc(DOMAIN_LEN, "comm in commit_rev_zones");
	char *zone;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d;

	if ((retval = ailsa_basic_query(dc, REV_ZONES_NET_RANGE, l)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_ZONES_NET_RANGE query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "No reverse zones were found");
		goto cleanup;
	}
	e = l->head;
	while (e) {
		d = e->data;
		zone = d->data->text;
		if (name) {
			if (strncmp(name, zone, DOMAIN_LEN) == 0)
				if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, zone)) != 0)
					ailsa_syslog(LOG_ERR, "Unable to validate zone %s");
		} else {
			if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, zone)) != 0)
				ailsa_syslog(LOG_ERR, "Unable to validate zone %s");
		}
		e = e->next;
	}
	if ((retval = cmdb_write_rev_zone_config(dc)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to create reverse config");
		goto cleanup;
	}
	snprintf(comm, CONFIG_LEN, "%s reload", dc->rndc);
	if ((retval = system(comm)) != 0)
		ailsa_syslog(LOG_ERR, "Reload of nameserver failed");
	cleanup:
		ailsa_list_full_clean(l);
		my_free(comm);
		return retval;
}

int
display_multi_a_records(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	int retval = 0, type = RECORDS_ON_DEST_AND_ID;
	unsigned int f = dnsa_extended_search_fields[type];
	unsigned int a = dnsa_extended_search_args[type];
	unsigned int max = cmdb_get_max(a, f);
	size_t len = sizeof(rev_zone_info_s);
	dnsa_s *dnsa;
	dbdata_s *start;
	rev_zone_info_s *rzone;
	record_row_s *records;
	preferred_a_s *prefer;

	dnsa = ailsa_calloc(sizeof(dnsa_s), "dnsa in display_multi_a_records");
	rzone = ailsa_calloc(len, "rzone in display_multi_a_records");
	init_rev_zone_struct(rzone);
	dnsa->rev_zones = rzone;
	if ((retval = dnsa_run_multiple_query(
		dc, dnsa, DUPLICATE_A_RECORD | PREFERRED_A)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	init_multi_dbdata_struct(&start, max);
	if (cm->dest) {
		select_specific_ip(dnsa, cm);
		if (!(dnsa->records))
			fprintf(stderr, "No multiple A records for IP %s\n",
				cm->dest);
		else
			print_multiple_a_records(dc, start, dnsa);
		clean_dbdata_struct(start);
		dnsa_clean_list(dnsa);
		return NONE;
	}
	snprintf(rzone->net_range, RANGE_S, "%s", cm->domain);
	if ((retval = dnsa_run_search(dc, dnsa, REV_ZONE_PREFIX)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	cm->prefix = rzone->prefix;
	fill_rev_zone_info(rzone, cm, dc);
	if (rzone->prefix == NONE) {
		printf("Net range %s does not exist in database\n", cm->domain);
		dnsa_clean_list(dnsa);
		clean_dbdata_struct(start);
		return NO_DOMAIN;
	}
	get_a_records_for_range(&(dnsa->records), dnsa->rev_zones);
	records = dnsa->records;
	if (!records)
		printf("No duplicate entries for range %s\n", cm->domain);
	while (records) {
		prefer = dnsa->prefer;
		printf("Destination %s has %lu records",
		       records->dest, records->id);
		while (prefer) {
			if (strncmp(prefer->ip, records->dest, RANGE_S) == 0)
				printf("; preferred PTR is %s", prefer->fqdn);
			prefer = prefer->next;
		}
		printf("\n");
		records = records->next;
	}
	records = dnsa->records;
	if (records) {
		printf("If you want to see the A records for a specific IP use the ");
		printf("-i option\nE.G. dnsa -u -i <IP-Address>\n");
	}
	clean_dbdata_struct(start);
	dnsa_clean_list(dnsa);
	return retval;
}

void
print_multiple_a_records(ailsa_cmdb_s *dc, dbdata_s *start, dnsa_s *dnsa)
{
	int i, j, k;
	char name[RBUFF_S], *fqdn;
	time_t create;
	dbdata_s *dlist;
	record_row_s *records = dnsa->records;
	preferred_a_s *prefer = dnsa->prefer, *mark = NULL;
	fqdn = &name[0];
	while (records) {
		dlist = start;
		printf("Destination %s has %lu records; * denotes preferred PTR record\n",
		records->dest, records->id);
		snprintf(dlist->args.text, RANGE_S, "%s", records->dest);
		i = dnsa_run_extended_search(dc, start, RECORDS_ON_DEST_AND_ID);
		for (j = 0; j < i; j++) {
			snprintf(fqdn, RBUFF_S, "%s.%s",
				 dlist->fields.text, dlist->next->fields.text);
			prefer = dnsa->prefer;
			k = 0;
			while (prefer) {
				if (strncmp(fqdn, prefer->fqdn, RBUFF_S) == 0) {
					mark = prefer;
					printf("     *  %s\n", fqdn);
					k++;
				}
				prefer=prefer->next;
			}
			if (k == 0)
				printf("\t%s\n", fqdn);
			dlist = dlist->next->next->next;
		}
		printf("\n");
		records = records->next;
		dlist = start->next->next->next;
		clean_dbdata_struct(dlist);
		dlist = start->next->next;
		dlist->next = NULL;
	}
	if (mark) {
		create = (time_t)mark->ctime;
		printf("Preferred A record created by %s on %s",
get_uname(mark->cuser), ctime(&create));
	}
}

int
mark_preferred_a_record(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	int retval = 0;
	uint32_t ip_addr;
	unsigned long int ip;
	dnsa_s *dnsa;
	zone_info_s *zone;
	preferred_a_s *prefer;

	dnsa = ailsa_calloc(sizeof(dnsa_s), "dnsa in mark_preferred_a_record");
	zone = ailsa_calloc(sizeof(zone_info_s), "zone in mark_preferred_a_record");
	init_zone_struct(zone);
	dnsa->zones = zone;
	if ((retval = dnsa_run_multiple_query(dc, dnsa,
		 DUPLICATE_A_RECORD | PREFERRED_A)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	select_specific_ip(dnsa, cm);
	prefer = dnsa->prefer;
	if (inet_pton(AF_INET, cm->dest, &ip_addr))
		ip = (unsigned long int)htonl(ip_addr);
	else
		return USER_INPUT_INVALID;
	while (prefer) {
		if (prefer->ip_addr == ip) {
			printf("IP %s already has preferred A record %s\n",
			       cm->dest, prefer->fqdn);
			dnsa_clean_list(dnsa);
			return NONE;
		} else {
			prefer = prefer->next;
		}
	}
	if (!(dnsa->records)) {
		fprintf(stderr, "No multiple A records for IP %s\n",
			cm->dest);
		return CANNOT_ADD_A_RECORD;
	} else {
		retval = get_preferred_a_record(dc, cm, dnsa);
	}
	if (retval != 0)
		return retval;
	printf("IP: %s\tIP Addr: %lu\tRecord ID: %lu\n",
	       dnsa->prefer->ip, dnsa->prefer->ip_addr, dnsa->prefer->record_id);
	dnsa->prefer->cuser = dnsa->prefer->muser = (unsigned long int)getuid();
	if ((retval = dnsa_run_insert(dc, dnsa, PREFERRED_AS)) != 0)
		fprintf(stderr, "Cannot insert preferred A record\n");
	else
		printf("Database updated with preferred A record\n");
	dnsa_clean_list(dnsa);
	return retval;
}

int
get_preferred_a_record(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm, dnsa_s *dnsa)
{
	char *name = cm->dest;
	char fqdn[RBUFF_S], cl_fqdn[RBUFF_S], *cl_name;
	int i = 0, type = RECORDS_ON_DEST_AND_ID;
	uint32_t ip_addr;
	unsigned int f = dnsa_extended_search_fields[type];
	unsigned int a = dnsa_extended_search_args[type];
	unsigned int max = cmdb_get_max(a, f);
	dbdata_s *start, *list;
	preferred_a_s *prefer;
	record_row_s *rec = dnsa->records;

	if (!(prefer = malloc(sizeof(preferred_a_s))))
		report_error(MALLOC_FAIL, "prefer in get_preferred_a_record");
	init_preferred_a_struct(prefer);
	dnsa->prefer = prefer;
	init_multi_dbdata_struct(&start, max);
	while (rec) {
		if (strncmp(name, rec->dest, RBUFF_S) == 0) {
			snprintf(prefer->ip, RANGE_S, "%s", cm->dest);
			inet_pton(AF_INET, rec->dest, &ip_addr);
			prefer->ip_addr = (unsigned long int) htonl(ip_addr);
			i++;
		}
		if (rec->next)
			rec = rec->next;
		else
			rec = NULL;
	}
	snprintf(start->args.text, RANGE_S, "%s", name);
	i = dnsa_run_extended_search(dc, start, RECORDS_ON_DEST_AND_ID);
	list = start;
	name = fqdn;
	cl_name = cl_fqdn;
	i = 0;
	while (list) {
		snprintf(name, RBUFF_S, "%s.%s",
list->fields.text, list->next->fields.text);
		snprintf(cl_name, RBUFF_S, "%s.%s",
cm->host, cm->domain);
		if (strncmp(name, cl_name, RBUFF_S) == 0) {
			i++;
			prefer->record_id = list->next->next->fields.number;
			snprintf(prefer->fqdn, RBUFF_S, "%s", name);
		}
		list = list->next->next->next;
	}
	if (i == 0) {
		fprintf(stderr,
"Your FQDN is not associated with this IP address\n\
If you want it associated with this IP address, please add it as an A record\n\
Curently you cannot add FQDN's not authoritative on this DNS server\n");
		return CANNOT_ADD_A_RECORD;
	}
	clean_dbdata_struct(start);
	return NONE;
}

int
delete_preferred_a(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	char *ip_addr = cm->dest;
	int retval = 0;
	int i = 0;
	dnsa_s *dnsa;
	dbdata_s data;
	preferred_a_s *prefer;

	dnsa = ailsa_calloc(sizeof(dnsa_s), "dnsa in delete_preferred_a");
	init_dbdata_struct(&data);
	if ((retval = dnsa_run_query(dc, dnsa, PREFERRED_A)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	prefer = dnsa->prefer;
	while (prefer) {
		if (strncmp(ip_addr, prefer->ip, RANGE_S) == 0) {
			printf("Found IP address %s in preferred list\n",
			       ip_addr);
			i++;
			break;
		}
		prefer = prefer->next;
	}
	if (i == 0) {
		fprintf(stderr, "IP %s does not have a preferred A record\n",
			ip_addr);
		dnsa_clean_list(dnsa);
		return USER_INPUT_INVALID;
	}
	data.args.number = prefer->prefa_id;
	if ((retval = dnsa_run_delete(dc, &data, PREFERRED_AS)) != 1)
		printf("Unable to delete IP %s from preferred list\n", ip_addr);
	else
		printf("Deleted IP %s from preferred list\n", ip_addr);
	dnsa_clean_list(dnsa);
	return retval;
}

int
add_host(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval;
	unsigned int query;
	void *data;
	uid_t uid = getuid();
	AILLIST *rec = ailsa_db_data_list_init();
	AILLIST *z = ailsa_db_data_list_init();

	if ((retval = cmdb_add_zone_id_to_list(cm->domain, FORWARD_ZONE, dc, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone id to list");
		goto cleanup;
	}
	if (rec->total == 0) {
		ailsa_syslog(LOG_INFO, "Zone %s does not exist", cm->domain);
		goto cleanup;
	} else if (rec->total > 1) {
		ailsa_syslog(LOG_INFO, "More than one domain for %s? Using first one", cm->domain);
		while (rec->total > 1) {
			retval = ailsa_list_remove(rec, rec->tail, &data);
			if (retval == 0 && rec->destroy != NULL)
				rec->destroy(data);
		}
	}
	if ((retval = dnsa_populate_record(dc, cm, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot populate list with record details");
		goto cleanup;
	}
	if ((retval = cmdb_check_for_fwd_record(dc, rec)) < 0) {
		ailsa_syslog(LOG_ERR, "Cannot check for fwd record");
		goto cleanup;
	} else if (retval > 0) {
		ailsa_syslog(LOG_INFO, "Record exists in database");
		retval = 0;
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	switch (rec->total) {
	case 6:
		query = INSERT_RECORD_BASE;
		break;
	case 7:
		query = INSERT_RECORD_MX;
		break;
	case 9:
		query = INSERT_RECORD_SRV;
		break;
	default:
		ailsa_syslog(LOG_ERR, "Wrong number in list record list: %zu", rec->total);
		retval = WRONG_LENGTH_LIST;
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(dc, query, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Insert record query %u failed", query);
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list((unsigned long int)uid, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add muser to update list");
		goto cleanup;
	}
	if ((retval = cmdb_add_zone_id_to_list(cm->domain, FORWARD_ZONE, dc, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone id to list");
		goto cleanup;
	}
	if ((retval = ailsa_update_query(dc, update_queries[SET_FWD_ZONE_UPDATED], z)) != 0)
		ailsa_syslog(LOG_ERR, "SET_FWD_ZONE_UPDATED query failed");

	cleanup:
		ailsa_list_full_clean(rec);
		ailsa_list_full_clean(z);
		return retval;
}

static int
dnsa_populate_record(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl, AILLIST *list)
{
	if (!(cbc) || !(dcl) || !(list))
		return AILSA_NO_DATA;
	int retval;

	if ((retval = cmdb_add_string_to_list(dcl->rtype, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add record type to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(dcl->host, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add host to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(dcl->dest, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add destination to list");
		goto cleanup;
	}
	if ((strcmp(dcl->rtype, "MX") == 0) || (strcmp(dcl->rtype, "SRV") == 0)) {
		if ((retval = cmdb_add_number_to_list(dcl->prefix, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add priority to list");
			goto cleanup;
		}
	}
	if (strcmp(dcl->rtype, "SRV") == 0) {
		if ((retval = cmdb_add_string_to_list(dcl->protocol, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add protocol to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(dcl->service, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add service to list");
			goto cleanup;
		}
	}
	cleanup:
		return retval;
}

int
add_cname_to_root_domain(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	AILLIST *c = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *data;
	char *domain = ailsa_calloc(DOMAIN_LEN, "domain in add_cname_to_root_domain");
	char *tmp;
	int retval;

	if ((retval = check_for_zones_and_hosts(dc, cm->domain, cm->toplevel, cm->host)) != 0)
		goto cleanup;
	if (!(cm->toplevel)) {
		snprintf(domain, DOMAIN_LEN, "%s", cm->domain);
		tmp = strchr(domain, '.');
		if (tmp) {
			tmp++;
			if ((retval = cmdb_add_zone_id_to_list(tmp, FORWARD_ZONE, dc, c)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot search for domain %s", tmp);
				goto cleanup;
			}
		} else {
			ailsa_syslog(LOG_ERR, "Cannot determine top level domain");
			retval = AILSA_NO_TOP_LEVEL_DOMAIN;
			goto cleanup;
		}
		cm->toplevel = strndup(tmp, DOMAIN_LEN);
	} else {
		if ((retval = cmdb_add_zone_id_to_list(cm->toplevel, FORWARD_ZONE, dc, c)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot get zone id for domain %s", domain);
			goto cleanup;
		}
	}
	if ((retval = cmdb_add_string_to_list("CNAME", c)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add CNAME type to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cm->host, c)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add host to list");
		goto cleanup;
	}
	snprintf(domain, DOMAIN_LEN, "%s.%s.", cm->host, cm->domain);
	if ((retval = cmdb_add_string_to_list(domain, c)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add destination to list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(c)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot populate cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(dc, INSERT_RECORD_BASE, c)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_RECORD_BASE query failed");
		goto cleanup;
	}
	e = c->head;
	data = e->data;
	if ((retval = set_db_row_updated(dc, SET_FWD_ZONE_UPDATED, NULL, data->data->number)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot set zone table updated");
		goto cleanup;
	}

	cleanup:
		ailsa_list_full_clean(c);
		my_free(domain);
		return retval;
}

static int
check_for_zones_and_hosts(ailsa_cmdb_s *dc, char *dom, char *top, char *host)
{
	if (!(dc) || !(dom) || !(host))
		return AILSA_NO_DATA;
	AILLIST *d = ailsa_db_data_list_init();
	AILLIST *t = ailsa_db_data_list_init();
	AILLIST *z = ailsa_db_data_list_init();
	size_t total;
	char *domain = ailsa_calloc(DOMAIN_LEN, "domain in add_cname_to_root_domain");
	char *tmp;
	int retval;

	if ((retval = cmdb_add_zone_id_to_list(dom, FORWARD_ZONE, dc, d)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot search for domain %s", dom);
		goto cleanup;
	}
	if (d->total == 0) {
		ailsa_syslog(LOG_ERR, "Domain %s does not exist", dom);
		goto cleanup;
	}
	if (top) {
		if ((retval = cmdb_add_zone_id_to_list(top, FORWARD_ZONE, dc, t)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot search for domain %s", top);
			goto cleanup;
		}
		if (t->total == 0) {
			ailsa_syslog(LOG_ERR, "Domain %s does not exist", top);
			goto cleanup;
		}
	} else {
		snprintf(domain, DOMAIN_LEN, "%s", dom);
		tmp = strchr(domain, '.');
		if (tmp) {
			tmp++;
			if ((retval = cmdb_add_zone_id_to_list(tmp, FORWARD_ZONE, dc, t)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot search for domain %s", tmp);
				goto cleanup;
			}
		} else {
			ailsa_syslog(LOG_ERR, "Cannot determine top level domain");
			retval = AILSA_NO_TOP_LEVEL_DOMAIN;
			goto cleanup;
		}
		if (t->total == 0) {
			ailsa_syslog(LOG_ERR, "Domain %s does not exist", tmp);
			goto cleanup;
		}
	}
	if ((retval = cmdb_add_string_to_list(host, d)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add host name to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(host, t)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add host name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, RECORD_ON_ZONE_AND_HOST_AND_A, d, z)) != 0) {
		ailsa_syslog(LOG_ERR, "RECORD_ON_ZONE_AND_HOST_AND_A query failed");
		goto cleanup;
	}
	total = z->total;
	if (total == 0) {
		ailsa_syslog(LOG_ERR, "A record %s does not exist in %s", host, dom);
		retval = AILSA_NO_HOST;
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, RECORD_ON_ZONE_AND_HOST, t, z)) != 0) {
		ailsa_syslog(LOG_ERR, "RECORD_ON_ZONE_AND_HOST query failed");
		goto cleanup;
	}
	if (total != z->total) {
		ailsa_syslog(LOG_ERR, "A record exists for host %s in domain %s", host, top);
		retval = AILSA_HOST_EXISTS;
	}
	cleanup:
		my_free(domain);
		ailsa_list_full_clean(d);
		ailsa_list_full_clean(t);
		ailsa_list_full_clean(z);
		return retval;
}

int
delete_record(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval;
	unsigned int query, delete;
	AILLIST *rec = ailsa_db_data_list_init();
	AILLIST *list = ailsa_db_data_list_init();

	if ((retval = cmdb_add_zone_id_to_list(cm->domain, FORWARD_ZONE, dc, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cm->rtype, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add record type to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cm->host, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add host to list");
		goto cleanup;
	}
	if (cm->prefix) {
		if ((retval = cmdb_add_number_to_list(cm->prefix, rec)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add priority to list");
			goto cleanup;
		}
	}
	if (cm->protocol) {
		if ((retval = cmdb_add_string_to_list(cm->protocol, rec)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add protocol to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(cm->service, rec)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add service to list");
			goto cleanup;
		}
	}
	switch (rec->total) {
	case 3:
		query = RECORD_ID_ON_HOST_ZONE_TYPE;
		delete = DELETE_REC_TYPE_HOST;
		break;
	case 4:
		query = RECORD_IN_ON_PRI;
		delete = DELETE_REC_PRI;
		break;
	case 6:
		query = RECORD_IN_ON_PROTOCOL;
		delete = DELETE_REC_PROTO;
		break;
	default:
		ailsa_syslog(LOG_INFO, "Got %zu numbers in list?");
		retval = WRONG_LENGTH_LIST;
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, query, rec, list)) != 0) {
		ailsa_syslog(LOG_ERR, "query for record to delete failed");
		goto cleanup;
	}
	if (list->total == 0) {
		ailsa_syslog(LOG_INFO, "No records found to delete");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(dc, delete_queries[delete], rec)) != 0)
		ailsa_syslog(LOG_ERR, "delete query failed");

	cleanup:
		ailsa_list_full_clean(rec);
		ailsa_list_full_clean(list);
		return retval;
}

int
add_fwd_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval = add_forward_zone(dc, cm->domain);
	return retval;
}

int
delete_fwd_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	char *command = ailsa_calloc(CONFIG_LEN, "command in delete_fwd_zone");
	int retval;
	AILLIST *z = ailsa_db_data_list_init();

	if ((retval = cmdb_add_zone_id_to_list(cm->domain, FORWARD_ZONE, dc, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone id to list");
		goto cleanup;
	}
	if (z->total == 0) {
		ailsa_syslog(LOG_INFO, "Domain %s does not exist in the database", cm->domain);
		goto cleanup;
	} else if (z->total > 1) {
		ailsa_syslog(LOG_INFO, "More than one domain for %s? Using first one", cm->domain);
	}
	// We are not checking if any of the records in this zone are in use. Beware!
	if ((retval = ailsa_delete_query(dc, delete_queries[DELETE_FWD_ZONE], z)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_FWD_ZONE query failed");
		goto cleanup;
	}
	if ((retval = cmdb_write_fwd_zone_config(dc)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write forward zone's config");
		goto cleanup;
	}
	snprintf(command, CONFIG_LEN, "%s reload", dc->rndc);
	if ((retval = system(command)) != 0)
		ailsa_syslog(LOG_ERR, "Reload of nameserver failed");

	cleanup:
		ailsa_list_full_clean(z);
		my_free(command);
		return retval;
}

int
add_rev_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	char *command = ailsa_calloc(CONFIG_LEN, "command in add_rev_zone");
	int retval;
	AILLIST *rev = ailsa_db_data_list_init();
	AILLIST *rid = ailsa_db_data_list_init();

	if ((retval = cmdb_add_zone_id_to_list(cm->domain, REVERSE_ZONE, dc, rid)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add reverse zone id to list");
		goto cleanup;
	}
	if (rid->total > 0) {
		ailsa_syslog(LOG_INFO, "Zone %s already in database", cm->domain);
		goto cleanup;
	}
	if ((retval = dnsa_populate_rev_zone(dc, cm, rev)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create list for DB insert");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(dc, INSERT_REVERSE_ZONE, rev)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_REVERSE_ZONE query failed");
		goto cleanup;
	}
	if (!(cm->ztype)) {
		if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, cm->domain)) != 0) {
			ailsa_syslog(LOG_ERR, "Unable to validate new zone %s", cm->domain);
			goto cleanup;
		}
	}
	if ((retval = cmdb_write_rev_zone_config(dc)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write reverse zones configuration");
		goto cleanup;
	}
	snprintf(command, CONFIG_LEN, "%s reload", dc->rndc);
	if ((retval = system(command)) != 0)
		ailsa_syslog(LOG_ERR, "Reload of nameserver failed!");

	cleanup:
		my_free(command);
		ailsa_list_full_clean(rev);
		ailsa_list_full_clean(rid);
		return retval;
}

int
delete_reverse_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	char *command = ailsa_calloc(CONFIG_LEN, "command in delete_reverse_zone");
	int retval;
	AILLIST *rev = ailsa_db_data_list_init();

	if ((retval = cmdb_add_zone_id_to_list(cm->domain, REVERSE_ZONE, dc, rev)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add reverse zone id to list");
		goto cleanup;
	}
	if (rev->total == 0) {
		ailsa_syslog(LOG_INFO, "Zone %s does not exist to delete", cm->domain);
		goto cleanup;
	} else if (rev->total > 1) {
		ailsa_syslog(LOG_INFO, "More than one zone for %s? Using first one", cm->domain);
	}
	if ((retval = ailsa_delete_query(dc, delete_queries[DELETE_REV_ZONE], rev)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_REV_ZONE query failed");
		goto cleanup;
	}
	if ((retval = cmdb_write_rev_zone_config(dc)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write out reverse zone's config");
		goto cleanup;
	}
	snprintf(command, CONFIG_LEN, "%s reload", dc->rndc);
	if ((retval = system(command)) != 0)
		ailsa_syslog(LOG_ERR, "Reload of nameserver failed");

	cleanup:
		ailsa_list_full_clean(rev);
		my_free(command);
		return retval;
}

int
build_reverse_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	int retval = 0, a_rec;
	unsigned long int serial;
	dnsa_s *dnsa;
	dbdata_s serial_d, zone_info_d, user_d, *data;
	record_row_s *rec;
// Set to NULL so we can check if there are no records to add / delete
	rev_record_row_s *add = NULL, *delete = NULL, *list;

	dnsa = ailsa_calloc(sizeof(dnsa_s), "dnsa in build_reverse_zone");
	init_multi_dbdata_struct(&data, 1);
	if ((retval = dnsa_run_multiple_query(
	       dc, dnsa, DUPLICATE_A_RECORD | PREFERRED_A | REV_ZONE | ZONE)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((retval = get_correct_rev_zone_and_preferred_records(dnsa, cm)) > 0) {
		dnsa_clean_list(dnsa);
		return retval;
	} else if (retval < 0) {
// No Duplicate records. Just convert all A records
		dnsa_clean_records(dnsa->records);
		dnsa_clean_prefer(dnsa->prefer);
		dnsa->records = NULL;
		dnsa->prefer = NULL;
	}
	rec = dnsa->records; // Holds duplicate A records
	dnsa->records = NULL;
	if ((retval = dnsa_run_multiple_query(dc, dnsa, ALL_A_RECORD | REV_RECORD)) != 0) {
		dnsa_clean_records(rec);
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((a_rec = get_a_records_for_range(&(dnsa->records), dnsa->rev_zones)) == 0) {
		dnsa_clean_records(rec);
		dnsa_clean_list(dnsa);
		printf("No A records for net_range %s\n", cm->domain);
		return NO_RECORDS;
	}
	get_rev_records_for_range(&(dnsa->rev_records), dnsa->rev_zones);
	add_int_ip_to_records(dnsa);
	trim_forward_record_list(dnsa, rec);
	retval = rev_records_to_add(dnsa, &add);
	retval = rev_records_to_delete(dnsa, &delete);
	dnsa_clean_rev_records (dnsa->rev_records);
	list = delete;
	while (list) {
		data->args.number = list->record_id;
		printf("Deleting record %lu\n", data->args.number);
		dnsa_run_delete(dc, data, REV_RECORDS);
		list = list->next;
	}
	dnsa->rev_records = list = add;
	retval = 0;
	while (dnsa->rev_records) {
		printf("Adding PTR record for %s\n", dnsa->rev_records->host);
		retval += dnsa_run_insert(dc, dnsa, REV_RECORDS);
		dnsa->rev_records = dnsa->rev_records->next;
	}
	if (add || delete) {
		serial = get_zone_serial();
		if (serial > dnsa->rev_zones->serial)
			dnsa->rev_zones->serial = serial;
		else
			dnsa->rev_zones->serial++;
		init_dbdata_struct(&serial_d);
		init_dbdata_struct(&zone_info_d);
		init_dbdata_struct(&user_d);
		serial_d.args.number = dnsa->rev_zones->serial;
		zone_info_d.args.number = dnsa->rev_zones->rev_zone_id;
		user_d.args.number = (unsigned long int)getuid();
		serial_d.next = &user_d;
		user_d.next = &zone_info_d;
		if ((retval = dnsa_run_update(dc, &serial_d, REV_ZONE_SERIAL)) != 0)
			fprintf(stderr, "Cannot update rev zone serial!\n");
		else
			fprintf(stderr, "Rev zone serial number updated\n");
	} else {
		printf("No rev records to add / delete for range %s\n",
		       dnsa->rev_zones->net_range);
	}
	dnsa->rev_records = add;
	clean_dbdata_struct(data);
	dnsa_clean_rev_records(delete);
	dnsa_clean_records(rec);
	dnsa_clean_list(dnsa);
	return NONE;
}

int
get_correct_rev_zone_and_preferred_records(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	int retval = 0;
	if ((retval = get_rev_zone(dnsa, cm)) != 0) {
		return retval;
	}
	if ((retval = get_a_records_for_range(&(dnsa->records), dnsa->rev_zones)) == 0) {
		return -1;
	} else {
		retval = NONE;
		if ((retval = get_pref_a_for_range(&(dnsa->prefer), dnsa->rev_zones)) == 0) {
			return retval;
		} else {
			retval = NONE;
		}
	}
	return retval;
}

int
get_fwd_zone(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	zone_info_s *fwd, *list, *next;
	fwd = dnsa->zones;
	list = NULL;
	if (fwd->next)
		next = fwd->next;
	else
		next = NULL;
	while (fwd) {
		if (strncmp(fwd->name, cm->domain, RBUFF_S) == 0) {
			list = fwd;
			fwd = next;
		} else {
			free (fwd);
			fwd = next;
		}
		if (next)
			next = fwd->next;
	}
	dnsa->zones = list;
	if (list) {
		list->next = NULL;
		return NONE;
	}
	fprintf(stderr, "Forward domain %s not found\n", cm->domain);
	return NO_DOMAIN;
}

int
get_rev_zone(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	rev_zone_info_s *rev, *list, *next;
	rev = dnsa->rev_zones;
	list = NULL;
	if (rev->next)
		next = rev->next;
	else
		next = NULL;
	while (rev) {
		if (strncmp(rev->net_range, cm->domain, RANGE_S) == 0) {
			list = rev;
			rev = next;
		} else {
			free(rev);
			rev = next;
		}
		if (next)
			next = rev->next;
	}
	dnsa->rev_zones = list;
	if (list) {
		list->next = NULL;
		return NONE;
	}
	fprintf(stderr, "Reverse domain %s not found\n", cm->domain);
	return NO_DOMAIN;
}

int
get_rev_records_for_range(rev_record_row_s **records, rev_zone_info_s *zone)
{
	int i = 0;
	rev_record_row_s *list, *prev, *next, *tmp, *rec;
	list = *records;
	rec = prev = list;
	if (rec)
		next = rec->next;
	else
		return NONE;
	while (rec) {
		tmp = rec;
		if (zone->rev_zone_id != rec->rev_zone) {
			free(rec);
			rec = next;
			if (rec)
				next = rec->next;
			if (tmp == list)
				list = prev = rec;
			else
				prev->next = rec;
		} else {
			i++;
			if (prev != rec)
				prev = prev->next;
			rec = next;
			if (rec)
				next = rec->next;
		}
	}
	if (*records != list)
		*records = list;
	return i;
}

void
trim_forward_record_list(dnsa_s *dnsa, record_row_s *rec)
{
	char host[RBUFF_S], *newhost;
	int retval;
	record_row_s *fwd = dnsa->records; // List of A records
	record_row_s *list, *tmp, *fwd_list, *prev;
	fwd_list = prev = fwd;
	while (fwd) {
// Get rid of @. in the start of these types of A records
		newhost = host;
		snprintf(newhost, RBUFF_S, "%s", fwd->host);
		if (host[0] == '@') {
			newhost = strchr(host, '.');
			newhost++;
			snprintf(fwd->host, RBUFF_S, "%s", newhost);
		}
		list = rec; // List of duplicate IP address in the net range
		tmp = fwd->next;
// Check if in duplicate and prefer list
		if ((retval = check_dup_and_pref_list(list, fwd, dnsa)) > 0) {
			if (fwd_list == fwd) {
				fwd_list = prev = tmp;
				free(fwd);
			} else {
				free(fwd);
				prev->next = tmp;
			}
/* As we are deleting this record, then no more checks should be performed.
 * Therefore, we should continue the loop */
			if ((prev->next != fwd) && (prev->next != tmp) && (prev != tmp))
				prev = prev->next;
			fwd = tmp;
			continue;
		}
		if (prev != fwd)
			prev = prev->next;
		fwd = fwd->next;
	}
	if (fwd_list != dnsa->records)
		dnsa->records = fwd_list;
}

// Build a list of the reverse records we need to add to the database.
int
rev_records_to_add(dnsa_s *dnsa, rev_record_row_s **rev)
{
	int i;
	size_t len;
	record_row_s *fwd = dnsa->records; // List of A records
	rev_record_row_s *rev_list;

	while (fwd) {
// Now check if the forward record is in the reverse list
		i = 0;
		rev_list = dnsa->rev_records;
		while (rev_list) {
			if (rev_list->ip_addr == fwd->ip_addr) {
				len = strlen(fwd->host);
				if (strncmp(fwd->host, rev_list->dest, len) == 0)
					i++;
				break;
			} else {
				rev_list = rev_list->next;
			}
		}
		if (i == 0)
			if ((insert_into_rev_add_list(dnsa, fwd, rev)) == 0)
				return 1;
		fwd = fwd->next;
	}
	return NONE;
}

int
check_dup_and_pref_list(record_row_s *list, record_row_s *fwd, dnsa_s *dnsa)
{
	preferred_a_s *prefer;
	while (list) {
		prefer = dnsa->prefer;
// Check if this is IP is in the duplicate list
		if (strncmp(list->dest, fwd->dest, RANGE_S) == 0) {
// Yes is is, so check if this has a preferred A record
			while (prefer) {
// Move preferred list to correct IP address for forward A record
				if (fwd->ip_addr != prefer->ip_addr) {
					prefer = prefer->next;
					continue;
				}
/* If this is the correct record, return none and we will keep it.
 * If this is not the correct record, return 1 and we will delete it from
 * the forward list
 */
				if (fwd->id != prefer->record_id) {
					return 1;
				} else {
					return NONE;
				}
			}
/* We did not find a preferred record for this duplicate IP address so add
 * it to the preferred list.
 */
			add_dup_to_prefer_list(dnsa, fwd);
		}
		list = list->next;
	}
// Not in the duplicate list, so return NONE and use this A record
	return NONE;
}

int
insert_into_rev_add_list(dnsa_s *dnsa, record_row_s *fwd, rev_record_row_s **rev)
{
	char rev_host[RANGE_S];
	rev_record_row_s *new, *list;
	zone_info_s *zones = dnsa->zones;

	new = ailsa_calloc(sizeof(rev_record_row_s), "new in insert_into_rev_add_list");
	list = *rev;
	init_rev_record_struct(new);
	new->rev_zone = dnsa->rev_zones->rev_zone_id;
	while (zones) {
		if (fwd->zone == zones->id)
			break;
		zones = zones->next;
	}
	if (!zones) { // fwd record belongs to non-existent zone
		free(new);
		return 0;
	}
	snprintf(new->dest, RBUFF_S, "%s.", fwd->host);
	if (get_rev_host(dnsa->rev_zones->prefix, rev_host, fwd->dest) != 0)
		return 0;
	else
		snprintf(new->host, 11, "%s", rev_host);
	printf("Adding record for %s -> %s\n", new->dest, new->host);
	if (!list) {
		*rev = new;
	} else {
		while (list->next)
			list = list->next;
		list->next = new;
	}
	return 1;
}

void
add_dup_to_prefer_list(dnsa_s *dnsa, record_row_s *fwd)
{
	preferred_a_s *new, *list;

	if (!(new = malloc(sizeof(preferred_a_s))))
		report_error(MALLOC_FAIL, "new in add_dup_to_prefer_list");
	init_preferred_a_struct(new);
	list = dnsa->prefer;
	snprintf(new->ip, RANGE_S, "%s", fwd->dest);
	snprintf(new->fqdn, RBUFF_S, "%s", fwd->host);
	new->record_id = fwd->id;
	new->ip_addr = fwd->ip_addr;
	if (!list) {
		dnsa->prefer = new;
	} else {
		while (list->next)
			list = list->next;
		list->next = new;
	}
	fprintf(stderr, "\
***\nDuplicate IP %s does not have a preferred A record\n", fwd->dest);
	fprintf(stderr, "\
Using %s for this PTR.\n\
If this is not what you want, please set up a preferred record for this PTR\n",
		fwd->host);
	fprintf(stderr, "***\n");
}

// Build list of the reverse records we need to delete from the database
int
rev_records_to_delete(dnsa_s *dnsa, rev_record_row_s **rev)
{
	int i = 0;
	size_t len;
	rev_record_row_s *list = dnsa->rev_records;
	record_row_s *fwd;
// Check for new preferred A records
	while (list) {
		fwd = dnsa->records;
		while (fwd) {
			if (list->ip_addr == fwd->ip_addr) {
				len = strlen(fwd->host);
				if (strncmp(fwd->host, list->dest, len) != 0) {
					insert_into_rev_del_list(list, rev);
					i++;
				}
			}
			fwd = fwd->next;
		}
		list = list->next;
	}
// Check for deleted A records
	list = dnsa->rev_records;
	while (list) {
		fwd = dnsa->records;
		while (fwd) {
			if (list->ip_addr == fwd->ip_addr)
				break;
			fwd = fwd->next;
		}
		if (!fwd) {
			insert_into_rev_del_list(list, rev);
			i++;
		}
		list = list->next;
	}
	return i;
}

void
insert_into_rev_del_list(rev_record_row_s *record, rev_record_row_s **rev)
{
	rev_record_row_s *new, *list;

	if (!(new = malloc(sizeof(rev_record_row_s))))
		report_error(MALLOC_FAIL, "new in insert_into_rev_add_list");
	list = *rev;
	init_rev_record_struct(new);
	new->record_id = record->record_id;
	printf("Deleting record id %lu; %s\n", new->record_id, record->dest);
	if (!list) {
		*rev = new;
	} else {
		while (list->next)
			list = list->next;
		list->next = new;
	}
}

int
get_rev_host(unsigned long int prefix, char *rev_host, char *host)
{
	char *tmp;
	int i;
	size_t len;

	*rev_host = '\0';
	if (prefix == 8) {
		for (i = 0; i < 3; i++) {
			tmp = strrchr(host, '.');
			tmp++;
			rev_host = strncat(rev_host, tmp, 4);
			rev_host = strncat(rev_host, ".", CH_S);
			tmp--;
			*tmp = '\0';
		}
		len = strlen(rev_host);
		rev_host[len - 1] = '\0';
	} else if (prefix == 16) {
		for (i = 0; i < 2; i++) {
			tmp = strrchr(host, '.');
			tmp++;
			rev_host = strncat(rev_host, tmp, 4);
			rev_host = strncat(rev_host, ".", CH_S);
			tmp--;
			*tmp = '\0';
		}
		len = strlen(rev_host);
		rev_host[len - 1] = '\0';
	} else if (prefix >= 24) {
		tmp = strrchr(host, '.');
		tmp++;
		strncpy(rev_host, tmp, 4);
	} else {
		printf("Prefix %lu invalid\n", prefix);
			return 1;
	}
	return 0;
}

void
add_int_ip_to_records(dnsa_s *dnsa)
{
	if (dnsa->records)
		add_int_ip_to_fwd_records(dnsa->records);
	if (dnsa->rev_records)
		add_int_ip_to_rev_records(dnsa);
}

void
add_int_ip_to_fwd_records(record_row_s *records)
{
	uint32_t ip;
	while (records) {
		if (inet_pton(AF_INET, records->dest, &ip))
			records->ip_addr = (unsigned long int) htonl(ip);
		else
			records->ip_addr = 0;
		records = records->next;
	}
}

int
add_int_ip_to_rev_records(dnsa_s *dnsa)
{
	char address[RANGE_S], host[RANGE_S], *ip_addr, *tmp;
	int i;
	uint32_t ip;
	unsigned long int prefix = dnsa->rev_zones->prefix;
	rev_record_row_s *rev = dnsa->rev_records;
	ip_addr = address;
	while (rev) {
		strncpy(ip_addr, dnsa->rev_zones->net_range, RANGE_S);
		strncpy(host, rev->host, RANGE_S);
		if (prefix == 8) {
			for (i = 0; i < 3; i++) {
				tmp = strrchr(ip_addr, '.');
				*tmp = '\0';
			}
			for (i = 0; i < 2; i++) {
				tmp = strrchr(host, '.');
				ip_addr = strncat(ip_addr, tmp, 4);
				*tmp = '\0';
			}
			ip_addr = strncat(ip_addr, ".", CH_S);
			strncat(ip_addr, host, 4);
		} else if (prefix == 16) {
			for (i = 0; i < 2; i++) {
				tmp = strrchr(ip_addr, '.');
				*tmp = '\0';
			}
			tmp = strrchr(host, '.');
			ip_addr = strncat(ip_addr, tmp, 4);
			*tmp = '\0';
			ip_addr = strncat(ip_addr, ".", CH_S);
			ip_addr = strncat(ip_addr, host, 4);
		} else if (prefix >= 24) {
			tmp = strrchr(ip_addr, '.');
			*tmp = '\0';
			ip_addr = strncat(ip_addr, ".", CH_S);
			ip_addr = strncat(ip_addr, host, 4);
		} else {
			return 1;
		}
		if (inet_pton(AF_INET, ip_addr, &ip)) {
			rev->ip_addr = (unsigned long int) htonl(ip);
		} else {
			rev->ip_addr = 0;
			return 1;
		}
		rev = rev->next;
	}
	return NONE;
}

unsigned long int
get_zone_serial(void)
{
	time_t now;
	struct tm *lctime;
	char sday[COMM_S], smonth[COMM_S], syear[COMM_S], sserial[RANGE_S];
	unsigned long int serial;

	now = time(0);
	if (!(lctime = localtime(&now)))
		report_error(GET_TIME_FAILED, strerror(errno));
	snprintf(syear, COMM_S, "%d", lctime->tm_year + 1900);
	if (lctime->tm_mon < 9)
		snprintf(smonth, COMM_S, "0%d", lctime->tm_mon + 1);
	else
		snprintf(smonth, COMM_S, "%d", lctime->tm_mon + 1);
	if (lctime->tm_mday < 10)
		snprintf(sday, COMM_S, "0%d", lctime->tm_mday);
	else
		snprintf(sday, COMM_S, "%d", lctime->tm_mday);
	snprintf(sserial, RANGE_S, "%s%s%s01",
		 syear,
		 smonth,
		 sday);
	serial = strtoul(sserial, NULL, 10);
	return serial;
}

static int
dnsa_populate_rev_zone(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl, AILLIST *list)
{
	if (!(cbc) || !(dcl) || !(list))
		return AILSA_NO_DATA;
	char buff[CONFIG_LEN];
	int retval;
	uint32_t ip_addr;
	unsigned long int range;

	if ((retval = cmdb_add_string_to_list(dcl->domain, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add net_range to list");
		goto cleanup;
	}
	memset(buff, 0, CONFIG_LEN);
	snprintf(buff, CONFIG_LEN, "%lu", dcl->prefix);
	if ((retval = cmdb_add_string_to_list(buff, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add prefix to list");
		goto cleanup;
	}
	memset(buff, 0, CONFIG_LEN);
	if ((retval = cmdb_add_string_to_list(dcl->domain, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add net_start to list");
		goto cleanup;
	}
	inet_pton(AF_INET, dcl->domain, &ip_addr);
	ip_addr = htonl(ip_addr);
	if ((retval = cmdb_add_number_to_list(ip_addr, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add start_ip to list");
		goto cleanup;
	}
	range = get_net_range(dcl->prefix);
	ip_addr += (uint32_t)range - 1;
	inet_ntop(AF_INET, &ip_addr, buff, RANGE_S);
	if ((retval = cmdb_add_string_to_list(buff, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add net_finish to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(ip_addr, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add finish_ip to list");
		goto cleanup;
	}
	if (!(dcl->ztype)) {
		if ((retval = cmdb_add_string_to_list(cbc->prins, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add primary nameserver to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(cbc->secns, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add secondary nameserver to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("NULL", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add NULL server to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(cbc->prins, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add primary nameserver to list");
			goto cleanup;
		}
	}
	if ((retval = cmdb_add_number_to_list(generate_zone_serial(), list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone serial number to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(cbc->refresh, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone refresh to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(cbc->retry, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone retry to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(cbc->expire, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone expire to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(cbc->ttl, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone ttl to list");
		goto cleanup;
	}
	if (!(dcl->ztype)) {
		if ((retval = cmdb_add_string_to_list("master", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone type to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list("NULL", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add NULL master to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list(dcl->ztype, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone type to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(dcl->master, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add master to list");
			goto cleanup;
		}
	}
	if ((retval = cmdb_populate_cuser_muser(list)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
	cleanup:
		return retval;
}

void
fill_rev_zone_info(rev_zone_info_s *zone, dnsa_comm_line_s *cm, ailsa_cmdb_s *dc)
{
	char address[RANGE_S], *addr;
	uint32_t ip_addr;
	unsigned long int range;

	addr = &(address[0]);
	snprintf(zone->pri_dns, RBUFF_S, "%s", dc->prins);
	snprintf(zone->sec_dns, RBUFF_S, "%s", dc->secns);
	snprintf(zone->net_range, RANGE_S, "%s", cm->domain);
	snprintf(zone->net_start, RANGE_S, "%s", cm->domain);
	zone->prefix = cm->prefix;
	zone->serial = get_zone_serial();
	zone->refresh = dc->refresh;
	zone->retry = dc->retry;
	zone->expire = dc->expire;
	zone->ttl = dc->ttl;
	inet_pton(AF_INET, zone->net_range, &ip_addr);
	ip_addr = htonl(ip_addr);
	zone->start_ip = ip_addr;
	range = get_net_range(cm->prefix);
	zone->end_ip = ip_addr + range - 1;
	ip_addr = htonl((uint32_t)zone->end_ip);
	inet_ntop(AF_INET, &ip_addr, addr, RANGE_S);
	snprintf(zone->net_finish, RANGE_S, "%s", addr);
	snprintf(zone->hostmaster, RBUFF_S, "%s", dc->hostmaster);
	snprintf(zone->master, RBUFF_S, "%s", cm->master);
	zone->cuser = zone->muser = (unsigned long int)getuid();
	if (cm->ztype)
		snprintf(zone->type, RANGE_S, "%s", cm->ztype);
	else
		snprintf(zone->type, COMM_S, "master");
}

void
select_specific_ip(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	record_row_s *records, *next, *ip;

	ip = NULL;
	records = dnsa->records;
	next = records->next;
	while (records) {
		if (strncmp(records->dest, cm->dest, RANGE_S) == 0) {
			ip = records;
			records = records->next;
			if (records)
				next = records->next;
		} else {
			free(records);
			records = next;
			if (records)
				next = records->next;
		}
	}
	dnsa->records = ip;
	if (ip)
		ip->next = NULL;
}

/*
 * In this function we need 4 counters; prev, next, tmp and list. This is so we
 * can remove entries from the linked list. list tracks the head node, and tmp
 * is only used to check if the head node is deleted so we can set list
 * accordingly. Once we have a head node, prev tracks the previous entry so
 * when we delete a member, the list can be updated (i.e. prev->next is set to
 * the member after the one deleted). Next tracks the next member so if we
 * free the current node we know where the next one is. Finally if the head
 * node is changed we set *records to list, the new head memeber
 */
int
get_a_records_for_range(record_row_s **records, rev_zone_info_s *zone)
{
	record_row_s *rec, *list, *tmp, *prev, *next;
	int i = 0;
	uint32_t ip_addr;
	unsigned long int ip;
	list = *records;
	rec = prev = list;
	if (rec)
		next = rec->next;
	else
		return NONE;
	while (rec) {
		tmp = rec;
		inet_pton(AF_INET, rec->dest, &ip_addr);
		ip = (unsigned long int) htonl(ip_addr);
		if (ip < zone->start_ip || ip > zone->end_ip) {
			free (rec);
			rec = next;
			if (rec)
				next = rec->next;
			if (tmp == list)
				list = prev = rec;
			else
				prev->next = rec;
		} else {
			i++;
			if (prev != rec)
				prev = prev->next;
			rec = next;
			if (rec)
				next = rec->next;
		}
	}
	if (*records != list)
		*records = list;
	return i;
}

int
get_pref_a_for_range(preferred_a_s **prefer, rev_zone_info_s *rev)
{
	preferred_a_s *pref, *list, *tmp, *prev, *next;
	int i = 0;
	list = *prefer;
	pref = prev = list;
	if (pref)
		next = pref->next;
	else
		return NONE;
	while (pref) {
		tmp = pref;
		if (pref->ip_addr < rev->start_ip || pref->ip_addr > rev->end_ip) {
			free(pref);
			pref = next;
			if (pref)
				next = pref->next;
			if (tmp == list)
				list = prev = pref;
			else
				prev->next = pref;
		} else {
			i++;
			if (prev != pref)
				prev = prev->next;
			pref = next;
			if (pref)
				next = pref->next;
		}
	}
	if (*prefer != list)
		*prefer = list;
	return i;
}

int
add_glue_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	int retval = NONE;
	dbdata_s data, user;
	dnsa_s *dnsa;
	glue_zone_info_s *glue;
	zone_info_s *zone;

	if (!(glue = malloc(sizeof(glue_zone_info_s))))
		report_error(MALLOC_FAIL, "glue in add_glue_zone");
	zone = ailsa_calloc(sizeof(zone_info_s), "zone in add_glue_zones");
	dnsa = ailsa_calloc(sizeof(dnsa_s), "dnsa in add_glue_zone");
	init_dbdata_struct(&data);
	init_dbdata_struct(&user);
	setup_glue_struct(dnsa, zone, glue);
	if (strchr(cm->glue_ns, ','))
		split_glue_ns(cm->glue_ns, glue);
	else
		snprintf(glue->pri_ns, RBUFF_S, "%s", cm->glue_ns);
	if (strchr(cm->glue_ip, ',')) {
		split_glue_ip(cm->glue_ip, glue);
		if (strncmp(glue->sec_ns, "none", COMM_S) == 0) {
			printf("Removing 2nd IP as no 2nd name provided.\n");
			snprintf(glue->sec_dns, COMM_S, "none");
		}
	} else {
		snprintf(glue->pri_dns, RANGE_S, "%s", cm->glue_ip);
		if (strncmp(glue->sec_ns, "none", COMM_S) != 0) {
			printf("Removing 2nd name as no 2nd IP provided.\n");
			snprintf(glue->sec_ns, RANGE_S, "none");
		}
	}
	snprintf(glue->name, RBUFF_S, "%s", cm->domain);
	if ((retval = get_glue_zone_parent(dc, dnsa)) != 0) {
		dnsa_clean_list(dnsa);
		printf("Zone %s has no parent!\n", cm->domain);
		return retval;
	}
	glue->cuser = glue->muser = (unsigned long int)getuid();
	check_glue_zone_input(glue);
	if ((retval = dnsa_run_insert(dc, dnsa, GLUES)) != 0) {
		dnsa_clean_list(dnsa);
		fprintf(stderr, "Cannot insert glue zone %s into database\n",
			cm->domain);
		return retval;
	}
	data.args.number = glue->zone_id;
	user.args.number = (unsigned long int)getuid();
	user.next = &data;
	if ((retval = dnsa_run_update(dc, &user, ZONE_UPDATED_YES)) != 0)
		fprintf(stderr, "Cannot set zone as update\n");
	printf("Glue records for zone %s added\n", cm->domain);
	dnsa_clean_list(dnsa);
	return retval;
}

int
delete_glue_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	int retval = NONE, c = NONE;
	unsigned long int glue_id = 0;
	dnsa_s *dnsa;
	dbdata_s data, user;
	glue_zone_info_s *glue;

	dnsa = ailsa_calloc(sizeof(dnsa_s), "dnsa in delete_glue_sone");
	init_dbdata_struct(&data);
	if ((retval = dnsa_run_query(dc, dnsa, GLUE)) != 0) {
		dnsa_clean_list(dnsa);
		fprintf(stderr, "Cannot get list of glue zones\n");
		return NO_GLUE_ZONE;
	}
	glue = dnsa->glue;
	while (glue) {
		if (strncmp(cm->domain, glue->name, CONF_S) != 0) {
			glue = glue->next;
		} else {
			c++;
			snprintf(data.args.text, CONF_S, "%s", glue->name);
			glue_id = glue->id;
			glue = glue->next;
		}
	}
	if (c == 0) {
		fprintf(stderr, "No glue zone %s found\n", cm->domain);
		return NO_GLUE_ZONE;
	} else if (c > 1) {
		fprintf(stderr, "Multiple glue zones for %s found\n", cm->domain);
	}
	if ((retval = dnsa_run_delete(dc, &data, GLUES)) == 0) {
		fprintf(stderr, "Unable to delete glue zone %s\n", cm->domain);
	} else {
		if (retval == 1)
			printf("Glue zone %s deleted\n", cm->domain);
		else if (retval > 1)
			fprintf(stderr, "Multiple glue zones deleted for %s\n", cm->domain);
		data.args.number = glue_id;
		user.args.number = (unsigned long int)getuid();
		user.next = &data;
		if ((retval = dnsa_run_update(dc, &user, ZONE_UPDATED_YES)) != 0)
			fprintf(stderr, "Cannot set zone as update\n");
	}
	retval = 0;
	dnsa_clean_list(dnsa);
	return retval;
}

void
list_glue_zones(ailsa_cmdb_s *dc)
{
	if (!(dc))
		return;
	int retval;
	size_t total = 6;
	AILLIST *g = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = ailsa_basic_query(dc, GLUE_ZONE_INFORMATION, g)) != 0) {
		ailsa_syslog(LOG_ERR, "GLUE_ZONE_INFORMATION query failed");
		goto cleanup;
	}
	if (g->total == 0) {
		printf("There are no glue zones to list\n");
		goto cleanup;
	} else if ((g->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong list element number. Factor is %zu, got %zu", total, g->total);
		goto cleanup;
	} else {
		printf("Glue zones\n\n");
	}
	e = g->head;
	while (e) {
		printf("%s\t", ((ailsa_data_s *)e->data)->data->text);
		printf("%s\t", ((ailsa_data_s *)e->next->data)->data->text);
		printf("%s,", ((ailsa_data_s *)e->next->next->data)->data->text);
		printf("%s\t", ((ailsa_data_s *)e->next->next->next->data)->data->text);
		printf("%s,", ((ailsa_data_s *)e->next->next->next->data)->data->text);
		printf("%s\n", ((ailsa_data_s *)e->next->next->next->next->next->data)->data->text);
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		ailsa_list_full_clean(g);
}

void
split_glue_ns(char *ns, glue_zone_info_s *glue)
{
	char *pnt;
	if (!(pnt = strchr(ns, ',')))
		return;
	*pnt = '\0';
	pnt++;
	snprintf(glue->pri_ns, RBUFF_S, "%s", ns);
	snprintf(glue->sec_ns, RBUFF_S, "%s", pnt);
}

void
split_glue_ip(char *ip, glue_zone_info_s *glue)
{
	char *pnt;
	if (!(pnt = strchr(ip, ',')))
		return;
	*pnt = '\0';
	pnt++;
	snprintf(glue->pri_dns, RANGE_S, "%s", ip);
	snprintf(glue->sec_dns, RANGE_S, "%s", pnt);
}

void
setup_glue_struct(dnsa_s *dnsa, zone_info_s *zone, glue_zone_info_s *glue)
{
	if (glue)
		init_glue_zone_struct(glue);
	else
		report_error(NO_GLUE_ZONE, "setup_glue_struct");
	if (zone)
		init_zone_struct(zone);
	if (dnsa) {
		dnsa->glue = glue;
		dnsa->zones = zone;
	}
}

int
get_glue_zone_parent(ailsa_cmdb_s *dc, dnsa_s *dnsa)
{
	char *parent;
	int retval = NONE;
	zone_info_s *zone = dnsa->zones;

	if ((retval = dnsa_run_query(dc, dnsa, ZONE)) != 0) {
		fprintf(stderr, "Unable to get zones from database\n");
		return retval;
	}
	parent = strchr(dnsa->glue->name, '.');
	parent++;
	while (zone) {
		if ((strncmp(zone->name, parent, RBUFF_S) == 0) &&
		    (strncmp(zone->type, "master", COMM_S) == 0)) {
			dnsa->glue->zone_id = zone->id;
			break;
		}
		zone = zone->next;
	}
	if (!(zone))
		retval = NO_PARENT_ZONE;
	return retval;
}

void
check_glue_zone_input(glue_zone_info_s *glue)
{
	if (!(glue))
		return;
	char *gzone = glue->name;
	const char *pri = glue->pri_ns;
	const char *sec = glue->sec_ns;
	if (strstr(pri, gzone))
		ailsa_add_trailing_dot(glue->pri_ns);
	if (strstr(sec, gzone))
		ailsa_add_trailing_dot(glue->sec_ns);
}

void
print_glue_zone(glue_zone_info_s *glue, zone_info_s *zone)
{
	char *pri, *sec;
	zone_info_s *list = NULL;
	if (zone)
		list = zone;
	else {
		fprintf(stderr, "No zone info passed to print_glue_zone??\n");
		exit(NO_ZONE_LIST);
	}
	while (list) {
		if (list->id == glue->zone_id) {
			pri = get_zone_fqdn_name(list, glue, 0);
			sec = get_zone_fqdn_name(list, glue, 1);
			printf("%s\t%s\t%s,%s\t%s,%s\n",
list->name, glue->name, glue->pri_dns, glue->sec_dns, pri, sec);

			free(sec);
			free(pri);
		}
		list = list->next;
	}
}

char *
get_zone_fqdn_name(zone_info_s *zone, glue_zone_info_s *glue, int ns)
{
	char *fqdn;
	size_t len;
	
	if (!(fqdn = calloc(URL_S, sizeof(char))))
		report_error(MALLOC_FAIL, "fqdn in get_zone_fqdn_name");
	if (ns == 0) {
		len = strlen(glue->pri_ns) - 1;
		if (*(glue->pri_ns + len) == '.') {
			*(glue->pri_ns + len) = '\0';
			snprintf(fqdn, URL_S, "%s", glue->pri_ns);
		} else {
			snprintf(fqdn, URL_S, "%s.%s", glue->pri_ns, zone->name);
		}
	} else if (ns == 1) {
		len = strlen(glue->sec_ns) - 1;
		if (*(glue->sec_ns + len) == '.') {
			*(glue->sec_ns + len) = '\0';
			snprintf(fqdn, URL_S, "%s", glue->sec_ns);
		} else if (strncmp(glue->sec_ns, "none", COMM_S) == 0) {
			snprintf(fqdn, URL_S, "%s", glue->sec_ns);
		} else {
			snprintf(fqdn, URL_S, "%s.%s", glue->sec_ns, zone->name);
		}
	} else {
		report_error(NOT_PRI_OR_SEC_NS, "get_zone_fqdn_name");
	}
	return fqdn;
}

