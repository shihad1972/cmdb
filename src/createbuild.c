/* 
 *
 *  cbc: Create Build Configuration
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
 *  createbuild.c
 * 
 *  Contains functions to display / add / modify / delete build details in the
 *  database for the main cbc program.
 * 
 *  (C) Iain M. Conochie 2012 - 2014
 * 
 */
#include <config.h>
#include <configmake.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <sys/stat.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_common.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "build.h"

#ifdef HAVE_DNSA

# include "cmdb_dnsa.h"
# include "dnsa_base_sql.h"
# include "cbc_dnsa.h"

#endif /* HAVE_DNSA */

static int
check_build_ip(ailsa_cmdb_s *cbs, cbc_comm_line_s *cml, unsigned long int *ip, AILLIST *dom);

int
create_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	if (!(cbt) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *b = ailsa_db_data_list_init();
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_build_id_to_list(cml->name, l)) != 0)
			goto cleanup;
	if (l->total > 0) {
		ailsa_syslog(LOG_INFO, "Build already exists");
		goto cleanup;
	}
	if ((retval = cmdb_add_server_id_to_list(cml->name, b)) != 0)
		goto cleanup;
	if ((retval = cbc_get_network_info(cbt, cml, build)) != 0)
		goto cleanup;
	if ((retval = cbc_get_varient(cbt, cml, build)) != 0)
		goto cleanup;
	if ((retval = cbc_get_os(cbt, cml, build)) != 0)
		goto cleanup;
	if ((retval = cbc_get_locale(cbt, cml, build)) != 0)
		goto cleanup;
	if ((retval = cbc_get_partition_scheme(cbt, cml, build)) != 0)
		goto cleanup;
// Now the searches will add stuff to the DB. We need to make sure we search first!
	if ((retval = cbc_add_disk(cbt, cml, build)) != 0)
		goto cleanup;
	if ((retval = cbc_get_ip_info(cbt, cml, build)) != 0)
		goto cleanup;
	build->cuser = build->muser = (unsigned long int)getuid();
	cbc->build = build;
	if ((retval = cbc_run_insert(cbt, cbc, BUILDS)) != 0) {
		fprintf(stderr, "Cannot insert build into db\n");
	} else {
		printf("Build inserted into db\n");
		set_build_domain_updated(cbt, cml->build_domain);
	}
	goto cleanup;

	cleanup:
		ailsa_list_full_clean(b);
		ailsa_list_full_clean(l);
		return retval;
}

int
cbc_get_network_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0, query = MAC_ON_SERVER_ID_DEV;
	unsigned int max;
	dbdata_s *data;

	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = build->server_id;
	if (strncmp(cml->netcard, "NULL", COMM_S) == 0) {
		fprintf(stderr, "No network card given. Using eth0\n");
		snprintf(cml->netcard, HOST_S, "eth0");
	}
	snprintf(data->next->args.text, COMM_S, "%s", cml->netcard);
	if ((retval = cbc_run_search(cbt, data, query)) == 0) {
		retval = NO_NETWORK_HARDWARE;
		goto cleanup;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple instances of %s?? Using first\n",
		 data->next->args.text);
		retval = 0;
	} else
		retval = 0;
	snprintf(build->mac_addr, MAC_S, "%s", data->fields.text);
	snprintf(build->net_int, RANGE_S, "%s", cml->netcard);
	goto cleanup;

	cleanup:
		clean_dbdata_struct(data);
		return retval;
}

int
cbc_get_varient(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0, query = VARIENT_ID_ON_VARIENT;
	dbdata_s *data;

	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, 1);
	snprintf(data->args.text, HOST_S, "%s", cml->varient);
	retval = cbc_run_search(cbt, data, query);
	if (retval == 0) {
		query = VARIENT_ID_ON_VALIAS;
		retval = cbc_run_search(cbt, data, query);
	}
	if (retval == 0) {
		retval = VARIENT_NOT_FOUND;
		goto cleanup;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple varients found for %s. Using 1st one\n",
		 cml->varient);
	}
	build->varient_id = data->fields.number;
	retval = 0;
	goto cleanup;

	cleanup:
		clean_dbdata_struct(data);
		return retval;
}

int
cbc_get_os(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0, query = OS_ID_ON_NAME;
	unsigned int max;
	dbdata_s *data;

	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, MAC_S, "%s", cml->os);
	snprintf(data->next->args.text, MAC_S, "%s", cml->os_version);
	snprintf(data->next->next->args.text, MAC_S, "%s", cml->arch);
	retval = cbc_run_search(cbt, data, query);
	if (retval == 0) {
		query = OS_ID_ON_ALIAS;
		retval = cbc_run_search(cbt, data, query);
	}
	if (retval == 0) {
		query = OS_ID_ON_NAME_VER_ALIAS;
		retval = cbc_run_search(cbt, data, query);
	}
	if (retval == 0) {
		query = OS_ID_ON_ALIAS_VER_ALIAS;
		retval = cbc_run_search(cbt, data, query);
	}
	if (retval > 1) {
		retval = 0;
		fprintf(stderr, "Multiple OS's found. Using first one\n");
	} else if (retval == 0) {
		retval = OS_NOT_FOUND;
		goto cleanup;
	} else {
		retval = 0;
	}
	build->os_id = data->fields.number;
	goto cleanup;

	cleanup:
		clean_dbdata_struct(data);
		return retval;
}

int
cbc_get_locale(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0;
	int query = GET_DEFAULT_LOCALE;
	unsigned long int isdefault;

	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	if (strncmp(cml->locale, "NULL", COMM_S) != 0) {
		retval = get_locale_id(cbt, cml->locale, &(build->locale_id));
		return retval;
	}
	if ((retval = get_default_id(cbt, query, NULL, &isdefault)) != 0) {
		fprintf(stderr, "Cannot find default locale\n");
		return retval;
	}
	build->locale_id = isdefault;
	return retval;
}

int
cbc_get_partition_scheme(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0, query = DEF_SCHEME_ID_ON_SCH_NAME;
	dbdata_s *data;

	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, 1);
	snprintf(data->args.text, CONF_S, "%s", cml->partition);
	retval = cbc_run_search(cbt, data, query);
	if (retval == 0) {
		retval = SCHEME_NOT_FOUND;
		goto cleanup;
	} else if (retval > 1)
		fprintf(stderr, "Multiple schemes found for %s. Using 1st one\n",
		 cml->partition);
	retval = 0;
	build->def_scheme_id = data->fields.number;
	goto cleanup;

	cleanup:
		clean_dbdata_struct(data);
		return retval;
}

int
cbc_get_ip_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0;
	cbc_s *cbc;
	cbc_build_ip_s *bip;
	unsigned long int ip[4]; // ip[0] = id, [1] = start, [2] = end, [3] = ip
	unsigned long int dnsip;

	memset(&ip, 0, sizeof(ip));
	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	if ((retval = cbc_search_for_ip(cbt, cml, build)) != 0)
		return 0;
	if ((retval = cbc_get_build_dom_info(cbt, cml, ip)) != 0)
		return retval;
	check_ip_in_dns(&(ip[3]), cml->name, cml->build_domain);
	dnsip = ip[3];
	if ((retval = cbc_check_in_use_ip(cbt, cml, ip)) != 0)
		return retval;
	if ((dnsip != 0) && (dnsip != ip[3]))
		fprintf(stderr, "We could not use the DNS ip address for this host\n");
	if (!(bip = malloc(sizeof(cbc_build_ip_s))))
		report_error(MALLOC_FAIL, "bip in cbc_get_ip_info");
	if (!(cbc = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "cbc in cbc_get_ip_info");
	init_cbc_struct(cbc);
	init_build_ip(bip);
	cbc->bip = bip;
	snprintf(bip->host, HOST_S, "%s", cml->name);
	snprintf(bip->domain, RBUFF_S, "%s", cml->build_domain);
	bip->ip = ip[3];
	bip->bd_id = ip[0];
	bip->server_id = build->server_id;
	bip->cuser = bip->muser = (unsigned long int)getuid();
	if ((retval = cbc_run_insert(cbt, cbc, BUILD_IPS)) != 0)
		goto cleanup;
		/*
#ifdef HAVE_DNSA
	int dret = 0;
	if ((dret = check_for_build_ip_in_dns(cbt, cml, cbc)) == 0) {
		printf("Hostname added into dns\n");
		if ((dret = cmdb_validate_zone(cbt, FORWARD_ZONE, cml->build_domain)) != 0)
			ailsa_syslog(LOG_INFO, "Cannot reload name server");
	} else if (dret == 1)
		printf("Unable to add IP to DNS\n");
	else if (dret == 2)
		printf("Hostname already in domain\n");
	else if (dret == 3)
		printf("Hostname modified in DNS\n");
	else if (dret == 4)
		printf("Domain not in database??\n");
	else
		goto cleanup;
#endif // HAVE_DNSA
*/
	cbc_search_for_ip(cbt, cml, build);
	goto cleanup;

	cleanup:
		clean_cbc_struct(cbc);
		return retval;
}

int
cbc_search_for_ip(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	if (!(cbt) || !(cml) || !(build))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *ip = ailsa_db_data_list_init();

	if ((retval = cmdb_add_number_to_list(build->server_id, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server_id to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, BUILD_IP_ON_SERVER_ID, list, ip)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_IP_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (ip->total > 1)
		fprintf(stderr, "Multiple build IP's found for server %s. Using 1st one\n",
		 cml->name);
	else if (ip->total > 0)
		build->ip_id = ((ailsa_data_s *)ip->head->data)->data->number;

	cleanup:
		ailsa_list_full_clean(list);
		ailsa_list_full_clean(ip);
		return retval;
}

int
cbc_check_in_use_ip(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, uli_t *ip)
{
	if (!(cbt) || !(cml) || !(ip))
		return AILSA_NO_DATA;
	int retval;
	char ip_addr[SERVICE_LEN];
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *dom = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d;
	uint32_t addr;

	if ((retval = cmdb_add_number_to_list(*ip, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add number to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, BUILD_IP_ON_BUILD_DOMAIN_ID, list, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_IP_ON_BUILD_DOMAIN_ID query failed");
		goto cleanup;
	}
	if (ip[3] == 0) {// dns query did not find an IP so start at start
		ip[3] = ip[1];
	}
	if ((retval = check_build_ip(cbt, cml, ip, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot find a build IP to use");
		goto cleanup;
	}
	e = dom->head;
	while (e) {
		d = e->data;
		if (d->data->number == ip[3]) {
			ip[3]++;
			e = dom->head;
			continue;
		}
		e = e->next;
	}
	if (ip[3] > ip[2]) {
		addr = htonl((uint32_t)ip[3]);
		if (!(inet_ntop(AF_INET, &addr, ip_addr, SERVICE_LEN))) {
			ailsa_syslog(LOG_ERR, "Cannot get IP address text in cbc_check_in_use_ip: %s", strerror(errno));
			goto cleanup;
		}
		ailsa_syslog(LOG_ERR, "Build IP is out of range: %s", ip_addr);
		goto cleanup;
	}

	goto cleanup;

	cleanup:
		ailsa_list_full_clean(list);
		ailsa_list_full_clean(dom);
		return retval;
}

static int
check_build_ip(ailsa_cmdb_s *cbs, cbc_comm_line_s *cml, unsigned long int *ip, AILLIST *dom)
{
	if (!(cbs) || !(ip) || !(dom))
		return AILSA_NO_DATA;
	int retval;
#ifdef HAVE_DNSA
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	char ip_addr[SERVICE_LEN];
	uint32_t addr, tmp;
#endif // HAVE_DNSA
	AILELEM *e = dom->head;
	ailsa_data_s *d;
	tmp = 0;
	while(e) {
#ifdef HAVE_DNSA
		addr = htonl((uint32_t)ip[3]);
		if (addr != tmp) {
			tmp = addr;
			if (!(inet_ntop(AF_INET, &addr, ip_addr, SERVICE_LEN))) {
				ailsa_syslog(LOG_ERR, "Cannot get IP address text in check_build_ip: %s", strerror(errno));
				goto cleanup;
			}
			if ((retval = cmdb_add_string_to_list(ip_addr, l)) != 0) {
				ailsa_syslog(LOG_ERR, "Cannot add ip to list: %s", ip_addr);
				goto cleanup;
			}
			if ((retval = ailsa_argument_query(cbs, A_RECORDS_WITH_IP, l, r)) != 0) {
				ailsa_syslog(LOG_ERR, "A_RECORDS_WITH_IP query failed in check_build_ip");
				goto cleanup;
			}
			if (r->total > 0) {
				e = r->head;
				d = e->data;
				if (strncmp(cml->name, d->data->text, HOST_LEN) == 0) {
					e = e->next;
					d = e->data;
					if (strncmp(cml->build_domain, d->data->text, DOMAIN_LEN) == 0)
						break;
				}
				ip[3]++;
				e = dom->head;
				continue;
			}
		}
#endif // HAVE_DNSA
		d = e->data;
		if (d->data->number == ip[3]) {
			ip[3]++;
			e = dom->head;
			continue;
		}
		e = e->next;
	}

	cleanup:
#ifdef HAVE_DNSA
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
#endif // HAVE_DNSA
		return retval;
}
int
cbc_find_build_ip(unsigned long int *ipinfo, dbdata_s *data)
{
	int retval = 0, i;
	dbdata_s *list;

	if (!(ipinfo) || !(data))
		return CBC_NO_DATA;
	if (*(ipinfo + 3) > *(ipinfo + 2))
		return BUILD_IP_OUT_OF_RANGE;
	while (*(ipinfo + 3) <= *(ipinfo + 2)) {
		i = FALSE;
		list = data;
		while (list) {
			if (list->fields.number == *(ipinfo + 3))
				i = TRUE;
			list = list->next;
		}
		if (i == FALSE)
			break;
		++*(ipinfo + 3);
	}
	if (*(ipinfo + 3) > *(ipinfo + 2))
		retval = NO_BUILD_IP;
	else
		retval = 0;
	return retval;
}

int
cbc_get_build_dom_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, uli_t *bd)
{
	if (!(cbt) || !(cml) || !(bd))
		return AILSA_NO_DATA;
	int retval;
	unsigned long int *ip_list = bd;
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *bdom = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cml->build_domain, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, BUILD_DOMAIN_NET_INFO, list, bdom)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_NET_INFO query failed");
		goto cleanup;
	}
	if (bdom->total == 0) {
		retval = BUILD_DOMAIN_NOT_FOUND;
		goto cleanup;
	}
	ip_list[0] = ((ailsa_data_s *)bdom->head->data)->data->number;
	ip_list[1] = ((ailsa_data_s *)bdom->head->next->data)->data->number;
	ip_list[2] = ((ailsa_data_s *)bdom->head->next->next->data)->data->number;

	cleanup:
		ailsa_list_full_clean(list);
		ailsa_list_full_clean(bdom);
		return retval;
}

int
cbc_add_disk(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0, query = DISK_DEV_ON_SERVER_ID_DEV;
	unsigned int max;
	dbdata_s *data = NULL, *lvm = 0;
	cbc_s *cbc = NULL;
	cbc_disk_dev_s *disk = NULL;

	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	if ((retval = cbc_search_for_disk(cbt, cml, build)) == 0)
		return retval;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = build->server_id;
	snprintf(data->next->args.text, HOST_S, "%s", cml->harddisk);
	if ((retval = cbc_run_search(cbt, data, query)) == 0) {
		retval = NO_HARD_DISK_DEV;
		goto cleanup;
	} else if (retval > 1)
		fprintf(stderr, "Multiple disks %s for server\n", cml->harddisk);
	init_multi_dbdata_struct(&lvm, 1);
	lvm->args.number = build->def_scheme_id;
	query = LVM_ON_DEF_SCHEME_ID;
	if ((retval = cbc_run_search(cbt, lvm, query)) == 0) {
		retval = PARTITIONS_NOT_FOUND;
		goto cleanup;
	}
	if (!(disk = malloc(sizeof(cbc_disk_dev_s))))
		report_error(MALLOC_FAIL, "disk in cbc_add_disk");
	if (!(cbc = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "cbc in cbc_add_disk");
	init_cbc_struct(cbc);
	init_disk_dev(disk);
	cbc->diskd = disk;
	snprintf(disk->device, HOST_S, "/dev/%s", cml->harddisk);
	disk->lvm = lvm->fields.small;
	disk->server_id = build->server_id;
	if ((retval = cbc_run_insert(cbt, cbc, DISK_DEVS)) != 0)
		fprintf(stderr, "Cannot insert disk device %s\n", cml->harddisk);
	else
		printf("Inserted disk device into db\n");
	goto cleanup;

	cleanup:
		clean_cbc_struct(cbc);
		clean_dbdata_struct(data);
		clean_dbdata_struct(lvm);
		return retval;
}

int
cbc_search_for_disk(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build)
{
	int retval = 0, query = BASIC_PART;
	unsigned int max;
	dbdata_s *data;

	if (!(cbt) || !(cml) || !(build))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = build->server_id;
	if ((retval = cbc_run_search(cbt, data, query)) == 0)
		retval = 1;
	else
		retval = 0;
	clean_dbdata_struct(data);
	return retval;
}

int
modify_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	char *os[3];
	int retval = NONE, type = NONE;
	unsigned long int sid = 0, vid = 0, osid = 0, dsid = 0, bid = 0;
	dbdata_s *data;

	if (cml->server_id == 0) {
		if ((retval = get_server_id(cbt, cml->name, &sid)) != 0)
			return retval;
		cml->server_id = sid;
	} else {
		sid = cml->server_id;
	}
	if ((retval = get_build_id(cbt, cml->server_id, cml->name, &bid)) != 0)
		return retval;
	if (strncmp(cml->varient, "NULL", COMM_S) != 0)
		if ((retval = get_varient_id(cbt, cml->varient, &vid)) != 0)
			return retval;
	if (strncmp(cml->os, "NULL", COMM_S) != 0) {
		os[0] = strndup(cml->arch, MAC_S);
		os[1] = strndup(cml->os_version, MAC_S);
		os[2] = strndup(cml->os, CONF_S);
		if ((retval = get_os_id(cbt, os, &osid)) != 0) {
			if (retval == OS_NOT_FOUND)
				fprintf(stderr, "Build os not found\n");
			return retval;
		}
	}
	if (strncmp(cml->partition, "NULL", COMM_S) != 0)
		if ((retval = get_scheme_id(cbt, cml->partition, &dsid)) != 0)
			return retval;
	unsigned long int ids[4] = { vid, osid, dsid, sid };
	if (strncmp(cml->build_domain, "NULL", COMM_S) != 0)
		return CANNOT_MODIFY_BUILD_DOMAIN;
	if (strncmp(cml->locale, "NULL", COMM_S) != 0)
		return LOCALE_NOT_IMPLEMENTED;
	if ((type = get_modify_query(ids)) == 0) {
		printf("No modifiers?\n");
		return NO_MODIFIERS;
	}
	init_multi_dbdata_struct(&data, cbc_update_args[type]);
	cbc_prep_update_dbdata(data, type, ids);
	if ((retval = cbc_run_update(cbt, data, type)) == 1) {
		printf("Build updated\n");
		retval = NONE;
	} else if (retval == 0) {
		printf("No build updated. Does server %s have a build?\n",
		       cml->name);
		retval = SERVER_BUILD_NOT_FOUND;
	} else {
		printf("Multiple builds??\n");
		retval = MULTIPLE_SERVER_BUILDS;
	}
	clean_dbdata_struct(data);
	return retval;
}

int
remove_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
#ifndef CLEAN_REMOVE_BUILD_CONFIG
# define CLEAN_REMOVE_BUILD_CONFIG(retval) { \
		free(data);                  \
		free(cbc);                   \
		free(scratch);               \
		return retval;               \
}
#endif
	int retval = NONE;
	dbdata_s *data;
	cbc_s *cbc, *scratch;

	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in remove_build_config");
	if (!(cbc = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "cbc in remove_build_config");
	if (!(scratch = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "cbc in remove_build_config");
	init_dbdata_struct(data);
	init_cbc_struct(cbc);
	init_cbc_struct(scratch);
	if ((retval = cbc_run_query(cbt, cbc, CSERVER)) != 0)
		CLEAN_REMOVE_BUILD_CONFIG(retval);
	if ((retval = cbc_get_server(cml, cbc, scratch)) != 0)
		CLEAN_REMOVE_BUILD_CONFIG(retval);
	data->args.number = scratch->server->server_id;
	if ((retval = cbc_run_delete(cbt, data, BUILD_ON_SERVER_ID)) == 1)
		printf("Build for server %s deleted\n", cml->name);
	else if (retval == 0)
		printf("Seems like there was no build for %s\n", cml->name);
	else
		printf("WOW! We have delete multiple builds for %s\n",
		       cml->name);
	if (cml->removeip == TRUE) {
		printf("You have asked to delete the build IP for %s\n", 
		       cml->name);
		printf("If this server is still online, this IP will be reused\n");
		printf("Duplicate IP addresses are a bad thing!\n");
		printf("Remember to delete from DNS too.\n");
#ifdef HAVE_DNSA
//		remove_ip_from_dns(cbt, cml, data);
#endif // HAVE_DNSA
		if ((retval = cbc_run_delete(cbt, data, BUILD_IP_ON_SER_ID)) == 1)
			printf("Delete 1 IP as requested\n");
		else if (retval == 0)
			printf("Don't worry, no build IP's deleted\n");
		else
			printf("Wow! Multiple build IP's deleted\n");
	}
	if ((retval = cbc_run_delete(cbt, data, DISK_DEV_ON_SERVER_ID)) == 0)
		printf("No disk dev to delete??\n");
	retval = NONE;
	CLEAN_REMOVE_BUILD_CONFIG(retval);
#undef CLEAN_REMOVE_BUILD_CONFIG
}

