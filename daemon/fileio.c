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
 *  fileio.c
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
#include "fileio.h"

/*
 * These are a list of file names; one for cbc, one for cmdb and one for dnsa
 * These will be used in the read / write functions
 * **NOTE**
 * These MUST follow the layout of the respcetive cmbd|dnsa|cbc_s structs so
 * we can use pointer arithmetic when passing these structs as a void data
 * type. The cmdb|cbc|dnsa_sizes are the sizes of the structs. The array
 * indexes MUST match up.
 */

static const char *cmdb_files[] = {
	"servers",
	"customers",
	"contacts",
	"services",
	"service-types",
	"hardware",
	"hardware-types",
	"vmhosts"
};

static const char *cbc_files[] = {
	"builds",
	"build-domains",
	"build-ips",
	"build-os",
	"build-types",
	"disk-devs",
	"locales",
	"build-packages",
	"partitions",
	"partition-schemes",
	"build-varients",
	"system-packages",
	"system-package-arguments",
	"system-package-configurations",
	"system-scripts",
	"system-script-arguments",
	"partition-options",
	NULL,
	NULL
};

static const char *dnsa_files[] = {
	"zones",
	"rev-zones",
	"records",
	"rev-records",
	"glue-zones",
	"preferred-a-records",
	NULL
};

static const size_t cmdb_sizes[] =  {
	sizeof(cmdb_server_s), 
	sizeof(cmdb_customer_s),
	sizeof(cmdb_contact_s),
	sizeof(cmdb_service_s),
	sizeof(cmdb_service_type_s),
	sizeof(cmdb_hardware_s),
	sizeof(cmdb_hard_type_s),
	sizeof(cmdb_vm_host_s)
};

static const size_t cbc_sizes[] = {
	sizeof(cbc_build_s),
	sizeof(cbc_build_domain_s),
	sizeof(cbc_build_ip_s),
	sizeof(cbc_build_os_s),
	sizeof(cbc_build_type_s),
	sizeof(cbc_disk_dev_s),
	sizeof(cbc_locale_s),
	sizeof(cbc_package_s),
	sizeof(cbc_pre_part_s),
	sizeof(cbc_seed_scheme_s),
	sizeof(cbc_varient_s),
	sizeof(cbc_syspack_s),
	sizeof(cbc_syspack_arg_s),
	sizeof(cbc_syspack_conf_s),
	sizeof(cbc_script_s),
	sizeof(cbc_script_arg_s),
	sizeof(cbc_part_opt_s),
	0, 0
};

static const size_t dnsa_sizes[] = {
	sizeof(zone_info_s),
	sizeof(rev_zone_info_s),
	sizeof(record_row_s),
	sizeof(rev_record_row_s),
	sizeof(glue_zone_info_s),
	sizeof(preferred_a_s),
	0
};

static int
write_bin_file(void *d, const char *file, size_t len);

static int
read_bin_file(void *d, const char *file, size_t len);


// Read functions
int
read_cbc(cbc_config_s *cbc, char *dir)
{
	int retval = 0;

	if (!cbc || !dir)
		return CBC_NO_DATA;
	size_t len;

	len = sizeof(cbc);
	printf("%zu: sizeof(cbc)\n", len);
	len = sizeof cbc;
	printf("%zu: sizeof cbc \n", len);
	len = sizeof(cbc_s);
	printf("%zu: sizeof(cbc_s)\n", len);
	len = sizeof(cbc_files);
	printf("%zu: sizeof(cbc_files)\n", len);
	len = sizeof cbc_files;
	printf("%zu: sizeof cbc_files\n", len);
	goto cleanup;

	cleanup:
		return retval;
}

int
read_cmdb(cmdb_config_s *cmdb, char *dir)
{
	int retval = 0;

	if (!cmdb || !dir)
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
	snprintf(f, CONF_S, "%s/data/raw/records", dir);
	len = sizeof(record_row_s);
	d->records = ailsa_malloc(len, "d->records in read_dnsa");
	if ((retval = read_bin_file(d->records, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/raw/zones", dir);
	len = sizeof(zone_info_s);
	d->zones = ailsa_malloc(len, "r->zones in read_dnsa");
	if ((retval = read_bin_file(d->zones, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/raw/rev-records", dir);
	len = sizeof(rev_record_row_s);
	d->rev_records = ailsa_malloc(len, "r->rev_records in read_dnsa");
	if ((retval = read_bin_file(d->rev_records, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/raw/rev-zones", dir);
	len = sizeof(rev_zone_info_s);
	d->rev_zones = ailsa_malloc(len, "r->rev_zones in read_dnsa");
	if ((retval = read_bin_file(d->rev_zones, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/raw/glue-zones", dir);
	len = sizeof(glue_zone_info_s);
	d->glue = ailsa_malloc(len, "r->glue in read_dnsa");
	if ((retval = read_bin_file(d->glue, f, len)) != 0)
		goto cleanup;
	snprintf(f, CONF_S, "%s/data/raw/preferred-a-records", dir);
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
read_bin_file(void *d, const char *file, size_t len)
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
write_cmdb(cmdb_config_s *cmdb, char *dir)
{
	int query, retval = 0;

	if (!cmdb || !dir)
		return CBC_NO_DATA;

	cmdb_s *c = ailsa_malloc(sizeof(cmdb_s), "c in write_cmdb");
	char *file = ailsa_malloc(CONF_S, "file in write_cmdb");
	query = SERVER | CUSTOMER | CONTACT | SERVICE | HARDWARE | VM_HOST;
	if ((retval = cmdb_run_multiple_query(cmdb, c, query)) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/servers", dir);
	if ((retval = write_bin_file(c->server, file, sizeof(cmdb_server_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/customers", dir);
	if ((retval = write_bin_file(c->customer, file, sizeof(cmdb_customer_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/contacts", dir);
	if ((retval = write_bin_file(c->contact, file, sizeof(cmdb_contact_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/services", dir);
	if ((retval = write_bin_file(c->service, file, sizeof(cmdb_service_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/service-types", dir);
	if ((retval = write_bin_file(c->servicetype, file, sizeof(cmdb_service_type_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/hardware", dir);
	if ((retval = write_bin_file(c->hardware, file, sizeof(cmdb_hardware_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/hardware-types", dir);
	if ((retval = write_bin_file(c->hardtype, file, sizeof(cmdb_hard_type_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/vmhosts", dir);
	if ((retval = write_bin_file(c->vmhost, file, sizeof(cmdb_vm_host_s))) != 0)
		goto cleanup;

	cleanup:
		my_free(file);
		cmdb_clean_list(c);
		return retval;
}

int
write_dnsa(dnsa_config_s *dnsa, char *dir)
{
	int query, retval = 0;

	if (!dnsa || !dir)
		return CBC_NO_DATA;

	dnsa_s *d = ailsa_malloc(sizeof(dnsa_s), "d in write_dnsa");
	char *file = ailsa_malloc(CONF_S, "file in write_dnsa");
	query = ZONE | REV_ZONE | RECORD | REV_RECORD | GLUE | PREFERRED_A;
	if ((retval = dnsa_run_multiple_query(dnsa, d, query)) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/zones", dir);
	if ((retval = write_bin_file(d->zones, file, sizeof(zone_info_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/rev-zones", dir);
	if ((retval = write_bin_file(d->rev_zones, file, sizeof(rev_zone_info_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/records", dir);
	if ((retval = write_bin_file(d->records, file, sizeof(record_row_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/rev-records", dir);
	if ((retval = write_bin_file(d->rev_records, file, sizeof(rev_record_row_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/glue-zones", dir);
	if ((retval = write_bin_file(d->glue, file, sizeof(glue_zone_info_s))) != 0)
		goto cleanup;
	snprintf(file, CONF_S, "%sdata/raw/preferred-a-records", dir);
	if ((retval = write_bin_file(d->prefer, file, sizeof(preferred_a_s))) != 0)
		goto cleanup;

	cleanup:
		my_free(file);
		dnsa_clean_list(d);
		return retval;
}
int
write_cbc(cbc_config_s *cbc, char *dir)
{
	char *line;
	int query, retval = 0;
	size_t i, plen, alen;
	void *ptr;

	if (!cbc || !dir)
		return CBC_NO_DATA;

	cbc_s *c = ailsa_malloc(sizeof(cbc_s), "c in write_cbc");
	char *file = ailsa_malloc(CONF_S, "file in write_cbc");
	alen = sizeof cbc_files;
	plen = alen / sizeof c;
	snprintf(file, CONF_S, "%sdata/raw/", dir);
	alen = strnlen(file, CONF_S);
	line = (strrchr(file, '/')) + 1;
	alen = CONF_S - (alen + 1);
	query = BUILD | BUILD_DOMAIN | BUILD_IP | BUILD_OS | BUILD_TYPE | 
                DISK_DEV | LOCALE | BPACKAGE | DPART | SSCHEME | VARIENT |
		SYSPACK | SYSARG | SYSCONF | SCRIPT | SCRIPTA | PARTOPT; // 17 elements

	if ((retval = cbc_run_multiple_query(cbc, c, query)) != 0)
		goto cleanup;
	for (i = 0; i < plen; i++) {
		ptr = *((char **)c + i);
		if (!cbc_files[i])
			break;
		snprintf(line, alen, "%s", cbc_files[i]);
		if ((retval = write_bin_file(ptr, file, cbc_sizes[i])) != 0)
			goto cleanup;
	}
	cleanup:
		my_free(file);
		clean_cbc_struct(c);
		return retval;
}

static int
write_bin_file(void *data, const char *file, size_t len)
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

