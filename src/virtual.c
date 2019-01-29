/* 
 *
 *  mkvm: Make Virtual Machine
 *  Copyright (C) 2018  Iain M Conochie <iain-AT-thargoid.co.uk> 
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

#ifndef DEBUG
	virSetErrorFunc(NULL, ailsa_custom_libvirt_err);
#endif
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
			printf("Unable to create XML to define storage\n");
			retval = -1;
			goto cleanup;
		}
	}
	if (!(net = virNetworkLookupByName(conn, vm->network))) {
		printf("Network %s not found\n", vm->network);
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

	if (!(vm))
		return AILSA_NO_DATA;

	return retval;
}

