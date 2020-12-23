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
#include "cmdb_dnsa.h"

static void
print_fwd_zone_records(AILLIST *r);

static void
print_fwd_ns_mx_srv_records(char *zone, AILLIST *m);

static void
print_glue_records(char *zone, AILLIST *g);

static void
print_rev_zone_info(char *in_addr, AILLIST *z);

static void
print_rev_zone_records(AILLIST *r);

static int
multi_a_range(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl);

static int
setup_net_range(ailsa_cmdb_s *dc, char *domain, char *range);

static int
get_search_net_range(char *range, char *domain, unsigned long int prefix);

static int
multi_a_ip_address(ailsa_cmdb_s *cbc, dnsa_comm_line_s *dcl);

static int
fill_rev_records(AILLIST *list, AILLIST *rev, unsigned long int index);

static int
fill_fwd_records(AILLIST *list, AILLIST *rev, unsigned long int index);

static int
fill_pref_records(AILLIST *list, AILLIST *pref);

static int
get_rev_records(ailsa_cmdb_s *dc, AILLIST *r, char *range, unsigned long int index);

static int
get_fwd_records(ailsa_cmdb_s *dc, AILLIST *r, char *range, unsigned long int prefix);

static int
get_pref_records(ailsa_cmdb_s *dc, AILLIST *r, char *range, unsigned long int prefix);

static int
cmdb_trim_record_list(AILLIST *rev, AILLIST *pref);

static int
cmdb_records_to_remove(char *range, unsigned long int prefix, AILLIST *rec, AILLIST *rev, AILLIST *remove);

static int
cmdb_records_to_add(char *range, unsigned long int zone_id, unsigned long int prefix, AILLIST *rec, AILLIST *rev, AILLIST *add);

static int
cmdb_get_rev_dest_from_search(char *search, char *dest, char *fqdn);

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
	if ((retval = ailsa_insert_clone(l, z->tail)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(dc, REV_ZONE_ID_ON_RANGE, l, i)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(dc, REV_RECORDS_ON_ZONE_ID, i, r)) != 0)
		goto cleanup;
	print_rev_zone_info(in_addr, z);
	print_rev_zone_records(r);
	cleanup:
		ailsa_list_full_clean(i);
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(z);
}

void
print_rev_zone_info(char *in_addr, AILLIST *z)
{
	if (!(in_addr) || !(z))
		return;
	AILELEM *e = z->head;

	printf("domain: %s\n", in_addr);
	printf("name server: %s\nserial #: %lu\n\n", ((ailsa_data_s *)e->next->data)->data->text, ((ailsa_data_s *)e->data)->data->number);
}

static void
print_rev_zone_records(AILLIST *r)
{
	if (!(r))
		return;
	AILELEM *e = r->head;
	ailsa_data_s *d;

	while (e) {
		d = e->data;
		printf("%s\t", d->data->text);
		e = e->next;
		d = e->data;
		printf("%s\n", d->data->text);
		e = e->next;
	}
}

int
check_zone(char *domain, ailsa_cmdb_s *dc)
{
	char *command, syscom[CONFIG_LEN];
	int retval;
	
	command = &syscom[0];
	
	snprintf(command, CONFIG_LEN, "%s %s %s%s", dc->chkz, domain, dc->dir, domain);
	retval = system(syscom);
	if (retval != 0)
		retval = AILSA_CHKZONE_FAIL;
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
		if ((retval = cmdb_validate_zone(dc, FORWARD_ZONE, zone, type, 0)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate zone %s", zone);
			goto cleanup;
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
			if ((retval = cmdb_validate_zone(dc, FORWARD_ZONE, zone, type, 0)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot validate zone %s", zone);
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
	char *zone, *type;
	unsigned long int prefix;
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
		d = e->next->data;
		type = d->data->text;
		d = e->next->next->data;
		prefix = d->data->number;
		if (name) {
			if (strncmp(name, zone, DOMAIN_LEN) == 0)
				if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, zone, type, prefix)) != 0)
					ailsa_syslog(LOG_ERR, "Unable to validate zone %s");
		} else {
			if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, zone, type, prefix)) != 0)
				ailsa_syslog(LOG_ERR, "Unable to validate zone %s");
		}
		e = ailsa_move_down_list(e, 2);
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
	int retval = 0;

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
		if ((retval = ailsa_argument_query(dc, FQDN_PREF_A_ON_IP, pre, mod)) != 0) {
			ailsa_syslog(LOG_ERR, "FQDN_PREF_A_ON_IP query failed");
			goto cleanup;
		}
		if (strlen(d->data->text) < 8)
			printf("%s\t\t", d->data->text);
		else
			printf("%s\t", d->data->text);
		d = e->next->data;
		printf("%lu\t", d->data->number);
		if (mod->total > 0) {
			d = mod->head->data;
			printf("%s\n", d->data->text);
		} else {
			printf("No preferred A record set\n");
		}
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
		if (fqdn->total > 0) {
			if (strncmp(hostname, ((ailsa_data_s *)fqdn->head->data)->data->text, DOMAIN_LEN) == 0)
				printf("   *\t%s\n", hostname);
			else
				printf("\t%s\n", hostname);
		} else {
			printf("\t%s\n", hostname);
		}
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
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	char *fqdn = ailsa_calloc(DOMAIN_LEN, "fqdn in mark_preferred_a_record");
	int retval;
	uint32_t ip_addr;
	unsigned long int ip;
	size_t total;
	AILLIST *i = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cm->dest, i)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add ip address to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, DUP_IP_A_RECORD, i, r)) != 0) {
		ailsa_syslog(LOG_ERR, "DUP_IP_A_RECORD query failed");
		goto cleanup;
	}
	if (r->total == 0) {
		ailsa_syslog(LOG_INFO, "IP address %s has no duplicate entries", cm->dest);
		goto cleanup;
	}
	ailsa_list_clean(r);
	if ((retval = ailsa_argument_query(dc, FQDN_PREF_A_ON_IP, i, r)) != 0) {
		ailsa_syslog(LOG_ERR, "FQDN_PREF_A_ON_IP query failed");
		goto cleanup;
	}
	if (r->total > 0) {
		ailsa_syslog(LOG_INFO, "IP address %s already has a preferred A record", cm->dest);
		goto cleanup;
	}
	ailsa_list_clean(r);
	if (inet_pton(AF_INET, cm->dest, &ip_addr)) {
		ip = (unsigned long int)htonl(ip_addr);
	} else {
		ailsa_syslog(LOG_ERR, "Cannot convert IP address: %s", cm->dest);
		retval =  DEST_INPUT_INVALID;
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(ip, i)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add network IP address to list");
		goto cleanup;
	}
	if ((retval = cmdb_check_add_zone_id_to_list(cm->domain, FORWARD_ZONE, "master", dc, r)) != 0)
		goto cleanup;
	if ((retval = dnsa_populate_record(dc, cm, r)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot populate record info");
		goto cleanup;
	}
	total = i->total;
	snprintf(fqdn, DOMAIN_LEN, "%s.%s", cm->host, cm->domain);
	if ((retval = ailsa_argument_query(dc, RECORD_ID_BASE, r, i)) != 0) {
		ailsa_syslog(LOG_ERR, "RECORD_ID_BASE query failed");
		goto cleanup;
	}
	if (total == i->total) {
		ailsa_syslog(LOG_ERR, "A record %s for %s does not exist", fqdn, cm->dest);
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(fqdn, i)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add fqdn to list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(i)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(dc, INSERT_PREF_A, i)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_PREF_A query failed");
		goto cleanup;
	}
	cleanup:
		my_free(fqdn);
		ailsa_list_full_clean(i);
		ailsa_list_full_clean(r);
		return retval;
}

int
delete_preferred_a(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cm->dest, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add IP to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(dc, delete_queries[DELETE_PREF_A], l)) != 0)
		ailsa_syslog(LOG_ERR, "DELETE_PREF_A query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
add_host(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval;
	unsigned int query;
	AILLIST *rec = ailsa_db_data_list_init();
	AILLIST *z = ailsa_db_data_list_init();

	if ((retval = cmdb_check_add_zone_id_to_list(cm->domain, FORWARD_ZONE, "master", dc, rec)) != 0)
		goto cleanup;
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
		retval = AILSA_WRONG_LIST_LENGHT;
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(dc, query, rec)) != 0) {
		ailsa_syslog(LOG_ERR, "Insert record query %u failed", query);
		goto cleanup;
	}
	if ((retval = set_db_row_updated(dc, SET_FWD_ZONE_UPDATED_ON_NAME, cm->domain, 0)) != 0)
		ailsa_syslog(LOG_ERR, "Failed to set zone %s as updated", cm->domain);

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
			if ((retval = cmdb_check_add_zone_id_to_list(tmp, FORWARD_ZONE, "master", dc, c)) != 0)
				goto cleanup;
		} else {
			ailsa_syslog(LOG_ERR, "Cannot determine top level domain");
			retval = AILSA_NO_TOP_LEVEL_DOMAIN;
			goto cleanup;
		}
		cm->toplevel = strndup(tmp, DOMAIN_LEN);
	} else {
		if ((retval = cmdb_check_add_zone_id_to_list(cm->toplevel, FORWARD_ZONE, "master", dc, c)) != 0)
			goto cleanup;
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

	if ((retval = cmdb_check_add_zone_id_to_list(dom, FORWARD_ZONE, "master", dc, d)) != 0)
		goto cleanup;
	if (top) {
		if ((retval = cmdb_check_add_zone_id_to_list(top, FORWARD_ZONE, "master", dc, t)) != 0)
			goto cleanup;
	} else {
		snprintf(domain, DOMAIN_LEN, "%s", dom);
		tmp = strchr(domain, '.');
		if (tmp) {
			tmp++;
			if ((retval = cmdb_check_add_zone_id_to_list(tmp, FORWARD_ZONE, "master", dc, t)) != 0)
				goto cleanup;
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
	AILLIST *zone = ailsa_db_data_list_init();

	if ((retval = cmdb_check_add_zone_id_to_list(cm->domain, FORWARD_ZONE, "master", dc, rec)) != 0)
		goto cleanup;
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
		retval = AILSA_WRONG_LIST_LENGHT;
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
	if ((retval = set_db_row_updated(dc, SET_FWD_ZONE_UPDATED_ON_NAME, cm->domain, 0)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot set zone %s to updated", cm->domain);
	cleanup:
		ailsa_list_full_clean(rec);
		ailsa_list_full_clean(list);
		ailsa_list_full_clean(zone);
		return retval;
}

int
add_fwd_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval = add_forward_zone(dc, cm->domain, cm->ztype, cm->master);
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

	if (cm->ztype) {
		if ((retval = cmdb_check_add_zone_id_to_list(cm->domain, FORWARD_ZONE, cm->ztype, dc, z)) != 0)
			goto cleanup;
	} else {
		if ((retval = cmdb_check_add_zone_id_to_list(cm->domain, FORWARD_ZONE, "master", dc, z)) != 0)
			goto cleanup;
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
	int retval = add_reverse_zone(dc, cm->domain, cm->ztype, cm->master, cm->prefix);
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

	if (cm->ztype) {
		if ((retval = cmdb_check_add_zone_id_to_list(cm->domain, REVERSE_ZONE, cm->ztype, dc, rev)) != 0)
			goto cleanup;
	} else {
		if ((retval = cmdb_check_add_zone_id_to_list(cm->domain, REVERSE_ZONE, "master", dc, rev)) != 0)
			goto cleanup;
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
	int retval;
	char comm[DOMAIN_LEN];
	unsigned long int prefix, index, zone_id;
	AILLIST *add = ailsa_db_data_list_init();
	AILLIST *net = ailsa_db_data_list_init();
	AILLIST *rem = ailsa_db_data_list_init();
	AILLIST *fix = ailsa_db_data_list_init();
	AILLIST *rec = ailsa_record_list_init();
	AILLIST *rev = ailsa_record_list_init();
	AILLIST *pref = ailsa_preferred_init();

	if ((retval = cmdb_add_string_to_list(cm->domain, net)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(dc, PREFIX_ON_NET_RANGE, net, fix)) != 0)
		goto cleanup;
	if (fix->total == 0)
		goto cleanup;
	prefix = strtoul(((ailsa_data_s *)fix->head->data)->data->text, NULL, 10);
	ailsa_list_clean(fix);
	if ((retval = cmdb_add_string_to_list("master", net)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(dc, REV_ZONE_ID_ON_RANGE, net, fix)) != 0)
		goto cleanup;
	if (fix->total > 0)
		zone_id = ((ailsa_data_s *)fix->head->data)->data->number;
	else
		goto cleanup;
	if ((retval = get_zone_index(prefix, &index)) != 0)
		goto cleanup;
	if ((retval = get_rev_records(dc, rev, cm->domain, index)) != 0)
		goto cleanup;
	if ((retval = get_fwd_records(dc, rec, cm->domain, prefix)) != 0)
		goto cleanup;
	if ((retval = get_pref_records(dc, pref, cm->domain, prefix)) != 0)
		goto cleanup;
	if ((retval = cmdb_trim_record_list(rec, pref)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot trim records list");
		goto cleanup;
	}
	if ((retval = cmdb_records_to_remove(cm->domain, prefix, rec, rev, rem)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create list of records to remove");
		goto cleanup;
	}
	if ((retval = cmdb_records_to_add(cm->domain, zone_id, prefix, rec, rev, add)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create list of records to add");
		goto cleanup;
	}
	if (rem->total > 0 || add->total > 0) {
		if (rem->total > 0) {
			if ((retval = ailsa_multiple_delete(dc, delete_queries[DELETE_REVERSE_RECORD], rem)) != 0) {
				ailsa_syslog(LOG_ERR, "DELETE_REVERSE_RECORD multi query failed");
				goto cleanup;
			}
		}
		if (add->total > 0) {
			if ((retval = ailsa_multiple_query(dc, insert_queries[INSERT_REVERSE_RECORD], add)) != 0) {
				ailsa_syslog(LOG_ERR, "INSERT_REVERSE_RECORD multi query failed");
				goto cleanup;
			}
		}
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
		if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, cm->domain, "master", prefix)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate zone %s", cm->domain);
			goto cleanup;
		}
		memset(comm, 0, DOMAIN_LEN);
		snprintf(comm, CONFIG_LEN, "%s reload", dc->rndc);
		if ((retval = system(comm)) != 0)
			ailsa_syslog(LOG_ERR, "Reload of nameserver failed");
        cleanup:

	}
	cleanup:
		ailsa_list_full_clean(add);
		ailsa_list_full_clean(net);
		ailsa_list_full_clean(rec);
		ailsa_list_full_clean(rem);
		ailsa_list_full_clean(pref);
		ailsa_list_full_clean(rev);
		ailsa_list_full_clean(fix);
		return retval;
}

static int
get_rev_records(ailsa_cmdb_s *dc, AILLIST *r, char *range, unsigned long int index)
{
	if (!(dc) || !(r) || !(range))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *net = ailsa_db_data_list_init();
	ailsa_data_s *data;
	unsigned long int i;

	if ((retval = cmdb_add_string_to_list(range, net)) != 0)
		goto cleanup;
	for (i = 0; i < index; i++) {
		if ((retval = cmdb_add_number_to_list(i, net)) != 0)
			goto cleanup;
		if ((retval = ailsa_argument_query(dc, FULL_REV_RECORDS, net, l)) != 0)
			goto cleanup;
		if ((retval = fill_rev_records(l, r, i)) != 0)
			goto cleanup;
		if ((retval = ailsa_list_remove(net, net->tail, (void **)&data)) == 0)
			net->destroy(data);
		else
			return AILSA_LIST_CANNOT_REMOVE;
		ailsa_list_clean(l);
	}
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(net);
		return retval;
}

static int
fill_rev_records(AILLIST *list, AILLIST *rev, unsigned long int index)
{
	if (!(list) || !(rev))
		return AILSA_NO_DATA;
	int retval = 0;
	AILELEM *e;
	ailsa_record_s *record;

	e = list->head;
	while (e) {
		record = ailsa_calloc(sizeof(ailsa_record_s), "record in fill_rev_records");
		record->host = strndup(((ailsa_data_s *)e->data)->data->text, HOST_LEN);
		record->dest = strndup(((ailsa_data_s *)e->next->data)->data->text, DOMAIN_LEN);
		record->id = ((ailsa_data_s *)e->next->next->data)->data->number;
		record->zone = ((ailsa_data_s *)e->next->next->next->data)->data->number;
		record->index = index;
		if ((retval = ailsa_list_insert(rev, record)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert record into list");
			return retval;
		}
		e = ailsa_move_down_list(e, 4);
	}
	return retval;
}

static int
get_fwd_records(ailsa_cmdb_s *dc, AILLIST *r, char *range, unsigned long int prefix)
{
	if (!(dc) || !(r) || !(range) || (prefix == 0))
		return AILSA_NO_DATA;
	int retval;
	char search[HOST_LEN];
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *net = ailsa_db_data_list_init();
	unsigned long int index, i;

	if ((retval = get_zone_index(prefix, &index)) != 0)
		goto cleanup;
	for (i = 0; i < index; i++) {
		memset(search, 0, HOST_LEN);
		if ((retval = get_range_search_string(range, search, prefix, i)) != 0)
			goto cleanup;
		if ((retval = cmdb_add_string_to_list(search, net)) != 0)
			goto cleanup;
		if ((retval = ailsa_argument_query(dc, RECORDS_ON_NET_RANGE, net, l)) != 0)
			goto cleanup;
		if ((retval = fill_fwd_records(l, r, i)) != 0)
			goto cleanup;
		ailsa_list_clean(l);
		ailsa_list_clean(net);
	}
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(net);
		return retval;
}

static int
fill_fwd_records(AILLIST *list, AILLIST *rec, unsigned long int index)
{
	if (!(list) || !(rec))
		return AILSA_NO_DATA;
	int retval = 0;
	AILELEM *e;
	ailsa_record_s *record;

	e = list->head;
	while (e) {
		record = ailsa_calloc(sizeof(ailsa_record_s), "record in fill_fwd_zones");
		record->id = ((ailsa_data_s *)e->next->data)->data->number;
		record->index = index;
		record->dest = strndup(((ailsa_data_s *)e->data)->data->text, DOMAIN_LEN);
		record->host = strndup(((ailsa_data_s *)e->next->next->data)->data->text, DOMAIN_LEN);
		record->domain = strndup(((ailsa_data_s *)e->next->next->next->data)->data->text, DOMAIN_LEN);
		if ((retval = ailsa_list_insert(rec, record)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert record into list");
			return retval;
		}
		e = ailsa_move_down_list(e, 4);
	}
	return retval;
}

static int
get_pref_records(ailsa_cmdb_s *dc, AILLIST *r, char *range, unsigned long int prefix)
{
	if (!(dc) || !(r) || !(range))
		return AILSA_NO_DATA;
	int retval;
	char search[HOST_LEN];
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *p = ailsa_db_data_list_init();
	unsigned long int index, i;

	if ((retval = get_zone_index(prefix, &index)) != 0)
		goto cleanup;
	for (i = 0; i < index; i++) {
		memset(search, 0, HOST_LEN);
		if ((retval = get_range_search_string(range, search, prefix, i)) != 0)
			goto cleanup;
		if ((retval = cmdb_add_string_to_list(search, l)) != 0)
			goto cleanup;
		if ((retval = ailsa_argument_query(dc, PREFER_A_INFO_ON_RANGE, l, p)) != 0)
			goto cleanup;
		if ((retval = fill_pref_records(p, r)) != 0)
			goto cleanup;
		ailsa_list_clean(l);
		ailsa_list_clean(p);
	}

	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(p);
		return retval;
}

static int
fill_pref_records(AILLIST *list, AILLIST *pref)
{
	if (!(list) || !(pref))
		return AILSA_NO_DATA;
	int retval = 0;
	AILELEM *e;
	ailsa_preferred_s *p;

	e = list->head;
	while (e) {
		p = ailsa_calloc(sizeof(ailsa_preferred_s), "p in fill_pref_records");
		p->ip = strndup(((ailsa_data_s *)e->data)->data->text, HOST_LEN);
		p->fqdn = strndup(((ailsa_data_s *)e->next->data)->data->text, DOMAIN_LEN);
		p->record_id = ((ailsa_data_s *)e->next->next->data)->data->number;
		if ((retval = ailsa_list_insert(pref, p)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert preferred into list");
			return retval;
		}
		e = ailsa_move_down_list(e, 3);
	}
	return retval;
}

static int
cmdb_trim_record_list(AILLIST *r, AILLIST *p)
{
	if (!(r) || !(p))
		return AILSA_NO_DATA;
	int retval = 0;
	void *data;
	AILELEM *record, *preferred, *tmp;
	ailsa_preferred_s *pref;
	ailsa_record_s *rec;
	if ((r->total == 0) || (p->total == 0))
		return retval;
	if (!(r->destroy))
		return AILSA_LIST_NO_DESTROY;
	preferred = p->head;
	while (preferred) {
		record = r->head;
		while (record) {
			pref = preferred->data;
			rec = record->data;
			if (strncmp(pref->ip, rec->dest, DOMAIN_LEN) == 0) {
				if (rec->id != pref->record_id) {
					if (record->prev) {
						tmp = record->prev;
						if ((retval = ailsa_list_remove(r, tmp->next, &data)) == 0) {
							r->destroy(data);
						} else {
							ailsa_syslog(LOG_ERR, "Cannot remove element from list");
							return AILSA_LIST_CANNOT_REMOVE;
						}
						record = tmp->next;
					} else {
						if ((retval = ailsa_list_remove(r, r->head, &data)) == 0) {
							r->destroy(data);
						} else {
							ailsa_syslog(LOG_ERR, "Cannot remove head element from list");
							return AILSA_LIST_CANNOT_REMOVE;
						}
						record = r->head;
					}
					continue;
				}
			}
			record = record->next;
		}
		preferred = preferred->next;
	}
	return retval;
}

static int
cmdb_records_to_remove(char *range, unsigned long int prefix, AILLIST *rec, AILLIST *rev, AILLIST *remove)
{
	if (!(range) || !(rec) || !(rev) || !(remove))
		return AILSA_NO_DATA;
	int retval;
	char search[HOST_LEN];
	char fqdn[DOMAIN_LEN];
	char *ptr;
	unsigned long int index, i;
	AILELEM *r, *f;
	ailsa_record_s *reverse, *forward;

	if ((retval = get_zone_index(prefix, &index)) != 0)
		return retval;
	for (i = 0; i < index; i++) {
		memset(search, 0, HOST_LEN);
		if ((retval = get_range_search_string(range, search, prefix, i)) != 0)
			return retval;
		if (!(ptr =  strrchr(search, '%')))
			goto cleanup;
		r = rev->head;
		while (r) {
			reverse = r->data;
			f = rec->head;
			while (f) {
				forward = f->data;
				snprintf(ptr, SERVICE_LEN, "%s", reverse->host);
				if (strncmp(search, forward->dest, HOST_LEN) == 0){
					memset(fqdn, 0, DOMAIN_LEN);
					if (strncmp("@", forward->host, BYTE_LEN) == 0)
						snprintf(fqdn, DOMAIN_LEN, "%s.", forward->domain);
					else
						snprintf(fqdn, DOMAIN_LEN, "%s.%s.", forward->host, forward->domain);
					if (strncmp(reverse->dest, fqdn, DOMAIN_LEN) == 0)
						break;
				}
				f = f->next;
			}
			if (!(f)) {
				if ((retval = cmdb_add_number_to_list(reverse->id, remove)) != 0)
					return retval;
			}
			r = r->next;
		}
	}
	return retval;
	cleanup:
		ailsa_syslog(LOG_ERR, "String manipulation failed");
		return AILSA_STRING_FAIL;
}

static int
cmdb_records_to_add(char *range, unsigned long int zone_id, unsigned long int prefix, AILLIST *rec, AILLIST *rev, AILLIST *add)
{
	if (!(range) || !(rec) || !(rev) || !(add))
		return AILSA_NO_DATA;
	int retval;
	char search[DOMAIN_LEN];
	char fqdn[DOMAIN_LEN];
	char *ptr, *pts;
	unsigned long int index, i;
	size_t len;
	AILELEM *r, *f;
	ailsa_record_s *reverse, *forward;

	if ((retval = get_zone_index(prefix, &index)) != 0)
		return retval;
	for (i = 0; i < index; i++) {
		memset(search, 0, DOMAIN_LEN);
		if ((retval = get_range_search_string(range, search, prefix, i)) != 0)
			return retval;
		if (!(ptr =  strrchr(search, '%')))
			goto cleanup;
		f = rec->head;
		while (f) {
			forward = f->data;
			r = rev->head;
			while (r) {
				reverse = r->data;
				snprintf(ptr, MAC_LEN, "%s", reverse->host);
				if (strncmp(forward->dest, search, DOMAIN_LEN) == 0) {
					memset(fqdn, 0, DOMAIN_LEN);
					if (strncmp(forward->host, "@", BYTE_LEN) == 0)
						snprintf(fqdn, DOMAIN_LEN, "%s.", forward->domain);
					else
						snprintf(fqdn, DOMAIN_LEN, "%s.%s.", forward->host, forward->domain);
					if ((strncmp(fqdn, reverse->dest, DOMAIN_LEN) == 0))
						break;
				}
				r = r->next;
			}
			if (!(r)) {
				memset(fqdn, 0, DOMAIN_LEN);
				snprintf(fqdn, DOMAIN_LEN, "%s", search);
				pts = strrchr(fqdn, '.');
				*pts = '\0';
				len = strlen(fqdn);
				if (strncmp(fqdn, forward->dest, len) == 0) {
					if ((retval = cmdb_add_number_to_list(zone_id, add)) != 0)
						return retval;
					if ((retval = cmdb_add_number_to_list(i, add)) != 0)
						return retval;
					memset(fqdn, 0, DOMAIN_LEN);
					if ((retval = cmdb_get_rev_dest_from_search(search, forward->dest, fqdn)) != 0)
						goto cleanup;
					if ((retval = cmdb_add_string_to_list(fqdn, add)) != 0)
						goto cleanup;
					memset(fqdn, 0, DOMAIN_LEN);
					if (strncmp(forward->host, "@", BYTE_LEN) == 0)
						snprintf(fqdn, DOMAIN_LEN, "%s.", forward->domain);
					else
						snprintf(fqdn, DOMAIN_LEN, "%s.%s.", forward->host, forward->domain);
					if ((retval = cmdb_add_string_to_list(fqdn, add)) != 0)
						return retval;
					if ((retval = cmdb_populate_cuser_muser(add)) != 0)
						goto cleanup;
				}
			}
			f = f->next;
		}
	}
	return retval;
	cleanup:
		ailsa_syslog(LOG_ERR, "String manipulation failed");
		return retval;
}

static int
cmdb_get_rev_dest_from_search(char *search, char *dest, char *fqdn)
{
	if (!(search) || !(dest) || !(fqdn))
		return AILSA_NO_DATA;
	size_t len;
	char *ptr;

	len = strlen(search) - 1;
	ptr = dest + len;
	snprintf(fqdn, MAC_LEN, "%s", ptr);
	return 0;
}

int
add_glue_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm)
{
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
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
	if ((retval = cmdb_check_add_zone_id_to_list(parent, FORWARD_ZONE, "master", dc, z)) != 0)
		goto cleanup;
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
	if ((retval = set_db_row_updated(dc, SET_FWD_ZONE_UPDATED_ON_NAME, parent, 0)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot set zone %s to be updated", parent);
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
	if (!(dc) || !(cm))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *g = ailsa_db_data_list_init();
	AILLIST *z = ailsa_db_data_list_init();
	AILLIST *i = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cm->domain, g)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add glue domain name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(dc, PARENT_ZONE_ID_ON_GLUE_ZONE, g, i)) != 0) {
		ailsa_syslog(LOG_ERR, "PARENT_ZONE_ID_ON_GLUE_ZONE query failed");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(dc, delete_queries[DELETE_GLUE_ZONE], g)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_GLUE_ZONE query failed");
		goto cleanup;
	}
	if (i->total == 0) {
		ailsa_syslog(LOG_ERR, "PARENT_ZONE_ID_ON_GLUE_ZONE query got nothing");
		goto cleanup;
	}
	if ((retval = set_db_row_updated(dc, SET_FWD_ZONE_UPDATED, NULL, ((ailsa_data_s *)i->head->data)->data->number)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot set parent zone for %s to be updated", cm->domain);
	
	cleanup:
		ailsa_list_full_clean(g);
		ailsa_list_full_clean(z);
		ailsa_list_full_clean(i);
		return retval;
}

void
list_glue_zones(ailsa_cmdb_s *dc)
{
	if (!(dc))
		return;
	char *str;
	int retval;
	size_t total = 6;
	AILLIST *g = ailsa_db_data_list_init();
	AILELEM *e;
	size_t len = 0;

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
		str = ((ailsa_data_s *)e->data)->data->text;
		len = strnlen(str, DOMAIN_LEN);
		printf("%s", str);
		if (len < 8)
			printf("\t\t");
		else if (len < 16)
			printf("\t");
		else
			printf("\n\t\t");
		str = ((ailsa_data_s *)e->next->data)->data->text;
		len = strnlen(str, DOMAIN_LEN);
		printf("%s", str);
		if (len < 8)
			printf("\t\t\t");
		else if (len < 16)
			printf("\t\t");
		else if (len < 24)
			printf("\t");
		else
			printf("\n\t\t\t");
		printf("%s ", ((ailsa_data_s *)e->next->next->data)->data->text);
		printf("%s ", ((ailsa_data_s *)e->next->next->next->next->data)->data->text);
		printf("%s\t", ((ailsa_data_s *)e->next->next->next->data)->data->text);
		printf("%s\n", ((ailsa_data_s *)e->next->next->next->next->next->data)->data->text);
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		ailsa_list_full_clean(g);
}

void
list_test_zones(ailsa_cmdb_s *dc)
{
	if (!(dc))
		return;
	int c = 5;
	int i, retval;
	char search[SERVICE_LEN];
	const char *zones[] = { "192.168.1.0/24", "192.168.80.0/21", "172.16.0.0/14", "10.0.0.0/8", "172.24.0.0/16" };
	unsigned long int prefixes[c], indicies[c], start[c], end[c], j;
	char **ips = ailsa_calloc((sizeof(char *)) * (size_t)c, "ips in list_test_zones");
	for (i = 0; i < c; i++) {
		if ((retval = get_ip_addr_and_prefix(zones[i], &(ips[i]), &prefixes[i])) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot convert index no %d: %s", i, zones[i]);
			goto cleanup;
		}
		if ((retval = get_zone_index(prefixes[i], &indicies[i])) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot get index number %d for prefix %lu", i, prefixes[i]);
			goto cleanup;
		}
		if ((retval = get_start_finsh_ips(ips[i], prefixes[i], &start[i], &end[i])) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot get start and end IP address");
			goto cleanup;
		}
		printf("Range: %s\n\tNetwork: %s\tPrefix: %lu\tIndexes: %lu\n", zones[i], ips[i], prefixes[i], indicies[i]);
		printf("  Start IP: %lu\t  End IP: %lu\n", start[i], end[i]);
		for (j = 0; j < indicies[i]; j++) {
			if ((retval = get_range_search_string(ips[i], search, prefixes[i], j)) != 0) {
				ailsa_syslog(LOG_ERR, "search range lookup failed for %s on %lu", zones[i], j);
				goto cleanup;
			}
			printf("\t%s\t%lu\n", search, j);
		}
		printf("\n");
	}
	cleanup:
		for (i = 0; i < c; i++) {
			if (ips[i])
				my_free(ips[i]);
		}
		my_free(ips);
}
