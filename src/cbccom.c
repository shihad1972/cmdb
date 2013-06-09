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

#ifdef HAVE_DNSA

# include "cmdb_dnsa.h"

#endif /* HAVE_DNSA */

int
parse_cbc_config_file(cbc_config_s *cbc, const char *config)
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
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DBTYPE=%s", cbc->dbtype);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "PASS=%s", cbc->pass);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "FILE=%s", cbc->file);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "HOST=%s", cbc->host);	
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "USER=%s", cbc->user);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DB=%s", cbc->db);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "SOCKET=%s", cbc->socket);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "PORT=%s", port);
		rewind (cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "TMPDIR=%s", cbc->tmpdir);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "TFTPDIR=%s", cbc->tftpdir);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "PXE=%s", cbc->pxe);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "TOPLEVELOS=%s", cbc->toplevelos);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "PRESEED=%s", cbc->preseed);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "KICKSTART=%s", cbc->kickstart);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DHCPCONF=%s", cbc->dhcpconf);
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

void
parse_cbc_config_error(int error)
{
	if (error == PORT_ERR)
		fprintf(stderr, "Port higher than 65535!\n");
	else if (error == TMP_ERR)
		fprintf(stderr, "Cannot add trailing / to TMPDIR: > 79 characters\n");
	else if (error == TFTP_ERR)
		fprintf(stderr, "Cannot add trailing / to TFTPDIR: > 79 characters\n");
	else if (error == PXE_ERR)
		fprintf(stderr, "Cannot add trailing / to PXE: > 79 characters\n");
	else if (error == OS_ERR)
		fprintf(stderr, "Cannot add trailing / to TOPLEVELOS: > 79 characters\n");
	else if (error == PRESEED_ERR)
		fprintf(stderr, "Cannot add trailing / to PRESEED: > 79 characters\n");
	else if (error == KICKSTART_ERR)
		fprintf(stderr, "Cannot add trailing / to KICKSTART: > 79 characters\n");
	else
		fprintf(stderr, "Unkown error code: %d\n", error);
}

void
init_cbc_config_values(cbc_config_s *cbc)
{
	sprintf(cbc->db, "cmdb");
	sprintf(cbc->dbtype, "none");
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
print_cbc_config(cbc_config_s *cbc)
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

#ifdef HAVE_DNSA

void
copy_cbc_config_to_dnsa(cbc_config_s *cbc, dnsa_config_s *dc)
{
}

#endif
/*
void
init_cbc_build_values(cbc_build_s *build_config)
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
print_cbc_build_ids(cbc_build_s *build)
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
print_cbc_build_values(cbc_build_s *build_config)
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
