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
 *  dnsa_ip.c: Holds functions for ip address related functions for dnsa
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
