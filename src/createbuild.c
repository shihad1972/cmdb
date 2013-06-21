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
 *  createbuild.c
 * 
 *  Contains functions to display / add / modify / delete build details in the
 *  database for the main cbc program.
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "build.h"

#ifdef HAVE_DNSA

# include "cmdb_dnsa.h"
# include "dnsa_base_sql.h"
# include "cbc_dnsa.h"

#endif /* HAVE_DNSA */

int
create_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml)
{
	int retval = NONE, query = NONE;
	cbc_s *cbc, *details;
	cbc_build_s *build, *list;
	cbc_build_ip_s *bip;
	
	if (!(cbc = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "cbc in display_build_config");
	if (!(details = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "details in display_build_config");
	if (!(build = malloc(sizeof(cbc_build_s))))
		report_error(MALLOC_FAIL, "build in display_build_config");
	init_cbc_struct(cbc);
	init_cbc_struct(details);
	init_build_struct(build);
#ifndef CLEAN_CREATE_BUILD_CONFIG
# define CLEAN_CREATE_BUILD_CONFIG(retval) { \
	clean_cbc_struct(cbc);               \
	free(details);                       \
	free(build);                         \
	return retval;                       \
}
#endif
	details->build = build;
	query = BUILD_DOMAIN | BUILD_IP | BUILD_TYPE | BUILD_OS | BUILD |
	  CSERVER | LOCALE | DPART | VARIENT | SSCHEME;
	if ((retval = cbc_run_multiple_query(cbt, cbc, query)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(MY_QUERY_FAIL);
	}
	if ((retval = cbc_get_server(cml, cbc, details)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(SERVER_NOT_FOUND);
	}
	list = cbc->build;
	while (list) {
		if (list->server_id == details->server->server_id) {
			printf("Server %s already has a build in the database\n",
			       details->server->name);
			CLEAN_CREATE_BUILD_CONFIG(BUILD_IN_DATABASE);
		}
		list = list->next;
	}
	if ((retval = cbc_get_os(cml, cbc, details)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(OS_NOT_FOUND);
	}
	if ((retval = cbc_get_build_domain(cml, cbc, details)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(BUILD_DOMAIN_NOT_FOUND);
	}
	if ((retval = cbc_get_seed_scheme(cml, cbc, details)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(SCHEME_NOT_FOUND);
	}
	if ((retval = cbc_get_varient(cml, cbc, details)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(VARIENT_NOT_FOUND);
	}
	if ((retval = cbc_get_locale(cml, cbc, details)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(LOCALE_NOT_FOUND);
	}
	if ((retval = cbc_get_build_config(cbc, details, build)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(retval);
	}
	if ((retval = cbc_get_network_info(cbt, cml, build)) != 0) {
		CLEAN_CREATE_BUILD_CONFIG(retval);
	}
	bip = cbc->bip;
	while (bip) {
		if ((strncmp(bip->host, cml->name, MAC_S) == 0) &&
		    (strncmp(bip->domain, cml->build_domain, RBUFF_S) == 0)) {
			printf("Build ip for %s.%s in database\n", 
				bip->host, bip->domain);
			break;
		}
		bip = bip->next;
	}
	if (!(bip)) {
		if ((retval = cbc_get_build_ip(cbt, cml, details)) != 0) {
			CLEAN_CREATE_BUILD_CONFIG(NO_BUILD_IP);
		}
		if ((retval = cbc_run_insert(cbt, details, BUILD_IPS)) != 0) {
			clean_build_ip(details->bip);
			clean_pre_part(details->dpart);
			printf("Unable to insert IP into database\n");
			CLEAN_CREATE_BUILD_CONFIG(retval);
		}
		clean_build_ip(details->bip);
		clean_build_ip(cbc->bip);
		cbc->bip = '\0';
		if ((retval = cbc_run_query(cbt, cbc, BUILD_IP)) != 0) {
			CLEAN_CREATE_BUILD_CONFIG(retval);
		}
	}
	bip = cbc->bip;
	while (bip) {
		if ((strncmp(bip->host, cml->name, MAC_S) == 0) &&
		    (strncmp(bip->domain, cml->build_domain, RBUFF_S) == 0)) {
			build->ip_id = bip->ip_id;
			details->bip = bip;
		}
		bip = bip->next;
	}
	if ((retval = check_for_disk_device(cbt, details)) != 0) {
		printf("Unable to find a disk device for the server\n");
		CLEAN_CREATE_BUILD_CONFIG(retval);
	}
#ifdef HAVE_DNSA
	int dret = NONE;
	dret = check_for_build_ip_in_dns(cbt, cml, details);
	if (dret == 1)
		printf("Unable to add IP to DNS\n");
	else if (dret == 2)
		printf("Hostname already in domain\n");
	else if (dret == 3)
		printf("Hostname modified in DNS\n");
	else if (dret == 4)
		printf("Domain not in database??\n");
	else if (dret == 0) {
		printf("Hostname added to DNS\n");
		write_zone_and_reload_nameserver(cml);
	} else
		return retval;
#endif /* HAVE_DNSA */
	if ((retval = cbc_run_insert(cbt, details, BUILDS)) != 0)
		printf("Unable to add build to database\n");
	else
		printf("Build added to database\n");
	clean_pre_part(details->dpart);
	CLEAN_CREATE_BUILD_CONFIG(retval);
#ifdef CLEAN_CREATE_BUILD_CONFIG
# undef CLEAN_CREATE_BUILD_CONFIG
#endif
}

int
cbc_get_os(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_build_os_s *list = cbc->bos;
	while (list) {
		if (((strncmp(list->os, cml->os, MAC_S) == 0) ||
		     (strncmp(list->alias, cml->os, MAC_S) == 0)) &&
		     (strncmp(list->version, cml->os_version, MAC_S) == 0) &&
		     (strncmp(list->arch, cml->arch, RANGE_S) == 0))
			details->bos = list;
		list = list->next;
	}
	if (!details->bos)
		retval = OS_NOT_FOUND;
	return retval;
}

int
cbc_get_build_domain(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_build_domain_s *list = cbc->bdom;
	while (list) {
		if (strncmp(list->domain, cml->build_domain, RBUFF_S) == 0)
			details->bdom = list;
		list = list->next;
	}
	if (!details->bdom)
		retval = BUILD_DOMAIN_NOT_FOUND;
	return retval;
}

int
cbc_get_build_ip(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_s *details)
{
	int retval = NONE;
	unsigned long int ip_addr = details->bdom->start_ip;
	cbc_build_ip_s *ip = '\0';
	dbdata_s *data = '\0', *list = '\0';

	if (!(ip = malloc(sizeof(cbc_build_ip_s))))
		report_error(MALLOC_FAIL, "ip in cbc_get_build_ip");
	init_build_ip(ip);
	cbc_init_initial_dbdata(&data, IP_ON_BD_ID);
	data->args.number = details->bdom->bd_id;
	retval = cbc_run_search(cbt, data, IP_ON_BD_ID);
#ifdef HAVE_DNSA
	get_dns_ip_list(cbt, details, data);
#endif
	if ((retval = cbc_find_build_ip(&ip_addr, details, data, list)) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	cbc_fill_build_ip(ip, cml, details->bdom, ip_addr, details->server);
	details->bip = ip;
	clean_dbdata_struct(data);
	return retval;
}

int
cbc_find_build_ip(unsigned long int *ip_addr, cbc_s *details, dbdata_s *data, dbdata_s *list)
{
	int retval, i;
	while (*ip_addr <= details->bdom->end_ip) {
		i = FALSE;
		list = data;
		while (list) {
			if (list->fields.number == *ip_addr)
				i = TRUE;
			list = list->next;
		}
		if (i == FALSE)
			break;
		++*ip_addr;
	}
	if (*ip_addr > details->bdom->end_ip)
		retval = NO_BUILD_IP;
	else
		retval = NONE;
	return retval;
}

int
cbc_get_varient(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_varient_s *list = cbc->varient;
	while (list) {
		if ((strncmp(list->varient, cml->varient, CONF_S) == 0) ||
		    (strncmp(list->valias, cml->varient, MAC_S) == 0))
			details->varient = list;
		list = list->next;
	}
	if (!details->varient)
		retval = VARIENT_NOT_FOUND;
	return retval;
}

int
cbc_get_seed_scheme(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_seed_scheme_s *list = cbc->sscheme;
	while (list) {
		if (strncmp(list->name, cml->partition, CONF_S) == 0)
			details->sscheme = list;
		list = list->next;
	}
	if (!details->sscheme)
		retval = SCHEME_NOT_FOUND;
	return retval;
}

int
cbc_get_locale(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_locale_s *list = cbc->locale;
	if (cml->locale != 0) {
		while (list) {
			if (list->locale_id == cml->locale)
				details->locale = list;
			list = list->next;
		}
	} else {
		while (list) {
			if (list->os_id == details->bos->os_id)
				details->locale = list;
			list = list->next;
		}
	}
	if (!details->locale)
		retval = LOCALE_NOT_FOUND;
	return retval;
}

int
cbc_get_build_config(cbc_s *cbc, cbc_s *details, cbc_build_s *build)
{
	int retval = NONE;

	if ((retval = cbc_get_build_partitons(cbc, details)) != 0)
		return retval;
	build->varient_id = details->varient->varient_id;
	build->server_id = details->server->server_id;
	build->os_id = details->bos->os_id;
	build->locale_id = details->locale->locale_id;
	build->def_scheme_id = details->sscheme->def_scheme_id;

	return retval;
}

int
cbc_get_build_partitons(cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	unsigned long int scheme_id = details->sscheme->def_scheme_id;
	cbc_pre_part_s *part, *new, *old, *next, *list = cbc->dpart;

	part = new = old = next = '\0';
	while (list) {
		if (list->link_id.def_scheme_id == scheme_id) {
			if (!(new = malloc(sizeof(cbc_pre_part_s))))
				report_error(MALLOC_FAIL, "new in get build part");
			memcpy(new, list, sizeof(cbc_pre_part_s));
			new->next = '\0';
			if (!part) {
				part = new;
			} else if (!part->next) {
				part->next = new;
			} else {
				next = part->next;
				old = part;
				while (next) {
					old = next;
					next = next->next;
				}
				old->next = new;
			}
		}
		list = list->next;
	}
	if (!part)
		retval = PARTITIONS_NOT_FOUND;
	details->dpart = part;
	return retval;
}

int
cbc_get_network_info(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = NONE;
	dbdata_s *data, *list;

	cbc_init_initial_dbdata(&data, NETWORK_CARD);
	data->args.number = build->server_id;
	if ((retval = cbc_run_search(cbt, data, NETWORK_CARD)) == 0) {
		clean_dbdata_struct(data);
		printf("No network hardware found for server %s\n",
		  cml->name);
		return NO_NETWORK_HARDWARE;
	}
	if (strncmp(cml->netcard, "NULL", COMM_S) == 0) {
		printf("No network card specified. Choosing automatically\n");
		snprintf(build->mac_addr, MAC_S, "%s", data->fields.text);
		snprintf(build->net_int, RANGE_S, "%s", data->next->fields.text);
		clean_dbdata_struct(data);
		return NONE;
	}
	list = data;
	while (list) {
		if (strncmp(cml->netcard, list->next->fields.text, HOST_S) == 0) {
			snprintf(build->mac_addr, MAC_S, "%s", data->fields.text);
			snprintf(build->net_int, RANGE_S, "%s", data->next->fields.text);
			break;
		}
		if (list->next)
			list = list->next->next;
	}
	if (list)
		retval = NONE;
	else
		retval = NO_NETWORK_HARDWARE;
	clean_dbdata_struct(data);
	return retval;
}

void
cbc_fill_build_ip(cbc_build_ip_s *ip, cbc_comm_line_s *cml, cbc_build_domain_s *bdom, unsigned long int ip_addr, cbc_server_s *server)
{
	ip->ip = ip_addr;
	/* This will trim cml->name */
	snprintf(ip->host, MAC_S, "%s", cml->name);
	snprintf(ip->domain, RBUFF_S, "%s", bdom->domain);
	ip->bd_id = bdom->bd_id;
	ip->server_id = server->server_id;
}

int
check_for_disk_device(cbc_config_s *cbc, cbc_s *details)
{
	int retval = NONE;
	dbdata_s *data;
	cbc_disk_dev_s *disk;

	if (!(disk = malloc(sizeof(cbc_disk_dev_s))))
		report_error(MALLOC_FAIL, "disk in check_for_disk_device");
	init_disk_dev(disk);
	details->diskd = disk;
	cbc_init_initial_dbdata(&data, HARD_DISK_DEV);
	data->args.number = disk->server_id = details->server->server_id;
	if ((retval = cbc_run_search(cbc, data, HARD_DISK_DEV)) == 0) {
		free(data);
		printf("You need to add a hard disk to build %s\n",
		       details->server->name);
		return NO_BASIC_DISK;
	} else if (retval > 1 )
		printf("Using fist disk /dev/%s\n", data->fields.text);
	snprintf(disk->device, HOST_S, "/dev/%s", data->fields.text);
	free(data);
	disk->lvm = details->sscheme->lvm;
	if ((retval = cbc_run_insert(cbc, details, DISK_DEVS)) != 0) {
		printf("Unable to insert disk %s into database\n",
		       disk->device);
		free(disk);
		return retval;
	}
	details->diskd = '\0';
	free(disk);
	return retval;
}

int
modify_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml)
{
	int retval = NONE;

	return retval;
}

int
remove_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml)
{
	int retval = NONE;

	return retval;
}
