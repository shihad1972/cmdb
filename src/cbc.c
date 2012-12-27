/* cbc.c
 * 
 * main function for the cbc program
 * 
 * (C) 2012 Iain M. Conochie
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "checks.h"

int main(int argc, char *argv[])
{
	cbc_config_t *cmc;
	cbc_comm_line_t *cml;
	cbc_build_t *cbt;
	char *cbc_config;
	int retval;
	
	retval = 0;
	if (!(cbc_config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "cbc_config in cbc.c");
	if (!(cmc = malloc(sizeof(cbc_config_t))))
		report_error(MALLOC_FAIL, "cmc in cbc.c");
	if (!(cml = malloc(sizeof(cbc_comm_line_t))))
		report_error(MALLOC_FAIL, "cml in cbc.c");
	if (!(cbt = malloc(sizeof(cbc_build_t))))
		report_error(MALLOC_FAIL, "cbt in cbc.c");
	
	strncpy(cbc_config, "/etc/dnsa/dnsa.conf", CONF_S - 1);
	
	init_all_config(cmc, cml, cbt);
	
	retval = parse_cbc_command_line(argc, argv, cml);
	if (retval < 0)
		display_cmdb_command_line_error(retval, argv[0]);
	
	retval = parse_cbc_config_file(cmc, cbc_config);
	
	free(cbc_config);
	if (retval > 1) {
		parse_cbc_config_error(retval);
		exit(retval);
	}
	get_server_name(cml, cmc);
	retval = get_build_info(cbt, cmc, cml->server_id);
	
	switch (cml->action) {
		case WRITE_CONFIG:
			write_tftp_config(cmc, cbt);
			write_dhcp_config(cmc, cbt);
			retval = write_build_config(cmc, cbt);
			break;
		case DISPLAY_CONFIG:
		/*	print_cbc_config(cmc);
			print_cbc_command_line_values(cml); */
			print_cbc_build_values(cbt);
			break;
		default:
			printf("Case %d not implemented yet\n", cml->action);
			break;
	}
	free(cmc);
	free(cml);
	free(cbt);
	exit(retval);
}