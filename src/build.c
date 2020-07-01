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
#include "cbc_data.h"
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
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

//const unsigned int spvar_len[] = { 7, 7, 5, 9, 3 };

const int sp_query[] = { 33, 63, 64, 20, 33 };

const int spvar_no = 5;

const unsigned int *cbc_search[] = { cbc_search_args, cbc_search_fields };

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
check_for_gb_keyboard(ailsa_cmdb_s *cbc, unsigned long int server_id, char *key);

static void
fill_tftp_output(cbc_comm_line_s *cml, dbdata_s *data, char *output);

static void
fill_net_output(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build);

static void
fill_mirror_output(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build);

static int
fill_kernel(cbc_comm_line_s *cml, string_len_s *build);

static void
fill_packages(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build, int i);

static int
fill_kick_base(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, string_len_s *build);

static int
fill_kick_partitions(ailsa_cmdb_s *cbc, cbc_comm_line_s *cmc, string_len_s *build);

static int
fill_kick_part_header(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, string_len_s *build);

static void
fill_kick_network_info(dbdata_s *data, string_len_s *build);

static void
fill_kick_packages(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build);

static void
fill_build_scripts(ailsa_cmdb_s *cbc, dbdata_s *data, int no, string_len_s *build, cbc_comm_line_s *cml);

static int
fill_partition(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml, string_len_s *build);

static int
fill_system_packages(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml, string_len_s *build);

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

int
display_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml)
{
	if (!(cbt) || !(cml))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *build = ailsa_db_data_list_init();

	if ((retval = cmdb_add_server_id_to_list(cml->name, cbt, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server id to list");
		goto cleanup;
	}
	if (server->total == 0) {
		ailsa_syslog(LOG_INFO, "Cannot find server %s", cml->name);
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
		printf("Failed to write dhcpd.hosts file\n");
		return retval;
	} else {
		printf("dhcpd.hosts file written\n");
	}
	if ((retval = write_tftp_config(cmc, cml)) != 0) {
		printf("Failed to write tftp configuration\n");
		return retval;
	} else {
		printf("tftp configuration file written\n");
	}
/*	if ((strncmp(cml->os, "debian", COMM_S) == 0) ||
	    (strncmp(cml->os, "ubuntu", COMM_S) == 0)) {
		if ((retval = write_preseed_build_file(cmc, cml)) != 0) {
			printf("Failed to write build file\n");
			return retval;
		} else {
			printf("build file written\n");
		}
		if ((retval = write_pre_host_script(cmc, cml)) != 0) {
			fprintf(stderr, "Failed to write host script\n");
			return retval;
		} else {
			printf("host script written\n");
		}
	} else if ((strncmp(cml->os, "centos", COMM_S) == 0) ||
	           (strncmp(cml->os, "fedora", COMM_S) == 0)) {
		if ((retval = write_kickstart_build_file(cmc, cml)) != 0) {
			printf("Failed to write build file\n");
			return retval;
		} else {
			printf("build file written\n");
		}
	} else {
		printf("OS %s does not exist\n", cml->os);
		return OS_NOT_FOUND;
	} */
	return retval;
}

int
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

int
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
	ailsa_tftp_s *l;

	if ((retval = cmdb_add_server_id_to_list(cml->name, cmc, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add server id to list");
		goto cleanup;
	}
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
	cleanup:
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
	if (((os->total % 3) != 0) || ((loc->total % 3) != 0) || ((tftp->total %4) != 0)) {
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
	ret->arch = strndup(d->data->text, SERVICE_LEN);
	d = loc->head->next->next->data;
	ret->keymap = strndup(d->data->text, SERVICE_LEN);
	d = tftp->head->data;
	ret->boot_line = strndup(d->data->text, CONFIG_LEN);
	d = tftp->head->next->data;
	ret->arg = strndup(d->data->text, SERVICE_LEN);
	d = tftp->head->next->next->data;
	ret->url = strndup(d->data->text, CONFIG_LEN);
	d = tftp->head->next->next->next->data;
	ret->net_int = strndup(d->data->text, SERVICE_LEN);
	return ret;

	cleanup:
		ailsa_clean_tftp(ret);
		return NULL;
}

int
write_preseed_build_file(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	char file[NAME_S];
	int retval = NONE, type = NET_BUILD_DETAILS;
	dbdata_s *data;
	string_len_s *build;

	snprintf(file, NAME_S, "%sweb/%s.cfg", cmc->toplevelos,  cml->name);
	if (!(build = malloc(sizeof(string_len_s))))
		report_error(MALLOC_FAIL, "build in write_preseed_build_file");
	init_string_len(build);
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return NO_NET_BUILD_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_NET_BUILD_ERR;
	} else {
		fill_net_output(cml, data, build);
		retval = 0;
	}
	clean_dbdata_struct(data);

	type = BUILD_MIRROR;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return NO_BUILD_MIRR_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_BUILD_MIRR_ERR;
	} else {
		fill_mirror_output(cml, data, build);
		retval = 0;
	}

	if ((retval = fill_partition(cmc, cml, build)) != 0)
		return retval;
	if ((retval = fill_kernel(cml, build)) != 0)
		return retval;
	clean_dbdata_struct(data);

	type = BUILD_PACKAGES;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return NO_BUILD_PACKAGES;
	} else {
		fill_packages(cml, data, build, retval);
		retval = 0;
	}
	clean_dbdata_struct(data);

	if ((retval = fill_system_packages(cmc, cml, build)) != 0)
		return retval;
	retval = write_file(file, build->string);
	clean_string_len(build);
	return retval;
}

int
write_kickstart_build_file(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	char file[NAME_S], url[CONF_S], *server = cml->name;
	int retval = NONE, type = KICK_BASE;
	unsigned long int bd_id;
	dbdata_s *data;
	string_len_s build = { .len = FILE_S, .size = NONE };

	snprintf(file, NAME_S, "%sweb/%s.cfg", cmc->toplevelos, server);
	if (!(build.string = calloc(build.len, sizeof(char))))
		report_error(MALLOC_FAIL, "build.string in write_kickstart_build_file");
	if ((retval = fill_kick_base(cmc, cml, &build)) != 0)
		return retval;
	if ((retval = fill_kick_partitions(cmc, cml, &build)) != 0)
		return retval;

	type = KICK_NET_DETAILS;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		fprintf(stderr, "Build for %s has no network information.\n",
		 server);
		return NO_NET_BUILD_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		fprintf(stderr, "Build for %s has multiple network configs.\n",
		 server);
		return MULTI_NET_BUILD_ERR;
	} else {
		fill_kick_network_info(data, &build);
	}
	clean_dbdata_struct(data);

	type = BUILD_PACKAGES;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		data = NULL;
		fprintf(stderr, "Build for %s has no packages associated.\n",
		 server);
	}
	fill_kick_packages(cml, data, &build);
	clean_dbdata_struct(data);

	type = BUILD_TYPE_URL;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		fprintf(stderr, "Build type for %s has no url??\n", server);
		return NO_BUILD_URL;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple url's?? Perhaps multiple build domains\n");
	}
	add_kick_base_script(data, &build);
	snprintf(url, CONF_S, "%s", data->fields.text);
	clean_dbdata_struct(data);

	type = BD_ID_ON_SERVER_ID;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		fprintf(stderr, "Build domain for server %s not found\n", server);
		return BUILD_DOMAIN_NOT_FOUND;
	} else if (retval > 1)
		fprintf(stderr, "Multiple build domains found for server %s\n", server);
	bd_id = data->fields.number;
	clean_dbdata_struct(data);

	type = SCRIPT_CONFIG;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = bd_id;
	snprintf(data->next->args.text, MAC_S, "%s", cml->os);
	if ((retval = cbc_run_search(cmc, data, type)) == 0)
		fprintf(stderr, "No scripts configured for this build\n");
	else
		fill_build_scripts(cmc, data, retval, &build, cml);
	add_kick_final_config(cml, &build, url);
	clean_dbdata_struct(data);
	retval = write_file(file, build.string);
	free(build.string);
	return retval;
}

#ifndef CHECK_DATA_LIST
# define CHECK_DATA_LIST(retval) {        \
	if (list->next)             \
		list = list->next;  \
	else                        \
		return retval;             \
}
#endif /* CHECK_DATA_LIST */

#ifndef PRINT_STRING_WITH_LENGTH_CHECK
# define PRINT_STRING_WITH_LENGTH_CHECK {            \
	len = strlen(line);                          \
	if ((build->size + len) >= build->len)        \
		resize_string_buff(build);           \
	pos = build->string + build->size;           \
	snprintf(pos, len + 1, "%s", line);          \
	build->size += len;                          \
	memset(line, 0, len);                        \
}
#endif /* PRINT_STRING_WITH_LENGTH_CHECK */
int
write_pre_host_script(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml)
{
	char *server, line[TBUFF_S], *pos;
	int retval = NONE, type, query = SCRIPT_CONFIG;
	size_t len = NONE;
	unsigned long int bd_id;
	dbdata_s *list = 0, *data = 0;
	string_len_s *build;

	if (!(build = malloc(sizeof(string_len_s))))
		report_error(MALLOC_FAIL, "build in write_pre_host_script");
	init_string_len(build);
	server = cml->name;
	type = BD_ID_ON_SERVER_ID;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(list);
		return NO_BD_CONFIG;
	} else if (retval > 1) {
		fprintf(stderr, "Associated with multiple build domains?\n");
		fprintf(stderr, "Using 1st one!!!\n");
	}
	retval = 0;
	bd_id = data->fields.number;
	clean_dbdata_struct(data);

	snprintf(build->string, RBUFF_S, "\
#!/bin/sh\n\
#\n\
#\n\
# Auto Generated install script for %s\n\
#\n", server);
	len = strlen(build->string);
	build->size = len;
	snprintf(line, TBUFF_S, "\
#\n\
#\n\
######################\n\
\n\
\n\
WGET=/usr/bin/wget\n\
\n\
$WGET %sscripts/disable_install.php > scripts.log 2>&1\n\
\n\
$WGET %sscripts/firstboot.sh\n\
chmod 755 firstboot.sh\n\
./firstboot.sh >> scripts.log 2>&1\n\
\n\
$WGET %sscripts/motd.sh\n\
chmod 755 motd.sh\n\
./motd.sh >> scripts.log 2>&1\n\
\n", cml->config, cml->config, cml->config);
	PRINT_STRING_WITH_LENGTH_CHECK

	cmdb_prep_db_query(&data, cbc_search, query);
	data->args.number = bd_id;
	snprintf(data->next->args.text, MAC_S, "%s", cml->os);
	if ((retval = cbc_run_search(cmc, data, query)) > 0)
		fill_build_scripts(cmc, data, retval, build, cml);
	server = cml->name;
	snprintf(line, CONF_S, "%shosts/%s.sh", cmc->toplevelos, server);
	retval = write_file(line, build->string);
	clean_dbdata_struct(data);
	free(build->string);
	free(build);
	return retval;
}

static void
fill_build_scripts(ailsa_cmdb_s *cbc, dbdata_s *list, int retval, string_len_s *build, cbc_comm_line_s *cml)
{
	if (!(list))
		return;
	char *pos, *script = list->fields.text;
	char *arg = 0, *newarg;
	char line[TBUFF_S];
	int scrno = 0;
	size_t len;
	unsigned long int argno = 0;
	dbdata_s *data = list;
	while (scrno <= retval) {
		if (data) {
			argno = data->next->next->fields.number;
			arg = data->next->fields.text;
		}
		if (!(newarg = cbc_complete_arg(cbc, cml->server_id, arg)))
			return;
		if (scrno == 0) {
			snprintf(line, TBUFF_S, "\
$WGET %sscripts/%s\n\
chmod 755 %s\n\
./%s %s ", cml->config, script, script, script, newarg);
		} else {
			if ((argno > 1) && (data)) {
				len = strlen(line);
				pos = line + len;
				snprintf(pos, TBUFF_S - len, "\
%s ", newarg);
			} else {
				len = strlen(line);
				pos = line + len;
				snprintf(pos, TBUFF_S - len, "\
>> %s.log 2>&1\n\n", script);
				PRINT_STRING_WITH_LENGTH_CHECK
				memset(line, 0, TBUFF_S);
				if (data) {
					script = data->fields.text;
					snprintf(line, TBUFF_S, "\
$WGET %sscripts/%s\n\
chmod 755 %s\n\
./%s %s ", cml->config, script, script, script, newarg);
				}
			}
		}
		scrno++;
		if (data)
			data = data->next->next->next;
		free(newarg);
		newarg = 0;
	}
}

static void
fill_tftp_output(cbc_comm_line_s *cml, dbdata_s *data, char *output)
{
	dbdata_s *list = data;
	char *bline = list->fields.text;
	CHECK_DATA_LIST()
	char *alias = list->fields.text;
	if (!(cml->os))
		cml->os = strndup(alias, CONF_S);
	else
		snprintf(cml->os, CONF_S, "%s", alias);
	CHECK_DATA_LIST()
	char *osver = list->fields.text;
	if (!(cml->os_version))
		cml->os_version = strndup(osver, MAC_S);
	else
		snprintf(cml->os_version, MAC_S, "%s", osver);
	CHECK_DATA_LIST()
	char *country = list->fields.text;
	CHECK_DATA_LIST()
	CHECK_DATA_LIST()
	CHECK_DATA_LIST()
	char *arg = list->fields.text;
	CHECK_DATA_LIST()
	char *url = list->fields.text;
	CHECK_DATA_LIST()
	char *arch = list->fields.text;
	CHECK_DATA_LIST()
	char *net_inst = list->fields.text;
	if (strncmp(alias, "debian", COMM_S) == 0) {
		if (cml->gui > 0)
			snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img %s %s=%s%s.cfg\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, bline, arg,
url, cml->name);
		else
			snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img %s %s=%s%s.cfg vga=off console=ttyS0,115200n8\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, bline, arg,
url, cml->name);
	} else if (strncmp(alias, "ubuntu", COMM_S) == 0) {
		if (cml->gui > 0)
			snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img country=%s \
console-setup/layoutcode=%s %s %s=%s%s.cfg\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, country, country,
bline, arg, url, cml->name);
		else
			snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img country=%s \
console-setup/layoutcode=%s %s %s=%s%s.cfg console=ttyS0,115200n8\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, country, country,
bline, arg, url, cml->name);
	} else if ((strncmp(alias, "centos", COMM_S) == 0) ||
		   (strncmp(alias, "fedora", COMM_S) == 0)) {
		snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img ksdevice=%s vga=off console=ttyS0,115200n8 ramdisk_size=8192\
 %s=%s%s.cfg\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, net_inst, arg, 
url, cml->name);
	}
	/* Store url for use in fill_packages */
	if (!(cml->config))
		cml->config = strndup(url, CONF_S);
	else
		snprintf(cml->config, CONF_S, "%s", url);
}

static void
fill_net_output(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build)
{
	char output[BUFF_S];
	char *ip, *ns, *nm, *gw, *tmp;
	dbdata_s *list = data;
	size_t len;

	if (!(ip = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in fill_net_output");
	if (!(ns = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ns in fill_net_output");
	if (!(nm = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "nm in fill_net_output");
	if (!(gw = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "gw in fill_net_output");
	char *locale = list->fields.text;
	CHECK_DATA_LIST()
	char *keymap = list->fields.text;
	CHECK_DATA_LIST()
	char *net_dev = list->fields.text;
	CHECK_DATA_LIST()
	uint32_t ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	CHECK_DATA_LIST()
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, ns, RANGE_S);
	CHECK_DATA_LIST()
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, nm, RANGE_S);
	CHECK_DATA_LIST()
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, gw, RANGE_S);
	CHECK_DATA_LIST()
	char *host = list->fields.text;
	CHECK_DATA_LIST()
	char *domain = list->fields.text;
	CHECK_DATA_LIST()
	char *lang = list->fields.text;

	if (strncmp(cml->os, "debian", COMM_S) == 0)
		snprintf(output, BUFF_S, "\
d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string %s\n\
d-i console-keymaps-at/keymap select %s\n\
d-i keyboard-configuration/xkb-keymap select %s\n\
d-i keymap select %s\n\
\n\
d-i preseed/early_command string /bin/killall.sh; /bin/netcfg\n\
d-i netcfg/enable boolean true\n\
d-i netcfg/confirm_static boolean true\n\
d-i netcfg/disable_dhcp boolean true\n\
d-i netcfg/choose_interface select %s\n\
d-i netcfg/get_nameservers string %s\n\
d-i netcfg/get_ipaddress string %s\n\
d-i netcfg/get_netmask string %s\n\
d-i netcfg/get_gateway string %s\n\
\n\
d-i netcfg/get_hostname string %s\n\
d-i netcfg/get_domain string %s\n",
locale, keymap, keymap, keymap, net_dev, ns, ip, nm, gw, host, domain);
	else if (strncmp(cml->os, "ubuntu", COMM_S) == 0)
/* Need to add the values into this!! */
		snprintf(output, BUFF_S, "\
d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string %s\n\
d-i debian-installer/language string %s\n\
d-i console-keymaps-at/keymap select %s\n\
d-i keymap select %s\n\
\n\
d-i netcfg/enable boolean true\n\
d-i netcfg/confirm_static boolean true\n\
d-i netcfg/disable_dhcp boolean true\n\
d-i netcfg/choose_interface select %s\n\
d-i netcfg/get_nameservers string %s\n\
d-i netcfg/get_ipaddress string %s\n\
d-i netcfg/get_netmask string %s\n\
d-i netcfg/get_gateway string %s\n\
\n\
d-i netcfg/get_hostname string %s\n\
d-i netcfg/get_domain string %s\n",
locale, lang, keymap, keymap, net_dev, ns, ip, nm, gw, host, domain);
	if ((len = strlen(output)) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "string in fill_net_output");
		else
			build->string = tmp;
	}
	snprintf(build->string, len + 1, "%s", output);
	build->size += len;
	free(ip);
	free(gw);
	free(ns);
	free(nm);
}

static void
fill_mirror_output(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build)
{
	char *mirror = data->fields.text;
	char *ver_alias = data->next->fields.text;
	char *alias = data->next->next->fields.text;
	char *country = data->next->next->next->fields.text;
	char *ntpserv = data->next->next->next->next->next->fields.text;
	char *arch = data->next->next->next->next->next->next->fields.text;
	char ntp[NAME_S], output[BUFF_S], *tmp;
	size_t len;

	if (strncmp(cml->os, "debian", COMM_S) == 0)
// Would be nice to be able to add a password for the user and root here.
		snprintf(output, BUFF_S, "\
d-i netcfg/wireless_wep string\n\
d-i hw-detect/load_firmware boolean true\n\
\n\
d-i mirror/country string manual\n\
d-i mirror/http/hostname string %s\n\
d-i mirror/http/directory string /%s\n\
\n\
d-i mirror/suite string %s\n\
\n\
### Account setup\n\
d-i passwd/root-password-crypted password $6$SF7COIid$q3o/XlLgy95kfJTuJwqshfRrVmZlhqT3sKDxUiyUd6OV2W0uwphXDJm.T1nXTJgY4.5UaFyhYjaixZvToazrZ/\n\
d-i passwd/root-login boolean false\n\
d-i passwd/user-fullname string Admin User\n\
d-i passwd/username string sysadmin\n\
d-i passwd/user-password-crypted password $6$loNBON/G$GN9geXUrajd7lPAZETkCz/c2DgkeZqNwMR9W.YpCqxAIxoNXdaHjXj1MH7DM3gMjoUvkIdgeRnkB4QDwrgqUS1\n\
d-i clock-setup/utc boolean true\n\
\n\
d-i time/zone string %s\n\
", mirror, alias, ver_alias, country);
	else if (strncmp(cml->os, "ubuntu", COMM_S) == 0)
		snprintf(output, BUFF_S, "\
d-i mirror/country string manual\n\
d-i mirror/http/hostname string %s\n\
d-i mirror/http/directory string /%s\n\
\n\
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
", mirror, alias, ver_alias, country);
	if (data->next->next->next->next->fields.small == 0)
			snprintf(ntp, NAME_S, "\
d-i clock-setup/ntp boolean false\n\
\n\
");
	else
			snprintf(ntp, NAME_S, "\
d-i clock-setup/ntp boolean true\n\
d-i clock-setup/ntp-server string %s\n\
\n\
", ntpserv);
	strncat(output, ntp, NAME_S);
	len = strlen(output);
	if ((build->size + len) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "tmp in fill_mirror_output");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", output);
	build->size += len;
	/* Store the arch for use in fill_kernel */
	if (!(cml->arch))
		cml->arch = strndup(arch, MAC_S);
	else
		snprintf(cml->arch, MAC_S, "%s", arch);
}

static int
fill_partition(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml, string_len_s *build)
{
	char *pos, line[FILE_S];
	int retval = NONE, type = BASIC_PART;
	short int lvm;
	size_t len;
	dbdata_s *data;

	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return NO_BASIC_DISK;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_BASIC_DISK;
	} else {
		add_pre_start_part(cml, data, line);
	}

	lvm = data->next->fields.small;
	len = strlen(line);
	pos = (line + len);
	snprintf(pos, URL_S, "\
d-i partman-auto/expert_recipe string \\\n\
      monkey :: \\\n");
	PRINT_STRING_WITH_LENGTH_CHECK

	if (lvm > 0)
		add_pre_volume_group(cml, build);
	retval = add_pre_parts(cmc, cml, build, lvm);
	memset(line, 0, FILE_S);
	snprintf(line, FILE_S, "\
\n\n\
d-i grub-installer/only_debian boolean true\n\
d-i grub-installer/bootdev  string %s\n", data->fields.text);
	PRINT_STRING_WITH_LENGTH_CHECK
	clean_dbdata_struct(data);
	return retval;
}

static int
fill_kernel(cbc_comm_line_s *cml, string_len_s *build)
{
//	char *arch = cml->arch, *tmp, output[BUFF_S], *os = cml->os;
	char *tmp, output[BUFF_S], *os = cml->os;
	size_t len;
	if (strncmp(os + 1, "ebian", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
\n\n\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
d-i apt-setup/services-select multiselect security\n\
d-i apt-setup/security_host string security.%s.org\n\
\n\n\
tasksel tasksel/first multiselect standard\n", os);
	} else {
		snprintf(output, BUFF_S, "\
\n\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
\n\
tasksel tasksel/first multiselect standard\n");
	}
	len = strlen(output);
	if ((build->size + len) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "next in fill_kernel");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", output);
	build->size += len;
	return NONE;
}

static void
fill_packages(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build, int i)
{
	char *next, *tmp, *pack;
	int j, k = i;
	size_t len;
	dbdata_s *list = data;

	if (!(pack = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "pack in fill_packages");
	snprintf(pack, MAC_S, "\nd-i pkgsel/include string");
	len = strlen(pack); /* should always be 26 */
	next = pack + len;
	for (j = 0; j < k; j++) {
		if (!(list))
			break;
		snprintf(next, HOST_S, " %s", list->fields.text);
		len = strlen(list->fields.text);
		next = next + len + 1;
		list = list->next;
	}
	len = strlen(pack) + 331 + strlen(cml->config);
	if (len > BUFF_S) {
		tmp = realloc(pack, BUFF_S * 2 * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "realloc in fill_packages");
		else
			pack = tmp;
	}
	snprintf(next, TBUFF_S, "\n\
d-i pkgsel/upgrade select none\n\
\n\
popularity-contest popularity-contest/participate boolean false\n\
\n\
d-i finish-install/keep-consoles boolean true\n\
\n\
d-i finish-install/reboot_in_progress note\n\
\n\
d-i cdrom-detect/eject boolean false\n\
\n\
d-i preseed/late_command string cd /target/root; wget %shosts/%s.sh \
&& sh /target/root/%s.sh\n",
		cml->config, cml->name, cml->name);
	len = strlen(pack);
	if ((len + build->size) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "next in fill_packages");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", pack);
	build->size += len;
	free(pack);
}

char *
add_pre_start_part(cbc_comm_line_s *cml, dbdata_s *data, char *disk)
{
	short int lvm = data->next->fields.small;
	size_t plen;

	if (!(cml->harddisk))
		cml->harddisk = strndup(data->fields.text, CONF_S);
	else
		snprintf(cml->harddisk, CONF_S, "%s", data->fields.text);
	if (lvm == 0)
		snprintf(disk, FILE_S, "\
d-i partman-auto/disk string %s\n\
d-i partman-auto/choose_recipe select monkey\n\
d-i partman-auto/method string regular\n\
d-i partman-auto/purge_lvm_from_device boolean true\n\
d-i partman/choose_partition select finish\n\
d-i partman/confirm_nooverwrite boolean true\n\
d-i partman/confirm boolean true\n\
d-i partman-md/device_remove_md boolean true\n\
#d-i partman-md/confirm boolean true\n\
d-i partman-partitioning/confirm_write_new_label boolean true\n\
#d-i partman/mount_style select uuid\n\
\n\
", cml->harddisk);
	else
		snprintf(disk, FILE_S, "\
d-i partman-auto/disk string %s\n\
d-i partman-auto/choose_recipe select monkey\n\
d-i partman-auto/method string lvm\n\
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
d-i partman/mount_style select uuid\n\
\n\
", cml->harddisk);
	plen = strlen(disk);
	return (disk + plen);
}

int
add_pre_parts(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, string_len_s *build, short int lvm)
{
	char *pos, line[BUFF_S], *fs, *mnt, *lv, *opt;
	int retval = 0, query = FULL_PART, optno, i;
	unsigned int dlen;
	unsigned long int pri, min, max;
	size_t len;
	dbdata_s *data, *opts, *list, *lopt;

	cmdb_prep_db_query(&data, cbc_search, query);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		clean_dbdata_struct(data);
		return NO_FULL_DISK;
	}
	dlen = cbc_search_fields[query];
	list = data;
	while (list) {
		if ((retval = check_data_length(list, dlen)) != 0) {
			clean_dbdata_struct(data);
			return CBC_DATA_WRONG_COUNT;
		}
		pri = list->fields.number;
		min = list->next->fields.number;
		max = list->next->next->fields.number;
		fs = list->next->next->next->fields.text;
		lv = list->next->next->next->next->fields.text;
		mnt = list->next->next->next->next->next->fields.text;
		optno = get_pre_part_options(cbc, cml, mnt, &opts);
		snprintf(line, RBUFF_S, "\
              %lu %lu %lu %s  \\\n", pri, min, max, fs);
		PRINT_STRING_WITH_LENGTH_CHECK
		if (lvm > 0) {
			snprintf(line, TBUFF_S, "\
                       $lvmok \\\n\
                       in_vg{ systemvg } \\\n\
                       lv_name{ %s } \\\n", lv);
			PRINT_STRING_WITH_LENGTH_CHECK
		}
		if ((strncmp(fs, "swap", COMM_S) != 0) &&
		    (strncmp(fs, "linux-swap", RANGE_S) != 0))
			snprintf(line, TBUFF_S, "\
                       method{ format } format{ } \\\n\
                       use_filesystem{ } filesystem{ %s } \\\n\
                       mountpoint{ %s } \\\n", fs, mnt);
		else
			snprintf(line, TBUFF_S, "\
                       method{ swap } format{ } \\\n");
		PRINT_STRING_WITH_LENGTH_CHECK
		list = move_down_list_data(list, dlen);
		if (optno > 0) {
			lopt = opts;
			i = optno;
			while (i > 0) {
				opt = lopt->fields.text;
				snprintf(line, RBUFF_S, "\
                       options/%s{ %s } \\\n", opt, opt);
				PRINT_STRING_WITH_LENGTH_CHECK
				lopt = lopt->next;
				i--;
			}
		}
		snprintf(line, MAC_S, "\
               . \\\n");
		PRINT_STRING_WITH_LENGTH_CHECK
		clean_dbdata_struct(opts);
	}
	build->size -= 2;
	clean_dbdata_struct(data);
	return retval;
}

int
get_pre_part_options(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, char *mnt, dbdata_s **opts)
{
	int retval, query = PART_OPT_ON_SCHEME_ID;

	cmdb_prep_db_query(opts, cbc_search, query);
	if ((retval = get_partition_id(cbc, cml->partition, mnt, &(*opts)->args.number)) != 0)
		return 0;
	if ((retval = get_scheme_id_from_build(cbc, cml->server_id, &(*opts)->next->args.number)) != 0)
		return 0;
	retval = cbc_run_search(cbc, *opts, query);
	return retval;
}

void
add_pre_volume_group(cbc_comm_line_s *cml, string_len_s *build)
{
	char line[RBUFF_S], *pos;
	size_t len;

	snprintf(line, RBUFF_S, "\
              100 1000 1000000000 ext3 \\\n\
                       $defaultignore{ } \\\n\
                       $primary{ } \\\n\
                       method{ lvm } \\\n");
	PRINT_STRING_WITH_LENGTH_CHECK
	memset(&line, 0, RBUFF_S);
	snprintf(line, RBUFF_S, "\
                       device{ %s } \\\n\
                       vg_name{ systemvg } \\\n\
              . \\\n", cml->harddisk);
	PRINT_STRING_WITH_LENGTH_CHECK
}

static int
fill_system_packages(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml, string_len_s *build)
{
	int retval, type = SYSP_INFO_ON_BD_ID;
	char *package = NULL;
	unsigned int max;
	unsigned long int bd_id;
	dbdata_s *data, *list;

	init_multi_dbdata_struct(&data, 1);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, BD_ID_ON_SERVER_ID)) == 0) {
		fprintf(stderr, "No build domain in fill_system_packages\n");
		return NO_RECORDS;
	}
	bd_id = data->fields.number;
	clean_dbdata_struct(data);
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = bd_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		printf("No system packages configured for domain\n");
		clean_dbdata_struct(data);
		return retval;
	} else {
		retval = 0;
	}
	list = data;
	while (list) {
		if (!(package) || (strncmp(package, list->fields.text, URL_S) != 0)) {
			if (build->size + 1 >= build->len)
				resize_string_buff(build);
			snprintf(build->string + build->size, 2, "\n");
			build->size++;
			package = list->fields.text;
		}
		add_system_package_line(cmc, cml->server_id, build, list);
		list = list->next->next->next->next;
	}
	clean_dbdata_struct(data);
	return retval;
}

void
add_system_package_line(ailsa_cmdb_s *cbc, uli_t server_id, string_len_s *build, dbdata_s *data)
{
	char *buff, *arg, *tmp;
	size_t blen, slen;

	if (!(buff = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buff in add_system_package_line");
	if ((snprintf(buff, BUFF_S, "%s\t%s\t%s\t", data->fields.text,
	      data->next->fields.text, data->next->next->fields.text)) >= BUFF_S)
		fprintf(stderr, "System package line truncated in preseed file!\n");
	blen = strlen(buff);
	tmp = data->next->next->next->fields.text;
	if (!(arg = cbc_complete_arg(cbc, server_id, tmp))) {
		fprintf(stderr, "Could not cbc_complete_arg for %s\n",
		 data->next->fields.text);
		free(buff);
		return;
	}
	slen = strlen(arg);
	if ((blen + slen + build->size) >= build->len)
		resize_string_buff(build);
	snprintf(build->string + build->size, blen + slen + 2, "%s%s\n", buff, arg);
	build->size += blen + slen + 1;
	free(arg);
	free(buff);
}

char *
cbc_complete_arg(ailsa_cmdb_s *cbc, uli_t server_id, char *arg)
{
	char *tmp, *new = 0, *pre, *post;
	int i, retval;
	size_t len;
	dbdata_s *data;

	if (!(arg))
		return new;
	if (!(new = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "new in cbc_complete_arg");
	snprintf(new, TBUFF_S, "%s", arg);
	for ( i = 0; i < spvar_no; i++ ) {
		if ((pre = strstr(new, spvars[i]))) {
// Check we are not the start of the new buffer
			if ((pre - new) > 0)
				*pre = '\0';
// Move forward to part we want to save
			tmp = pre + strlen(spvars[i]);
// and save it
			post = strndup(tmp, TBUFF_S);
			init_multi_dbdata_struct(&data, cbc_search_fields[sp_query[i]]);
			data->args.number = server_id;
			if ((retval = cbc_run_search(cbc, data, sp_query[i])) == 0) {
				fprintf(stderr,
"Query returned 0 entries in cbc_complete_arg for id %lu turn %d, no #%d\n", server_id, i, sp_query[i]);
				goto cleanup;
			}
			if (!(tmp = get_replaced_syspack_arg(data, i))) {
				fprintf(stderr,
"Cannot get new string in cbc_complete_arg for id %lu turn %d, no #%d\n", server_id, i, sp_query[i]);
				goto cleanup;
			}
			len = strlen(tmp) + strlen(post);
			if ((len + strlen(new)) < TBUFF_S)
				snprintf(pre, len + 1, "%s%s", tmp, post);
			clean_dbdata_struct(data);
			free(tmp);
			free(post);
		}
	}
	return new;

	cleanup:
		free(new);
		clean_dbdata_struct(data);
		return NULL;
}

char *
get_replaced_syspack_arg(dbdata_s *data, int loop)
{
// This function NEEDS validated inputs
	char *str = NULL, *tmp, addr[RANGE_S], *ip;
	uint32_t ip_addr;

	switch(loop) {
		case 0:
			str = strndup(data->fields.text, RANGE_S - 1);
			tmp = strrchr(str, '.');
			tmp++;
			*tmp = '0';
			*(tmp + 1) = '\0';
			break;
		case 1:
			str = strndup(data->fields.text, RBUFF_S - 1);
			break;
		case 2:
			if (!(str = calloc(RBUFF_S, sizeof(char))))
				report_error(MALLOC_FAIL, "str in get_replaced_syspack_arg");
			snprintf(str, RBUFF_S, "%s.%s", data->fields.text, data->next->fields.text);
			break;
		case 3:
			str = strndup(data->fields.text, HOST_S - 1);
			break;
		case 4:
// This is broken. The IP address is a number and will need to be converted to a string
			ip_addr = htonl((uint32_t)data->fields.number);
			ip = addr;
			inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
			str = strndup(ip, RANGE_S - 1);
			break;
	}
	return str;
}

static int
fill_kick_base(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, string_len_s *build)
{
	char buff[FILE_S], *key, *loc, *tim;
	int retval, type = KICK_BASE;
	size_t len;
	dbdata_s *data;

	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cbc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return NO_KICKSTART_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_KICKSTART_ERR;
	} else {
		retval = NONE;
	}

	key = data->fields.text;
	loc = data->next->fields.text;
	tim = data->next->next->fields.text;
/*
 * Redhat 6 kickstart crashes if you supply gb as keyboard type.
 * Need to check this and updated to uk if found. All other OS
 * seem to be unaffected.
 */
	check_for_gb_keyboard(cbc, cml->server_id, key);
	/* root password is k1Ckstart */
	snprintf(buff, FILE_S, "\
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
install\n\
\n", key, loc, tim);
	if ((len = strlen(buff)) > build->len)
		resize_string_buff(build);
	snprintf(build->string, len + 1, "%s", buff);
	build->size += len;
	clean_dbdata_struct(data);
	return retval;
}

static int
fill_kick_partitions(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, string_len_s *build)
{
	char line[TBUFF_S], *fs, *lv, *mount, *opts, *pos;
	int retval, query = FULL_PART;
	size_t len;
	short int lvm;
	unsigned int dlen;
	unsigned long int psize;
	dbdata_s *part, *list;

	if ((retval = fill_kick_part_header(cbc, cml, build)) != 0)
		return retval;
	lvm = cml->lvm;
	cmdb_prep_db_query(&part, cbc_search, query);
	part->args.number = cml->server_id;
	if ((retval = cbc_run_search(cbc, part, query)) == 0) {
		clean_dbdata_struct(part);
		return NO_FULL_DISK;
	}

	dlen = cbc_search_fields[query];
	list = part;
	while (list) {
		if ((retval = check_data_length(list, dlen)) != 0) {
			clean_dbdata_struct(part);
			return CBC_DATA_WRONG_COUNT;
		}
		psize = list->next->fields.number;
		fs = list->next->next->next->fields.text;
		lv = list->next->next->next->next->fields.text;
		mount = list->next->next->next->next->next->fields.text;
		opts = get_kick_part_opts(cbc, cml, mount);
		if ((lvm > 0) && (strncmp(mount, "/boot", COMM_S) != 0))
			snprintf(line, BUFF_S, "\
logvol %s --fstype \"%s\" --name=%s --vgname=%s --size=%lu\
", mount, fs, lv, cml->name, psize);
		else if (strncmp(mount, "/boot", COMM_S) != 0)
			snprintf(line, BUFF_S, "\
part %s --fstype=\"%s\" --size=%lu\
", mount, fs, psize);
		else
			fprintf(stderr, "\
Sorry, we have already added a boot partition\n");
		len = strlen(line);
		pos = line + len;
		if (opts)
			snprintf(pos, HOST_S, "\
 --fsoptions=\"%s\"", opts);
		len = strlen(line);	
		pos = line + len;
		snprintf(pos, COMM_S, "\n");
		PRINT_STRING_WITH_LENGTH_CHECK
		list = move_down_list_data(list, dlen);
		free(opts);
	}

	clean_dbdata_struct(part);
	return 0;
}

char *
get_kick_part_opts(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, char *mnt)
{	
	int retval;
	char *opts = 0, *pos;
	size_t len;
	dbdata_s *data = 0, *list;

	if ((retval = get_pre_part_options(cbc, cml, mnt, &data)) == 0) {
		clean_dbdata_struct(data);
		return opts;
	}
	opts = ailsa_calloc(HOST_S, "opts in get_kick_part_opts");
	snprintf(opts, HOST_S, "%s", data->fields.text);
	list = data->next;
	retval--;
	while (retval > 0) {
		len = strlen(opts);
		pos = opts + len;
		snprintf(pos, HOST_S - len, ",%s", list->fields.text);
		list = list->next;
		retval--;
	}
	clean_dbdata_struct(data);
	return opts;
}

static int
fill_kick_part_header(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, string_len_s *build)
{
	char *device, *pos, line[FILE_S];
	int retval, type = BASIC_PART;
	size_t len;
	short int lvm;
	dbdata_s *data;

	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cbc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return NO_BASIC_DISK;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_BASIC_DISK;
	}
	device = data->fields.text;
	cml->lvm = lvm = data->next->fields.small;
	if (lvm > 0)
		snprintf(line, FILE_S, "\
zerombr\n\
bootloader --location=mbr --driveorder=%s\n\
clearpart --all --initlabel\n\
part /boot --asprimary --fstype \"ext3\" --size=512\n\
part pv.1 --asprimary --size=1 --grow\n\
volgroup %s --pesize=32768 pv.1\n\
",  device, cml->name);
	else
		snprintf(line, FILE_S, "\
zerombr\n\
bootloader --location=mbr --driveorder=%s\n\
clearpart --all --initlabel\n\
", device);
	PRINT_STRING_WITH_LENGTH_CHECK
	clean_dbdata_struct(data);
	return 0;
}

static void
fill_kick_network_info(dbdata_s *data, string_len_s *build)
{
	char buff[FILE_S], ip[RANGE_S], nm[RANGE_S], gw[RANGE_S], ns[RANGE_S];
	char *tmp, *addr, *mirror, *alias, *arch, *ver, *dev, *host, *domain;
	size_t len = NONE;
	uint32_t ip_addr;
	dbdata_s *list = data;
	mirror = list->fields.text;
	CHECK_DATA_LIST()
	alias = list->fields.text;
	CHECK_DATA_LIST()
	arch = list->fields.text;
	CHECK_DATA_LIST()
	ver = list->fields.text;
	CHECK_DATA_LIST()
	dev = list->fields.text;
	CHECK_DATA_LIST()
	addr = ip;
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, addr, RANGE_S);
	CHECK_DATA_LIST()
	addr = nm;
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, addr, RANGE_S);
	CHECK_DATA_LIST()
	addr = gw;
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, addr, RANGE_S);
	CHECK_DATA_LIST()
	addr = ns;
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, addr, RANGE_S);
	CHECK_DATA_LIST()
	host = list->fields.text;
	CHECK_DATA_LIST()
	domain = list->fields.text;
	if (strncmp(alias, "centos", COMM_S) == 0)
		snprintf(buff, FILE_S, "\
url --url=http://%s/%s/%s/os/%s\n\
network --bootproto=static --device=%s --ip %s --netmask %s --gateway %s --nameserver %s \
--hostname %s.%s --onboot=on\n\n", mirror, alias, ver, arch, dev, ip, nm, gw, ns, host, domain);
	else if (strncmp(alias, "fedora", COMM_S) == 0)
		snprintf(buff, FILE_S, "\
url --url=http://%s/releases/%s/Fedora/%s/os/\n\
network --bootproto=static --device=%s --ip %s --netmask %s --gateway %s --nameserver %s \
--hostname %s.%s --onboot=on\n\n", mirror, ver, arch, dev, ip, nm, gw, ns, host, domain);
	len = strlen(buff);
	if ((build->size + len) >= build->len)
		resize_string_buff(build);
	tmp = build->string + build->size;
	snprintf(tmp, len + 1, "%s", buff);
	build->size +=len;
}

static void
fill_kick_packages(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build)
{
	char buff[BUFF_S], *tmp;
	size_t len = NONE;
	dbdata_s *list = data;

	snprintf(buff, MAC_S, "\
%%packages\n\
\n\
@Base\n\
");
	len = strlen(buff);
	if ((build->size + len) >= build->len)
		resize_string_buff(build);
	tmp = build->string + build->size;
	snprintf(tmp, len + 1, "%s", buff);
	build->size += len;
	while (list) {
		len = strlen(list->fields.text);
		len++;
		if ((build->size + len) >= build->len)
			resize_string_buff(build);
		tmp = build->string + build->size;
		snprintf(tmp, len + 1, "%s\n", list->fields.text);
		build->size += len;
		list = list->next;
	}
	tmp = build->string + build->size;
	snprintf(tmp, CH_S, "\n");
	build->size++;
	tmp++;
	if ((build->size + 6) >= build->len)
		resize_string_buff(build);
// This breaks a Centos 5 build
	if (strncmp(cml->os_version, "5", COMM_S) != 0) {
		snprintf(tmp, COMM_S, "%%end\n\n");
		build->size += 6;
	}
}

void
add_kick_base_script(dbdata_s *data, string_len_s *build)
{
	char buff[BUFF_S], *tmp;
	size_t len = NONE;
	if (!(data))
		return;
	char *script = data->fields.text;

	snprintf(buff, BUFF_S, "\
\n\
%%post\n\
WGET=/usr/bin/wget\n\
cd /root\n\
wget %sscripts/disable_install.php > /root/disable.log 2>&1\n\
\n\
$WGET %sscripts/firstboot.sh\n\
chmod 755 firstboot.sh\n\
./firstboot.sh >> scripts.log 2>&1\n\
\n\
wget %sscripts/motd.sh\n\
chmod 755 motd.sh\n\
./motd.sh > motd.log\n\n", script, script, script);
	len = strlen(buff);
	if ((build->size + len) >= build->len)
		resize_string_buff(build);
	tmp = build->string + build->size;
	snprintf(tmp, len + 1, "%s", buff);
	build->size += len;
}

#ifndef CHECK_KICK_CONFIG
# define CHECK_KICK_CONFIG(conf) {                \
	if (data->next) {                         \
		server = data->next->fields.text; \
	} else {                                  \
		fprintf(stderr,                   \
		 "Only one data struct in linked list in conf config\n"); \
		return;                           \
	}                                         \
	if (strncmp(url, "NULL", COMM_S) == 0) {  \
		fprintf(stderr, "url set to NULL in conf config\n");      \
		return;                           \
	}                                         \
	if (!(server)) {                          \
		fprintf(stderr, "Nothing in DB for conf server\n");       \
		return;                           \
	}                                         \
	if (strncmp(server, "NULL", COMM_S) == 0) {                      \
		fprintf(stderr, "conf server set to NULL\n");             \
		return;                           \
	}                                         \
}
#endif /* CHECK_KICK_CONFIG */

void
add_kick_final_config(cbc_comm_line_s *cml, string_len_s *build, char *url)
{
	char buff[BUFF_S], *tmp;
	size_t len = NONE;

	snprintf(buff, BUFF_S, "\
wget %sscripts/kick-final.sh\n\
chmod 755 kick-final.sh\n\
./kick-final.sh > final.log 2>&1\n\
\n", url);
	len = strlen(buff);
	if ((build->size + len) >= build->len)
		resize_string_buff(build);
	tmp = build->string + build->size;
	snprintf(tmp, len + 1, "%s", buff);
	build->size += len;
	tmp += len;
	if ((build->size + 6) >= build->len)
		resize_string_buff(build);
// This breaks a Centos 5 build
	if (strncmp(cml->os_version, "5", COMM_S) != 0) {
		snprintf(tmp, COMM_S, "%%end\n\n");
		build->size += 6;
	}
}

int
get_build_id(ailsa_cmdb_s *cmc, uli_t id, char *name, uli_t *build_id)
{
	int retval = NONE, type;
	unsigned int max;
	dbdata_s *data;

	type = BUILD_ID_ON_SERVER_ID;
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = id;
	if ((retval = cbc_run_search(cmc, data, BUILD_ID_ON_SERVER_ID)) == 1) {
		*build_id = data->fields.number;
		retval = NONE;
	} else if (retval > 1) {
		fprintf(stderr,
			"Multiple builds found for server %s\n", name);
		retval = MULTIPLE_SERVER_BUILDS;
	} else {
		fprintf(stderr,
			"No build found for server %s\n", name);
		retval = SERVER_BUILD_NOT_FOUND;
	}
	clean_dbdata_struct(data);
	return retval;
}

int
get_modify_query(unsigned long int ids[])
{
	int retval = NONE;
	unsigned long int vid = ids[0], osid = ids[1], dsid = ids[2];

	if (vid > 0) {
		if (osid > 0) {
			if (dsid > 0) {
				retval = UP_BUILD_VAR_OS_PART;
			} else {
				retval = UP_BUILD_VAR_OS;
			}
		} else {
			if (dsid > 0) {
				retval = UP_BUILD_VAR_PART;
			} else {
				retval = UP_BUILD_VARIENT;
			}
		}
	} else {
		if (osid > 0) {
			if (dsid > 0) {
				retval = UP_BUILD_OS_PART;
			} else {
				retval = UP_BUILD_OS;
			}
		} else {
			if (dsid > 0) {
				retval = UP_BUILD_PART;
			}
		}
	}
	return retval;
}

void
cbc_prep_update_dbdata(dbdata_s *data, int type, unsigned long int ids[])
{
	if (type == UP_BUILD_VAR_OS_PART) {
		data->args.number = ids[0];
		data->next->args.number = ids[1];
		data->next->next->args.number = ids[2];
		data->next->next->next->args.number = ids[3];
	} else if (type == UP_BUILD_VAR_OS) {
		data->args.number = ids[0];
		data->next->args.number = ids[1];
		data->next->next->args.number = ids[3];
	} else if (type == UP_BUILD_VAR_PART) {
		data->args.number = ids[0];
		data->next->args.number = ids[2];
		data->next->next->args.number = ids[3];
	} else if (type == UP_BUILD_OS_PART) {
		data->args.number = ids[1];
		data->next->args.number = ids[2];
		data->next->next->args.number = ids[3];
	} else if (type == UP_BUILD_VARIENT) {
		data->args.number = ids[0];
		data->next->args.number = ids[3];
	} else if (type == UP_BUILD_OS) {
		data->args.number = ids[1];
		data->next->args.number = ids[3];
	} else if (type == UP_BUILD_PART) {
		data->args.number = ids[2];
		data->next->args.number = ids[3];
	}
}

// Static functions
void
check_for_gb_keyboard(ailsa_cmdb_s *cbc, unsigned long int server_id, char *key)
{
	if (!(cbc) || !(key) || !(server_id))
		return;
	int retval = 0, type = GET_BUILD_OS_FROM_SERVER_ID;
	unsigned int max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	dbdata_s *data;

	init_multi_dbdata_struct(&data, max);
	data->args.number = server_id;
	if ((retval = cbc_run_search(cbc, data, type)) == 1) {
		if ((strncmp(data->fields.text, "Centos", RBUFF_S) == 0) ||
		 (strncmp(data->fields.text, "Redhat", RBUFF_S) == 0)) {
			if (strncmp(data->next->fields.text, "6", RBUFF_S) == 0)
				if (strncmp(key, "gb", RBUFF_S) == 0)
					snprintf(key, COMM_S, "uk");
		}
	}
}

#undef PRINT_STRING_WITH_LENGTH_CHECK

