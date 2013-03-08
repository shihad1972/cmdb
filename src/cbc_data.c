/* 
 *
 *  cbc: Create Build Config
 *  Copyright (C) 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbc_data.c:
 *
 *  Contains functions to initalise the various data structs and clean them up.
 */

#include "../config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "cmdb.h"
#include "cbc_data.h"

void
init_cbc_struct (cbc_t *cbc)
{
	cbc->bootl = '\0';
	cbc->build = '\0';
	cbc->bdom = '\0';
	cbc->bip = '\0';
	cbc->bos = '\0';
	cbc->btype = '\0';
	cbc->diskd = '\0';
	cbc->locale = '\0';
	cbc->package = '\0';
	cbc->dpart = '\0';
	cbc->spart = '\0';
	cbc->sscheme = '\0';
	cbc->server = '\0';
	cbc->varient = '\0';
	cbc->vmhost = '\0';
}

void
clean_cbc_struct (cbc_t *cbc)
{
	if (cbc->bootl)
		clean_boot_line(cbc->bootl);
	if (cbc->build)
		clean_build_struct(cbc->build);
	if (cbc->bdom)
		clean_build_domain(cbc->bdom);
	if (cbc->bip)
		clean_build_ip(cbc->bip);
	if (cbc->bos)
		clean_build_os(cbc->bos);
	if (cbc->btype)
		clean_build_type(cbc->btype);
	if (cbc->diskd)
		clean_disk_dev(cbc->diskd);
	if (cbc->locale)
		clean_locale(cbc->locale);
	if (cbc->package)
		clean_package(cbc->package);
	if (cbc->dpart)
		clean_pre_part(cbc->dpart);
	if (cbc->spart)
		clean_pre_part(cbc->spart);
	if (cbc->sscheme)
		clean_seed_scheme(cbc->sscheme);
	if (cbc->server)
		clean_cbc_server(cbc->server);
	if (cbc->varient)
		clean_varient(cbc->varient);
	if (cbc->vmhost)
		clean_vm_hosts(cbc->vmhost);
}

void
init_boot_line(cbc_boot_line_t *boot)
{
	boot->boot_id = NONE;
	boot->bt_id = NONE;
	snprintf(boot->os, COMM_S, "NULL");
	snprintf(boot->os_ver, COMM_S, "NULL");
	snprintf(boot->boot_line, COMM_S, "NULL");
	boot->next = '\0';
}

void
clean_boot_line(cbc_boot_line_t *boot)
{
	cbc_boot_line_t *list, *next;

	if (boot)
		list = boot;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_build_struct(cbc_build_t *build)
{
	snprintf(build->mac_addr, COMM_S, "NULL");
	snprintf(build->net_int, COMM_S, "NULL");
	build->build_id = NONE;
	build->varient_id = NONE;
	build->server_id = NONE;
	build->os_id = NONE;
	build->boot_id = NONE;
	build->ip_id = NONE;
	build->locale_id = NONE;
	build->next = '\0';
}

void
clean_build_struct(cbc_build_t *build)
{
	cbc_build_t *list, *next;

	if (build)
		list = build;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_build_domain(cbc_build_domain_t *dom)
{
	snprintf(dom->domain, COMM_S, "NULL");
	snprintf(dom->country, COMM_S, "NULL");
	snprintf(dom->language, COMM_S, "NULL");
	snprintf(dom->keymap, COMM_S, "NULL");
	snprintf(dom->ntp_server, COMM_S, "NULL");
	snprintf(dom->ldap_url, COMM_S, "NULL");
	snprintf(dom->ldap_dn, COMM_S, "NULL");
	snprintf(dom->ldap_bind, COMM_S, "NULL");
	snprintf(dom->ldap_host, COMM_S, "NULL");
	snprintf(dom->ldap_server, COMM_S, "NULL");
	snprintf(dom->log_server, COMM_S, "NULL");
	snprintf(dom->nfs_domain, COMM_S, "NULL");
	snprintf(dom->smtp_server, COMM_S, "NULL");
	snprintf(dom->xymon_server, COMM_S, "NULL");
	dom->config_ntp = NONE;
	dom->ldap_ssl = NONE;
	dom->config_ldap = NONE;
	dom->config_log = NONE;
	dom->config_email = NONE;
	dom->config_xymon = NONE;
	dom->bd_id = NONE;
	dom->start_ip = NONE;
	dom->end_ip = NONE;
	dom->netmask = NONE;
	dom->gateway = NONE;
	dom->ns = NONE;
	dom->next = '\0';
}

void
clean_build_domain(cbc_build_domain_t *dom)
{
	cbc_build_domain_t *list, *next;

	if (dom)
		list = dom;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_build_ip(cbc_build_ip_t *ip)
{
	snprintf(ip->host, COMM_S, "NULL");
	snprintf(ip->domain, COMM_S, "NULL");
	ip->ip = NONE;
	ip->ip_id = NONE;
	ip->bd_id = NONE;
	ip->next = '\0';
}

void
clean_build_ip(cbc_build_ip_t *ip)
{
	cbc_build_ip_t *list, *next;

	if (ip)
		list = ip;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_build_os(cbc_build_os_t *os)
{
	snprintf(os->os, COMM_S, "NULL");
	snprintf(os->version, COMM_S, "NULL");
	snprintf(os->alias, COMM_S, "NULL");
	snprintf(os->ver_alias, COMM_S, "NULL");
	snprintf(os->arch, COMM_S, "NULL");
	os->os_id = NONE;
	os->boot_id = NONE;
	os->bt_id = NONE;
	os->next = '\0';
}

void
clean_build_os(cbc_build_os_t *os)
{
	cbc_build_os_t *list, *next;

	if (os)
		list = os;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_build_type(cbc_build_type_t *type)
{
	snprintf(type->alias, COMM_S, "NULL");
	snprintf(type->build_type, COMM_S, "NULL");
	snprintf(type->arg, COMM_S, "NULL");
	snprintf(type->url, COMM_S, "NULL");
	snprintf(type->mirror, COMM_S, "NULL");
	type->bt_id = NONE;
	type->next = '\0';
}

void
clean_build_type(cbc_build_type_t *type)
{
	cbc_build_type_t *list, *next;

	if (type)
		list = type;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_pre_part(cbc_pre_part_t *prep)
{
	snprintf(prep->mount, COMM_S, "NULL");
	snprintf(prep->fs, COMM_S, "NULL");
	snprintf(prep->log_vol, COMM_S, "NULL");
	prep->min = NONE;
	prep->max = NONE;
	prep->pri = NONE;
	prep->server_id = NONE;
	prep->id.part_id = NONE;
	prep->link_id.server_id = NONE;
	prep->next = '\0';
}

void
clean_pre_part(cbc_pre_part_t *prep)
{
	cbc_pre_part_t *list, *next;

	if (prep)
		list = prep;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_disk_dev(cbc_disk_dev_t *disk)
{
	snprintf(disk->device, COMM_S, "NULL");
	disk->lvm = NONE;
	disk->disk_id = NONE;
	disk->server_id = NONE;
	disk->next = '\0';
}

void
clean_disk_dev(cbc_disk_dev_t *disk)
{
	cbc_disk_dev_t *list, *next;

	if (disk)
		list = disk;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_locale(cbc_locale_t *locale)
{
	snprintf(locale->locale, COMM_S, "NULL");
	snprintf(locale->country, COMM_S, "NULL");
	snprintf(locale->language, COMM_S, "NULL");
	snprintf(locale->keymap, COMM_S, "NULL");
	snprintf(locale->timezone, COMM_S, "NULL");
	locale->locale_id = NONE;
	locale->os_id = NONE;
	locale->bt_id = NONE;
	locale->next = '\0';
}

void
clean_locale(cbc_locale_t *locale)
{
	cbc_locale_t *list, *next;

	if (locale)
		list = locale;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_package(cbc_package_t *pack)
{
	snprintf(pack->package, COMM_S, "NULL");
	pack->pack_id = NONE;
	pack->vari_id = NONE;
	pack->os_id = NONE;
	pack->next = '\0';
}

void
clean_package(cbc_package_t *pack)
{
	cbc_package_t *list, *next;

	if (pack)
		list = pack;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_seed_scheme(cbc_seed_scheme_t *seed)
{
	snprintf(seed->name, COMM_S, "NULL");
	seed->lvm = NONE;
	seed->def_scheme_id = NONE;
	seed->next = '\0';
}

void
clean_seed_scheme(cbc_seed_scheme_t *seed)
{
	cbc_seed_scheme_t *list, *next;

	if (seed)
		list = seed;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_cbc_server(cbc_server_t *server)
{
	snprintf(server->vendor, COMM_S, "NULL");
	snprintf(server->make, COMM_S, "NULL");
	snprintf(server->model, COMM_S, "NULL");
	snprintf(server->uuid, COMM_S, "NULL");
	snprintf(server->name, COMM_S, "NULL");
	server->server_id = NONE;
	server->cust_id = NONE;
	server->vm_server_id = NONE;
	server->next = '\0';
}

void
clean_cbc_server(cbc_server_t *server)
{
	cbc_server_t *list, *next;

	if (server)
		list = server;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_varient(cbc_varient_t *vari)
{
	snprintf(vari->varient, COMM_S, "NULL");
	snprintf(vari->valias, COMM_S, "NULL");
	vari->varient_id = NONE;
	vari->next = '\0';
}

void
clean_varient(cbc_varient_t *vari)
{
	cbc_varient_t *list, *next;

	if (vari)
		list = vari;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_vm_hosts(cbc_vm_server_hosts *vm)
{
	snprintf(vm->vm_server, COMM_S, "NULL");
	snprintf(vm->type, COMM_S, "NULL");
	vm->vm_s_id = NONE;
	vm->server_id = NONE;
	vm->next = '\0';
}

void
clean_vm_hosts(cbc_vm_server_hosts *vm)
{
	cbc_vm_server_hosts *list, *next;

	if (vm)
		list = vm;
	else
		return;
	next = list->next;
	while (list) {
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}