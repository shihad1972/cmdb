/*
 *
 *  dnsa: Domain Name System Admistration
 *  (C) 2014 - 2020 Iain M. Conochie <iain-AT-thargoid.co.uk>
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
 *  dnsa_net.c: Holds functions for network related funcitons for dnsa
 *
 */
#include <config.h>
#include <configmake.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "dnsa_data.h"
#include "cmdb_dnsa.h"
#include "dnsa_net.h"

static int
write_fwd_zone_file(ailsa_cmdb_s *cbc, char *zone);

static int
write_rev_zone_file(ailsa_cmdb_s *cbc, char *zone);

static int
ailsa_check_for_zone_update(ailsa_cmdb_s *cbc, AILLIST *l, char *zone);

static int
ailsa_check_for_rev_zone_update(ailsa_cmdb_s *cbs, AILLIST *l, char *zone);

static void
write_zone_file_header(int fd, AILLIST *n, AILLIST *s, char *master);

static void
write_fwd_header_records(int fd, AILLIST *r, char *zone);

static void
write_fwd_records(int fd, AILLIST *r, char *zone);

static int
cmdb_validate_fwd_zone(ailsa_cmdb_s *cbc, char *zone);

static int
cmdb_validate_rev_zone(ailsa_cmdb_s *cbc, char *zone);

static int
cmdb_check_zone(ailsa_cmdb_s *cbs, char *zone);

static int
write_glue_records(ailsa_cmdb_s *cbc, int fd, AILLIST *g, const char *zone);

static void
write_rev_zone_header(int fd, AILLIST *soa, char *hostmaster);

static void
write_rev_zone_records(int fd, AILLIST *soa);

unsigned long int
get_net_range(unsigned long int prefix)
{
        unsigned long int range;
        range = 4294967295UL;
        range = (range >> prefix) + 1;
        return range;
}

int
do_rev_lookup(char *ip, char *host, size_t size)
{
	int retval = 0;
	struct addrinfo hints, *res;
	socklen_t len = sizeof(struct sockaddr_in6);
	socklen_t hlen = (socklen_t)size;

	if (!(ip) || !(host))
		return NULL_POINTER_PASSED;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((retval = getaddrinfo(ip, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "Getaddrinfo in do_rev_lookup: %s\n",
		  gai_strerror(retval));
		return NET_FUNC_FAILED;
	}
	if ((retval = getnameinfo(res->ai_addr, len, host, hlen, NULL, 0, NI_NAMEREQD)) != 0) {
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(retval));
		retval = DNS_LOOKUP_FAILED;
	}
	return retval;
}

int
cmdb_validate_zone(ailsa_cmdb_s *cbc, int type, char *zone)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	int retval;

	switch(type) {
	case FORWARD_ZONE:
		if ((retval = cmdb_validate_fwd_zone(cbc, zone)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate forward zone");
			return retval;
		}
		break;
	case REVERSE_ZONE:
		if ((retval = cmdb_validate_rev_zone(cbc, zone)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate reverse zone");
			return retval;
		}
		break;
	default:
		ailsa_syslog(LOG_ERR, "Unknown zone type %s", type);
		retval = UNKNOWN_ZONE_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_validate_fwd_zone(ailsa_cmdb_s *cbc, char *zone)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	char *command = ailsa_calloc(CONFIG_LEN, "command in cmdb_validate_fwd_zone");
	int retval;

	if ((retval = write_fwd_zone_file(cbc, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write zone file for domain %s", zone);
		goto cleanup;
	}
	if ((retval = cmdb_check_zone(cbc, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Checking the zone failed");
		if ((retval = cmdb_add_string_to_list("no", l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add valid to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("yes", l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add invalid to list");
			goto cleanup;
		}
	}
	if ((retval = cmdb_add_string_to_list(zone, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone name to list");
		goto cleanup;
	}
	if ((retval = ailsa_update_query(cbc, update_queries[FWD_ZONE_VALIDATE], l)) != 0)
		ailsa_syslog(LOG_ERR, "FWD_ZONE_VALIDATE update query failed");
	cleanup:
		my_free(command);
		ailsa_list_full_clean(l);
		return retval;
}

static int
write_fwd_zone_file(ailsa_cmdb_s *cbc, char *zone)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	AILLIST *a = ailsa_db_data_list_init();
	AILLIST *g = ailsa_db_data_list_init();
	AILLIST *n = ailsa_db_data_list_init();
	AILLIST *s = ailsa_db_data_list_init();
	AILLIST *hr = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	int retval, flags, fd;
	mode_t um, mask;
	char *name = ailsa_calloc(DOMAIN_LEN, "name in write_fwd_zone_file");

	if ((retval = cmdb_add_string_to_list(zone, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone name to argument list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, NAME_SERVERS_ON_NAME, a, n)) != 0) {
		ailsa_syslog(LOG_ERR, "NAME_SERVERS_ON_NAME query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, ZONE_SOA_ON_NAME, a, s)) != 0) {
		ailsa_syslog(LOG_ERR, "ZONE_SOA_ON_NAME query failed");
		goto cleanup;
	}
	if ((retval = ailsa_check_for_zone_update(cbc, s, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Checking for zone updated failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, NS_MX_SRV_RECORDS, a, hr)) != 0) {
		ailsa_syslog(LOG_ERR, "NS_MX_SRV_RECORDS query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, ZONE_RECORDS_ON_NAME, a, r)) != 0) {
		ailsa_syslog(LOG_ERR, "ZONE_RECORDS_ON_NAME query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, GLUE_ZONE_ON_ZONE_NAME, a, g)) != 0) {
		ailsa_syslog(LOG_ERR, "GLUE_ZONE_ON_ZONE_NAME query failed");
		goto cleanup;
	}
	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((snprintf(name, DOMAIN_LEN, "%s%s", cbc->dir, zone)) >= DOMAIN_LEN)
		ailsa_syslog(LOG_INFO, "Path truncated in write_fwd_zone_file");
	if ((fd = open(name, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "%s", strerror(errno));
		retval = FILE_O_FAIL;
		goto cleanup;
	}
	write_zone_file_header(fd, n, s, cbc->hostmaster);
	write_fwd_header_records(fd, hr, zone);
	write_fwd_records(fd, r, zone);
	if ((retval = write_glue_records(cbc, fd, g, zone)) != 0)
		ailsa_syslog(LOG_ERR, "Writing glue records failed");
	close(fd);
	mask = umask(um);
	cleanup:
		ailsa_list_full_clean(a);
		ailsa_list_full_clean(g);
		ailsa_list_full_clean(n);
		ailsa_list_full_clean(s);
		ailsa_list_full_clean(hr);
		ailsa_list_full_clean(r);
		my_free(name);
		return retval;
}

static int
ailsa_check_for_zone_update(ailsa_cmdb_s *cbs, AILLIST *l, char *zone)
{
	if (!(l) || !(cbs) || !(zone))
		return AILSA_NO_DATA;
	if ((l->total % 6) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number in list. wanted 6 got %zu", l->total);
		return WRONG_LENGTH_LIST;
	}
	int retval = 0;
	AILLIST *s = ailsa_db_data_list_init();
	AILELEM *e;
	unsigned long int serial;

	e = l->head;
	if (strcmp("yes", ((ailsa_data_s *)e->next->next->next->next->next->data)->data->text) == 0) {
		serial = generate_zone_serial();
		if (serial <= ((ailsa_data_s *)e->next->data)->data->number)
			serial = ++((ailsa_data_s *)e->next->data)->data->number;
		else
			((ailsa_data_s *)e->next->data)->data->number = serial;
		if ((retval = cmdb_add_number_to_list(serial, s)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add serial number to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(zone, s)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone name to list");
			goto cleanup;
		}
		if ((retval = ailsa_update_query(cbs, update_queries[FWD_ZONE_SERIAL_UPDATE], s)) != 0) {
			ailsa_syslog(LOG_ERR, "FWD_ZONE_SERIAL_UPDATE query failed");
			goto cleanup;
		}
	}

	cleanup:
		ailsa_list_full_clean(s);
		return retval;
}

static int
ailsa_check_for_rev_zone_update(ailsa_cmdb_s *cbs, AILLIST *l, char *zone)
{
	if (!(cbs) || !(l) || !(zone))
		return AILSA_NO_DATA;
	if ((l->total % 8) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number in list. wanted 8 got %zu", l->total);
		return WRONG_LENGTH_LIST;
	}
	int retval = 0;
	AILLIST *s = ailsa_db_data_list_init();
	AILELEM *e;
	unsigned long int serial;

	e = l->head;
	if (strcmp("yes", ((ailsa_data_s *)e->next->next->next->next->next->next->next->data)->data->text) == 0) {
		serial = generate_zone_serial();
		if (serial <= ((ailsa_data_s *)e->next->next->next->data)->data->number)
			serial = ++((ailsa_data_s *)e->next->next->next->data)->data->number;
		else
			((ailsa_data_s *)e->next->next->next->data)->data->number = serial;
		if ((retval = cmdb_add_number_to_list(serial, s)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add serial number to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(zone, s)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add net range to list");
			goto cleanup;
		}
		if ((retval = ailsa_update_query(cbs, update_queries[REV_ZONE_SERIAL_UPDATE], s)) != 0) {
			ailsa_syslog(LOG_ERR, "REV_ZONE_SERIAL_UPDATE query failed");
			goto cleanup;
		}
	}
	cleanup:
		ailsa_list_full_clean(s);
		return retval;
}

static void
write_zone_file_header(int fd, AILLIST *n, AILLIST *s, char *master)
{
	if (!(n) || !(s) || !(master) || (fd == 0))
		return;
	if (s->total != 6) {
		ailsa_syslog(LOG_ERR, "Cannot write zone file header");
		return;
	}
	char *pri = ((ailsa_data_s *)n->head->data)->data->text;
	char *sec = ((ailsa_data_s *)n->head->next->data)->data->text;
	size_t plen = strlen(pri);
	size_t slen = strlen(pri);
	if (pri[plen - 1] != '.')
		dprintf(fd, "$TTL %lu\n@\tIN\tSOA\t%s.\t%s\t(\n", ((ailsa_data_s *)s->head->data)->data->number, pri, master);
	else 
		dprintf(fd, "$TTL %lu\n@\tIN\tSOA\t%s\t%s\t(\n", ((ailsa_data_s *)s->head->data)->data->number, pri, master);
	dprintf(fd, "\t\t\t\t%lu\t; Serial\n", ((ailsa_data_s *)s->head->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t\t; Refresh\n", ((ailsa_data_s *)s->head->next->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t\t; Retry\n", ((ailsa_data_s *)s->head->next->next->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t\t; Expire\n", ((ailsa_data_s *)s->head->next->next->next->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t\t); TTL\n;\n", ((ailsa_data_s *)s->head->data)->data->number);
	if (pri[plen - 1] != '.')
		dprintf(fd, "\tIN\tNS\t%s.\n", pri);
	else
		dprintf(fd, "\tIN\tNS\t%s\n", pri);
	if (sec[slen - 1] != '.')
		dprintf(fd, "\tIN\tNS\t%s.\n", sec);
	else
		dprintf(fd, "\tIN\tNS\t%s\n", sec);
}

static void
write_fwd_header_records(int fd, AILLIST *r, char *zone)
{
	if (!(r) || !(zone) || (fd == 0))
		return;
	char *dest, *proto, *service, *host;
	size_t slen, len = 6;
	int retval;
	unsigned int port;
	unsigned long int pri;
	if ((r->total % len) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number of data objects in SOA records list: %zu", r->total);
		return;
	}
	AILELEM *e = r->head;
	while(e) {
		if (strcmp(((ailsa_data_s *)e->data)->data->text, "NS") != 0) {
			e = ailsa_move_down_list(e, len);
		} else {
			e = ailsa_move_down_list(e, len - 1);
			dprintf(fd, "\tIN\tNS\t%s\n", ((ailsa_data_s *)e->data)->data->text);
			e = ailsa_move_down_list(e, 1);
		}
	}
	e = r->head;
	while (e) {
		if (strcmp(((ailsa_data_s *)e->data)->data->text, "MX") != 0) {
			e = ailsa_move_down_list(e, len);
		} else {
			e = ailsa_move_down_list(e, len - 2);
			dprintf(fd, "\tIN\tMX\t%lu\t%s\n", ((ailsa_data_s *)e->data)->data->number,
			  ((ailsa_data_s *)e->next->data)->data->text);
			e = ailsa_move_down_list(e, 2);
		}
	}
	e = r->head;
	while (e) {
		if (strcmp(((ailsa_data_s *)e->data)->data->text, "SRV") != 0) {
			e = ailsa_move_down_list(e, len);
		} else {
			proto = ((ailsa_data_s *)e->next->next->data)->data->text;
			service = ((ailsa_data_s *)e->next->next->next->data)->data->text;
			pri = ((ailsa_data_s *)e->next->next->next->next->data)->data->number;
			host = ((ailsa_data_s *)e->next->data)->data->text;
			if ((retval = cmdb_get_port_number(proto, service, &port)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot get port number");
				break;
			}
			dest = ((ailsa_data_s *)e->next->next->next->next->next->data)->data->text;
			slen = strlen(dest);
			if (dest[slen - 1] != '.') {
				dprintf(fd, "_%s._%s.%s\tIN SRV %lu 0 %u\t%s.%s.\n", host, proto, zone, pri, port, dest, zone);
			} else {
				dprintf(fd, "_%s._%s.%s\tIN SRV %lu 0 %u\t%s\n", host, proto, zone, pri, port, dest);
			}
			e = ailsa_move_down_list(e, len);
		}
	}
}

static void
write_fwd_records(int fd, AILLIST *r, char *zone)
{
	if (!(r) || !(zone) || (fd == 0))
		return;
	size_t len = 3, hlen;
	char *type, *host, *dest;
	AILELEM *e;

	if ((r->total % len) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number of data objects in records list: %zu", r->total);
		return;
	}
	e = r->head;
	while (e) {
		type = ((ailsa_data_s *)e->data)->data->text;
		host = ((ailsa_data_s *)e->next->data)->data->text;
		dest = ((ailsa_data_s *)e->next->next->data)->data->text;
		hlen = strlen(host);
		if (hlen < 8)
			dprintf(fd, "%s\t\tIN\t%s\t%s\n", host, type, dest);
		else 
			dprintf(fd, "%s\tIN\t%s\t%s\n", host, type, dest);
		e = ailsa_move_down_list(e, len);
	}
}

static int
write_glue_records(ailsa_cmdb_s *cbc, int fd, AILLIST *g, const char *zone)
{
// This function could return void if we do not allow non-FQDN's in NS records
	if (!(cbc) || (fd == 0) || !(g) || !(zone))
		return AILSA_NO_DATA;
	int retval = 0;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	size_t len = 3;
	char *name, *pri, *sec;
	AILELEM *e;

	if ((g->total % len) != 0) {
		ailsa_syslog(LOG_ERR, "List contains wrong factor: want %zu got total of %zu", len, g->total);
		goto cleanup;
	}
	e = g->head;
	while (e) {
		name = ((ailsa_data_s *)e->data)->data->text;
		pri = ((ailsa_data_s *)e->next->data)->data->text;
		sec = ((ailsa_data_s *)e->next->next->data)->data->text;
		dprintf(fd, "%s\tIN\tNS\t%s\n", name, pri);
		if (sec)
			if ((strlen(sec) > 0) && (strcmp(sec, "none") != 0))
				dprintf(fd, "%s\tIN\tNS\t%s\n", name, sec);
// At this point, we should check if the NS records are FQDNs. Alternatively,
// do not allow non FQDN records
		e = ailsa_move_down_list(e, len);
	}
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		return retval;
}

static int
cmdb_check_zone(ailsa_cmdb_s *cbs, char *zone)
{
	if (!(cbs) || !(zone))
		return AILSA_NO_DATA;
	int retval;
	char command[CONFIG_LEN];

	memset(command, 0, CONFIG_LEN);
	snprintf(command, CONFIG_LEN, "%s %s %s%s", cbs->chkz, zone, cbs->dir, zone);
	if (system(command) != 0)
		retval = CHKZONE_FAIL;
	else
		retval = 0;
	return retval;
}

static int
cmdb_validate_rev_zone(ailsa_cmdb_s *cbc, char *zone)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	int retval;

	if ((retval = write_rev_zone_file(cbc, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write zone file for zone %s", zone);
		goto cleanup;
	}
	if ((retval = cmdb_check_zone(cbc, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Zone %s could not be validated");
		if ((retval = cmdb_add_string_to_list("no", l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add invalid to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("yes", l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add valid to list");
			goto cleanup;
		}
	}
	if ((retval = cmdb_add_string_to_list(zone, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add rev zone name to list");
		goto cleanup;
	}
	if ((retval = ailsa_update_query(cbc, update_queries[REV_ZONE_VALIDATE], l)) != 0)
		ailsa_syslog(LOG_ERR, "REV_ZONE_VALIDATE update query failed");
	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static int
write_rev_zone_file(ailsa_cmdb_s *cbc, char *zone)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	char *name = ailsa_calloc(DOMAIN_LEN, "name in write_rev_zone_file");
	int retval, flags, fd;
	mode_t um, mask;
	AILLIST *a = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILLIST *s = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(zone, a)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone name to argument list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, REV_SOA_ON_NET_RANGE, a, s)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_SOA_ON_NET_RANGE query failed");
		goto cleanup;
	}
	if ((retval = ailsa_check_for_rev_zone_update(cbc, s, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot check or set zone updated");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, REV_RECORDS_ON_NET_RANGE, a, r)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_RECORDS_ON_NET_RANGE query failed");
		goto cleanup;
	}
	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((snprintf(name, DOMAIN_LEN, "%s%s", cbc->dir, zone)) >= DOMAIN_LEN)
		ailsa_syslog(LOG_INFO, "path truncated in write_rev_zone_file");
	if ((fd = open(name, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open zone file for writing: %s", strerror(errno));
		retval = FILE_O_FAIL;
		goto cleanup;
	}
	write_rev_zone_header(fd, s, cbc->hostmaster);
	write_rev_zone_records(fd, r);
	close(fd);
	mask = umask(um);
	cleanup:
		ailsa_list_full_clean(a);
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(s);
		my_free(name);
		return retval;
}

static void
write_rev_zone_header(int fd, AILLIST *soa, char *hostmaster)
{
	if (!(soa) || !(hostmaster) || (fd == 0))
		return;
	if (soa->total != 8) {
		ailsa_syslog(LOG_ERR, "Wrong number in soa list in write_rev_zone_header: %zu", soa->total);
		return;
	}
	char *pri = ((ailsa_data_s *)soa->head->next->data)->data->text;
	char *sec = ((ailsa_data_s *)soa->head->next->next->data)->data->text;
	size_t plen = strlen(pri);
	size_t slen = strlen(sec);

	dprintf(fd, "$TTL %lu\n", ((ailsa_data_s *)soa->head->data)->data->number);
	if (pri[plen - 1] != '.')
		dprintf(fd, "@\tIN\tSOA\t%s.\t%s (\n", pri, hostmaster);
	else
		dprintf(fd, "@\tIN\tSOA\t%s\t%s (\n", pri, hostmaster);
	dprintf(fd, "\t\t\t\t%lu\t; Serial\n", ((ailsa_data_s *)soa->head->next->next->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t; Refresh\n",
		((ailsa_data_s *)soa->head->next->next->next->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t; Retry\n",
		((ailsa_data_s *)soa->head->next->next->next->next->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t; Expire\n",
		((ailsa_data_s *)soa->head->next->next->next->next->next->next->data)->data->number);
	dprintf(fd, "\t\t\t\t%lu\t); Cache TTL\n",
		((ailsa_data_s *)soa->head->data)->data->number);
	dprintf(fd, ";\n");
	if (pri[plen - 1] != '.')
		dprintf(fd, "\t\tIN\tNS\t%s.\n", pri);
	else
		dprintf(fd, "\t\tIN\tNS\t%s\n", pri);
	if (sec[slen - 1] != '.')
		dprintf(fd, "\t\tIN\tNS\t%s.\n", sec);
	else
		dprintf(fd, "\t\tIN\tNS\t%s\n", sec);
}

static void
write_rev_zone_records(int fd, AILLIST *soa)
{
	if (!(soa) || (fd == 0))
		return;
	char *host;
	char *dest;
	AILELEM *e;
	ailsa_data_s *d;

	if ((soa->total % 2) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number of elements in list: %zu", soa->total);
		return;
	}
	e = soa->head;
	while (e) {
		d = e->data;
		host = d->data->text;
		e = e->next;
		d = e->data;
		dest = d->data->text;
		dprintf(fd, "%s\tPTR\t%s\n", host, dest);
		e = e->next;
	}
}

int
cmdb_get_port_number(char *proto, char *service, unsigned int *port)
{
	if (!(proto) || !(service) || !(port))
		return AILSA_NO_DATA;
	int retval = 0;
	struct servent *res = getservbyname(service, proto);

	if (!(res)) {
		ailsa_syslog(LOG_ERR, "%s", strerror(errno));
		retval = -1;
	} else {
		*port = ntohs((uint16_t)res->s_port);
	}
	return retval;
}

// Should re-write this to use a linked list AILLIST
int
get_port_number(record_row_s *rec, char *name, unsigned short int *port)
{
	char *host;
	int retval = 0;
	size_t len;
	struct addrinfo hints, *srvinfo;
	struct sockaddr_in *ipv4;

	if (!(host = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "host in get_port_number");
	len = strlen(rec->dest);
	if (rec->dest[len - 1] == '.')
		snprintf(host, RBUFF_S, "%s", rec->dest);
	else
		snprintf(host, TBUFF_S, "%s.%s", rec->dest, name);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	if ((strncmp(rec->protocol, "tcp", COMM_S)) == 0)
		hints.ai_socktype = SOCK_STREAM;
	else if ((strncmp(rec->protocol, "udp", COMM_S)) == 0)
		hints.ai_socktype = SOCK_DGRAM;
	else {
		fprintf(stderr, "Unknown protocol type %s in %s\n",
		 rec->protocol, rec->host);
		free(host);
		return WRONG_PROTO;
	}
	hints.ai_flags = AI_PASSIVE;
	if ((retval = getaddrinfo(host, rec->service, &hints, &srvinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
		return retval;
	}
	if (srvinfo->ai_family == AF_INET) {
		ipv4 = (struct sockaddr_in *)srvinfo->ai_addr;
		*port = (unsigned short int) htons((uint16_t)ipv4->sin_port);
	} else {
		retval = WRONG_PROTO;
	}
	free(host);
	freeaddrinfo(srvinfo);
	return retval;
}

unsigned long int
generate_zone_serial(void)
{
        time_t now;
        struct tm *lctime;
        char sday[SERVICE_LEN], smonth[SERVICE_LEN], syear[SERVICE_LEN], sserial[HOST_LEN];
        unsigned long int serial;

        now = time(0);
        if (!(lctime = localtime(&now)))
                report_error(GET_TIME_FAILED, strerror(errno));
        snprintf(syear, SERVICE_LEN, "%d", lctime->tm_year + 1900);
        if (lctime->tm_mon < 9)
                snprintf(smonth, SERVICE_LEN, "0%d", lctime->tm_mon + 1);
        else
                snprintf(smonth, SERVICE_LEN, "%d", lctime->tm_mon + 1);
        if (lctime->tm_mday < 10)
                snprintf(sday, SERVICE_LEN, "0%d", lctime->tm_mday);
        else
                snprintf(sday, SERVICE_LEN, "%d", lctime->tm_mday);
        snprintf(sserial, HOST_LEN, "%s%s%s01",
                 syear,
                 smonth,
                 sday);
        serial = strtoul(sserial, NULL, 10);
        return serial;
}

int
cmdb_write_fwd_zone_config(ailsa_cmdb_s *cbs)
{
	if (!(cbs))
		return AILSA_NO_DATA;
	int retval, flags, fd;
	int t = 0;
	size_t len = 4;
	char *name, *type, *sec, *master;
	char *filename = ailsa_calloc(DOMAIN_LEN, "filename in cmdb_write_fwd_zone_config");
	char *ip = ailsa_calloc(INET6_ADDRSTRLEN, "ip in cmdb_write_fwd_zone_config");
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;
	mode_t um, mask;

	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((snprintf(filename, DOMAIN_LEN, "%s%s", cbs->bind, cbs->dnsa)) >= DOMAIN_LEN)
		ailsa_syslog(LOG_INFO, "Path truncated in cmdb_write_fwd_zone_config");
	if ((fd = open(filename, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "%s", strerror(errno));
		return FILE_O_FAIL;
	}
	if ((retval = ailsa_basic_query(cbs, FWD_ZONE_CONFIG, l)) != 0) {
		ailsa_syslog(LOG_ERR, "FWD_ZONE_CONFIG query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "No forward zones in the database?");
		goto cleanup;
	} else if ((l->total % len) != 0) {
		ailsa_syslog(LOG_ERR, "Incorrect number of elements in list: %zu", l->total);
		goto cleanup;
	}
	e = l->head;
	while (e) {
		name = ((ailsa_data_s *)e->data)->data->text;
		type = ((ailsa_data_s *)e->next->data)->data->text;
		sec = ((ailsa_data_s *)e->next->next->data)->data->text;
		master = ((ailsa_data_s *)e->next->next->next->data)->data->text;
		dprintf(fd, "\
zone \"%s\" {\n\
\t\t\ttype %s;\n\
\t\t\tfile \"%s%s\";\n", name, type, cbs->dir, name);
		if (strcmp(type, "master") == 0) {
			if ((retval = cmdb_getaddrinfo(sec, ip, &t)) != 0)
				goto cleanup;
			if (t == 4)
				dprintf(fd, "\t\t\tnotify-source %s;\n\t\t};\n", ip);
			else if (t == 6)
				dprintf(fd, "\t\t\tnotify-source-v6 %s\n\t\t};\n", ip);
			memset(ip, 0, INET6_ADDRSTRLEN);
		} else if (strcmp(type, "slave") == 0) {
			dprintf(fd, "\t\t\tmasters { %s; };\n\t\t};\n", master);
		}
		e = ailsa_move_down_list(e, len);
		
	}
	close(fd);
	mask = umask(um);
	cleanup:
		my_free(ip);
		my_free(filename);
		ailsa_list_full_clean(l);
		return retval;
}

int
cmdb_write_rev_zone_config(ailsa_cmdb_s *cbs)
{
	if (!(cbs))
		return AILSA_NO_DATA;
	char *filename = ailsa_calloc(DOMAIN_LEN, "filename in cmdb_write_rev_zone_config");
	char *ip = ailsa_calloc(INET6_ADDRSTRLEN, "ip in cmdb_write_rev_zone_config");
	char *master_ip = ailsa_calloc(INET6_ADDRSTRLEN, "master_ip in cmdb_write_rev_zone_config");
	char *type, *range, *master;
	int retval, flags, iptype;
	int fd = 0;
	size_t total = 6;
	unsigned long int prefix;
	mode_t um, mask;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = ailsa_basic_query(cbs, REV_ZONE_CONFIG, l)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_ZONE_CONFIG query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "No reverse zones found in the database");
		goto cleanup;
	} else if ((l->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong factor. Expected 4 got total of %zu", l->total);
		goto cleanup;
	}
	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((snprintf(filename, DOMAIN_LEN, "%s%s", cbs->bind, cbs->rev)) >= DOMAIN_LEN)
		ailsa_syslog(LOG_INFO, "Path truncated in cmdb_write_rev_zone_config");
	if ((fd = open(filename, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "%s", strerror(errno));
		return FILE_O_FAIL;
	}
	e = l->head;
	while (e) {
		memset(ip, 0, INET6_ADDRSTRLEN);
		memset(master_ip, 0, INET6_ADDRSTRLEN);
		type = ((ailsa_data_s *)e->data)->data->text;
		range = ((ailsa_data_s *)e->next->data)->data->text;
		master = ((ailsa_data_s *)e->next->next->next->next->data)->data->text;
		prefix = strtoul(((ailsa_data_s *)e->next->next->next->next->next->data)->data->text, NULL, 10);
		get_in_addr_string(ip, range, prefix);
		if (strcmp(type, "master") == 0) {
			dprintf(fd, "\
zone \"%s\" {\n\
\t\t\ttype master;\n\
\t\t\tfile \"%s%s\";\n\
\t\t};\n", ip, cbs->dir, range);
		} else if (strcmp(type, "slave") == 0) {
			if ((retval = cmdb_getaddrinfo(master, master_ip, &iptype)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot get IP address for name %s", master);
				goto cleanup;
			}
			dprintf(fd, "\
zone \"%s\" {\n\
\t\t\ttype slave;\n\
\t\t\tmasters { %s; };\n\
\t\t\tfile \"%s%s\";\n\
\t\t};\n", ip, master_ip, cbs->dir, range);
		}
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		if (fd > 0) {
			close(fd);
			mask = umask(um);
		}
		ailsa_list_full_clean(l);
		my_free(filename);
		my_free(ip);
		my_free(master_ip);
		return retval;
}

int
cmdb_getaddrinfo(char *name, char *ip, int *type)
{
	if (!(name) || !(ip) || !(type))
		return AILSA_NO_DATA;
	int retval = 0;
	const char *service = "domain";
	struct addrinfo hints, *p;
	struct addrinfo *results = NULL;
	void *addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if ((retval = getaddrinfo(name, service, &hints, &results)) != 0) {
		ailsa_syslog(LOG_ERR, "getaddrinfo failed in cmdb_getaddrinfo: %s", gai_strerror(retval));
		retval = NAME_TO_IP_FAIL;
		goto cleanup;
	}
	for (p = results; p != NULL; p = p->ai_next) {
		if (p->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			*type = 4;
			if (!(inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN))) {
				ailsa_syslog(LOG_INFO, "Cannot convert %s to IP: %s", name, strerror(errno));
				retval = NAME_TO_IP_FAIL;
				goto cleanup;
			}
			break;
		} else if (p->ai_family == AF_INET6) {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			*type = 6;
			if (!(inet_ntop(AF_INET6, addr, ip, INET6_ADDRSTRLEN))) {
				ailsa_syslog(LOG_INFO, "Cannot convert %s to IP: %s", name, strerror(errno));
				retval = NAME_TO_IP_FAIL;
				goto cleanup;
			}
			break;
		} else {
			ailsa_syslog(LOG_ERR, "Wrong protocol in cmdb_getaddrinfo");
			goto cleanup;
		}
	}
	cleanup:
		if (results)
			freeaddrinfo(results);
		return retval;
}

void
get_in_addr_string(char *in_addr, char range[], unsigned long int prefix)
{
	size_t len;
	char *tmp, *line, *classless;
	char louisa[] = ".in-addr.arpa";
	int c, i;

	c = '.';
	i = 0;
	tmp = 0;
	len = strlen(range);
	len++;/* Got to remember the terminating \0 :) */
	if (!(line = calloc(len, sizeof(char))))
		report_error(MALLOC_FAIL, "line in get_in_addr_string");
	if (!(classless = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "classless in get_in_addr_string");

	snprintf(line, len, "%s", range);
	if (prefix == 24) {
		tmp = strrchr(line, c);
		*tmp = '\0';
		while ((tmp = strrchr(line, c))) {
			++tmp;
			len = strlen(tmp);
			strncat(in_addr, tmp, len);
			strcat(in_addr, ".");
			--tmp;
			*tmp = '\0';
			i++;
		}
	} else if (prefix == 16) {
		tmp = strrchr(line, c);
		*tmp = '\0';
		tmp = strrchr(line, c);
		*tmp = '\0';
		while ((tmp = strrchr(line, c))) {
			++tmp;
			len = strlen(tmp);
			strncat(in_addr, tmp, len);
			strcat(in_addr, ".");
			--tmp;
			*tmp = '\0';
			i++;
		}
	} else if(prefix == 8) {
		tmp = strrchr(line, c);
		*tmp = '\0';
		tmp = strrchr(line, c);
		*tmp = '\0';
		tmp = strrchr(line, c);
		*tmp = '\0';
		while ((tmp = strrchr(line, c))) {
			++tmp;
			len = strlen(tmp);
			strncat(in_addr, tmp, len);
			strcat(in_addr, ".");
			--tmp;
			*tmp = '\0';
			i++;
		}
	} else if (prefix == 25 || prefix == 26 || prefix == 27 || 
		prefix == 28 || prefix == 29 || prefix == 30 ||
		prefix == 31 || prefix == 32) {
		tmp = strrchr(line, c);
		++tmp;
		len = strlen(tmp);
		strncat(in_addr, tmp, len);
		strcat(in_addr, ".");
		--tmp;
		*tmp = '\0';
		snprintf(classless, CONF_S, "/%lu.", prefix);
		len = strlen(classless);
		strncat(in_addr, classless, len);
		while ((tmp = strrchr(line, c))) {
			++tmp;
			len = strlen(tmp);
			strncat(in_addr, tmp, len);
			strcat(in_addr, ".");
			--tmp;
			*tmp = '\0';
			i++;
		}
	}
	len = strlen(line);
	strncat(in_addr, line, len);
	len = strlen(louisa);
	strncat(in_addr, louisa, len);
	free(line);
	free(classless);
}
