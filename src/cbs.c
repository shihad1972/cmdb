/* cbs.c
 * 
 * main function for the cbs program
 * 
 * (C) 2012 Iain M. Conochie
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cbs.h"
#include "checks.h"

int main(int argc, char *argv[])
{
	cbs_config_t cbs_c, *cmc;
	char *cbs_config;
	int retval;
	
	retval = 0;
	if (!(cbs_config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "cbs_config in cbs.c");
	
	cmc = &cbs_c;
	strncpy(cbs_config, "/etc/dnsa/dnsa.conf", CONF_S - 1);
	
	init_cbs_config_values(cmc);
	
	retval = parse_cbs_config_file(cmc, cbs_config);
	if (retval == -2) {
		retval = 1;
		printf("Port value higher that 65535!\n");
		exit (retval);
	}
	
	exit(retval);
}