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
#include <libxml/parser.h>
#include <libxml/tree.h>

static int
ailsa_connect_libvirt(virConnectPtr *conn, const char *uri);

static int
ailsa_create_storage_xml(ailsa_mkvm_s *vm);

int
mkvm_create_vm(ailsa_mkvm_s *vm)
{
	int retval = 0;
	virConnectPtr conn;
	virStoragePoolPtr pool = NULL;

	if ((retval = ailsa_connect_libvirt(&conn, (const char *)vm->uri)) != 0)
		return retval;
	if (!(pool = virStoragePoolLookupByName(conn, vm->pool))) {
		fprintf(stderr, "Cannot find pool %s\n", vm->pool);
		retval = -1;
		goto cleanup;
/*	} else {
		printf("Found pool %s\n", vm->pool);
		goto cleanup; */
	}
	if ((retval = ailsa_create_storage_xml(vm)) != 0) {
		printf("Unable to create XML to define storage\n");
		retval = -1;
		goto cleanup;
	}
	
	cleanup:
		if (pool)
			virStoragePoolFree(pool);
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
ailsa_create_storage_xml(ailsa_mkvm_s *vm)
{
	int retval = 0;
	xmlDocPtr doc = NULL;
	xmlNodePtr root, node, next;
	xmlDtdPtr dptr = NULL;

	root = node = next = NULL;
	if (!(vm))
		return AILSA_NO_DATA;

	return retval;
}

