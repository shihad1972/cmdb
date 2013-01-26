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
"SELECT def_scheme_id, scheme_name, lvm FROM seed_schemes");
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(NO_PARTITION_SCHEMES, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))) {
		add_display_partition_scheme(part, cbc_row);
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	
	saved = part;
	do {
		printf("Partition scheme id %lu: %s ", saved->id, saved->name);
		if (saved->lvm)
			printf("LVM\n");
		else
			printf("No LVM\n");
		get_schemes_partitions(saved->id, config);
		saved = saved->next;
	} while (saved);
	
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
	new->lvm = strtoul(cbc_row[2], NULL, 10);
		
}

partition_schemes_t *init_part_scheme_struct(void)
{
	partition_schemes_t *node;

	if (!(node = malloc(sizeof(partition_schemes_t))))
		report_error(MALLOC_FAIL, "snode in display_partition_schemes");
	
	snprintf(node->name, CONF_S, "NULL");
	node->id = 0;
	node->next = '\0';
	node->lvm = 0;
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
logical_volume FROM default_part WHERE def_scheme_id = %lu ORDER BY mount_point\n", id);
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(SERVER_PART_NOT_FOUND, sserver_id);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		head_part = part_node_add(head_part, cbc_row);
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
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
}

void display_build_operating_systems(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	size_t len;
	char *query;
	const char *cbc_query;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_build_operating_systems");
	cbc_query = query;
	snprintf(query, RBUFF_S,
"SELECT DISTINCT bo.alias, build_type FROM build_os bo LEFT JOIN build_type bt \
on bo.bt_id = bt.bt_id ORDER BY os");
	
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(OS_NOT_FOUND, query);
	}
	printf("Operating System\tBuild Type\n");
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		len = strlen(cbc_row[0]);
		if ((len / 8) > 1) {
			printf("%s\t%s\n", cbc_row[0], cbc_row[1]);
		} else if ((len / 8) > 0) {
			printf("%s\t\t%s\n", cbc_row[0], cbc_row[1]);
		} else {
			printf("%s\t\t\t%s\n", cbc_row[0], cbc_row[1]);
		}
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
}

void display_build_os_versions(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	size_t len, alen;
	char *query, *os, *version;
	const char *cbc_query;
	int os_check, i;
	
	i = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_build_os_versions");
	if (!(os = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "os in display_build_os_versions");
	if (!(version = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "version in display_build_os_versions");
	snprintf(os, MAC_S, "none");
	cbc_query = query;
	snprintf(query, RBUFF_S,
"SELECT os, os_version, ver_alias, arch FROM build_os ORDER BY os, os_version, arch");
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		free(os);
		free(version);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		free(os);
		free(version);
		report_error(OS_VERSION_NOT_FOUND, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		len = strlen(cbc_row[0]);
		alen = strlen(cbc_row[2]);
		if ((os_check = strncmp(os, cbc_row[0], MAC_S) == 0)) {
			if ((os_check = strncmp(version, cbc_row[1], MAC_S) == 0)) {
				printf(" and %s", cbc_row[3]);
			} else {
				printf("\n");
				if ((alen / 8) > 0) {
					printf("\t\t%s\t%s\t%s",
					 cbc_row[1], cbc_row[2], cbc_row[3]);
				} else {
					printf("\t\t%s\t%s\t\t%s",
					 cbc_row[1], cbc_row[2], cbc_row[3]);
				}
			}
		} else {
			snprintf(os, MAC_S, "%s", cbc_row[0]);
			if (i > 0)
				printf("\n");
			if ((len / 8) > 1) {
				printf("%s:\n\t\tversion\talias\t\tarch\n", cbc_row[0]);
			} else if ((len / 8) > 0) {
				printf("%s:\tversion\talias\t\tarch\n", cbc_row[0]);
			} else {
				printf("%s:\t\tversion\talias\t\tarch\n", cbc_row[0]);
			}
			if ((alen / 8) > 0) {
				printf("\t\t%s\t%s\t%s",
				 cbc_row[1], cbc_row[2], cbc_row[3]);
			} else {
				printf("\t\t%s\t%s\t\t%s",
				 cbc_row[1], cbc_row[2], cbc_row[3]);
			}
		}
		snprintf(version, MAC_S, "%s", cbc_row[1]);
		i++;
	}
	printf("\n");
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	free(os);
	free(version);
}

void display_build_domains(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char sip_address[16], eip_address[16];
	char *query;
	const char *cbc_query;
	size_t len;
	uint32_t sip_addr, eip_addr;
	unsigned long int sip, eip;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_build_domains");
	
	snprintf(query, RBUFF_S,
"SELECT domain, start_ip, end_ip FROM build_domain");
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(BUILD_DOMAIN_NOT_FOUND, query);
	}
	printf("Build domains:\nDomain\t\t\tStart IP\tEnd IP\n");
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		len = strlen(cbc_row[0]);
		sip = strtoul(cbc_row[1], NULL, 10);
		eip = strtoul(cbc_row[2], NULL, 10);
		sip_addr = htonl((unsigned int)sip);
		eip_addr = htonl((unsigned int)eip);
		inet_ntop(AF_INET, &sip_addr, sip_address, 16);
		inet_ntop(AF_INET, &eip_addr, eip_address, 16);
		if ((len / 8) > 1)
			printf("%s\t%s\t%s\n", cbc_row[0], sip_address, eip_address);
		else if ((len / 8) > 0)
			printf("%s\t\t%s\t%s\n", cbc_row[0], sip_address, eip_address);
		else
			printf("%s\t\t\t%s\t%s\n", cbc_row[0], sip_address, eip_address);
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
}

void display_build_varients(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	size_t len;
	char *query;
	const char *cbc_query;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_build_varients");
	
	cbc_query = query;
	snprintf(query, RBUFF_S, "SELECT varient, valias FROM varient");
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		cmdb_mysql_clean_full(cbc_res, &cbc, query);
		report_error(VARIENT_NOT_FOUND, query);
	}
	printf("Build varients:\nAlias\t\tFull Name\n");
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		len = strlen(cbc_row[1]);
		if ((len / 8) > 0)
			printf("%s\t%s\n", cbc_row[1], cbc_row[0]);
		else
			printf("%s\t\t%s\n", cbc_row[1], cbc_row[0]);
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
}

void display_build_locales(cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query, *input;
	const char *cbc_query;
	int retval;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in display_build_locales");
	if (!(input = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in display_build_locales");
	snprintf(query, RBUFF_S,
"SELECT DISTINCT alias FROM build_os");
	printf("Please choose the OS you wish to view locales for\n\n");
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
		report_error(OS_NOT_FOUND, query);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res)))
		printf("%s\n", cbc_row[0]);
	mysql_free_result(cbc_res);
	printf("\n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the OS from the above list:\n");
		input = fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(query, RBUFF_S,
"SELECT locale_id, os_version, arch, locale, country, language, keymap FROM \
locale l, build_os bo WHERE bo.os_id = l.os_id AND bo.alias = '%s' \
ORDER BY os_version", input);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_clean(&cbc, query);
		free(input);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		printf("OS %s does not have any locales\n", input);
		printf("Perhaps you may like to add some?\n");
	} else {
		printf("Locales for %s\n", input);
		printf("ID\tOS Ver\tArch\tLocale\t\tCountry\tLanguage\tKeymap\n");
		while ((cbc_row = mysql_fetch_row(cbc_res))) {
			printf("%s\t%s\t%s\t%s\t%s\t%s\t\t%s\n",
cbc_row[0], cbc_row[1], cbc_row[2], cbc_row[3], cbc_row[4], cbc_row[5], cbc_row[6]);
		}
	}
	cmdb_mysql_clean_full(cbc_res, &cbc, query);
	free(input);
}