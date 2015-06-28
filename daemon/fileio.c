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

static int
setup_write(void *s, const char *files[], const size_t sizes[], size_t num, const char *dir);

static int
setup_write_new(AILLIST *list, const char *files[], const size_t sizes[], size_t num, const char *dir);

static int
write_bin_file(void *d, const char *file, size_t len);

static int
setup_read(void *s, const char *files[], const size_t sizes[], size_t num, const char *dir);

static int
setup_read_new(AILLIST *list, const char *files[], const size_t sizes[], size_t num, const char *dir);

static int
read_bin_file_new(AILLIST *d, const char *file, size_t len);

static int
read_bin_file(void *d, const char *file, size_t len);

static int
fill_hard_type(AILLIST *l, size_t len);

static void
clean_list(AILLIST *list, size_t len);

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

int
read_cbc(cbc_config_s *cbc, char *dir)
{
	if (!cbc || !dir)
		return CBC_NO_DATA;

	cbc_s *c = ailsa_malloc(sizeof(cbc_s), "c in read_cbc");
	char *f = ailsa_malloc(CONF_S, "f in read_cbc");
	size_t len = sizeof cbc_files / sizeof c;
	snprintf(f, CONF_S, "%sdata/raw/", dir);
	int retval = setup_read(c, cbc_files, cbc_sizes, len, f);

	my_free(f);
	clean_cbc_struct(c);
	return retval;
}

int
write_cbc(cbc_config_s *cbc, char *dir)
{
	if (!cbc || !dir)
		return CBC_NO_DATA;

	int retval = 0;
	cbc_s *c = ailsa_malloc(sizeof(cbc_s), "c in write_cbc");
	char *file = ailsa_malloc(CONF_S, "file in write_cbc");
	size_t len = sizeof cbc_files / sizeof c;
	snprintf(file, CONF_S, "%sdata/raw/", dir);
	int query = BUILD | BUILD_DOMAIN | BUILD_IP | BUILD_OS | BUILD_TYPE | 
                DISK_DEV | LOCALE | BPACKAGE | DPART | SSCHEME | VARIENT |
		SYSPACK | SYSARG | SYSCONF | SCRIPT | SCRIPTA | PARTOPT; // 17 elements

	if ((retval = cbc_run_multiple_query(cbc, c, query)) != 0)
		goto cleanup;
	retval = setup_write(c, cbc_files, cbc_sizes, len, file);

	cleanup:
		my_free(file);
		clean_cbc_struct(c);
		return retval;
}

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

int
read_cmdb(cmdb_config_s *cmdb, char *dir)
{
	int retval;
	char *server, *coid, *hdev, *hdet, *htype, *hclass, *vm; // Servers
	unsigned long int id, cmp, cst, vmid;
	AILELEM *el, *tel;
	cmdb_server_s *ser;
	cmdb_customer_s *cust;
	cmdb_hardware_s *hard;
	cmdb_vm_host_s *virt;
	if (!cmdb || !dir)
		return CBC_NO_DATA;

	char *file = ailsa_malloc(CONF_S, "file in write_cmdb");
	size_t len = sizeof cmdb_files / sizeof file;
	snprintf(file, CONF_S, "%sdata/raw/", dir);
	AILLIST list[len], *l, *tmp;
	if ((retval = setup_read_new(list, cmdb_files, cmdb_sizes, len, file)) != 0)
		goto cleanup;
	l = &list[0];
	if ((retval = fill_hard_type(l, len)) != 0)
		goto cleanup;
	el = l->head;
	while (el) {
		// Get server info
		ser = (cmdb_server_s *)el->data;
		cmp = ser->server_id;
		cst = ser->cust_id;
		server = ser->name;
		vmid = ser->vm_server_id;
		// Get Customer info
		tmp = &list[1];
		tel = tmp->head;
		do {
			cust = (cmdb_customer_s *)tel->data;
			coid = cust->coid;
			id = cust->cust_id;
			tel = tel->next;
		} while (id != cst && tel);
		if (id != cst)
			coid = NULL;
		// Get Vitual Machine info
		tmp = &list[7];
		tel = tmp->head;
		do {
			virt = (cmdb_vm_host_s *)tel->data;
			vm = virt->name;
			id = virt->server_id;
			tel = tel->next;
		} while (id != vmid && tel);
		if (id != vmid)
			vm = NULL;
		// Output the fuckers
		if (vm)
			printf("Server: %s\tCOID: %s\tVM Host: %s\n", server, coid, vm);
		else
			printf("Server: %s\tCOID: %s\n", server, coid);
		// Get hardware info
		tmp = &list[5];
		tel = tmp->head;
		while (tel) {
			hard = (cmdb_hardware_s *)tel->data;
			id = hard->server_id;
			if (id == cmp) {
				hdev = hard->device;
				hdet = hard->detail;
				htype = hard->hardtype->type;
				hclass = hard->hardtype->hclass;
				if (strncmp(htype, "storage", MAC_S) == 0)
					printf("\t%s\t/dev/%s\t%s\n", hclass, hdev, hdet);
				else if (strncmp(htype, "network", MAC_S) == 0)
					printf("\t%s\t%s\t%s\n", hclass, hdev, hdet);
				else
					printf("\t%s\t%s\n", hclass, hdet);
			}
			tel = tel->next;
		}
		el = el->next;
	}

	cleanup:
		my_free(file);
		clean_list(list, len);
		return retval;
}

int
write_cmdb(cmdb_config_s *cmdb, char *dir)
{
	if (!cmdb || !dir)
		return CBC_NO_DATA;

	int retval = 0;
	cmdb_s *c = ailsa_malloc(sizeof(cmdb_s), "c in write_cmdb");
	char *file = ailsa_malloc(CONF_S, "file in write_cmdb");
	size_t len = sizeof cmdb_files / sizeof c;
	snprintf(file, CONF_S, "%sdata/raw/", dir);
	int query = SERVER | CUSTOMER | CONTACT | SERVICE | HARDWARE | VM_HOST;

	if ((retval = cmdb_run_multiple_query(cmdb, c, query)) != 0)
		goto cleanup;
	retval = setup_write(c, cmdb_files, cmdb_sizes, len, file);

	cleanup:
		my_free(file);
		cmdb_clean_list(c);
		return retval;
}

static const char *dnsa_files[] = {
	"zones",
	"rev-zones",
	"records",
	"rev-records",
	"glue-zones",
	"preferred-a-records",
	NULL
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

int
read_dnsa(dnsa_config_s *dnsa, char *dir)
{
	char *f;
	int retval = 0;
	zone_info_s *z;
	record_row_s *r;
	rev_record_row_s *v;
	rev_zone_info_s *e;
	glue_zone_info_s *g;
	preferred_a_s *p;
	AILELEM *el;

	if (!dnsa)
		return CBC_NO_DATA;

	f = ailsa_malloc(CONF_S, "f in read_dnsa");
	snprintf(f, CONF_S, "%s/data/raw/", dir);
	size_t len = sizeof dnsa_files / sizeof f;
	AILLIST list[len], *l;
	if ((retval = setup_read_new(list, dnsa_files, dnsa_sizes, len, f)) != 0)
		goto cleanup;
	l = &list[0];
	el = l->head;
	while (el) {
		z = (zone_info_s *)el->data;
		printf("zone %s\n", z->name);
		el = el->next;
	}
	printf("\n");
	l = &list[2];
	el = l->head;
	while (el) {
		r = (record_row_s *)el->data;
		printf("%s\t%s\n", r->host, r->dest);
		el = el->next;
	}
	printf("\n");
	l = &list[1];
	el = l->head;
	while (el) {
		e = (rev_zone_info_s *)el->data;
		printf("%s\n", e->net_range);
		el = el->next;
	}
	printf("\n");
	l = &list[3];
	el = l->head;
	while (el) {
		v = (rev_record_row_s *)el->data;
		printf("%s\t%s\n", v->host, v->dest);
		el = el->next;
	}
	printf("\n");
	l = &list[4];
	el = l->head;
	while (el) {
		g = (glue_zone_info_s *)el->data;
		printf("%s\n", g->name);
		el = el->next;
	}
	printf("\n");
	l = &list[5];
	el = l->head;
	while (el) {
		p = (preferred_a_s *)el->data;
		printf("%s\t%s\n", p->ip, p->fqdn);
		el = el->next;
	}
	cleanup:
		free(f);
		clean_list(list, len);
		return retval;
}

int
write_dnsa(dnsa_config_s *dnsa, char *dir)
{
	int retval = 0;

	if (!dnsa || !dir)
		return CBC_NO_DATA;

	dnsa_s *d = ailsa_malloc(sizeof(dnsa_s), "d in write_dnsa");
	char *file = ailsa_malloc(CONF_S, "file in write_dnsa");
	size_t len = sizeof dnsa_files / sizeof d;
	snprintf(file, CONF_S, "%sdata/raw/", dir);
	int query = ZONE | REV_ZONE | RECORD | REV_RECORD | GLUE | PREFERRED_A;

	if ((retval = dnsa_run_multiple_query(dnsa, d, query)) != 0)
		goto cleanup;
	retval = setup_write(d, dnsa_files, dnsa_sizes, len, file);

	cleanup:
		my_free(file);
		dnsa_clean_list(d);
		return retval;
}

/*
 * When we pass the pointer into here, it is a struct contaning only pointers.
 * In effect, we can think of this as an array of pointers. The num value
 * tells us how many elements, whereas the sizes array tell us the size of
 * the pointed to structs. The files array is the file names we use to
 * read / write to. We then use this information to malloc() an area of RAM
 * and read 1 record into it at a time. Because the next element in the
 * struct is the top / first element, this is the value in the region of
 * RAM that *ptr is pointing to. We can use this to setup the linked list.
 */

static int
setup_read(void *s, const char *files[], const size_t sizes[], size_t num, const char *dir)
{
	char *file;
	int retval = 0;
	size_t i;
	void *ptr;

	file = ailsa_malloc(CONF_S, "file in setup_read");
	for(i = 0; i < num; i++) {
		ptr = ailsa_malloc(sizes[i], "ptr in setup_read");
		*((char **)s + i) = ptr;
		if (!files[i])
			break;
		snprintf(file, CONF_S, "%s%s", dir, files[i]);
		if ((retval = read_bin_file(ptr, file, sizes[i])) != 0) {
			retval = 1;
			break;
		}
	}
	my_free(file);
	return retval;
}

static int
setup_read_new(AILLIST *list, const char *files[], const size_t sizes[], size_t num, const char *dir)
{
	char *file;
	int retval = 0;
	size_t i;
	void *ptr;

	file = ailsa_malloc(CONF_S, "file in setup_read");
	for (i = 0; i < num; i++)
		ailsa_list_init(&list[i], free);
	for (i = 0; i < num; i++) {
		if (!(files[i]))
			break;
		snprintf(file, CONF_S, "%s%s", dir, files[i]);
		if ((retval = read_bin_file_new(&list[i], file, sizes[i])) != 0) {
			retval = 1;
			break;
		}
	}
	my_free(file);
	return retval;
}

static int
read_bin_file_new(AILLIST *d, const char *file, size_t len)
{
	int retval = 0, mode = O_RDONLY;
	int fd;
	ssize_t slen;
	void *p, *prev;

	if ((fd = open(file, mode)) < 0) {
		retval = 1;
		perror("open() in read_bin_file_new");
		goto cleanup;
	}
	p = ailsa_malloc(len, "p in read_bin_file_new");
	while ((slen = read(fd, p, len)) > 0) {
		if ((retval = ailsa_list_ins_next(d, (AILELEM *)d->tail, p)) != 0) {
			retval = 1;
			fprintf(stderr, "ailsa_list_ins_next failed\n");
			goto cleanup;
		}
		p = ailsa_malloc(len, "p in read_bin_file_new loop");
	}
	if (slen < 0) {
		retval = 1;
		perror("read() failure");
		goto cleanup;
	}
	my_free(p);

	cleanup:
		if (fd > 0)
			if (close(fd) < 0)
				perror("close() failure");
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
		perror("open() in read_bin_file");
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
		if (fd > 0)
			if (close(fd) < 0)
				perror("close() failure");
	return retval;
}

static int
setup_write(void *s, const char *files[], const size_t sizes[], size_t num, const char *dir)
{
	char *file;
	int retval = 0;
	size_t i;
	void *ptr;

	file = ailsa_malloc(CONF_S, "file in setup_write");
	for (i = 0; i < num; i++) {
		ptr = *((char **)s + i);
		if (!files[i])
			break;
		snprintf(file, CONF_S, "%s%s", dir, files[i]);
		if ((retval = write_bin_file(ptr, file, sizes[i])) != 0) {
			retval = 1;
			break;
		}
	}
	my_free(file);
	return retval;
}
 
static int
setup_write_new(AILLIST *list, const char *files[], const size_t sizes[], size_t num, const char *dir)
{
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
		if (fd > 0)
			if (close(fd) < 0)
				perror("close() failure");
		um = umask(um);
		return retval;
}

static void
clean_list(AILLIST *list, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		ailsa_list_destroy(&list[i]);
}
int
fill_hard_type(AILLIST *l, size_t len)
{
	if (!(l)) {
		fprintf(stderr, "NULL pointer passed to fill_hard_type\n");
		return -1;
	}
	if (len < 8) {
		fprintf(stderr, "AILLIST array to small\n");
		return -1;
	}
	int retval = 0;
	AILLIST *hard, *type;
	AILELEM *h, *t;
	cmdb_hardware_s *ch;
	cmdb_hard_type_s *cht;
	hard = &l[5];
	type = &l[6];
	unsigned long int hid, htid;
	h = hard->head;
	while (h) {
		ch = h->data;
		hid = ch->ht_id;
		t = type->head;
		do {
			cht = t->data;
			htid = cht->ht_id;
			ch->hardtype = cht;
			t = t->next;
		} while (htid != hid && (t));
		if (htid != hid) {
			fprintf(stderr, "Hardware %s %s has no type?\n", ch->device, ch->detail);
			retval = -1;
			break;
		}
		h = h->next;
	}
	return retval;
}

