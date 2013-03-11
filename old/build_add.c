/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  build_add.c
 * 
 *  Functions to add build information into the database.
 * 
 *  Part of the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>	/* required for IP address conversion */
#include <sys/stat.h>
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_mysql.h"
#include "checks.h"
#include "build.h"

void
get_partition_data(pre_disk_part_t *head, int lvm, char *input);

unsigned long int
insert_scheme_name_into_db(cbc_config_t *config, char *scheme_name, int lvm);

int
get_another_partition(void);

int
check_for_scheme_name(cbc_config_t *conf, char *name);

void
show_all_partitions(pre_disk_part_t *head);

void
add_pre_part_to_db(pre_disk_part_t *partition, cbc_config_t *config);

int add_partition_scheme(cbc_config_t *config)
{
	unsigned long int id;
	int retval, use_lvm, i;
	char *input, *scheme_name;
	pre_disk_part_t *head_part, *node, *saved;
	
	retval = NONE;
	use_lvm = i = 0;
	head_part = part_node_create();
	printf("***Add Partition Scheme to the database***\n\n");
	
	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_partition_scheme");
	if (!(scheme_name = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "scheme_name in add_partition_scheme");
	
	printf("Do you want to use the logical volume manager?: ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		use_lvm = 1;
	}
	
	printf("Please input the name of the scheme: ");
	scheme_name = fgets(scheme_name, CONF_S, stdin);
	chomp(scheme_name);
	if ((retval = validate_user_input(scheme_name, NAME_REGEX) < 0)) {
		printf("Scheme name %s not valid\n", scheme_name);
		free(input);
		free(scheme_name);
		return 1;
	}
	do {
		get_partition_data(head_part, use_lvm, input);
	} while ((retval = get_another_partition() > 0));
	show_all_partitions(head_part);
	
	if ((retval = check_for_scheme_name(config, scheme_name) > 0)) {
		printf("Scheme name %s already used in database\n", scheme_name);
		free(input);
		free(scheme_name);
		return 1;
	}
	
	if ((id = insert_scheme_name_into_db(config, scheme_name, use_lvm)) < 1) {
		printf("Unable to get id from database for scheme name %s\n", scheme_name);
		free(input);
		free(scheme_name);
		return 1;
	}
	
	node = head_part;
	/* Should add some error checking to this really */
	do {
		node->part_id = id;
		add_pre_part_to_db(node, config);
		node = node->nextpart;
	} while (node);
	
	node = saved = head_part;
	while (node) {
		saved = node->nextpart;
		free(node);
		node = saved;
	}
	return retval;
}

void add_pre_part_to_db(pre_disk_part_t *partition, cbc_config_t *config)
{
	MYSQL cbc;
	char *query;
	const char *cbc_query;
	int log_vol_check;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_pre_part_to_db");
	if ((log_vol_check = strncmp(partition->log_vol, "NULL", CONF_S) == 0)) {
		snprintf(query, RBUFF_S,
"INSERT INTO default_part (minimum, maximum, priority, def_scheme_id, \
mount_point, filesystem) \
VALUES (%lu, %lu, %lu, %lu, '%s', '%s')", 
partition->min, partition->max, partition->pri, partition->part_id, partition->mount_point,
partition->filesystem);
	} else {
		snprintf(query, RBUFF_S,
"INSERT INTO default_part (minimum, maximum, priority, def_scheme_id, \
mount_point, filesystem, logical_volume) \
VALUES (%lu, %lu, %lu, %lu, '%s', '%s', '%s')", 
partition->min, partition->max, partition->pri, partition->part_id, partition->mount_point,
partition->filesystem, partition->log_vol);
	}
	
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	cmdb_mysql_clean(&cbc, query);
}

int check_for_scheme_name(cbc_config_t *conf, char *name)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in check_for_scheme_name");
	snprintf(query, RBUFF_S,
"SELECT * FROM seed_schemes WHERE scheme_name = '%s'", name);
	cbc_query = query;
	cbc_mysql_init(conf, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return 0;
	} else {
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return 1;
	}
}

unsigned long int
insert_scheme_name_into_db(cbc_config_t *conf, char *name, int lvm)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	unsigned long int id;
	id = 0L;
	retval = 0;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_scheme_name_into_db");
	
	snprintf(query, RBUFF_S,
"INSERT INTO seed_schemes (scheme_name, lvm) VALUES ('%s', %d)", name, lvm);
	cbc_query = query;
	cbc_mysql_init(conf, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	cbc_rows = mysql_affected_rows(&cbc);
	if (cbc_rows < 1) {
		cmdb_mysql_clean(&cbc, query);
		return id;
	}
	snprintf(query, RBUFF_S,
"SELECT def_scheme_id FROM seed_schemes WHERE scheme_name = '%s'", name);
	cbc_mysql_init(conf, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) != 1)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return id;
	}
	cbc_row = mysql_fetch_row(cbc_res);
	snprintf(query, CONF_S, "%s", cbc_row[0]);
	if ((retval = validate_user_input(query, ID_REGEX)) < 0) {
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return id;
	}
	id = strtoul(query, NULL, 10);
	printf("Partition has id %lu in the database\n", id);
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return id;
	
}

int get_another_partition(void)
{
	char *answer;
	
	if (!(answer = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "answer in get_another_partition");
	
	printf("Enter another partition (y/n)? ");
	answer = fgets(answer, CONF_S, stdin);
	chomp(answer);
	if ((strncmp(answer, "y", CH_S)) == 0 || (strncmp(answer, "Y", CH_S) == 0)) {
		printf("Getting another partition\n");
		free(answer);
		return 1;
	} else {
		printf("OK. No more partitions\n");
		free(answer);
		return 0;
	}
}

void get_partition_data(pre_disk_part_t *head, int lvm, char *input)
{
	pre_disk_part_t *new, *saved = '\0';
	
	int retval;
	
	/* First check if head has been filled */
	retval = strncmp(head->mount_point, "NULL", CONF_S);
	if (retval != 0) {
		new = part_node_create();
		saved = head;
		while (saved->nextpart)
			saved = saved->nextpart;
		saved->nextpart = new;
	} else {
		new = head;
	}
	
	printf("Please enter the mount point: ");
	input = fgets(input, HOST_S, stdin);
	chomp(input);
	if ((strncmp(input, "swap", HOST_S) == 0)) {
		retval = 0;
	} else {
		retval = validate_user_input(input, PATH_REGEX);
	}
	while (retval < 0) {
		printf("Mount point %s not valid\n", input);
		printf("Please enter the mount point: ");
		input = fgets(input, HOST_S, stdin);
		chomp(input);
		retval = validate_user_input(input, PATH_REGEX);
	}
	snprintf(new->mount_point, HOST_S, "%s", input);

	printf("Please enter the file system: ");
	input = fgets(input, RANGE_S, stdin);
	chomp(input);
	retval = validate_user_input(input, FS_REGEX);
	while (retval < 0) {
		printf("Filesystem not valid\n");
		printf("Please enter the file system: ");
		input = fgets(input, RANGE_S, stdin);
		chomp(input);
		retval = validate_user_input(input, FS_REGEX);
	}
	snprintf(new->filesystem, RANGE_S, "%s", input);

	printf("Please enter the minimum partition size (in MB): ");
	input = fgets(input, RANGE_S, stdin);
	chomp(input);
	retval = validate_user_input(input, ID_REGEX);
	while (retval < 0) {
		printf("Size not valid\n");
		printf("Please enter the minimum partition size (in MB): ");
		input = fgets(input, RANGE_S, stdin);
		chomp(input);
		retval = validate_user_input(input, ID_REGEX);
	}
	new->min = strtoul(input, NULL, 10);
	
	printf("Please enter the maximum partition size (in MB): ");
	input = fgets(input, RANGE_S, stdin);
	chomp(input);
	retval = validate_user_input(input, ID_REGEX);
	while ((retval < 0) || (new->min > (strtoul(input, NULL, 10)))) {
		printf("Size not valid\n");
		printf("Please enter the maximum partition size (in MB): ");
		input = fgets(input, RANGE_S, stdin);
		chomp(input);
		retval = validate_user_input(input, ID_REGEX);
		if (retval >= 0) {
			new->max = strtoul(input, NULL, 10);
			if (new->max < new->min) {
				retval = -1;
				printf("Max must be greater than min\n");
			}
		}
	}
	new->max = strtoul(input, NULL, 10);
	
	printf("Please enter the priority of the max size: ");
	input = fgets(input, RANGE_S, stdin);
	chomp(input);
	retval = validate_user_input(input, ID_REGEX);
	while (retval < 0) {
		printf("Size not valid\n");
		printf("Please enter the priority of the max size: ");
		input = fgets(input, RANGE_S, stdin);
		chomp(input);
		retval = validate_user_input(input, ID_REGEX);
	}
	new->pri = strtoul(input, NULL, 10);
	
	if (lvm == 1) {
		printf("Name for the logical volume: ");
		input = fgets(input, RANGE_S, stdin);
		chomp(input);
		if ((retval = validate_user_input(input, NAME_REGEX) < 0))
			printf("Logical volume name not valid\n");
		snprintf(new->log_vol, RANGE_S, "%s", input);
	}
	
	printf("\n\nInput values\n");
	printf("Mount point: %s\n", new->mount_point);
	printf("Filesystem: %s\n", new->filesystem);
	printf("Minimum size: %lu\n", new->min);
	printf("Maximum size: %lu\n", new->max);
	printf("Priority of the maximum size: %lu\n", new->pri);
	if (lvm == 1)
		printf("Logical volume name: %s\n", new->log_vol);
	printf("Accept (y/n)? ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		;
	} else {
		if (head != new) {
			free(new);
			saved->nextpart = 0;
		}
	}
}

void show_all_partitions(pre_disk_part_t *head)
{
	pre_disk_part_t *node;
	size_t len;
	node = head;
	printf("\n");
	do {
		printf("Partition %s on FS %s\n", node->mount_point, node->filesystem);
		printf("Min size: %lu\tMax Size: %lu\tPriority: %lu\n",
		       node->min, node->max, node->pri);
		if ((len = strlen(node->log_vol) > 0))
			printf("Logical volume: %s\n", node->log_vol);
		printf("\n");
		node = node->nextpart;
	} while (node);
}

int create_build_config(cbc_config_t *config, cbc_comm_line_t *cml, cbc_build_t *cbt)
{
/*	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query; */
	int retval;
	
	retval = 0;
	if ((retval = get_server_name(cml, config)) != 0)
		return retval;
	if ((strncmp(cml->os, "NULL", CONF_S) == 0))
		if ((retval = get_os_from_user(config, cml)) != 0)
			return retval;
	if ((strncmp(cml->os_version, "NULL", MAC_S) == 0))
		if ((retval = get_os_version_from_user(config, cml)) != 0)
			return retval;
	if ((strncmp(cml->arch, "NULL", MAC_S) == 0))
		if ((retval = get_os_arch_from_user(config, cml)) != 0)
			return retval;
	if ((retval = get_build_os_id(config, cml)) != 0)
		return retval;
	if (cml->locale == 0)
		if ((retval = get_locale_from_user(config, cml)) != 0)
			return retval;
	if ((strncmp(cml->build_domain, "NULL", RBUFF_S) == 0))
		if ((retval = get_build_domain_from_user(config, cml)) != 0)
			return retval;
	if ((strncmp(cml->varient, "NULL", CONF_S) == 0))
		if ((retval = get_build_varient_from_user(config, cml)) != 0)
			return retval;
	if ((strncmp(cml->partition, "NULL", CONF_S) == 0))
		if ((retval = get_disk_scheme_from_user(config, cml)) != 0)
			return retval;
	if ((retval = copy_build_values(cml, cbt)) != 0)
		return retval;
	if ((retval = delete_build_if_exists(config, cbt)) != 0)
		return CANNOT_DELETE_BUILD;
	if ((retval = get_build_hardware(config, cbt)) != 0)
		return retval;
	if ((retval = get_build_varient_id(config, cbt)) != 0)
		return retval;
	if ((retval = get_build_partition_id(config, cbt)) != 0)
		return retval;
	if ((retval = get_build_domain_id(config, cbt)) != 0)
		return retval;
	get_base_os_version(cbt);
	if ((retval = get_build_boot_line_id(config, cbt)) != 0)
		return retval;
	if ((retval = insert_build_into_database(config, cbt)) != 0)
		return retval;
	print_cbc_build_values(cbt);
	print_cbc_build_ids(cbt);
	/* print_cbc_command_line_values(cml); */

	return retval;
}

int get_os_from_user(cbc_config_t *config, cbc_comm_line_t *cml)
{
	char *input;
	int retval;
	
	retval = 0;
	if (!(input = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_os_from_user");
	printf("Please choose the OS you wish to use.\n\n");
	display_build_operating_systems(config);
	printf("\n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the OS from the above list:\n");
		input = fgets(input, CONF_S, stdin);
		chomp(input);
	}
	if (retval > 0)
		retval = 0;
	snprintf(cml->os, CONF_S, "%s", input);
	free(input);
	
	return retval;
}

int get_os_version_from_user(cbc_config_t *config, cbc_comm_line_t *cml)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query, *input;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_os_version_from_user");
	if (!(input = calloc(MAC_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_os_version_from_user");
	snprintf(query, RBUFF_S,
"SELECT DISTINCT os_version FROM build_os WHERE alias = '%s' ORDER BY \
os_version", cml->os);
	printf("Please choose the OS version you wish to use.\n\n");
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(OS_VERSION_NOT_FOUND, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res)))
		printf("%s\n", cbc_row[0]);
	mysql_free_result(cbc_res);
	printf("\n");
	input = fgets(input, MAC_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, OS_VER_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the OS version from the above list:\n");
		input = fgets(input, MAC_S, stdin);
		chomp(input);
	}
	if (retval > 0)
		retval = 0;
	snprintf(cml->os_version, MAC_S, "%s", input);
	mysql_close(&cbc);
	mysql_library_end();
	free(query);
	free(input);
	
	return retval;
}

int get_os_arch_from_user(cbc_config_t *config, cbc_comm_line_t *cml)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query, *input;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_os_arch_from_user");
	if (!(input = calloc(MAC_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_os_arch_from_user");
	snprintf(query, RBUFF_S,
"SELECT DISTINCT arch FROM build_os WHERE alias = '%s' AND os_version = '%s' \
ORDER BY arch", cml->os, cml->os_version);
	printf("Please choose the OS arch you wish to use.\n\n");
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(WRONG_OS_ARCH, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res)))
		printf("%s\n", cbc_row[0]);
	mysql_free_result(cbc_res);
	printf("\n");
	input = fgets(input, MAC_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the OS arch from the above list:\n");
		input = fgets(input, MAC_S, stdin);
		chomp(input);
	}
	if (retval > 0)
		retval = 0;
	snprintf(cml->arch, MAC_S, "%s", input);
	mysql_close(&cbc);
	mysql_library_end();
	free(query);
	free(input);
	
	return retval;
}

int get_build_os_id(cbc_config_t *config, cbc_comm_line_t *cml)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_os_arch_from_user");
	snprintf(query, RBUFF_S,
"SELECT os_id FROM build_os WHERE alias = '%s' AND os_version = '%s' AND \
arch = '%s'", cml->os, cml->os_version, cml->arch);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)) {
		retval = OS_DOES_NOT_EXIST;
	} else if (cbc_rows > 1) {
		retval = MULTIPLE_OS;
	} else {
		cbc_row = mysql_fetch_row(cbc_res);
		cml->os_id = strtoul(cbc_row[0], NULL, 10);
	}
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	free(query);
	
	return retval;
}

int get_build_domain_from_user(cbc_config_t *config, cbc_comm_line_t *cml)
{
	char *input;
	int retval;
	
	retval = 0;
	if (!(input = calloc(MAC_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_build_doman_from_user");
	printf("Please choose the build domain you wish to use.\n\n");
	display_build_domains(config);
	printf("\n");
	input = fgets(input, MAC_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, DOMAIN_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the OS arch from the above list:\n");
		input = fgets(input, MAC_S, stdin);
		chomp(input);
	}
	if (retval > 0)
		retval = 0;
	snprintf(cml->build_domain, MAC_S, "%s", input);
	
	return retval;
}

int get_build_varient_from_user(cbc_config_t *config, cbc_comm_line_t *cml)
{
	char *input;
	int retval;
	
	retval = 0;
	if (!(input = calloc(MAC_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_build_varient_from_user");
	printf("Please choose the varient you wish to use.\n\n");
	display_build_varients(config);
	printf("\n");
	input = fgets(input, MAC_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the varient from the above list:\n");
		input = fgets(input, MAC_S, stdin);
		chomp(input);
	}
	if (retval > 0)
		retval = 0;
	snprintf(cml->varient, MAC_S, "%s", input);
	free(input);
	
	return retval;
}

int get_locale_from_user(cbc_config_t *config, cbc_comm_line_t *cml)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query, *input;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_locale_from_user");
	if (!(input = calloc(MAC_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_locale_from_user");
	snprintf(query, RBUFF_S,
"SELECT locale_id, locale, country, language, keymap FROM locale WHERE \
os_id = %lu", cml->os_id);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		free(input);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		free(input);
		report_error(NO_LOCALE_FOR_OS, cml->os);
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		printf("Only 1 locale for OS %s, ver %s, arch %s\n",
		 cml->os, cml->os_version, cml->arch);
		cml->locale = strtoul(cbc_row[0], NULL, 10);
	} else {
		printf("Please choose the locale you wish to use.\n\n");
		printf("Locale Selection; select ID\n");
		printf("ID\tLocale\t\tCountry\tLanguage\tKeymap\n");
		while ((cbc_row = mysql_fetch_row(cbc_res)))
			printf("%s\t%s\t%s\t%s\t%s\n",
		  cbc_row[0], cbc_row[1], cbc_row[2], cbc_row[3], cbc_row[4]);
		printf("\n");
		input = fgets(input, MAC_S, stdin);
		chomp(input);
		while ((retval = validate_user_input(input, ID_REGEX) < 0)) {
			printf("User input not valid!\n");
			printf("Please input the OS arch from the above list:\n");
			input = fgets(input, MAC_S, stdin);
			chomp(input);
		}
		if (retval > 0)
			retval = 0;
		cml->locale = strtoul(input, NULL, 10);
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	free(input);
	
	return retval;
}

int get_disk_scheme_from_user(cbc_config_t *config, cbc_comm_line_t *cml)
{
	char *scheme_name;
	int retval;
	
	retval = 0;
	if (!(scheme_name = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "scheme_name in get_disk_scheme_from_user");
	printf("Partition Schemes\n");
	display_partition_schemes(config);
	printf("Input the name of the scheme you wish to use\n");
	scheme_name = fgets(scheme_name, CONF_S, stdin);
	chomp(scheme_name);
	while ((retval = validate_user_input(scheme_name, NAME_REGEX) < 0)) {
		printf("Scheme name not valid!\n");
		printf("Please input one of the scheme names show above\n");
		scheme_name = fgets(scheme_name, CONF_S, stdin);
		chomp(scheme_name);
	}
	snprintf(cml->partition, CONF_S, "%s", scheme_name);
	free(scheme_name);
	return retval;
}

int copy_build_values(cbc_comm_line_t *cml, cbc_build_t *cbt)
{
	int retval;
	
	retval = 0;
	copy_initial_build_values(cml, cbt);
	
	return retval;
}

void copy_initial_build_values(cbc_comm_line_t *cml, cbc_build_t *cbt)
{
	snprintf(cbt->part_scheme_name, CONF_S, "%s", cml->partition);
	snprintf(cbt->hostname, CONF_S, "%s", cml->name);
	snprintf(cbt->domain, RBUFF_S, "%s", cml->build_domain);
	snprintf(cbt->alias, CONF_S, "%s", cml->os);
	snprintf(cbt->version, MAC_S, "%s", cml->os_version);
	snprintf(cbt->varient, CONF_S, "%s", cml->varient);
	snprintf(cbt->arch, MAC_S, "%s", cml->arch);
	cbt->server_id = cml->server_id;
	cbt->os_id = cml->os_id;
	cbt->locale_id = cml->locale;
}

int get_build_hardware(cbc_config_t *config, cbc_build_t *cbt)
{
	char ntype[HOST_S] = "network";
	char nclass[HOST_S] = "Network Card";
	char stype[HOST_S] = "storage";
	char sclass[HOST_S] = "Hard Disk";
	int retval;
	unsigned long int disk, net;
	
	retval = 0;
	net = get_hard_type_id(config, ntype, nclass); 
	disk = get_hard_type_id(config, stype, sclass);
	if ((retval = get_build_hardware_device(config, net, cbt->server_id, ntype, nclass)) != 0)
		return retval;
	if ((retval = get_build_hardware_device(config, disk, cbt->server_id, stype, sclass)) != 0)
		return retval;
	snprintf(cbt->diskdev, MAC_S, "%s", stype);
	snprintf(cbt->netdev, RANGE_S, "%s", ntype);
	snprintf(cbt->mac_address, MAC_S, "%s", nclass);
	return retval;
}

unsigned long int get_hard_type_id(cbc_config_t *config, char *htype, char *hclass)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	unsigned long int retval;

	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_hard_type_id");
	snprintf(query, RBUFF_S,
"SELECT hard_type_id FROM hard_type WHERE type = '%s' AND class = '%s'",
	 htype, hclass);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(NO_HARDWARE_TYPES, hclass);
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		retval = strtoul(cbc_row[0], NULL, 10);
	} else if (cbc_rows > 1) {
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(MULTIPLE_HARDWARE_TYPES, hclass);
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return retval;
	
}

int get_build_hardware_device(cbc_config_t *config, unsigned long int id, unsigned long int sid, char *device, char *detail)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_hard_type_id");
	snprintf(query, RBUFF_S,
"SELECT device, detail FROM hardware WHERE hard_type_id = %lu AND server_id = %lu",
	 id, sid);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(NO_HARDWARE_TYPES, detail);
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		snprintf(device, MAC_S, "%s", cbc_row[0]);
		snprintf(detail, HOST_S, "%s", cbc_row[1]);
	} else if (cbc_rows > 1) {
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(MULTIPLE_HARDWARE_TYPES, detail);
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return retval;
}

int get_build_varient_id(cbc_config_t *config, cbc_build_t *cbt)
{	
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_varient_id");
	snprintf(query, RBUFF_S,
"SELECT varient_id FROM varient WHERE valias = '%s'", cbt->varient);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(VARIENT_NOT_FOUND, cbt->varient);
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		cbt->varient_id = strtoul(cbc_row[0], NULL, 10);
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return retval;
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return MULTIPLE_VARIENTS;
}

int get_build_partition_id(cbc_config_t *config, cbc_build_t *cbt)
{	
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_partition_id");
	snprintf(query, RBUFF_S,
"SELECT def_scheme_id, lvm FROM seed_schemes WHERE scheme_name = '%s'",
	 cbt->part_scheme_name);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(SCHEME_NOT_FOUND, cbt->part_scheme_name);
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		cbt->def_scheme_id = strtoul(cbc_row[0], NULL, 10);
		if ((strncmp(cbc_row[1], "1", COMM_S)) == 0)
			cbt->use_lvm = 1;
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return retval;
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return MULTIPLE_SCHEMES;
}

void get_base_os_version(cbc_build_t *cbt)
{
	const char *k;
	char *j;
	
	snprintf(cbt->base_ver, MAC_S, "%s", cbt->version);
	k = cbt->base_ver;
	j = strchr(k, '.');
	*j = '\0';
	printf("Base Version: %s\n", cbt->base_ver);
}

int get_build_domain_id(cbc_config_t *config, cbc_build_t *cbt)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_varient_id");
	snprintf(query, RBUFF_S,
"SELECT bd_id FROM build_domain WHERE domain = '%s'",
	 cbt->domain);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(BUILD_DOMAIN_NOT_FOUND, cbt->alias);
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		cbt->bd_id = strtoul(cbc_row[0], NULL, 10);
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return retval;
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return MULTIPLE_BUILD_DOMAINS;
}

int get_build_boot_line_id (cbc_config_t *config, cbc_build_t *cbt)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_varient_id");
	snprintf(query, RBUFF_S,
"SELECT boot_id FROM boot_line WHERE os = '%s' AND os_ver = '%s'",
	 cbt->alias, cbt->base_ver);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(BOOT_LINE_NOT_FOUND, cbt->alias);
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		cbt->boot_id = strtoul(cbc_row[0], NULL, 10);
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return retval;
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return MULTIPLE_BOOT_LINES;
}

int insert_build_into_database(cbc_config_t *config, cbc_build_t *cbt)
{
	int retval;
	
	retval = 0;
	if (!(cbt->build_dom = malloc(sizeof(cbc_build_domain_t))))
		report_error(MALLOC_FAIL, "cbt->build_dom in insert_build_into_database");
	if ((retval = get_build_domain_info_on_id(config, cbt->build_dom, cbt->bd_id)) != 0) {
		free(cbt->build_dom);
		return retval;
	}
	if ((retval = get_build_ip(config, cbt->build_dom)) != 0) {
		free(cbt->build_dom);
		free(cbt->build_dom->iplist);
		return retval;
	}
	convert_build_ip_address(cbt);
	if ((retval = insert_ip_into_db(config, cbt)) != 0) {
		free(cbt->build_dom);
		free(cbt->build_dom->iplist);
		return retval;
	}
	if ((retval = insert_into_build_table(config, cbt)) != 0) {
		free(cbt->build_dom);
		free(cbt->build_dom->iplist);
		return retval;
	}
	if ((retval = insert_build_partitions(config, cbt)) != 0) {
		free(cbt->build_dom);
		free(cbt->build_dom->iplist);
		return retval;
	}
	if ((retval = insert_disk_device(config, cbt)) != 0) {
		free(cbt->build_dom);
		free(cbt->build_dom->iplist);
		return retval;
	}
	return retval;
}

int get_build_domain_info_on_id(cbc_config_t *config, cbc_build_domain_t *cbdt, unsigned long int id)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_domain_info_on_id");
	snprintf(query, RBUFF_S,
"SELECT start_ip, end_ip,  netmask, gateway, ns FROM build_domain WHERE bd_id = %lu",
	 id);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		retval = BUILD_DOMAIN_NOT_FOUND;
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		cbdt->start_ip = strtoul(cbc_row[0], NULL, 10);
		cbdt->end_ip = strtoul(cbc_row[1], NULL, 10);
		cbdt->netmask = strtoul(cbc_row[2], NULL, 10);
		cbdt->gateway = strtoul(cbc_row[3], NULL, 10);
		cbdt->ns = strtoul(cbc_row[4], NULL, 10);
		cbdt->iplist = 0;
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		retval = 0;
	} else {
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		retval = MULTIPLE_BUILD_DOMAINS;
	}
	return retval;
}

int
get_build_ip(cbc_config_t *config, cbc_build_domain_t *bd)
{
	cbc_domain_ip_t *iplist, *saved;
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval, found;
	unsigned long int ip;
	
	retval = 0;
	found = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_varient_id");
	snprintf(query, RBUFF_S,
"SELECT ip, hostname FROM build_ip WHERE ip >= %lu AND ip <= %lu",
	 bd->start_ip, bd->end_ip);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		ip_node_add_basic(bd, bd->start_ip, "free");
	} else {
		while ((cbc_row = mysql_fetch_row(cbc_res))) {
			ip = strtoul(cbc_row[0], NULL, 10);
			if ((ip < bd->start_ip) || (ip > bd->end_ip)) {
				continue;
			} else {
				ip_node_add(bd, cbc_row);
			}
		}
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
	}
	
	iplist = bd->iplist;
	for (ip = bd->start_ip; ip <= bd->end_ip; ip++) {
		found = 0;
		if (ip == iplist->ip) {
			if ((strncmp(iplist->hostname, "free", COMM_S)) == 0) {
				break;
			} else {
				continue;
			}
		}
		iplist = bd->iplist;
		while (iplist) {
			if (iplist->ip == ip) {
				found = 1;
				break;
			} else {
				iplist = iplist->next;
			}
		}
		if (found == 1) {
			continue;
		} else {
			break;
		}
	}
	if (found == 0)
		bd->iplist->ip = ip;
	else
		retval = NO_BUILD_IP;
	saved = iplist = bd->iplist->next;
	while (saved) {
		saved = iplist->next;
		free(iplist);
		iplist = saved;
	}
	return retval;
}

void convert_build_ip_address(cbc_build_t *cbt)
{
	
	char ip_address[16];
	uint32_t ip_addr;
	
	ip_addr = htonl((unsigned int)cbt->build_dom->iplist->ip);
	inet_ntop(AF_INET, &ip_addr, ip_address, 16);
	snprintf(cbt->ip_address, RANGE_S, "%s", ip_address);
	ip_addr = htonl((unsigned int)cbt->build_dom->netmask);
	inet_ntop(AF_INET, &ip_addr, ip_address, 16);
	snprintf(cbt->netmask, RANGE_S, "%s", ip_address);
	ip_addr = htonl((unsigned int)cbt->build_dom->gateway);
	inet_ntop(AF_INET, &ip_addr, ip_address, 16);
	snprintf(cbt->gateway, RANGE_S, "%s", ip_address);
	ip_addr = htonl((unsigned int)cbt->build_dom->ns);
	inet_ntop(AF_INET, &ip_addr, ip_address, 16);
	snprintf(cbt->nameserver, RANGE_S, "%s", ip_address);
}

int insert_ip_into_db(cbc_config_t *config, cbc_build_t *cbt)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_ip_into_db");
	snprintf(query, RBUFF_S,
"SELECT * FROM build_ip WHERE ip = %lu\n", cbt->build_dom->iplist->ip);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) > 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		return BUILD_IP_IN_USE;
	}
	mysql_free_result(cbc_res);
	snprintf(query, RBUFF_S,
"INSERT INTO build_ip (ip, hostname, domainname, bd_id) VALUES \
(%lu, '%s', '%s', %lu)",
	  cbt->build_dom->iplist->ip, cbt->hostname, cbt->domain, cbt->bd_id);
	cmdb_mysql_query(&cbc, cbc_query);
	if ((cbc_rows = mysql_affected_rows(&cbc)) != 1) {
		cmdb_mysql_clean(&cbc, query);
		retval = CANNOT_INSERT_IP;
	}
	snprintf(query, RBUFF_S,
"SELECT ip_id FROM build_ip WHERE ip = %lu", cbt->build_dom->iplist->ip);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) > 1)){
		retval = MULTIPLE_BUILD_IPS;
	} else if (cbc_rows == 1) {
		cbc_row = mysql_fetch_row(cbc_res);
		cbt->ip_id = strtoul(cbc_row[0], NULL, 10);
		retval = 0;
	} else {
		retval = CANNOT_FIND_BUILD_IP;
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	return retval;
}

int insert_into_build_table(cbc_config_t *config, cbc_build_t *cbt)
{
	MYSQL cbc;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_into_build_table");
	snprintf(query, RBUFF_S,
"INSERT INTO build (server_id, os_id, varient_id, boot_id, ip_id, locale_id, mac_addr, net_inst_int) \
VALUES (%lu, %lu, %lu, %lu, %lu, %lu, '%s', '%s')",
		 cbt->server_id,
		 cbt->os_id,
		 cbt->varient_id,
		 cbt->boot_id,
		 cbt->ip_id,
		 cbt->locale_id,
		 cbt->mac_address,
		 cbt->netdev);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if ((cbc_rows = mysql_affected_rows(&cbc)) != 1) {
		retval = CANNOT_INSERT_BUILD;
	}
	cmdb_mysql_clean(&cbc, query);
	return retval;
}

int insert_build_partitions(cbc_config_t *config, cbc_build_t *cbt)
{
	MYSQL cbc;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_build_partitions");
	snprintf(query, RBUFF_S,
"INSERT INTO seed_part (server_id, minimum, maximum, priority, mount_point, filesystem, logical_volume) \
SELECT '%lu', minimum, maximum, priority, mount_point, filesystem, logical_volume FROM \
default_part  WHERE def_scheme_id = %lu", cbt->server_id, cbt->def_scheme_id);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if ((cbc_rows = mysql_affected_rows(&cbc)) < 1) {
		retval = CANNOT_INSERT_PARTITIONS;
	}
	cmdb_mysql_clean(&cbc, query);
	return retval;
}

int insert_disk_device(cbc_config_t *config, cbc_build_t *cbt)
{
	MYSQL cbc;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	int retval;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_disk_device");
	snprintf(query, RBUFF_S,
"INSERT INTO disk_dev (server_id, device, lvm) VALUES (%lu, '/dev/%s', %d)",
		 cbt->server_id, cbt->diskdev, cbt->use_lvm);
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if ((cbc_rows = mysql_affected_rows(&cbc)) < 1) {
		retval = CANNOT_INSERT_DISK_DEVICE;
	}
	cmdb_mysql_clean(&cbc, query);
	return retval;
}
