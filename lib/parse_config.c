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
 *  parse_config.c
 *
 *  Functions for parsing the configuration file for cmdbd and cmdbcd
 *
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <ailsacmdb.h>
#include "cmdb.h"

struct nv_pair {
	char *name;
	char *value;
	int total;
};

static struct nv_pair nv;

static char *
get_line(FILE *f, char *line, int len);

static int
nv_split(char *line);

static void
log_config_error(int retval, int lineno);

static int
cmdbd_fill_config(struct cmdbd_config *data);

static void
cmdbc_fill_config(struct cmdbc_config *data);

void
cmdbd_parse_config(const char *file, void *data, size_t len)
{
	char *line = NULL;
	int retval = 0, lineno = 0;
	FILE *f;

	if (!(line = calloc(RBUFF_S, sizeof(char)))) {
		syslog(LOG_CRIT, "Out Of Memory!");
		exit(1);
	}
	if (!(f = fopen(file, "r"))) {
		syslog(LOG_ALERT, "Cannot open config file %s", file);
		exit(1);
	}
	while (get_line(f, line, RBUFF_S)) {
		lineno++;
		if ((retval = nv_split(line)) != 0) {
			log_config_error(retval, lineno);
			continue;
		}
		if (len == sizeof(struct cmdbd_config))
			retval = cmdbd_fill_config((struct cmdbd_config *)data);
		else if (len == sizeof(struct cmdbc_config))
			cmdbc_fill_config((struct cmdbc_config *)data);
		if (retval != 0)
			syslog(LOG_ALERT, "Config directory %s too long", nv.value);
		memset(line, 0, RBUFF_S);
	}
	fclose(f);
	free(line);
}

void
cmdbd_clean_config(struct cmdbd_config *c)
{
	if (c->dbtype)
		my_free(c->dbtype);
	if (c->db)
		my_free(c->db);
	if (c->file)
		my_free(c->file);
	if (c->user)
		my_free(c->user);
	if (c->pass)
		my_free(c->pass);
	if (c->host)
		my_free(c->host);
	if (c->dir)
		my_free(c->dir);
	if (c->bind)
		my_free(c->bind);
	if (c->dnsa)
		my_free(c->dnsa);
	if (c->rev)
		my_free(c->rev);
	if (c->rndc)
		my_free(c->rndc);
	if (c->chkz)
		my_free(c->chkz);
	if (c->chkc)
		my_free(c->chkc);
	if (c->socket)
		my_free(c->socket);
	if (c->hostmaster)
		my_free(c->hostmaster);
	if (c->prins)
		my_free(c->prins);
	if (c->secns)
		my_free(c->secns);
	if (c->pridns)
		my_free(c->pridns);
	if (c->secdns)
		my_free(c->secdns);
	if (c->toplevelos)
		my_free(c->toplevelos);
	if (c->pxe)
		my_free(c->pxe);
	if (c->tmpdir)
		my_free(c->tmpdir);
	if (c->preseed)
		my_free(c->preseed);
	if (c->kickstart)
		my_free(c->kickstart);
	if (c->tftpdir)
		my_free(c->tftpdir);
	if (c->dhcpconf)
		my_free(c->dhcpconf);
}

static char *
get_line(FILE *f, char *line, int len)
{
	if (fgets(line, len, f)) {
		ailsa_chomp(line);
		return line;
	}
	return NULL;
}

static int
nv_split(char *line)
{
	char *saved = NULL, *string = NULL;
	size_t len;

	len = sizeof(nv);
	memset(&nv, 0, len);
	if (!(string = strtok_r(line, "=", &saved)))
		return 1;	// This should be an empty line
	if (string[0] == '#')
		return 1;	// This line is a comment
	nv.name = string;
	if ((string = strtok_r(NULL, " \t", &saved)))
		nv.value = string;
	else
		return 2;
	string = nv.name;
	while (*string) {
		nv.total += *string;
		string++;
	}
	return 0;
}

static void
log_config_error(int retval, int lineno)
{
	switch(retval) {
	case 1:
		break;	// Empty line or comment
	case 2:
		syslog(LOG_WARNING, "Name %s has no value at line %d", nv.name, lineno);
		break;
	default:
		syslog(LOG_WARNING, "Unknown config error %d at line %d", retval, lineno);
		break;
	}
}

static int
cmdbd_fill_config(struct cmdbd_config *conf)
{
	int retval = 0;
	unsigned long int i;
	switch(nv.total) {
	case 134:
		conf->db = strndup(nv.value, CONF_S - 1);
		break;
	case 223:
		conf->dir = ailsa_malloc(CONF_S, "conf->dir in cmdbd_fill_config");
		snprintf(conf->dir, CONF_S, "%s", nv.value);
		retval = add_trailing_slash(conf->dir);
		break;
	case 237:
		if (strncmp(nv.name, "REV", COMM_S) == 0) {
			conf->rev = strndup(nv.value, CONF_S - 1);
		} else if (strncmp(nv.name, "PXE", COMM_S) == 0) {
			conf->pxe = ailsa_malloc(RBUFF_S, "conf->pxe in cmdbd_fill_config");
			snprintf(conf->pxe, CONF_S, "%s", nv.value);
			retval = add_trailing_slash(conf->pxe);
		}
		break;
	case 244:
		conf->ttl = strtoul(nv.value, NULL, 10);
		break;
	case 281:
		conf->chkc = strndup(nv.value, CONF_S - 1);
		break;
	case 285:
		conf->bind = ailsa_malloc(CONF_S, "conf->bind in cmdbd_fill_config");
		snprintf(conf->bind, CONF_S, "%s", nv.value);
		retval = add_trailing_slash(conf->bind);
		break;
	case 288:
		conf->file = strndup(nv.value, CONF_S - 1);
		break;
	case 294:
		conf->dnsa = strndup(nv.value, CONF_S - 1);
		break;
	case 295:
		conf->rndc = strndup(nv.value, CONF_S - 1);
		break;
	case 304:
		conf->chkz = strndup(nv.value, CONF_S - 1);
		break;
	case 311:
		conf->pass = strndup(nv.value, CONF_S - 1);
		break;
	case 318:
		conf->host = strndup(nv.value, CONF_S - 1);
		break;
	case 319:
		conf->user = strndup(nv.value, CONF_S - 1);
		break;
	case 325:
		i = strtoul(nv.value, NULL, 10);
		if (i > 65535)
			conf->port = 3306;
		else
			conf->port = (unsigned int)i;
		break;
	case 380:
		conf->secns = strndup(nv.value, RBUFF_S - 1);
		break;
	case 396:
		conf->prins = strndup(nv.value, RBUFF_S - 1);
		break;
	case 406:
		conf->retry = strtoul(nv.value, NULL, 10);
		break;
	case 448:
		conf->secdns = strndup(nv.value, MAC_S - 1);
		break;
	case 456:
		conf->dbtype = strndup(nv.value, MAC_S - 1);
		break;
	case 461:
		conf->expire = strtoul(nv.value, NULL, 10);
		break;
	case 464:
		if (strncmp(nv.name, "PRIDNS", COMM_S) == 0) {
			conf->pridns = strndup(nv.value, RBUFF_S - 1);
		} else if (strncmp(nv.name, "TMPDIR", COMM_S) == 0) {
			conf->tmpdir = ailsa_malloc(CONF_S, "conf->tmpdir in cmdbd_parse_config");
			snprintf(conf->tmpdir, CONF_S, "%s", nv.value);
			retval = add_trailing_slash(conf->tmpdir);
		}
		break;
	case 520:
		conf->preseed = ailsa_malloc(CONF_S, "conf->preseed in cmdbd_parse_config");
		snprintf(conf->preseed, CONF_S, "%s", nv.value);
		retval = add_trailing_slash(conf->preseed);
		break;
	case 527:
		conf->refresh = strtoul(nv.value, NULL, 10);
		break;
	case 541:
		conf->tftpdir = ailsa_malloc(CONF_S, "conf->tftpdir in cmdbd_parse_config");
		snprintf(conf->tftpdir, CONF_S, "%s", nv.value);
		retval = add_trailing_slash(conf->tftpdir);
		break;
	case 581:
		conf->dhcpconf = ailsa_malloc(CONF_S, "conf->dhcpconf in cmdbd_parse_config");
		snprintf(conf->dhcpconf, CONF_S, "%s", nv.value);
		retval = add_trailing_slash(conf->dhcpconf);
		break;
	case 688:
		conf->kickstart = ailsa_malloc(CONF_S, "conf->kickstart in cmdbd_parse_config");
		snprintf(conf->kickstart, CONF_S, "%s", nv.value);
		retval = add_trailing_slash(conf->kickstart);
		break;
	case 778:
		conf->hostmaster = strndup(nv.value, RBUFF_S - 1);
		break;
	case 781:
		conf->toplevelos = ailsa_malloc(CONF_S, "conf->toplevelos in cmdbd_parse_config");
		snprintf(conf->toplevelos, CONF_S, "%s", nv.value);
		retval = add_trailing_slash(conf->toplevelos);
		break;
	default:
		syslog(LOG_WARNING, "Unknown config type %s", nv.name);	
		break;
	}
	return retval;
}

static void
cmdbc_fill_config(struct cmdbc_config *conf)
{
	if (!conf)
		return;
}

