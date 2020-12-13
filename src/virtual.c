/* 
 *
 *  mkvm: Make Virtual Machine
 *  Copyright (C) 2018 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  virtual.c
 *
 *  Contains functions to manipulate Virtual machines
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

// Storage data structures and functions

typedef struct ailsa_virt_stor_s {
	virStoragePoolPtr pool;
	virStorageVolPtr vol;
} ailsa_virt_stor_s;


// End of Storage

static int
mkvm_add_to_cmdb(ailsa_cmdb_s *cms, ailsa_mkvm_s *vm);

static int
ailsa_connect_libvirt(virConnectPtr *conn, const char *uri);

static int
ailsa_create_volume_xml(ailsa_mkvm_s *vm);

static int
ailsa_create_domain_xml(ailsa_mkvm_s *vm, ailsa_string_s *dom);

static int
ailsa_get_vol(virStorageVolPtr vol, ailsa_mkvm_s *vm);

static int
ailsa_get_vol_type(virStorageVolInfoPtr info, ailsa_mkvm_s *vm);

static int
ailsa_create_storage_pool_xml(ailsa_mkvm_s *vm, ailsa_string_s *dom);

static int
mkvm_fill_server_list(ailsa_cmdb_s *cmdb, ailsa_mkvm_s *vm, AILLIST *server);

static int
cmdb_add_hardware_to_new_vm(ailsa_cmdb_s *cmdb, ailsa_mkvm_s *vm);

#ifndef DEBUG
static void
ailsa_custom_libvirt_err(void *data, virErrorPtr err)
{
	if (!(data) || !(err))
		return;
}
#endif

int
mkvm_create_vm(ailsa_cmdb_s *cms, ailsa_mkvm_s *vm)
{
	int retval = 0;
	virConnectPtr conn;
	virStoragePoolPtr pool = NULL;
	virStorageVolPtr vol = NULL;
	virNetworkPtr net = NULL;
	virDomainPtr dom = NULL;
	ailsa_string_s *domain = NULL;

#ifndef DEBUG
	virSetErrorFunc(NULL, ailsa_custom_libvirt_err);
#endif
	domain = ailsa_calloc(sizeof(ailsa_string_s), "domain in mkvm_create_mv");
	ailsa_init_string(domain);
	if ((retval = ailsa_connect_libvirt(&conn, (const char *)vm->uri)) != 0)
		return retval;
	if ((dom = virDomainLookupByName(conn, (const char *)vm->name))) {
// If the domain is _inactive_ it will not be returned here.
		fprintf(stderr, "Domain %s already exists!\n", vm->name);
		goto cleanup;
	}
	if (!(pool = virStoragePoolLookupByName(conn, vm->pool))) {
		fprintf(stderr, "Cannot find pool %s\n", vm->pool);
		retval = -1;
		goto cleanup;
	}
	if (!(vol = virStorageVolLookupByName(pool, vm->name))) {
		if ((retval = ailsa_create_volume_xml(vm)) != 0) {
			fprintf(stderr, "Unable to create XML to define storage\n");
			retval = -1;
			goto cleanup;
		}
		if (!(vol = virStorageVolCreateXML(pool, (const char *)vm->storxml, 0))) {
			fprintf(stderr, "Unable to create storage volume %s\n", vm->name);
			retval = -1;
			goto cleanup;
		}
	}
	if ((retval = ailsa_get_vol(vol, vm)) != 0) {
		fprintf(stderr, "Unable to get vol info for %s\n", vm->name);
		retval = -1;
		goto cleanup;
	}
	if (!(net = virNetworkLookupByName(conn, vm->network))) {
		fprintf(stderr, "Network %s not found\n", vm->network);
		retval = -1;
		goto cleanup;
	}
	if ((retval = ailsa_create_domain_xml(vm, domain)) != 0) {
		fprintf(stderr, "Unable to create XML document for domain\n");
		retval = -1;
		goto cleanup;
	}
	if (!(dom = virDomainDefineXML(conn, domain->string))) {
		fprintf(stderr, "Unable to create domain %s!\n", vm->name);
		retval = -1;
		goto cleanup;
	}
/* If we put this inside cleanup, then we will always add to cmdb. This is
   probably what we want, as even if the domain fails, we want to try to add
   to cmdb. mkvm_add_to_cmdb will need to be idempotent so if will not add
   the server and hardware twice. */
	if (vm->cmdb > 0)
		retval = mkvm_add_to_cmdb(cms, vm);
	cleanup:
		if (pool)
			virStoragePoolFree(pool);
		if (vol)
			virStorageVolFree(vol);
		if (net)
			virNetworkFree(net);
		if (dom)
			virDomainFree(dom);
		if (domain)
			ailsa_clean_string(domain);
		virConnectClose(conn);
		return retval;
}

static int
ailsa_connect_libvirt(virConnectPtr *conn, const char *uri)
{
	int retval = 0;

	if (!(conn))
		return AILSA_NO_DATA;
	if (!(uri))
		uri = "qemu:///system";
	if (!(*conn = virConnectOpen(uri)))
		return AILSA_NO_CONNECT;
	return retval;
}

static int
ailsa_create_volume_xml(ailsa_mkvm_s *vm)
{
	int retval = 0;
	unsigned long int capacity = 0;

	if (!(vm))
		return AILSA_NO_DATA;
	vm->storxml = ailsa_calloc(FILE_LEN, "vm->xmlstor in ailsa_create_volume_xml");
	capacity = vm->size * 1024 * 1024 * 1024;
	sprintf(vm->storxml, "\
<volume>\n\
  <name>%s</name>\n\
  <capacity unit='bytes'>%lu</capacity>\n\
  <allocation unit='bytes'>%lu</allocation>\n\
</volume>\n", vm->name, capacity, capacity);
	return retval;
}

static int
ailsa_create_domain_xml(ailsa_mkvm_s *vm, ailsa_string_s *dom)
{
	int retval = 0;
	char *uuid = NULL;
	char buf[FILE_LEN];
	char mac[MAC_LEN];
	unsigned long int ram = 0;

	if (!(vm) || !(dom))
		return -1;
	ram = vm->ram * 1024;
	uuid = ailsa_gen_uuid_str();
	memset(mac, 0, MAC_LEN);
	if ((retval = ailsa_gen_mac(mac, AILSA_KVM)) != 0)
		goto cleanup;
	if (!(vm->mac = strndup(mac, MAC_LEN)))
		goto cleanup;
	if (!(vm->uuid = strndup(uuid, UUID_LEN)))
		goto cleanup;
	snprintf(buf, FILE_LEN, "\
<domain type='kvm'>\n\
  <name>%s</name>\n\
  <uuid>%s</uuid>\n\
  <memory unit='KiB'>%lu</memory>\n\
  <currentMemory unit='KiB'>%lu</currentMemory>\n\
  <vcpu placement='static'>%lu</vcpu>\n\
  <os>\n\
    <type arch='x86_64' machine='pc-i440fx-2.1'>hvm</type>\n\
  </os>\n\
  <features>\n\
    <acpi/>\n\
    <apic/>\n\
    <pae/>\n\
  </features>\n\
  <cpu mode='custom' match='exact'>\n\
    <model fallback='allow'>Westmere</model>\n\
  </cpu>\n\
  <clock offset='utc'>\n\
    <timer name='rtc' tickpolicy='catchup'/>\n\
    <timer name='pit' tickpolicy='delay'/>\n\
    <timer name='hpet' present='no'/>\n\
  </clock>\n\
  <on_poweroff>destroy</on_poweroff>\n\
  <on_reboot>restart</on_reboot>\n\
  <on_crash>restart</on_crash>\n\
  <pm>\n\
    <suspend-to-mem enabled='no'/>\n\
    <suspend-to-disk enabled='no'/>\n\
  </pm>\n\
", vm->name, uuid, ram, ram, vm->cpus);
	ailsa_fill_string(dom, buf);
	memset(buf, 0, FILE_LEN);
	snprintf(buf, FILE_LEN, "\
  <devices>\n\
    <emulator>/usr/bin/kvm</emulator>\n\
    <disk type='%s' device='disk'>\n\
      <driver name='qemu' type='raw'/>\n\
      <source %s='%s'/>\n\
      <target dev='vda' bus='virtio'/>\n\
      <boot order='2'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x07' function='0x0'/>\n\
    </disk>\n\
", vm->vt, vm->vtstr, vm->path);
	ailsa_fill_string(dom, buf);
	memset(buf, 0, FILE_LEN);
	snprintf(buf, FILE_LEN, "\
    <controller type='usb' index='0' model='ich9-ehci1'>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x05' function='0x7'/>\n\
    </controller>\n\
    <controller type='usb' index='0' model='ich9-uhci1'>\n\
      <master startport='0'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x05' function='0x0' multifunction='on'/>\n\
    </controller>\n\
    <controller type='usb' index='0' model='ich9-uhci2'>\n\
      <master startport='2'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x05' function='0x1'/>\n\
    </controller>\n\
    <controller type='usb' index='0' model='ich9-uhci3'>\n\
      <master startport='4'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x05' function='0x2'/>\n\
    </controller>\n\
    <controller type='pci' index='0' model='pci-root'/>\n\
    <controller type='virtio-serial' index='0'>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x06' function='0x0'/>\n\
    </controller>\n\
");
	ailsa_fill_string(dom, buf);
	memset(buf, 0, FILE_LEN);
	if (vm->netdev)
		snprintf(buf, FILE_LEN, "\
    <interface type='bridge'>\n\
      <mac address='%s'/>\n\
      <source bridge='%s'/>\n\
      <model type='virtio'/>\n\
      <boot order='1'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x03' function='0x0'/>\n\
", mac, vm->netdev);
	else
		snprintf(buf, FILE_LEN, "\
    <interface type='network'>\n\
      <mac address='%s'/>\n\
      <source network='%s'/>\n\
      <model type='virtio'/>\n\
      <boot order='1'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x03' function='0x0'/>\n\
    </interface>\n\
", mac, vm->network);
	ailsa_fill_string(dom, buf);
	memset(buf, 0, FILE_LEN);
	snprintf(buf, FILE_LEN, "\
    <serial type='pty'>\n\
      <source path='/dev/pts/1'/>\n\
      <target port='0'/>\n\
      <alias name='serial0'/>\n\
    </serial>\n\
    <console type='pty' tty='/dev/pts/1'>\n\
      <source path='/dev/pts/1'/>\n\
      <target type='serial' port='0'/>\n\
      <alias name='serial0'/>\n\
    </console>\n\
    <channel type='spicevmc'>\n\
      <target type='virtio' name='com.redhat.spice.0'/>\n\
      <alias name='channel0'/>\n\
      <address type='virtio-serial' controller='0' bus='0' port='1'/>\n\
    </channel>\n\
    <input type='mouse' bus='ps2'/>\n\
    <input type='keyboard' bus='ps2'/>\n\
    <graphics type='spice' port='5901' autoport='yes' listen='127.0.0.1'>\n\
      <listen type='address' address='127.0.0.1'/>\n\
    </graphics>\n\
");
	ailsa_fill_string(dom, buf);
	memset(buf, 0, FILE_LEN);
	snprintf(buf, FILE_LEN, "\
    <video>\n\
      <model type='qxl' ram='65536' vram='65536' heads='1'/>\n\
      <alias name='video0'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x02' function='0x0'/>\n\
    </video>\n\
    <redirdev bus='usb' type='spicevmc'>\n\
      <alias name='redir0'/>\n\
    </redirdev>\n\
    <redirdev bus='usb' type='spicevmc'>\n\
      <alias name='redir1'/>\n\
    </redirdev>\n\
    <redirdev bus='usb' type='spicevmc'>\n\
      <alias name='redir2'/>\n\
    </redirdev>\n\
    <redirdev bus='usb' type='spicevmc'>\n\
      <alias name='redir3'/>\n\
    </redirdev>\n\
    <memballoon model='virtio'>\n\
      <alias name='balloon0'/>\n\
      <address type='pci' domain='0x0000' bus='0x00' slot='0x08' function='0x0'/>\n\
    </memballoon>\n\
  </devices>\n\
</domain>\n\
");
	ailsa_fill_string(dom, buf);
	memset(buf, 0, FILE_LEN);

	cleanup:
		if (uuid)
			my_free(uuid);
		return retval;
}

static int
ailsa_get_vol(virStorageVolPtr uvol, ailsa_mkvm_s *vm)
{
	int retval = 0;
	if (!(uvol) || !(vm))
		return AILSA_NO_DATA;
	virStorageVolPtr vol = uvol;
	virStorageVolInfoPtr vol_info = NULL;

	vol_info = ailsa_calloc(sizeof(virStorageVolInfo), "vol_info in ailsa_get_vol");
	if ((retval = virStorageVolGetInfo(vol, vol_info)) != 0) {
		fprintf(stderr, "Unable to get vol info for %s\n", vm->name);
		goto cleanup;
	}
	if ((retval = ailsa_get_vol_type(vol_info, vm)) != 0) {
		fprintf(stderr, "Unable to get the volume type for volume %s\n", vm->name);
		retval = -1;
		goto cleanup;
	}
	if (!(vm->path = virStorageVolGetPath(vol))) {
		fprintf(stderr, "Unable to get the volume path for volume %s\n", vm->name);
		retval = -1;
		goto cleanup;
	}
	cleanup:
		if (vol_info)
			my_free(vol_info);
		return retval;
}

static int
ailsa_get_vol_type(virStorageVolInfoPtr info, ailsa_mkvm_s *vm)
{
	int retval = 0;
	char *str = ailsa_calloc(MAC_LEN, "str in ailsa_get_vol_type");
	char *vt = ailsa_calloc(MAC_LEN, "vt in ailsa_get_vol_type");

	if (info->type == VIR_STORAGE_VOL_FILE) {
		snprintf(str, MAC_LEN, "file");
		snprintf(vt, MAC_LEN, "file");
	} else if (info->type == VIR_STORAGE_VOL_BLOCK) {
		snprintf(str, MAC_LEN, "dev");
		snprintf(vt, MAC_LEN, "block");
	} else {
		my_free(str);
		my_free(vt);
		fprintf(stderr, "Unknown vol type\n");
		retval = -1;
	}
	vm->vt = vt;
	vm->vtstr = str;
	return retval;
}

int
mkvm_add_to_cmdb(ailsa_cmdb_s *cmdb, ailsa_mkvm_s *vm)
{
	if (!(cmdb) || !(vm))
		return AILSA_NO_DATA;
	int retval =  0;
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *res = ailsa_db_data_list_init();

	if ((retval = cmdb_add_string_to_list(vm->name, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot add VM name to list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmdb, SERVER_ID_ON_NAME, server, res)) != 0) {
		ailsa_syslog(LOG_ERR, "SERVER_ID_ON_NAME query failed");
		goto cleanup;
	}
	if (res->total > 0) {
		ailsa_syslog(LOG_INFO, "Server %s exists in database", vm->name);
		goto cleanup;
	}
	if ((retval = mkvm_fill_server_list(cmdb, vm, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill server list for database insert");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cmdb, INSERT_SERVER, server)) != 0) {
		ailsa_syslog(LOG_ERR, "INSERT_SERVER query failed");
		goto cleanup;
	}
	if ((retval = cmdb_add_hardware_to_new_vm(cmdb, vm)) != 0)
		goto cleanup;
	cleanup:
		ailsa_list_full_clean(server);
		ailsa_list_full_clean(res);
		return retval;
}

int
mksp_create_storage_pool(ailsa_mkvm_s *sp)
{
	int retval = 0;
	virConnectPtr conn;
	ailsa_string_s *domain = NULL;
	virStoragePoolPtr pool = NULL;

	if (!(sp))
		return AILSA_NO_DATA;
#ifndef DEBUG
	virSetErrorFunc(NULL, ailsa_custom_libvirt_err);
#endif // DEBUG
	domain = ailsa_calloc(sizeof(ailsa_string_s), "domain in mkvm_create_storage_pool");
	ailsa_init_string(domain);
	if ((retval = ailsa_connect_libvirt(&conn, (const char *)sp->uri)) != 0)
		return retval;
	if (!(pool = virStoragePoolLookupByName(conn, sp->name))) {
		if ((retval = ailsa_create_storage_pool_xml(sp, domain)) != 0)
			goto cleanup;
		if (!(pool = virStoragePoolDefineXML(conn, domain->string, 0))) {
			fprintf(stderr, "Unable to define storage pool %s\n", sp->name);
			retval = AILSA_NO_POOL;
			goto cleanup;
		}
		if ((retval = virStoragePoolCreate(pool, 0)) != 0) {
			fprintf(stderr, "Unable to start storage pool %s\n", sp->name);
			retval = AILSA_NO_POOL;
			goto cleanup;
		}
		if ((retval = virStoragePoolSetAutostart(pool, 1)) != 0) {
			fprintf(stderr, "Unable to set pool %s to autostart\n", sp->name);
			goto cleanup;
			retval = AILSA_NO_POOL;
		}
	// If the pool is already defined, we want to make it active and autostart it
	} else {
		if ((retval = virStoragePoolIsActive(pool)) == 0) {
			if ((retval = virStoragePoolCreate(pool, 0)) != 0) {
				fprintf(stderr, "Unable to activate storage pool %s\n", sp->name);
				retval = AILSA_NO_POOL;
				goto cleanup;
			}
		}
		if ((retval = virStoragePoolSetAutostart(pool, 1)) != 0) {
			fprintf(stderr, "Unable to set pool %s to autostart\n", sp->name);
			goto cleanup;
			retval = AILSA_NO_POOL;
		}
		
	}
	cleanup:
		if (pool)
			virStoragePoolFree(pool);
		if (domain)
			ailsa_clean_string(domain);
		return retval;
}

static int
ailsa_create_storage_pool_xml(ailsa_mkvm_s *vm, ailsa_string_s *dom)
{
	int retval = 0;
	char buf[FILE_LEN];
	char type[MAC_LEN];

	if (!(vm) || !(dom))
		return AILSA_NO_DATA;
	if (vm->sptype == AILSA_LOGVOL)
		sprintf(type, "logical");
	else if (vm->sptype == AILSA_DIRECTORY)
		sprintf(type, "dir");
	else
		goto cleanup;
	snprintf(buf, FILE_LEN, "\
<pool type='%s'>\n\
  <name>%s</name>\n\
  <source>\n", type, vm->name);
	ailsa_fill_string(dom, buf);
	memset(buf, 0, FILE_LEN);
	if (vm->sptype == AILSA_LOGVOL) {
		sprintf(buf, "\
    <name>%s</name>\n\
    <format type='lvm2'/>\n\
", vm->logvol);
		ailsa_fill_string(dom, buf);
		memset(buf, 0, FILE_LEN);
	}
	memset(type, 0, MAC_LEN);
	if (vm->sptype == AILSA_LOGVOL)
		sprintf(type, "/dev/%s", vm->logvol);
	else if (vm->sptype == AILSA_DIRECTORY)
		sprintf(type, "%s", vm->path);
	else
		return AILSA_NO_TYPE;
	snprintf(buf, FILE_LEN, "\
  </source>\n\
  <target>\n\
    <path>%s</path>\n\
  </target>\n\
</pool>\n", type);
	ailsa_fill_string(dom, buf);

	cleanup:
		return retval;
}

static int
mkvm_fill_server_list(ailsa_cmdb_s *cmdb, ailsa_mkvm_s *vm, AILLIST *server)
{
	if (!(cmdb) || !(vm) || !(server))
		return AILSA_NO_DATA;
	int retval;
	char *vm_server = ailsa_calloc(CONFIG_LEN, "vm_server in mkvm_fill_server_list");

	if ((retval = cmdb_add_string_to_list("Virtual Machine", server)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_string_to_list("x86_64", server)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_string_to_list("KVM", server)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_string_to_list(vm->uuid, server)) != 0)
		goto cleanup;
	if ((retval = cmdb_check_add_cust_id_to_list(vm->coid, cmdb, server)) != 0) 
		goto cleanup;
	if (server->total != 6) {
		ailsa_syslog(LOG_ERR, "Cannot get customer ID from coid %s", vm->coid);
		goto cleanup;
	}
	if ((retval = gethostname(vm_server, CONFIG_LEN)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get hostname: %s", strerror(errno));
		goto cleanup;
	}
	if ((retval = cmdb_add_vm_server_id_to_list(vm_server, cmdb, server)) != 0)
		goto cleanup;
	if (server->total != 7) {
		if ((retval = cmdb_add_number_to_list(0, server)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add 0 vm_server_id to list");
			goto cleanup;
		}
	}
	if ((retval = cmdb_populate_cuser_muser(server)) != 0)
		goto cleanup;
	if (server->total != 9) {
		retval = -1;
		ailsa_syslog(LOG_ERR, "Wrong number of elements in server list: %zu", server->total);
		goto cleanup;
	}
	cleanup:
		my_free(vm_server);
		return retval;
}

static int
cmdb_add_hardware_to_new_vm(ailsa_cmdb_s *cmdb, ailsa_mkvm_s *vm)
{
	if (!(cmdb) || !(vm))
		return AILSA_NO_DATA;
	int retval;
	char buff[MAC_LEN];
	unsigned long int server_id;
	AILLIST *l = ailsa_db_data_list_init();

	if ((retval = cmdb_check_add_server_id_to_list(vm->name, cmdb, l)) != 0)
		goto cleanup;
	server_id = ((ailsa_data_s *)l->head->data)->data->number;
	sprintf(buff, "Network Card");
	if ((retval = cmdb_add_hard_type_id_to_list(buff, cmdb, l)) != 0)
		goto cleanup;
	memset(buff, 0, MAC_LEN);
	if ((retval = cmdb_add_string_to_list(vm->mac, l)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_string_to_list("eth0", l)) != 0)
		goto cleanup;
	if ((retval = cmdb_populate_cuser_muser(l)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_number_to_list(server_id, l)) != 0)
		goto cleanup;
	sprintf(buff, "Hard Disk");
	if ((retval = cmdb_add_hard_type_id_to_list(buff, cmdb, l)) != 0)
		goto cleanup;
	memset(buff, 0, MAC_LEN);
	if ((snprintf(buff, MAC_LEN, "%lu GB", vm->size)) >= MAC_LEN)
		ailsa_syslog(LOG_ERR, "disk size buff truncated!");
	if ((retval = cmdb_add_string_to_list(buff, l)) != 0)
		goto cleanup;
	memset(buff, 0, MAC_LEN);
	sprintf(buff, "vda");
	if ((retval = cmdb_add_string_to_list(buff, l)) != 0)
		goto cleanup;
	if ((retval = cmdb_populate_cuser_muser(l)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_number_to_list(server_id, l)) != 0)
		goto cleanup;
	sprintf(buff, "Virtual CPU");
	if ((retval = cmdb_add_hard_type_id_to_list(buff, cmdb, l)) != 0)
		goto cleanup;
	memset(buff, 0, MAC_LEN);
	if ((snprintf(buff, MAC_LEN, "%lu vCPU", vm->cpus)) >= MAC_LEN)
		ailsa_syslog(LOG_ERR, "cpu buff truncated!");
	if ((retval = cmdb_add_string_to_list(buff, l)) != 0)
		goto cleanup;
	memset(buff, 0, MAC_LEN);
	if ((retval = cmdb_add_string_to_list("cpu", l)) != 0)
		goto cleanup;
	if ((retval = cmdb_populate_cuser_muser(l)) != 0)
		goto cleanup;
	if ((retval = cmdb_add_number_to_list(server_id, l)) != 0)
		goto cleanup;
	sprintf(buff, "Virtual RAM");
	if ((retval = cmdb_add_hard_type_id_to_list(buff, cmdb, l)) != 0)
		goto cleanup;
	memset(buff, 0, MAC_LEN);
	if ((snprintf(buff, MAC_LEN, "%lu RAM", vm->ram)) >= MAC_LEN)
		ailsa_syslog(LOG_ERR, "cpu buff truncated!");
	if ((retval = cmdb_add_string_to_list(buff, l)) != 0)
		goto cleanup;
	memset(buff, 0, MAC_LEN);
	if ((retval = cmdb_add_string_to_list("ram", l)) != 0)
		goto cleanup;
	if ((retval = cmdb_populate_cuser_muser(l)) != 0)
		goto cleanup;
	if ((l->total % 6) != 0) {
		ailsa_syslog(LOG_ERR, "Wrong number in list? %lu", l->total);
		goto cleanup;
	}
	if ((retval = ailsa_multiple_query(cmdb, insert_queries[INSERT_HARDWARE], l)) != 0)
		ailsa_syslog(LOG_ERR, "INSERT_HARDWARE query failed");

	cleanup:
		ailsa_list_full_clean(l);
		return retval;
}

int
ailsa_list_networks(ailsa_mkvm_s *vm)
{
        if (!(vm))
                return AILSA_NO_DATA;
        int retval, i;
        virConnectPtr conn;
	virNetworkPtr *ptr = NULL;
	virNetworkPtr *n;
	unsigned int flags = VIR_CONNECT_LIST_NETWORKS_ACTIVE | VIR_CONNECT_LIST_NETWORKS_PERSISTENT | VIR_CONNECT_LIST_NETWORKS_AUTOSTART;

	if ((retval = ailsa_connect_libvirt(&conn, vm->uri)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot connect to libvirt URL %s", vm->uri);
		return retval;
	}
	if ((retval = virConnectListAllNetworks(conn, &ptr, flags)) == -1) {
		ailsa_syslog(LOG_ERR, "Cannot list networks: %s", virGetLastErrorMessage());
		goto cleanup;
	}
	n = ptr;
	for (i = 0; i< retval; i++) {
		printf("%s\n", virNetworkGetName(*n));
		virNetworkFree(*n);
		n++;
	}
	cleanup:
		virConnectClose(conn);
	        return retval;
}

int
ailsa_add_network(ailsa_cmdb_s *cbs, ailsa_mkvm_s *vm)
{
	if (!(cbs) || !(vm))
		return AILSA_NO_DATA;
	int retval, counter, flag;
	char iface_name[SERVICE_LEN];
	AILLIST *ice = ailsa_iface_list_init();
	AILELEM *element = NULL;
	ailsa_iface_s *iface = NULL;

	if ((retval = ailsa_get_iface_list(ice)) != 0)
		goto cleanup;
	for (counter = 0; counter < BUFFER_LEN; counter++) {
		flag = false;
		memset(iface_name, 0, SERVICE_LEN);
		snprintf(iface_name, SERVICE_LEN, "virbr%d", counter);
		element = ice->head;
		while (element) {
			iface = element->data;
			if ((strncmp(iface_name, iface->name, SERVICE_LEN)) == 0) {
				flag = true;
				break;
			}
			element = element->next;
		}
		if (flag == false)
			break;
	}
	printf("Using interface %s\n", iface_name);
	cleanup:
		ailsa_list_full_clean(ice);
		return retval;
}
