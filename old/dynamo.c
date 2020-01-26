/* 
 *
 *  dynamo:
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
 *  dynamo.c
 *
 *  Contains main function for dynamo program
 */

#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>

int
main(int argc, char *argv[])
{
	virConnectPtr conn;
	const char *uri = "qemu:///system";
	int retval;

	conn  = virConnectOpen(uri);
	if (!(conn)) {
		fprintf(stderr, "Connection to %s failed\n", uri);
		return 1;
	}
	retval = virConnectClose(conn);
	return retval;
}
