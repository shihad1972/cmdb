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
#include "cmdb.h"
#include <ailsacmdb.h>
#include "dnsa_data.h"
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
			comp->action = ADD_HOST;
			comp->type = FORWARD_ZONE;
		} else if (opt == 'b') {
			comp->action = BUILD_REV;
			comp->type = REVERSE_ZONE;
		} else if (opt == 'd') {
			comp->action = DISPLAY_ZONE;
		} else if (opt == 'e') {
			comp->action = ADD_PREFER_A;
			comp->type = REVERSE_ZONE;
		} else if (opt == 'g') {
			comp->action = DELETE_PREFERRED;
			comp->type = REVERSE_ZONE;
		} else if (opt == 'l') {
			comp->action = LIST_ZONES;
		} else if (opt == 'm') {
			comp->action = ADD_CNAME_ON_ROOT;
			comp->type = FORWARD_ZONE;
		} else if (opt == 'r') {
			comp->action = DELETE_RECORD;
			comp->type = FORWARD_ZONE;
		} else if (opt == 'u') {
			comp->action = MULTIPLE_A;
			comp->type = REVERSE_ZONE;
		} else if (opt == 'w') {
			comp->action = COMMIT_ZONES;
		} else if (opt == 'x') {
			comp->action = DELETE_ZONE;
		} else if (opt == 'z') {
			comp->action = ADD_ZONE;
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
			comp->action = CVERSION;
		}
	}
	if (comp->rtype) {
		if (strncmp(comp->rtype, "SRV", COMM_S) == 0) {
/* Check if user has specified destination with -h and act accordingly */
			if ((comp->host) && (!(comp->dest)))
				comp->dest = strndup(comp->host, DOMAIN_LEN);
			if (!(comp->protocol)) {
				ailsa_syslog(LOG_INFO, "No protocol provided with -o. Setting to tcp!\n");
				comp->protocol = strdup("tcp");
			} else if (!((strncmp(comp->protocol, "tcp", COMM_S) == 0) ||
			     (strncmp(comp->protocol, "udp", COMM_S) == 0)))
				report_error(USER_INPUT_INVALID, "protocol");
			if (comp->prefix == 0) {
				ailsa_syslog(LOG_INFO, "No priority provided with -p. Setting to 100!\n");
				comp->prefix = 100;
			}
		}
		if ((strncmp(comp->rtype, "MX", COMM_S) == 0) && comp->prefix == 0) {
			comp->prefix = 100;
			fprintf(stderr, "No priority specified for MX record, using 100\n");
		}
	}
	if (comp->ztype) {
		if (!(comp->master))
			retval = NO_MASTER;
	}
	if ((comp->action == NONE) && (comp->type == NONE) && (!(comp->domain)))
		retval = DISPLAY_USAGE;
	else if (comp->action == CVERSION)
		retval = CVERSION;
	else if (comp->action == NONE)
		retval = NO_ACTION;
	else if (comp->type == NONE)
		retval = NO_TYPE;
	else if ((!(comp->domain)) && (comp->action != LIST_ZONES)
		  && (comp->action != MULTIPLE_A) && (comp->action != DELETE_PREFERRED)
		  && (comp->action != COMMIT_ZONES))
		retval = NO_DOMAIN_NAME;
	else if ((comp->action == MULTIPLE_A) && (!(comp->domain)) && (!(comp->dest)))
		retval = NO_DOMAIN_NAME;
	else if ((comp->action == MULTIPLE_A) && (comp->domain) && (comp->dest))
		retval = DOMAIN_AND_IP_GIVEN;
	else if ((comp->action == ADD_HOST) && (!(comp->dest)))
		retval = NO_IP_ADDRESS;
	else if (((comp->action == ADD_HOST) || (comp->action == DELETE_RECORD) ||
	      (comp->action == ADD_CNAME_ON_ROOT)) && (!(comp->host)))
		retval = NO_HOST_NAME;
	else if (comp->action == ADD_HOST && (!(comp->rtype)))
		retval = NO_RECORD_TYPE;
	else if ((comp->action == ADD_ZONE && comp->type == REVERSE_ZONE && comp->prefix == 0))
		retval = NO_PREFIX;
	else if ((comp->type == GLUE_ZONE) && (!(comp->glue_ns)) && (comp->action == ADD_ZONE))
		retval = NO_GLUE_NS;
	if (retval == NO_GLUE_NS) {
		comp->glue_ns = strdup("ns1,ns2");
		retval = NONE;
	}
	if (retval == 0)
		retval = validate_comm_line(comp);
	return retval;
}

int
parse_dnsa_config_file(ailsa_cmdb_s *dc, char *config)
{
	int retval = 0;
	FILE *cnf;
#ifdef HAVE_WORDEXP_H
	char **uconf;
	wordexp_t p;
#endif /* HAVE_WORDEXP_H */

	dc->port = 3306;
	dc->cliflag = 0;

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		retval = CONF_ERR;
	} else {
		read_dnsa_config_values(dc, cnf);
		fclose(cnf);
	}
#ifdef HAVE_WORDEXP
	if ((retval = wordexp("~/.dnsa.conf", &p, 0)) == 0) {
		uconf = p.we_wordv;
		if ((cnf = fopen(*uconf, "r"))) {
			read_dnsa_config_values(dc, cnf);
			fclose(cnf);
		}
		wordfree(&p);
	}
#endif /* HAVE_WORDEXP */
	if ((retval = ailsa_add_trailing_slash(dc->dir)) != 0)
		return DIR_ERR;
	if ((retval = ailsa_add_trailing_slash(dc->bind)) != 0)
		return BIND_ERR;
	if ((retval = ailsa_add_trailing_dot(dc->hostmaster)) != 0)
		return HOSTM_ERR;
	return retval;
}

int
read_dnsa_config_values(ailsa_cmdb_s *dc, FILE *cnf)
{
	char buff[RBUFF_S] = "";
	char port[RANGE_S] = "";
	char refresh[MAC_S] = "";
	char retry[MAC_S] = "";
	char expire[MAC_S] = "";
	char ttl[MAC_S] = "";
	char *hostmaster;
	int retval = 0;
	unsigned long int portno;
	
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DBTYPE=%s", dc->dbtype);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PASS=%s", dc->pass);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "FILE=%s", dc->file);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "HOST=%s", dc->host);	
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "USER=%s", dc->user);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DB=%s", dc->db);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "SOCKET=%s", dc->socket);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PORT=%s", port);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DIR=%s", dc->dir);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "BIND=%s", dc->bind);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "REV=%s", dc->rev);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DNSA=%s", dc->dnsa);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "RNDC=%s", dc->rndc);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "CHKZ=%s", dc->chkz);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "CHKC=%s", dc->chkc);
	rewind(cnf);
	while ((fgets(buff, MAC_S, cnf)))
		sscanf(buff, "REFRESH=%s", refresh);
	rewind(cnf);
	while ((fgets(buff, MAC_S, cnf)))
		sscanf(buff, "RETRY=%s", retry);
	rewind(cnf);
	while ((fgets(buff, MAC_S, cnf)))
		sscanf(buff, "EXPIRE=%s", expire);
	rewind(cnf);
	while ((fgets(buff, MAC_S, cnf)))
		sscanf(buff, "TTL=%s", ttl);
	rewind(cnf);
	while ((fgets(buff, MAC_S, cnf)))
		sscanf(buff, "PRIDNS=%s", dc->pridns);
	rewind(cnf);
	while ((fgets(buff, MAC_S, cnf)))
		sscanf(buff, "SECDNS=%s", dc->secdns);
	rewind(cnf);
	while ((fgets(buff, RBUFF_S - 1, cnf)))
		sscanf(buff, "HOSTMASTER=%s", dc->hostmaster);
	rewind(cnf);
	while ((fgets(buff, RBUFF_S - 1, cnf)))
		sscanf(buff, "PRINS=%s", dc->prins);
	rewind(cnf);
	while ((fgets(buff, RBUFF_S - 1, cnf)))
		sscanf(buff, "SECNS=%s", dc->secns);
	rewind(cnf);
	retval = NONE;

	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
	} else {
		dc->port = (unsigned int) portno;
	}
	if (strlen(refresh) > 0)
		dc->refresh = strtoul(refresh, NULL, 10);
	if (strlen(retry) > 0)
		dc->retry = strtoul(retry, NULL, 10);
	if (strlen(expire) > 0)
		dc->expire = strtoul(expire, NULL, 10);
	if (strlen(ttl) > 0)
		dc->ttl = strtoul(ttl, NULL, 10);
	hostmaster = strchr(dc->hostmaster, '@');
	if (hostmaster)
		*hostmaster = '.';
	return retval;
}

void
parse_dnsa_config_error(int error)
{
	if (error == PORT_ERR)
		fprintf(stderr, "Port higher than 65535!\n");
	else if (error == DIR_ERR)
		fprintf(stderr, "Cannot add trailing / to DIR: > 79 characters\n");
	else if (BIND_ERR)
		fprintf(stderr, "Cannot add trailing / to BIND: > 79 characters\n");
}

int
validate_comm_line(dnsa_comm_line_s *comm)
{
	int retval = 0;
	
	if ((comm->action == LIST_ZONES) || (comm->action == COMMIT_ZONES))
		return retval;
	if ((comm->type == FORWARD_ZONE) || (comm->type == GLUE_ZONE))
		retval = validate_fwd_comm_line(comm);
	else if (comm->type == REVERSE_ZONE)
		retval = validate_rev_comm_line(comm);
	else
		report_error(UNKNOWN_ZONE_TYPE, "validate_comm_line");
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
		if (strncmp(comm->rtype, "A", COMM_S) == 0) {
			if (comm->dest)
				if (ailsa_validate_input(comm->dest, IP_REGEX) < 0)
					return DEST_INPUT_INVALID;
			if (strncmp(comm->host, "@", COMM_S) == 0) {
				if (ailsa_validate_input(comm->dest, NAME_REGEX) < 0)
					if (ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0)
						return DEST_INPUT_INVALID;
			} else {
				if (ailsa_validate_input(comm->host, NAME_REGEX) < 0)
					if (ailsa_validate_input(comm->host, DOMAIN_REGEX) < 0)
						return HOST_INPUT_INVALID;
			}
		} else if ((strncmp(comm->rtype, "NS", COMM_S) == 0) ||
			   (strncmp(comm->rtype, "MX", COMM_S) == 0)) {
			if (comm->dest)
				if (ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0)
					return DEST_INPUT_INVALID;
			if ((strncmp(comm->host, "NULL", COMM_S) != 0) &&
			    (strncmp(comm->host, "@", COMM_S) != 0))
				if (ailsa_validate_input(comm->host, NAME_REGEX) < 0)
						return HOST_INPUT_INVALID;
		} else if (strncmp(comm->rtype, "SRV", COMM_S) == 0) {
			if (comm->dest)
				if ((ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0) &&
			   	 (ailsa_validate_input(comm->dest, NAME_REGEX) < 0))
					return DEST_INPUT_INVALID;
			if (ailsa_validate_input(comm->service, NAME_REGEX) < 0)
				return SERVICE_INPUT_INVALID;
		} else if (strncmp(comm->rtype, "CNAME", COMM_S) == 0) {
			if (comm->dest)
				if ((ailsa_validate_input(comm->dest, DOMAIN_REGEX) < 0) &&
			   	 (ailsa_validate_input(comm->dest, NAME_REGEX) < 0))
					return DEST_INPUT_INVALID;
			if ((ailsa_validate_input(comm->host, DOMAIN_REGEX) < 0) &&
			    (ailsa_validate_input(comm->host, NAME_REGEX) < 0))
				return HOST_INPUT_INVALID;
		}
	}
	if (comm->action == DELETE_RECORD) {
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
	if (comm->action == ADD_CNAME_ON_ROOT) {
		if (ailsa_validate_input(comm->domain, DOMAIN_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
		if (ailsa_validate_input(comm->host, NAME_REGEX) < 0)
			return HOST_INPUT_INVALID;
		if (!(comm->rtype))
			return NO_RTYPE;
	} else {
		if ((comm->rtype) && (comm->host)) {
			if (strncmp(comm->rtype, "TXT", COMM_S) == 0) {
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
	if (comm->type == GLUE_ZONE && comm->action != DISPLAY_ZONE)
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
	if ((ilen + dlen + 4) > RBUFF_S)
		report_error(BUFFER_TOO_SMALL, "regex in validate_glue_comm_line");
	regex = ailsa_calloc(RBUFF_S, "regex in validate_glue_comm_line");
	if (comm->glue_ip) {
		if (strchr(comm->glue_ip, ',')) {
			snprintf(regex, ilen, "%s", regexps[IP_REGEX]);
			strncat(regex, "\\,", 3);
			strncat(regex, regexps[IP_REGEX] + 1, ilen);
			if (ailsa_validate_string(comm->glue_ip, regex) < 0) {
				retval = GLUE_IP_INPUT_INVALID;
				goto cleanup;
			}
		} else if (comm->action != DELETE_ZONE) {
			if (ailsa_validate_input(comm->glue_ip, IP_REGEX) < 0) {
				retval =  GLUE_IP_INPUT_INVALID;
				goto cleanup;
			}
		}
	}
	memset(regex, 0, RBUFF_S);
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
		cmdb_free(regex, RBUFF_S);
		return retval;
}

int
validate_rev_comm_line(dnsa_comm_line_s *comm)
{
	int retval = 0;
	if ((comm->domain) && (comm->action != ADD_PREFER_A) && (comm->action != DELETE_PREFERRED))
		if (ailsa_validate_input(comm->domain, IP_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
	if (comm->action == ADD_PREFER_A) {
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
