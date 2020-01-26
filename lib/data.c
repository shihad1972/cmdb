/*
 *
 *  alisacmdb: Alisatech Configuration Management Database library
 *  Copyright (C) 2015 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  data.c
 *
 *  Contains the functions to initialise and destroy various data types
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ailsacmdb.h>

// Various data functions.

int
ailsa_init_client_info(struct client_info *ci)
{
	AILLIST *list;

	if (!(ci))
		return -1;
	list = ailsa_calloc(sizeof(AILLIST), "hard list in ailsa_init_client_info");
	ailsa_list_init(list, ailsa_clean_hard);
	ci->hard = list;
	list = ailsa_calloc(sizeof(AILLIST), "iface list in ailsa_init_client_info");
	ailsa_list_init(list, ailsa_clean_iface);
	ci->iface = list;
	list = ailsa_calloc(sizeof(AILLIST), "route list in ailsa_init_client_info");
	ailsa_list_init(list, ailsa_clean_route);
	ci->route = list;
	list = ailsa_calloc(sizeof(AILLIST), "file list in ailsa_init_client_info");
	ailsa_list_init(list, ailsa_clean_file);
	ci->file = list;
	list = ailsa_calloc(sizeof(AILLIST), "pkg list in ailsa_init_client_info");
	ailsa_list_init(list, ailsa_clean_pkg);
	ci->pkg = list;
	return 0;
}

// Data clean functions to be used for destroy() in AILLIST

void
ailsa_clean_hard(void *hard)
{
	CMDBHARD *h;

	h = hard;
	if (h->name)
		my_free(h->name);
	if (h->type)
		my_free(h->type);
	if (h->detail)
		my_free(h->detail);
	free(h);
}

void
ailsa_clean_iface(void *iface)
{
	CMDBIFACE *i;

	i = iface;
	if (i->name)
		my_free(i->name);
	if (i->type)
		my_free(i->type);
	if (i->ip)
		my_free(i->ip);
	if (i->nm)
		my_free(i->nm);
	if (i->mac)
		my_free(i->mac);
	free(i);
}
void
ailsa_clean_mkvm(void *vm)
{
	ailsa_mkvm_s *i;

	i = vm;
	if (i->name)
		my_free(i->name);
	if (i->pool)
		my_free(i->pool);
	if (i->uri)
		my_free(i->uri);
	if (i->storxml)
		my_free(i->storxml);
	if (i->path)
		my_free(i->path);
	if (i->network)
		my_free(i->network);
	if (i->netdev)
		my_free(i->netdev);
	if (i->vt)
		my_free(i->vt);
	if (i->vtstr)
		my_free(i->vtstr);
	if (i->mac)
		my_free(i->mac);
	free(i);
}

void
ailsa_clean_route(void *route)
{
	CMDBROUTE *r;

	r = route;
	if (r->dest)
		my_free(r->dest);
	if (r->gw)
		my_free(r->gw);
	if (r->nm)
		my_free(r->nm);
	if (r->iface)
		my_free(r->iface);
	free(r);
}

void
ailsa_clean_file(void *file)
{
	CMDBFILE *f;

	f = file;
	if (f->name)
		my_free(f->name);
	if (f->data)
		my_free(f->data);
	free(f);
}

void
ailsa_clean_pkg(void *pkg)
{
	CMDBPKG *p;

	p = pkg;
	if (p->name)
		my_free(p->name);
	if (p->version)
		my_free(p->version);
	free(p);
}

void
ailsa_clean_client_info(struct client_info *ci)
{
	if (!(ci))
		return;
	if (ci->uuid)
		my_free(ci->uuid);
	if (ci->fqdn)
		my_free(ci->fqdn);
	if (ci->hostname)
		my_free(ci->hostname);
	if (ci->os)
		my_free(ci->os);
	if (ci->distro)
		my_free(ci->distro);
	if (ci->hard) {
		ailsa_list_destroy(ci->hard);
		my_free(ci->hard);
	}
	if (ci->iface) {
		ailsa_list_destroy(ci->iface);
		my_free(ci->iface);
	}
	if (ci->route) {
		ailsa_list_destroy(ci->route);
		my_free(ci->route);
	}
	if (ci->file) {
		ailsa_list_destroy(ci->file);
		my_free(ci->file);
	}
	if (ci->pkg) {
		ailsa_list_destroy(ci->pkg);
		my_free(ci->pkg);
	}
	memset(ci, 0, sizeof(struct client_info));
}

void
ailsa_clean_cmdb(void *cmdb)
{
	ailsa_cmdb_s *i;

	i = cmdb;
	if (i->db)
		my_free(i->db);
	if (i->dbtype)
		my_free(i->dbtype);
	if (i->file)
		my_free(i->file);
	if (i->user)
		my_free(i->user);
	if (i->pass)
		my_free(i->pass);
	if (i->host)
		my_free(i->host);
	if (i->dir)
		my_free(i->dir);
	if (i->bind)
		my_free(i->bind);
	if (i->dnsa)
		my_free(i->dnsa);
	if (i->rev)
		my_free(i->rev);
	if (i->rndc)
		my_free(i->rndc);
	if (i->chkz)
		my_free(i->chkz);
	if (i->chkc)
		my_free(i->chkc);
	if (i->socket)
		my_free(i->socket);
	if (i->hostmaster)
		my_free(i->hostmaster);
	if (i->prins)
		my_free(i->prins);
	if (i->secns)
		my_free(i->secns);
	if (i->pridns)
		my_free(i->pridns);
	if (i->secdns)
		my_free(i->secdns);
	if (i->tmpdir)
		my_free(i->tmpdir);
	if (i->tftpdir)
		my_free(i->tftpdir);
	if (i->pxe)
		my_free(i->pxe);
	if (i->toplevelos)
		my_free(i->toplevelos);
	if (i->dhcpconf)
		my_free(i->dhcpconf);
	if (i->kickstart)
		my_free(i->kickstart);
	if (i->preseed)
		my_free(i->preseed);
	free(i);
}

void
ailsa_init_data(ailsa_data_s *data)
{
	if (!(data))
		return;
	data->data = ailsa_calloc(sizeof(ailsa_data_u), "data->data in ailsa_init_data");
}

void
ailsa_clean_data(void *data)
{
	ailsa_data_s *tmp;
	if (!(data))
		return;
	tmp = data;
	if (tmp->type == AILSA_DB_TEXT)
		free(tmp->data->text);
	my_free(tmp->data);
	my_free(tmp);
}

void
ailsa_init_string(ailsa_string_s *str)
{
	str->size = FILE_LEN;
	str->len = 0;
	str->string = ailsa_calloc(FILE_LEN, "string->string in ailsa_init_string");
}

void
ailsa_clean_string(ailsa_string_s *str)
{
	if (!(str))
		return;
	if (str->string)
		my_free(str->string);
	my_free(str);
}

void
ailsa_resize_string(ailsa_string_s *str)
{
	char *tmp;

	str->size *= 2;
	tmp = ailsa_realloc(str->string, str->size * sizeof(char), "In ailsa_resize_string");
	str->string = tmp;
}

void
ailsa_fill_string(ailsa_string_s *str, const char *s)
{
	size_t len;

	len = strlen(s);
	if (len + str->len >= str->size)
		ailsa_resize_string(str);
	len++;
	snprintf(str->string + str->len, len, "%s", s);
	str->len = strlen(str->string);
}

