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
 *  networking.c
 *
 *  Functions for networking for cmdbd and cmdbcd
 *
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <ailsacmdb.h>

// Temporary

enum {
	COMM_S = 8,
	RANGE_S = 16,
	HOST_S = 64,
	RBUFF_S = 256,
	TBUFF_S = 512,
	BUFF_S = 1024
};

/*
static int
ailsa_tcp_socket(const char *node, const char *service); */

static char *
print_sock_addr(const struct sockaddr *addr);

static int
ailsa_do_client_checkin(int s, char *line, struct cmdb_client_config *c);

static int
ailsa_do_client_hostname(int s, char *line, struct cmdb_client_config *c);

static int
ailsa_do_client_finishup(int s, char *line);

static const int MAXPENDING = 5; // Max outstanding connect requests

int
ailsa_tcp_socket_bind(const char *node, const char *service)
{
	struct addrinfo acrit;
	memset(&acrit, 0, sizeof(acrit));
	acrit.ai_family = AF_UNSPEC;
	acrit.ai_flags = AI_PASSIVE;
	acrit.ai_socktype = SOCK_STREAM;

	struct addrinfo *saddr;
	int retval = getaddrinfo(node, service, &acrit, &saddr);
	if (retval != 0) {
		syslog(LOG_ALERT, "1: Cannot get address: %s", gai_strerror(retval));
		return -1;
	}

	int ssock = -1;
	for (struct addrinfo *addr = saddr; addr != NULL; addr = addr->ai_next) {
		ssock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (ssock < 0)
			continue;	// Socket creation failure; try next address
		if ((bind(ssock, addr->ai_addr, addr->ai_addrlen) == 0) && 
		    (listen(ssock, MAXPENDING) == 0)) {
			struct sockaddr laddr;
			socklen_t asize = sizeof(laddr);
			if (getsockname(ssock, (struct sockaddr *) &laddr, &asize) < 0) {
				syslog(LOG_ALERT, "getsockname() failure: %s", strerror(errno));
				return -1;
			}
			syslog(LOG_NOTICE, "Binding to address %s", print_sock_addr((struct sockaddr *)&laddr));
			break; // bind() successful
		}
		close(ssock);
		ssock = -1;
		syslog(LOG_WARNING, "Cannot bind to %s:%s", node, service);
	}
	freeaddrinfo(saddr);
	return ssock;
}

int
ailsa_tcp_socket(const char *node, const char *service)
{
	struct addrinfo crit;
	memset(&crit, 0, sizeof(crit));
	crit.ai_family = AF_UNSPEC;
	crit.ai_socktype = SOCK_STREAM;
	crit.ai_protocol = IPPROTO_TCP;

	struct addrinfo *saddr;
	int retval = getaddrinfo(node, service, &crit, &saddr);
	if (retval != 0) {
		ailsa_syslog(LOG_ALERT, "getaddrinfo() failure: %s. Does cmdb have an entry in /etc/services?", gai_strerror(retval));
		return -1;
	}
	int sock = -1;
	for (struct addrinfo *addr = saddr; addr != NULL; addr = addr->ai_next) {
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sock < 0)
			continue; // Try next one..
		if ((retval = connect(sock, addr->ai_addr, addr->ai_addrlen)) == 0)
			break; // Got one!
		ailsa_syslog(LOG_ALERT, "connect() failure: %s.", strerror(errno));
		close(sock); // conect failed, try next one
		sock = -1;
	}
	freeaddrinfo(saddr);
	return sock;
}

int
ailsa_do_client_send(int s, struct cmdb_client_config *c)
{
	char line[BUFF_S], *p;
	int retval;
	ssize_t len;

	memset(&line, 0, BUFF_S);
	p = line;
	if ((len = read(s, p, BUFF_S)) < 0) {
		if ((retval = ailsa_handle_recv_error(errno)) < 0)
			goto cleanup;
		// need signal handler here
	}
// Here is where we should check for the protocol version of the server
	memset(&line, 0, BUFF_S);
	if ((retval = ailsa_do_client_checkin(s, line, c)) != 0)
		goto cleanup;
	if ((retval = ailsa_do_client_hostname(s, line, c)) != 0)
		goto cleanup;
	if ((retval = ailsa_do_client_finishup(s, line)) != 0)
		goto cleanup;
	cleanup:
		close(s);
		return retval;
}

int
ailsa_get_fqdn(char *host, char *fqdn, char *ip)
{
	if (!(host) || !(fqdn) || !(ip)) {
		syslog(LOG_ALERT, "Null pointer passed to ailsa_get_fqdn");
		return -1;
	}
	int status, retval;
	void *addr;
	struct addrinfo hints, *res, *p;
	socklen_t len = sizeof(struct sockaddr_in6);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
		syslog(LOG_ALERT, "3: getaddrinfo failure: %s", gai_strerror(status));
		return -1;
	}
	p = res;
	status = 0;
	while (p) {
		if (p->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			if (!(inet_ntop(AF_INET, addr, ip, INET6_ADDRSTRLEN))) {
				syslog(LOG_ALERT, "inet_ntop error: %s", strerror(errno));
			} else {
				status = 1;
				if ((retval = getnameinfo(p->ai_addr, len, fqdn, RBUFF_S, NULL, 0, NI_NAMEREQD)) != 0) {
					syslog(LOG_ALERT, "getnameinfo error: %s", gai_strerror(retval));
					retval = -1;
				}
			}
		} else if (p->ai_family == AF_INET6) {
			struct sockaddr_in *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin_addr);
			if (!(inet_ntop(AF_INET6, addr, ip, INET6_ADDRSTRLEN))) {
				syslog(LOG_ALERT, "inet_ntop error: %s", strerror(errno));
			} else {
				status = 1;
				if ((retval = getnameinfo(p->ai_addr, len, fqdn, RBUFF_S, NULL, 0, NI_NAMEREQD)) != 0) {
					syslog(LOG_ALERT, "getnameinfo error: %s", gai_strerror(retval));
					retval = -1;
				}
			}
		}
		p = p->ai_next;
	}
	if (status == 0) {
		syslog(LOG_ALERT, "Unable to get IP address for name %s", host);
		retval = -1;
	}
	return retval;
}

static char *
print_sock_addr(const struct sockaddr *addr)
{
	const void *num_addr;
	char addr_buff[INET6_ADDRSTRLEN], *retval;
	retval = addr_buff;
	switch (addr->sa_family) {
	case AF_INET:
		num_addr = &((const struct sockaddr_in *) addr)->sin_addr;
		break;
	case AF_INET6:
		num_addr = &((const struct sockaddr_in6 *) addr)->sin6_addr;
		break;
	default:
		syslog(LOG_WARNING, "Unkown AF_FAMILY type");
		return NULL;
	}
	if (inet_ntop(addr->sa_family, num_addr, addr_buff, sizeof(addr_buff)) == NULL) {
		syslog(LOG_WARNING, "Invalid IP address");
	} else {
		return retval;
	}
	return NULL;
}

bool
sock_addrs_eq(const struct sockaddr *addr1, const struct sockaddr *addr2)
{
	if (addr1 == NULL || addr2 == NULL)
		return addr1 == addr2;
	else if (addr1->sa_family != addr2->sa_family)
		return false;
	else if (addr1->sa_family == AF_INET) {
		const struct sockaddr_in *ipv4Addr1 = (const struct sockaddr_in *) addr1;
		const struct sockaddr_in *ipv4Addr2 = (const struct sockaddr_in *) addr2;
		return ipv4Addr1->sin_addr.s_addr == ipv4Addr2->sin_addr.s_addr
			&& ipv4Addr1->sin_port == ipv4Addr2->sin_port;
	} else if (addr1->sa_family == AF_INET6) {
		const struct sockaddr_in6 *ipv6Addr1 = (const struct sockaddr_in6 *) addr1;
		const struct sockaddr_in6 *ipv6Addr2 = (const struct sockaddr_in6 *) addr2;
		return memcmp(&ipv6Addr1->sin6_addr, &ipv6Addr2->sin6_addr,
			sizeof(struct in6_addr)) == 0 && ipv6Addr1->sin6_port
			== ipv6Addr2->sin6_port;
	} else
		return false;
}

int
ailsa_accept_tcp_connection(int ssock)
{
	struct sockaddr_storage caddr; // Client address
	socklen_t clen = sizeof(caddr);

	int csock = accept(ssock, (struct sockaddr *) &caddr, &clen);
	if (csock < 0) {
		syslog(LOG_WARNING, "accept() failure: %s", strerror(errno));
		return -1;
	}
	syslog(LOG_NOTICE, "Accepting client %s", print_sock_addr((struct sockaddr *)&caddr));
	return csock;
}

int
ailsa_send_response(int c, char *b)
{
	int retval = 0;
	ssize_t slen;
	memset(b, 0, TBUFF_S);
	snprintf(b, COMM_S, "OK\r\n");
	size_t len = strlen(b);
	if ((slen = send(c, b, len, 0)) < 0)
                retval = ailsa_handle_send_error(errno);
	return retval;
}

int
ailsa_do_close(int client, char *buf)
{
	int retval = 0;
	ssize_t slen;
	memset(buf, 0, TBUFF_S);
	if ((shutdown(client, SHUT_RD)) < 0)
		syslog(LOG_INFO, "Shutdown on socket failed: %s", strerror(errno));
	snprintf(buf, RANGE_S, "SEEYA\r\n");
	size_t len = strlen(buf);
	if ((slen = send(client, buf, len, 0)) < 0)
                retval = ailsa_handle_send_error(errno);
	return retval;
}

int
ailsa_handle_send_error(int error)
{
	int retval = -1;

	switch (error) {
	case ECONNRESET: case EFAULT: case EINVAL: case EBADF: case ENOTSOCK:
	case ENOTCONN: case EOPNOTSUPP: case EDESTADDRREQ: case EMSGSIZE:
	case ENOBUFS: case ENOMEM: case EPIPE:
		syslog(LOG_ALERT, "Sending failed: %s", strerror(error));
		break;
	default:
		retval = 0;
		break;
	}

	return retval;
}

int
ailsa_handle_recv_error(int error)
{
	int retval = -1;

	switch (error) {
	case ECONNRESET: case EFAULT: case EINVAL: case EBADF: case ENOTSOCK:
	case ENOTCONN: case EOPNOTSUPP: case EDESTADDRREQ: case EMSGSIZE:
	case ENOBUFS: case ENOMEM: case EPIPE:
		syslog(LOG_ALERT, "Receiving failed: %s", strerror(error));
		break;
	default:
		retval = 0;
		break;
	}

	return retval;
}

static int
ailsa_do_client_checkin(int s, char *line, struct cmdb_client_config *c)
{
	if (!(line))
		return -1;
	int retval = 0;
	ssize_t size;
	size_t len;

	sprintf(line, "CHECKIN: %s\r\n", c->uuid);
	len = strlen(line);
	if ((size = send(s, line, len, 0)) < 0)
		retval = ailsa_handle_send_error(errno);
	memset(line, 0, len);
	if ((size = recv(s, line, BUFF_S, 0)) < 0)
		retval = ailsa_handle_recv_error(errno);
// Should probably check the response is OK
	return retval;
}

static int
ailsa_do_client_hostname(int s, char *line, struct cmdb_client_config *c)
{
	int retval;
	ssize_t size;
	size_t len;

	sprintf(line, "HOST: %s\r\n", c->fqdn);
	len = strlen(line);
	if ((size = send(s, line, len, 0)) < 0)
		retval = ailsa_handle_send_error(errno);
	memset(line, 0, len);
	if ((size = recv(s, line, BUFF_S, 0)) < 0)
		retval = ailsa_handle_recv_error(errno);
// Should probably check the response is OK
	return retval;
}

static int
ailsa_do_client_finishup(int s, char *line)
{
	int retval;
	ssize_t size;
	size_t len;

	sprintf(line, "CLOSE: \r\n");
	len = strlen(line);
	if ((size = send(s, line, len, 0)) < 0)
		retval = ailsa_handle_send_error(errno);
	memset(line, 0, len);
	if ((size = recv(s, line, BUFF_S, 0)) < 0)
		retval = ailsa_handle_recv_error(errno);
// Should probably check the response is OK
	return retval;
}

