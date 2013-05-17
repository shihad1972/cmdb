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
	while ((opt = getopt(argc, argv, "n:i:u:wdmcrl")) != -1) {
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
	cbt->package = 0;
	snprintf(cbt->name, CONF_S, "NULL");
	snprintf(cbt->uuid, CONF_S, "NULL");
	snprintf(cbt->action_type, MAC_S, "NULL");
	snprintf(cbt->os, CONF_S, "NULL");
	snprintf(cbt->os_version, MAC_S, "NULL");
	snprintf(cbt->partition, CONF_S, "NULL");
	snprintf(cbt->varient, CONF_S, "NULL");
	snprintf(cbt->build_domain, RBUFF_S, "NULL");
	snprintf(cbt->arch, MAC_S, "NULL");
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
	if ((retval = run_multiple_query(cbt, cbc, query)) != 0) {
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
	osid = bid = ipid = lid = vid = bdid = btid = 0;

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

	if ((retval = write_dhcp_config(cmc, cml)) != 0)
		return retval;
	if ((retval = write_tftp_config(cmc, cml)) != 0)
		return retval;
	if ((retval = write_build_file(cmc, cml)) != 0)
		return retval;
	return retval;
}

int
write_dhcp_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char *ip;
	int retval = NONE;
	dbdata_s *data;
	uint32_t ip_addr;

	if (!(ip = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in write_dhcp_config");
	if ((retval = get_server_id(cmc, cml, &cml->server_id)) != 0)
		return retval;
	if (strncmp(cml->name, "NULL", COMM_S) == 0)
		if ((retval = get_server_name(cmc, cml, cml->server_id)) != 0)
			return retval;
/*	printf("Writing build files for server %s\n\n", cml->name); 
	printf("Got server id %lu\tname: %s\n", cml->server_id, cml->name); */
	cbc_init_initial_dbdata(&data, DHCP_DETAILS);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, DHCP_DETAILS)) == 0) {
		printf("Cannot find dhcp details for server id %lu\n", cml->server_id);
		clean_dbdata_struct(data);
		return NO_DHCP_B_ERR;
	} else if (retval > 1) {
		printf("Multiple dhcp details for server id %lu\n", cml->server_id);
		clean_dbdata_struct(data);
		return MULTI_DHCP_B_ERR;
	} else {
		ip_addr = htonl((uint32_t)data->next->fields.number);
		inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
		printf("host %s { hardware ethernet %s; fixed-address %s; \
option domain-name \"%s\"; }\n\n", cml->name, data->fields.text, ip,
data->next->next->fields.text);
		retval = 0;
	}
	free(ip);
	clean_dbdata_struct(data);
	return retval;
}

int
write_tftp_config(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char out[BUFF_S];
	int retval = NONE;
	dbdata_s *data;

	if (cml->server_id == 0)
		if ((retval = get_server_id(cmc, cml, &cml->server_id)) != 0)
			return retval;
	if (strncmp(cml->name, "NULL", COMM_S) == 0)
		if ((retval = get_server_name(cmc, cml, cml->server_id)) != 0)
			return retval;
/*	printf("\nTFTP\nGot server id %lu\tname %s\n", cml->server_id, cml->name); */
	cbc_init_initial_dbdata(&data, TFTP_DETAILS);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, TFTP_DETAILS)) == 0) {
		printf("Cannot find TFTP details for server %s\n", cml->name);
		clean_dbdata_struct(data);
		return NO_TFTP_B_ERR;
	} else if (retval > 1) {
		printf("Multiple TFTP details for server %s\n", cml->name);
		clean_dbdata_struct(data);
		return MULTI_TFTP_B_ERR;
	} else {
		fill_tftp_output(cml, data, out);
		printf("%s", out);
		retval = 0;
	}
	clean_dbdata_struct(data);
	return retval;
}

int
write_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml)
{
	char net_build[BUFF_S];
	int retval = NONE;
	size_t len;
	dbdata_s *data;

	if (cml->server_id == 0)
		if ((retval = get_server_id(cmc, cml, &cml->server_id)) != 0)
			return retval;
	if (strncmp(cml->name, "NULL", COMM_S) == 0)
		if ((retval = get_server_name(cmc, cml, cml->server_id)) != 0)
			return retval;
	cbc_init_initial_dbdata(&data, NET_BUILD_DETAILS);
	data->args.number = cml->server_id;
	if ((retval = cbc_run_search(cmc, data, NET_BUILD_DETAILS)) == 0) {
		printf("Cannot find NET_BUILD_DETAILS for server %s\n",
		       cml->name);
		clean_dbdata_struct(data);
		return NO_NET_BUILD_ERR;
	} else if (retval > 1) {
		printf("Multiple NET_BUILD_DETAILS for server %s\n", cml->name);
		clean_dbdata_struct(data);
		return MULTI_NET_BUILD_ERR;
	} else {
		fill_net_build_output(cml, data, net_build);
		printf("%s", net_build);
		retval = 0;
	}
	len = strlen(net_build);
	printf("Length is %zu\n", len);
	clean_dbdata_struct(data);
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
}

void
fill_net_build_output(cbc_comm_line_s *cml, dbdata_s *data, char *output)
{
	dbdata_s *list = data;
	char *ip, *ns, *nm, *gw;

	if (!(ip = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in fill_net_build_output");
	if (!(ns = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in fill_net_build_output");
	if (!(nm = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in fill_net_build_output");
	if (!(gw = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in fill_net_build_output");
	char *locale = list->args.text;
	list = list->next;
	char *keymap = list->args.text;
	list = list->next;
	char *net_dev = list->args.text;
	list = list->next;
	uint32_t ip_addr = htonl((uint32_t)list->args.number);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	list = list->next;
	ip_addr = htonl((uint32_t)list->args.number);
	inet_ntop(AF_INET, &ip_addr, ns, RANGE_S);
	list = list->next;
	ip_addr = htonl((uint32_t)list->args.number);
	inet_ntop(AF_INET, &ip_addr, nm, RANGE_S);
	list = list->next;
	ip_addr = htonl((uint32_t)list->args.number);
	inet_ntop(AF_INET, &ip_addr, gw, RANGE_S);
	list = list->next;
	char *host = list->args.text;
	list = list->next;
	char *domain = list->args.text;

	if (strncmp(cml->os, "debian", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string %s\n\
d-i console-keymaps-at/keymap select %s\n\
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
locale, keymap, keymap, net_dev, ns, ip, nm, gw, host, domain);
	} else if (strncmp(cml->os, "ubuntu", COMM_S) == 0) {
		snprintf(output, BUFF_S, "\
d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string \n\
d-i debian-installer/language string \n\
d-i console-keymaps-at/keymap select \n\
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
	}
	free(ip);
	free(gw);
	free(ns);
	free(nm);
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
