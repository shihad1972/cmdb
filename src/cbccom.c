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
 *  cbccom.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cbc.h"

#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */


int
parse_cbc_config_file(cbc_config_t *cbc, char *config)
{
	FILE *cnf;	/* File handle for config file */
	int retval;
	unsigned long int portno;

	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = CONF_ERR;
	} else {
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PASS=%s", cbc->pass);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "HOST=%s", cbc->host);	
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "USER=%s", cbc->user);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DB=%s", cbc->db);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "SOCKET=%s", cbc->socket);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PORT=%s", port);
		}
		rewind (cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "TMPDIR=%s", cbc->tmpdir);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "TFTPDIR=%s", cbc->tftpdir);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PXE=%s", cbc->pxe);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "TOPLEVELOS=%s", cbc->toplevelos);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PRESEED=%s", cbc->preseed);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "KICKSTART=%s", cbc->kickstart);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DHCPCONF=%s", cbc->dhcpconf);
		}
		retval = 0;
		fclose(cnf);
	}
	
	/* We need to check the value of portno before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
		return retval;
	} else {
		cbc->port = (unsigned int) portno;
	}

	if ((retval = add_trailing_slash(cbc->tmpdir)) != 0)
		retval = TMP_ERR;
	if ((retval = add_trailing_slash(cbc->tftpdir)) != 0)
		retval = TFTP_ERR;
	if ((retval = add_trailing_slash(cbc->pxe)) != 0)
		retval = PXE_ERR;
	if ((retval = add_trailing_slash(cbc->toplevelos)) != 0)
		retval = OS_ERR;
	if ((retval = add_trailing_slash(cbc->preseed)) != 0)
		retval = PRESEED_ERR;
	if ((retval = add_trailing_slash(cbc->kickstart)) !=0)
		retval = KICKSTART_ERR;

	return retval;
}

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_t *cb)
{
	int retval, opt;
	
	retval = NONE;

	while ((opt = getopt(argc, argv, "n:u:i:p::o::v::b::x::l::r:wdac")) != -1) {
		switch (opt) {
			case 'n':
				snprintf(cb->name, CONF_S, "%s", optarg);
				cb->server = TRUE;
				break;
			case 'u':
				snprintf(cb->uuid, CONF_S, "%s", optarg);
				cb->server = TRUE;
				break;
			case 'i':
				cb->server_id = strtoul(optarg, NULL, 10);
				cb->server = TRUE;
				break;
			case 'p':
				snprintf(cb->action_type, MAC_S, "partition");
				snprintf(cb->partition, CONF_S, "%s", optarg);
				break;
			case 'o':
				snprintf(cb->action_type, MAC_S, "os");
				snprintf(cb->os, CONF_S, "%s", optarg);
				break;
			case 'v':
				snprintf(cb->action_type, MAC_S, "os_version");
				snprintf(cb->os_version, MAC_S, "%s", optarg);
				break;
			case 'b':
				snprintf(cb->action_type, MAC_S, "build_domain");
				snprintf(cb->build_domain, RBUFF_S, "%s", optarg);
				break;
			case 'x':
				snprintf(cb->action_type, MAC_S, "varient");
				snprintf(cb->varient, CONF_S, "%s", optarg);
				break;
			case 'l':
				snprintf(cb->action_type, MAC_S, "locale");
				if (optarg)
					cb->locale = strtoul(optarg, NULL, 10);
				break;
			case 'r':
				snprintf(cb->arch, MAC_S, "%s", optarg);
				break;
			case 'w':
				cb->action = WRITE_CONFIG;
				break;
			case 'd':
				cb->action = DISPLAY_CONFIG;
				break;
			case 'a':
				cb->action = ADD_CONFIG;
				break;
			case 'c':
				cb->action = CREATE_CONFIG;
				break;
			default:
				printf("Unknown option: %c\n", opt);
				retval = DISPLAY_USAGE;
				break; 
		}
	}
	if ((cb->action == NONE) && 
	 (cb->server == NONE) &&
	 (strncmp(cb->action_type, "NULL", MAC_S) == 0))
		retval = DISPLAY_USAGE;
	else if (cb->action == NONE)
		retval = NO_ACTION;
	else if ((cb->server == NONE) &&
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
		snprintf(cb->action_type, MAC_S, "create config");
	return retval;
	
}

void
parse_cbc_config_error(int error)
{
	switch(error) {
		case PORT_ERR:
			fprintf(stderr, "Port higher than 65535!\n");
			break;
		case TMP_ERR:
			fprintf(stderr, "Cannot add trailing / to TMPDIR: > 79 characters\n");
			break;
		case TFTP_ERR:
			fprintf(stderr, "Cannot add trailing / to TFTPDIR: > 79 characters\n");
			break;
		case PXE_ERR:
			fprintf(stderr, "Cannot add trailing / to PXE: > 79 characters\n");
			break;
		case OS_ERR:
			fprintf(stderr, "Cannot add trailing / to TOPLEVELOS: > 79 characters\n");
			break;
		case PRESEED_ERR:
			fprintf(stderr, "Cannot add trailing / to PRESEED: > 79 characters\n");
			break;
		case KICKSTART_ERR:
			fprintf(stderr, "Cannot add trailing / to KICKSTART: > 79 characters\n");
			break;
		default:
			fprintf(stderr, "Unkown error code: %d\n", error);
			break;
	}
}

void
init_all_config(cbc_config_t *cct, cbc_comm_line_t *cclt/*, cbc_build_t *cbt*/)
{
	init_cbc_config_values(cct);
	init_cbc_comm_values(cclt);
/*	init_cbc_build_values(cbt); */
}

void
init_cbc_config_values(cbc_config_t *cbc)
{
	sprintf(cbc->db, "cmdb");
	sprintf(cbc->user, "root");
	sprintf(cbc->host, "localhost");
	sprintf(cbc->pass, "%s", "");
	sprintf(cbc->socket, "%s", "");
	sprintf(cbc->tmpdir, "/tmp/cbc");
	sprintf(cbc->tftpdir, "/tftpboot");
	sprintf(cbc->pxe, "pxelinx.cfg");
	sprintf(cbc->toplevelos, "/build");
	sprintf(cbc->dhcpconf, "/etc/dhcpd/dhcpd.hosts");
	sprintf(cbc->preseed, "preseed");
	sprintf(cbc->kickstart, "kickstart");
	cbc->port = 3306;
	cbc->cliflag = 0;
}

void
init_cbc_comm_values(cbc_comm_line_t *cbt)
{
	cbt->action = NONE;
	cbt->server_id = NONE;
	cbt->server = NONE;
	cbt->locale = 0;
	cbt->os_id = 0;
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
/*
void
init_cbc_build_values(cbc_build_t *build_config)
{
	snprintf(build_config->ip_address, COMM_S, "NULL");
	snprintf(build_config->mac_address, COMM_S, "NULL");
	snprintf(build_config->hostname, COMM_S, "NULL");
	snprintf(build_config->domain, COMM_S, "NULL");
	snprintf(build_config->alias, COMM_S, "NULL");
	snprintf(build_config->version, COMM_S, "NULL");
	snprintf(build_config->base_ver, COMM_S, "NULL");
	snprintf(build_config->varient, COMM_S, "NULL");
	snprintf(build_config->arch, COMM_S, "NULL");
	snprintf(build_config->boot, COMM_S, "NULL");
	snprintf(build_config->gateway, COMM_S, "NULL");
	snprintf(build_config->nameserver, COMM_S, "NULL");
	snprintf(build_config->netmask, COMM_S, "NULL");
	snprintf(build_config->ver_alias, COMM_S, "NULL");
	snprintf(build_config->build_type, COMM_S, "NULL");
	snprintf(build_config->arg, COMM_S, "NULL");
	snprintf(build_config->url, COMM_S, "NULL");
	snprintf(build_config->country, COMM_S, "NULL");
	snprintf(build_config->locale, COMM_S, "NULL");
	snprintf(build_config->language, COMM_S, "NULL");
	snprintf(build_config->keymap, COMM_S, "NULL");
	snprintf(build_config->netdev, COMM_S, "NULL");
	snprintf(build_config->mirror, COMM_S, "NULL");
	snprintf(build_config->ntpserver, COMM_S, "NULL");
	snprintf(build_config->diskdev, COMM_S, "NULL");
	snprintf(build_config->part_scheme_name, CONF_S, "NULL");
	build_config->config_ntp = 0;
	build_config->use_lvm = 0;
	build_config->server_id = 0;
	build_config->bd_id = 0;
	build_config->def_scheme_id = 0;
	build_config->os_id = 0;
	build_config->varient_id = 0;
	build_config->boot_id = 0;
	build_config->locale_id = 0;
	build_config->ip_id = 0;
	build_config->build_dom = '\0';
}

void
print_cbc_build_ids(cbc_build_t *build)
{
	fprintf(stderr, "Server: %lu\n", build->server_id);
	fprintf(stderr, "Build domain: %lu\n", build->bd_id);
	fprintf(stderr, "OS: %lu\n", build->os_id);
	fprintf(stderr, "Varient: %lu\n", build->varient_id);
	fprintf(stderr, "Boot: %lu\n", build->boot_id);
	fprintf(stderr, "Locale: %lu\n", build->locale_id);
	fprintf(stderr, "Partition Scheme: %lu\n", build->def_scheme_id);
}

void
print_cbc_build_values(cbc_build_t *build_config)
{
	fprintf(stderr, "########\nBuild Values\n");
	fprintf(stderr, "DISK DEVICE: %s\n", build_config->diskdev);
	if (!build_config->use_lvm)
		fprintf(stderr, "USE LVM: No\n");
	else if (build_config->use_lvm == 1)
		fprintf(stderr, "USE LVM: Yes\n");
	else
		fprintf(stderr, "USE LVM: Unknown!!: %d\n", build_config->use_lvm);
	fprintf(stderr, "PART SCHEME NAME: %s\n", build_config->part_scheme_name);
	fprintf(stderr, "MAC: %s\n", build_config->mac_address);
	fprintf(stderr, "NETWORK DEVICE: %s\n", build_config->netdev);
	fprintf(stderr, "IP: %s\n", build_config->ip_address);
	fprintf(stderr, "NETMASK: %s\n", build_config->netmask);
	fprintf(stderr, "GW: %s\n", build_config->gateway);
	fprintf(stderr, "NS: %s\n", build_config->nameserver);
	fprintf(stderr, "SERVER_ID: %ld\n", build_config->server_id);
	fprintf(stderr, "HOST: %s\n", build_config->hostname);
	fprintf(stderr, "DOMAIN: %s\n", build_config->domain);
	if (build_config->config_ntp > 0)
		fprintf(stderr, "NTP SERVER: %s\n", build_config->ntpserver);
	fprintf(stderr, "OS: %s\n", build_config->alias);
	fprintf(stderr, "OS VERSION: %s\n", build_config->version);
	fprintf(stderr, "OS ALIAS: %s\n", build_config->ver_alias);
	fprintf(stderr, "OS ID: %lu\n", build_config->os_id);
	fprintf(stderr, "ARCH: %s\n", build_config->arch);
	fprintf(stderr, "BUILD VARIENT: %s\n", build_config->varient);
	fprintf(stderr, "BOOT LINE: %s\n", build_config->boot);
	fprintf(stderr, "BUILD TYPE: %s\n", build_config->build_type);
	fprintf(stderr, "ARG: %s\n", build_config->arg);
	fprintf(stderr, "URL: %s\n", build_config->url);
	fprintf(stderr, "MIRROR: %s\n", build_config->mirror);
	fprintf(stderr, "COUNTRY: %s\n", build_config->country);
	fprintf(stderr, "LOCALE: %s\n", build_config->locale);
	fprintf(stderr, "LANGUAGE: %s\n", build_config->language);
	fprintf(stderr, "KEYMAP: %s\n", build_config->keymap);
	fprintf(stderr, "\n");
}
*/
void
print_cbc_command_line_values(cbc_comm_line_t *command_line)
{
	fprintf(stderr, "########\nCommand line Values\n");
	switch (command_line->action) {
		case WRITE_CONFIG:
			fprintf(stderr, "Action: Write configuration file\n");
			break;
		case DISPLAY_CONFIG:
			fprintf(stderr, "Action: Display configuration\n");
			break;
		case ADD_CONFIG:
			fprintf(stderr, "Action: Add configuration for build\n");
			break;
		case CREATE_CONFIG:
			fprintf(stderr, "Action: Create build configuration\n");
			break;
		default:
			fprintf(stderr, "Action: Unknown!!\n");
	}
	fprintf(stderr, "Config: %s\n", command_line->config);
	fprintf(stderr, "Name: %s\n", command_line->name);
	fprintf(stderr, "UUID: %s\n", command_line->uuid);
	fprintf(stderr, "Server ID: %ld\n", command_line->server_id);
	fprintf(stderr, "OS ID: %lu\n", command_line->os_id);
	fprintf(stderr, "OS: %s\n", command_line->os);
	fprintf(stderr, "OS Version: %s\n", command_line->os_version);
	fprintf(stderr, "Architecture: %s\n", command_line->arch);
	fprintf(stderr, "Locale ID: %lu\n", command_line->locale);
	fprintf(stderr, "Build Domain: %s\n", command_line->build_domain);
	fprintf(stderr, "Action Type: %s\n", command_line->action_type);
	fprintf(stderr, "Partition: %s\n", command_line->partition);
	fprintf(stderr, "Varient: %s\n", command_line->varient);
	
	fprintf(stderr, "\n");
}

void
print_cbc_config(cbc_config_t *cbc)
{
	fprintf(stderr, "########\nConfig Values\n");
	fprintf(stderr, "DB: %s\n", cbc->db);
	fprintf(stderr, "USER: %s\n", cbc->user);
	fprintf(stderr, "PASS: %s\n", cbc->pass);
	fprintf(stderr, "HOST: %s\n", cbc->host);
	fprintf(stderr, "PORT: %d\n", cbc->port);
	fprintf(stderr, "SOCKET: %s\n", cbc->socket);
	fprintf(stderr, "TMPDIR: %s\n", cbc->tmpdir);
	fprintf(stderr, "TFTPDIR: %s\n", cbc->tftpdir);
	fprintf(stderr, "PXE: %s\n", cbc->pxe);
	fprintf(stderr, "TOPLEVELOS: %s\n", cbc->toplevelos);
	fprintf(stderr, "DHCPCONF: %s\n", cbc->dhcpconf);
	fprintf(stderr, "PRESEED: %s\n", cbc->preseed);
	fprintf(stderr, "KICKSTART: %s\n", cbc->kickstart);
	fprintf(stderr, "\n");
}
