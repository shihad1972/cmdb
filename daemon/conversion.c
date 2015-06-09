/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  conversion.c
 *
 *  Contains functions to convert new to old config structs for old DB code
 *
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ailsacmdb.h>
#include "cmdb.h"
#include "cmdb_data.h"
#include "dnsa_data.h"
#include "cbc_data.h"
#include "cmdb_dnsa.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cmdb_base_sql.h"
#include "dnsa_base_sql.h"
#include "cmdb_cbc.h"
#include "conversion.h"

int
convert_dnsa_config(dnsa_config_s *dcs, struct cmdbd_config *cdc)
{
	if (!cdc || !dcs)
		return CBC_NO_DATA;
	snprintf(dcs->dbtype, RANGE_S, "%s", cdc->dbtype);
	snprintf(dcs->db, CONF_S, "%s", cdc->db);
	snprintf(dcs->file, CONF_S, "%s", cdc->file);
	snprintf(dcs->user, CONF_S, "%s", cdc->user);
	snprintf(dcs->pass, CONF_S, "%s", cdc->pass);
	snprintf(dcs->host, CONF_S, "%s", cdc->host);
	snprintf(dcs->dir, CONF_S, "%s", cdc->dir);
	snprintf(dcs->bind, CONF_S, "%s", cdc->bind);
	snprintf(dcs->dnsa, CONF_S, "%s", cdc->dnsa);
	snprintf(dcs->rev, CONF_S, "%s", cdc->rev);
	snprintf(dcs->rndc, CONF_S, "%s", cdc->rndc);
	snprintf(dcs->chkz, CONF_S, "%s", cdc->chkz);
	snprintf(dcs->chkc, CONF_S, "%s", cdc->chkc);
	snprintf(dcs->socket, CONF_S, "%s", cdc->socket);
	snprintf(dcs->hostmaster, RBUFF_S, "%s", cdc->hostmaster);
	snprintf(dcs->prins, RBUFF_S, "%s", cdc->prins);
	snprintf(dcs->secns, RBUFF_S, "%s", cdc->secns);
	snprintf(dcs->pridns, MAC_S, "%s", cdc->pridns);
	snprintf(dcs->secdns, MAC_S, "%s", cdc->secdns);
	dcs->refresh = cdc->refresh;
	dcs->retry = cdc->retry;
	dcs->expire = cdc->expire;
	dcs->ttl = cdc->ttl;
	dcs->port = cdc->port;
	dcs->cliflag = cdc->cliflag;
	return 0;
}

int
convert_cbc_config(cbc_config_s *ccs, struct cmdbd_config *cdc)
{
	if (!ccs || !cdc)
		return CBC_NO_DATA;
	snprintf(ccs->dbtype, RANGE_S, "%s", cdc->dbtype);
	snprintf(ccs->db, CONF_S, "%s", cdc->db);
	snprintf(ccs->file, CONF_S, "%s", cdc->file);
	snprintf(ccs->user, CONF_S, "%s", cdc->user);
	snprintf(ccs->pass, CONF_S, "%s", cdc->pass);
	snprintf(ccs->host, CONF_S, "%s", cdc->host);
	snprintf(ccs->socket, CONF_S, "%s", cdc->socket);
	snprintf(ccs->tmpdir, CONF_S, "%s", cdc->tmpdir);
	snprintf(ccs->tftpdir, CONF_S, "%s", cdc->tftpdir);
	snprintf(ccs->pxe, CONF_S, "%s", cdc->pxe);
	snprintf(ccs->toplevelos, CONF_S, "%s", cdc->toplevelos);
	snprintf(ccs->dhcpconf, CONF_S, "%s", cdc->dhcpconf);
	snprintf(ccs->kickstart, CONF_S, "%s", cdc->kickstart);
	snprintf(ccs->preseed, CONF_S, "%s", cdc->preseed);
	ccs->port = cdc->port;
	ccs->cliflag = cdc->cliflag;
	return 0;
}

int
convert_cmdb_config(cmdb_config_s *ccs, struct cmdbd_config *cdc)
{
	if (!ccs || !cdc)
		return CBC_NO_DATA;
	snprintf(ccs->dbtype, RANGE_S, "%s", cdc->dbtype);
	snprintf(ccs->db, CONF_S, "%s", cdc->db);
	snprintf(ccs->file, CONF_S, "%s", cdc->file);
	snprintf(ccs->user, CONF_S, "%s", cdc->user);
	snprintf(ccs->pass, CONF_S, "%s", cdc->pass);
	snprintf(ccs->host, CONF_S, "%s", cdc->host);
	snprintf(ccs->socket, CONF_S, "%s", cdc->socket);
	ccs->port = cdc->port;
	ccs->cliflag = cdc->cliflag;
	return 0;
}

int
convert_all_config(struct all_config *c)
{
	int retval = 0;
	memset(c->dnsa, 0, sizeof(dnsa_config_s));
	memset(c->cmdb, 0, sizeof(cmdb_config_s));
	memset(c->cbc, 0, sizeof(cbc_config_s));
	if ((retval = convert_cmdb_config(c->cmdb, c->cmdbd)) != 0)
		return retval;
	if ((retval = convert_cbc_config(c->cbc, c->cmdbd)) != 0)
		return retval;
	if ((retval = convert_dnsa_config(c->dnsa, c->cmdbd)) != 0)
		return retval;
	return retval;
}

