/* 
 *
 *  cmdb : Configuration Management Database
 *  Copyright (C) 2016 - 2020 Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  cmdb-identity.c
 *
 *  Contains functions for cmdb-identity program
 */
#define _DEFAULT_SOURCE
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <syslog.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <ailsasql.h>
#define PROGRAM "cmdb-identity"

typedef struct cmdb_identity_comm_line_s { /* Hold parsed command line args for cmdb-identity */
	char *hash;
	char *pass;
        char *server;
	char *user;
	short int action;
        short int ask_pass;
} cmdb_identity_comm_line_s;

static int
parse_command_line(int argc, char *argv[], cmdb_identity_comm_line_s *cml);

static int
check_command_line(cmdb_identity_comm_line_s *cml);

static int
validate_command_line(cmdb_identity_comm_line_s *cml);

static void
display_usage();

static void
clean_cmdb_identity_comm_line(cmdb_identity_comm_line_s *comm);

static int
add_identity(ailsa_cmdb_s *cc, cmdb_identity_comm_line_s *cml);

static int
list_identity(ailsa_cmdb_s *cc, cmdb_identity_comm_line_s *cml);

static void
display_identity(AILELEM *e);

int
main(int argc, char *argv[])
{
        int retval = 0;
        ailsa_start_syslog(basename(argv[0]));
        cmdb_identity_comm_line_s *cml = ailsa_calloc(sizeof(cmdb_identity_comm_line_s), "cml in main");
        ailsa_cmdb_s *cc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cc in main");

        if ((retval = parse_command_line(argc, argv, cml)) != 0) {
                display_usage();
                goto cleanup;
        }
        parse_cmdb_config(cc);
        switch(cml->action) {
        case CMDB_ADD:
                retval = add_identity(cc, cml);
                break;
        case CMDB_LIST:
                retval = list_identity(cc, cml);
                break;
        default:
                display_usage();
                break;
        }
        cleanup:
                clean_cmdb_identity_comm_line(cml);
                ailsa_clean_cmdb(cc);
                return retval;
}

static int
parse_command_line(int argc, char *argv[], cmdb_identity_comm_line_s *cml)
{
        int retval = 0;
        int opt;
        const char *optstr = "alrs:u:p";
#ifdef HAVE_GETOPT_H
	int index;
        struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
                {"list",                no_argument,            NULL,   'l'},
                {"remove",              no_argument,            NULL,   'r'},
                {"delete",              no_argument,            NULL,   'r'},
                {"server",              required_argument,      NULL,   's'},
                {"user",                required_argument,      NULL,   'u'},
                {"password",            no_argument,            NULL,   'p'},
                {NULL, 0, NULL, 0}
	};
        while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
        {
		if (opt == 'a')
			cml->action = CMDB_ADD;
                else if (opt == 'l')
                        cml->action = CMDB_LIST;
                else if (opt == 'r')
                        cml->action = CMDB_RM;
                else if (opt == 's')
                        cml->server = strndup(optarg, HOST_LEN);
                else if (opt == 'u')
                        cml->user = strndup(optarg, CONFIG_LEN);
                else if (opt == 'p')
                        cml->ask_pass = true;
                else
			retval = CMDB_USAGE;
        }
        if ((retval = check_command_line(cml)) != 0)
                return retval;
        if ((retval = validate_command_line(cml)) != 0)
                return retval;
        return retval;
}

static int
check_command_line(cmdb_identity_comm_line_s *cml)
{
        if (cml->action == 0)
                return AILSA_NO_ACTION;
        if ((cml->action == CMDB_ADD) || (cml->action == CMDB_RM)) {
                if (!(cml->server))
                        return AILSA_NO_NAME;
                else if (!(cml->user))
                        return AILSA_NO_USER;
        }
        return 0;
}

static int
validate_command_line(cmdb_identity_comm_line_s *cml)
{
        if (!(cml))
                return AILSA_NO_DATA;
        if (cml->user)
                if (ailsa_validate_input(cml->user, DEV_REGEX) != 0)
                        return AILSA_INPUT_INVALID;
        if (cml->server)
                if (ailsa_validate_input(cml->server, NAME_REGEX) != 0)
                        return AILSA_INPUT_INVALID;
        return 0;
}

static void
clean_cmdb_identity_comm_line(cmdb_identity_comm_line_s *comm)
{
	if (!(comm))
		return;
	if (comm->hash)
		my_free(comm->hash);
	if (comm->user)
		my_free(comm->user);
	if (comm->pass)
		my_free(comm->pass);
        if (comm->server)
                my_free(comm->server);
	my_free(comm);
}

static void
display_usage()
{
        ailsa_syslog(LOG_ERR, "Usage: %s ( -a | -l | -r ) [ -p ] [ -u user ] [ -s server ]", PROGRAM);
}

static int
add_identity(ailsa_cmdb_s *cc, cmdb_identity_comm_line_s *cml)
{
        if (!(cc) || !(cml))
                return AILSA_NO_DATA;
        AILLIST *id = ailsa_db_data_list_init();
        AILLIST *check = ailsa_db_data_list_init();
        size_t len = 25;
        size_t salt_len = 12;
        char *salt = ailsa_calloc(salt_len + 4, "salt in add_identity");
        char **ident = ailsa_calloc((sizeof(char *) * 2), "ident in add_identity");
        int retval;

        ident[0] = cml->server;
        ident[1] = cml->user;
        if (cml->ask_pass) {
                cml->pass = getpass("Input password: ");
        } else {
                cml->pass = ailsa_calloc(len + 1, "cml->pass in add_identity");
                random_string(cml->pass, len);
        }
        sprintf(salt, "$6$");
        random_string(salt + 3, salt_len - 3);
        sprintf((salt + salt_len - 2), "$");
        cml->hash = crypt(cml->pass, salt);
        printf("Salt: %s\nPass: %s\nHash: %s\n", salt, cml->pass, cml->hash);
        if ((retval = cmdb_check_add_identity_id_to_list(ident, cc, check)) == CANNOT_FIND_IDENTITY_ID) {
                if ((retval = cmdb_check_add_server_id_to_list(cml->server, cc, id)) != 0)
                        goto cleanup;
                if ((retval = cmdb_add_string_to_list(cml->user, id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add user name to list");
                        goto cleanup;
                }
                if ((retval = cmdb_add_string_to_list(cml->pass, id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add password to list");
                        goto cleanup;
                }
                if ((retval = cmdb_add_string_to_list(cml->hash, id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add hash to list");
                        goto cleanup;
                }
                if ((retval = cmdb_populate_cuser_muser(id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
                        goto cleanup;
                }
                if ((retval = ailsa_insert_query(cc, INSERT_IDENTITY, id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add identity into database");
                        goto cleanup;
                }
        } else if (retval == 0) {
                if ((retval = cmdb_add_string_to_list(cml->pass, id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add password to list");
                        goto cleanup;
                }
                if ((retval = cmdb_add_string_to_list(cml->hash, id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add hash to list");
                        goto cleanup;
                }
                if ((retval = cmdb_add_number_to_list((unsigned long int)getuid(), id)) != 0) {
                        ailsa_syslog(LOG_ERR, "Cannot add muser to list");
                        goto cleanup;
                }
                if ((retval = cmdb_check_add_identity_id_to_list(ident, cc, id)) != 0)
                        goto cleanup;
                if ((retval = ailsa_update_query(cc, update_queries[UPDATE_IDENTITY], id)) != 0) {
                        ailsa_syslog(LOG_ERR, "UPDATE_IDENTITY query failed");
                        goto cleanup;
                }
        } else {
                ailsa_syslog(LOG_ERR, "identity check failed");
                goto cleanup;
        }
        cleanup:
                my_free(salt);
                my_free(ident);
                ailsa_list_full_clean(id);
                ailsa_list_full_clean(check);
                return retval;
}
static int
list_identity(ailsa_cmdb_s *cc, cmdb_identity_comm_line_s *cml)
{
        if (!(cc) || !(cml))
                return AILSA_NO_DATA;
        int retval;
        size_t len = 6;
        AILLIST *id = ailsa_db_data_list_init();
        AILELEM *ptr;
        ailsa_data_s *server, *user;

        if ((retval = ailsa_basic_query(cc, IDENTITIES, id)) != 0) {
                ailsa_syslog(LOG_ERR, "IDENTITIES query failed");
                goto cleanup;
        }
        ptr = id->head;
        if (ptr)
                printf("Server\t\tUser\tCreated by\tModified by\tCreation time\t\tModification Time\n");
        while (ptr) {
                server = ptr->data;
                user = ptr->next->data;
                if (cml->server) {
                        if (cml->user) {
                                if ((strncmp(cml->user, user->data->text, CONFIG_LEN) == 0) &&
                                    (strncmp(cml->server, server->data->text, HOST_LEN) == 0))
                                        display_identity(ptr);
                        } else {
                                if (strncmp(cml->server, server->data->text, HOST_LEN) == 0)
                                        display_identity(ptr);
                        }
                } else if (cml->user) {
                        if (strncmp(cml->user, user->data->text, CONFIG_LEN) == 0)
                                display_identity(ptr);
                } else {
                        display_identity(ptr);
                }
                ptr = ailsa_move_down_list(ptr, len);
        }
        cleanup:
                ailsa_list_full_clean(id);
                return retval;
}

static void
display_identity(AILELEM *e)
{
        ailsa_data_s *server, *user, *cuser, *muser, *ctime, *mtime;

        server = e->data;
        if (strlen(server->data->text) < 8)
                printf("%s\t\t", server->data->text);
        else
                printf("%s\t", server->data->text);
        user = e->next->data;
        printf("%s\t", user->data->text);
        cuser = e->next->next->data;
        printf("%s\t\t", cmdb_get_uname(cuser->data->number));
        muser = e->next->next->next->data;
        printf("%s\t\t", cmdb_get_uname(muser->data->number));
        ctime = e->next->next->next->next->data;
        printf("%s\t", ctime->data->text);
        mtime = e->next->next->next->next->next->data;
        printf("%s\n", mtime->data->text);
}