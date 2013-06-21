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
 *  build.c
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


int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_s *cb)
{
	int retval, opt;

	retval = NONE;
	while ((opt = getopt(argc, argv, "b:e:i:k:n:o:p:t:u:v:x:adglmrw")) != -1) {
		if (opt == 'n') {
			snprintf(cb->name, CONF_S, "%s", optarg);
			cb->server = NAME;
		} else if (opt == 'u') {
			snprintf(cb->uuid, CONF_S, "%s", optarg);
			cb->server = UUID;
		} else if (opt == 'i') {
			cb->server_id = strtoul(optarg, NULL, 10);
			cb->server = ID;
		} else if (opt == 'm') {
			cb->action = MOD_CONFIG;
		} else if (opt == 'r') {
			cb->action = RM_CONFIG;
		} else if (opt == 'w') {
			cb->action = WRITE_CONFIG;
		} else if (opt == 'd') {
			cb->action = DISPLAY_CONFIG;
		} else if (opt == 'a') {
			cb->action = ADD_CONFIG;
		} else if (opt == 'l') {
			cb->action = LIST_CONFIG;
		} else if (opt == 'b') {
			snprintf(cb->build_domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'e') {
			cb->locale = strtoul(optarg, NULL, 10);
		} else if (opt == 'o') {
			snprintf(cb->os, MAC_S, "%s", optarg);
		} else if (opt == 'p') {
			snprintf(cb->partition, CONF_S, "%s", optarg);
		} else if (opt == 't') {
			snprintf(cb->arch, RANGE_S, "%s", optarg);
		} else if (opt == 'v') {
			snprintf(cb->os_version, MAC_S, "%s", optarg);
		} else if (opt == 'x') {
			snprintf(cb->varient, CONF_S, "%s", optarg);
		} else if (opt == 'k') {
			snprintf(cb->netcard, HOST_S, "%s", optarg);
		} else {
			printf("Unknown option: %c\n", opt);
			retval = DISPLAY_USAGE;
		}
	}
/* This needs updated to reflect the new command line options */
	if ((cb->action == NONE) && 
	 (cb->server == NONE) &&
	 (strncmp(cb->action_type, "NULL", MAC_S) == 0))
		retval = DISPLAY_USAGE;
	else if (cb->action == NONE)
		retval = NO_ACTION;
	else if ((cb->action == WRITE_CONFIG) &&
		 (cb->server == 0))
		return NO_NAME_OR_ID;
/*	else if ((cb->server == NONE) &&
	 (strncmp(cb->action_type, "NULL", CONF_S) == 0))
		retval = NO_NAME_OR_ID;
	else if ((cb->action == ADD_CONFIG) &&
	 (strncmp(cb->action_type, "NULL", CONF_S) == 0) &&
	 (cb->server == NONE))
		retval = NO_NAME_OR_ID;
	else if ((cb->action == CREATE_CONFIG) &&
	 (cb->server == NONE))
		retval = NO_NAME_OR_ID;
	if (cb->action == CREATE_CONFIG)
		snprintf(cb->action_type, MAC_S, "create config"); */
	return retval;
	
}

void
print_cbc_command_line_values(cbc_comm_line_s *cml)
{
	fprintf(stderr, "########\nCommand line Values\n");
	if (cml->action == WRITE_CONFIG)
		fprintf(stderr, "Action: Write configuration file\n");
	else if (cml->action == DISPLAY_CONFIG)
		fprintf(stderr, "Action: Display configuration\n");
	else if (cml->action == ADD_CONFIG)
		fprintf(stderr, "Action: Add configuration for build\n");
	else if (cml->action == CREATE_CONFIG)
		fprintf(stderr, "Action: Create build configuration\n");
	else
		fprintf(stderr, "Action: Unknown!!\n");
	fprintf(stderr, "Config: %s\n", cml->config);
	fprintf(stderr, "Name: %s\n", cml->name);
	fprintf(stderr, "UUID: %s\n", cml->uuid);
	fprintf(stderr, "Server ID: %ld\n", cml->server_id);
	fprintf(stderr, "OS ID: %lu\n", cml->os_id);
	fprintf(stderr, "OS: %s\n", cml->os);
	fprintf(stderr, "OS Version: %s\n", cml->os_version);
	fprintf(stderr, "Architecture: %s\n", cml->arch);
	fprintf(stderr, "Locale ID: %lu\n", cml->locale);
	fprintf(stderr, "Build Domain: %s\n", cml->build_domain);
	fprintf(stderr, "Action Type: %s\n", cml->action_type);
	fprintf(stderr, "Partition: %s\n", cml->partition);
	fprintf(stderr, "Varient: %s\n", cml->varient);
	
	fprintf(stderr, "\n");
}

void
init_all_config(cbc_config_s *cct, cbc_comm_line_s *cclt/*, cbc_build_s *cbt*/)
{
	init_cbc_config_values(cct);
	init_cbc_comm_values(cclt);
/*	init_cbc_build_values(cbt); */
}

void
init_cbc_comm_values(cbc_comm_line_s *cbt)
{
	cbt->action = NONE;
	cbt->server_id = NONE;
	cbt->server = NONE;
	cbt->locale = 0;
	cbt->os_id = 0;
	cbt->removeip = 0;
	snprintf(cbt->name, CONF_S, "NULL");
	snprintf(cbt->uuid, CONF_S, "NULL");
	snprintf(cbt->action_type, MAC_S, "NULL");
	snprintf(cbt->os, CONF_S, "NULL");
	snprintf(cbt->os_version, MAC_S, "NULL");
	snprintf(cbt->partition, CONF_S, "NULL");
	snprintf(cbt->varient, CONF_S, "NULL");
	snprintf(cbt->build_domain, RBUFF_S, "NULL");
	snprintf(cbt->arch, MAC_S, "NULL");
	snprintf(cbt->netcard, COMM_S, "NULL");
	snprintf(cbt->config, CONF_S, "/etc/dnsa/dnsa.conf");
}

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
	  CSERVER | LOCALE | SPART | VARIENT | SSCHEME;
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
			if (strncmp(server->name, cml->name, MAC_S) == 0) {
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
	details->spart = cbc->spart;
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
	char *name = details->server->name;
	unsigned long int sid = details->server->server_id;
	char ip[RANGE_S], *addr;
	uint32_t ip_addr;
	cbc_pre_part_s *part = details->spart;

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
	if (part) {
		printf("Partition information:\n");
		if (details->sscheme)
			printf("Name:\t%s\n", details->sscheme->name);
		while (part) {
			if (part->link_id.server_id == sid)
				printf("\t%s\t%s\t%s\n", part->fs, part->log_vol,
				 part->mount);
			part = part->next;
		}
	}
}

int
list_build_servers(cbc_config_s *cmc)
{
	int retval = NONE;
	dbdata_s *data, *list;

	cbc_init_initial_dbdata(&data, SERVERS_WITH_BUILD);
	if ((retval = cbc_run_search(cmc, data, SERVERS_WITH_BUILD)) == 0) {
		printf("No servers have build configurations\n");
		clean_dbdata_struct(data);
		return NONE;
	} else {
		printf("We have %d servers with build configurations\n", retval);
		list = data;
		while (list) {
			printf("%s\n", list->fields.text);
			list = list->next;
		}
		retval = NONE;
	}
	clean_dbdata_struct(data);
	return retval;
}

int
write_build_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
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
	if ((retval = write_build_file(cmc, cml)) != 0) {
		printf("Failed to write build file\n");
		return retval;
	} else {
		printf("build file written\n");
	}
	return retval;
}

int
write_dhcp_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char *ip, line[RBUFF_S];
	int retval = NONE;
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
	dhcp->len = RBUFF_S;
	if ((retval = get_server_id(cmc, cml, &cml->server_id)) != 0)
		return retval;
	if (strncmp(cml->name, "NULL", COMM_S) == 0)
		if ((retval = get_server_name(cmc, cml, cml->server_id)) != 0)
			return retval;
	cbc_init_initial_dbdata(&data, DHCP_DETAILS);
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
	strncpy(dhconf->file, cmc->dhcpconf, CONF_S);
	fill_dhcp_hosts(line, dhcp, dhconf);
	retval = write_file(cmc->dhcpconf, dhcp->string);
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
	char *buff, *cont, *tmp;
	FILE *dhcp_hosts;
	size_t len = 0, blen;

	if (!(dhcp->string = calloc(dhcp->len, sizeof(char))))
		report_error(MALLOC_FAIL, "dhcp->string in write_dhcp_config");
	if (!(buff = calloc(RBUFF_S,  sizeof(char))))
		report_error(MALLOC_FAIL, "buff in fill_dhcp_hosts");
	if (!(dhcp_hosts = fopen(dhconf->file, "r")))
		report_error(FILE_O_FAIL, dhconf->file);
	while (fgets(buff, RBUFF_S, dhcp_hosts)) {
		blen = strlen(buff);
		if (dhcp->len < (blen + len)) {
			dhcp->len *= 2;
			tmp = realloc(dhcp->string, dhcp->len);
			if (!tmp)
				report_error(MALLOC_FAIL, "string in fill_dhcp_hosts");
			else
				dhcp->string = tmp;
		}
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
	if (dhcp->len < (len + RBUFF_S)) {
		dhcp->len *= 2;
		tmp = realloc(dhcp->string, dhcp->len);
		if (!tmp) 
			report_error(MALLOC_FAIL, "string in fill_dhcp_hosts");
		else
			dhcp->string = tmp;
	}
	cont = dhcp->string;
	strncpy(cont + len, line, RBUFF_S);
	free(buff);
}

int
write_tftp_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char out[BUFF_S], pxe[RBUFF_S];
	int retval = NONE;
	dbdata_s *data;

	if (cml->server_id == 0)
		if ((retval = get_server_id(cmc, cml, &cml->server_id)) != 0)
			return retval;
	cbc_init_initial_dbdata(&data, BUILD_IP_ON_SERVER_ID);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, BUILD_IP_ON_SERVER_ID)) == 0) {
		clean_dbdata_struct(data);
		return CANNOT_FIND_BUILD_IP;
	} else if (retval > 1) 
		fprintf(stderr, "Multiple build IP's! Using the first one!\n");
	snprintf(pxe, RBUFF_S, "%s%s%lX", cmc->tftpdir, cmc->pxe,
		 data->fields.number);
	clean_dbdata_struct(data);
	cbc_init_initial_dbdata(&data, TFTP_DETAILS);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, TFTP_DETAILS)) == 0) {
		clean_dbdata_struct(data);
		return NO_TFTP_B_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_TFTP_B_ERR;
	} else {
		fill_tftp_output(cml, data, out);
		retval = write_file(pxe, out);
	}
	if (retval == 0)
	clean_dbdata_struct(data);
	return retval;
}

int
write_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char file[NAME_S];
	int retval = NONE;
	dbdata_s *data;
	string_len_s build = {.len = BUFF_S};

	snprintf(file, NAME_S, "/var/lib/cmdb/web/%s.cfg", cml->name);
	if (cml->server_id == 0)
		if ((retval = get_server_id(cmc, cml, &cml->server_id)) != 0)
			return retval;
	if (!(build.string = calloc(build.len, sizeof(char))))
		report_error(MALLOC_FAIL, "build.string in write_build_file");
	cbc_init_initial_dbdata(&data, NET_BUILD_DETAILS);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, NET_BUILD_DETAILS)) == 0) {
		clean_dbdata_struct(data);
		return NO_NET_BUILD_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_NET_BUILD_ERR;
	} else {
		fill_net_output(cml, data, &build);
		retval = 0;
	}
	clean_dbdata_struct(data);
	cbc_init_initial_dbdata(&data, BUILD_MIRROR);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, BUILD_MIRROR)) == 0) {
		clean_dbdata_struct(data);
		return NO_BUILD_MIRR_ERR;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_BUILD_MIRR_ERR;
	} else {
		fill_mirror_output(cml, data, &build);
		retval = 0;
	}
	if ((retval = fill_partition(cmc, cml, &build)) != 0)
		return retval;
	if ((retval = fill_kernel(cml, &build)) != 0)
		return retval;
	clean_dbdata_struct(data);
	cbc_init_initial_dbdata(&data, BUILD_PACKAGES);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, BUILD_PACKAGES)) == 0) {
		clean_dbdata_struct(data);
		return NO_BUILD_PACKAGES;
	} else {
		fill_packages(cml, data, &build, retval);
		retval = 0;
	}
	clean_dbdata_struct(data);
	if ((retval = fill_app_config(cmc, cml, &build)) != 0) {
		printf("Failed to get application configuration\n");
		return retval;
	}
	retval = write_file(file, build.string);
	free(build.string);
	return retval;
}

void
fill_tftp_output(cbc_comm_line_s *cml, dbdata_s *data, char *output)
{
	dbdata_s *list = data;
	char *bline = list->fields.text;
	list = list->next;
	char *alias = list->fields.text;
	snprintf(cml->os, CONF_S, "%s", alias);
	list = list->next;
	char *osver = list->fields.text;
	list = list->next;
	char *country = list->fields.text;
	list = list->next->next->next;
	char *arg = list->fields.text;
	list = list->next;
	char *url = list->fields.text;
	list = list->next;
	char *arch = list->fields.text;
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
	list = list->next;
	char *keymap = list->fields.text;
	list = list->next;
	char *net_dev = list->fields.text;
	list = list->next;
	uint32_t ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	list = list->next;
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, ns, RANGE_S);
	list = list->next;
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, nm, RANGE_S);
	list = list->next;
	ip_addr = htonl((uint32_t)list->fields.number);
	inet_ntop(AF_INET, &ip_addr, gw, RANGE_S);
	list = list->next;
	char *host = list->fields.text;
	list = list->next;
	char *domain = list->fields.text;

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
d-i debian-installer/locale string \n\
d-i debian-installer/language string \n\
d-i console-keymaps-at/keymap select \n\
d-i keyboard-configuration/xkb-keymap select \n\
d-i keymap select \n\
\n\
d-i netcfg/enable boolean true\n\
d-i netcfg/confirm_static boolean true\n\
d-i netcfg/get_nameservers string \n\
d-i netcfg/get_ipaddress string \n\
d-i netcfg/get_netmask string \n\
d-i netcfg/get_gateway string \n\
\n\
d-i netcfg/get_hostname string \n\
d-i netcfg/get_domain string \n");
	if ((len = strlen(output)) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, len * sizeof(char));
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

	if (strncmp(cml->os, "debian", COMM_S) == 0) {
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
d-i passwd/root-password-crypted password $1$d/0w8MHb$tdqENqvXIz53kZp2svuak1\n\
d-i passwd/user-fullname string Monkey User\n\
d-i passwd/username string monkey\n\
d-i passwd/user-password-crypted password $1$Hir6Ul13$.T1tAO.yfK5g7WDKSw0nI/\n\
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
	}
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
	char *next, disk[FILE_S];
	int retval;
	short int lvm;
	size_t len;
	dbdata_s *data;

	if (cml->server_id == 0)
		if ((retval = get_server_id(cmc, cml, &cml->server_id)) != 0)
			return retval;
	cbc_init_initial_dbdata(&data, BASIC_PART);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, BASIC_PART)) == 0) {
		clean_dbdata_struct(data);
		return NO_BASIC_DISK;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		return MULTI_BASIC_DISK;
	} else {
		add_pre_start_part(cml, data, disk);
	}
	lvm = data->next->fields.small;
	len = strlen(disk);
	next = (disk + len);
	snprintf(next, CONF_S, "\
d-i partman-auto/expert_recipe string                         \\\n");
	next +=64;
	snprintf(next, CONF_S, "\
      monkey ::                                               \\\n");
	next +=64;
	clean_dbdata_struct(data);
	cbc_init_initial_dbdata(&data, FULL_PART);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, FULL_PART)) == 0) {
		clean_dbdata_struct(data);
		return NO_FULL_DISK;
	} else {
		if (lvm > 0) {
			next = add_pre_volume_group(cml, next);
			next = add_pre_lvm_part(data, retval, next);
			snprintf(next, COMM_S, "\n\n");
		} else {
			next = add_pre_part(data, retval, next);
			snprintf(next, COMM_S, "\n\n");
		}
		next++;
	}
	clean_dbdata_struct(data);
	len = strlen(disk);
	if ((build->size + len) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		next = realloc(build->string, build->len * sizeof(char));
		if (!next)
			report_error(MALLOC_FAIL, "next in fill_partition");
		else
			build->string = next;
	}
	snprintf(build->string + build->size, len + 1, "%s", disk);
	build->size += len;
	return NONE;
}

int
fill_kernel(cbc_comm_line_s *cml, string_len_s *build)
{
	char *arch = cml->arch, *tmp, output[BUFF_S];
	size_t len;
	if (strncmp(arch, "i386", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
d-i base-installer/kernel/image string linux-image-2.6-686\n\
\n\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
d-i apt-setup/services-select multiselect security, volatile\n\
d-i apt-setup/security_host string security.debian.org\n\
d-i apt-setup/volatile_host string volatile.debian.org\n\
\n\
tasksel tasksel/first multiselect standard\n");
	} else if (strncmp(arch, "x86_64", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
d-i base-installer/kernel/image string linux-image-2.6-amd64\n\
\n\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
d-i apt-setup/services-select multiselect security, volatile\n\
d-i apt-setup/security_host string security.debian.org\n\
d-i apt-setup/volatile_host string volatile.debian.org\n\
\n\
tasksel tasksel/first multiselect standard\n");
	} else {
		return NO_ARCH;
	}
	len = strlen(output);
	if ((build->size + len) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "next in fill_partition");
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
d-i preseed/late_command string cd /target/root; wget %sscripts/base.sh && chmod 755 base.sh && ./base.sh\n",
		cml->config);
	if ((len + build->size) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "next in fill_partition");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", pack);
	build->size += len;
}

char *
add_pre_start_part(cbc_comm_line_s *cml, dbdata_s *data, char *disk)
{
	short int lvm = data->next->fields.small;
	size_t plen;

	snprintf(cml->partition, CONF_S, "%s", data->fields.text);
	if (lvm == 0)
		snprintf(disk, FILE_S, "\
d-i partman-auto/disk string %s\n\
\n\
d-i partman/choose_partition select Finish partitioning and write changes to disk\n\
d-i partman/confirm_nooverwrite boolean true\n\
d-i partman/confirm boolean true\n\
d-i partman-auto/choose_recipe select monkey\n\
d-i partman-auto/method string regular\n\
d-i partman-auto/purge_lvm_from_device boolean true\n\
d-i partman-md/device_remove_md boolean true\n\
d-i partman-partitioning/confirm_write_new_label boolean true\n\
\n\
", cml->partition);
	else
		snprintf(disk, FILE_S, "\
d-i partman-auto/disk string %s\n\
\n\
d-i partman-auto/choose_recipe select monkey\n\
d-i partman-auto-lvm/guided_size string 100%%\n\
d-i partman-auto-lvm/no_boot boolean true\n\
d-i partman-auto/method string lvm\n\
d-i partman-auto/purge_lvm_from_device boolean true\n\
d-i partman/choose_partition select Finish partitioning and write changes to disk\n\
d-i partman/confirm_nooverwrite boolean true\n\
d-i partman/confirm boolean true\n\
d-i partman-lvm/confirm boolean true\n\
d-i partman-lvm/confirm_nooverwrite boolean true\n\
d-i partman-lvm/device_remove_lvm boolean true\n\
d-i partman-lvm/device_remove_lvm_span boolean true\n\
d-i partman-md/device_remove_md boolean true\n\
d-i partman-partitioning/confirm_write_new_label boolean true\n\
\n\
", cml->partition);
	plen = strlen(disk);
	return (disk + plen);
}

char *
add_pre_volume_group(cbc_comm_line_s *cml, char *next)
{
	char line[CONF_S];
	int i;
	size_t plen;
			snprintf(next, URL_S + 1, "\
              100 1000 1000000000 ext3                        \\\n\
                       $defaultignore{ }                      \\\n\
                       $primary{ }                            \\\n\
                       method{ lvm }                          \\\n");
			next +=256;
			snprintf(line, HOST_S, "\
                       device{ %s }", cml->partition);
			plen = strlen(line);
			for (i = (int)plen; i < 62; i++)
				strcat(line, " ");
			strcat(line, "\\\n");
			snprintf(next, CONF_S, "%s", line);
			next +=64;
			snprintf(next, NAME_S + 1, "\
                       vg_name{ systemlv }                    \\\n\
              .                                               \\\n");
			next += 128;
			return next;
}

char *
add_pre_part(dbdata_s *data, int retval, char *disk)
{
	char *next = disk, line[CONF_S + 1], *fs, *mount;
	int i, j, k = retval;
	size_t len;
	dbdata_s *list = data;

	for (j = 0; j < k; j++) {
		fs = list->next->next->next->fields.text;
		mount = list->next->next->next->next->next->fields.text;
		snprintf(line, HOST_S + 1, "\
              %lu %lu %lu %s", list->fields.number,
		list->next->fields.number,
		list->next->next->fields.number,
		fs);
		len = strlen(line);
		for (i = (int)len; i < 62; i++)
			strcat(line, " ");
		strcat(line, "\\\n");
		snprintf(next, HOST_S + 1, "%s", line);
		next += 64;
		if ((strncmp(fs, "swap", COMM_S) != 0) &&
		    (strncmp(fs, "linux-swap", RANGE_S) != 0)) {
			snprintf(next, HOST_S + 1, "\
                       method{ format } format{ }             \\\n");
			next += 64;
			snprintf(line, HOST_S, "\
                       use_filesystem{ } filesystem{ %s }", fs);
			len = strlen(line);
			for (i = (int)len; i < 62; i++)
				strcat(line, " ");
			strcat(line, "\\\n");
			snprintf(next, HOST_S + 1, "%s", line);
			next +=64;
			snprintf(line, HOST_S, "\
                       mountpoint{ %s }", mount);
			len = strlen(line);
			for (i = (int)len; i < 62; i++)
				strcat(line, " ");
			strcat(line, "\\\n");
			snprintf(next, HOST_S + 1, "%s", line);
			next += 64;
		} else {
			snprintf(next, HOST_S + 1, "\
                       method{ swap } format{ }               \\\n");
			next += 64;
		}
		snprintf(next, HOST_S + 1, "\
              .                                               \\\n");
		next += 64;
		list = list->next->next->next->next->next->next;
	}
	next -=2;
	return next;
}

char *
add_pre_lvm_part(dbdata_s *data, int retval, char *disk)
{
	char *next = disk, line[CONF_S], *fs;
	int i, j, k = retval;
	size_t plen;
	dbdata_s *list = data;

	for (j = 0; j < k; j++) {
		snprintf(line, HOST_S + 1, "\
              %lu %lu %lu %s",	list->fields.number,
		list->next->fields.number,
		list->next->next->fields.number,
		list->next->next->next->fields.text);
		plen = strlen(line);
		for (i = (int)plen; i < 62; i++)
			strcat(line, " ");
		strcat(line, "\\\n");
		snprintf(next, HOST_S + 1, "%s", line);
		next += 64;
		snprintf(next, NAME_S + 1, "\
                       $lvmok                                 \\\n\
                       in_vg{ systemlv }                      \\\n");
		next += 128;
		snprintf(line, HOST_S, "\
                       lv_name{ %s }", list->next->next->next->next->fields.text);
		plen = strlen(line);
		for (i = (int)plen; i < 62; i++)
			strcat(line, " ");
		strcat(line, "\\\n");
		snprintf(next, HOST_S + 1, "%s", line);
		next += 64;
		fs = list->next->next->next->fields.text;
		if ((strncmp(fs, "swap", COMM_S) != 0) &&
		    (strncmp(fs, "linux-swap", RANGE_S) != 0)) {
			snprintf(next, HOST_S + 1, "\
                       method{ format } format{ }             \\\n");
			next += 64;
			snprintf(line, HOST_S, "\
                       use_filesystem{ } filesystem{ %s }", fs);
			plen = strlen(line);
			for (i = (int)plen; i < 62; i++)
				strcat(line, " ");
			strcat(line, "\\\n");
			snprintf(next, HOST_S + 1, "%s", line);
			next +=64;
			snprintf(line, HOST_S, "\
                       mountpoint{ %s }", 
	     list->next->next->next->next->next->fields.text);
			plen = strlen(line);
			for (i = (int)plen; i < 62; i++)
				strcat(line, " ");
			strcat(line, "\\\n");
			snprintf(next, HOST_S + 1, "%s", line);
			next += 64;
		} else {
			snprintf(next, HOST_S + 1, "\
                       method{ swap } format{ }               \\\n");
			next += 64;
		}
		snprintf(next, HOST_S + 1, "\
              .                                               \\\n");
		next += 64;
		list = list->next->next->next->next->next->next;
	}
	next -=2;
	return next;
}

int
fill_app_config(cbc_config_s *cmc, cbc_comm_line_s *cml, string_len_s *build)
{
	int retval;
	dbdata_s *data;

	cbc_init_initial_dbdata(&data, LDAP_CONFIG);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, LDAP_CONFIG)) == 0) {
		printf("No build domain associated with %s?\n", cml->name);
		clean_dbdata_struct(data);
		return BUILD_DOMAIN_NOT_FOUND;
	} else if (retval > 1) {
		printf("Multiple build domains associated with %s?\n",
		       cml->name);
		clean_dbdata_struct(data);
		return MULTIPLE_BUILD_DOMAINS;
	} else {
		if (data->fields.small > 0)
			fill_ldap_config(data, build);
		else
			printf("LDAP authentication configuration skipped\n");
	}
	clean_dbdata_struct(data);
	cbc_init_initial_dbdata(&data, XYMON_CONFIG);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, XYMON_CONFIG)) == 0) {
		printf("No build domain associated with %s?\n", cml->name);
		clean_dbdata_struct(data);
		return BUILD_DOMAIN_NOT_FOUND;
	} else if (retval > 1) {
		printf("Multiple build domains associated with %s?\n",
		       cml->name);
		clean_dbdata_struct(data);
		return MULTIPLE_BUILD_DOMAINS;
	} else {
		if (data->fields.small > 0)
			fill_xymon_config(cml, data, build);
		else
			printf("Xymon configuration skipped\n");
	}
	clean_dbdata_struct(data);
	cbc_init_initial_dbdata(&data, SMTP_CONFIG);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, SMTP_CONFIG)) == 0) {
		printf("No build domain associated with %s?\n", cml->name);
		clean_dbdata_struct(data);
		return BUILD_DOMAIN_NOT_FOUND;
	} else if (retval > 1) {
		printf("Multiple build domains associated with %s?\n",
		       cml->name);
		clean_dbdata_struct(data);
		return MULTIPLE_BUILD_DOMAINS;
	} else {
		fill_smtp_config(cml, data, build);
	}
	return NONE;
}

void
fill_ldap_config(dbdata_s *data, string_len_s *build)
{
	char url[URL_S], buff[BUFF_S], *tmp;
	dbdata_s *list = data->next;
	char *server = list->fields.text;
	list = list->next;
	short int ssl = list->fields.small;
	list = list->next;
	char *base = list->fields.text;
	list = list->next;
	char *root = list->fields.text;
	size_t len;
	if (ssl > 0)
		snprintf(url, URL_S, "ldaps://%s", server);
	else
		snprintf(url, URL_S, "ldap://%s", server);
	snprintf(buff, BUFF_S, "\n\
# Application Configuration\n\
libnss-ldap     libnss-ldap/bindpw      password\n\
libnss-ldap     libnss-ldap/rootbindpw  password\n\
libnss-ldap     libnss-ldap/dblogin     boolean false\n\
libnss-ldap     libnss-ldap/override    boolean true\n\
libnss-ldap     shared/ldapns/base-dn   string  %s\n\
libnss-ldap     libnss-ldap/rootbinddn  string  %s\n\
libnss-ldap     shared/ldapns/ldap_version      select  3\n\
libnss-ldap     libnss-ldap/binddn      string  %s\n\
libnss-ldap     shared/ldapns/ldap-server       string %s\n\
libnss-ldap     libnss-ldap/nsswitch    note\n\
libnss-ldap     libnss-ldap/confperm    boolean false\n\
libnss-ldap     libnss-ldap/dbrootlogin boolean true\n\
\n", base, root, root, url);
	len = strlen(buff);
	if ((len + build->size) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "tmp in fill_ldap_config");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", buff);
	build->size += len;
	snprintf(buff, BUFF_S, "\
libpam-ldap     libpam-ldap/rootbindpw  password\n\
libpam-ldap     libpam-ldap/bindpw      password\n\
libpam-runtime  libpam-runtime/profiles multiselect     unix, winbind, ldap\n\
libpam-ldap     shared/ldapns/base-dn   string  %s\n\
libpam-ldap     libpam-ldap/override    boolean true\n\
libpam-ldap     shared/ldapns/ldap_version      select  3\n\
libpam-ldap     libpam-ldap/dblogin     boolean false\n\
libpam-ldap     shared/ldapns/ldap-server       string  %s\n\
libpam-ldap     libpam-ldap/pam_password        select  crypt\n\
libpam-ldap     libpam-ldap/binddn      string  %s\n\
libpam-ldap     libpam-ldap/rootbinddn  string  %s\n\
libpam-ldap     libpam-ldap/dbrootlogin boolean true\n\
\n", base, url, root, root);
	len = strlen(buff);
	if ((len + build->size) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "tmp in fill_ldap_config");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", buff);
	build->size += len;
}

void
fill_xymon_config(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build)
{
	dbdata_s *list = data->next;
	char *server = list->fields.text;
	char *domain = list->next->fields.text;
	char buff[BUFF_S], *tmp;
	size_t len;

	snprintf(buff, BUFF_S, "\
xymon-client    hobbit-client/HOBBITSERVERS     string  %s\n\
xymon-client    hobbit-client/CLIENTHOSTNAME    string  %s.%s\n\
", server, cml->name, domain);
	len = strlen(buff);
	if ((len + build->size) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "tmp in fill_xymon_config");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", buff);
	build->size += len;
}

void
fill_smtp_config(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build)
{
	short int relay = data->fields.small;
	dbdata_s *list = data->next;
	char *smtp = list->fields.text;
	list = list->next;
	char *domain = list->fields.text;
	char buff[BUFF_S], *tmp;
	size_t len;
	if (relay > 0)
		snprintf(buff, BUFF_S, "\n\
postfix postfix/mailname        string  %s.%s\n\
postfix postfix/main_mailer_type        select  Internet with smarthost\n\
postfix postfix/destinations    string  %s.%s, localhost.%s, localhost\n\
postfix postfix/relayhost       string  %s\n\
", cml->name, domain, cml->name, domain, domain, smtp);
	else
		snprintf(buff, BUFF_S, "\
postfix postfix/mailname        string  %s.%s\n\
postfix postfix/main_mailer_type        select  Internet Site\n\
postfix postfix/destinations    string  %s.%s, localhost.%s, localhost\n\
", cml->name, domain, cml->name, domain, domain);
	len = strlen(buff);
	if ((len + build->size) > build->len) {
		while ((build->size + len) > build->len)
			build->len *=2;
		tmp = realloc(build->string, build->len * sizeof(char));
		if (!tmp)
			report_error(MALLOC_FAIL, "tmp in fill_smtp_config");
		else
			build->string = tmp;
	}
	snprintf(build->string + build->size, len + 1, "%s", buff);
	build->size += len;
}

int
get_server_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *server_id)
{
	int retval = NONE;
	dbdata_s *data;

	if (cml->server == NAME) {
		cbc_init_initial_dbdata(&data, SERVER_ID_ON_SNAME);
		snprintf(data->args.text, CONF_S, "%s", cml->name);
		if ((retval = cbc_run_search(cmc, data, SERVER_ID_ON_SNAME)) == 0) {
			printf("Server %s does not exist\n", cml->name);
			clean_dbdata_struct(data);
			return SERVER_NOT_FOUND;
		} else if (retval > 1) {
			printf("Multiple servers found for name %s\n", cml->name);
			clean_dbdata_struct(data);
			return MULTIPLE_SERVERS;
		} else {
			*server_id = data->fields.number;
			retval = 0;
		}
	} else if (cml->server == UUID) {
		cbc_init_initial_dbdata(&data, SERVER_ID_ON_UUID);
		snprintf(data->args.text, CONF_S, "%s", cml->uuid);
		if ((retval = cbc_run_search(cmc, data, SERVER_ID_ON_UUID)) == 0) {
			printf("Server with uuid %s does not exist\n", cml->uuid);
			clean_dbdata_struct(data);
			return SERVER_NOT_FOUND;
		} else if (retval > 1) {
			printf("Multiple servers found for uuid %s\n", cml->uuid);
			clean_dbdata_struct(data);
			return MULTIPLE_SERVERS;
		} else {
			*server_id = data->fields.number;
			retval = 0;
		}
	} else if (cml->server == ID) {
/*		printf("Writing build files for server id %lu\n\n", cml->server_id); */
		*server_id = cml->server_id;
	} else {
		return NO_NAME_UUID_ID;
	}
	clean_dbdata_struct(data);
	return retval;
}

int
get_server_name(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int server_id)
{
	int retval = NONE;
	dbdata_s *data;

	cbc_init_initial_dbdata(&data, SERVER_NAME_ON_ID);
	data->args.number = server_id;
	if ((retval = cbc_run_search(cmc, data, SERVER_NAME_ON_ID)) == 0) {
		printf("Cannot find server name based on id %lu\n", server_id);
		clean_dbdata_struct(data);
		return SERVER_NOT_FOUND;
	} else if (retval > 1) {
		printf("Multiple servers found base on id %lu\n", server_id);
		printf("Check your database!!!!\n");
		clean_dbdata_struct(data);
		return MULTIPLE_SERVERS;
	} else {
		snprintf(cml->name, CONF_S, "%s", data->fields.text);
		retval = NONE;
	}
	clean_dbdata_struct(data);
	return retval;
}
