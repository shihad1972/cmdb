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
#include <string.h>
#include <ailsacmdb.h>
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

#ifndef DEBUG
static void
ailsa_custom_libvirt_err(void *data, virErrorPtr err)
{
	if (!(data) || !(err))
		return;
}
#endif

int
mkvm_create_vm(ailsa_mkvm_s *vm)
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

	if (!(vm))
		return -1;
	ram = vm->ram * 1024;
	uuid = ailsa_gen_uuid_str();
	memset(mac, 0, MAC_LEN);
	if ((retval = ailsa_gen_mac(mac, AILSA_KVM)) != 0)
		goto cleanup;
	if (!(vm->mac = strndup(mac, MAC_LEN)))
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
	int retval =  0;
	size_t len;
	AILSS *select = ailsa_calloc(sizeof(AILSS), "select in mkvm_add_to_cmdb");
	AILDBV *fields = ailsa_calloc(sizeof(AILDBV), "fields in mkvm_add_to_cmdb");
	AILDBV *args = ailsa_calloc(sizeof(AILDBV), "fields in mkvm_add_to_cmdb");
	AILLIST *results = ailsa_calloc(sizeof(AILLIST), "results in mkvm_add_to_cmdb");

	if (!(cmdb) || !(vm))
		return AILSA_NO_DATA;
	ailsa_list_init(results, ailsa_clean_data);
	if ((retval = ailsa_init_ss(select)) != 0) {
		fprintf(stderr, "Cannot initialise AILSS select\n");
		goto cleanup;
	}
	select->query = strndup("SELECT server_id FROM server WHERE name = ?", CONFIG_LEN);
	fields->name = strdup("server_id");
	fields->type = AILSA_DB_LINT;
	if ((retval = ailsa_list_ins_next(select->fields, NULL, fields)) != 0) {
		fprintf(stderr, "Cannot insert fields into list in mkvm_add_to_cmdb\n");
		goto cleanup;
	}
	if ((len = strlen(vm->name)) >= CONFIG_LEN)
		fprintf(stderr, "Name of vm trundated to 255 characters!\n");
	args->name = strndup(vm->name, CONFIG_LEN);
	args->type = AILSA_DB_TEXT;
	if ((retval = ailsa_list_ins_next(select->args, NULL, args)) != 0) {
		fprintf(stderr, "Cannot insert args into list in mkvm_add_to_cmdb\n");
		goto cleanup;
	}
	if ((retval = ailsa_simple_select(cmdb, select, results)) < 0) {
		fprintf(stderr, "Cannot get server_id from database in mkvm_add_to_cmdb\n");
		goto cleanup;
	} else if (retval > 0) {
		fprintf(stderr, "VM %s exists in database\n", vm->name);
		goto cleanup;
	}
	cleanup:
		ailsa_clean_ss(select);
		if (results) {
			ailsa_list_destroy(results);
			my_free(results);
		}
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
