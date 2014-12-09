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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "cmdb.h"
#include "cbc_data.h"

void
init_cbc_struct (cbc_s *cbc)
{
	memset(cbc, 0, sizeof(cbc_s));
}

void
clean_cbc_struct (cbc_s *cbc)
{
	if (!(cbc))
		return;
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
	free(cbc);
}

void
init_boot_line(cbc_boot_line_s *boot)
{
	memset(boot, 0, sizeof(cbc_boot_line_s));
	snprintf(boot->os, COMM_S, "NULL");
	snprintf(boot->os_ver, COMM_S, "NULL");
	snprintf(boot->boot_line, COMM_S, "NULL");
}

void
clean_boot_line(cbc_boot_line_s *boot)
{
	cbc_boot_line_s *list, *next;

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
init_build_struct(cbc_build_s *build)
{
	memset(build, 0, sizeof(cbc_build_s));
	snprintf(build->mac_addr, COMM_S, "NULL");
	snprintf(build->net_int, COMM_S, "NULL");
}

void
clean_build_struct(cbc_build_s *build)
{
	cbc_build_s *list, *next;

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
init_build_domain(cbc_build_domain_s *dom)
{
	memset(dom, 0, sizeof(cbc_build_domain_s));
	snprintf(dom->domain, COMM_S, "NULL");
	snprintf(dom->ntp_server, COMM_S, "NULL");
	snprintf(dom->ldap_dn, COMM_S, "NULL");
	snprintf(dom->ldap_bind, COMM_S, "NULL");
	snprintf(dom->ldap_host, COMM_S, "NULL");
	snprintf(dom->ldap_server, COMM_S, "NULL");
	snprintf(dom->log_server, COMM_S, "NULL");
	snprintf(dom->nfs_domain, COMM_S, "NULL");
	snprintf(dom->smtp_server, COMM_S, "NULL");
	snprintf(dom->xymon_server, COMM_S, "NULL");
}

void
clean_build_domain(cbc_build_domain_s *dom)
{
	cbc_build_domain_s *list, *next;

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
display_build_domain(cbc_build_domain_s *bdom)
{
	char *ip;
	time_t create;
	uint32_t ip_addr;

	if (!(ip = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ip in display_build_domain");
	printf("Details for build domain %s\n", bdom->domain);
	ip_addr = htonl((uint32_t)bdom->start_ip);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	printf("Network configuration\n\tStart IP: %s\n", ip);
	ip_addr = htonl((uint32_t)bdom->end_ip);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	printf("\tEnd IP: %s\n", ip);
	ip_addr = htonl((uint32_t)bdom->netmask);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	printf("\tNetmask: %s\n", ip);
	ip_addr = htonl((uint32_t)bdom->gateway);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	printf("\tGateway: %s\n", ip);
	ip_addr = htonl((uint32_t)bdom->ns);
	inet_ntop(AF_INET, &ip_addr, ip, RANGE_S);
	printf("\tName server: %s\n", ip);
	if (bdom->config_ntp > 0)
		printf("NTP server: %s\n", bdom->ntp_server);
	else
		printf("No NTP configuration\n");
	if (bdom->config_ldap > 0) {
		printf("LDAP configuration:\n");
		printf("\tLDAP Server: %s\n", bdom->ldap_server);
		if (bdom->ldap_ssl > 0)
			printf("\tLDAP URL: ldaps://%s\n", bdom->ldap_server);
		else
			printf("\tLDAP URL: ldap://%s\n", bdom->ldap_server);
		printf("\tLDAP base DN: %s\n", bdom->ldap_dn);
		printf("\tLDAP bind DN: %s\n", bdom->ldap_bind);
	} else 
		printf("No LDAP configuration\n");
	if (bdom->config_log > 0)
		printf("Logging server: %s\n", bdom->log_server);
	else
		printf("No logging server configuration\n");
	if (bdom->config_email > 0)
		printf("SMTP relay server: %s\n", bdom->smtp_server);
	else
		printf("No SMTP relay configuration\n");
	if (bdom->config_xymon > 0)
		printf("Xymon monitoring server: %s\n", bdom->xymon_server);
	else
		printf("No xymon monitoring configuration\n");
	create = (time_t)bdom->ctime;
	printf("Build domain created by %s on %s", get_uname(bdom->cuser), ctime(&create));
	create = (time_t)bdom->mtime;
	printf("Build domain updated by %s at %s", get_uname(bdom->muser), ctime(&create));
	free(ip);
}

void
init_build_ip(cbc_build_ip_s *ip)
{
	memset(ip, 0, sizeof(cbc_build_ip_s));
	snprintf(ip->host, COMM_S, "NULL");
	snprintf(ip->domain, COMM_S, "NULL");
}

void
clean_build_ip(cbc_build_ip_s *ip)
{
	cbc_build_ip_s *list, *next;

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
init_build_os(cbc_build_os_s *os)
{
	memset(os, 0, sizeof(cbc_build_os_s));
	snprintf(os->os, COMM_S, "NULL");
	snprintf(os->version, COMM_S, "NULL");
	snprintf(os->alias, COMM_S, "NULL");
	snprintf(os->ver_alias, COMM_S, "NULL");
	snprintf(os->arch, COMM_S, "NULL");
}

void
clean_build_os(cbc_build_os_s *os)
{
	cbc_build_os_s *list, *next;

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
init_build_type(cbc_build_type_s *type)
{
	memset(type, 0, sizeof(cbc_build_type_s));
	snprintf(type->alias, COMM_S, "NULL");
	snprintf(type->build_type, COMM_S, "NULL");
	snprintf(type->arg, COMM_S, "NULL");
	snprintf(type->url, COMM_S, "NULL");
	snprintf(type->mirror, COMM_S, "NULL");
}

void
clean_build_type(cbc_build_type_s *type)
{
	cbc_build_type_s *list, *next;

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
init_disk_dev(cbc_disk_dev_s *disk)
{
	memset(disk, 0, sizeof(cbc_disk_dev_s));
	snprintf(disk->device, COMM_S, "NULL");
}

void
clean_disk_dev(cbc_disk_dev_s *disk)
{
	cbc_disk_dev_s *list, *next;

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
init_locale(cbc_locale_s *locale)
{
	memset(locale, 0, sizeof(cbc_locale_s));
	snprintf(locale->locale, COMM_S, "NULL");
	snprintf(locale->country, COMM_S, "NULL");
	snprintf(locale->language, COMM_S, "NULL");
	snprintf(locale->keymap, COMM_S, "NULL");
	snprintf(locale->timezone, COMM_S, "NULL");
}

void
clean_locale(cbc_locale_s *locale)
{
	cbc_locale_s *list, *next;

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
init_package(cbc_package_s *pack)
{
	memset(pack, 0, sizeof(cbc_package_s));
	snprintf(pack->package, COMM_S, "NULL");
}

void
clean_package(cbc_package_s *pack)
{
	cbc_package_s *list, *next;

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
init_pre_part(cbc_pre_part_s *prep)
{
	memset(prep, 0, sizeof(cbc_pre_part_s));
	snprintf(prep->mount, COMM_S, "NULL");
	snprintf(prep->fs, COMM_S, "NULL");
	snprintf(prep->log_vol, COMM_S, "NULL");
}

void
clean_pre_part(cbc_pre_part_s *prep)
{
	cbc_pre_part_s *list, *next;

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
init_seed_scheme(cbc_seed_scheme_s *seed)
{
	memset(seed, 0, sizeof(cbc_seed_scheme_s));
	snprintf(seed->name, COMM_S, "NULL");
}

void
clean_seed_scheme(cbc_seed_scheme_s *seed)
{
	cbc_seed_scheme_s *list, *next;

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
init_cbc_server(cbc_server_s *server)
{
	memset(server, 0, sizeof(cbc_server_s));
	snprintf(server->vendor, COMM_S, "NULL");
	snprintf(server->make, COMM_S, "NULL");
	snprintf(server->model, COMM_S, "NULL");
	snprintf(server->uuid, COMM_S, "NULL");
	snprintf(server->name, COMM_S, "NULL");
}

void
clean_cbc_server(cbc_server_s *server)
{
	cbc_server_s *list, *next;

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
init_varient(cbc_varient_s *vari)
{
	memset(vari, 0, sizeof(cbc_varient_s));
	snprintf(vari->varient, COMM_S, "NULL");
	snprintf(vari->valias, COMM_S, "NULL");
}

void
clean_varient(cbc_varient_s *vari)
{
	cbc_varient_s *list, *next;

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
init_vm_hosts(cbc_vm_server_hosts_s *vm)
{
	memset(vm, 0, sizeof(cbc_vm_server_hosts_s));
	snprintf(vm->vm_server, COMM_S, "NULL");
	snprintf(vm->type, COMM_S, "NULL");
}

void
clean_vm_hosts(cbc_vm_server_hosts_s *vm)
{
	cbc_vm_server_hosts_s *list, *next;

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

void
init_cbc_dhcp(cbc_dhcp_s *dh)
{
	memset(dh, 0, sizeof(cbc_dhcp_s));
	if (!(dh->iname = malloc(RBUFF_S)))
		report_error(MALLOC_FAIL, "dh->iname");
	if (!(dh->dname = malloc(RBUFF_S)))
		report_error(MALLOC_FAIL, "dh->dname");
	memset(dh->iname, 0, RBUFF_S);
	memset(dh->dname, 0, RBUFF_S);
}

void
clean_cbc_dhcp(cbc_dhcp_s *dh)
{
	cbc_dhcp_s *list, *next;

	if (dh)
		list = dh;
	else
		return;
	next = list->next;
	while (list) {
		free(list->iname);
		free(list->dname);
		if (list->dom_search)
			clean_string_l(list->dom_search);
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_cbc_iface(cbc_iface_s *ifa)
{
	memset(ifa, 0, sizeof(cbc_iface_s));
	if (!(ifa->name = malloc(RBUFF_S)))
		report_error(MALLOC_FAIL, "ifa->name");
	memset(ifa->name, 0, RBUFF_S);
}

void
clean_cbc_iface(cbc_iface_s *ifa)
{
	cbc_iface_s *list, *next;

	if (ifa)
		list = ifa;
	else
		return;
	next = list->next;
	while (list) {
		free(list->name);
		free(list);
		list = next;
		if (next)
			next = next->next;
		else
			next = '\0';
	}
}

void
init_cbc_syspack(cbc_sys_pack_s *spack)
{
	memset(spack, 0, sizeof(cbc_sys_pack_s));
}

void
clean_cbc_syspack(cbc_sys_pack_s *spack)
{
	cbc_sys_pack_s *list, *next;
	if (spack)
		list = spack;
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
init_cbc_syspack_conf(cbc_sys_pack_conf_s *spack)
{
	memset(spack, 0, sizeof(cbc_sys_pack_conf_s));
}

void
clean_cbc_syspack_conf(cbc_sys_pack_conf_s *spack)
{
	cbc_sys_pack_conf_s *list, *next;
	if (spack)
		list = spack;
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
init_cbc_syspack_arg(cbc_sys_pack_arg_s *spack)
{
	memset(spack, 0, sizeof(cbc_sys_pack_arg_s));
}

void
clean_cbc_syspack_arg(cbc_sys_pack_arg_s *spack)
{
	cbc_sys_pack_arg_s *list, *next;
	if (spack)
		list = spack;
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

