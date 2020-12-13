/*
 *
 *  alisacmdb: Alisatech Configuration Management Database library
 *  Copyright (C) 2015 - 2020 Iain M Conochie <iain-AT-thargoid.co.uk>
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
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <time.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <ailsacmdb.h>
#include <ailsasql.h>

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
	if (i->uuid)
		my_free(i->uuid);
	if (i->coid)
		my_free(i->coid);
	if (i->range)
		my_free(i->range);
	my_free(i);
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
#ifdef HAVE_MYSQL
	if (tmp->type == AILSA_DB_TIME)
		free(tmp->data->time);
#endif
	my_free(tmp->data);
	my_free(tmp);
}

void
ailsa_clean_dhcp(void *dhcp)
{
	if (!(dhcp))
		return;
	ailsa_dhcp_s *data = dhcp;
	if (data->iname)
		my_free(data->iname);
	if (data->dname)
		my_free(data->dname);
	if (data->network)
		my_free(data->network);
	if (data->nameserver)
		my_free(data->nameserver);
	if (data->gateway)
		my_free(data->gateway);
	if (data->netmask)
		my_free(data->netmask);
	my_free(data);
}

void
ailsa_clean_tftp(void *tftp)
{
	if (!(tftp))
		return;
	ailsa_tftp_s *data = tftp;
	if (data->alias)
		my_free(data->alias);
	if (data->version)
		my_free(data->version);
	if (data->arch)
		my_free(data->arch);
	if (data->country)
		my_free(data->country);
	if (data->locale)
		my_free(data->locale);
	if (data->keymap)
		my_free(data->keymap);
	if (data->boot_line)
		my_free(data->boot_line);
	if (data->arg)
		my_free(data->arg);
	if (data->url)
		my_free(data->url);
	if (data->net_int)
		my_free(data->net_int);
	if (data->build_type)
		my_free(data->build_type);
	my_free(data);
}

void
ailsa_clean_build(void *build)
{
	if (!(build))
		return;
	ailsa_build_s *data = build;
	if (data->locale)
		my_free(data->locale);
	if (data->language)
		my_free(data->language);
	if (data->keymap)
		my_free(data->keymap);
	if (data->country)
		my_free(data->country);
	if (data->net_int)
		my_free(data->net_int);
	if (data->ip)
		my_free(data->ip);
	if (data->ns)
		my_free(data->ns);
	if (data->nm)
		my_free(data->nm);
	if (data->gw)
		my_free(data->gw);
	if (data->ntp)
		my_free(data->ntp);
	if (data->host)
		my_free(data->host);
	if (data->domain)
		my_free(data->domain);
	if (data->mirror)
		my_free(data->mirror);
	if (data->os)
		my_free(data->os);
	if (data->version)
		my_free(data->version);
	if (data->os_ver)
		my_free(data->os_ver);
	if (data->arch)
		my_free(data->arch);
	if (data->url)
		my_free(data->url);
	if (data->fqdn)
		my_free(data->fqdn);
	my_free(data);
}

void
ailsa_clean_partition(void *partition)
{
	if (!(partition))
		return;
	ailsa_partition_s *data = partition;
	if (data->mount)
		my_free(data->mount);
	if (data->fs)
		my_free(data->fs);
	if (data->logvol)
		my_free(data->logvol);
	my_free(data);
}

void
ailsa_clean_syspack(void *sysp)
{
	if (!(sysp))
		return;
	ailsa_syspack_s *data = sysp;

	if (data->name)
		my_free(data->name);
	if (data->field)
		my_free(data->field);
	if (data->type)
		my_free(data->type);
	if (data->arg)
		my_free(data->arg);
	if (data->newarg)
		my_free(data->newarg);
	my_free(data);
}

void
ailsa_clean_sysscript(void *script)
{
	if (!(script))
		return;
	ailsa_sysscript_s *data = script;

	if (data->name)
		my_free(data->name);
	if (data->arg)
		my_free(data->arg);
	my_free(data);
}

void
ailsa_clean_dhcp_config(void *dhcp)
{
	if (!(dhcp))
		return;
	ailsa_dhcp_conf_s *data = dhcp;
	if (data->name)
		my_free(data->name);
	if (data->mac)
		my_free(data->mac);
	if (data->ip)
		my_free(data->ip);
	if (data->domain)
		my_free(data->domain);
	my_free(data);
}

void
ailsa_clean_cbcos(void *cbcos)
{
	if (!(cbcos))
		return;
	ailsa_cbcos_s *data = cbcos;
	if (data->os)
		my_free(data->os);
	if (data->os_version)
		my_free(data->os_version);
	if (data->alias)
		my_free(data->alias);
	if (data->ver_alias)
		my_free(data->ver_alias);
	if (data->arch)
		my_free(data->arch);
	if (data->ctime)
		my_free(data->ctime);
	if (data->mtime)
		my_free(data->mtime);
	my_free(data);
}

void
ailsa_clean_iface(void *iface)
{
	if (!(iface))
		return;
	ailsa_iface_s *data = iface;
	if (data->name)
		my_free(data->name);
	my_free(data);
}

void
clean_cbc_syss_s(cbc_syss_s *scr)
{
	if (!(scr))
		return;
	if (scr->name)
		free(scr->name);
	if (scr->arg)
		free(scr->arg);
	if (scr->domain)
		free(scr->domain);
	if (scr->type)
		free(scr->type);
	free(scr);
}

void
ailsa_clean_account(void *acc)
{
	if (!(acc))
		return;
	ailsa_account_s *data = acc;
	if (data->username)
		my_free(data->username);
	if (data->hash)
		my_free(data->hash);
	if (data->pass)
		my_free(data->pass);
	my_free(data);
}
void
ailsa_init_string(ailsa_string_s *str)
{
	str->size = FILE_LEN;
	str->len = 0;
	str->string = ailsa_calloc(FILE_LEN, "str->string in ailsa_init_string");
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

int
cbc_fill_partition_details(AILLIST *list, AILLIST *dest)
{
	if (!(list) || !(dest))
		return AILSA_NO_DATA;
	int retval;
	AILELEM *e = list->head;
	ailsa_data_s *d;
	ailsa_partition_s *p;
	size_t total = 6;
	if ((list->total == 0) || ((list->total % total) != 0)) {
		ailsa_syslog(LOG_ERR, "list in cbc_fill_partition_details has wrong length %zu", list->total);
		return AILSA_WRONG_LIST_LENGHT;
	}
	while (e) {
		p = ailsa_calloc(sizeof(ailsa_partition_s), "p in cbc_fill_partition_details");
		d = e->data;
		p->min = d->data->number;
		d = e->next->data;
		p->max = d->data->number;
		d = e->next->next->data;
		p->pri = d->data->number;
		d = e->next->next->next->data;
		p->mount = strndup(d->data->text, DOMAIN_LEN);
		d = e->next->next->next->next->data;
		p->fs = strndup(d->data->text, SERVICE_LEN);
		d = e->next->next->next->next->next->data;
		p->logvol = strndup(d->data->text, MAC_LEN);
		e = ailsa_move_down_list(e, total);
		if ((retval = ailsa_list_insert(dest, p)) != 0)
			return retval;
	}
	return 0;
}

char *
cmdb_get_uname(unsigned long int uid)
{
	struct passwd pwd;
	struct passwd *result;
	char *buf, *user;
	size_t bsize, usize;
	long sconf;

	if ((sconf = sysconf(_SC_GETPW_R_SIZE_MAX)) < 0)
		bsize = 16384;
	else
		bsize = (size_t)sconf;
	if ((sconf = sysconf(_SC_LOGIN_NAME_MAX)) < 0)
		usize = 256;
	else
		usize = (size_t)sconf;
	buf = ailsa_calloc(bsize, "buf in cmdb_get_uname");
	user = ailsa_calloc(usize, "user in cmdb_get_uname");
	getpwuid_r((uid_t)uid, &pwd, buf, bsize, &result);
	if (!(result)) {
		snprintf(user, usize, "NULL");
	} else {
		snprintf(user, usize, "%s",  pwd.pw_name);
	}
	free(buf);
	return user;
}

AILELEM *
ailsa_clone_data_element(AILELEM *e)
{
	if (!(e))
		return NULL;
	AILELEM *tmp = ailsa_calloc(sizeof(AILELEM), "tmp in ailsa_clone_data");
	ailsa_data_s *data = ailsa_calloc(sizeof(ailsa_data_s), "data in ailsa_clone_data");
	ailsa_data_u *u = ailsa_calloc(sizeof(ailsa_data_u), "u in ailsa_clone_data");
	ailsa_data_s *d = e->data;

	tmp->data = data;
	data->data = u;
	data->type = d->type;
	switch (d->type) {
	case AILSA_DB_TEXT:
		u->text = strndup(d->data->text, CONFIG_LEN);
		break;
#ifdef HAVE_MYSQL
	case AILSA_DB_TIME:
		u->time = ailsa_calloc(sizeof(MYSQL_TIME), "time in ailsa_clone_data");
		memcpy(u->time, d->data->time, sizeof(MYSQL_TIME));
		break;
#endif
	default:
		memcpy(d->data, u, sizeof(ailsa_data_u));
		break;
	}
	return tmp;
}

AILLIST *
ailsa_db_data_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_db_data_list_init");
	ailsa_list_init(list, ailsa_clean_data);
	list->clone = ailsa_clone_data_element;
	return list;
}

AILLIST *
ailsa_dhcp_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_dhcp_list_init");
	ailsa_list_init(list, ailsa_clean_dhcp);
	return list;
}

AILLIST *
ailsa_iface_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_iface_list_init");
	ailsa_list_init(list, ailsa_clean_iface);
	return list;
}

AILLIST *
ailsa_dhcp_config_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_dhcp_config_list_init");
	ailsa_list_init(list, ailsa_clean_dhcp_config);
	return list;
}

AILLIST *
ailsa_cbcos_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_cbcos_list_init");
	ailsa_list_init(list, ailsa_clean_cbcos);
	return list;
}

AILLIST *
ailsa_partition_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_partition_list_init");
	ailsa_list_init(list, ailsa_clean_partition);
	return list;
}

AILLIST *
ailsa_syspack_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_syspack_list_init");
	ailsa_list_init(list, ailsa_clean_syspack);
	return list;
}

AILLIST *
ailsa_sysscript_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_sysscript_list_init");
	ailsa_list_init(list, ailsa_clean_sysscript);
	return list;
}

AILLIST *
ailsa_account_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_account_list_init");
	ailsa_list_init(list, ailsa_clean_account);
	return list;
}

ailsa_data_s *
ailsa_db_text_data_init(void)
{
	ailsa_data_s *data = ailsa_calloc(sizeof(ailsa_data_s), "data in ailsa_db_text_data_init");
	ailsa_init_data(data);
	data->type = AILSA_DB_TEXT;
	return data;
}

ailsa_data_s *
ailsa_db_lint_data_init(void)
{
	ailsa_data_s *data = ailsa_calloc(sizeof(ailsa_data_s), "data in ailsa_db_lint_data_init");
	ailsa_init_data(data);
	data->type = AILSA_DB_LINT;
	return data;
}

ailsa_data_s *
ailsa_db_sint_data_init(void)
{
	ailsa_data_s *data = ailsa_calloc(sizeof(ailsa_data_s), "data in ailsa_db_sint_data_init");
	ailsa_init_data(data);
	data->type = AILSA_DB_SINT;
	return data;
}

int
cmdb_populate_cuser_muser(AILLIST *list)
{
	int retval;
	uid_t uid = getuid();
	ailsa_data_s *data = ailsa_db_lint_data_init();

	data->data->number = (unsigned long int)uid;
	if ((retval = ailsa_list_insert(list, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert cuser into list");
		return retval;
	}
	data = ailsa_db_lint_data_init();
	data->data->number = (unsigned long int)uid;
	if ((retval = ailsa_list_insert(list, data)) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert muser into list");
	return retval;
}

	// The following functions should probably be moved into the ailsasql library
void
cmdb_add_os_name_or_alias_to_list(char *os, char *alias, AILLIST *list)
{
	if ((!(os) && !(alias)) || !(list))
		return;
	ailsa_data_s *d = ailsa_db_text_data_init();
	char *name;
	if (os)
		name = os;
	else
		name = alias;
	d->data->text = strndup(name, MAC_LEN);
	if (ailsa_list_insert(list, d) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert name into list");
		return;
	}
	d = ailsa_db_text_data_init();
	d->data->text = strndup(name, MAC_LEN);
	if (ailsa_list_insert(list, d) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert alias into list");
	return;
}

void
cmdb_add_os_version_or_alias_to_list(char *ver, char *valias, AILLIST *list)
{
	if ((!(ver) && !(valias)) || !(list))
		return;
	ailsa_data_s *d = ailsa_db_text_data_init();
	char *version;
	if (ver)
		version = ver;
	else
		version = valias;
	d->data->text = strndup(version, SERVICE_LEN);
	if (ailsa_list_insert(list, d) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert version into list");
		return;
	}
	d = ailsa_db_text_data_init();
	d->data->text = strndup(version, SERVICE_LEN);
	if (ailsa_list_insert(list, d) != 0)
		ailsa_syslog(LOG_ERR, "Cannot insert valias into list");
	return;
}

int
cmdb_add_string_to_list(const char *str, AILLIST *list)
{
	if (!(str) || !(list))
		return AILSA_NO_DATA;
	int retval;
	ailsa_data_s *d = ailsa_db_text_data_init();
	d->data->text = strndup(str, CONFIG_LEN);
	retval = ailsa_list_insert(list, d);
	return retval;
}

int
cmdb_add_number_to_list(unsigned long int number, AILLIST *list)
{
	if (!(list))
		return AILSA_NO_DATA;
	int retval;
	ailsa_data_s *d = ailsa_db_lint_data_init();
	d->data->number = number;
	retval = ailsa_list_insert(list, d);
	return retval;
}

int
cmdb_add_short_to_list(short int small, AILLIST *list)
{
	if (!(list))
		return AILSA_NO_DATA;
	int retval;
	ailsa_data_s *d = ailsa_db_sint_data_init();
	d->data->small = small;
	retval = ailsa_list_insert(list, d);
	return retval;
}

char *
cmdb_get_string_from_data_list(AILLIST *list, size_t n)
{
	char *str = NULL;
	AILELEM *element = NULL;
	ailsa_data_s *data = NULL;
	if (!(list) || (n == 0))
		return str;
	if (n > list->total)
		return str;
	if (!(element = ailsa_list_get_element(list, n)))
		return str;
	data = element->data;
	if (data->type == AILSA_DB_TEXT)
		str = data->data->text;
	return str;
}

void
random_string(char *str, size_t len)
{
	if (!(str))
		return;
	size_t pos;
	size_t index;
	char charset[] = "0123456789" 
			 "abcdefghijklmnopqrstuvwxyz"
			 "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	srand((unsigned int)(time(NULL)));
	pos = 0;
	while (pos < (len - 1)) {
		index = (size_t) rand() % (sizeof charset - 1);
		*str++ = charset[index];
		pos++;
	}
	*str = '\0';
}
