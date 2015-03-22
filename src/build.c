/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
#include "../config.h"
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "build.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

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

int
display_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml)
{
	int retval, query;
	cbc_s *cbc, *details;
	
	if (!(cbc = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "cbc in display_build_config");
	if (!(details = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "details in display_build_config");
	init_cbc_struct(cbc);
	init_cbc_struct(details);
	query = BUILD | BUILD_DOMAIN | BUILD_IP | BUILD_TYPE | BUILD_OS | 
	  CSERVER | LOCALE | DPART | VARIENT | SSCHEME | PARTOPT;
	if ((retval = cbc_run_multiple_query(cbt, cbc, query)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return MY_QUERY_FAIL;
	}
	if ((retval = cbc_get_server(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return SERVER_NOT_FOUND;
	}
	if ((retval = cbc_get_build_details(cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return retval;
	}
	print_build_config(details);
	clean_cbc_struct(cbc);
	free(details);
	return NONE;
}

int
cbc_get_server(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	cbc_server_s *server = cbc->server;
	if (strncmp(cml->name, "NULL", COMM_S) != 0) {
		while (server) {
			if (strncmp(server->name, cml->name, HOST_S) == 0) {
				details->server = server;
				break;
			} else {
				server = server->next;
			}
		}
	} else if (strncmp(cml->uuid, "NULL", COMM_S) != 0) {
		while (server) {
			if (strncmp(server->uuid, cml->uuid, CONF_S) == 0) {
				details->server = server;
				break;
			} else {
				server = server->next;
			}
		}
	} else if (cml->server_id != 0) {
		while (server) {
			if (server->server_id == cml->server_id) {
				details->server = server;
				break;
			} else {
				server = server->next;
			}
		}
	} else {
		printf("No server specifier??\n");
		return NO_SERVERS;
	}
	if (!details->server)
		return SERVER_NOT_FOUND;
	return NONE;
}

/* Bloody horrible function. What is calling this?
   Ahh - display_build_config */
int
cbc_get_build_details(cbc_s *cbc, cbc_s *details)
{
	unsigned long int sid = details->server->server_id;
	unsigned long int osid, bid, ipid, lid, vid, bdid, btid, ssid;
	cbc_build_s *build = cbc->build;
	cbc_build_domain_s *dom = cbc->bdom;
	cbc_build_ip_s *bip = cbc->bip;
	cbc_build_os_s *bos = cbc->bos;
	cbc_build_type_s *type = cbc->btype;
	details->dpart = cbc->dpart;
	cbc_locale_s *loc = cbc->locale;
	cbc_varient_s *vari = cbc->varient;
	cbc_seed_scheme_s *sch = cbc->sscheme;
	osid = bid = ipid = lid = vid = bdid = btid = ssid = 0;

	while (build) {
		if (build->server_id == sid) {
			bid = build->build_id;
			osid = build->os_id;
			ipid = build->ip_id;
			lid = build->locale_id;
			vid = build->varient_id;
			ssid = build->def_scheme_id;
			details->build = build;
		}
		build = build->next;
	}
	if (!details->build)
		return SERVER_BUILD_NOT_FOUND;
	while (bos) {
		if (bos->os_id == osid) {
			btid = bos->bt_id;
			details->bos = bos;
		}
		bos = bos->next;
	}
	while (bip) {
		if (bip->ip_id == ipid) {
			bdid = bip->bd_id;
			details->bip = bip;
		}
		bip = bip->next;
	}
	while (dom) {
		if (dom->bd_id == bdid)
			details->bdom = dom;
		dom = dom->next;
	}
	while (type) {
		if (type->bt_id == btid)
			details->btype = type;
		type = type->next;
	}
	while (loc) {
		if (loc->locale_id == lid)
			details->locale = loc;
		loc = loc->next;
	}
	while (vari) {
		if (vari->varient_id == vid)
			details->varient = vari;
		vari = vari->next;
	}
	while (sch) {
		if (sch->def_scheme_id == ssid)
			details->sscheme = sch;
		sch = sch->next;
	}
	return NONE;
}

void
print_build_config(cbc_s *details)
{
	char *name = details->server->name, ip[RANGE_S], *addr;
	char *cuser = get_uname(details->build->cuser);
	char *muser = get_uname(details->build->muser);
	time_t crtime = (time_t)details->build->ctime;
	time_t motime = (time_t)details->build->mtime;
	unsigned long int sid = details->build->def_scheme_id;
	uint32_t ip_addr;
	cbc_pre_part_s *part = details->dpart;

	addr = ip;
	if (details->bip) {
		ip_addr = htonl((uint32_t)details->bip->ip);
		inet_ntop(AF_INET, &ip_addr, addr, RANGE_S);
	}
	printf("Build details for server %s\n\n", details->server->name);
	if (details->bdom)
		printf("Build domain:\t%s\n", details->bdom->domain);
	if (details->btype)
		printf("Build type:\t%s\n", details->btype->build_type);
	else
		printf("No build type associated with %s\n", name);
	if (details->bos)
		printf("OS:\t\t%s, version %s, arch %s\n", details->bos->os,
		 details->bos->version, details->bos->arch);
	else
		printf("No build os associated with server %s\n", name);
	if (details->varient) 
		printf("Build varient:\t%s\n", details->varient->varient);
	else
		printf("No build varient associated with %s\n", name);
	if (details->bip)
		printf("IP address:\t%s\n", addr);
	else
		printf("No build IP associated with server %s\n", name);
	printf("Build created by %s at %s", cuser, ctime(&crtime));
	printf("Build updated by %s at %s", muser, ctime(&motime));
	if (part) {
		printf("Partition information:\n");
		if (details->sscheme)
			printf("Name:\t%s\n", details->sscheme->name);
		while (part) {
			if (part->link_id.def_scheme_id == sid)
				printf("\t%s\t%s\t%s\n", part->fs, part->log_vol,
				 part->mount);
			part = part->next;
		}
	}
}

void
list_build_servers(cbc_config_s *cmc)
{
	int retval = NONE, type = SERVERS_WITH_BUILD;
	unsigned int max;
	dbdata_s *data, *list;

	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	if ((retval = cbc_run_search(cmc, data, SERVERS_WITH_BUILD)) == 0) {
		printf("No servers have build configurations\n");
	} else {
		printf("We have %d servers with build configurations\n", retval);
		list = data;
		while (list) {
			printf("%s\n", list->fields.text);
			list = list->next;
		}
	}
	clean_dbdata_struct(data);
}

int
write_build_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	int retval = NONE;

	if ((retval = get_server_id(cmc, cml->name, &cml->server_id)) != 0)
		return retval;
	if ((retval = get_scheme_name(cmc, cml->server_id, cml->partition)) != 0)
		return retval;
	if ((retval = write_dhcp_config(cmc, cml)) != 0) {
		printf("Failed to write dhcpd.hosts file\n");
		return retval;
	} else {
		printf("dhcpd.hosts file written\n");
	}
/* This will add the OS alias to cml. This will be useful in writing the
    host script */
	if ((retval = write_tftp_config(cmc, cml)) != 0) {
		printf("Failed to write tftp configuration\n");
		return retval;
	} else {
		printf("tftp configuration file written\n");
	}
	if ((strncmp(cml->os, "debian", COMM_S) == 0) ||
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
		return OS_DOES_NOT_EXIST;
	}
	return retval;
}

int
write_dhcp_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char *ip, line[RBUFF_S];
	int retval = NONE, type = DHCP_DETAILS;
	unsigned int max;
	uint32_t ip_addr;
	dbdata_s *data;
	cbc_dhcp_config_s *dhconf;
	string_len_s *dhcp;

	if (!(ip = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in write_dhcp_config");
	if (!(dhcp = malloc(sizeof(string_len_s))))
		report_error(MALLOC_FAIL, "dhcp in write_dhcp_config");
	if (!(dhconf = malloc(sizeof(cbc_dhcp_config_s))))
		report_error(MALLOC_FAIL, "dhconf in write_dhcp_config");
	init_string_len(dhcp);
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, DHCP_DETAILS)) == 0) {
		clean_dbdata_struct(data);
		return NO_DHCP_B_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_DHCP_B_ERR;
	} else {
		ip_addr = htonl((uint32_t)data->next->fields.number);
		inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
		fill_dhconf(cml->name, data, ip, dhconf);
		snprintf(line, RBUFF_S, "host %s { hardware ethernet %s; fixed-address %s; \
option domain-name \"%s\"; }\n", cml->name, data->fields.text, ip,
data->next->next->fields.text);
		retval = 0;
	}
	snprintf(dhconf->file, CONF_S, "%s/dhcpd.hosts", cmc->dhcpconf);
	fill_dhcp_hosts(line, dhcp, dhconf);
	retval = write_file(dhconf->file, dhcp->string);
	/* Could use a free_strings macro here - check out 21st century C */
	free(ip);
	free(dhcp->string);
	free(dhcp);
	free(dhconf);
	clean_dbdata_struct(data);
	return retval;
}

void
fill_dhconf(char *name, dbdata_s *data, char *ip, cbc_dhcp_config_s *dhconf)
{
	strncpy(dhconf->name, name, CONF_S);
	strncpy(dhconf->eth, data->fields.text, MAC_S);
	strncpy(dhconf->ip, ip, RANGE_S);
	strncpy(dhconf->domain, data->next->next->fields.text, RBUFF_S);
}

void
fill_dhcp_hosts(char *line, string_len_s *dhcp, cbc_dhcp_config_s *dhconf)
{
	char *buff, *cont;
	FILE *dhcp_hosts;
	size_t len = 0, blen;

	if (!(buff = calloc(RBUFF_S,  sizeof(char))))
		report_error(MALLOC_FAIL, "buff in fill_dhcp_hosts");
	if (!(dhcp_hosts = fopen(dhconf->file, "r")))
		report_error(FILE_O_FAIL, dhconf->file);
	while (fgets(buff, RBUFF_S, dhcp_hosts)) {
		blen = strlen(buff);
		if (dhcp->len < (blen + len))
			resize_string_buff(dhcp);
		cont = dhcp->string;
		if (!(strstr(buff, dhconf->name))) {
			if (!(strstr(buff, dhconf->eth))) {
				if (!(strstr(buff, dhconf->ip))) {
					strncpy(cont + len, buff, blen + 1);
					len += blen;
				}
			}
		}
	}
	fclose(dhcp_hosts);
	if (dhcp->len < (len + RBUFF_S))
		resize_string_buff(dhcp);
	cont = dhcp->string;
	strncpy(cont + len, line, RBUFF_S);
	free(buff);
}

int
write_tftp_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char out[BUFF_S], pxe[RBUFF_S];
	int retval = NONE, type = BUILD_IP_ON_SERVER_ID;
	dbdata_s *data;

	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return CANNOT_FIND_BUILD_IP;
	} else if (retval > 1) 
		fprintf(stderr, "Multiple build IP's! Using the first one!\n");
	snprintf(pxe, RBUFF_S, "%s%s%lX", cmc->tftpdir, cmc->pxe,
		 data->fields.number);
	clean_dbdata_struct(data);

	type = TFTP_DETAILS;
	cmdb_prep_db_query(&data, cbc_search, type);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return NO_TFTP_B_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_TFTP_B_ERR;
	} else {
		fill_tftp_output(cml, data, out);
		retval = write_file(pxe, out);
	}
	clean_dbdata_struct(data);
	return retval;
}

int
write_preseed_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml)
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
write_kickstart_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml)
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
	fill_kick_packages(data, &build);
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
	add_kick_final_config(&build, url);
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
write_pre_host_script(cbc_config_s *cmc, cbc_comm_line_s *cml)
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

void
fill_build_scripts(cbc_config_s *cbc, dbdata_s *list, int retval, string_len_s *build, cbc_comm_line_s *cml)
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

void
fill_tftp_output(cbc_comm_line_s *cml, dbdata_s *data, char *output)
{
	dbdata_s *list = data;
	char *bline = list->fields.text;
	CHECK_DATA_LIST()
	char *alias = list->fields.text;
	snprintf(cml->os, CONF_S, "%s", alias);
	CHECK_DATA_LIST()
	char *osver = list->fields.text;
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
		snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img %s %s=%s%s.cfg\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, bline, arg,
url, cml->name);
	} else if (strncmp(alias, "ubuntu", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img country=%s \
console-setup/layoutcode=%s %s %s=%s%s.cfg\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, country, country,
bline, arg, url, cml->name);
	} else if ((strncmp(alias, "centos", COMM_S) == 0) ||
		   (strncmp(alias, "fedora", COMM_S) == 0)) {
		snprintf(output, BUFF_S, "\
default %s\n\
\n\
label %s\n\
kernel vmlinuz-%s-%s-%s\n\
append initrd=initrd-%s-%s-%s.img ksdevice=%s console=tty0 ramdisk_size=8192\
 %s=%s%s.cfg\n\n",
cml->name, cml->name, alias, osver, arch, alias, osver, arch, net_inst, arg, 
url, cml->name);
	}
	/* Store url for use in fill_packages */
	snprintf(cml->config, CONF_S, "%s", url);
}

void
fill_net_output(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build)
{
	char output[BUFF_S];
	char *ip, *ns, *nm, *gw, *tmp;
	dbdata_s *list = data;
	size_t len;

	if (!(ip = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in fill_net_build_output");
	if (!(ns = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ns in fill_net_build_output");
	if (!(nm = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "nm in fill_net_build_output");
	if (!(gw = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "gw in fill_net_build_output");
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

void
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
	snprintf(cml->arch, MAC_S, "%s", arch);
}

int
fill_partition(cbc_config_s *cmc, cbc_comm_line_s *cml, string_len_s *build)
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
	clean_dbdata_struct(data);

	if (lvm > 0)
		add_pre_volume_group(cml, build);
	if ((retval = add_pre_parts(cmc, cml, build, lvm)) != 0)
		return retval;
	return NONE;
}

int
fill_kernel(cbc_comm_line_s *cml, string_len_s *build)
{
	char *arch = cml->arch, *tmp, output[BUFF_S], *os = cml->os;
	size_t len;
	if (strncmp(arch, "i386", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
\n\n\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
d-i apt-setup/services-select multiselect security\n\
d-i apt-setup/security_host string security.%s.org\n\
\n\n\
tasksel tasksel/first multiselect standard\n", os);
	} else if (strncmp(arch, "x86_64", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
\n\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
d-i apt-setup/services-select multiselect security\n\
d-i apt-setup/security_host string security.%s.org\n\
\n\
tasksel tasksel/first multiselect standard\n", os);
	} else {
		return NO_ARCH;
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

void
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
add_pre_parts(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build, short int lvm)
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
get_pre_part_options(cbc_config_s *cbc, cbc_comm_line_s *cml, char *mnt, dbdata_s **opts)
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

int
fill_system_packages(cbc_config_s *cmc, cbc_comm_line_s *cml, string_len_s *build)
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
add_system_package_line(cbc_config_s *cbc, uli_t server_id, string_len_s *build, dbdata_s *data)
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
cbc_complete_arg(cbc_config_s *cbc, uli_t server_id, char *arg)
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

int
fill_kick_base(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build)
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

int
fill_kick_partitions(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build)
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
get_kick_part_opts(cbc_config_s *cbc, cbc_comm_line_s *cml, char *mnt)
{	
	int retval;
	char *opts = 0, *pos;
	size_t len;
	dbdata_s *data = 0, *list;

	if ((retval = get_pre_part_options(cbc, cml, mnt, &data)) == 0) {
		clean_dbdata_struct(data);
		return opts;
	}
	opts = cmdb_malloc(HOST_S, "opts in get_kick_part_opts");
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

int
fill_kick_part_header(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build)
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

void
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

void
fill_kick_packages(dbdata_s *data, string_len_s *build)
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
	snprintf(tmp, COMM_S, "%%end\n\n");
	build->size += 6;
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
add_kick_final_config (string_len_s *build, char *url)
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
	snprintf(tmp, COMM_S, "%%end\n\n");
	build->size += 6;
}

int
get_os_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *os_id)
{
	int retval = NONE, type = OS_ID_ON_NAME;
	unsigned int max;
	dbdata_s *data;

	if (strncmp(cml->arch, "NULL", COMM_S) == 0) {
		fprintf(stderr, "No architecture provided for OS\n");
		return NO_ARCH;
	}
	if (strncmp(cml->os_version, "NULL", COMM_S) == 0) {
		fprintf(stderr, "No version or version alias provided\n");
		return NO_OS_VERSION;
	}
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	fill_dbdata_os_search(data, cml);
	if ((retval = cbc_run_search(cmc, data, OS_ID_ON_NAME)) == 1) {
		*os_id = data->fields.number;
		retval = NONE;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple OS's found!\n");
		retval = MULTIPLE_OS;
	} else {
		if ((retval = cbc_run_search(cmc, data, OS_ID_ON_ALIAS)) == 1) {
			*os_id = data->fields.number;
			retval = NONE;
		} else if (retval > 1) {
			fprintf(stderr, "Multiple OS's found!\n");
			retval = MULTIPLE_OS;
		} else {
			if ((retval = cbc_run_search
			  (cmc, data, OS_ID_ON_NAME_VER_ALIAS)) == 1) {
				*os_id = data->fields.number;
				retval = NONE;
			} else if (retval > 1) {
				fprintf(stderr, "Multiple OS's found!\n");
				retval = MULTIPLE_OS;
			} else {
				if ((retval = cbc_run_search
				 (cmc, data, OS_ID_ON_ALIAS_VER_ALIAS)) == 1) {
					 *os_id = data->fields.number;
					 retval = NONE;
				 } else if (retval > 1) {
					fprintf(stderr,
					 "Multiple OS's found!\n");
					retval = MULTIPLE_OS;
				} else {
					fprintf(stderr,
					 "No OS found!\n");
					retval = OS_NOT_FOUND;
				 }
			}
		}
	}
	clean_dbdata_struct(data);
	return retval;
}

int
get_build_id(cbc_config_s *cmc, uli_t id, char *name, uli_t *build_id)
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

void
fill_dbdata_os_search(dbdata_s *data, cbc_comm_line_s *cml)
{
	snprintf(data->args.text, CONF_S, "%s", cml->os);
	snprintf(data->next->args.text, MAC_S, "%s", cml->os_version);
	snprintf(data->next->next->args.text, MAC_S, "%s", cml->arch);
}

#undef PRINT_STRING_WITH_LENGTH_CHECK

