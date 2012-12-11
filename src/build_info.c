/* build_info.c
 * 
 * Functions to retrieve build information from the database and
 * to create the specific build files required.
 * 
 * Part of the cbc program
 * 
 * (C) 2012 Iain M. Conochie
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_mysql.h"

void
fill_build_info(cbc_build_t *cbt, MYSQL_ROW br);

void get_server_name(cbc_comm_line_t *info, cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query, server_id[40];
	const char *cbc_query;
	
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_server_name");
	
	if (info->server_id != 0)
		sprintf(query, "SELECT server_id, name, uuid FROM server WHERE server_id = %ld", info->server_id);
	else if ((strncmp(info->name, "NULL", CONF_S)))
		sprintf(query, "SELECT server_id, name, uuid FROM server WHERE name = '%s'", info->name);
	else if ((strncmp(info->uuid, "NULL", CONF_S)))
		sprintf(query, "SELECT server_id, name, uuid FROM server WHERE uuid = '%s'", info->uuid);
	
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	sprintf(server_id, "%ld", info->server_id);
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		if (strncmp(info->name, "NULL", CONF_S))
			report_error(SERVER_NOT_FOUND, info->name);
		else if (strncmp(info->uuid, "NULL", CONF_S))
			report_error(SERVER_UUID_NOT_FOUND, info->uuid);
		else if (info->server_id > 1)
			report_error(SERVER_ID_NOT_FOUND, server_id);
		else
			report_error(NO_NAME_UUID_ID, server_id);
	} else if (cbc_rows > 1) {
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MULTIPLE_SERVERS, info->name);
	}
	cbc_row = mysql_fetch_row(cbc_res);
	info->server_id = strtoul(cbc_row[0], NULL, 10);
	strncpy(info->name, cbc_row[1], CONF_S);
	strncpy(info->uuid, cbc_row[2], CONF_S);
	
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	free(query);
}

int get_build_info(cbc_build_t *build_info, cbc_config_t *config, unsigned long int server_id)
{
	MYSQL build;
	MYSQL_RES *build_res;
	MYSQL_ROW build_row;
	my_ulonglong build_rows;
	int retval;
	char *query;
	char sserver_id[40];
	const char *build_query;
	
	retval = NONE;
	sprintf(sserver_id, "%ld", server_id);
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_info");
	
	sprintf(query,
"SELECT arch, alias, os_version, INET_NTOA(ip), mac_addr, INET_NTOA(netmask),\
 INET_NTOA(gateway), INET_NTOA(ns), hostname, domainname, boot_line, valias,\
 ver_alias FROM build_ip bi LEFT JOIN (build_domain bd, build_os bo, build b,\
 server s, boot_line bootl, varient v) ON (bi.bd_id = bd.bd_id AND\
 b.ip_id = bi.ip_id AND bo.os_id = b.os_id AND s.server_id = b.server_id AND\
 bootl.boot_id = bo.boot_id AND b.varient_id = v.varient_id)\
 WHERE s.server_id = %ld", server_id);
	build_query = query;
	cbc_mysql_init(config, &build);
	cmdb_mysql_query(&build, build_query);
	if (!(build_res = mysql_store_result(&build))) {
		mysql_close(&build);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&build));
	}
	if (((build_rows = mysql_num_rows(build_res)) == 0)){
		mysql_close(&build);
		mysql_library_end();
		free(query);
		report_error(SERVER_ID_NOT_FOUND, sserver_id);
	} else if (build_rows > 1) {
		mysql_free_result(build_res);
		mysql_close(&build);
		mysql_library_end();
		free(query);
		report_error(MULTIPLE_SERVER_IDS, sserver_id);
	}
	build_row = mysql_fetch_row(build_res);
	fill_build_info(build_info, build_row);
	mysql_free_result(build_res);
	mysql_close(&build);
	mysql_library_end();
	free(query);
	return retval;
}

void write_build_config(cbc_comm_line_t *cclt, cbc_config_t *cct, cbc_build_t *cbt)
{
	uint32_t ip_addr;
	char ip_address[16];
	char hex_ip[10];
	sprintf(ip_address, "%s", cbt->ip_address);
	inet_pton(AF_INET, ip_address, &ip_addr);
	ip_addr = htonl(ip_addr);
	sprintf(hex_ip, "%lX", (unsigned long)ip_addr);
}

void fill_build_info(cbc_build_t *cbt, MYSQL_ROW br)
{
	sprintf(cbt->arch, "%s", br[0]);
	sprintf(cbt->alias, "%s", br[1]);
	sprintf(cbt->version, "%s", br[2]);
	sprintf(cbt->ip_address, "%s", br[3]);
	sprintf(cbt->mac_address, "%s", br[4]);
	sprintf(cbt->netmask, "%s", br[5]);
	sprintf(cbt->gateway, "%s", br[6]);
	sprintf(cbt->nameserver, "%s", br[7]);
	sprintf(cbt->hostname, "%s", br[8]);
	sprintf(cbt->domain, "%s", br[9]);
	sprintf(cbt->boot, "%s", br[10]);
	sprintf(cbt->varient, "%s", br[11]);
	if (br[12])
		sprintf(cbt->ver_alias, "%s", br[12]);
}

