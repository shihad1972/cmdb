/*
 * 
 *  dnsa: DNS Administration
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
 *  dnsacom.c:
 *  Contains functions to deal with command line arguments and also
 *  to read the values from the configuration file.
 *
 *  Part of the DNSA program
 * 
 * 
 */
#include "../config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "checks.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

int
parse_dnsa_command_line(int argc, char **argv, comm_line_t *comp)
{
	int i, retval;
	
	retval = 0;

	comp->action = NONE;
	comp->type = NONE;
	comp->prefix = NONE;
	strncpy(comp->domain, "NULL", CONF_S);
	strncpy(comp->dest, "NULL", RANGE_S);
	strncpy(comp->rtype, "NULL", RANGE_S);
	strncpy(comp->host, "NULL", RANGE_S);
	strncpy(comp->config, "/etc/dnsa/dnsa.conf", CONF_S);
	
	for (i = 1; i < argc; i++) {
		if ((strncmp(argv[i], "-d", COMM_S) == 0)) {
			comp->action = DISPLAY_ZONE;
		} else if ((strncmp(argv[i], "-z", COMM_S) == 0)) {
			comp->action = ADD_ZONE;
		} else if ((strncmp(argv[i], "-c", COMM_S) == 0)) {
			comp->action = COMMIT_ZONES;
			strncpy(comp->domain, "none", CONF_S);
		} else if ((strncmp(argv[i], "-a", COMM_S) == 0)) {
			comp->action = ADD_HOST;
			comp->type = FORWARD_ZONE;
		} else if ((strncmp(argv[i], "-l", COMM_S) == 0)) {
			comp->action = LIST_ZONES;
			strncpy(comp->domain, "all", CONF_S);
		} else if ((strncmp(argv[i], "-b", COMM_S) == 0)) {
			comp->action = BUILD_REV;
			comp->type = REVERSE_ZONE;
		} else if ((strncmp(argv[i], "-m", COMM_S) == 0)) {
			comp->action = MULTIPLE_A;
			comp->type = REVERSE_ZONE;
		} else if ((strncmp(argv[i], "-e", COMM_S) == 0)) {
			comp->action = ADD_PREFER_A;
			comp->type = REVERSE_ZONE;
		} else if ((strncmp(argv[i], "-f", COMM_S) == 0)) {
			comp->type = FORWARD_ZONE;
		} else if ((strncmp(argv[i], "-r", COMM_S) == 0)) {
			comp->type = REVERSE_ZONE;
		} else if ((strncmp(argv[i], "-n", COMM_S) == 0)) {
			i++;
			if (i >= argc) 
				retval = NO_DOMAIN_NAME;
			else
				strncpy(comp->domain, argv[i], CONF_S);
		} else if ((strncmp(argv[i], "-i", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_IP_ADDRESS;
			else
				strncpy(comp->dest, argv[i], RBUFF_S);
		} else if ((strncmp(argv[i], "-h", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_HOST_NAME;
			else
				strncpy(comp->host, argv[i], RBUFF_S);
		} else if ((strncmp(argv[i], "-t", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_RECORD_TYPE;
			else
				strncpy(comp->rtype, argv[i], RANGE_S);
		} else if ((strncmp(argv[i], "-p", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_PREFIX;
			else
				comp->prefix = strtoul(argv[i], NULL, 10);
		} else {
			retval = DISPLAY_USAGE;
		}
	}
	if (comp->action == NONE && comp->type == NONE && strncmp(comp->domain, "NULL", CONF_S) == 0)
		retval = DISPLAY_USAGE;
	else if (comp->action == NONE)
		retval = NO_ACTION;
	else if (comp->type == NONE)
		retval = NO_TYPE;
	else if ((strncmp(comp->domain, "NULL", CONF_S) == 0) &&
		(comp->action != MULTIPLE_A))
		retval = NO_DOMAIN_NAME;
	else if (comp->action == MULTIPLE_A && 
		strncmp(comp->domain, "NULL", COMM_S) == 0 &&
		strncmp(comp->dest, "NULL", COMM_S) == 0)
		retval = NO_DOMAIN_NAME;
	else if (comp->action == MULTIPLE_A &&
		strncmp(comp->domain, "NULL", COMM_S != 0) &&
		strncmp(comp->dest, "NULL", COMM_S != 0))
		retval = DOMAIN_AND_IP_GIVEN;
	else if ((comp->action == ADD_HOST && strncmp(comp->dest, "NULL", RANGE_S) == 0))
		retval = NO_IP_ADDRESS;
	else if ((comp->action == ADD_HOST && strncmp(comp->host, "NULL", RBUFF_S) == 0))
		retval = NO_HOST_NAME;
	else if ((comp->action == ADD_HOST && strncmp(comp->rtype, "NULL", RANGE_S) == 0))
		retval = NO_RECORD_TYPE;
	else if ((comp->action == ADD_ZONE && comp->type == REVERSE_ZONE && comp->prefix == 0))
		retval = NO_PREFIX;
	else if ((strncmp(comp->rtype, "MX", COMM_S) == 0) && comp->prefix == 0) {
		comp->prefix = 100;
		fprintf(stderr, "No priority specified for MX record, using 100\n");
	}
	return retval;
}

int
parse_dnsa_config_file(dnsa_config_t *dc, char *config)
{
	char buff[RBUFF_S] = "";
	char port[RANGE_S] = "";
	char refresh[MAC_S] = "";
	char retry[MAC_S] = "";
	char expire[MAC_S] = "";
	char ttl[MAC_S] = "";
	char *hostmaster;
	int retval;
	unsigned long int portno;
	FILE *cnf;	/* File handle for config file */

	dc->port = 3306;
	dc->cliflag = 0;

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = CONF_ERR;
	} else {
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
		fclose(cnf);
	}
	
	/* We need to check the value of portnop before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
	} else {
		dc->port = (unsigned int) portno;
	}
	dc->refresh = strtoul(refresh, NULL, 10);
	dc->retry = strtoul(retry, NULL, 10);
	dc->expire = strtoul(expire, NULL, 10);
	dc->ttl = strtoul(ttl, NULL, 10);
	hostmaster = strchr(dc->hostmaster, '@');
	if (hostmaster)
		*hostmaster = '.';
	/* The next 2 values need to be checked for a trailing /
	 * If there is not one then add it
	 */
	
	if ((retval = add_trailing_slash(dc->dir)) != 0)
		retval = DIR_ERR;
	if ((retval = add_trailing_slash(dc->bind)) != 0)
		retval = BIND_ERR;
	if ((retval = add_trailing_dot(dc->hostmaster)) != 0)
		retval = HOSTM_ERR; /*
	if ((retval = add_trailing_dot(dc->prins)) != 0)
		retval = PRINS_ERR;
	if ((retval = add_trailing_dot(dc->secns)) != 0)
		retval = SECNS_ERR; */
	return retval;
}

void
parse_dnsa_config_error(int error)
{
	switch(error) {
		case PORT_ERR:
			fprintf(stderr, "Port higher than 65535!\n");
			break;
		case DIR_ERR:
			fprintf(stderr, "Cannot add trailing / to DIR: > 79 characters\n");
			break;
		case BIND_ERR:
			fprintf(stderr, "Cannot add trailing / to BIND: > 79 characters\n");
			break;
		}
}

int
validate_comm_line(comm_line_t *comm)
{
	int retval;
	
	retval = 0;
	
	retval = validate_user_input(comm->host, NAME_REGEX);
	if (retval < 0)
		return retval;
	retval = validate_user_input(comm->dest, IP_REGEX);
	if (retval < 0)
		return retval;
	return retval;
}

void
init_config_values(dnsa_config_t *dc)
{
	char *buff;
	buff = dc->socket;
	sprintf(dc->file, "/var/lib/cmdb/cmdb.sql");
	sprintf(dc->dbtype, "sqlite");
	sprintf(dc->db, "bind");
	sprintf(dc->user, "root");
	sprintf(dc->host, "localhost");
	sprintf(dc->pass, "%s", "");
	sprintf(dc->dir, "/var/named/");
	sprintf(dc->bind, "/var/named/");
	sprintf(dc->dnsa, "dnsa.conf");
	sprintf(dc->rev, "dnsa-rev.conf");
	sprintf(dc->rndc, "/usr/sbin/rndc");
	sprintf(dc->chkz, "/usr/sbin/named-checkzone");
	sprintf(dc->chkc, "/usr/sbin/named-checkconf");
	sprintf(buff, "%s", "");
}

void
init_dnsa_struct(dnsa_t *dnsa)
{
	dnsa->zones = '\0';
	dnsa->rev_zones = '\0';
	dnsa->records = '\0';
	dnsa->rev_records = '\0';
	dnsa->prefer = '\0';
	dnsa->file = '\0';
}

void
init_zone_struct(zone_info_t *zone)
{
	zone->id = zone->owner = 0;
	zone->serial = zone->expire = zone->retry = 0;
	zone->refresh = zone->ttl = 0;
	snprintf(zone->name, COMM_S, "NULL");
	snprintf(zone->pri_dns, COMM_S, "NULL");
	snprintf(zone->sec_dns, COMM_S, "NULL");
	snprintf(zone->valid, COMM_S, "NULL");
	snprintf(zone->updated, COMM_S, "NULL");
	snprintf(zone->web_ip, COMM_S, "NULL");
	snprintf(zone->ftp_ip, COMM_S, "NULL");
	snprintf(zone->mail_ip, COMM_S, "NULL");
	zone->next = '\0';
}

void
init_rev_zone_struct(rev_zone_info_t *rev)
{
	rev->rev_zone_id = rev->owner = 0;
	rev->prefix = rev->serial = rev->refresh = rev->retry = rev->ttl = 0;
	rev->start_ip = rev->end_ip = rev->expire = 0;
	snprintf(rev->net_range, COMM_S, "NULL");
	snprintf(rev->net_start, COMM_S, "NULL");
	snprintf(rev->net_finish, COMM_S, "NULL");
	snprintf(rev->pri_dns, COMM_S, "NULL");
	snprintf(rev->sec_dns, COMM_S, "NULL");
	snprintf(rev->updated, COMM_S, "NULL");
	snprintf(rev->valid, COMM_S, "NULL");
	snprintf(rev->hostmaster, COMM_S, "NULL");
	rev->next = '\0';
}

void
init_record_struct(record_row_t *record)
{
	record->id = record->pri = record->zone = 0;
	snprintf(record->dest, COMM_S, "NULL");
	snprintf(record->host, COMM_S, "NULL");
	snprintf(record->type, COMM_S, "NULL");
	snprintf(record->valid, COMM_S, "NULL");
	record->next = '\0';
}

void
init_rev_record_struct(rev_record_row_t *rev)
{
	rev->record_id = 0;
	rev->rev_zone = 0;
	snprintf(rev->host, COMM_S, "NULL");
	snprintf(rev->dest, COMM_S, "NULL");
	snprintf(rev->valid, COMM_S, "NULL");
	rev->next = '\0';
}

void
init_preferred_a_struct(preferred_a_t *prefer)
{
	prefer->prefa_id = 0;
	prefer->ip_addr = 0;
	prefer->record_id = 0;
	snprintf(prefer->ip, COMM_S, "NULL");
	snprintf(prefer->fqdn, COMM_S, "NULL");
	prefer->next = '\0';
}

void
init_dbdata_struct(dbdata_t *data)
{
	data->fields.number = 0;
	data->args.number = 0;
	data->next = '\0';
}

void
dnsa_clean_list(dnsa_t *dnsa)
{
	if (dnsa->zones)
		dnsa_clean_zones(dnsa->zones);
	if (dnsa->rev_zones)
		dnsa_clean_rev_zones(dnsa->rev_zones);
	if (dnsa->records)
		dnsa_clean_records(dnsa->records);
	if (dnsa->rev_records)
		dnsa_clean_rev_records(dnsa->rev_records);
	if (dnsa->prefer)
		dnsa_clean_prefer(dnsa->prefer);
	free(dnsa);
}

void
dnsa_clean_zones(zone_info_t *list)
{
	zone_info_t *zone, *next;

	if (list)
		zone = list;
	else
		return;
	if (zone->next)
		next = zone->next;
	else
		next = '\0';
	while (zone) {
		free(zone);
		if (next)
			zone = next;
		else
			return;
		if (zone->next)
			next = zone->next;
		else
			next = '\0';
	}
}

void
dnsa_clean_rev_zones(rev_zone_info_t *list)
{
	rev_zone_info_t *zone, *next;

	if (list)
		zone = list;
	else
		return;
	if (zone->next)
		next = zone->next;
	else
		next = '\0';
	while (zone) {
		free(zone);
		if (next)
			zone = next;
		else
			return;
		if (zone->next)
			next = zone->next;
		else
			next = '\0';
	}
}

void
dnsa_clean_records(record_row_t *list)
{
	record_row_t *rec, *next;

	if (list)
		rec = list;
	else
		return;
	if (rec->next)
		next = rec->next;
	else
		next = '\0';
	while (rec) {
		free(rec);
		if (next)
			rec = next;
		else
			return;
		if (rec->next)
			next = rec->next;
		else
			next = '\0';
	}
}

void
dnsa_clean_rev_records(rev_record_row_t *list)
{
	rev_record_row_t *rec, *next;

	if (list)
		rec = list;
	else
		return;
	if (rec->next)
		next = rec->next;
	else
		next = '\0';
	while (rec) {
		free(rec);
		if (next)
			rec = next;
		else
			return;
		if (rec->next)
			next = rec->next;
		else
			next = '\0';
	}
}

void
dnsa_clean_prefer(preferred_a_t *list)
{
	preferred_a_t *prefer, *next;

	if (list)
		prefer = list;
	else
		return;
	if (prefer->next)
		next = prefer->next;
	else
		next = '\0';
	while (prefer) {
		free(prefer);
		if (next)
			prefer = next;
		else
			return;
		if (prefer->next)
			next = prefer->next;
		else
			next = '\0';
	}
}

void
dnsa_clean_dbdata_list(dbdata_t *list)
{
	dbdata_t *data, *next;
	
	if (list)
		data = list;
	else
		return;
	if (data->next)
		next = data->next;
	else
		next = '\0';
	while (data) {
		free(data);
		if (next)
			data = next;
		else
			return;
		if (data->next)
			next = data->next;
		else
			next = '\0';
	}
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
		report_error(MALLOC_FAIL, "line in get_in_addr_string2");
	if (!(classless = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "classless in get_in_addr_string2");

	snprintf(line, len, "%s", range);
	switch (prefix) {
		case 24:
			tmp = strrchr(line, c);
			*tmp = '\0';
			while ((tmp = strrchr(line, c))) {
				++tmp;
				len = strlen(tmp);
				strncat(in_addr, tmp, len);
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		case 16:
			tmp = strrchr(line, c);
			*tmp = '\0';
			tmp = strrchr(line, c);
			*tmp = '\0';
			while ((tmp = strrchr(line, c))) {
				++tmp;
				len = strlen(tmp);
				strncat(in_addr, tmp, len);
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		case 8:
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
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		case 25: case 26: case 27: case 28: case 29: case 30:
		case 31: case 32:
			tmp = strrchr(line, c);
			++tmp;
			len = strlen(tmp);
			strncat(in_addr, tmp, len);
			strncat(in_addr, ".", 1);
			--tmp;
			*tmp = '\0';
			snprintf(classless, CONF_S, "/%lu.", prefix);
			len = strlen(classless);
			strncat(in_addr, classless, len);
			while ((tmp = strrchr(line, c))) {
				++tmp;
				len = strlen(tmp);
				strncat(in_addr, tmp, len);
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		default:
			break;
	}
	len = strlen(line);
	strncat(in_addr, line, len);
	len = strlen(louisa);
	strncat(in_addr, louisa, len);
	free(line);
	free(classless);
}

void 
print_rev_zone_info(rev_zone_info_t *rzi)
{
	printf("rev_zone-id: %lu\n", rzi->rev_zone_id);
	printf("prefix: %lu\n", rzi->prefix);
	printf("owner: %lu\n", rzi->owner);
	printf("start_ip: %lu\n", rzi->start_ip);
	printf("end_ip: %lu\n", rzi->end_ip);
	printf("serial: %lu\n", rzi->serial);
	printf("refresh: %lu\n", rzi->refresh);
	printf("retry: %lu\n", rzi->retry);
	printf("expire: %lu\n", rzi->expire);
	printf("ttl: %lu\n", rzi->ttl);
	printf("net_range: %s\n", rzi->net_range);
	printf("net_start: %s\n", rzi->net_start);
	printf("net_finish: %s\n", rzi->net_finish);
	printf("pri_dns: %s\n", rzi->pri_dns);
	printf("sec_dns: %s\n", rzi->sec_dns);
	printf("valid: %s\n", rzi->valid);
	printf("updated: %s\n", rzi->updated);
	printf("hostmaster: %s\n", rzi->hostmaster);
}
