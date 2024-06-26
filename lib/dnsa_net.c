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
#include <math.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <errno.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb_dnsa.h"


/*
 * Temporary variables while I work out how to define these in the
 * database
 */

const char *fed_tld = "/sites/dl.fedoraproject.org/pub/fedora/linux";
const char *fed_boot = "/isolinux";
const char *deb_i386_boot = "/main/installer-i386/current/images/netboot/debian-installer/i386";
const char *deb_amd64_boot = "/main/installer-amd64/current/images/netboot/debian-installer/amd64";
const char *ubu_i386_boot = "/main/installer-i386/current/images/netboot/ubuntu-installer/i386";
const char *ubu_amd64_boot = "/main/installer-amd64/current/images/netboot/ubuntu-installer/amd64";
const char *ubu_new_amd64_boot = "/main/installer-amd64/current/legacy-images/netboot/ubuntu-installer/amd64";

static int
write_fwd_zone_file(ailsa_cmdb_s *cbc, char *zone);

static int
write_rev_zone_file(ailsa_cmdb_s *cbc, char *zone, unsigned long int prefix, unsigned long int index);

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
cmdb_validate_fwd_zone(ailsa_cmdb_s *cbc, char *zone, const char *ztype);

static int
cmdb_validate_rev_zone(ailsa_cmdb_s *cbc, char *zone, const char *ztype, unsigned long int prefix);

static int
cmdb_check_zone(ailsa_cmdb_s *cbs, char *zone);

static int
write_glue_records(ailsa_cmdb_s *cbc, int fd, AILLIST *g, const char *zone);

static void
write_rev_zone_header(int fd, AILLIST *soa, char *hostmaster);

static void
write_rev_zone_records(int fd, AILLIST *soa);

static void
fill_addrtcp(struct addrinfo *c);

static int
decode_http_header(FILE *rx, unsigned long int *len);

unsigned long int
get_net_range(unsigned long int prefix)
{
        unsigned long int range;
        range = 4294967295UL;
        range = (range >> prefix) + 1;
        return range;
}

int
cmdb_validate_zone(ailsa_cmdb_s *cbc, int type, char *zone, const char *ztype, unsigned long int prefix)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	int retval;

	switch(type) {
	case FORWARD_ZONE:
		if ((retval = cmdb_validate_fwd_zone(cbc, zone, ztype)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate forward zone");
			return retval;
		}
		break;
	case REVERSE_ZONE:
		if ((retval = cmdb_validate_rev_zone(cbc, zone, ztype, prefix)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate reverse zone");
			return retval;
		}
		break;
	default:
		ailsa_syslog(LOG_ERR, "Unknown zone type %s", type);
		retval = AILSA_WRONG_ZONE_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_validate_fwd_zone(ailsa_cmdb_s *cbc, char *zone, const char *ztype)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	char *command = ailsa_calloc(CONFIG_LEN, "command in cmdb_validate_fwd_zone");
	int retval;

	if (ztype) {
		if (strncmp(ztype, "slave", BYTE_LEN) == 0) {
			if ((retval = cmdb_add_string_to_list("yes", l)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot add invalid to list");
				goto cleanup;
			}
			goto validate;
		}
	}
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
	validate:
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
		retval = AILSA_FILE_ERROR;
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
		return AILSA_WRONG_LIST_LENGHT;
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
		return AILSA_WRONG_LIST_LENGHT;
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
	if (strncmp(sec, "none", 4) != 0) {
		if (sec[slen - 1] != '.')
			dprintf(fd, "\tIN\tNS\t%s.\n", sec);
		else
			dprintf(fd, "\tIN\tNS\t%s\n", sec);
	}
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
				dprintf(fd, "_%s._%s.%s.\tIN SRV %lu 0 %u\t%s.%s.\n", host, proto, zone, pri, port, dest, zone);
			} else {
				dprintf(fd, "_%s._%s.%s.\tIN SRV %lu 0 %u\t%s\n", host, proto, zone, pri, port, dest);
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
		dprintf(fd, "%s.\tIN\tNS\t%s\n", name, pri);
		if (sec)
			if ((strlen(sec) > 0) && (strcmp(sec, "none") != 0))
				dprintf(fd, "%s.\tIN\tNS\t%s\n", name, sec);
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
		retval = AILSA_CHKZONE_FAIL;
	else
		retval = 0;
	return retval;
}

static int
cmdb_validate_rev_zone(ailsa_cmdb_s *cbc, char *zone, const char *ztype, unsigned long int prefix)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	AILLIST *l = ailsa_db_data_list_init();
	char addr[MAC_LEN];
	int retval;
	unsigned long int index, i;
	if (ztype) {
		if (strncmp(ztype, "slave", BYTE_LEN) == 0) {
			if ((retval = cmdb_add_string_to_list("yes", l)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot add valid to list");
				goto cleanup;
			}
			goto validate;
		}
	}
	if ((retval = get_zone_index(prefix, &index)) != 0)
		goto cleanup;
	for (i = 0; i < index; i++) {
		memset(addr, 0, MAC_LEN);
		if ((retval = get_offset_ip(zone, addr, prefix, i)) != 0)
			goto cleanup;
		if ((retval = write_rev_zone_file(cbc, zone, prefix, i)) != 0)
			goto cleanup;
		if ((retval = cmdb_check_zone(cbc, addr)) != 0) {
			if ((retval = cmdb_add_string_to_list("no", l)) != 0)
				goto cleanup;
			goto validate;
		}
	}
	if ((retval = cmdb_add_string_to_list("yes", l)) != 0)
		goto cleanup;

	validate:
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
write_rev_zone_file(ailsa_cmdb_s *cbc, char *zone, unsigned long int prefix, unsigned long int index)
{
	if (!(cbc) || !(zone))
		return AILSA_NO_DATA;
	char *name = ailsa_calloc(DOMAIN_LEN, "name in write_rev_zone_file");
	char *ip = ailsa_calloc(MAC_LEN, "ip in write_rev_zone_file");
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
	if ((retval = cmdb_add_number_to_list(index, a)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(cbc, REV_RECORDS_ON_NET_RANGE, a, r)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_RECORDS_ON_NET_RANGE query failed");
		goto cleanup;
	}
	if ((retval = get_offset_ip(zone, ip, prefix, index)) != 0)
		goto cleanup;
	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((snprintf(name, DOMAIN_LEN, "%s%s", cbc->dir, ip)) >= DOMAIN_LEN)
		ailsa_syslog(LOG_INFO, "path truncated in write_rev_zone_file");
	if ((fd = open(name, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open zone file for writing: %s", strerror(errno));
		retval = AILSA_FILE_ERROR;
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
		my_free(ip);
		return retval;
}

int
dnsa_populate_rev_zone(ailsa_cmdb_s *cbc, char *range, char *master, unsigned long int prefix, AILLIST *list)
{
	if (!(cbc) || !(range) || (prefix == 0) || !(list))
		return AILSA_NO_DATA;
	char buff[CONFIG_LEN];
	int retval;
	unsigned long int start, end;

	memset(buff, 0, CONFIG_LEN);
	if ((retval = cmdb_add_string_to_list(range, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add net_range to list");
		goto cleanup;
	}
	snprintf(buff, CONFIG_LEN, "%lu", prefix);
	if ((retval = cmdb_add_string_to_list(buff, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add prefix to list");
		goto cleanup;
	}
	memset(buff, 0, CONFIG_LEN);
	if ((retval = cmdb_add_string_to_list(range, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add net_start to list");
		goto cleanup;
	}
	if ((retval = get_start_finsh_ips(range, prefix, &start, &end)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get start and end IP's");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(start, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add start_ip to list");
		goto cleanup;
	}
	if ((retval = convert_bin_ipv4_to_text(end, buff)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot convert end IP to text");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(buff, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add net_finish to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_number_to_list(end, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add finish_ip to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cbc->prins, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add primary nameserver to list");
		goto cleanup;
	}
	if (cbc->secns) {
		if ((retval = cmdb_add_string_to_list(cbc->secns, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add secondary nameserver to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("none", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add none nameserver to list");
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
	if (!(master)) {
		if ((retval = cmdb_add_string_to_list("master", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone type to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list("NULL", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add NULL master to list");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("slave", list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone type to list");
			goto cleanup;
		}
		if ((retval = cmdb_add_string_to_list(master, list)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add master to list");
			goto cleanup;
		}
	}
	if ((retval = cmdb_add_string_to_list("no", list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add valid to zone list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(list)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
	cleanup:
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
	if (sec) {
		if (sec[slen - 1] != '.')
			dprintf(fd, "\t\tIN\tNS\t%s.\n", sec);
		else
			dprintf(fd, "\t\tIN\tNS\t%s\n", sec);
	}
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

unsigned long int
generate_zone_serial(void)
{
        time_t now;
        struct tm *lctime;
        char sday[SERVICE_LEN], smonth[SERVICE_LEN], syear[SERVICE_LEN], sserial[HOST_LEN];
        unsigned long int serial;

        now = time(0);
        if (!(lctime = localtime(&now))) {
		ailsa_syslog(LOG_ERR, "Get time failed");
		return 0;
	}
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
		return AILSA_FILE_ERROR;
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
		if ((strcmp(type, "master") == 0) && (strcmp(sec, "none") != 0)) {
			if ((retval = cmdb_getaddrinfo(sec, ip, &t)) != 0)
				goto cleanup;
			if (t == 4)
				dprintf(fd, "\t\t\tnotify-source %s;\n\t\t};\n", ip);
			else if (t == 6)
				dprintf(fd, "\t\t\tnotify-source-v6 %s\n\t\t};\n", ip);
			memset(ip, 0, INET6_ADDRSTRLEN);
		} else if (strcmp(type, "slave") == 0) {
			dprintf(fd, "\t\t\tmasters { %s; };\n\t\t};\n", master);
		} else {
			dprintf(fd, "\t\t};\n");
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
	int retval, flags;
	int fd = 0;
	mode_t um, mask;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *z = ailsa_rev_zone_list_init();
	AILELEM *e;
	ailsa_rev_zone_s *r;

	if ((retval = ailsa_basic_query(cbs, REV_ZONE_CONFIG, l)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_ZONE_CONFIG query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "No reverse zones found in DB");
		goto cleanup;
	}
	if ((retval = ailsa_fill_rev_zone_list(l, z)) != 0)
		goto cleanup;
	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((snprintf(filename, DOMAIN_LEN, "%s%s", cbs->bind, cbs->rev)) >= DOMAIN_LEN)
		ailsa_syslog(LOG_INFO, "Path truncated in cmdb_write_rev_zone_config");
	if ((fd = open(filename, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "%s", strerror(errno));
		return AILSA_FILE_ERROR;
	}
	e = z->head;
	while (e) {
		r = e->data;
		dprintf(fd, "\
zone \"%s\" {\n", r->in_addr);
		if (strcmp(r->type, "master") == 0) {
			dprintf(fd, "\
\t\t\ttype master;\n");
		} else if (strcmp(r->type, "slave") == 0) {
			dprintf(fd, "\
\t\t\ttype slave;\n\
\t\t\tmasters { %s; };\n", r->master);
		}
		dprintf(fd, "\
\t\t\tfile \"%s%s\";\n\
\t\t};\n", cbs->dir, r->net_range);
		e = e->next;
	}
	cleanup:
		if (fd > 0) {
			close(fd);
			mask = umask(um);
		}
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(z);
		my_free(filename);
		return retval;
}

int
ailsa_fill_rev_zone_list(AILLIST *l, AILLIST *z)
{
	if (!(l) || !(z))
		return AILSA_NO_DATA;
	int retval;
	char *range;
	size_t total = 6;
	AILELEM *e;
	ailsa_rev_zone_s *rev;
	unsigned long int prefix, index, i;

	if ((l->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong factor. Expected 6 got total of %zu", l->total);
		return AILSA_WRONG_LIST_LENGHT;
	}
	e = l->head;
	while (e) {
		prefix = strtoul(((ailsa_data_s *)e->next->next->next->next->next->data)->data->text, NULL, 10);
		if ((prefix != 8) && (prefix != 16) && (prefix != 24)) {
			if ((retval = get_zone_index(prefix, &index)) != 0)
				return retval;
			for (i = 0; i < index; i++) {
				rev = ailsa_calloc(sizeof(ailsa_rev_zone_s), "rev in ailsa_fill_rev_zone");
				rev->type = strndup(((ailsa_data_s *)e->data)->data->text, SERVICE_LEN);
				rev->master = strndup(((ailsa_data_s *)e->next->next->next->next->data)->data->text, DOMAIN_LEN);
				rev->net_range = ailsa_calloc(MAC_LEN, "rev->net_range in ailsa_fill_rev_zone");
				range = ((ailsa_data_s *)e->next->data)->data->text;
				if ((retval = get_offset_ip(range, rev->net_range, prefix, i)) != 0) {
					ailsa_syslog(LOG_ERR, "Cannot get net_range");
					if (ailsa_list_insert(z, rev) != 0)
						ailsa_clean_rev_zone(rev);
					return retval;
				}
				rev->in_addr = ailsa_calloc(HOST_LEN, "rev->in_addr in ailsa_fill_rev_zone_list");
				get_in_addr_string(rev->in_addr, rev->net_range, prefix);
				if ((retval = ailsa_list_insert(z, rev)) != 0) {
					ailsa_syslog(LOG_ERR, "Cannot add rev into list z");
					ailsa_clean_rev_zone(rev);
					return retval;
				}
			}
		} else {
			rev = ailsa_calloc(sizeof(ailsa_rev_zone_s), "rev in ailsa_fill_rev_zone");
			rev->type = strndup(((ailsa_data_s *)e->data)->data->text, SERVICE_LEN);
			if (strncmp(rev->type, "slave", BYTE_LEN) == 0)
				rev->master = strndup(((ailsa_data_s *)e->next->next->next->next->data)->data->text, DOMAIN_LEN);
			rev->net_range = strndup(((ailsa_data_s *)e->next->data)->data->text, DOMAIN_LEN);
			rev->in_addr = ailsa_calloc(HOST_LEN, "rev->in_addr in ailsa_fill_rev_zone_list");
			get_in_addr_string(rev->in_addr, rev->net_range, prefix);
			if ((retval = ailsa_list_insert(z, rev)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot add rev zone into list z");
				ailsa_clean_rev_zone(rev);
				return retval;
			}
		}
		e = ailsa_move_down_list(e, total);
	}
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
		retval = AILSA_GETADDR_FAIL;
		goto cleanup;
	}
	for (p = results; p != NULL; p = p->ai_next) {
		if (p->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			*type = 4;
			if (!(inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN))) {
				ailsa_syslog(LOG_INFO, "Cannot convert %s to IP: %s", name, strerror(errno));
				retval = AILSA_GETADDR_FAIL;
				goto cleanup;
			}
			break;
		} else if (p->ai_family == AF_INET6) {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			*type = 6;
			if (!(inet_ntop(AF_INET6, addr, ip, INET6_ADDRSTRLEN))) {
				ailsa_syslog(LOG_INFO, "Cannot convert %s to IP: %s", name, strerror(errno));
				retval = AILSA_GETADDR_FAIL;
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

int
ailsa_get_iface_list(AILLIST *list)
{
/* This function completely ignores IPv6 addresses. In fact, many areas of
 * this software collection ignore the existance of IPv6. *sigh*
 */

	if (!(list))
		return AILSA_NO_DATA;
	int retval;
	struct ifaddrs *iface, *p;
	ailsa_iface_s *ice;

	if ((retval = getifaddrs(&iface)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get interface list: %s", strerror(errno));
		goto cleanup;
	}

	p = iface;
	while (p) {
		if (p->ifa_addr->sa_family == AF_INET) {
			ice = ailsa_calloc(sizeof(ailsa_iface_s), "ice in ailsa_get_iface_list");
			ice->name = strndup(p->ifa_name, SERVICE_LEN);
			ice->ip = ntohl(((struct sockaddr_in *)p->ifa_addr)->sin_addr.s_addr);
			ice->nm = ntohl(((struct sockaddr_in *)p->ifa_netmask)->sin_addr.s_addr);
			ice->nw = ice->ip & ice->nm;
			ice->bc = ice->nw | (~ice->nm);
			ice->sip = ice->nw + 1;
			ice->fip = ice->bc - 1;
			if ((retval = ailsa_list_insert(list, ice)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot insert iface into list");
				goto cleanup;
			}
		}
		p = p->ifa_next;
	}
	cleanup:
		if (iface)
			freeifaddrs(iface);
		return retval;
}

char *
get_iface_name(const char *name)
{
	int counter, retval, flag;
	char *iface = NULL;
	ailsa_iface_s *ifce = NULL;
	AILELEM *e;
	AILLIST *ice = ailsa_iface_list_init();

	if ((retval = ailsa_get_iface_list(ice)) != 0)
		return iface;
	iface = ailsa_calloc(SERVICE_LEN, "iface in get_iface_name");
/* This function will NOT find a bridge that is part of an inactive network.
   While the network is inactive, this is probably not an issue. Cannot see
   how to use libvirt to query avaiable bridge interface names.

   This _IS_ an issue, and libvirt fails to define a network with a bridge
   name that is already used. Can use the function 
      char *virNetworkGetBridgeName	(virNetworkPtr network)
   to return the bridge name of a network. I need to loop through all networks
   to check if a bridge name is in use.
   
   However, this function is no longer in use. Instead, call the bridge name
   the name of the network. */
	for (counter = 0; counter < BUFFER_LEN; counter++) {
		flag = false;
		memset(iface, 0, SERVICE_LEN);
		snprintf(iface, SERVICE_LEN, "%s%d", name, counter);
		e = ice->head;
		while (e) {
			ifce = e->data;
			if ((strncmp(iface, ifce->name, SERVICE_LEN)) == 0) {
				flag = true;
				break;
			}
			e = e->next;
		}
		if (flag == false)
			break;
	}
	ailsa_list_full_clean(ice);
	return iface;
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
	line = ailsa_calloc(len, "line in get_in_addr_string");
	classless = ailsa_calloc(CONFIG_LEN, "classless in get_in_addr_string");

	snprintf(line, len, "%s", range);
	if ((prefix <= 24) && (prefix > 16)) {
		tmp = strrchr(line, c);
		*tmp = '\0';
		while ((tmp = strrchr(line, c))) {
			++tmp;
			strncat(in_addr, tmp, SERVICE_LEN);
			strcat(in_addr, ".");
			--tmp;
			*tmp = '\0';
			i++;
		}
	} else if ((prefix <= 16) && (prefix > 8)) {
		tmp = strrchr(line, c);
		*tmp = '\0';
		tmp = strrchr(line, c);
		*tmp = '\0';
		while ((tmp = strrchr(line, c))) {
			++tmp;
			strncat(in_addr, tmp, SERVICE_LEN);
			strcat(in_addr, ".");
			--tmp;
			*tmp = '\0';
			i++;
		}
	} else if (prefix == 8) {
		tmp = strrchr(line, c);
		*tmp = '\0';
		tmp = strrchr(line, c);
		*tmp = '\0';
		tmp = strrchr(line, c);
		*tmp = '\0';
		while ((tmp = strrchr(line, c))) {
			++tmp;
			strncat(in_addr, tmp, SERVICE_LEN);
			strcat(in_addr, ".");
			--tmp;
			*tmp = '\0';
			i++;
		}
	}
	strncat(in_addr, line, SERVICE_LEN);
	strncat(in_addr, louisa, SERVICE_LEN);
	free(line);
	free(classless);
}

int
dnsa_populate_zone(ailsa_cmdb_s *cbs, char *domain, const char *type, const char *master, AILLIST *zone)
{
	if (!(cbs) || !(domain) || !(zone))
		return AILSA_NO_DATA;
	int retval;

	if ((retval = cmdb_add_string_to_list(domain, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add zone name to list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(cbs->prins, zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add primary DNS server to list");
		return retval;
	}
	if (cbs->secns) {
		if ((retval = cmdb_add_string_to_list(cbs->secns, zone)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add secondary DNS server to list");
			return retval;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("none", zone)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add none secondary DNS server to list");
			return retval;
		}
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
	if (master) {
		if ((retval = cmdb_add_string_to_list(type, zone)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone type to list");
			return retval;
		}
		if ((retval = cmdb_add_string_to_list(master, zone)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add master IP address to list");
			return retval;
		}
	} else {
		if ((retval = cmdb_add_string_to_list("master", zone)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add zone type master to list");
			return retval;
		}
	}
	if ((retval = cmdb_populate_cuser_muser(zone)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
		return retval;
	}
	return retval;
}

int
add_forward_zone(ailsa_cmdb_s *dc, char *domain, const char *type, const char *master)
{
	if (!(dc) || !(domain))
		return AILSA_NO_DATA;
	char *command = ailsa_calloc(CONFIG_LEN, "command in add_forward_zone");
	int retval;
	unsigned int query = INSERT_FORWARD_ZONE;
	AILLIST *l = ailsa_db_data_list_init();

	if (type) {
		if ((master) && (strncmp(type, "slave", BYTE_LEN) == 0)) {
			query = INSERT_FORWARD_SLAVE_ZONE;
		} else if (!(master) && (strncmp(type, "slave", BYTE_LEN) == 0)) {
			ailsa_syslog(LOG_ERR, "Trying to insert slave zone with no master IP address");
			retval = AILSA_NO_MASTER;
			goto cleanup;
		}
	}
	if ((retval = cmdb_check_for_fwd_zone(dc, domain, type)) > 0) {
		ailsa_syslog(LOG_INFO, "Zone %s already in database", domain);
		retval = 0;
	} else if (retval == -1) {
		goto cleanup;
	} else {
		if ((retval = dnsa_populate_zone(dc, domain, type, master, l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot populate zone list");
			goto cleanup;
		}
		if ((retval = ailsa_insert_query(dc, query, l)) != 0) {
			ailsa_syslog(LOG_ERR, "INSERT_FORWARD_ZONE query failed");
			goto cleanup;
		}
		if ((retval = cmdb_validate_zone(dc, FORWARD_ZONE, domain, type, 0)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot validate zone %s", domain);
			goto cleanup;
		}
		if ((retval = cmdb_write_fwd_zone_config(dc)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot write forward zones config");
			goto cleanup;
		}
		snprintf(command, CONFIG_LEN, "%s reload", dc->rndc);
		if ((retval = system(command)) != 0)
			ailsa_syslog(LOG_ERR, "Reload of nameserver failed");
	}
	cleanup:
		ailsa_list_full_clean(l);
		my_free(command);
		return retval;
}

int
add_reverse_zone(ailsa_cmdb_s *dc, char *range, const char *type, char *master, unsigned long int prefix)
{

	if (!(dc) || !(range) || (prefix == 0))
		return AILSA_NO_DATA;

	char *command = ailsa_calloc(CONFIG_LEN, "command in add_reverse_zone");
	int retval;
	AILLIST *rev = ailsa_db_data_list_init();
	AILLIST *rid = ailsa_db_data_list_init();
	unsigned long int start, end;

	if ((retval = get_start_finsh_ips(range, prefix, &start, &end)) != 0)
		goto cleanup;
	if ((retval = check_for_rev_zone_overlap(dc, start, end)) != 0) {
		ailsa_syslog(LOG_ERR, "Reverse zone %s overlaps", range);
		goto cleanup;
	}
	if ((retval = dnsa_populate_rev_zone(dc, range, master, prefix, rev)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create list for DB insert");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(dc, INSERT_REVERSE_ZONE, rev)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_REVERSE_ZONE query failed");
		goto cleanup;
	}
	if ((retval = cmdb_validate_zone(dc, REVERSE_ZONE, range, type, prefix)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to validate new zone %s", range);
		goto cleanup;
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
cbc_get_boot_files(ailsa_cmdb_s *cmc, char *os, char *ver, char *arch, char *vail)
{
	int retval = 0;
	int s;
	unsigned long int size;
	size_t len;
	FILE *tx = NULL, *rx = NULL, *krn = NULL, *intrd = NULL;
	char kfile[BUFFER_LEN], infile[BUFFER_LEN];
	char *buff = NULL, *kernel = NULL, *initrd = NULL;
	char *host;
	struct addrinfo h, *r, *p;
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *mirror = ailsa_db_data_list_init();

	r = NULL;
	if (!(kernel = calloc(BUFFER_LEN, 1)))
		goto cleanup;
	if (!(initrd = calloc(BUFFER_LEN, 1)))
		goto cleanup;
	if ((retval = cmdb_add_string_to_list(os, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add os alias to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, MIRROR_ON_BUILD_ALIAS, list, mirror)) != 0) {
		ailsa_syslog(LOG_ERR, "MIRROR_ON_BUILD_ALIAS query failed");
		goto cleanup;
	}
	if (mirror->total > 0) {
		host = ((ailsa_data_s *)mirror->head->data)->data->text;
	} else {
		ailsa_syslog(LOG_ERR, "Nothing back from query MIRROR_ON_BUILD_ALIAS");
		retval = AILSA_NO_MIRROR;
		goto cleanup;
	}
	if (strncmp(os, "debian", BYTE_LEN) == 0) {
		if (strncmp(arch, "i386", BYTE_LEN) == 0) {
			snprintf(kernel, BUFFER_LEN, "GET /debian/dists/%s%s/linux HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, deb_i386_boot, host);
			snprintf(initrd, BUFFER_LEN, "GET /debian/dists/%s%s/initrd.gz HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, deb_i386_boot, host);
		} else if (strncmp(arch, "x86_64", BYTE_LEN) == 0) {
			snprintf(kernel, BUFFER_LEN, "GET /debian/dists/%s%s/linux HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, deb_amd64_boot, host);
			snprintf(initrd, BUFFER_LEN, "GET /debian/dists/%s%s/initrd.gz HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, deb_amd64_boot, host);
		}
	} else if (strncmp(os, "ubuntu", BYTE_LEN) == 0) {
		if (strncmp(arch, "i386", BYTE_LEN) == 0) {
			snprintf(kernel, BUFFER_LEN, "GET /ubuntu/dists/%s%s/linux HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, ubu_i386_boot, host);
			snprintf(initrd, BUFFER_LEN, "GET /ubuntu/dists/%s%s/initrd.gz HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, ubu_i386_boot, host);
		} else if (strncmp(arch, "x86_64", BYTE_LEN) == 0) {
			snprintf(kernel, BUFFER_LEN, "GET /ubuntu/dists/%s%s/linux HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, ubu_amd64_boot, host);
			snprintf(initrd, BUFFER_LEN, "GET /ubuntu/dists/%s%s/initrd.gz HTTP/1.1\r\nHOST: %s\r\n\r\n",
			 vail, ubu_amd64_boot, host);
		}
	} else if (strncmp(os, "centos", BYTE_LEN) == 0) {
		snprintf(kernel, BUFFER_LEN, "GET /centos/%s/os/%s/isolinux/vmlinuz HTTP/1.1\r\nHOST: %s\r\n\r\n",
		 ver, arch, host);
		snprintf(initrd, BUFFER_LEN, "GET /centos/%s/os/%s/isolinux/initrd.img HTTP/1.1\r\nHOST: %s\r\n\r\n",
		 ver, arch, host);
	} else if (strncmp(os, "fedora", BYTE_LEN) == 0) {
		snprintf(kernel, BUFFER_LEN, "GET %s/releases/%s/Server/%s/os/isolinux/vmlinuz HTTP/1.1\r\nHOST: %s\r\n",
		 fed_tld, ver, arch, host);
		snprintf(initrd, BUFFER_LEN, "GET %s/releases/%s/Server/%s/os/isolinux/initrd.img HTTP/1.1\r\nHOST: %s\r\n",
		 fed_tld, ver, arch, host);
	}
	fill_addrtcp(&h);
	if ((retval = getaddrinfo(host, "http", &h, &r)) != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
		goto cleanup;
	}
	for (p = r; p != NULL; p = p->ai_next) {
		if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;
		if (connect(s, p->ai_addr, p->ai_addrlen) == -1)
			continue;
		break;
	}
	if (!(p)) {
		fprintf(stderr, "Cannot connect to host %s\n", host);
		retval = 1;
		goto cleanup;
	}
	if (!(rx = fdopen(s, "r"))) {
		fprintf(stderr, "fdopen(rx): %s\n", strerror(errno));
		goto cleanup;
	}
	if (!(tx = fdopen(dup(s), "w"))) {
		fprintf(stderr, "fdopen(tx): %s\n", strerror(errno));
		goto cleanup;
	}
	snprintf(kfile, CONFIG_LEN, "%s/vmlinuz-%s-%s-%s", cmc->tftpdir, os, ver, arch);
	snprintf(infile, CONFIG_LEN, "%s/initrd-%s-%s-%s.img", cmc->tftpdir, os, ver, arch);
	if ((retval = setvbuf(rx, NULL, _IOLBF, MAXDATASIZE)) != 0) {
		perror("setvbuf: ");
		goto cleanup;
	}
	fprintf(tx, "%s", kernel);
	fflush(tx);
	if ((retval = decode_http_header(rx, &size)) != 0)
		goto cleanup;
	if (size == 0) {
		fprintf(stderr, "Cannot determine incoming file size\n");
		goto cleanup;
	}
	buff = calloc(size, 1);
	fprintf(stderr, "Grabbing Kernel. Size: %lu...\n", size);
	if ((len = fread(buff, 1, size, rx)) != size)
		fprintf(stderr, "Only read %zu bytes of %lu\n", len, size);
	fprintf(stderr, "Got it\n");
	if (!(krn = fopen(kfile, "w"))) {
		fprintf(stderr, "fdopen(kfile): %s\n", strerror(errno));
		goto cleanup;
	}
	if ((size = fwrite(buff, 1, len, krn)) != len)
		fprintf(stderr, "Only wrote %lu bytes of %zu\n", size, len);
	free(buff);
	buff = NULL;
	fprintf(tx, "%s", initrd);
	fflush(tx);
	size = 0;
	if ((retval = decode_http_header(rx, &size)) != 0)
		goto cleanup;
	if (size == 0) {
		fprintf(stderr, "Cannot determine incoming file size\n");
		goto cleanup;
	}
	buff = calloc(size, 1);
	fprintf(stderr, "Grabbing initrd. Size: %lu...\n", size);
	if ((len = fread(buff, 1, size, rx)) != size)
		fprintf(stderr, "Only read %zu bytes of %lu\n", len, size);
	fprintf(stderr, "Got it\n");
	if (!(intrd = fopen(infile, "w"))) {
		fprintf(stderr, "fdopen(intrd): %s\n", strerror(errno));
		goto cleanup;
	}
	if ((size = fwrite(buff, 1, len, intrd)) != len)
		fprintf(stderr, "Only wrote %lu bytes of %zu\n", size, len);
	goto cleanup;

	cleanup:
		if (tx) {
			fclose(tx);
		}
		if (rx)
			fclose(rx);
		if (krn)
			fclose(krn);
		if (intrd)
			fclose(intrd);
		if (buff)
			free(buff);
		if (kernel)
			free(kernel);
		if (initrd)
			free(initrd);
		if (r)
			freeaddrinfo(r);
		ailsa_list_full_clean(list);
		ailsa_list_full_clean(mirror);
		return retval;
}


static void
fill_addrtcp(struct addrinfo *c)
{
	memset(c, 0, sizeof(struct addrinfo));
	c->ai_family = AF_UNSPEC;
	c->ai_socktype = SOCK_STREAM;
	c->ai_flags = AI_PASSIVE;
}

static int
decode_http_header(FILE *rx, unsigned long int *len)
{
	char *buf, *t;
	int retval = AILSA_DOWNLOAD_FAIL;
	unsigned long int code;

	buf = ailsa_calloc(MAXDATASIZE, "buff in decode_http_header");
	if (!(fgets(buf, MAXDATASIZE, rx))) {
		fprintf(stderr, "Cannot read HTTP line\n");
		goto cleanup;
	}
	if (!(t = strtok(buf, " "))) {
		fprintf(stderr, "Cannot tokenise response string\n");
		goto cleanup;
	}
	if (!(t = strtok(NULL, " "))) {
		fprintf(stderr, "Only 1 space in response??\n");
		goto cleanup;
	}
	code = strtoul(t, NULL, 10);
	if (code != 200) {
		fprintf(stderr, "Server response code: %lu\n", code);
		goto cleanup;
	}
	while (fgets(buf, MAXDATASIZE, rx)) {
		if (strncmp(buf, "\r\n", 2) == 0) {
			retval = 0;
			goto cleanup;
		}
		t = strtok(buf, " ");
/*
 * The following does not necessarily have to be provided. The content could
 * be dynamic. Since we are requesting a binary file with a fixed length
 * this should be safe enough. If the file you are requesting is text
 * there is a chance that it could be classified as dynamic and no
 * content length will be provided. In that case you should look for the
 * header:
 *  Transfer-Encoding: chunked
 * In this case, from nginx testing, it seems that after the final '\r\n'
 * of the header, there is the content length of the chunk, on a single line,
 * and encoded in hexadecimal.
 *
 * Full http protocol specification at:
 *
 * http://www.w3.org/Protocols/rfc2616/rfc2616.html
 */
		if (strncmp(t, "Content-Length:", SERVICE_LEN) == 0) {
			if (!(t = strtok(NULL, " "))) {
				fprintf(stderr, "Cannot get length\n");
				goto cleanup;
			}
			*len = strtoul(t, NULL, 10);
		}
	}

	cleanup:
		my_free(buf);
		return retval;
}

int
check_for_build_domain_overlap(ailsa_cmdb_s *cbs, unsigned long int *ips)
{
	if (!(cbs) || !(ips))
		return AILSA_NO_DATA;
	unsigned long int *ip = ips;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();

	while (*ip) {
		if ((retval = cmdb_add_number_to_list(*ip, l)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert first IP address into list");
			return retval;
		}
		ip++;
	}
	if ((retval = ailsa_argument_query(cbs, BUILD_DOMAIN_OVERLAP, l, r)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_OVERLAP query failed");
		goto cleanup;
	}
	if (r->total > 0) {
		ailsa_syslog(LOG_ERR, "Network details for build domain overlap");
		retval = AILSA_BUILD_DOMAIN_OVERLAP;
	}

	cleanup:
		ailsa_list_full_clean(r);
		ailsa_list_full_clean(l);
		return retval;
}

int
check_for_rev_zone_overlap(ailsa_cmdb_s *cbc, unsigned long int start, unsigned long int end)
{
	if (!(cbc) || (start == 0) || (end == 0) || (end < start))
		return AILSA_NO_DATA;
	int retval, i;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();

	if ((retval = cmdb_add_number_to_list(start, l)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_number_to_list(end, l)) != 0)
		goto cleanup;
	for (i = 0; i < 2; i++) {
		if ((retval = cmdb_add_number_to_list(start, l)) != 0)
			goto cleanup;
	}
	for (i = 0; i < 2; i++) {
		if ((retval = cmdb_add_number_to_list(end, l)) != 0)
			goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, REV_ZONE_OVERLAP, l, r)) != 0) {
		ailsa_syslog(LOG_ERR, "REV_ZONE_OVERLAP query failed");
		goto cleanup;
	}
	if (r->total > 0)
		retval = AILSA_REV_ZONE_OVERLAP;

	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		return retval;
}

int
get_zone_index(unsigned long int prefix, unsigned long int *index)
{
	if (!(index) || (prefix == 0))
		return AILSA_NO_DATA;
	unsigned long int i = 0;
	double power;
	if (prefix < 8)
		return AILSA_PREFIX_OUT_OF_RANGE;
	else if ((prefix > 8) && (prefix < 16))
		i = 16 - prefix;
	else if ((prefix > 16) && (prefix < 24))
		i = 24 - prefix;
	if (i != 0)
		power = pow(2, (double)i);
	else
		power = 1;
	i = (unsigned long int)power;
	*index = i;
	return 0;
}

int
get_ip_addr_and_prefix(const char *ip, char **range, unsigned long int *prefix)
{
	if (!(ip) || !(range) || !(prefix))
		return AILSA_NO_DATA;
	char *tmp = strndup(ip, MAC_LEN);
	char *ptr;
	if (!(ptr = strchr(tmp, '/'))) {
		ailsa_syslog(LOG_ERR, "Character / not in string for netowrk range: %s", ip);
		return AILSA_INPUT_INVALID;
	}
	*ptr++ = '\0';
	if (!(*range = strndup(tmp, SERVICE_LEN))) {
		ailsa_syslog(LOG_ERR, "strndup failed for range in get_ip_addr_and_prefix");
		return AILSA_STRING_FAIL;
	}
	if (strlen(ptr) > 2) {
		ailsa_syslog(LOG_ERR, "Netmask can only be donoted in prefix format, e.g. /24");
		return AILSA_STRING_FAIL;
	} else {
		*prefix = strtoul(ptr, NULL, 10);
	}
	my_free(tmp);
	return 0;
}

uint32_t
prefix_to_mask_ipv4(unsigned long int prefix)
{
	uint32_t pf;
	if (prefix) {
		pf = (uint32_t)(4294967295 << (32 - prefix));
		return pf;
	} else {
		return 0;
	}
}

int
do_rev_lookup(char *ip, char *host, size_t size)
{
	int retval = 0;
	struct addrinfo hints, *res;
	socklen_t len = sizeof(struct sockaddr_in6);
	socklen_t hlen = (socklen_t)size;

	if (!(ip) || !(host))
		return AILSA_NO_DATA;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((retval = getaddrinfo(ip, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "Getaddrinfo in do_rev_lookup: %s\n",
		  gai_strerror(retval));
		return AILSA_GETADDR_FAIL;
	}
	if ((retval = getnameinfo(res->ai_addr, len, host, hlen, NULL, 0, NI_NAMEREQD)) != 0) {
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(retval));
		retval = AILSA_DNS_LOOKUP_FAIL;
	}
	return retval;
}

int
convert_bin_ipv4_to_text(unsigned long int ip, char *addr)
{
	if (!(addr) || (ip == 0))
		return AILSA_NO_DATA;
	uint32_t bin = htonl((u_int32_t)ip);

	if (!(inet_ntop(AF_INET, &bin, addr, SERVICE_LEN))) {
		ailsa_syslog(LOG_ERR, "IP address to text conversion failed");
		return AILSA_IP_CONVERT_FAILED;
	}
	return 0;
}

int
convert_text_ipv4_to_bin(unsigned long int *ip, const char *addr)
{
        if (!(ip) || !(addr))
                return AILSA_NO_DATA;
        u_int32_t bin, host;
        int retval;

        if ((retval = inet_pton(AF_INET, addr, &bin)) != 1) {
                ailsa_syslog(LOG_ERR, "IP addresss to binary conversion failed");
                return AILSA_IP_CONVERT_FAILED;
        }
        host = ntohl(bin);
        *ip = (unsigned long int)host;
        return 0;
}

int
get_range_search_string(const char *range, char *search, unsigned long int prefix, unsigned long int index)
{
        if (!(range) || !(search) || (prefix == 0))
                return AILSA_NO_DATA;
        int retval;
        char *p;

        if ((retval = get_offset_ip(range, search, prefix, index)) != 0) {
                ailsa_syslog(LOG_ERR, "Cannot get offset IP address");
                return retval;
        }
        if (!(p = strchr(search, '.')))
                return AILSA_IP_CONVERT_FAILED;
        p++;
        if (prefix > 8) {
                if (!(p = strchr(p, '.')))
                        return AILSA_IP_CONVERT_FAILED;
                p++;
        }
        if (prefix > 16) {
                if (!(p = strchr(p, '.')))
                        return AILSA_IP_CONVERT_FAILED;
                p++;
        }
        snprintf(p, 2, "%%");
        return retval;
}

int
get_offset_ip(const char *range, char *addr, unsigned long int prefix, unsigned long int index)
{
        if (!(range) || !(addr) || (prefix == 0))
                return AILSA_NO_DATA;
        int retval;
        unsigned long int ip;
        unsigned long int third = 256;
        unsigned long int second = 256 * 256;

        if ((retval = convert_text_ipv4_to_bin(&ip, range)) != 0)
                return retval;
        if ((prefix > 16) && (prefix <= 24)) {
                if (index > 0)
                        ip += (third * index);
        } else if ((prefix > 8 ) && (prefix <= 16)) {
                if (index > 0)
                        ip += (second * index);
        } else if ((prefix == 8)) {
                ;
        } else {
                return AILSA_IP_CONVERT_FAILED;
        }
        retval = convert_bin_ipv4_to_text(ip, addr);
        return retval;
}

int
get_start_finsh_ips(const char *range, unsigned long int prefix, unsigned long int *start, unsigned long int *end)
{
        if (!(range) && (prefix == 0))
                return AILSA_NO_DATA;
        char *ip = ailsa_calloc(MAC_LEN, "ip in get_start_finish_ips");
        int retval;
        unsigned long int index, last;

        if ((retval = convert_text_ipv4_to_bin(start, range)) != 0) {
                ailsa_syslog(LOG_ERR, "Cannot convert range %s to binary", range);
                goto cleanup;
        }
        if ((retval = get_zone_index(prefix, &index)) != 0) {
                ailsa_syslog(LOG_ERR, "Cannot get index for prefix %lu", prefix);
                goto cleanup;
        }
        if ((retval = get_offset_ip(range, ip, prefix, index)) != 0) {
                ailsa_syslog(LOG_ERR, "Cannot get index IP address");
                goto cleanup;
        }
        if ((retval = convert_text_ipv4_to_bin(&last, ip)) != 0) {
                ailsa_syslog(LOG_ERR, "Cannot convert offset %s to binary", ip);
                goto cleanup;
        }
        *end = --last;
        cleanup:
                my_free(ip);
                return retval;
}
