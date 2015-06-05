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
#include "cmdb.h"
#include "cmdbd.h"

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

static void
cmdbd_fill_config(struct cmdbd_config *data);

static void
cmdbc_fill_config(struct cmdbc_config *data);

int
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
		syslog(LOG_ALERT, "Cannot open config file %s\n", file);
		exit(1);
	}
	while (get_line(f, line, RBUFF_S)) {
		lineno++;
		if ((retval = nv_split(line)) != 0) {
			log_config_error(retval, lineno);
			continue;
		}
		if (len == sizeof(struct cmdbd_config))
			cmdbd_fill_config((struct cmdbd_config *)data);
		else if (len == sizeof(struct cmdbc_config))
			cmdbc_fill_config((struct cmdbc_config *)data);
		memset(line, 0, RBUFF_S);
	}
	fclose(f);
	free(line);
	return 0;
}

static char *
get_line(FILE *f, char *line, int len)
{
	char *p;

	if (fgets(line, len, f)) {
		// Get rid of newline
		p = strchr(line, '\n');
		if (p)
			*p = '\0';
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

static void
cmdbd_fill_config(struct cmdbd_config *conf)
{
	unsigned long int i;
	switch(nv.total) {
	case 134:
		conf->db = strndup(nv.value, CONF_S - 1);
		break;
	case 223:
		conf->dir = strndup(nv.value, CONF_S - 1);
		break;
	case 237:
		conf->rev = strndup(nv.value, CONF_S - 1);
		break;
	case 244:
		conf->ttl = strtoul(nv.value, NULL, 10);
		break;
	case 281:
		conf->chkc = strndup(nv.value, CONF_S - 1);
		break;
	case 285:
		conf->bind = strndup(nv.value, CONF_S - 1);
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
		conf->pass = strndup(nv.value, CONF_S - 1);
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
	case 327:
		conf->pxe = strndup(nv.value, RBUFF_S - 1);
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
		if (strncmp(nv.value, "PRIDNS", COMM_S) == 0)
			conf->pridns = strndup(nv.value, RBUFF_S - 1);
		else if (strncmp(nv.value, "TMPDIR", COMM_S) == 0)
			conf->tmpdir = strndup(nv.value, CONF_S - 1);
		break;
	case 520:
		conf->preseed = strndup(nv.value, CONF_S - 1);
		break;
	case 527:
		conf->refresh = strtoul(nv.value, NULL, 10);
		break;
	case 541:
		conf->tftpdir = strndup(nv.value, CONF_S - 1);
		break;
	case 581:
		conf->dhcpconf = strndup(nv.value, CONF_S - 1);
		break;
	case 688:
		conf->kickstart = strndup(nv.value, CONF_S - 1);
		break;
	case 778:
		conf->hostmaster = strndup(nv.value, RBUFF_S - 1);
		break;
	case 781:
		conf->toplevelos = strndup(nv.value, RBUFF_S - 1);
		break;
	default:
		syslog(LOG_WARNING, "Unkown config type %s", nv.name);
		break;
	}
}

static void
cmdbc_fill_config(struct cmdbc_config *conf)
{
}

