/* build_add.c
 * 
 * Functions to add build information into the database.
 * 
 * Part of the cbc program
 * 
 * (C) 2012 Iain M. Conochie
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>	/* required for IP address conversion */
#include <sys/stat.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_mysql.h"
#include "checks.h"
#include "build.h"

void
get_partition_data(pre_disk_part_t *head, int lvm, char *input);

unsigned long int
insert_scheme_name_into_db(cbc_config_t *config, char *scheme_name);

int
check_for_scheme_name(cbc_config_t *conf, char *name);

int add_partition_scheme(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	unsigned long int id;
	int retval, use_lvm, i;
	char *input, *scheme_name;
	pre_disk_part_t *node, *saved, *head_part;
	
	retval = NONE;
	use_lvm = i = 0;
	head_part = part_node_create();
	printf("***Add Partition Scheme to the database***\n\n");
	
	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_partition_scheme");
	if (!(scheme_name = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "scheme_name in add_partition_scheme");
	
	printf("Do you want to use the logical volume manager?: ");
	fgets(input, CONF_S, stdin);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		use_lvm = 1;
	}
	
	printf("Please input the name of the scheme: ");
	fgets(scheme_name, CONF_S, stdin);
	chomp(scheme_name);
	if ((retval = validate_user_input(scheme_name, NAME_REGEX) < 0)) {
		printf("Scheme name %s not valid\n", scheme_name);
		free(input);
		free(scheme_name);
		return 1;
	}
	
	if ((retval = check_for_scheme_name(config, scheme_name) > 0)) {
		printf("Scheme name %s already used in database\n", scheme_name);
		free(input);
		free(scheme_name);
		return 1;
	}
	
	if ((id = insert_scheme_name_into_db(config, scheme_name)) < 1) {
		printf("Unable to get id from database for scheme name %s\n", scheme_name);
		free(input);
		free(scheme_name);
		return 1;
	}
	while (get_another_partition()) {
		get_partition_data(head_part, lvm, input);
		i++;
	}
	
	free(head_part);
	return retval;
}

int
check_for_scheme_name(cbc_config_t *conf, char *name)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in check_for_scheme_name");
	snprintf(query, RBUFF_S,
"SELECT * FROM partition_schemes WHERE scheme_name = '%s'", name);
	cbc_query = query;
	cbc_mysql_init(conf, &cbc);
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
		return 0;
	} else {
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		return 1;
	}
}

unsigned long int
insert_scheme_name_into_db(cbc_config_t *conf, char *name)
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
"INSERT INTO partition_schemes (scheme_name) VALUES ('%s')", name);
	cbc_query = query;
	cbc_mysql_init(conf, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	cbc_rows = mysql_affected_rows(&cbc);
	if (cbc_rows < 1) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		return id;
	}
	snprintf(query, RBUFF_S,
"SELECT def_scheme_id FROM partition_schemes WHERE scheme_name = '%s'", name);
	cbc_mysql_init(conf, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) != 1)){
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		return id;
	}
	cbc_row = mysql_fetch_row(cbc_res);
	snprintf(query, CONF_S, "%s", cbc_row[0]);
	if ((retval = validate_user_input(query, ID_REGEX)) < 0) {
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		return id;
	}
	id = strtoul(query, NULL, 10);
	printf("Returning %lu\n", id);
	return id;
	
}
void get_partition_data(pre_disk_part_t *head, int lvm, char *input)
{
	pre_disk_part_t *new, *saved;
	int retval;
	
	new = part_node_create();
	saved = head;
	while (saved->nextpart)
		saved = saved->nextpart;
	saved->nextpart = new;
	
	printf("Please enter the mount point: ");
	fgets(input, HOST_S, stdin);
	chomp(input);
	if ((retval = validate_user_input(input, PATH_REGEX) < 0))
		printf("Mount point %s not valid\n", input);
	snprintf(new->mount_point, HOST_S, "%s", input);

	printf("Please enter the file system: ");
	fgets(input, RANGE_S, stdin);
	chomp(input);
	if ((retval = validate_user_input(input, FS_REGEX) < 0))
		printf("Filesystem not valid\n");
	snprintf(new->filesystem, RANGE_S, "%s", input);

	printf("Please enter the minimum partition size (in MB): ");
	fgets(input, RANGE_S, stdin);
	chomp(input);
	if ((retval = validate_user_input(input, ID_REGEX) < 0))
		printf("Size not valid\n");
	new->min = strtoul(input, NULL, 10);
	
	printf("Please enter the maximum partition size (in MB): ");
	fgets(input, RANGE_S, stdin);
	chomp(input);
	if ((retval = validate_user_input(input, ID_REGEX) < 0))
		printf("Size not valid\n");
	new->max = strtoul(input, NULL, 10);
	
	printf("Please enter the priority of the max size: ");
	fgets(input, RANGE_S, stdin);
	chomp(input);
	if ((retval = validate_user_input(input, ID_REGEX) < 0))
		printf("Size not valid\n");
	new->pri = strtoul(input, NULL, 10);
	
	if (lvm == 1) {
		printf("Name for the logical volume: ");
		fgets(input, RANGE_S, stdin);
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
	fgets(input, CONF_S, stdin);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		;
	} else {
		free(new);
		saved->nextpart = 0;
	}
}

void chomp(char *input)
{
	size_t len;
	len = strlen(input);
	if (input[len -1] == '\n')
		input[len -1] = '\0';
}