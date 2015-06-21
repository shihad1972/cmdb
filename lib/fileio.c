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
 *  fileio.c
 *
 *  Contains file / directory functions for the ailsacmdb library
 *
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ailsacmdb.h>

int
check_directory_access(const char *dir, int create)
{
	int retval;
	struct stat st;

	if ((retval = stat(dir, &st)) != 0) {
		if (errno == ENOENT && create) {
			mode_t um = umask(0);
			mode_t mask = S_IRWXU | S_IRWXG | S_ISGID;
			if ((retval = mkdir(dir, mask)) != 0)
				syslog(LOG_ALERT, "Cannot create dir %s: %s", dir, strerror(errno));
			mask = umask(um);
		}
	} else {
		if (!S_ISDIR(st.st_mode))
			retval = -1;
	}
	return retval;
}

int
ailsa_write_file(const char *name, void *data, size_t len)
{
	int retval, fd, cfd, flags;
	ssize_t size, tmp;
	mode_t um, mask;

	um = umask(0);
	mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
	flags = O_CREAT | O_WRONLY;
	fd = 0;
	retval = 0;
	if ((fd = open(name, flags, mask)) < 0) {
		syslog(LOG_ALERT, "Cannot open file %s for writing: %s", name, strerror(errno));
		retval = -1;
		goto cleanup;
	}
	if ((size = write(fd, data, len)) < 0) {
		syslog(LOG_ALERT, "Cannot write to file %s: %s", name, strerror(errno));
		retval = -1;
		goto cleanup;
	}
	while (size != (ssize_t)len) {	// keep trying until all data written
		char *ptr;
		ptr = data;
		if ((tmp = write(fd, ptr + size, len - (size_t)size)) < 0) {
			syslog(LOG_ALERT, "Cannot write to file %s: %s", name, strerror(errno));
			retval = -1;
			goto cleanup;
		}
		size += tmp;
	}

	cleanup:
		if (fd > 0)
			if ((cfd = close(fd)) != 0)
				syslog(LOG_ALERT, "Error closing file %s: %s", name, strerror(errno));
		mask = umask(um);
		return retval;
}

int
ailsa_read_file(const char *name, void *data, size_t len)
{
	int retval, fd, cfd, flags;
	ssize_t size, tmp;

	fd = 0;
	retval = 0;
	flags = O_RDONLY;
	if ((fd = open(name, flags)) < 0) {
		syslog(LOG_ALERT, "Cannot open file %s for reading: %s", name, strerror(errno));
		retval = -1;
		goto cleanup;
	}
	if ((size = read(fd, data, len)) < 0) {
		syslog(LOG_ALERT, "Cannot read from file %s: %s", name, strerror(errno));
		retval = -1;
		goto cleanup;
	}
	while (size != (ssize_t)len) {	// keep trying to read from the file
		char *ptr;
		ptr = data;
		if ((tmp = read(fd, ptr + size, len - (size_t)size)) < 0) {
			syslog(LOG_ALERT, "Cannot read from file %s: %s", name, strerror(errno));
			retval = -1;
			goto cleanup;
		}
		size += tmp;
	}

	cleanup:
		if (fd > 0)
			if ((cfd = close(fd)) != 0)
				syslog(LOG_ALERT, "Error closing file %s: %s", name, strerror(errno));
		return retval;
}

int
ailsa_append_file(const char *name, void *data, size_t len)
{
	int retval, fd, cfd, flags;
	ssize_t size, tmp;

	flags = O_APPEND | O_WRONLY;
	fd = 0;
	retval = 0;
	if ((fd = open(name, flags)) < 0) {
		syslog(LOG_ALERT, "Cannot open file %s for appending: %s", name, strerror(errno));
		retval = -1;
		goto cleanup;
	}
	if ((size = write(fd, data, len)) < 0) {
		syslog(LOG_ALERT, "Cannot write to file %s: %s", name, strerror(errno));
		retval = -1;
		goto cleanup;
	}
	while (size != (ssize_t)len) {	// keep trying until all data written
		char *ptr;
		ptr = data;
		if ((tmp = write(fd, ptr + size, len - (size_t)size)) < 0) {
			syslog(LOG_ALERT, "Cannot write to file %s: %s", name, strerror(errno));
			retval = -1;
			goto cleanup;
		}
		size += tmp;
	}

	cleanup:
		if (fd > 0)
			if ((cfd = close(fd)) != 0)
				syslog(LOG_ALERT, "Error closing file %s: %s", name, strerror(errno));
		return retval;
}

