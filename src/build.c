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
 *  build.c
 * 
 *  Contains functions to display / add / modify / delete build details in the
 *  database for the main cbc program.
 * 
 */
#include <config.h>
#include <configmake.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "build.h"

/* Hopefully this will be the file to need these variables
   These are used to substitue these values from the database when used as
   arguments for system_package_conf */
const char *spvars[] = {
	"%baseip",
	"%domain",
	"%fqdn",
	"%hostname",
	"%ip"
};

const int sp_position[] = { 5, 11, 18, 10, 5 };

//const unsigned int spvar_len[] = { 7, 7, 5, 9, 3 };

const int sp_query[] = { 33, 63, 64, 20, 33 };

const int spvar_no = 5;

static int
write_preseed_build_file(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml);

static int
write_kickstart_build_file(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml);

static int
cbc_write_kickstart_base(ailsa_cmdb_s *cmc, char *name, int fd, AILLIST *list);

static int
cbc_write_kickstart_partitions(int fd, AILLIST *disk, AILLIST *part);

static int
cbc_write_kickstart_net(int fd, ailsa_build_s *bld);

static int
cbc_write_kickstart_packages(int fd, AILLIST *pack);

static int
cbc_write_kickstart_scripts(int fd, char *url, AILLIST *sys);

static int
cbc_check_gb_keyboard(ailsa_cmdb_s *cmc, char *host, AILLIST *list);

static int
write_build_config_file(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml);

static int
write_dhcp_config(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml);

static int
write_tftp_config(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml);

static int
write_preseed_net_mirror(int fd, ailsa_build_s *bld);

static int
write_partition_head(int fd, AILLIST *disk, ailsa_build_s *build);

static int
write_preseed_partitions(int fd, AILLIST *disk, AILLIST *part);

static int
write_preseed_system_packages(int fd, AILLIST *sys);

static void
write_preseed_lvm_group(int fd, char *dev);

static int
write_preseed_packages(int fd, ailsa_build_s *build, AILLIST *pack);

static int
write_build_host_script(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml);

static int
cbc_write_script_file(char *file, char *host, AILLIST *domain, AILLIST *sys);

static void
cbc_write_system_scripts(char *url, int fd, AILLIST *sys);

static int
cbc_print_ip_net_info(ailsa_cmdb_s *cbc, AILLIST *list);

void
display_cbc_ip_net_info(AILLIST *ip);

static int
cbc_print_os_build_type(ailsa_cmdb_s *cbc, AILLIST *list);

static void
display_build_os_info(AILLIST *os);

static int
cbc_print_build_varient(ailsa_cmdb_s *cbc, AILLIST *list);

static int
cbc_print_build_locale(ailsa_cmdb_s *cbc, AILLIST *list);

static void
display_build_locale(AILLIST *list);

static int
cbc_print_build_scheme(ailsa_cmdb_s *cbc, AILLIST *list);

static void
display_build_scheme(AILLIST *list);

static int
cbc_print_build_times_and_users(ailsa_cmdb_s *cbc, AILLIST *list);

static void
display_build_times_and_users(AILLIST *bt);

static void
print_string_8_16(char *string, size_t len);

static int
cbc_get_dhcp_info(ailsa_cmdb_s *cbc, AILLIST *dhcp);

static int
cbc_fill_dhcp_conf(AILLIST *db, AILLIST *dhcp);

static int
cbc_write_dhcp_config_file(char *filename, AILLIST *dhcp);

static ailsa_tftp_s *
cbc_fill_tftp_values(AILLIST *os, AILLIST *loc, AILLIST *tftp);

static int
cbc_write_tftp_config_file(cbc_comm_line_s *cml, char *filename, ailsa_tftp_s *tftp);

static ailsa_build_s *
cbc_fill_build_details(AILLIST *build);

static int
cbc_fill_partition_details(AILLIST *list, AILLIST *dest);

static int
cbc_fill_sys_pack_details(AILLIST *sys, AILLIST *pack, ailsa_build_s *bld);

static void
cbc_get_sys_newarg(ailsa_syspack_s *sys, ailsa_build_s *bld);

static int
cbc_fill_system_scripts(AILLIST *list, AILLIST *dest);

int
display_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	if (!(cbt) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *build = ailsa_db_data_list_init();

	if ((retval = cmdb_check_add_server_id_to_list(cml->name, cbt, server)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_build_id_to_list(cml->name, cbt, build)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add build id to list");
		goto cleanup;
	}
	if (build->total == 0) {
		ailsa_syslog(LOG_INFO, "No build for server %s", cml->name);
		goto cleanup;
	}
	printf("Build details for server %s\n\n", cml->name);
	if ((retval = cbc_print_ip_net_info(cbt, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot print IP network information");
		goto cleanup;
	}
	if ((retval = cbc_print_os_build_type(cbt, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot print os and build type");
		goto cleanup;
	}
	if ((retval = cbc_print_build_varient(cbt, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot print build varient");
		goto cleanup;
	}
	if ((retval = cbc_print_build_locale(cbt, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot print build locale");
		goto cleanup;
	}
	if ((retval = cbc_print_build_scheme(cbt, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot print build parition info");
		goto cleanup;
	}
	if ((retval = cbc_print_build_times_and_users(cbt, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot print build times and users");
		goto cleanup;
	}
	cleanup:
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(build);
		return retval;
}

static int
cbc_print_ip_net_info(ailsa_cmdb_s *cbc, AILLIST *list)
{
	if (!(cbc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *ip = ailsa_db_data_list_init();

	if ((retval = ailsa_argument_query(cbc, IP_NET_ON_SERVER_ID, list, ip)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get network info");
		goto cleanup;
	}
	if (ip->total == 0) {
		ailsa_syslog(LOG_INFO, "No IP information found");
		goto cleanup;
	}
	display_cbc_ip_net_info(ip);

	cleanup:
		ailsa_list_full_clean(ip);
		return retval;
}

void
display_cbc_ip_net_info(AILLIST *ip)
{
	if (!(ip))
		return	;
	char ip_text[SERVICE_LEN];
	AILELEM *e = ip->head;
	ailsa_data_s *d = e->data;
	uint32_t ip_addr;

	printf("Domain name:\t%s\n", d->data->text);
	e = e->next;
	d = e->data;
	ip_addr = htonl((uint32_t)d->data->number);
	inet_ntop(AF_INET, &ip_addr, ip_text, SERVICE_LEN);
	printf("IP Address:\t%s\n", ip_text);
}

static int
cbc_print_os_build_type(ailsa_cmdb_s *cbc, AILLIST *list)
{
	if (!(cbc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *os = ailsa_db_data_list_init();

	if ((retval = ailsa_argument_query(cbc, BUILD_OS_AND_TYPE, list, os)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_OS_AND_TYPE query failed");
		goto cleanup;
	}
	if (os->total == 0) {
		ailsa_syslog(LOG_INFO, "No build OS found");
		goto cleanup;
	}
	display_build_os_info(os);

	cleanup:
		ailsa_list_full_clean(os);
		return retval;
}

static void
display_build_os_info(AILLIST *os)
{
	if (!(os))
		return;
	AILELEM *e = os->head;
	ailsa_data_s *d = e->data;

	printf("Build OS:\t%s, ", d->data->text);
	e = e->next;
	d = e->data;
	printf("Version %s, ", d->data->text);
	e = e->next;
	d = e->data;
	printf("Architecture %s\n", d->data->text);
	e = e->next;
	d = e->data;
	printf("Build Type:\t%s\n", d->data->text);
}

static int
cbc_print_build_varient(ailsa_cmdb_s *cbc, AILLIST *list)
{
	if (!(cbc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *v = ailsa_db_data_list_init();
	ailsa_data_s *d;

	if ((retval = ailsa_argument_query(cbc, BUILD_VARIENT_ON_SERVER_ID, list, v)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_VARIENT_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (v->total == 0) {
		ailsa_syslog(LOG_INFO, "Cannot find build varient");
		goto cleanup;
	}
	d = v->head->data;
	printf("Build Varient\t%s\n", d->data->text);

	cleanup:
		ailsa_list_full_clean(v);
		return retval;
}

static int
cbc_print_build_locale(ailsa_cmdb_s *cbc, AILLIST *list)
{
	if (!(cbc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = ailsa_argument_query(cbc, LOCALE_DETAILS_ON_SERVER_ID, list, l)) != 0) {
		ailsa_syslog(LOG_ERR, "LOCALE_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (l->total == 0) {
		ailsa_syslog(LOG_INFO, "Cannot find build locale");
		goto cleanup;
	}
	display_build_locale(l);

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

static void
display_build_locale(AILLIST *list)
{
	if (!(list))
		return;
	AILELEM *e = list->head;
	ailsa_data_s *d = e->data;

	printf("Locale:\t\t%s\n", d->data->text);
	e = e->next;
	d = e->data;
	printf("Language:\t%s\n", d->data->text);
	e = e->next;
	d = e->data;
	printf("Timezone:\t%s\n", d->data->text);
}

static int
cbc_print_build_times_and_users(ailsa_cmdb_s *cbc, AILLIST *list)
{
	if (!(cbc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *bt = ailsa_db_data_list_init();

	if ((retval = ailsa_argument_query(cbc, BUILD_TIMES_AND_USERS, list, bt)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_TIMES_AND_USERS query failed");
		goto cleanup;
	}
	if (bt->total == 0) {
		ailsa_syslog(LOG_INFO, "Cannot get build times and users");
		goto cleanup;
	}
	display_build_times_and_users(bt);

	cleanup:
		ailsa_list_full_clean(bt);
		return retval;
}

static void
display_build_times_and_users(AILLIST *bt)
{
	if (!(bt))
		return;
	char *uname = NULL, *cname = NULL;
	AILELEM *e = bt->head;
	ailsa_data_s *d = e->data;

	uname = cmdb_get_uname(d->data->number);
	printf("Build created by %s on ", uname);
	e = e->next;
	d = e->data;
#ifdef HAVE_MYSQL
	if (d->type == AILSA_DB_TIME)
		printf("%s\n", ailsa_convert_mysql_time(d->data->time));
	else
#endif // HAVE_MYSQL
		printf("%s\n", d->data->text);
	e = e->next;
	d = e->data;
	cname = cmdb_get_uname(d->data->number);
	printf("Build modified by %s on ", cname);
	e = e->next;
	d = e->data;
#ifdef HAVE_MYSQL
	if (d->type == AILSA_DB_TIME)
		printf("%s\n", ailsa_convert_mysql_time(d->data->time));
	else
#endif // HAVE_MYSQL
		printf("%s\n", d->data->text);
	my_free(uname);
	my_free(cname);
	
}

static int
cbc_print_build_scheme(ailsa_cmdb_s *cbc, AILLIST *list)
{
	if (!(cbc) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *ps = ailsa_db_data_list_init();
	AILLIST *scheme = ailsa_db_data_list_init();
	ailsa_data_s *d;

	if ((retval = ailsa_argument_query(cbc, PART_SCHEME_NAME_ON_SERVER_ID, list, scheme)) != 0) {
		ailsa_syslog(LOG_ERR, "PART_SCHEME_NAME_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (scheme->total == 0) {
		ailsa_syslog(LOG_INFO, "Cannot find partition scheme");
		goto cleanup;
	}
	d = scheme->head->data;
	printf("\nPart scheme\t%s\n", d->data->text);
	if ((retval = ailsa_argument_query(cbc, PARTITIOINS_ON_SERVER_ID, list, ps)) != 0) {
		ailsa_syslog(LOG_ERR, "PARTITIOINS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (ps->total == 0) {
		ailsa_syslog(LOG_ERR, "Cannot find the partitions for the scheme");
		goto cleanup;
	}
	printf("Mount Point\tFilesystem\tLogical Volume\n");
	display_build_scheme(ps);

	cleanup:
		ailsa_list_full_clean(ps);
		ailsa_list_full_clean(scheme);
		return retval;
}

static void
display_build_scheme(AILLIST *list)
{
	if (!(list))
		return;
	size_t len = 3;
	size_t slen;
	AILELEM *e = list->head;
	ailsa_data_s *d = e->data;

	if ((list->total % len) != 0)
		return;
	while (e) {
		d = e->data;
		slen = strlen(d->data->text);
		print_string_8_16(d->data->text, slen);
		d = e->next->data;
		slen = strlen(d->data->text);
		print_string_8_16(d->data->text, slen);
		d = e->next->next->data;
		slen = strlen(d->data->text);
		print_string_8_16(d->data->text, slen);
		printf("\n");
		e = ailsa_move_down_list(e, len);
	}
	printf("\n");
}

static void
print_string_8_16(char *string, size_t len)
{
	if (!(string))
		return;
	if (len < 8)
		printf("%s\t\t", string);
	else
		printf("%s\t", string);
}
void
list_build_servers(ailsa_cmdb_s *cmc)
{
	if (!(cmc))
		return;
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *e;
	ailsa_data_s *d;

	if ((retval = ailsa_basic_query(cmc, ALL_SERVERS_WITH_BUILD, list)) != 0) {
		ailsa_syslog(LOG_ERR, "ALL_SERVERS_WITH_BUILD query failed");
		goto cleanup;
	}
	e = list->head;
	while (e) {
		d = e->data;
		printf("%s\n", d->data->text);
		e = e->next;
	}
	cleanup:
		ailsa_list_full_clean(list);
}

int
write_build_config(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	int retval = NONE;

	if ((retval = write_dhcp_config(cmc, cml)) != 0) {
		ailsa_syslog(LOG_ERR, "Failed to write dhcpd.hosts file");
		return retval;
	} else {
		printf("dhcpd.hosts file written\n");
	}
	if ((retval = write_tftp_config(cmc, cml)) != 0) {
		ailsa_syslog(LOG_ERR, "Failed to write tftp configuration");
		return retval;
	} else {
		printf("tftp configuration file written\n");
	}
	if ((retval = write_build_config_file(cmc, cml)) != 0) {
		ailsa_syslog(LOG_ERR, "Failed to write build file");
		return retval;
	} else {
		printf("build configuration file written\n");
	}
	if ((retval = write_build_host_script(cmc, cml)) != 0) {
		ailsa_syslog(LOG_ERR, "Failed to write host script");
		return retval;
	} else {
		printf("host script file written\n");
	}
	return retval;
}

static int
write_dhcp_config(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	if (!(cmc) || !(cml))
		return AILSA_NO_DATA;
	char file[DOMAIN_LEN];
	int retval;
	AILLIST *dhcp = ailsa_dhcp_config_list_init();

	if ((retval = cbc_get_dhcp_info(cmc, dhcp)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get dhcp info");
		goto cleanup;
	}
	snprintf(file, DOMAIN_LEN, "%sdhcpd.hosts", cmc->dhcpconf);
	if ((retval = cbc_write_dhcp_config_file(file, dhcp)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write dhcp config file");
		goto cleanup;
	}

	cleanup:
		ailsa_list_full_clean(dhcp);
		return retval;
}

static int
cbc_get_dhcp_info(ailsa_cmdb_s *cbc, AILLIST *dhcp)
{
	if (!(cbc || !(dhcp)))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *db = ailsa_db_data_list_init();
	if ((retval = ailsa_basic_query(cbc, DHCP_INFORMATION, db)) != 0) {
		ailsa_syslog(LOG_ERR, "DHCP_INFORMATION query failed in cbc_get_dhcp_info");
		goto cleanup;
	}
	if ((retval = cbc_fill_dhcp_conf(db, dhcp)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill dhcp list");
		goto cleanup;
	}

	cleanup:
		ailsa_list_full_clean(db);
		return retval;
}

static int
cbc_fill_dhcp_conf(AILLIST *db, AILLIST *dhcp)
{
	if (!(db) || !(dhcp))
		return AILSA_NO_DATA;
	int retval;
	char ip_addr[HOST_LEN];
	size_t total = 4;
	uint32_t ip;
	AILELEM *e = db->head;
	ailsa_data_s *d = e->data;
	ailsa_dhcp_conf_s *p;

	if ((db->total % total) != 0)
		return -1;
	while (e) {
		memset(ip_addr, 0, HOST_LEN);
		d = e->data;
		p = ailsa_calloc(sizeof(ailsa_dhcp_conf_s), "p in cbc_fill_dhcp_conf");
		p->name = strndup(d->data->text, DOMAIN_LEN);
		d = e->next->data;
		p->mac = strndup(d->data->text, DOMAIN_LEN);
		d = e->next->next->data;
		ip = htonl((uint32_t)d->data->number);
		if (!(inet_ntop(AF_INET, &ip, ip_addr, HOST_LEN)) != 0) {
			retval = AILSA_IP_CONVERT_FAILED;
			goto cleanup;
		}
		p->ip = strndup(ip_addr, HOST_LEN);
		d = e->next->next->next->data;
		p->domain = strndup(d->data->text, DOMAIN_LEN);
		if ((retval = ailsa_list_insert(dhcp, p)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert data into list in cbc_fill_dhcp_conf");
			goto cleanup;
		}
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		return retval;
}

static int
cbc_write_dhcp_config_file(char *filename, AILLIST *dhcp)
{
	if (!(dhcp))
		return AILSA_NO_DATA;
	int retval, fd, flags;
	mode_t um, mask;
	AILELEM *e = dhcp->head;
	ailsa_dhcp_conf_s *d;

	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	retval = 0;
	if ((fd = open(filename, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open dhcp config file %s: %s", filename, strerror(errno));
		retval = FILE_O_FAIL;
		return retval;
	}
	while (e) {
		d = e->data;
		dprintf(fd, "host %s { hardware ethernet %s; fixed-address %s; option domain-name \"%s\"; }\n",
		  d->name, d->mac, d->ip, d->domain);
		e = e->next;
	}
	close(fd);
	mask = umask(um);
	return retval;
}

static int
write_tftp_config(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	if (!(cmc) || !(cml))
		return AILSA_NO_DATA;
	char filename[DOMAIN_LEN];
	int retval;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *ip = ailsa_db_data_list_init();
	AILLIST *os = ailsa_db_data_list_init();
	AILLIST *locale = ailsa_db_data_list_init();
	AILLIST *tftp = ailsa_db_data_list_init();
	ailsa_data_s *d;
	ailsa_tftp_s *l = NULL;

	if ((retval = cmdb_check_add_server_id_to_list(cml->name, cmc, server)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(cmc, IP_NET_ON_SERVER_ID, server, ip)) != 0) {
		ailsa_syslog(LOG_ERR, "IP_NET_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (ip->total == 0) {
		ailsa_syslog(LOG_INFO, "Cannot get build IP information");
		goto cleanup;
	}
	if (ip->head->next)
		d = ip->head->next->data;
	else
		goto cleanup;
	snprintf(filename, DOMAIN_LEN, "%s%s%lX", cmc->tftpdir, cmc->pxe, d->data->number);
	if ((retval = ailsa_argument_query(cmc, BUILD_OS_DETAILS_ON_SERVER_ID, server, os)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_OS_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	d = os->head->data;
	cml->os = strndup(d->data->text, MAC_LEN);
	if ((retval = ailsa_argument_query(cmc, FULL_LOCALE_DETAILS_ON_SERVER_ID, server, locale)) != 0) {
		ailsa_syslog(LOG_ERR, "FULL_LOCALE_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, TFTP_DETAILS_ON_SERVER_ID, server, tftp)) != 0) {
		ailsa_syslog(LOG_ERR, "TFTP_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (!(l = cbc_fill_tftp_values(os, locale, tftp)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill ailsa_tftp_s struct with tftp values");
		goto cleanup;
	}
	if ((retval = cbc_write_tftp_config_file(cml, filename, l)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write tftp boot config file");
		goto cleanup;
	}
	cleanup:
		if (l)
			ailsa_clean_tftp(l);
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(ip);
		ailsa_list_full_clean(os);
		ailsa_list_full_clean(locale);
		ailsa_list_full_clean(tftp);
		return retval;
}

static ailsa_tftp_s *
cbc_fill_tftp_values(AILLIST *os, AILLIST *loc, AILLIST *tftp)
{
	if (!(os) || !(loc) || !(tftp))
		return NULL;
	ailsa_tftp_s *ret = ailsa_calloc(sizeof(ailsa_tftp_s), "ret in cbc_fill_tftp_values");
	ailsa_data_s *d = os->head->data;

	if ((os->total == 0) || (loc->total == 0) || (tftp->total == 0)) {
		ailsa_syslog(LOG_ERR, "Empty list passed into cbc_fill_tftp_values");
		goto cleanup;
	}
	if (((os->total % 3) != 0) || ((loc->total % 4) != 0) || ((tftp->total % 5) != 0)) {
		ailsa_syslog(LOG_ERR, "Wrong length for lists in cbc_fill_tftp_values");
		goto cleanup;
	}
	ret->alias = strndup(d->data->text, MAC_LEN);
	d = os->head->next->data;
	ret->version = strndup(d->data->text, MAC_LEN);
	d = os->head->next->next->data;
	ret->arch = strndup(d->data->text, SERVICE_LEN);
	d = loc->head->data;
	ret->country = strndup(d->data->text, SERVICE_LEN);
	d = loc->head->next->data;
	ret->locale = strndup(d->data->text, SERVICE_LEN);
	d = loc->head->next->next->data;
	ret->keymap = strndup(d->data->text, SERVICE_LEN);
	d = tftp->head->data;
	ret->boot_line = strndup(d->data->text, CONFIG_LEN);
	d = tftp->head->next->data;
	ret->net_int = strndup(d->data->text, SERVICE_LEN);
	d = tftp->head->next->next->data;
	ret->arg = strndup(d->data->text, SERVICE_LEN);
	d = tftp->head->next->next->next->data;
	ret->url = strndup(d->data->text, CONFIG_LEN);
	d = tftp->head->next->next->next->next->data;
	ret->build_type = strndup(d->data->text, MAC_LEN);
	return ret;

	cleanup:
		ailsa_clean_tftp(ret);
		return NULL;
}

static int
cbc_write_tftp_config_file(cbc_comm_line_s *cml, char *filename, ailsa_tftp_s *tftp)
{
	if (!(filename) || !(tftp))
		return AILSA_NO_DATA;
	char *host = cml->name;
	int retval, fd, flags;
	mode_t um, mask;

	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	retval = 0;
	if ((fd = open(filename, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open tftp config file %s: %s", filename, strerror(errno));
		retval = FILE_O_FAIL;
		return retval;
	}
	dprintf(fd, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img ", host, host, tftp->alias, tftp->version, tftp->arch, tftp->alias, tftp->version, tftp->arch);
	if (strncmp(tftp->build_type, "preseed", SERVICE_LEN) == 0) {
		dprintf(fd, "\
county=%s console-setup/layoutcode=%s %s %s=%s%s.cfg ", tftp->country, tftp->country, tftp->boot_line, tftp->arg, tftp->url, host);
		if (cml->gui > 0) {
			dprintf(fd, "\
vga=788\n\n");
		} else {
			dprintf(fd, "\
vga=off console=ttyS0,115200n8\n\n");
		}
	}
	if (strncmp(tftp->build_type, "kickstart", SERVICE_LEN) == 0) {
		dprintf(fd, "\
ksdevice=%s ramdisk_size=8192 %s=%s%s.cfg ", tftp->net_int, tftp->arg, tftp->url, host);
		if (cml->gui > 0) {
			dprintf(fd, "\
console=tty0\n\n");
		} else {
			dprintf(fd, "\
console=ttyS0,115200n8\n\n");
		}
	}
	close(fd);
	mask = umask(um);
	return retval;
}

static int
write_build_config_file(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml)
{
	if (!(cbc) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *build = ailsa_db_data_list_init();
	ailsa_data_s *d;

	if ((retval = cmdb_check_add_server_id_to_list(cml->name, cbc, server)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(cbc, BUILD_TYPE_ON_SERVER_ID, server, build)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_TYPE_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (build->total == 0) {
		ailsa_syslog(LOG_INFO, "Cannot find build type for server %s", cml->name);
		goto cleanup;
	}
	d = build->head->data;
	if (strcmp(d->data->text, "preseed") == 0) {
		if ((retval = write_preseed_build_file(cbc, cml)) != 0)
			goto cleanup;
	} else if (strcmp(d->data->text, "kickstart") == 0) {
		if ((retval = write_kickstart_build_file(cbc, cml)) != 0)
			goto cleanup;
	} else {
		ailsa_syslog(LOG_INFO, "Build type %s not supported", d->data->text);
	}
	cleanup:
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(build);
		return retval;
}
static int
write_preseed_build_file(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	char file[DOMAIN_LEN];
	int retval, flags;
	int fd = 0;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *build = ailsa_db_data_list_init();
	AILLIST *disk = ailsa_db_data_list_init();
	AILLIST *partitions = ailsa_db_data_list_init();
	AILLIST *part = ailsa_partition_list_init();
	AILLIST *packages = ailsa_db_data_list_init();
	AILLIST *sys = ailsa_db_data_list_init();
	AILLIST *syspack = ailsa_syspack_list_init();
	ailsa_build_s *bld = NULL;
	mode_t um, mask;

	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	snprintf(file, DOMAIN_LEN, "%sweb/%s.cfg", cmc->toplevelos,  cml->name);
	if ((retval = cmdb_check_add_server_id_to_list(cml->name, cmc, server)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(cmc, BUILD_DETAILS, server, build)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DETAILS query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, DISK_DEV_DETAILS_ON_SERVER_ID, server, disk)) != 0) {
		ailsa_syslog(LOG_ERR, "DISK_DEV_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, BUILD_PARTITIONS_ON_SERVER_ID, server, partitions)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_PARTITIONS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, BUILD_PACKAGES_ON_SERVER_ID, server, packages)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_PACKAGES_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, SYSTEM_PACKAGES_ON_SERVER_ID, server, sys)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_PACKAGES_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if (!(bld = cbc_fill_build_details(build))) {
		ailsa_syslog(LOG_ERR, "Cannot fill build details");
		goto cleanup;
	}
	if ((retval = cbc_fill_partition_details(partitions, part)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill partition details");
		goto cleanup;
	}
	if ((retval = cbc_fill_sys_pack_details(sys, syspack, bld)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill system package details");
		goto cleanup;
	}
	if ((fd = open(file, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open preseed build file %s for writing: %s", file, strerror(errno));
		retval = FILE_O_FAIL;
		goto cleanup;
	}
	if ((retval = write_preseed_net_mirror(fd, bld)) != 0)
		goto cleanup;
	if ((retval = write_partition_head(fd, disk, bld)) != 0)
		goto cleanup;
	if ((retval = write_preseed_partitions(fd, disk, part)) != 0)
		goto cleanup;
	if ((retval = write_preseed_packages(fd, bld, packages)) != 0)
		goto cleanup;
	if ((retval = write_preseed_system_packages(fd, syspack)) != 0)
		goto cleanup;
	cleanup:
		if (fd > 0)
			close(fd);
		mask = umask(um);
		if (bld)
			ailsa_clean_build(bld);
		ailsa_list_full_clean(build);
		ailsa_list_full_clean(disk);
		ailsa_list_full_clean(packages);
		ailsa_list_full_clean(partitions);
		ailsa_list_full_clean(part);
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(sys);
		ailsa_list_full_clean(syspack);
		return retval;
}

static int
write_preseed_net_mirror(int fd, ailsa_build_s *bld)
{
	if (!(bld))
		return AILSA_NO_DATA;
	int retval = 0;

	dprintf(fd, "\
d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string %s\n\
d-i debian-installer/language string %s\n\
d-i debian-installer/country string %s\n\
d-i console-keymaps-at/keymap select %s\n\
d-i keyboard-configuration/xkb-keymap select %s\n\
d-i keymap select %s\n\
\n", bld->locale, bld->language, bld->country, bld->keymap, bld->keymap, bld->keymap);
	if (strncmp(bld->os, "debian", SERVICE_LEN) == 0)
		dprintf(fd, "\
d-i preseed/early_command string /bin/killall.sh; /bin/netcfg\n");
	dprintf(fd, "\
d-i netcfg/enable boolean true\n\
d-i netcfg/confirm_static boolean true\n\
d-i netcfg/disable_dhcp boolean true\n\
d-i netcfg/choose_interface select %s\n\
d-i netcfg/get_nameservers string %s\n\
d-i netcfg/get_ipaddress string %s\n\
d-i netcfg/get_netmask string %s\n\
d-i netcfg/get_gateway string %s\n\
d-i netcfg/get_hostname string %s\n\
d-i netcfg/get_domain string %s\n\
\n", bld->net_int, bld->ns, bld->ip, bld->nm, bld->gw, bld->host, bld->domain);
	if (strncmp(bld->os, "debian", SERVICE_LEN) == 0)
		dprintf(fd, "\
d-i netcfg/wireless_wep string\n\
d-i hw-detect/load_firmware boolean true\n");
	dprintf(fd, "\
\n\
d-i mirror/country string manual\n\
d-i mirror/http/hostname string %s\n\
d-i mirror/http/directory string /%s\n\
d-i mirror/suite string %s\n\
\n\
### Account setup\n\
d-i passwd/root-password-crypted password $6$SF7COIid$q3o/XlLgy95kfJTuJwqshfRrVmZlhqT3sKDxUiyUd6OV2W0uwphXDJm.T1nXTJgY4.5UaFyhYjaixZvToazrZ/\n\
d-i passwd/user-fullname string Admin User\n\
d-i passwd/username string sysadmin\n\
d-i passwd/user-password-crypted password $6$loNBON/G$GN9geXUrajd7lPAZETkCz/c2DgkeZqNwMR9W.YpCqxAIxoNXdaHjXj1MH7DM3gMjoUvkIdgeRnkB4QDwrgqUS1\n\
d-i clock-setup/utc boolean true\n\
\n\
d-i time/zone string %s\n\
", bld->mirror, bld->os, bld->version, bld->country);
	if (bld->do_ntp > 0)
		dprintf(fd, "\
d-i clock-setup/ntp boolean true\n\
d-i clock-setup/ntp-server string %s\n", bld->ntp);
	else
		dprintf(fd, "\
d-i clock-setup/ntp boolean false\n");
	return retval;
}

static int
write_partition_head(int fd, AILLIST *disk, ailsa_build_s *build)
{
	if ((fd == 0) || !(disk) || !(build))
		return AILSA_NO_DATA;
	ailsa_data_s *d = disk->head->data;
	ailsa_data_s *l = disk->head->next->data;
	dprintf(fd, "\
d-i partman-auto/disk string %s\n\
d-i partman-auto/choose_recipe select monkey\n", d->data->text);
	if (l->data->number > 0)
		dprintf(fd, "\
d-i partman-auto/method string lvm\n");
	else
		dprintf(fd, "\
d-i partman-auto/method string regular\n");
	dprintf(fd, "\
d-i partman-auto/purge_lvm_from_device boolean true\n\
d-i partman-auto-lvm/guided_size string 100%%\n\
d-i partman-auto-lvm/no_boot boolean true\n\
d-i partman/choose_partition select finish\n\
d-i partman/confirm_nooverwrite boolean true\n\
d-i partman/confirm boolean true\n\
d-i partman-lvm/confirm boolean true\n\
d-i partman-lvm/confirm_nooverwrite boolean true\n\
d-i partman-lvm/device_remove_lvm boolean true\n\
d-i partman-lvm/device_remove_lvm_span boolean true\n\
d-i partman-md/device_remove_md boolean true\n\
d-i partman-md/confirm boolean true\n\
d-i partman-partitioning/confirm_write_new_label boolean true\n\
d-i partman/mount_style select uuid\n");
	return 0;
}

static int
write_preseed_partitions(int fd, AILLIST *disk, AILLIST *part)
{
	if ((fd == 0) || !(part))
		return AILSA_NO_DATA;
	short int lvm = ((ailsa_data_s *)disk->head->next->data)->data->small;
	char *diskdev = ((ailsa_data_s *)disk->head->data)->data->text;
	AILELEM *e = part->head;
	ailsa_partition_s *p;
	dprintf(fd, "\
\n\
d-i partman-auto/expert_recipe string \\\n\
      monkey :: \\\n");
	if (lvm > 0)
		write_preseed_lvm_group(fd, diskdev);
	while (e) {
		p = e->data;
		dprintf(fd, "\
              %lu %lu %lu %s  \\\n", p->pri, p->min, p->max, p->fs);
		if (lvm > 0)
			dprintf(fd, "\
                       $lvmok \\\n\
                       in_vg{ systemvg } \\\n\
                       lv_name{ %s }\\\n", p->logvol);
		if ((strncmp(p->fs, "swap", BYTE_LEN)) != 0) {
			dprintf(fd, "\
                       method{ format } format{ } \\\n\
                       use_filesystem{ } filesystem{ %s } \\\n\
                       mountpoint{ %s } \\\n", p->fs, p->mount);
		} else {
			dprintf(fd, "\
                       method{ swap } format{ } \\\n");
		}
// This would be the place to add the partition options
		e = e->next;
		if (e)
			dprintf(fd, "\
              . \\\n");
		else
			dprintf(fd, "\
              .\n\n");
	}
	dprintf(fd, "\
d-i grub-installer/only_debian boolean true\n\
d-i grub-installer/bootdev string %s\n", diskdev);
	return 0;
}

static void
write_preseed_lvm_group(int fd, char *dev)
{
	dprintf(fd, "\
              100 1000 1000000000 ext3 \\\n\
                       $defaultignore{} \\\n\
                       $primary{} \\\n\
                       method{ lvm } \\\n\
                       device{ %s } \\\n\
                       vg_name{ systemvg }\\\n\
              . \\\n", dev);
}

static int
write_preseed_packages(int fd, ailsa_build_s *bld, AILLIST *pack)
{
	if (!(bld) || !(pack) || (fd == 0))
		return AILSA_NO_DATA;
	AILELEM *e = pack->head;
	ailsa_data_s *d;

	dprintf(fd, "\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n");
	if (strncmp(bld->os, "debian", BYTE_LEN) == 0)
		dprintf(fd, "\
d-i apt-setup/services-select multiselect security\n\
d-i apt-setup/security_host string security.%s.org\n", bld->os);
	dprintf(fd, "\
tasksel tasksel/first multiselect standard\n\n\
d-i pkgsel/upgrade select none\n\
d-i pkgsel/include string");
	while (e) {
		d = e->data;
		dprintf(fd, " %s", d->data->text);
		e = e->next;
	}
	dprintf(fd, "\n\
popularity-contest popularity-contest/participate boolean false\n\
d-i finish-install/keep-consoles boolean true\n\
d-i finish-install/reboot_in_progress note\n\
d-i cdrom-detect/eject boolean false\n\
d-i preseed/late_command string cd /target/root; \
wget %shosts/%s.sh && sh /target/root/%s.sh\n\n", bld->url, bld->host, bld->host);
	return 0;
}

static int
write_preseed_system_packages(int fd, AILLIST *sys)
{
	if (!(sys) || (fd == 0))
		return AILSA_NO_DATA;
	if (sys->total == 0)
		return AILSA_NO_DATA;
	AILELEM *e = sys->head;
	ailsa_syspack_s *pack;

	while (e) {
		pack = e->data;
		dprintf(fd, "\
%s\t%s\t%s\t", pack->name, pack->field, pack->type);
		if (pack->newarg)
			dprintf(fd, "%s\n", pack->newarg);
		else
			dprintf(fd, "%s\n", pack->arg);
		e = e->next;
	}
	return 0;
}

static int
write_build_host_script(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml)
{
	if (!(cbc) || !(cml))
		return AILSA_NO_DATA;
	char file[DOMAIN_LEN];
	int retval;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *domain = ailsa_db_data_list_init();
	AILLIST *script = ailsa_db_data_list_init();
	AILLIST *sys = ailsa_sysscript_list_init();

	memset(file, 0, DOMAIN_LEN);
	snprintf(file, DOMAIN_LEN, "%shosts/%s.sh", cbc->toplevelos, cml->name);
	if ((retval = cmdb_check_add_server_id_to_list(cml->name, cbc, server)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(cbc, DOMAIN_BUILD_ALIAS_ON_SERVER_ID, server, domain)) != 0) {
		ailsa_syslog(LOG_ERR, "DOMAIN_BUILD_ALIAS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cbc, SYSTEM_SCRIPTS_ON_DOMAIN_AND_BUILD_TYPE, domain, script)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPTS_ON_DOMAIN_AND_BUILD_TYPE query failed");
		goto cleanup;
	}
	if ((retval = cbc_fill_system_scripts(script, sys)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot populate system script list");
		goto cleanup;
	}
	if ((retval = cbc_write_script_file(file, cml->name, domain, sys)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot write script file for %s", cml->name);
		goto cleanup;
	}
	cleanup:
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(domain);
		ailsa_list_full_clean(script);
		ailsa_list_full_clean(sys);
		return retval;
}


static int
cbc_write_script_file(char *file, char *host, AILLIST *domain, AILLIST *sys)
{
	if (!(file) || !(sys) || !(domain))
		return AILSA_NO_DATA;
	char *url;
	int retval, fd, flags;
	mode_t um, mask;
	ailsa_data_s *d;

	if ((domain->total == 0) || ((domain->total % 3) != 0)) {
		ailsa_syslog(LOG_ERR, "domain list empty in cbc_write_script_file");
		return AILSA_NO_DATA;
	}
	d = domain->head->next->next->data;
	url = d->data->text;
	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	retval = 0;
	if ((fd = open(file, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open file %s for writing", file);
		retval = FILE_O_FAIL;
		return retval;
	}
	dprintf(fd, "\
#!/bin/sh\n\
#\n\
#\n\
# Auto Generated install script for %s\n\
#\n\
#\n\
#\n\
###################\n\
\n\
\n\
WGET=/usr/bin/wget\n\
cd /root\n\
\n\
$WGET %sscripts/disable_install.php > scripts.log 2>&1\n\
\n\
$WGET %sscripts/firstboot.sh\n\
chmod 755 firstboot.sh\n\
./firstboot.sh >> scripts.log 2>&1\n\
\n\
$WGET %sscripts/motd.sh\n\
chmod 755 motd.sh\n\
./motd.sh >> scritps.log 2>&1\
\n", host, url, url, url);
	cbc_write_system_scripts(url, fd, sys);
	close(fd);
	mask = umask(um);
	return retval;
}

static void
cbc_write_system_scripts(char *url, int fd, AILLIST *sys)
{
	if (!(url) || (fd == 0) || !(sys))
		return;
	if (sys->total == 0)
		return;
	AILELEM *e = sys->head;
	ailsa_sysscript_s *ss = e->data;
	char *name = ss->name;

	while (e) {
		ss = e->data;
		if (ss->no == 1) {
			if (strncmp(name, ss->name, DOMAIN_LEN) != 0)
				dprintf(fd, " > %s.log 2>&1\n", name);
			name = ss->name;
			dprintf(fd, "\n\
$WGET %sscripts/%s\n\
chmod 755 %s\n\
./%s %s", url, ss->name, ss->name, ss->name, ss->arg);
		} else {
			dprintf(fd, " %s", ss->arg);
		}
		e = e->next;
	}
	dprintf(fd, " > %s.log 2>&1\n", name);
}

static ailsa_build_s *
cbc_fill_build_details(AILLIST *build)
{
	if (!(build))
		return NULL;
	char addr[SERVICE_LEN];
	uint32_t ip;
	size_t len = 18;
	ailsa_build_s *b = ailsa_calloc(sizeof(ailsa_build_s), "b in cbc_fill_build_details");
	AILELEM *e = build->head;
	ailsa_data_s *d = e->data;

	memset(addr, 0, SERVICE_LEN);
	if (build->total < len) {
		ailsa_syslog(LOG_ERR, "cbc_fill_build_details list only has %zu elements", build->total);
		goto cleanup;
	}
	b->locale = strndup(d->data->text, MAC_LEN);
	e = e->next;
	d = e->data;
	b->language = strndup(d->data->text, SERVICE_LEN);
	e = e->next;
	d = e->data;
	b->keymap = strndup(d->data->text, SERVICE_LEN);
	e = e->next;
	d = e->data;
	b->country = strndup(d->data->text, SERVICE_LEN);
	e = e->next;
	d = e->data;
	b->net_int = strndup(d->data->text, SERVICE_LEN);
	e = e->next;
	d = e->data;
	ip = htonl((uint32_t)d->data->number);
	if (!(inet_ntop(AF_INET, &ip, addr, SERVICE_LEN)))
		goto cleanup;
	b->ip = strndup(addr, SERVICE_LEN);
	memset(addr, 0, SERVICE_LEN);
	e = e->next;
	d = e->data;
	ip = htonl((uint32_t)d->data->number);
	if (!(inet_ntop(AF_INET, &ip, addr, SERVICE_LEN)))
		goto cleanup;
	b->ns = strndup(addr, SERVICE_LEN);
	memset(addr, 0, SERVICE_LEN);
	e = e->next;
	d = e->data;
	ip = htonl((uint32_t)d->data->number);
	if (!(inet_ntop(AF_INET, &ip, addr, SERVICE_LEN)))
		goto cleanup;
	b->nm = strndup(addr, SERVICE_LEN);
	memset(addr, 0, SERVICE_LEN);
	e = e->next;
	d = e->data;
	ip = htonl((uint32_t)d->data->number);
	if (!(inet_ntop(AF_INET, &ip, addr, SERVICE_LEN)))
		goto cleanup;
	b->gw = strndup(addr, SERVICE_LEN);
	e = e->next;
	d = e->data;
	b->do_ntp = d->data->small;
	e = e->next;
	d = e->data;
	b->ntp = strndup(d->data->text, DOMAIN_LEN);
	e = e->next;
	d = e->data;
	b->host = strndup(d->data->text, HOST_LEN);
	e = e->next;
	d = e->data;
	b->domain = strndup(d->data->text, DOMAIN_LEN);
	e = e->next;
	d = e->data;
	b->mirror = strndup(d->data->text, DOMAIN_LEN);
	e = e->next;
	d = e->data;
	b->os = strndup(d->data->text, MAC_LEN);
	e = e->next;
	d = e->data;
	b->version = strndup(d->data->text, SERVICE_LEN);
	e = e->next;
	d = e->data;
	b->os_ver = strndup(d->data->text, SERVICE_LEN);
	e = e->next;
	d = e->data;
	b->arch = strndup(d->data->text, SERVICE_LEN);
	e = e->next;
	d = e->data;
	b->url = strndup(d->data->text, DOMAIN_LEN);
	b->fqdn = ailsa_calloc(DOMAIN_LEN, "b->fqdn in cbc_fill_build_details");
	snprintf(b->fqdn, DOMAIN_LEN, "%s.%s", b->host, b->domain);
	return b;

	cleanup:
		ailsa_clean_build(b);
		return NULL;
}

static int
cbc_fill_partition_details(AILLIST *list, AILLIST *dest)
{
	if (!(list) || !(dest))
		return AILSA_NO_DATA;
	int retval;
	AILELEM *e = list->head;
	ailsa_data_s *d;
	ailsa_partition_s *p;
	size_t total = 6;
	if ((list->total == 0) || ((list->total % total) != 0)) {
		ailsa_syslog(LOG_ERR, "list in cbc_fill_partition_details has wrong length %zu", list->total);
		return WRONG_LENGTH_LIST;
	}
	while (e) {
		p = ailsa_calloc(sizeof(ailsa_partition_s), "p in cbc_fill_partition_details");
		d = e->data;
		p->min = d->data->number;
		d = e->next->data;
		p->max = d->data->number;
		d = e->next->next->data;
		p->pri = d->data->number;
		d = e->next->next->next->data;
		p->mount = strndup(d->data->text, DOMAIN_LEN);
		d = e->next->next->next->next->data;
		p->fs = strndup(d->data->text, SERVICE_LEN);
		d = e->next->next->next->next->next->data;
		p->logvol = strndup(d->data->text, MAC_LEN);
		e = ailsa_move_down_list(e, total);
		if ((retval = ailsa_list_insert(dest, p)) != 0)
			return retval;
	}
	return 0;
}

static int
cbc_fill_sys_pack_details(AILLIST *sys, AILLIST *pack, ailsa_build_s *bld)
{
	if (!(sys) || !(pack) || !(bld))
		return AILSA_NO_DATA;
	int retval;
	size_t total = 4;
	ailsa_syspack_s *config;
	AILELEM *e;
	ailsa_data_s *d;

	if ((sys->total == 0) || ((sys->total % total) != 0))
		return WRONG_LENGTH_LIST;
	e = sys->head;
	while (e) {
		config = ailsa_calloc(sizeof(ailsa_syspack_s), "config in cbc_fill_sys_pack_details");
		d = e->data;
		config->name = strndup(d->data->text, CONFIG_LEN);
		d = e->next->data;
		config->field = strndup(d->data->text, CONFIG_LEN);
		d = e->next->next->data;
		config->type = strndup(d->data->text, MAC_LEN);
		d = e->next->next->next->data;
		config->arg = strndup(d->data->text, CONFIG_LEN);
		e = ailsa_move_down_list(e, total);
		cbc_get_sys_newarg(config, bld);
		if ((retval = ailsa_list_insert(pack, config)) != 0)
			return retval;
	}
	return 0;
}

static void
cbc_get_sys_newarg(ailsa_syspack_s *sys, ailsa_build_s *bld)
{
	if (!(sys) || !(bld))
		return;
	char *ptr, *new, *arg, *tmp;
	char **a;
	int i, pos;
	size_t len, lena;
	arg = sys->arg;
	for (i = 0; i < spvar_no; i++) {
		if ((tmp = strstr(arg, spvars[i]))) {
			len = strlen(spvars[i]);
			lena = strlen(arg);
			pos = sp_position[i];
			a = (char **)bld;
			new = a[pos];
			sys->newarg = ailsa_calloc(DOMAIN_LEN, "sys->newarg in cbc_get_sys_newarg");
			if (len == lena) {
				snprintf(sys->newarg, DOMAIN_LEN, "%s", new);
			} else if (tmp == arg) {
				ptr = arg + len;
				snprintf(sys->newarg, DOMAIN_LEN, "%s%s", new, ptr);
			}
		}
	}
}

static int
cbc_fill_system_scripts(AILLIST *list, AILLIST *dest)
{
	if (!(list) || !(dest))
		return AILSA_NO_DATA;
	size_t total = 3;
	if ((list->total % total) != 0)
		return WRONG_LENGTH_LIST;
	int retval = 0;
	AILELEM *e = list->head;
	ailsa_sysscript_s *sys;
	ailsa_data_s *d;
	while (e) {
		sys = ailsa_calloc(sizeof(ailsa_sysscript_s), "sys in cbc_fill_system_scripts");
		d = e->data;
		sys->name = strndup(d->data->text, DOMAIN_LEN);
		d = e->next->data;
		sys->arg = strndup(d->data->text, DOMAIN_LEN);
		d = e->next->next->data;
		sys->no = d->data->number;
		if ((retval = ailsa_list_insert(dest, sys)) != 0)
			return retval;
		e = ailsa_move_down_list(e, total);
	}
	return retval;
}

static int
write_kickstart_build_file(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	if (!(cmc) || !(cml))
		return AILSA_NO_DATA;
	AILLIST *build = ailsa_db_data_list_init();
	AILLIST *domain = ailsa_db_data_list_init();
	AILLIST *disk = ailsa_db_data_list_init();
	AILLIST *locale = ailsa_db_data_list_init();
	AILLIST *partitions = ailsa_db_data_list_init();
	AILLIST *part = ailsa_partition_list_init();
	AILLIST *pack = ailsa_db_data_list_init();
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *scripts = ailsa_db_data_list_init();
	AILLIST *sys = ailsa_sysscript_list_init();
	char file[DOMAIN_LEN];
	int retval, flags;
	int fd = 0;
	mode_t um = 0;
	mode_t mask;
	ailsa_build_s *bld = NULL;

	if ((retval = cmdb_check_add_server_id_to_list(cml->name, cmc, server)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(cmc, FULL_LOCALE_DETAILS_ON_SERVER_ID, server, locale)) != 0) {
		ailsa_syslog(LOG_ERR, "FULL_LOCALE_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, DISK_DEV_DETAILS_ON_SERVER_ID, server, disk)) != 0) {
		ailsa_syslog(LOG_ERR, "DISK_DEV_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, BUILD_PARTITIONS_ON_SERVER_ID, server, partitions)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_PARTITIONS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, BUILD_PACKAGES_ON_SERVER_ID, server, pack)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_PACKAGES_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, DOMAIN_BUILD_ALIAS_ON_SERVER_ID, server, domain)) != 0) {
		ailsa_syslog(LOG_ERR, "DOMAIN_BUILD_ALIAS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, SYSTEM_SCRIPTS_ON_DOMAIN_AND_BUILD_TYPE, domain, scripts)) != 0) {
		ailsa_syslog(LOG_ERR, "SYSTEM_SCRIPTS_ON_DOMAIN_AND_BUILD_TYPE query failed");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, BUILD_DETAILS, server, build)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DETAILS query failed");
		goto cleanup;
	}
	if ((retval = cbc_fill_partition_details(partitions, part)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill in partitin details");
		goto cleanup;
	}
	if (!(bld = cbc_fill_build_details(build))) {
		ailsa_syslog(LOG_ERR, "Cannot fill build details");
		goto cleanup;
	}
	if ((retval = cbc_fill_system_scripts(scripts, sys)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill system scripts list");
		goto cleanup;
	}
	snprintf(file, DOMAIN_LEN, "%sweb/%s.cfg", cmc->toplevelos,  cml->name);
	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	if ((fd = open(file, flags, mask)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot open kickstart build file %s for writing: %s", file, strerror(errno));
		retval = FILE_O_FAIL;
		goto cleanup;
	}
	if ((retval = cbc_write_kickstart_base(cmc, cml->name, fd, locale)) != 0)
		goto cleanup;
	if ((retval = cbc_write_kickstart_partitions(fd, disk, part)) != 0)
		goto cleanup;
	if ((retval = cbc_write_kickstart_net(fd, bld)) != 0)
		goto cleanup;
	if ((retval = cbc_write_kickstart_packages(fd, pack)) != 0)
		goto cleanup;
	if ((retval = cbc_write_kickstart_scripts(fd, bld->url, sys)) != 0)
		goto cleanup;

	cleanup:
		if (fd > 0)
			close(fd);
		if (um > 0)
			mask = umask(um);
		if (bld)
			ailsa_clean_build(bld);
		ailsa_list_full_clean(build);
		ailsa_list_full_clean(domain);
		ailsa_list_full_clean(disk);
		ailsa_list_full_clean(locale);
		ailsa_list_full_clean(partitions);
		ailsa_list_full_clean(pack);
		ailsa_list_full_clean(part);
		ailsa_list_full_clean(scripts);
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(sys);
		return retval;
}

static int
cbc_write_kickstart_base(ailsa_cmdb_s *cmc, char *name, int fd, AILLIST *list)
{
	if (!(cmc) || !(name) || !(list) || (fd == 0))
		return AILSA_NO_DATA;
	int retval;
	char *key, *lang, *time;
	ailsa_data_s *d;
	size_t total = 4;

	if ((list->total == 0) || ((list->total % total) != 0))
		return WRONG_LENGTH_LIST;
	d = list->head->next->data;
	lang = d->data->text;
	d = list->head->next->next->data;
	key = d->data->text;
	d = list->tail->data;
	time = d->data->text;
	if ((retval = cbc_check_gb_keyboard(cmc, name, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot check for GB keyboard");
		return retval;
	}
	/* root password is k1Ckstart */
	dprintf(fd, "\
auth --useshadow --enablemd5\n\
bootloader --location=mbr\n\
text\n\
firewall --disabled\n\
firstboot --disable\n\
keyboard %s\n\
lang %s\n\
logging --level=info\n\
reboot\n\
rootpw --iscrypted $6$YuyiUAiz$8w/kg1ZGEnp0YqHTPuz2WpveT0OaYG6Vw89P.CYRAox7CaiaQE49xFclS07BgBHoGaDK4lcJEZIMs8ilgqV84.\n\
selinux --disabled\n\
skipx\n\
timezone  %s\n\
install\n", key, lang, time);
	return retval;
}

static int
cbc_write_kickstart_partitions(int fd, AILLIST *disk, AILLIST *part)
{
	if (!(disk) || !(part) || (fd == 0))
		return AILSA_NO_DATA;
	AILELEM *e;
	ailsa_data_s *d;
	ailsa_partition_s *p;
	if ((disk->total == 0) || (part->total == 0))
		return AILSA_NO_DATA;
	d = disk->head->data;
	dprintf(fd, "\
\n\
zerombr\n\
bootloader --location=mbr --driveorder=%s\n\
clearpart --all --initlabel\n", d->data->text);
	d = disk->head->next->data;
	e = part->head;
	if (d->data->small > 0) {
		dprintf(fd, "\
part /boot --asprimary --fstype=\"ext3\" --size=512\n\
part pv.1 --asprimary --size=1 --grow\n\
volgroup system_vg --pesize=32768 pv.1\n");
		while (e) {
			p = e->data;
			dprintf(fd, "\
logvol %s --fstype=\"%s\" --name=%s --vgname=system_vg --size=%lu\n\
", p->mount, p->fs, p->logvol, p->min);
			e = e->next;
		}
	} else {
		while (e) {
			p = e->data;
			dprintf(fd, "\
part %s --fstype=\"%s\" --size=%lu\n\
", p->mount, p->fs, p->min);
			e = e->next;
		}
	}
	dprintf(fd, "\n");
	return 0;
}

static int
cbc_write_kickstart_net(int fd, ailsa_build_s *bld)
{
	if (!(bld) || (fd == 0))
		return AILSA_NO_DATA;
	dprintf(fd, "\
\n\
url --url=http://%s/%s/%s/os/%s\n\
network --bootproto=static --device=%s --ip=%s --netmask=%s --gateway=%s --nameserver=%s --hostname=%s.%s --onboot=on\n\
", bld->mirror, bld->os, bld->os_ver, bld->arch, bld->net_int, bld->ip, bld->nm, bld->gw, bld->ns, bld->host, bld->domain);
	return 0;
}

static int
cbc_write_kickstart_packages(int fd, AILLIST *pack)
{
	if (!(pack) || (fd == 0))
		return AILSA_NO_DATA;
	AILELEM *e;
	ailsa_data_s *d;

	dprintf(fd, "\
\n\
%%packages\n\
\n\
@Base\n\
");
	e = pack->head;
	while (e) {
		d = e->data;
		dprintf(fd, "%s\n", d->data->text);
		e = e->next;
	}
	dprintf(fd, "\n%%end\n\n");
	return 0;
}

static int
cbc_write_kickstart_scripts(int fd, char *url, AILLIST *sys)
{
	if (!(url) || !(sys) || (fd == 0))
		return AILSA_NO_DATA;

	dprintf(fd, "\
%%post\n\
\n\
WGET=/usr/bin/wget\n\
cd /root\n\
$WGET %sscripts/disable_install.php > disable.log 2>&1\n\
\n\
$WGET %sscripts/firstboot.sh\n\
chmod 755 firstboot.sh\n\
./firstboot.sh > firstboot.log 2>&1\n\
\n\
wget %sscripts/motd.sh\n\
chmod 755 motd.sh\n\
./motd.sh > motd.log 2>&1\n", url, url, url);
	cbc_write_system_scripts(url, fd, sys);
	return 0;
}

static int
cbc_check_gb_keyboard(ailsa_cmdb_s *cmc, char *host, AILLIST *list)
{
	if (!(cmc) || !(host) || !(list))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *os = ailsa_db_data_list_init();
	ailsa_data_s *d;


	if ((retval = cmdb_check_add_server_id_to_list(host, cmc, server)) != 0)
		goto cleanup;
	if ((retval = ailsa_argument_query(cmc, BUILD_OS_DETAILS_ON_SERVER_ID, server, os)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_OS_DETAILS_ON_SERVER_ID query failed");
		goto cleanup;
	}
	if ((os->total == 0) || (os->total < 2)) {
		ailsa_syslog(LOG_ERR, "OS list wrong length in cbc_check_gb_keyboard");
		goto cleanup;
	}
	d = os->head->next->data;
	if (strncmp(d->data->text, "6", BYTE_LEN) == 0) {
		d = list->head->next->next->data;
		if (strncmp(d->data->text, "gb", BYTE_LEN) == 0) {
			snprintf(d->data->text, BYTE_LEN, "uk");
		}
	}
	cleanup:
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(os);
		return retval;
}

int
view_defaults_for_cbc(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	if (!(cbt) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *dom = ailsa_db_data_list_init();
	AILLIST *os = ailsa_db_data_list_init();
	AILLIST *part = ailsa_db_data_list_init();
	AILLIST *var = ailsa_db_data_list_init();

	if ((retval = ailsa_basic_query(cbt, DEFAULT_DOMAIN_DETAILS, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_DOMAIN_DETAILS query failed");
		goto cleanup;
	}
	if ((retval = ailsa_basic_query(cbt, DEFAULT_OS_DETAILS, os)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_OS_DETAILS query failed");
		goto cleanup;
	}
	if ((retval = ailsa_basic_query(cbt, DEFAULT_SCHEME_DETAILS, part)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_SCHEME_DETAILS query failed");
		goto cleanup;
	}
	if ((retval = ailsa_basic_query(cbt, DEFAULT_VARIENT_DETAILS, var)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_VARIENT_DETAILS query failed");
		goto cleanup;
	}
	if (dom->total == 0)
		printf("No default build domain\n");
	else
		printf("Default build domain: %s\n", ((ailsa_data_s *)dom->head->data)->data->text);
	if (part->total == 0)
		printf("No default partition scheme\n");
	else
		printf("Default partition scheme: %s\n", ((ailsa_data_s *)part->head->data)->data->text);
	if (var->total == 0)
		printf("No default varient\n");
	else
		printf("Default build varient: %s\n", ((ailsa_data_s *)var->head->data)->data->text);
	if (os->total == 0) {
		printf("No default build OS\n");
	} else {
		printf("Default build os: %s; ", ((ailsa_data_s *)os->head->data)->data->text);
		printf("Version: %s; ", ((ailsa_data_s *)os->head->next->data)->data->text);
		printf("Arch: %s\n", ((ailsa_data_s *)os->head->next->next->data)->data->text);
	}
	cleanup:
		ailsa_list_full_clean(dom);
		ailsa_list_full_clean(os);
		ailsa_list_full_clean(part);
		ailsa_list_full_clean(var);
		return retval;

}
