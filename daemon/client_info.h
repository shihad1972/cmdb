/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  client_info.h: Function prototypes for client data manipulation
 */

#ifndef __CLIENT_INFO_H_
# define __CLIENT_INFO_H_

int
ailsa_accept_client(int sock);

void
ailsa_handle_client(int client);

int
get_command(char *buffer);

char *
get_uuid(char *buffer);

char *
get_host(char *buffer);

int
get_client_info(int client, int command, char *buf, struct client_info *ci);

#endif // __CLIENT_INFO_H_
