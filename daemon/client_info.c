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
 *  client_info.c
 *
 *  Contains functions for manipulating the client information and data.
 *
 */

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ailsacmdb.h>
#include "client_info.h"

// Temporary

enum {
	COMM_S = 8,
	RANGE_S = 16,
	HOST_S = 64,
	RBUFF_S = 256,
	TBUFF_S = 512,
	BUFF_S = 1024
};

static void
ailsa_handle_client(int client);

int
ailsa_accept_client(int sock)
{
	int c = ailsa_accept_tcp_connection(sock);
	if (c < 0)
		return c;
	static unsigned int cc = 0;	// child count
	pid_t proc_id = fork();
	if (proc_id < 0) {
		syslog(LOG_ALERT, "forking failed: %s", strerror(errno));
		return -1;
	} else if (proc_id == 0) {
		close(sock);	// Child does not need this socket
		ailsa_handle_client(c);
		exit(0);
	}
	syslog(LOG_INFO, "Spawned child process %d", proc_id);
	close(c);
	cc++;
	while (cc) {
		proc_id = waitpid(-1, NULL, WNOHANG);
		if (proc_id < 0) {
			syslog(LOG_ALERT, "waitpid() failure: %s", strerror(errno));
			exit(1);	// Should probably be more clever here 
		} else if (proc_id == 0) {
			break;
		} else {
			cc--;
		}
	}
	return 0;
}

static void
ailsa_handle_client(int client)
{
	char sbuf[BUFF_S];
	int retval, command;
	size_t len;
	ssize_t slen;
	struct client_info ci;

	memset(&ci, 0, sizeof(struct client_info));
	if ((retval = ailsa_init_client_info(&ci)) < 0)
		goto cleanup;
	snprintf(sbuf, BUFF_S, "AILCMDB: %s\r\n", AILSAVERSION);
	len = strlen(sbuf);
	if ((slen = send(client, sbuf, len, 0)) < 0) {
		if ((retval = ailsa_handle_send_error(errno)) != 0)
			goto cleanup;
	}
	while (1) {	// loop through getting client commands
		memset(&sbuf, 0, TBUFF_S + 1);
		// Could use select() here - write out data to disk etc if client is not communicating
		if ((slen = recv(client, sbuf, TBUFF_S, 0)) < 0) {
			if ((retval = ailsa_handle_recv_error(errno)) != 0)
				goto cleanup;
		} else {
/* 
 * Here we should retrieve the command the client has sent us, and deal
 * with it. Need to define the MAX length of a command (probably 15 chars
 * will be enough) with the entire command string a max of 64 chars.
 * As we are using \r\n this in effect is 62 characters. If we want to
 * use FQDN names this will not be enough!!
 * We should up this to 512 characters (TBUFF_S). A good size.
 */
			if ((command = get_command(sbuf)) == 0)
				goto cleanup;
		}
		switch (command) {
		case CHECKIN:
			if (!(ci.uuid = get_uuid(sbuf)))
				goto cleanup;
			if ((retval = ailsa_send_response(client, sbuf)) < 0)
				goto cleanup;
			break;
		case HOST:
			if (!(ci.hostname = get_host(sbuf)))
				goto cleanup;
			if ((retval = ailsa_send_response(client, sbuf)) < 0)
				goto cleanup;
			break;
		case DATA: case UPDATE:
			if ((retval = get_client_info(client, command, sbuf, &ci)) < 0)
				goto cleanup;
			break;
		case CLOSE:
			retval = ailsa_do_close(client, sbuf);
			goto cleanup;
			break;
		default:
			goto cleanup;
			break;
		}
	}
	cleanup:
		close(client);
		ailsa_clean_client_info(&ci);
		return;
}

int
get_command(char *buffer)
{
	if (strncasecmp("CHECKIN: ", buffer, 9) == 0)
		return CHECKIN;
	else if (strncasecmp("HOST: ", buffer, 6) == 0)
		return HOST;
	else if (strncasecmp("DATA: ", buffer, 6) == 0)
		return DATA;
	else if (strncasecmp("UPDATE: ", buffer, 8) == 0)
		return UPDATE;
	else if (strncasecmp("CLOSE: ", buffer, 7) == 0)
		return CLOSE;
	else
		return 0;
}

char *
get_uuid(char *buffer)
{
	char *uuid = NULL;
	char *ptr;
	int retval;
	size_t len;

	if ((len = strnlen(buffer, HOST_S)) == HOST_S)
		return uuid;
	if ((retval = strncmp(buffer, "CHECKIN: ", COMM_S + 1)) != 0)
		return uuid;
	if (!(ptr = strchr(buffer, ' ')))
		return uuid;
	ptr++;
	ailsa_munch(ptr);
	if ((len = strlen(ptr)) != 36)
		return uuid;
	// Should do a regex here - can base on checks.c|h
	uuid = strndup(ptr, 36);
	return uuid;
}

char *
get_host(char *buffer)
{
	char *host = NULL;
	char *ptr;
	int retval;
	size_t len;

	if ((len = strnlen(buffer, TBUFF_S)) == TBUFF_S)
		return host;
	if ((retval = strncmp(buffer, "HOST: ", 6)) != 0)
		return host;
	if (!(ptr = strchr(buffer, ' ')))
		return host;
	ptr++;
	ailsa_munch(ptr);
	if ((len = strlen(ptr)) > RBUFF_S)
		return host;
	// should do a regex here
	host = strndup(ptr, RBUFF_S);
	return host;
}

int
get_client_info(int client, int command, char *buf, struct client_info *ci)
{
	int retval = 0;

	if (!(buf) || !(ci)) {
		retval = AILSA_NO_DATA;
		goto cleanup;
	}
	cleanup:
		return retval;
}

