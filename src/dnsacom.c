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
 *  dnsacom.c:
 *  Contains functions to deal with command line arguments and also
 *  to read the values from the configuration file.
 *
 *  Part of the DNSA program
 * 
 * 
 */
#include <config.h>
#include <configmake.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // H)AVE_GETOPT_H
#include <ailsacmdb.h>
#include "cmdb_dnsa.h"


int
parse_dnsa_command_line(int argc, char **argv, dnsa_comm_line_s *comp)
{
	const char *optstr = "abdeglmruvwxzFGI:M:N:RSh:i:j:n:o:p:s:t:";
	int opt, retval;
	retval = 0;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"build",		no_argument,		NULL,	'b'},
		{"display",		no_argument,		NULL,	'd'},
		{"add-preferred-a",	no_argument,		NULL,	'e'},
		{"delete-preferred-a",	no_argument,		NULL,	'g'},
		{"host",		required_argument,	NULL,	'h'},
		{"destination",		required_argument,	NULL,	'i'},
		{"top-level",		required_argument,	NULL,	'j'},
		{"list",		no_argument,		NULL,	'l'},
		{"add-cname",		no_argument,		NULL,	'm'},
		{"zone-name",		required_argument,	NULL,	'n'},
		{"protocol",		required_argument,	NULL,	'o'},
		{"prefix",		required_argument,	NULL,	'p'},
		{"priority",		required_argument,	NULL,	'p'},
		{"delete-record",	no_argument,		NULL,	'r'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"service",		required_argument,	NULL,	's'},
		{"record-type",		required_argument,	NULL,	't'},
		{"display-multi-a",	no_argument,		NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"write",		no_argument,		NULL,	'w'},
		{"commit",		no_argument,		NULL,	'w'},
		{"delete-zone",		no_argument,		NULL,	'x'},
		{"exterminate",		no_argument,		NULL,	'x'},
		{"add-zone",		no_argument,		NULL,	'z'},
		{"forward-zone",	no_argument,		NULL,	'F'},
		{"glue-zone",		no_argument,		NULL,	'G'},
		{"name-server-ip",	required_argument,	NULL,	'I'},
		{"master-ip",		required_argument,	NULL,	'M'},
		{"name-servers",	required_argument,	NULL,	'N'},
		{"reverse-zone",	no_argument,		NULL,	'R'},
		{"slave-zone",		no_argument,		NULL,	'S'},
		{NULL, 0, NULL, 0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a') {
			comp->action = DNSA_AHOST;
			comp->type = FORWARD_ZONE;
		} else if (opt == 'b') {
			comp->action = DNSA_BREV;
			comp->type = REVERSE_ZONE;
		} else if (opt == 'd') {
			comp->action = DNSA_DISPLAY;
		} else if (opt == 'e') {
			comp->action = DNSA_ADD_MULTI;
			comp->type = REVERSE_ZONE;
			comp->rtype = strndup("A", MAC_LEN);
		} else if (opt == 'g') {
			comp->action = DNSA_DPREFA;
			comp->type = REVERSE_ZONE;
		} else if (opt == 'l') {
			comp->action = DNSA_LIST;
		} else if (opt == 'm') {
			comp->action = DNSA_CNAME;
			comp->type = FORWARD_ZONE;
		} else if (opt == 'r') {
			comp->action = DNSA_DREC;
			comp->type = FORWARD_ZONE;
		} else if (opt == 'u') {
			comp->action = DNSA_DISPLAY_MULTI;
			comp->type = REVERSE_ZONE;
		} else if (opt == 'w') {
			comp->action = DNSA_COMMIT;
		} else if (opt == 'x') {
			comp->action = DNSA_DZONE;
		} else if (opt == 'z') {
			comp->action = DNSA_AZONE;
		} else if (opt == 'F') {
			comp->type = FORWARD_ZONE;
		} else if (opt == 'R') {
			comp->type = REVERSE_ZONE;
		} else if (opt == 'G') {
			comp->type = GLUE_ZONE;
		} else if (opt == 'S') {
			comp->ztype = strdup("slave");
		} else if (opt == 'M') {
			comp->master = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 'I') {
			comp->glue_ip = strndup(optarg, MAC_LEN);
		} else if (opt == 'N') {
			comp->glue_ns = strndup(optarg, FILE_LEN);
		} else if (opt == 'h') {
			comp->host = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 'i') {
			comp->dest = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 'n') {
			comp->domain = strndup(optarg, DOMAIN_LEN);
		} else if (opt == 'o') {
			comp->protocol = strndup(optarg, SERVICE_LEN);
		} else if (opt == 'p') {
			comp->prefix = strtoul(optarg, NULL, 10);
		} else if (opt == 's') {
			comp->service = strndup(optarg, SERVICE_LEN);
			if (!(comp->host))
				comp->host = strndup(optarg, SERVICE_LEN);
		} else if (opt == 't') {
			comp->rtype = strndup(optarg, SERVICE_LEN);
		} else if (opt == 'v') {
			comp->action = AILSA_VERSION;
		}
	}
	if (comp->rtype) {
		if (strncmp(comp->rtype, "SRV", BYTE_LEN) == 0) {
/* Check if user has specified destination with -h and act accordingly */
			if ((comp->host) && (!(comp->dest)))
				comp->dest = strndup(comp->host, DOMAIN_LEN);
			if (!(comp->protocol)) {
				ailsa_syslog(LOG_INFO, "No protocol provided with -o. Setting to tcp!\n");
				comp->protocol = strdup("tcp");
			} else if (!((strncmp(comp->protocol, "tcp", BYTE_LEN) == 0) ||
			     (strncmp(comp->protocol, "udp", BYTE_LEN) == 0)))
				return PROTOCOL_INVALID;
			if (comp->prefix == 0) {
				ailsa_syslog(LOG_INFO, "No priority provided with -p. Setting to 100!\n");
				comp->prefix = 100;
			}
		}
		if ((strncmp(comp->rtype, "MX", BYTE_LEN) == 0) && comp->prefix == 0) {
			comp->prefix = 100;
			fprintf(stderr, "No priority specified for MX record, using 100\n");
		}
	}
	if (comp->ztype) {
		if (!(comp->master))
			retval = AILSA_NO_MASTER;
	}
	if ((comp->action == NONE) && (comp->type == NONE) && (!(comp->domain)))
		retval = AILSA_DISPLAY_USAGE;
	else if (comp->action == AILSA_VERSION)
		retval = AILSA_VERSION;
	else if (comp->action == NONE)
		retval = AILSA_NO_ACTION;
	else if (comp->type == NONE)
		retval = AILSA_NO_TYPE;
	else if ((!(comp->domain)) && (comp->action != DNSA_LIST)
		  && (comp->action != DNSA_DISPLAY_MULTI) && (comp->action != DNSA_DPREFA)
		  && (comp->action != DNSA_COMMIT))
		retval = AILSA_NO_DOMAIN_NAME;
	else if ((comp->action == DNSA_DISPLAY_MULTI) && (!(comp->domain)) && (!(comp->dest)))
		retval = AILSA_NO_DOMAIN_NAME;
	else if ((comp->action == DNSA_DISPLAY_MULTI) && (comp->domain) && (comp->dest))
		retval = AILSA_DOMAIN_AND_IP_GIVEN;
	else if ((comp->action == DNSA_AHOST) && (!(comp->dest)))
		retval = AILSA_NO_IP_ADDRESS;
	else if (((comp->action == DNSA_AHOST) || (comp->action == DNSA_DREC) ||
	      (comp->action == DNSA_CNAME)) && (!(comp->host)))
		retval = AILSA_NO_HOST_NAME;
	else if (comp->action == DNSA_AHOST && (!(comp->rtype)))
		retval = AILSA_NO_RECORD_TYPE;
	else if ((comp->action == DNSA_AZONE && comp->type == REVERSE_ZONE && comp->prefix == 0))
		retval = AILSA_NO_PREFIX;
	else if ((comp->type == GLUE_ZONE) && (!(comp->glue_ns)) && (comp->action == DNSA_AZONE))
		retval = AILSA_NO_GLUE_NS;
	if (retval == AILSA_NO_GLUE_NS) {
		comp->glue_ns = strdup("ns1,ns2");
		retval = NONE;
	}
	if (retval == 0)
		retval = validate_comm_line(comp);
	return retval;
}

int
validate_comm_line(dnsa_comm_line_s *comm)
{
	int retval = 0;
	
	if ((comm->action == DNSA_LIST) || (comm->action == DNSA_COMMIT))
		return retval;
	if ((comm->type == FORWARD_ZONE) || (comm->type == GLUE_ZONE)) {
		retval = validate_fwd_comm_line(comm);
	} else if (comm->type == REVERSE_ZONE) {
		retval = validate_rev_comm_line(comm);
	} else {
		ailsa_syslog(LOG_ERR, "Unknown zone type %d", comm->type);
		retval = AILSA_UNKNOWN_ZONE_TYPE;
	}
	return retval;
}

int
validate_fwd_comm_line(dnsa_comm_line_s *comm)
{
	char *host = NULL;
	int retval = 0;

	if (comm)
		host = comm->host;
	else
		return AILSA_NO_DATA;
	if (comm->rtype)
		if (ailsa_validate_input(comm->rtype, RESOURCE_TYPE_REGEX) < 0)
			return RTYPE_INPUT_INVALID;
	if (comm->domain)
		if (ailsa_validate_input(comm->domain, DOMAIN_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
/* Test values for different RR's. Still need to add check for AAAA */
	if (comm->rtype) {
		if (strncmp(comm->rtype, "A", BYTE_LEN) == 0) {
			if (comm->dest)
				if (ailsa_validate_input(comm->dest, IP_REGEX) < 0)
					return DEST_INPUT_INVALID;
			if (strncmp(comm->host, "@", BYTE_LEN) == 0) {
				if (ailsa_validate_input(comm->dest, NAME_REGEX) < 0)
					if (ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0)
						return DEST_INPUT_INVALID;
			} else {
				if (ailsa_validate_input(comm->host, NAME_REGEX) < 0)
					if (ailsa_validate_input(comm->host, DOMAIN_REGEX) < 0)
						return HOST_INPUT_INVALID;
			}
		} else if ((strncmp(comm->rtype, "NS", BYTE_LEN) == 0) ||
			   (strncmp(comm->rtype, "MX", BYTE_LEN) == 0)) {
			if (comm->dest)
				if (ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0)
					return DEST_INPUT_INVALID;
			if ((strncmp(comm->host, "NULL", BYTE_LEN) != 0) &&
			    (strncmp(comm->host, "@", BYTE_LEN) != 0))
				if (ailsa_validate_input(comm->host, NAME_REGEX) < 0)
						return HOST_INPUT_INVALID;
		} else if (strncmp(comm->rtype, "SRV", BYTE_LEN) == 0) {
			if (comm->dest)
				if ((ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0) &&
			   	 (ailsa_validate_input(comm->dest, NAME_REGEX) < 0))
					return DEST_INPUT_INVALID;
			if (ailsa_validate_input(comm->service, NAME_REGEX) < 0)
				return SERVICE_INPUT_INVALID;
		} else if (strncmp(comm->rtype, "CNAME", BYTE_LEN) == 0) {
			if (comm->dest)
				if ((ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0) &&
			   	 (ailsa_validate_input(comm->dest, NAME_REGEX) < 0))
					return DEST_INPUT_INVALID;
			if ((ailsa_validate_input(comm->host, DOMAIN_REGEX) < 0) &&
			    (ailsa_validate_input(comm->host, NAME_REGEX) < 0))
				return HOST_INPUT_INVALID;
		}
	}
	if (comm->action == DNSA_DREC) {
		if (comm->protocol) {
			if (!(comm->prefix))
				return AILSA_NO_PRIORITY;
			if (!(comm->service))
				return AILSA_NO_SERVICE;
		} else if (comm->service) {
			if (!(comm->prefix))
				return AILSA_NO_PRIORITY;
			if (!(comm->protocol))
				return AILSA_NO_PROTOCOL;
		}
	}
	if (comm->action == DNSA_CNAME) {
		if (ailsa_validate_input(comm->domain, DOMAIN_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
		if (ailsa_validate_input(comm->host, NAME_REGEX) < 0)
			return HOST_INPUT_INVALID;
		if (comm->toplevel)
			if (ailsa_validate_input(comm->toplevel, DOMAIN_REGEX) < 0)
				return DOMAIN_INPUT_INVALID;
	} else {
		if ((comm->rtype) && (comm->host)) {
			if (strncmp(comm->rtype, "TXT", BYTE_LEN) == 0) {
				if (host[0] == '_') {
					if (ailsa_validate_input(host + 1, NAME_REGEX) < 0)
						return HOST_INPUT_INVALID;
				} else {
					if (ailsa_validate_input(host, NAME_REGEX) < 0)
						return HOST_INPUT_INVALID;
				}
				if (comm->dest) {
					if (ailsa_validate_input(comm->dest, TXTRR_REGEX) < 0)
						return DEST_INPUT_INVALID;
				}
			}
		}
	}
	if (comm->service)
		if (ailsa_validate_input(comm->service, NAME_REGEX) < 0)
			return SERVICE_INPUT_INVALID;
	if (comm->type == GLUE_ZONE && comm->action != DNSA_DISPLAY)
		retval = validate_glue_comm_line(comm);
	return retval;
}

int
validate_glue_comm_line(dnsa_comm_line_s *comm)
{
	char *regex;
	size_t dlen, ilen;
	int retval = 0;

	dlen = strlen(regexps[DOMAIN_REGEX]);
	ilen = strlen(regexps[IP_REGEX]);
	if ((ilen + dlen + 4) > CONFIG_LEN) {
		ailsa_syslog(LOG_ERR, "Buffer too small for regexs in validate_glue_comm_line");
		return AILSA_BUFFER_TOO_SMALL;
	}
	regex = ailsa_calloc(CONFIG_LEN, "regex in validate_glue_comm_line");
	if (comm->glue_ip) {
		if (strchr(comm->glue_ip, ',')) {
			snprintf(regex, ilen, "%s", regexps[IP_REGEX]);
			strncat(regex, "\\,", 3);
			strncat(regex, regexps[IP_REGEX] + 1, ilen);
			if (ailsa_validate_string(comm->glue_ip, regex) < 0) {
				retval = GLUE_IP_INPUT_INVALID;
				goto cleanup;
			}
		} else if (comm->action != DNSA_DZONE) {
			if (ailsa_validate_input(comm->glue_ip, IP_REGEX) < 0) {
				retval =  GLUE_IP_INPUT_INVALID;
				goto cleanup;
			}
		}
	}
	memset(regex, 0, CONFIG_LEN);
	if (comm->glue_ns) {
		if (strchr(comm->glue_ns, ',')) {
			snprintf(regex, dlen, "%s", regexps[DOMAIN_REGEX]);
			strncat(regex, "\\,", 3);
			strncat(regex, regexps[DOMAIN_REGEX] + 1, dlen);
			if (ailsa_validate_string(comm->glue_ns, regex) < 0) {
				retval =  GLUE_NS_INPUT_INVALID;
				goto cleanup;
			}
		} else {
			if (ailsa_validate_input(comm->glue_ns, DOMAIN_REGEX) < 0) {
				retval =  GLUE_NS_INPUT_INVALID;
				goto cleanup;
			}
		}
	}
	cleanup:
		my_free(regex);
		return retval;
}

int
validate_rev_comm_line(dnsa_comm_line_s *comm)
{
	int retval = 0;
	if ((comm->domain) && (comm->action != DNSA_ADD_MULTI) && (comm->action != DNSA_DPREFA))
		if (ailsa_validate_input(comm->domain, IP_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
	if (comm->action == DNSA_ADD_MULTI) {
		if (ailsa_validate_input(comm->domain, DOMAIN_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
		if (ailsa_validate_input(comm->dest, IP_REGEX) < 0)
			return DEST_INPUT_INVALID;
		if (ailsa_validate_input(comm->host, NAME_REGEX) < 0)
			return HOST_INPUT_INVALID;
	}
	if (comm->dest)
		if (ailsa_validate_input(comm->dest, IP_REGEX) < 0)
			return DEST_INPUT_INVALID;
	return retval;
}
