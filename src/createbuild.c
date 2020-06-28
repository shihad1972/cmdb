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

#endif /* HAVE_DNSA */

static int
cbc_get_network_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build);

static int
cbc_get_ip_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build);

static int
cbc_get_build_dom_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *bd);

static int
cbc_calculate_build_ip(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *bdom);

#ifdef HAVE_DNSA
static int
cbc_check_ip_in_dns(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, unsigned long int ip);
#endif // HAVE_DNSA

static int
cbc_add_ip_to_build(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, unsigned long int ip);

static int
cbc_add_disk(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build);

static int
ailsa_get_modified_build(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build);

static int
ailsa_modify_build_varient(char *varient, ailsa_cmdb_s *cbt, AILLIST *build);

static int
ailsa_modify_build_partition_scheme(char *partition, ailsa_cmdb_s *cbt, AILLIST *build);

static int
ailsa_modify_build_locale(char *locale, ailsa_cmdb_s *cbt, AILLIST *build);

static int
ailsa_modify_build_netcard(char *netdev, ailsa_cmdb_s *cbt, AILLIST *build);

int
modify_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	if (!(cbt) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *build = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cml->name, args)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, BUILD_DETAILS_ON_SERVER_NAME, args, build)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DETAILS_ON_SERVER_NAME query failed");
		goto cleanup;
	}
	if ((retval = ailsa_get_modified_build(cbt, cml, build)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot modify build for %s", cml->name);
		goto cleanup;
	}
	cleanup:
		ailsa_list_full_clean(args);
		ailsa_list_full_clean(build);
		return retval;
}

int
remove_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	if (!(cbt) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *disk = ailsa_db_data_list_init();
	AILLIST *ip = ailsa_db_data_list_init();

	if ((retval = cmdb_add_server_id_to_list(cml->name, cbt, args)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_ip_id_to_list(cml->name, cbt, ip)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add IP id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_disk_id_to_list(cml->name, cbt, disk)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add disk id to list");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbt, delete_queries[DELETE_BUILD_ON_SERVER_ID], args)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_BUILD_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_delete_query(cbt, delete_queries[DELETE_DISK_DEV], disk)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_DISK_DEV query failed");
		goto cleanup;
	}
	if (cml->removeip) {
		if ((retval = ailsa_delete_query(cbt, delete_queries[DELETE_BUILD_IP], ip)) != 0) {
			ailsa_syslog(LOG_ERR, "DELETE_BUILD_IP query failed");
			goto cleanup;
		}
	}
	cleanup:
		ailsa_list_full_clean(args);
		ailsa_list_full_clean(disk);
		ailsa_list_full_clean(ip);
		return retval;
}

int
create_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	if (!(cbt) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	char **os = ailsa_calloc((sizeof(char *) * 3), "os in create_build_config");
	AILLIST *b = ailsa_db_data_list_init();
	AILLIST *l = ailsa_db_data_list_init();

	os[0] = cml->os;
	os[1] = cml->os_version;
	os[2] = cml->arch;
	if ((retval = cmdb_add_build_id_to_list(cml->name, cbt, l)) != 0)
			goto cleanup;
	if (l->total > 0) {
		ailsa_syslog(LOG_INFO, "Build already exists");
		goto cleanup;
	}
	if ((retval = cmdb_add_server_id_to_list(cml->name, cbt, b)) != 0)
		goto cleanup;
	if (b->total != 1) {
		ailsa_syslog(LOG_ERR, "Cannot get server id");
		goto cleanup;
	}
	if ((retval = cbc_get_network_info(cbt, cml, b)) != 0)
		goto cleanup;
	if (b->total != 3) {
		ailsa_syslog(LOG_ERR, "Cannot get network info");
		goto cleanup;
	}
	if ((retval = cmdb_add_varient_id_to_list(cml->varient, cbt, b)) != 0)
		goto cleanup;
	if (b->total != 4) {
		ailsa_syslog(LOG_ERR, "Cannot get varient");
		goto cleanup;
	}
	if ((retval = cmdb_add_os_id_to_list(os, cbt, b)) != 0)
		goto cleanup;
	if (b->total != 5) {
		ailsa_syslog(LOG_ERR, "Cannot get OS");
		goto cleanup;
	}
	if ((retval = cmdb_add_locale_id_to_list(cml->locale, cbt, b)) != 0)
		goto cleanup;
	if (b->total != 6) {
		ailsa_syslog(LOG_ERR, "Cannot get locale ID");
		goto cleanup;
	}
	if ((retval = cmdb_add_scheme_id_to_list(cml->partition, cbt, b)) != 0)
		goto cleanup;
	if (b->total != 7) {
		ailsa_syslog(LOG_ERR, "Cannot get partition ID");
		goto cleanup;
	}
// Now the searches will add stuff to the DB. We need to make sure we search first!
	if ((retval = cbc_add_disk(cbt, cml, b)) != 0)
		goto cleanup;
	if ((retval = cbc_get_ip_info(cbt, cml, b)) != 0)
		goto cleanup;
	if (b->total != 8) {
		ailsa_syslog(LOG_ERR, "Cannot get IP ID");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(b)) != 0)
		goto cleanup;
	if ((retval = ailsa_insert_query(cbt, INSERT_BUILD, b)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_BUILD query failed");

	cleanup:
		my_free(os);
		ailsa_list_full_clean(b);
		ailsa_list_full_clean(l);
		return retval;
}

static int
cbc_get_network_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build)
{
	if (!(cbt) || !(cml) || !(build))
		return AILSA_NO_DATA;
	int retval = 0;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d;

	if ((retval = cmdb_add_string_to_list(cml->netcard, build)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add network card to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, MAC_ADDRESS_FOR_BUILD, build, l)) != 0) {
		ailsa_syslog(LOG_ERR, "MAC_ADDRESS_FOR_BUILD query failed");
		goto cleanup;
	}
	if (l->total > 0) {
		e = l->head;
		d = e->data;
		if ((retval = cmdb_add_string_to_list(d->data->text, build)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add mac address to list");
			goto cleanup;
		}
	} else {
		retval = AILSA_NO_MAC_ADDR;
	}

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
cbc_get_ip_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build)
{
	if (!(cbt) || !(cml) || !(build))
		return AILSA_NO_DATA;
	int retval;
	size_t total = build->total;
	unsigned long int ip[4]; // ip[0] = id, [1] = start, [2] = end, [3] = ip
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *bdom = ailsa_db_data_list_init();
	void *data;

	memset(&ip, 0, sizeof(ip));
	if ((retval = cmdb_add_ip_id_to_list(cml->name, cbt, build)) != 0)
		goto cleanup;
	if (build->total == (total + 1))
		goto cleanup;
	else if (build->total > (total + 1)) {
		while (build->total != (total + 1)) {
			if (!(build->destroy))
				goto cleanup;
			if ((retval = ailsa_list_remove(build, build->tail, &data)) == 0)
				build->destroy(data);
		}
		goto cleanup;
	}
	if ((retval = cbc_get_build_dom_info(cbt, cml, bdom)) != 0)
		return retval;
	if ((retval = cbc_calculate_build_ip(cbt, cml, bdom)) != 0)
		return retval;

	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(bdom);
		return retval;
}

int
cbc_get_build_dom_info(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *bdom)
{
	if (!(cbt) || !(cml) || !(bdom))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *list = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(cml->build_domain, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, BUILD_DOMAIN_NET_INFO, list, bdom)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_NET_INFO query failed");
		goto cleanup;
	}
	if (bdom->total == 0)
		retval = BUILD_DOMAIN_NOT_FOUND;

	cleanup:
		ailsa_list_full_clean(list);
		return retval;
}

static int
cbc_calculate_build_ip(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *bdom)
{
	if (!(cbt) || !(cml) || !(bdom))
		return AILSA_NO_DATA;
	int retval;
	unsigned long int ip, bip, lip;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d = bdom->head->next->data;
	ailsa_data_s *f = bdom->head->next->next->data;

	if ((retval = cmdb_add_build_domain_id_to_list(cml->build_domain, cbt, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, BUILD_IP_ON_BUILD_DOMAIN_ID, l, r)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_IP_ON_BUILD_DOMAIN_ID query failed");
		goto cleanup;
	}
	ip = d->data->number;
	lip = f->data->number;
	if (r->total > 0) {
		e = r->head;
		while (e) {
			if (ip > lip) {
				ailsa_syslog(LOG_ERR, "No more build IP's in domain %s", cml->build_domain);
				goto cleanup;
			}
			d = e->data;
			bip = d->data->number;
			if (bip == ip) {
				ip++;
				e = r->head;
				continue;
			}
#ifdef HAVE_DNSA
			if (e == r->tail) {
				if ((retval = cbc_check_ip_in_dns(cbt, cml, ip)) > 0) {
					ip++;
					e = r->head;
					continue;
				} else if (retval < 0) {
					ailsa_syslog(LOG_ERR, "Cannot check for ip in dns");
					goto cleanup;
				}
			}
#endif // HAVE_DNSA
			e = e->next;
		}
	}
	if ((retval = cbc_add_ip_to_build(cbt, cml, ip)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add IP address to build");
		goto cleanup;
	}
	if ((retval = cmdb_add_ip_id_to_list(cml->name, cbt, bdom)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add IP id to bdom list");
		goto cleanup;
	}

	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		return retval;
}

#ifdef HAVE_DNSA
static int
cbc_check_ip_in_dns(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, unsigned long int ip)
{
	if (!(cbt) || !(cml) || (ip == 0))
		return AILSA_NO_DATA;
	char ip_addr[INET6_ADDRSTRLEN];
	int retval;
	uint32_t addr = htonl((uint32_t)ip);
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *r = ailsa_db_data_list_init();

	if (!(inet_ntop(AF_INET, &addr, ip_addr, INET6_ADDRSTRLEN))) {
		retval = -1;
		ailsa_syslog(LOG_ERR, "Cannot convert IP %u", addr);
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(ip_addr, l)) != 0) {
		retval = -1;
		ailsa_syslog(LOG_ERR, "Cannot add ip_addr to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, A_RECORDS_WITH_IP, l, r)) != 0) {
		retval = -1;
		ailsa_syslog(LOG_ERR, "A_RECORDS_WITH_IP query failed");
		goto cleanup;
	}
	retval = (int)r->total;
	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(r);
		return retval;
}
#endif // HAVE_DNSA

static int
cbc_add_ip_to_build(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, unsigned long int ip)
{
	if (!(cbt) || !(cml) || (ip == 0))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_add_number_to_list(ip, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add IP address to addition list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cml->name, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add hostname to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(cml->build_domain, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add domain name to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_build_domain_id_to_list(cml->build_domain, cbt, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build domain id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_server_id_to_list(cml->name, cbt, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server id to list");
		goto cleanup;
	}
	if ((retval = cmdb_populate_cuser_muser(l)) != 0)
		goto cleanup;
	if ((retval = ailsa_insert_query(cbt, INSERT_BUILD_IP, l)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_BUILD_IP query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static int
cbc_add_disk(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build)
{
	if (!(cbt) || !(cml) || !(build))
		return AILSA_NO_DATA;
	char disk[HOST_LEN];
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *list = ailsa_db_data_list_init();
	ailsa_data_s *d;

	if ((retval = cmdb_add_disk_dev_id_to_list(cml->name, cbt, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add disk dev id to list");
		goto cleanup;
	}
	if (list->total > 0)
		goto cleanup;
	if ((retval = cmdb_add_server_id_to_list(cml->name, cbt, args)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server id to list");
		goto cleanup;
	}
	snprintf(disk, HOST_LEN, "/dev/%s", cml->harddisk);
	if ((retval = cmdb_add_string_to_list(disk, args)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add disk device to list");
		goto cleanup;
	}
	d = build->tail->data;
	if ((retval = cmdb_add_number_to_list(d->data->number, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add def_scheme_id to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, SEED_SCHEME_LVM_ON_ID, list, args)) != 0) {
		ailsa_syslog(LOG_ERR, "SEED_SCHEME_LVM_ON_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cbt, INSERT_DISK_DEV, args)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_DISK_DEV query failed");
	else
		ailsa_syslog(LOG_INFO, "Disk %s inserted into database", cml->harddisk);

	cleanup:
		ailsa_list_full_clean(args);
		ailsa_list_full_clean(list);
		return retval;
}

static int
ailsa_get_modified_build(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, AILLIST *build)
{
	if (!(cbt) || !(cml) || !(build))
		return AILSA_NO_DATA;
	int retval = 0;

	if (cml->varient)
		if ((retval = ailsa_modify_build_varient(cml->varient, cbt, build)) != 0)
			goto cleanup;
	if (cml->partition)
		if ((retval = ailsa_modify_build_partition_scheme(cml->partition, cbt, build)) != 0)
			goto cleanup;
	if (cml->locale)
		if ((retval = ailsa_modify_build_locale(cml->locale, cbt, build)) != 0)
			goto cleanup;
	if (cml->netcard)
		if ((retval = ailsa_modify_build_netcard(cml->netcard, cbt, build)) != 0)
			goto cleanup;
	cleanup:
		return retval;
}

static int
ailsa_modify_build_varient(char *varient, ailsa_cmdb_s *cbt, AILLIST *build)
{
	if (!(varient) || !(cbt) || !(build))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = cmdb_add_varient_id_to_list(varient, cbt, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add varient id to list");
		goto cleanup;
	}
	if (l->total > 0) {
		e = l->head;
		if ((retval = cmdb_replace_data_element(build, e, 0)) != 0) {
			retval = AILSA_VARIENT_REPLACE_FAIL;
			ailsa_syslog(LOG_ERR, "Cannot replace varient id in list");
			goto cleanup;
		}
	} else {
		ailsa_syslog(LOG_INFO, "Varient %s not found in database", varient);
	}
	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static int
ailsa_modify_build_partition_scheme(char *partition, ailsa_cmdb_s *cbt, AILLIST *build)
{
	if (!(partition) || !(cbt) || !(build))
		goto cleanup;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = cmdb_add_scheme_id_to_list(partition, cbt, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add partition scheme id to list");
		goto cleanup;
	}
	if (l->total > 0) {
		e = l->head;
		if ((retval = cmdb_replace_data_element(build, e, 4)) != 0) {
			retval = AILSA_PARTITION_REPLACE_FAIL;
			ailsa_syslog(LOG_ERR, "Cannot replace partition scheme id in list");
			goto cleanup;
		}
	} else {
		ailsa_syslog(LOG_INFO, "Partition scheme %s not found in database", partition);
	}

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static int
ailsa_modify_build_locale(char *locale, ailsa_cmdb_s *cbt, AILLIST *build)
{
	if (!(locale) || !(cbt) || !(build))
		goto cleanup;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = cmdb_add_locale_id_to_list(locale, cbt, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add locale id to list");
		goto cleanup;
	}
	if (l->total > 0) {
		e = l->head;
		if ((retval =  cmdb_replace_data_element(build, e, 3)) != 0) {
			retval = AILSA_LOCALE_REPLACE_FAIL;
			ailsa_syslog(LOG_ERR, "Cannot replace locale id in list");
			goto cleanup;
		}
	} else {
		ailsa_syslog(LOG_INFO, "Locale %s not found in database", locale);
	}

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static int
ailsa_modify_build_netcard(char *netdev, ailsa_cmdb_s *cbt, AILLIST *build)
{
	if (!(netdev) || !(cbt) || !(build))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();
	AILLIST *m = ailsa_db_data_list_init();
	AILELEM *e = build->head;
	ailsa_data_s *d = e->data;

	if ((retval = cmdb_add_number_to_list(d->data->number, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server id to list");
		goto cleanup;
	}
	if ((retval = cmdb_add_string_to_list(netdev, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add network device to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbt, MAC_ADDRESS_FOR_BUILD, l, m)) != 0) {
		ailsa_syslog(LOG_ERR, "MAC_ADDRESS_FOR_BUILD query failed");
		goto cleanup;
	}
	e = l->tail;
	if (m->total > 0) {
		if ((retval = cmdb_replace_data_element(build, e, 6)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot replace netdev in list");
			goto cleanup;
		}
		e = m->head;
		if ((retval = cmdb_replace_data_element(build, e, 5)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot replace mac address in list");
			goto cleanup;
		}
	} else {
		ailsa_syslog(LOG_INFO, "Network device %s not found in database");
	}

	cleanup:
		ailsa_list_full_clean(l);
		ailsa_list_full_clean(m);
		return retval;
}
