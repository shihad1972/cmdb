/* build_display.c
 * 
 * Functions to display build information from the database.
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
get_partition_schemes(cbc_config_t *config, partition_schemes_t *partition);

void
add_display_partition_scheme(partition_schemes_t *part, MYSQL_ROW cbc_row);

partition_schemes_t
*init_part_scheme_struct(void);

void
get_schemes_partitions(unsigned long int id, cbc_config_t *config);

void display_partition_schemes(cbc_config_t *config)
{
	partition_schemes_t *snode;
	
	snode = init_part_scheme_struct();
	get_partition_schemes(config, snode);
}

void
get_partition_schemes(cbc_config_t *config, partition_schemes_t *part)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	partition_schemes_t *saved, *node;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_partition_schemes");
	cbc_query = query;
	
	snprintf(query, RBUFF_S,
"SELECT def_scheme_id, scheme_name FROM partition_schemes");
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(NO_PARTITION_SCHEMES, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))) {
		add_display_partition_scheme(part, cbc_row);
	}
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	
	saved = part;
	do {
		printf("Partition scheme id %lu: %s\n", saved->id, saved->name);
		get_schemes_partitions(saved->id, config);
		saved = saved->next;
	} while (saved);
	free(query);
	
	node = saved = part;
	while (node) {
		saved = node->next;
		free(node);
		node = saved;
	}
}

void add_display_partition_scheme(partition_schemes_t *head, MYSQL_ROW cbc_row)
{
	partition_schemes_t *new, *saved;
	int retval;
	
	retval = strncmp(head->name, "NULL", CONF_S);
	if (retval != 0) {
		new = init_part_scheme_struct();
		saved = head;
		while (saved->next)
			saved = saved->next;
		saved->next = new;
	} else {
		new = head;
	}
	
	new->id = strtoul(cbc_row[0], NULL, 10);
	snprintf(new->name, CONF_S, "%s", cbc_row[1]);
		
}

partition_schemes_t *init_part_scheme_struct(void)
{
	partition_schemes_t *node;

	if (!(node = malloc(sizeof(partition_schemes_t))))
		report_error(MALLOC_FAIL, "snode in display_partition_schemes");
	
	snprintf(node->name, CONF_S, "NULL");
	node->id = 0;
	node->next = '\0';
	return node;
}

void
get_schemes_partitions(unsigned long int id, cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	char sserver_id[HOST_S];
	const char *cbc_query;
	int logvol_check;
	pre_disk_part_t *node, *saved;
	pre_disk_part_t *head_part = 0;
	
	snprintf(sserver_id, HOST_S, "%lu", id);
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_schemes_partitions");
	
	cbc_query = query;
	snprintf(query, BUFF_S,
"SELECT minimum, maximum, priority, mount_point, filesystem, def_part_id, \
logical_volume FROM default_partitions WHERE def_scheme_id = %lu ORDER BY mount_point\n", id);
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
		report_error(SERVER_PART_NOT_FOUND, sserver_id);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		head_part = part_node_add(head_part, cbc_row);
	}
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	node = head_part;
	while(node) {
		printf("\t%s\t%s\t%lu\t%lu\t%lu\t",
		       node->mount_point,
		       node->filesystem,
		       node->min,
		       node->max,
		       node->pri);
		if ((logvol_check = strncmp(node->log_vol, "none", RANGE_S) != 0))
			printf("%s", node->log_vol);
		printf("\n");
		node = node->nextpart;
	}
	node = saved = head_part;
	while (node) {
		saved = node->nextpart;
		free(node);
		node = saved;
	}
	free(query);
}

void display_build_operating_systems(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query;
	const char *cbc_query;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_build_operating_systems");
	cbc_query = query;
	snprintf(query, RBUFF_S,
"SELECT DISTINCT os, build_type FROM build_os bo LEFT JOIN build_type bt \
on bo.bt_id = bt.bt_id ORDER BY os");
	
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
		report_error(OS_NOT_FOUND, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		printf("We have an os %s with a build type %s\n",
		      cbc_row[0], cbc_row[1]);
	}
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	free(query);
}

void display_build_os_versions(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query, *os;
	const char *cbc_query;
	int os_check;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_build_os_versions");
	if (!(os = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "os in display_build_os_versions");
	snprintf(os, MAC_S, "none");
	cbc_query = query;
	snprintf(query, RBUFF_S,
"SELECT os, os_version, ver_alias, arch FROM build_os ORDER BY os, os_version, arch");
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		free(os);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		free(os);
		report_error(OS_VERSION_NOT_FOUND, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		if ((os_check = strncmp(os, cbc_row[0], MAC_S) == 0)) {
			printf("\t%s\t%s\t%s\n",
			       cbc_row[1], cbc_row[2], cbc_row[3]);
		} else {
			snprintf(os, MAC_S, "%s", cbc_row[0]);
			printf("Operating system %s\n\tversion\talias\tarch\n", cbc_row[0]);
			printf("\t%s\t%s\t%s\n",
			       cbc_row[1], cbc_row[2], cbc_row[3]);
		}
	}
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	free(query);
	free(os);
}
