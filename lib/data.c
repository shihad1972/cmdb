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
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include <cmdb.h>

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
#ifdef HAVE_MYSQL
	if (tmp->type == AILSA_DB_TIME)
		free(tmp->data->time);
#endif
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


void
get_config_file_location(char *config)
{
	FILE *cnf;
	const char *conf = config;

	if (snprintf(config, CONF_S, "%s/cmdb/cmdb.conf", SYSCONFDIR) >= CONF_S)
		report_error(BUFFER_TOO_SMALL, "for config file");
	if ((cnf = fopen(conf, "r"))) {
		fclose(cnf);
	} else	{
		if (snprintf(config, CONF_S, "%s/dnsa/dnsa.conf", SYSCONFDIR) >= CONF_S)
			report_error(BUFFER_TOO_SMALL, "for config file");
		if ((cnf = fopen(conf, "r")))
			fclose(cnf);
		else
			report_error(CONF_ERR, "no config file");
	}
}

int
write_file(char *filename, char *output)
{
	int retval = 0;
// Ensure files are written group writable
	mode_t mode = S_IWOTH;
	mode_t em;
	FILE *zonefile;
	em = umask(mode);
	if (!(zonefile = fopen(filename, "w"))) {
		retval = FILE_O_FAIL;
	} else {
		fputs(output, zonefile);
		fclose(zonefile);
		retval = NONE;
	}
	umask(em);
	return retval;
}

void
convert_time(char *timestamp, unsigned long int *store)
{
	char *tmp, *line;
	struct tm timval;
	size_t len;
	time_t epoch;

	if (strncmp(timestamp, "0", COMM_S) == 0) {
		*store = 0;
		return;
	}
	memset(&timval, 0, sizeof timval);
	len = strlen(timestamp) + 1;
	line = strndup(timestamp, len - 1);
	tmp = strtok(line, "-");
	timval.tm_year = (int)strtol(tmp, NULL, 10);
	if (timval.tm_year > 1900)
		timval.tm_year -= 1900;
	tmp = strtok(NULL, "-");
	timval.tm_mon = (int)strtol(tmp, NULL, 10) - 1;
	tmp = strtok(NULL, " ");
	timval.tm_mday = (int)strtol(tmp, NULL, 10);
	tmp = strtok(NULL, ":");
	timval.tm_hour = (int)strtol(tmp, NULL, 10);
	tmp = strtok(NULL, ":");
	timval.tm_min = (int)strtol(tmp, NULL, 10);
	tmp = strtok(NULL, ":");
	timval.tm_sec = (int)strtol(tmp, NULL, 10);
	timval.tm_isdst = -1;
	epoch = mktime(&timval);
	if (epoch < 0)
		*store = 0;
	else
		*store = (unsigned long int)epoch;
	free(line);
}

char *
get_uname(unsigned long int uid)
{
	struct passwd *user;

	if ((user = getpwuid((uid_t)uid)))
		return user->pw_name;
	else
		return NULL;
}


int
get_ip_from_hostname(dbdata_s *data)
{
	int retval = 0;
	struct addrinfo hints, *srvinfo;
	void *addr;
	dbdata_s *list;

	if (!(data))
		return CBC_NO_DATA;
	list = data;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (strlen(list->fields.text) == 0) {
		if ((retval = gethostname(list->fields.text, RBUFF_S)) != 0) {
			fprintf(stderr, "%s", strerror(errno));
			return NO_NAME;
		}
	}
	if ((retval = getaddrinfo(list->fields.text, "http", &hints, &srvinfo)) != 0) {
		fprintf(stderr, "lib getaddrinfo error: %s\n", gai_strerror(retval));
		return NO_IP_ADDRESS;
	}
	if (srvinfo->ai_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)srvinfo->ai_addr;
		addr = &(ipv4->sin_addr);
	} else {
		fprintf(stderr, "ai_family %d not supported\n", srvinfo->ai_family);
		freeaddrinfo(srvinfo);
		return NO_NAME;
	}
	inet_ntop(srvinfo->ai_family, addr, list->args.text, RBUFF_S);
	freeaddrinfo(srvinfo);
	return retval;
}

void
init_dbdata_struct(dbdata_s *data)
{
	memset(data, 0, sizeof(dbdata_s));
}

void
init_multi_dbdata_struct(dbdata_s **list, unsigned int i)
{
	unsigned int max;
	dbdata_s *data = NULL, *dlist = NULL;
	*list = NULL;

	for (max = 0; max < i; max++) {
		if (!(data = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "data in init_multi_dbdata_struct");
		init_dbdata_struct(data);
		if (!(*list)) {
			*list = dlist = data;
		} else {
			while (dlist->next)
				dlist = dlist->next;
			dlist->next = data;
		}
	}
}

void
clean_dbdata_struct(dbdata_s *list)
{
	dbdata_s *data = NULL, *next = NULL;

	if (list)
		data = list;
	else
		return;
	if (data->next)
		next = data->next;
	else
		next = NULL;
	while (data) {
		free(data);
		if (next)
			data = next;
		else
			return;
		if (data->next)
			next = data->next;
		else
			next = NULL;
	}
}

void
init_string_len(string_len_s *string)
{
	string->len = BUFF_S;
	string->size = NONE;
	string->string = ailsa_calloc(BUFF_S, "string->string in init_string_len");
}

void
clean_string_len(string_len_s *string)
{
	if (string) {
		if (string->string)
			free(string->string);
		free(string);
	}
}

void
init_string_l(string_l *string)
{
	memset(string, 0, sizeof(string_l));
	if (!(string->string = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stirng->string in init_string_l");
}

void
clean_string_l(string_l *list)
{
	string_l *data, *next;

	if (list)
		data = list;
	else
		return;
	next = data->next;
	while (data) {
		free(data->string);
		free(data);
		if (next)
			data = next;
		else
			return;
		next = data->next;
	}
}

void
init_initial_string_l(string_l **string, int count)
{
	int i;
	string_l *data, *list = NULL;

	for (i = 0; i < count; i++) {
		if (!(data = malloc(sizeof(string_l))))
			report_error(MALLOC_FAIL, "data in init_initial_string_l");
		init_string_l(data);
		if (!(list)) {
			*string = list = data;
		} else {
			while (list->next)
				list = list->next;
			list ->next = data;
		}
	}
}

AILLIST *
ailsa_db_data_list_init(void)
{
	AILLIST *list = ailsa_calloc(sizeof(AILLIST), "list in ailsa_db_data_list_init");
	ailsa_list_init(list, ailsa_clean_data);
	return list;
}

ailsa_data_s *
ailsa_db_text_data_init(void)
{
	ailsa_data_s *data = ailsa_calloc(sizeof(ailsa_data_s), "data in cmdb_list_contacts_for_customers");
	ailsa_init_data(data);
	data->type = AILSA_DB_TEXT;
	return data;
}

ailsa_data_s *
ailsa_db_lint_data_init(void)
{
	ailsa_data_s *data = ailsa_calloc(sizeof(ailsa_data_s), "data in cmdb_list_contacts_for_customers");
	ailsa_init_data(data);
	data->type = AILSA_DB_LINT;
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
