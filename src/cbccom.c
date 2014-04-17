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
 *  cbccom.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc program
 * 
 */

#include "../config.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif /* HAVE_WORDEXP_H */
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
	FILE *cnf;
	int retval;
#ifdef HAVE_WORDEXP_H
	char **uconf;
	wordexp_t p;
#endif /* HAVE_WORDEXP_H */

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = CONF_ERR;
	} else {
		read_cbc_config_values(cbc, cnf);
		fclose(cnf);
	}
#ifdef HAVE_WORDEXP
	if ((retval = wordexp("~/.dnsa.conf", &p, 0)) == 0) {
		uconf = p.we_wordv;
		if ((cnf = fopen(*uconf, "r"))) {
			if ((retval = read_cbc_config_values(cbc, cnf)) != 0)
				retval = UPORT_ERR;
			fclose(cnf);
		}
		wordfree(&p);
	}
#endif /* HAVE_WORDEXP */
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
read_cbc_config_values(cbc_config_s *cbc, FILE *cnf)
{
	int retval = 0;
	unsigned long int portno;
	char buff[CONF_S] = "";
	char port[CONF_S] = "";

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
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
		return retval;
	} else {
		cbc->port = (unsigned int) portno;
	}
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
