/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcos.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcos program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include <config.h>
#include <configmake.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <ailsasql.h>

typedef struct cbcos_comm_line_s {
	char *alias;
	char *arch;
	char *os;
	char *ver_alias;
	char *version;
	short int action;
	short int force;
} cbcos_comm_line_s;

static int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col);

static void
list_cbc_build_os(ailsa_cmdb_s *cmc);

static int
check_for_build_os(ailsa_cbcos_s *cos, char *version, char *valias, char *arch);

static int
display_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static int
cbc_fill_os_details(char *name, AILLIST *list, AILLIST *dest);

static int
add_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static int
remove_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static int
cbcos_grab_boot_files(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col);

static void
cbcos_check_for_null_in_comm_line(cbcos_comm_line_s *col, int *test);

static void
cbcos_check_for_os(cbcos_comm_line_s *col, AILELEM *head, int *test);

static void
cbcos_clean_comm_line(cbcos_comm_line_s *cl);

static int
cmdb_fill_os_details(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col, AILLIST *os);

static int
cbcos_create_os_profile(ailsa_cmdb_s *cmc, AILLIST *os);

static int
cbcos_insert_os_id_user_into_list(AILLIST *pack, unsigned long int id);

static int
ailsa_insert_cuser_muser(AILLIST *list, AILELEM *elem);

static void
display_server_name_for_build_os_id(AILLIST *list);

static int
cbcos_set_default_os(ailsa_cmdb_s *cc, cbcos_comm_line_s *ccl);

int
main (int argc, char *argv[])
{
	int retval = 0;
	ailsa_cmdb_s *cmc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmc in main");
	cbcos_comm_line_s *cocl = ailsa_calloc(sizeof(cbcos_comm_line_s), "cocl in main");

	if ((retval = parse_cbcos_comm_line(argc, argv, cocl)) != 0) {
		cbcos_clean_comm_line(cocl);
		ailsa_clean_cmdb(cmc);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(cmc);
	if (cocl->action == CMDB_LIST)
		list_cbc_build_os(cmc);
	else if (cocl->action == CMDB_DISPLAY)
		retval = display_cbc_build_os(cmc, cocl);
	else if (cocl->action == CMDB_ADD)
		retval = add_cbc_build_os(cmc, cocl);
	else if (cocl->action == CMDB_RM)
		retval = remove_cbc_build_os(cmc, cocl);
	else if (cocl->action == DOWNLOAD)
		retval = cbcos_grab_boot_files(cmc, cocl);
	else if (cocl->action == CMDB_DEFAULT)
		retval = cbcos_set_default_os(cmc, cocl);
	else
		printf("Unknown action type\n");
	ailsa_clean_cmdb(cmc);
	cbcos_clean_comm_line(cocl);
	exit(retval);
}

static int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col)
{
	const char *optstr = "ade:fghln:o:rs:t:vz";
	int opt;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"display",		no_argument,		NULL,	'd'},
		{"version-alias",	required_argument,	NULL,	'e'},
		{"force",		no_argument,		NULL,	'f'},
		{"download",		no_argument,		NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"list",		no_argument,		NULL,	'l'},
		{"name",		required_argument,	NULL,	'n'},
		{"os",			required_argument,	NULL,	'n'},
		{"os-version",		required_argument,	NULL,	'o'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"alias",		required_argument,	NULL,	's'},
		{"os-alias",		required_argument,	NULL,	's'},
		{"architecture",	required_argument,	NULL,	't'},
		{"os-arch",		required_argument,	NULL,	't'},
		{"version",		no_argument,		NULL,	'v'},
		{"set-default",		no_argument,		NULL,	'z'},
		{NULL,			0,			NULL,	0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'a')
			col->action = CMDB_ADD;
		else if (opt == 'd')
			col->action = CMDB_DISPLAY;
		else if (opt == 'l')
			col->action = CMDB_LIST;
		else if (opt == 'r')
			col->action = CMDB_RM;
		else if (opt == 'v')
			col->action = AILSA_VERSION;
		else if (opt == 'g')
			col->action = DOWNLOAD;
		else if (opt == 'z')
			col->action = CMDB_DEFAULT;
		else if (opt == 'h')
			return AILSA_DISPLAY_USAGE;
		else if (opt == 'e')
			col->ver_alias = strndup(optarg, MAC_LEN);
		else if (opt == 'f')
			col->force = 1;
		else if (opt == 'n')
			col->os = strndup(optarg, MAC_LEN);
		else if (opt == 'o')
			col->version = strndup(optarg, MAC_LEN);
		else if (opt == 's')
			col->alias = strndup(optarg, MAC_LEN);
		else if (opt == 't')
			col->arch = strndup(optarg, SERVICE_LEN);
		else {
			printf("Unknown option: %c\n", opt);
			return AILSA_DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		return AILSA_DISPLAY_USAGE;
	if (col->action == AILSA_VERSION)
		return AILSA_VERSION;
	if (col->action == 0 && argc != 1) {
		printf("No action provided\n");
		return AILSA_NO_ACTION;
	}
	if (((col->action == CMDB_ADD) || (col->action == CMDB_DEFAULT) ||
	      (col->action == CMDB_RM)) && ((!(col->version)) || (!(col->os)) || (!(col->arch)))) {
			printf("Some details were not provided\n");
			return AILSA_DISPLAY_USAGE;
	}
	if ((col->action != CMDB_LIST && col->action != DOWNLOAD) && 
		!((col->os))) {
		printf("No OS name was provided\n");
		return AILSA_DISPLAY_USAGE;
	}
	return NONE;
}

static void
list_cbc_build_os(ailsa_cmdb_s *cmc)
{
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *name;
	ailsa_data_s *one;

	if (!(cmc))
		goto cleanup;
	if ((retval = ailsa_basic_query(cmc, BUILD_OS_NAME_TYPE, list)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	name = list->head;
	if (list->total > 0) {
		printf("Operating Systems\n");
	} else {
		ailsa_syslog(LOG_INFO, "No build operating systems were found");
		goto cleanup;
	}
	while (name) {
		one = (ailsa_data_s *)name->data;
		printf("%s\n", one->data->text);
		name = name->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
}

static int
check_for_build_os(ailsa_cbcos_s *cos, char *version, char *valias, char *arch)
{
	int retval = 0;
	if (!(version) && !(valias) && !(arch))
		return 1;
	if (arch) {
		if (version || valias) {
			if (version) {
				if (strncmp(cos->os_version, version, MAC_LEN) == 0)
					retval = 1;
				else
					retval = 0;
			} else if (valias) {
				if (strncmp(cos->ver_alias, valias, MAC_LEN) == 0)
					retval = 1;
				else
					retval = 0;
			}
			if (retval == 1) {
				if (strncmp(cos->arch, arch, SERVICE_LEN) == 0)
					retval = 1;
				else
					retval = 0;
			}
		} else if (strncmp(cos->arch, arch, SERVICE_LEN) == 0) {
			retval = 1;
		}
		return retval;
	} else {
		if (version) {
			if (strncmp(cos->os_version, version, MAC_LEN) == 0)
				retval = 1;
			else
				retval = 0;
		} else if (valias) {
			if (strncmp(cos->ver_alias, valias, MAC_LEN) == 0)
				retval = 1;
			else
				retval = 0;
		}
		return retval;
	}
	return retval;
}

static int
display_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	if (!(cmc) || !(col))
		return AILSA_NO_DATA;
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *os = ailsa_cbcos_list_init();
	AILELEM *e;
	ailsa_cbcos_s *cos;
	char *name = col->os;
	char *version = col->version;
	char *valias = col->ver_alias;
	char *arch = col->arch;
	char *uname;

	if ((retval = ailsa_basic_query(cmc, BUILD_OSES, list)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL query returned %d", retval );
		goto cleanup;
	}
	if ((retval = cbc_fill_os_details(name, list, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot fill cbcos struct");
		goto cleanup;
	}
	if (os->total > 0) {
		printf("Operating System %s\n", name);
		printf("Version\tVersion alias\tArchitecture\tCreated by\tCreation time\n");
		e = os->head;
		while (e) {
			cos = e->data;
			if (check_for_build_os(cos, version, valias, arch) == 0) {
				e = e->next;
				continue;
			}
			if (strncasecmp(cos->ver_alias, "none", BYTE_LEN) == 0) {
				printf("%s\tnone\t\t%s\t\t",
				     cos->os_version, cos->arch);
			} else {
				if (strlen(cos->ver_alias) < 8)
					printf("%s\t%s\t\t%s\t\t",
					     cos->os_version, cos->ver_alias, cos->arch);
				else
					printf("%s\t%s\t%s\t\t",
					cos->os_version, cos->ver_alias, cos->arch);
			}
			uname = cmdb_get_uname(cos->cuser);
			if (strlen(uname) < 8)
				printf("%s\t\t%s\n", uname, cos->ctime);
			else
				printf("%s\t%s\n", uname, cos->ctime);
			my_free(uname);
			e = e->next;
		}
	} else {
		printf("OS %s not found\n", name);
	}
	cleanup:
		ailsa_list_full_clean(list);
		ailsa_list_full_clean(os);
		return retval;
}
static int
cbc_fill_os_details(char *name, AILLIST *list, AILLIST *dest)
{
	if (!(name) || !(list) || !(dest))
		return AILSA_NO_DATA;
	int retval;
	size_t total = 7;
	AILELEM *elem;
	ailsa_data_s *data;
	ailsa_cbcos_s *cos;

	if (list->total == 0)
		return AILSA_NO_DATA;
	if ((list->total % total) != 0)
		return AILSA_NO_DATA;
	elem = list->head;
	while (elem) {
		data = elem->data;
		if (name) {
			if (strncasecmp(data->data->text, name, MAC_LEN) != 0) {
				elem = ailsa_move_down_list(elem, total);
				continue;
			}
		}
		cos = ailsa_calloc(sizeof(ailsa_cbcos_s), "cos in cbc_fill_os_details");
		cos->os = strndup(((ailsa_data_s *)elem->data)->data->text, MAC_LEN);
		elem = elem->next;
		cos->os_version = strndup(((ailsa_data_s *)elem->data)->data->text, SERVICE_LEN);
		elem = elem->next;
		cos->alias = strndup(((ailsa_data_s *)elem->data)->data->text, SERVICE_LEN);
		elem = elem->next;
		cos->arch = strndup(((ailsa_data_s *)elem->data)->data->text, SERVICE_LEN);
		elem = elem->next;
		cos->ver_alias = strndup(((ailsa_data_s *)elem->data)->data->text, SERVICE_LEN);
		elem = elem->next;
		cos->cuser = ((ailsa_data_s *)elem->data)->data->number;
		elem = elem->next;
		cos->ctime = strndup(((ailsa_data_s *)elem->data)->data->text, HOST_LEN);
		elem = elem->next;
		if ((retval = ailsa_list_insert(dest, cos)) != 0)
			return retval;
	}
	return 0;
}

static int
add_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	int retval;
	AILLIST *os = ailsa_db_data_list_init();

	if (!(col->ver_alias))
		col->ver_alias = strdup("none");
	if ((retval = cmdb_check_for_os(cmc, col->os, col->arch, col->version)) > 0) {
		retval = 0;
		ailsa_syslog(LOG_ERR, "Build OS already in database");
		goto cleanup;
	} else if (retval < 0) {
		goto cleanup;
	}

	if ((retval = cmdb_fill_os_details(cmc, col, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to fill in OS details");
		goto cleanup;
	}
	if ((retval = ailsa_insert_query(cmc, INSERT_BUILD_OS, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert os %s into database", col->os);
		goto cleanup;
	}
	if ((retval = cbcos_create_os_profile(cmc, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot create new OS build profile");
		goto cleanup;
	}
	if ((retval = cbc_get_boot_files(cmc, col->alias, col->version, col->arch, col->ver_alias)) != 0)
		ailsa_syslog(LOG_ERR, "Unable to download boot files\n");

	cleanup:
		ailsa_list_destroy(os);
		my_free(os);
		return retval;
}

static int
remove_cbc_build_os(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	if (!(cmc) || !(col))
		return AILSA_NO_DATA;
	char **args = ailsa_calloc((sizeof(char *) * 3), "args in remove_cbc_build_os");
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *build = ailsa_db_data_list_init();
	int retval;
	unsigned long int id;
	AILELEM *element;
	ailsa_data_s *data;

	args[0] = col->os;
	args[1] = col->version;
	args[2] = col->arch;
	if ((retval = cmdb_add_os_id_to_list(args, cmc, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get OS id");
		retval = AILSA_OS_NOT_FOUND;
		goto cleanup;
	}
	if (list->total == 0) {
		retval = AILSA_OS_NOT_FOUND;
		goto cleanup;
	}
	element = list->head;
	data = element->data;
	id = data->data->number;
	if ((retval = check_builds_for_os_id(cmc, id, build)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot check for server builds with this OS");
		goto cleanup;
	}
	if (build->total > 0) {
		display_server_name_for_build_os_id(build);
		if (col->force == 0) {
			ailsa_syslog(LOG_ERR, "Build OS in use. If you want to delete, use -f");
			retval = AILSA_BUILD_OS_IN_USE;
			goto cleanup;
		}
	}
	if ((retval = ailsa_delete_query(cmc, delete_queries[DELETE_BUILD_OS], list)) != 0) {
		ailsa_syslog(LOG_ERR, "DELETE_BUILD_OS query failed");
		goto cleanup;
	}
	cleanup:
		ailsa_list_destroy(list);
		ailsa_list_destroy(build);
		my_free(args);
		my_free(list);
		my_free(build);
		return retval;
}

static void
display_server_name_for_build_os_id(AILLIST *list)
{
	if (!(list))
		return;
	size_t i;
	AILELEM *element;
	ailsa_data_s *data;

	element = list->head;

	printf("The following servers are using the build os:\n\t");
	for (i = 0; i < list->total; i++) {
		if (!(element))
			break;
		data = element->data;
		printf(" %s", data->data->text);
		element = element->next;
	}
	printf("\n");
}

static int
cbcos_grab_boot_files(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col)
{
	int retval = 0;
	int test = 0;
	int count = 0;
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *os = ailsa_cbcos_list_init();
	AILELEM *elem = NULL;
	ailsa_cbcos_s *cos;

	cbcos_check_for_null_in_comm_line(col, &test);
	if ((retval = ailsa_basic_query(cmc, BUILD_OSES, list)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	if (col->os) {
		if ((retval = cbc_fill_os_details(col->os, list, os)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot fill cbcos details");
			goto cleanup;
		}
	} else if(col->alias) {
		if ((retval = cbc_fill_os_details(col->alias, list, os)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot fill cbcos details");
			goto cleanup;
		}
	} else {
		if ((retval = cbc_fill_os_details(NULL, list, os)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot fill cbcos details");
			goto cleanup;
		}
	}
	elem = os->head;
	if (os->total < 1)
		goto cleanup;
	while (elem) {
		cos = elem->data;
		if ((test & 7) == 7) {
			printf("Will download OS %s, version %s, arch %s\n", cos->os, cos->os_version, cos->arch);
			count++;
			if ((retval = cbc_get_boot_files(cmc, cos->alias, cos->os_version, cos->arch, cos->ver_alias)) != 0)
				ailsa_syslog(LOG_ERR, "Error downloading OS\n");
		} else {
			cbcos_check_for_os(col, elem, &test);
			if ((test & 56) == 56) {
				printf("Will download OS %s, version %s, arch %s\n", cos->os, cos->os_version, cos->arch);
				count ++;
				if ((retval = cbc_get_boot_files(cmc, cos->alias, cos->os_version, cos->arch, cos->ver_alias)) != 0)
					ailsa_syslog(LOG_ERR, "Error downloading OS\n");
			}
		}
		elem = elem->next;
		test = test & 7;
	}
	if (count == 0)
		ailsa_syslog(LOG_ERR, "No OS found to download\n");
	cleanup:
		ailsa_list_full_clean(list);
		ailsa_list_full_clean(os);
		return retval;
}

static void
cbcos_check_for_null_in_comm_line(cbcos_comm_line_s *col, int *test)
{
	*test = 0;	// Sanity
/* Set bit fields from command line input */
	if (!(col->arch))
		*test = *test | 1;
	if (!(col->version) && !(col->ver_alias))
		*test = *test | 2;
	if (!(col->os) && !(col->alias))
		*test = *test | 4;
}

static void
cbcos_check_for_os(cbcos_comm_line_s *col, AILELEM *head, int *test)
{
	if (!(col) || !(head) || !(test))
		return;
	ailsa_cbcos_s *os = head->data;

	if ((*test & 4) == 4) {		// check if os name / alias set on command line
		*test = *test | 32;
	} else if (col->os) {
		if (strncasecmp(col->os, os->os, MAC_LEN) == 0)
			*test = *test | 32;
	} else if (col->alias) {
		if (strncasecmp(col->alias, os->alias, MAC_LEN) == 0)
			*test = *test | 32;
	}
	if ((*test & 1) == 1)		// check if os arch set on command line
		*test = *test | 8;
	else if (strncasecmp(col->arch, os->arch, SERVICE_LEN) == 0)
		*test = *test | 8;
	if ((*test & 2) == 2) {		// check if os version set on command line
		*test = *test | 16;
	} else if (col->version) {
		if (strncasecmp(col->version, os->os_version, MAC_LEN) == 0)
			*test = *test | 16;
	} else if (col->ver_alias) {
		if (strncasecmp(col->ver_alias, os->ver_alias, MAC_LEN) == 0)
			*test = *test | 16;
	}
}

static void
cbcos_clean_comm_line(cbcos_comm_line_s *cl)
{
	if (!(cl))
		return;
	if (cl->alias)
		my_free(cl->alias);
	if (cl->arch)
		my_free(cl->arch);
	if (cl->os)
		my_free(cl->os);
	if (cl->ver_alias)
		my_free(cl->ver_alias);
	if (cl->version)
		my_free(cl->version);
	my_free(cl);
}

static int
cmdb_fill_os_details(ailsa_cmdb_s *cmc, cbcos_comm_line_s *col, AILLIST *os)
{
	int retval;
	if (!(cmc) || !(col) || !(os))
		return AILSA_NO_DATA;

	if ((retval = cmdb_add_string_to_list(col->os, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS name into list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(col->version, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS version into list");
		return retval;
	}
	if (col->alias) {
		if ((retval = cmdb_add_string_to_list(col->alias, os)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert OS alias into list");
			return retval;
		}
	} else {
		if ((retval = cmdb_add_os_alias_to_list(col->os, cmc, os)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add os alias to list");
			return retval;
		}
	}
	if ((retval = cmdb_add_string_to_list(col->ver_alias, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS version alias into list");
		return retval;
	}
	if ((retval = cmdb_add_string_to_list(col->arch, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert OS architecture into list");
		return retval;
	}
	if ((retval = cmdb_add_build_type_id_to_list(col->alias, cmc, os)) != 0)
		return AILSA_BUILD_TYPE_NO_FOUND;
	retval = cmdb_populate_cuser_muser(os);
	return retval;
}

static int
cbcos_create_os_profile(ailsa_cmdb_s *cmc, AILLIST *os)
{
	if (!(cmc) || !(os))
		return AILSA_NO_DATA;
	int retval = 0;
	unsigned long int new_os_id;
	void *ptr;
	if (os->total != 8)
		return AILSA_WRONG_LIST_LENGHT;
	AILLIST *pack = ailsa_db_data_list_init();
	AILLIST *list = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	AILELEM *text = os->head;
	text = text->next->next->next->next;
	AILELEM *id = text->next;
	ailsa_data_s *tmp = id->data;

	if ((retval = cmdb_add_number_to_list(tmp->data->number, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert bt_id into query list in cbcos_create_os_profile");
		goto cleanup;
	}
	tmp = text->data;
	if ((retval = cmdb_add_string_to_list(tmp->data->text, list)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert arch into query list in cbcos_create_os_profile");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cmc, OS_FROM_BUILD_TYPE_AND_ARCH, list, results)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL query for os_id failed with %d", retval);
		goto cleanup;
	}
	id = results->head;
	tmp = id->data;
	new_os_id = tmp->data->number;
	if ((retval = ailsa_list_remove(results, id, &ptr)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot remove member from os_id list");
		goto cleanup;
	}
	ailsa_clean_data((ailsa_data_s *)ptr);
	if ((retval = ailsa_argument_query(cmc, PACKAGE_DETAIL_ON_OS_ID, results, pack)) != 0) {
		ailsa_syslog(LOG_ERR, "PACKAGE_DETAIL_ON_OS_ID query failed: %d", retval);
		goto cleanup;
	}
	if (pack->total == 0) {
		ailsa_syslog(LOG_ERR, "Unable to get package info for previous varients");
		ailsa_syslog(LOG_ERR, "You will have to manually create the varients for this OS");
		goto cleanup;
	}
	if ((retval = cbcos_insert_os_id_user_into_list(pack, new_os_id)) != 0) {
		ailsa_syslog(LOG_ERR, "Inserting new os_id into list failed");
		goto cleanup;
	}
	if ((retval = ailsa_multiple_query(cmc, insert_queries[INSERT_BUILD_PACKAGE], pack)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert build packages into database: %d", retval);
		goto cleanup;
	}
	cleanup:
		ailsa_list_destroy(pack);
		ailsa_list_destroy(list);
		ailsa_list_destroy(results);
		my_free(pack);
		my_free(list);
		my_free(results);
		return retval;
}

static int
cbcos_insert_os_id_user_into_list(AILLIST *pack, unsigned long int id)
{
	if (!(pack) || (id == 0))
		return AILSA_NO_DATA;
	int retval = 0;
	size_t i, total;
	AILELEM *p = pack->head;
	ailsa_data_s *data;

	total = pack->total / 2;
	p = p->next;
	for (i = 0; i < total; i++) {
		data = ailsa_db_lint_data_init();
		data->data->number = id;
		if ((retval = ailsa_list_ins_next(pack, p, (void *)data)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert os id into list");
			goto cleanup;
		}
		p = p->next;
		if ((retval = ailsa_insert_cuser_muser(pack, p)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert cuser and muser into list");
			goto cleanup;
		}
		if (p->next->next->next)
			p = p->next->next->next->next;
	}
	cleanup:
		return retval;
}

static int
ailsa_insert_cuser_muser(AILLIST *list, AILELEM *elem)
{
	if (!(list) || !(elem))
		return AILSA_NO_DATA;
	int i, retval;
	uid_t uid = getuid();
	ailsa_data_s *data;
	AILELEM *p = elem;
	for (i = 0; i < 2; i++) {
		data = ailsa_db_lint_data_init();
		data->data->number = (unsigned long int)uid;
		if ((retval = ailsa_list_ins_next(list, p, (void *)data)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert user id into list round %d", i);
			return retval;
		}
		p = p->next;
	}
	return retval;
}

static int
cbcos_set_default_os(ailsa_cmdb_s *cc, cbcos_comm_line_s *ccl)
{
	if (!(cc) || !(ccl))
		return AILSA_NO_DATA;
	if ((!(ccl->os) && !(ccl->alias)) || (!(ccl->version) && !(ccl->ver_alias)) || !(ccl->arch))
		return AILSA_NO_DATA;
	char **args = ailsa_calloc((sizeof(char *) * 3), "args in cbcos_set_default_os");
	AILLIST *os = ailsa_db_data_list_init();
	AILLIST *def = ailsa_db_data_list_init();

	int retval;
	if (ccl->os)
		args[0] = ccl->os;
	else
		args[0] = ccl->alias;
	if (ccl->version)
		args[1] = ccl->version;
	else
		args[1] = ccl->ver_alias;
	args[2] = ccl->arch;
	if ((retval = cmdb_add_os_id_to_list(args, cc, os)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get OS id");
		goto cleanup;
	}
	if (os->total == 0) {
		ailsa_syslog(LOG_ERR, "Cannot find OS %s; version %s; architecture %s", args[0], args[1], args[2]);
		goto cleanup;
	}
	if ((retval = ailsa_basic_query(cc, DEFAULT_OS, def)) != 0) {
		ailsa_syslog(LOG_ERR, "DEFAULT_OS query failed");
		goto cleanup;
	}
	if (def->total == 0) {
		if ((retval = cmdb_populate_cuser_muser(os)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add cuser and muser to list");
			goto cleanup;
		}
		if ((retval = ailsa_insert_query(cc, INSERT_DEFAULT_OS, os)) != 0) {
			ailsa_syslog(LOG_ERR, "INSERT_DEFAULT_OS query failed");
			goto cleanup;
		}
	} else {
		if ((retval = cmdb_add_number_to_list((unsigned long int)getuid(), os)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot add muser to list in cbcos_set_default_os");
			goto cleanup;
		}
		if ((retval = ailsa_update_query(cc, update_queries[UPDATE_DEFAULT_OS], os)) != 0) {
			ailsa_syslog(LOG_ERR, "UPDATE_DEFAULT_OS query failed");
			goto cleanup;
		}
	}
	cleanup:
		ailsa_list_full_clean(os);
		ailsa_list_full_clean(def);
		my_free(args);
		return retval;
}
