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
 *  cmdbd.c
 *
 *  Contains main() function for cmdbd program
 *
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>
#include <errno.h>
#include <libgen.h>
#include <sys/time.h>
#include <ailsacmdb.h>
#include "cmdb.h"

static void
parse_command_line(struct cmdbc_config *cm, int argc, char *argv[]);

int
main(int argc, char *argv[])
{
	int retval = 0;
	int s, err, wk;
	struct cmdbc_config *cm = ailsa_malloc(sizeof(struct cmdbc_config), "cm in main");

	ailsa_start_syslog(basename(argv[0]));
/*	if (daemon(0, 0) < 0) {
		syslog(LOG_ALERT, "Failed to daemonise: %s", strerror(errno));
		exit(1);
	} else {
		syslog(LOG_INFO, "Starting cmdb...");
	} */
	if (argc > 1)
		parse_command_line(cm, argc, argv);
	if (!(cm->service))
		s = ailsa_tcp_socket_bind(cm->host, "cmdb");
	else
		s = ailsa_tcp_socket_bind(cm->host, cm->service);
	if (s < 0)
		exit(1);
	else
		wk = s + 1;
	while(1) {	// run forever
		fd_set sset;
		struct timeval tmo;
		tmo.tv_sec = 2;
		tmo.tv_usec = 0;
		FD_ZERO(&sset);
		FD_SET(s, &sset);
		if ((retval = select(wk, &sset, NULL, NULL, &tmo) < 0)) {
			err = errno;
			if (err != EINTR) {
				syslog(LOG_ALERT, "Select() error: %s", strerror(err));
				break;
			}
			// Should handle the signal interrupt here.
			continue;
		}
		if (FD_ISSET(s, &sset)) {
			if (ailsa_accept_client(s) == 0)
				retval = 0;
		}
		// Can do other tasks here.
	}
	return retval;
}

static void
parse_command_line(struct cmdbc_config *cm, int argc, char *argv[])
{
	int c;

	static const struct option longopts[] = {
		{ "host", required_argument,	0, 'h' },
		{ "service", required_argument,	0, 's' },
		{ NULL, 0, 0, 0 }
	};
	while ((c = getopt_long(argc, argv, "h:s:", longopts, NULL)) != -1) {
		switch (c) {
		case 'h':
			cm->host = strndup(optarg, RBUFF_S);
			break;
		case 's':
			cm->service = strndup(optarg, RANGE_S);
			break;
		default:
			syslog(LOG_WARNING, "Unknown option %c", c);
		}
	}
}

