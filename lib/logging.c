/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  logging.c
 *
 *  Functions for logging for cmdbd and cmdbcd
 *
 */
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <ailsacmdb.h>
#include "config.h"

void
ailsa_start_syslog(const char *prog)
{
	int fac = LOG_DAEMON;
	int opt = LOG_PID;

	openlog(prog, opt, fac);
}

void
ailsa_syslog(int priority, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	if (getppid() > 1) {
		vfprintf(stderr, msg, ap);
		fprintf(stderr, "\n");
	} else {
		vsyslog(priority, msg, ap);
	}
	va_end(ap);
		
}

void
ailsa_show_error(int retval)
{
	switch(retval) {
	case AILSA_NO_ACTION:
		fprintf(stderr, "No action was specified on the command line\n\n");
		break;
	case AILSA_NO_CONNECT:
		fprintf(stderr, "Program was unable to make connection to libvirtd\n\n");
		break;
	default:
		return;
	}
}

void
display_version(char *prog)
{
        if (strrchr(prog, '/')) {
                prog = strrchr(prog, '/');
                prog++;
        }
        printf("%s: %s\n", prog, VERSION);
}

