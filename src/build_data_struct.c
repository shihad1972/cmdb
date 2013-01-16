/* build_data_struct.c
 * 
 * Functions to manipulate data structures for the cbc program
 * 
 * Part of the cbc program
 * 
 * (C) 2012 - 2013 Iain M. Conochie
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
#include "build.h"

pre_disk_part_t *part_node_create(void)
{
	pre_disk_part_t *dptr;
	
	if (!(dptr = malloc(sizeof(pre_disk_part_t))))
		report_error(MALLOC_FAIL, "disk_part in part_node_create");
	dptr->nextpart = 0;
	dptr->min = dptr->max = dptr->pri = 0;
	sprintf(dptr->mount_point, "NULL");
	sprintf(dptr->filesystem, "NULL");
	return dptr;
}

pre_disk_part_t *part_node_add(pre_disk_part_t *head_node, MYSQL_ROW part_row)
{
	pre_disk_part_t *new_node, *node;
/*	new_node = part_node_create(); */
	if (!(new_node = malloc(sizeof(pre_disk_part_t))))
		report_error(MALLOC_FAIL, "disk_part in part_node_add");
	
	new_node->min = strtoul(part_row[0], NULL, 10);
	new_node->max = strtoul(part_row[1], NULL, 10);
	new_node->pri = strtoul(part_row[2], NULL, 10);
	snprintf(new_node->mount_point, HOST_S, "%s", part_row[3]);
	snprintf(new_node->filesystem, RANGE_S, "%s", part_row[4]);
	new_node->part_id = strtoul(part_row[5], NULL, 10);
	snprintf(new_node->log_vol, RANGE_S, "%s", part_row[6]);
	new_node->nextpart = NULL;
	if (!head_node) {
		head_node = new_node;
	} else {
		for (node = head_node; node->nextpart; node=node->nextpart) {
			;
		}
		node->nextpart = new_node;
	}
	return head_node;
}

cbc_domain_ip_t *ip_node_create(void)
{
	cbc_domain_ip_t *node;
	
	if (!(node = malloc(sizeof(cbc_domain_ip_t))))
		report_error(MALLOC_FAIL, "node in ip_node_create");
	node->ip = 0;
	node->next = 0;
	snprintf(node->hostname, CONF_S, "NULL");
	return node;
}

void ip_node_add(cbc_build_domain_t *bd, MYSQL_ROW myrow)
{
	cbc_domain_ip_t *saved, *new;
	
	new = ip_node_create();
	new->ip = strtoul(myrow[0], NULL, 0);
	snprintf(new->hostname, CONF_S, "%s", myrow[1]);
	new->next = 0;
	if (!(bd->iplist)) {
		bd->iplist = new;
	} else {
		for (saved = bd->iplist; saved->next; saved=saved->next) {
			;
		}
		saved->next = new;
	}
}

void ip_node_add_basic(cbc_build_domain_t *bd, unsigned long int ip, const char *host)
{
	cbc_domain_ip_t *saved, *new;
	
	new = ip_node_create();
	new->ip = ip;
	snprintf(new->hostname, CONF_S, "%s", host);
	new->next = 0;
	if (!(bd->iplist)) {
		bd->iplist = new;
	} else {
		for (saved = bd->iplist; saved->next; saved=saved->next) {
			;
		}
		saved->next = new;
	}
}