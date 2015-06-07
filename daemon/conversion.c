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
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cmdb_base_sql.h"
#include "dnsa_base_sql.h"
#include "cmdb_cmdb.h"
#include "dnsa_data.h"
#include "cmdb_dnsa.h"
#include "cbc_data.h"
#include "cmdb_cbc.h"
#include "conversion.h"

static int
write_bin_file(void *data, char *file, size_t len);

static int
read_bin_file(void *d, char *file, size_t len);

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

// Read functions
int
read_cmdb(cmdb_config_s *cmdb)
{
	int retval = 0;

	if (!cmdb)
		return CBC_NO_DATA;

	return retval;
}

int
read_dnsa(dnsa_config_s *dnsa, char *dir)
{
	char *f;
	int retval = 0;
	size_t len = sizeof(dnsa_s);
	zone_info_s *z;
	record_row_s *r;
	rev_record_row_s *v;
	rev_zone_info_s *e;
	glue_zone_info_s *g;
	preferred_a_s *p;

	if (!dnsa)
		return CBC_NO_DATA;

	dnsa_s *d = ailsa_malloc(len, "d in read_dnsa");
	f = ailsa_malloc(CONF_S, "f in read_dnsa");
	snprintf(f, CONF_S, "%s/data/records", dir);
	len = sizeof(record_row_s);
	d->records = ailsa_malloc(len, "d->records in read_dnsa");
	if ((retval = read_bin_file(d->records, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/zones", dir);
	len = sizeof(zone_info_s);
	d->zones = ailsa_malloc(len, "r->zones in read_dnsa");
	if ((retval = read_bin_file(d->zones, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/rev-records", dir);
	len = sizeof(rev_record_row_s);
	d->rev_records = ailsa_malloc(len, "r->rev_records in read_dnsa");
	if ((retval = read_bin_file(d->rev_records, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/rev-zones", dir);
	len = sizeof(rev_zone_info_s);
	d->rev_zones = ailsa_malloc(len, "r->rev_zones in read_dnsa");
	if ((retval = read_bin_file(d->rev_zones, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/glue-zones", dir);
	len = sizeof(glue_zone_info_s);
	d->glue = ailsa_malloc(len, "r->glue in read_dnsa");
	if ((retval = read_bin_file(d->glue, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/preferred-a-records", dir);
	len = sizeof(preferred_a_s);
	d->prefer = ailsa_malloc(len, "r->prefer in read_dnsa");
	if ((retval = read_bin_file(d->prefer, f, len)) != 0)
		goto cleanup;

	z = d->zones;
	while (z) {
		printf("Zone %s\n", z->name);
		z = z->next;
	}
	r = d->records;
	while (r) {
		printf("Record: %s\t%s\t%s\n", r->host, r->dest, r->type);
		r = r->next;
	}
	e = d->rev_zones;
	while (e) {
		printf("Rev zone: %s\n", e->net_range);
		e = e->next;
	}
	v = d->rev_records;
	while (v) {
		printf("PTR: %s -> %s\n", v->host, v->dest);
		v = v->next;
	}
	g = d->glue;
	while (g) {
		printf("Glue zone: %s\n", g->name);
		g = g->next;
	}
	p = d->prefer;
	while (p) {
		printf("Prefer: %s for %s\n", p->fqdn, p->ip);
		p = p->next;
	}

	cleanup:
		free(f);
		dnsa_clean_list(d);
		return retval;
}

static int
read_bin_file(void *d, char *file, size_t len)
{
	int retval = 0, mode = O_RDONLY;
	int fd;
	ssize_t slen = 1;
	void *p, *prev;

	if ((fd = open(file, mode)) < 0) {
		retval = 1;
		perror("open() int read_bin_file");
		goto cleanup;
	}
	p = d;
	prev = p;
	while ((slen = read(fd, p, len)) > 0) {
		prev = p;
		p = ailsa_malloc(len, "p in read_bin_file");
		*(char **)prev = p;
	}
	if (slen < 0) {
		retval = 1;
		perror("read() failure");
		goto cleanup;
	}
	*(char **)prev = NULL;
	my_free(p);

	cleanup:
		if (fd != 0)
			if (close(fd) < 0)
				perror("close() failure");
	return retval;
}

// write functions
int
write_cmdb(cmdb_config_s *cmdb)
{
	int retval = 0;

	if (!cmdb)
		return CBC_NO_DATA;

	return retval;
}

int
write_dnsa(dnsa_config_s *dnsa, char *dir)
{
	int query, retval = 0;

	if (!dnsa || !dir)
		return CBC_NO_DATA;

	dnsa_s *d = ailsa_malloc(sizeof(dnsa_s), "d in write_zones");
	char *file = ailsa_malloc(CONF_S, "file in write_zones");
	query = ZONE | REV_ZONE | RECORD | REV_RECORD | GLUE | PREFERRED_A;
	if ((retval = dnsa_run_multiple_query(dnsa, d, query)) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%s/data/records", dir);
	if ((retval = write_bin_file(d->records, file, sizeof(record_row_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%s/data/zones", dir);
	if ((retval = write_bin_file(d->zones, file, sizeof(zone_info_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%s/data/rev-records", dir);
	if ((retval = write_bin_file(d->rev_records, file, sizeof(rev_record_row_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%s/data/rev-zones", dir);
	if ((retval = write_bin_file(d->rev_zones, file, sizeof(rev_zone_info_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%s/data/glue-zones", dir);
	if ((retval = write_bin_file(d->glue, file, sizeof(glue_zone_info_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%s/data/preferred-a-records", dir);
	if ((retval = write_bin_file(d->prefer, file, sizeof(preferred_a_s))) != 0)
		goto cleanup;

	cleanup:
		free(file);
		dnsa_clean_list(d);
		return retval;
}

static int
write_bin_file(void *data, char *file, size_t len)
{
	int retval = 0, fd = 0;
	int mode = O_CREAT | O_TRUNC | O_WRONLY;
	int flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	ssize_t slen;
	mode_t um;

	um = umask(0);
	if ((fd = open(file, mode, flags)) < 0) {
		perror("open() in write_bin_file");
		goto cleanup;
	}
	while (data) {
		if ((slen = write(fd, data, len)) < 0) {
			perror("write() failure");
			goto cleanup;
		}
		data = *(char **)data;
	}

	cleanup:
		if (fd != 0)
			if (close(fd) < 0)
				perror("close() failure");
		um = umask(um);
		return retval;
}

