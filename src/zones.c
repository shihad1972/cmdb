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
multi_a_range(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl);

static int
setup_net_range(ailsa_cmdb_s *dc, char *domain, char *range);

static int
get_search_net_range(char *range, char *domain, unsigned long int prefix);

static int
multi_a_ip_address(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl);

static int
cmdb_trim_record_list(AILLIST *rev, AILLIST *pref);

static int
cmdb_records_to_remove(char *range, AILLIST *rec, AILLIST *rev, AILLIST *remove);

static int
cmdb_remove_reverse_records(ailsa_cmdb_s *dc, char *range, AILLIST *remove);

static int
cmdb_records_to_add(char *range, AILLIST *rec, AILLIST *rev, AILLIST *add);

static int
cmdb_add_reverse_records(ailsa_cmdb_s *dc, char *range, AILLIST *add);

static int
dnsa_populate_rev_zone(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl, AILLIST *list);

static int
dnsa_populate_record(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl, AILLIST *list);

static int
check_for_zones_and_hosts(ailsa_cmdb_s *dc, char *domain, char *top, char *host);

static int
dnsa_split_glue_ns(char *ns, AILLIST *list);

static int
dnsa_get_ip_for_glue_ns(char *ip, AILLIST *list);

static int
dnsa_split_glue_ip(char *ip, AILLIST *list);

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
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval;

	if (cm->domain)
		retval = multi_a_range(dc, cm);
	else if (cm->dest)
		retval = multi_a_ip_address(dc, cm);
	return retval;
}

static int
multi_a_range(ailsa_cmdb_s *dc, dnsa_comm_line_s *dcl)
{
	if (!(dc) || !(dcl))
		return AILSA_NO_DATA;
	char *range = ailsa_calloc(MAC_LEN, "range in multi_a_range");
	int retval;
	size_t total = 2;
	AILLIST *net = ailsa_db_data_list_init();
	AILLIST *pre = ailsa_db_data_list_init();
	AILLIST *mod = ailsa_db_data_list_init();
	AILLIST *ip = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d;

	if ((retval = setup_net_range(dc, dcl->domain, range)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get search net range");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(range, net)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add range to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, DUP_IP_NET_RANGE, net, ip)) != 0) {
		ailsa_syslog(LOG_ERR, "DUP_IP_NET_RANGE query failed");
		goto cleanup;
	}
	if (ip->total == 0) {
		ailsa_syslog(LOG_INFO, "No duplicate IP's in range %s", dcl->domain);
		goto cleanup;
	}
	if ((ip->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number in list. Expected factor of %zu, got %zu", total, ip->total);
		goto cleanup;
	}
	e = ip->head;
	printf("Destination\t#\thost\n");
	while (e) {
		d = e->data;
		ailsa_list_clean(pre);
		if ((retval = cmdb_add_string_to_list(d->data->text, pre)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add IP to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(dc, A_RECORDS_WITH_IP, pre, mod)) != 0) {
			ailsa_syslog(LOG_ERR, "A_RECORDS_WITH_IP query failed");
			goto cleanup;
		}
		if (strlen(d->data->text) < 8)
			printf("%s\t\t", d->data->text);
		else
			printf("%s\t", d->data->text);
		d = e->next->data;
		printf("%lu\t", d->data->number);
		d = mod->head->data;
		printf("%s.", d->data->text);
		d = mod->head->next->data;
		printf("%s\n", d->data->text);
		e = ailsa_move_down_list(e, total);
		ailsa_list_clean(mod);
	}
	cleanup:
		ailsa_list_full_clean(net);
		ailsa_list_full_clean(pre);
		ailsa_list_full_clean(mod);
		ailsa_list_full_clean(ip);
		my_free(range);
		return retval;
}

static int
setup_net_range(ailsa_cmdb_s *dc, char *domain, char *range)
{
	if (!(dc) || !(domain) || !(range))
		return AILSA_NO_DATA;
	int retval;
	unsigned long int prefix;
	AILLIST *net = ailsa_db_data_list_init();
	AILLIST *pre = ailsa_db_data_list_init();
	ailsa_data_s *d;

	if ((retval = cmdb_add_string_to_list(domain, net)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add net range domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, PREFIX_ON_NET_RANGE, net, pre)) != 0) {
		ailsa_syslog(LOG_ERR, "PREFIX_ON_NET_RANGE query failed");
		goto cleanup;
	}
	if (pre->total == 0) {
		ailsa_syslog(LOG_INFO, "net range %s not found in reverse zones", domain);
		goto cleanup;
	}
	d = pre->head->data;
	prefix = strtoul(d->data->text, NULL, 10);
	if ((retval = get_search_net_range(range, domain, prefix)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot get search range from %s/%lu", domain, prefix);

	cleanup:
		ailsa_list_full_clean(net);
		ailsa_list_full_clean(pre);
		return retval;
}

static int
get_search_net_range(char *range, char *domain, unsigned long int prefix)
{
	if (!(range) || !(domain) || (prefix == 0))
		return AILSA_NO_DATA;
	char *tmp;
	int retval = 0;

	if (snprintf(range, MAC_LEN, "%s", domain) >= MAC_LEN)
		ailsa_syslog(LOG_ERR, "range truncated in get_search_net_range");
	tmp = strrchr(range, '.');
	if (!(tmp))
		return AILSA_RANGE_ERROR;
	if (prefix < 24) {
		*tmp = '\0';
		tmp = strrchr(range, '.');
		if (!(tmp))
			return AILSA_RANGE_ERROR;
		if (prefix < 16) {
			*tmp = '\0';
			tmp = strrchr(range, '.');
			if (!(tmp))
				return AILSA_RANGE_ERROR;
		}
	}
	tmp++;
	sprintf(tmp, "%%");
	return retval;
}

static int
multi_a_ip_address(ailsa_cmdb_s *dc, dnsa_comm_line_s *dcl)
{
	if (!(dc) || !(dcl))
		return AILSA_NO_DATA;
	char *hostname = ailsa_calloc(DOMAIN_LEN, "fqdn in multi_a_ip_address");
	int retval;
	size_t total = 2;
	AILLIST *fqdn = ailsa_db_data_list_init();
	AILLIST *ip = ailsa_db_data_list_init();
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d, *h;

	if ((retval = cmdb_add_string_to_list(dcl->dest, ip)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add IP address to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, A_RECORDS_WITH_IP, ip, l)) != 0) {
		ailsa_syslog(LOG_ERR, "DUP_IP_A_RECORD query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "IP address %s has no records", dcl->dest);
		goto cleanup;
	}
	if ((l->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number in list. Expected factor of %zu, got %zu", total, l->total);
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, FQDN_PREF_A_ON_IP, ip, fqdn)) != 0) {
		ailsa_syslog(LOG_ERR, "FQDN_PREF_A_ON_IP query failed");
		goto cleanup;
	}
	e = l->head;
	printf("Displaying A records with IP address %s. * indicates preferred A record\n\n", dcl->dest);
	while (e) {
		h = e->data;
		d = e->next->data;
		snprintf(hostname, DOMAIN_LEN, "%s.%s", h->data->text, d->data->text);
		if (strncmp(hostname, ((ailsa_data_s *)fqdn->head->data)->data->text, DOMAIN_LEN) == 0)
			printf("   *\t%s\n", hostname);
		else
			printf("\t%s\n", hostname);
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		my_free(hostname);
		ailsa_list_full_clean(fqdn);
		ailsa_list_full_clean(ip);
		ailsa_list_full_clean(l);
		return retval;
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
	char *domain = ailsa_calloc(DOMAIN_LEN, "domain in check_for_zones_and_hosts");
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
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	char *range = ailsa_calloc(MAC_LEN, "range in build_reverse_zone");
	char *tmp;
	int retval;
	size_t len;
	AILLIST *add = ailsa_db_data_list_init();
	AILLIST *net = ailsa_db_data_list_init();
	AILLIST *rec = ailsa_db_data_list_init();
	AILLIST *rem = ailsa_db_data_list_init();
	AILLIST *pref = ailsa_db_data_list_init();
	AILLIST *rev = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cm->domain, net)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add network range to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, REV_RECORDS_ON_NET_RANGE, net, rev)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_RECORDS_ON_NET_RANGE query failed");
		goto cleanup;
	}
	if ((retval = setup_net_range(dc, cm->domain, range)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get search range");
		goto cleanup;
	}
	ailsa_list_clean(net);
	if ((retval = cmdb_add_string_to_list(range, net)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add search net range to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, RECORDS_ON_NET_RANGE, net, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "DUP_IP_NET_RANGE query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, PREFER_A_INFO_ON_RANGE, net, pref)) != 0) {
		ailsa_syslog(LOG_ERR, "PREFER_A_INFO_ON_RANGE query failed");
		goto cleanup;
	}
	if ((retval = cmdb_trim_record_list(rec, pref)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot trim records list");
		goto cleanup;
	}
	len = strlen(range);
	tmp = range + len - 1;
	*tmp = '\0';
	if ((retval = cmdb_records_to_remove(range, rec, rev, rem)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create list of records to remove");
		goto cleanup;
	}
	*tmp = '\0';
	if ((retval = cmdb_records_to_add(range, rec, rev, add)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create list of records to add");
		goto cleanup;
	}
	if ((retval = cmdb_remove_reverse_records(dc, cm->domain, rem)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot remove stale rev records");
		goto cleanup;
	}
	if ((retval = cmdb_add_reverse_records(dc, cm->domain, add)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add new reverse records");
		goto cleanup;
	}
	if (rem->total > 0 || add->total > 0) {
		ailsa_list_clean(net);
		if ((retval = cmdb_add_number_to_list((unsigned long int)getuid(), net)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add muser to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(cm->domain, net)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add net range to update list");
			goto cleanup;
		}
		if ((retval = ailsa_update_query(dc, update_queries[SET_REV_ZONE_UPDATED], net)) != 0) {
			ailsa_syslog(LOG_ERR, "Update query SET_REV_ZONE_UPDATED failed");
			goto cleanup;
		}
		if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, cm->domain)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate zone %s", cm->domain);
			goto cleanup;
		}
	}
	cleanup:
		ailsa_list_full_clean(add);
		ailsa_list_full_clean(net);
		ailsa_list_full_clean(rec);
		ailsa_list_full_clean(rem);
		ailsa_list_full_clean(pref);
		ailsa_list_full_clean(rev);
		my_free(range);
		return retval;
}

static int
cmdb_trim_record_list(AILLIST *r, AILLIST *p)
{
	if (!(r) || !(p))
		return AILSA_NO_DATA;
	int retval = 0;
	size_t rec_tot = 4;
	size_t pref_tot = 3;
	AILELEM *rec, *pref, *tmp;
	ailsa_data_s *pd, *rev;
	if ((r->total == 0) || (p->total == 0))
		return AILSA_NO_DATA;
	if (((r->total % rec_tot) != 0) || ((p->total % pref_tot) != 0))
		return WRONG_LENGTH_LIST;
	if (!(r->destroy))
		return AILSA_LIST_NO_DESTROY;
	pref = p->head;
	while (pref) {
		rec = r->head;
		while (rec) {
			pd = pref->data;
			rev = rec->data;
			if (strncmp(pd->data->text, rev->data->text, DOMAIN_LEN) == 0) {
				pd = pref->next->next->data;
				rev = rec->next->data;
				if (pd->data->number != rev->data->number) {
					tmp = rec->prev;
					if ((retval = ailsa_list_remove(r, tmp->next, (void **)&pd)) == 0)
						r->destroy(pd);
					else
						return AILSA_LIST_CANNOT_REMOVE;
					if ((retval = ailsa_list_remove(r, tmp->next, (void **)&pd)) == 0)
						r->destroy(pd);
					else
						return AILSA_LIST_CANNOT_REMOVE;
					if ((retval = ailsa_list_remove(r, tmp->next, (void **)&pd)) == 0)
						r->destroy(pd);
					else
						return AILSA_LIST_CANNOT_REMOVE;
					if ((retval = ailsa_list_remove(r, tmp->next, (void **)&pd)) == 0)
						r->destroy(pd);
					else
						return AILSA_LIST_CANNOT_REMOVE;
					rec = tmp->next;
					continue;
				}
			}
			rec = ailsa_move_down_list(rec, rec_tot);
		}
		pref = ailsa_move_down_list(pref, pref_tot);
	}
	return retval;
}

static int
cmdb_records_to_remove(char *range, AILLIST *rec, AILLIST *rev, AILLIST *remove)
{
	if (!(range) || !(rec) || !(rev) || !(remove))
		return AILSA_NO_DATA;
	int retval = 0;
	size_t rec_tot = 4;
	size_t rev_tot = 2;
	size_t len = strlen(range);
	size_t gap = MAC_LEN - len;
	char *tmp = range + len;
	AILELEM *ce, *ve;
	ailsa_data_s *c, *v;
	if ((rec->total == 0) || (rev->total == 0))
		return AILSA_NO_DATA;
	if (((rec->total % rec_tot) != 0) || ((rev->total % rev_tot) != 0))
		return WRONG_LENGTH_LIST;
	ve = rev->head;
	while (ve) {
		ce = rec->head;
		while (ce) {
			tmp = range + len;
			memset(tmp, 0, gap);
			c = ce->data;
			v = ve->data;
			snprintf(tmp, gap, "%s", v->data->text);
			if (strncmp(range, c->data->text, MAC_LEN) == 0)
				break;
			ce = ailsa_move_down_list(ce, rec_tot);
		}
		if (!(ce)) {
			if ((retval = cmdb_add_string_to_list(v->data->text, remove)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot add record to delete list");
				return retval;
			}
		}
		ve = ailsa_move_down_list(ve, rev_tot);
	}
	return retval;
}

static int
cmdb_remove_reverse_records(ailsa_cmdb_s *dc, char *range, AILLIST *rem)
{
	if (!(dc) || !(range) || !(rem))
		return AILSA_NO_DATA;
	char *host;
	int retval;
	size_t total = 1;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d;

	if (rem->total == 0)
		goto cleanup;

	if ((retval = cmdb_add_zone_id_to_list(range, REVERSE_ZONE, dc, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get id for reverse zone");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_ERR, "Cannot find reverse zone %s\n", range);
		goto cleanup;
	}
	e = rem->head;
	while (e) {
		d = e->data;
		host = d->data->text;
		if ((retval = cmdb_add_string_to_list(host, l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add host string to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(dc, REV_RECORD_ID_ON_ZONE_HOST, l, r)) != 0) {
			ailsa_syslog(LOG_ERR, "REV_RECORD_ID_ON_ZONE_HOST query failed");
			goto cleanup;
		}
		if ((retval = ailsa_list_remove(l, l->tail, (void **)&d)) == 0)
			l->destroy(d);
		else
			goto cleanup;
		e = ailsa_move_down_list(e, total);
	}
	if ((retval = ailsa_multiple_query(dc, delete_queries[DELETE_REVERSE_RECORD], r)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_REVERSE_RECORD multi query failed");
		goto cleanup;
	}
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		return retval;
}

static int
cmdb_records_to_add(char *range, AILLIST *rec, AILLIST *rev, AILLIST *add)
{
	if (!(range) || !(rec) || !(rev) || !(add))
		return AILSA_NO_DATA;
	int retval = 0;
	size_t rec_tot = 4;
	size_t rev_tot = 2;
	size_t len = strlen(range);
	size_t gap = MAC_LEN - len;
	char *tmp = range + len;
	AILELEM *ce, *ve;
	ailsa_data_s *c, *v, *reg;
	if ((rev->total == 0) || (rev->total == 0))
		return AILSA_NO_DATA;
	if (((rec->total % rec_tot) != 0) || ((rev->total % rev_tot) != 0))
		return WRONG_LENGTH_LIST;
	ce = rec->head;
	while (ce) {
		ve = rev->head;
		while (ve) {
			tmp = range + len;
			memset(tmp, 0, gap);
			c = ce->data;
			v = ve->data;
			snprintf(tmp, gap, "%s", v->data->text);
			if (strncmp(range, c->data->text, MAC_LEN) == 0)
				break;
			ve = ailsa_move_down_list(ve, rev_tot);
		}
		if (!(ve)) {
			reg = ce->next->data;
			if ((retval = cmdb_add_number_to_list(reg->data->number, add)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot add PTR destination to list");
				return retval;
			}
		}
		ce = ailsa_move_down_list(ce, rec_tot);
	}
	return retval;
}

static int
cmdb_add_reverse_records(ailsa_cmdb_s *dc, char *range, AILLIST *add)
{
	if (!(dc) || !(range) || !(add))
		return AILSA_NO_DATA;
	int retval;
	char *host;
	char *domain = ailsa_calloc(DOMAIN_LEN, "domain in cmdb_add_reverse_records");
	size_t total = 1;
	unsigned long int record_id;
	AILLIST *a = ailsa_db_data_list_init();
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *rec = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d, *h;

	if (add->total == 0)
		goto cleanup;
	e = add->head;
	while (e) {
		d = e->data;
		record_id = d->data->number;
		if ((retval = cmdb_add_number_to_list(record_id, rec)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add record_id to list");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(dc, DESTINATION_ON_RECORD_ID, rec, l)) != 0) {
			ailsa_syslog(LOG_ERR, "DESTINATION_ON_RECORD_ID query failed");
			goto cleanup;
		}
		if (l->total > 0)
			d = l->head->data;
		else
			goto jump;
		host = strrchr(d->data->text, '.');
		if (host)
			host++;
		else
			goto jump;
		if ((retval = cmdb_add_zone_id_to_list(range, REVERSE_ZONE, dc, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add rev_zone_id to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(host, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add host to list");
			goto cleanup;
		}
		d = l->head->next->data;
		h = l->head->next->next->data;
		snprintf(domain, DOMAIN_LEN, "%s.%s.", d->data->text, h->data->text);
		if ((retval = cmdb_add_string_to_list(domain, a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add FQDN to list");
			goto cleanup;
		}
		if ((retval = cmdb_populate_cuser_muser(a)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
			goto cleanup;
		}
		jump:
			ailsa_list_clean(l);
			ailsa_list_clean(rec);
			e = ailsa_move_down_list(e, total);
			memset(domain, 0, DOMAIN_LEN);
	}
	if ((retval = ailsa_multiple_query(dc, insert_queries[INSERT_REVERSE_RECORD], a)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_REVERSE_RECORD multi query failed");
		goto cleanup;
	}
	cleanup:
		my_free(domain);
		ailsa_list_full_clean(a);
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(rec);
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

int
add_glue_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		goto cleanup;
	char *parent;
	int retval;
	AILLIST *p = ailsa_db_data_list_init();
	AILLIST *u = ailsa_db_data_list_init();
	AILLIST *z = ailsa_db_data_list_init();

	parent = strchr(cm->domain, '.');
	if (parent) {
		parent++;
	} else {
		retval = AILSA_CANNOT_GET_PARENT;
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(parent, p)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add parent domain to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_zone_id_to_list(parent, FORWARD_ZONE, dc, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add parent zone id to list");
		goto cleanup;
	}
	if (z->total == 0) {
		ailsa_syslog(LOG_INFO, "Parent zones %s does not exist", parent);
		retval = AILSA_NO_PARENT;
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cm->domain, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain to list");
		goto cleanup;
	}
	if ((retval = dnsa_split_glue_ns(cm->glue_ns, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot split nameservers");
		goto cleanup;
	}
	if ((retval = dnsa_get_ip_for_glue_ns(cm->glue_ip, z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get IP addresses for name servers");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(z)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(dc, INSERT_GLUE_ZONE, z)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_GLUE_ZONE query failed");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list((unsigned long int)getuid(), u)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert muser into update list");
		goto cleanup;
	}
	if ((retval = cmdb_add_zone_id_to_list(parent, FORWARD_ZONE, dc, u)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone id to update list");
		goto cleanup;
	}
	if ((retval = ailsa_update_query(dc, update_queries[SET_FWD_ZONE_UPDATED], u)) != 0) {
		ailsa_syslog(LOG_ERR, "SET_FWD_ZONE_UPDATED query failed");
		goto cleanup;
	}
	cleanup:
		ailsa_list_full_clean(p);
		ailsa_list_full_clean(u);
		ailsa_list_full_clean(z);
		return retval;
}

static int
dnsa_split_glue_ns(char *ns, AILLIST *l)
{
	if (!(ns) || !(l))
		return AILSA_NO_DATA;
	int retval;
	char *server, *tmp, *ptr;

	server = tmp = strndup(ns, FILE_LEN);
	ptr = strchr(tmp, ',');
	if (ptr) {
		*ptr = '\0';
		ptr++;
	}
	if ((retval = cmdb_add_string_to_list(tmp, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add name server to list");
		return retval;
	}
	tmp = ptr;
	if (tmp) {
		ptr = strchr(tmp, ',');
		if (ptr) {
			*ptr = '\0';
			ailsa_syslog(LOG_INFO, "Too many parent name servers. Only using 2");
		}
		if ((retval = cmdb_add_string_to_list(tmp, l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add name server to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("none", l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add name server to list");
			goto cleanup;
		}
	}
	cleanup:
		my_free(server);
		return retval;
}

static int
dnsa_get_ip_for_glue_ns(char *ip, AILLIST *list)
{
	if (!(list))
		return AILSA_NO_DATA;
	char *ip_addr = ailsa_calloc(INET6_ADDRSTRLEN, "ip in cmdb_write_fwd_zone_config");
	int retval, type;
	AILELEM *e;
	ailsa_data_s *d;
	if (ip) {
		retval = dnsa_split_glue_ip(ip, list);
		goto cleanup;
	}
	e = list->head->next->next;
	d = e->data;
	if ((retval = cmdb_getaddrinfo(d->data->text, ip_addr, &type)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot resolve name %s", d->data->text);
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(ip_addr, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add first IP to list");
		goto cleanup;
	}
	e = e->next;
	d = e->data;
	memset(ip_addr, 0, INET6_ADDRSTRLEN);
	if (strncmp(d->data->text, "none", BYTE_LEN) != 0) {
		if ((retval = cmdb_getaddrinfo(d->data->text, ip_addr, &type)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot resolve name %s", d->data->text);
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(ip_addr, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add 2nd IP to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("none", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add 2nd IP to list");
			goto cleanup;
		}
	}
	cleanup:
		my_free(ip_addr);
		return retval;
}

static int
dnsa_split_glue_ip(char *ip, AILLIST *list)
{
	if (!(ip) || !(list))
		return AILSA_NO_DATA;
	char *ip_addr, *tmp, *ptr;
	int retval;

	ip_addr = tmp = strndup(ip, DOMAIN_LEN);
	ptr = strchr(tmp, ',');
	if (ptr) {
		*ptr = '\0';
		ptr++;
	}
	if ((retval = cmdb_add_string_to_list(tmp, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add first NS IP to list");
		goto cleanup;
	}
	tmp = ptr;
	if (tmp) {
		ptr = strchr(tmp, ',');
		if (ptr) {
			ptr = '\0';
			ailsa_syslog(LOG_INFO, "Too many parent NS IP addresses. Only using 2");
		}
		if ((retval = cmdb_add_string_to_list(tmp, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add 2nd NS IP to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("none", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add 2nd IP to list");
			goto cleanup;
		}
	}
	cleanup:
		my_free(ip_addr);
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
