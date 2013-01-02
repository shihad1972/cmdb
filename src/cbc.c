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
	if (retval < 0) {
		free(cmc);
		free(cml);
		free(cbt);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	retval = parse_cbc_config_file(cmc, cbc_config);
	
	free(cbc_config);
	if (retval > 1) {
		parse_cbc_config_error(retval);
		free(cml);
		free(cmc);
		free(cbt);
		exit(retval);
	}
	
	switch (cml->action) {
		case WRITE_CONFIG:
			get_server_name(cml, cmc);
			retval = get_build_info(cmc, cbt, cml->server_id);
			write_tftp_config(cmc, cbt);
			write_dhcp_config(cmc, cbt);
			retval = write_build_config(cmc, cbt);
			break;
		case DISPLAY_CONFIG:
			if ((cml->server > 0) && 
			 (strncmp(cml->action_type, "NULL", MAC_S) == 0)) {
		/*	print_cbc_config(cmc);
			print_cbc_command_line_values(cml); */
				get_server_name(cml, cmc);
				retval = get_build_info(cmc, cbt, cml->server_id);
				print_cbc_build_values(cbt);
			} else if (cml->server == 0) {
				if ((strncmp(cml->action_type, "partition", MAC_S) == 0))
					display_partition_schemes(cmc);
				else if ((strncmp(cml->action_type, "os", MAC_S) == 0))
					display_build_operating_systems(cmc);
				else
					printf("Can only display partition config\n");
			} else if (cml->server > 0) {
				printf("Cannot handle server and type actions\n");
			}
			break;
		case ADD_CONFIG:
			if (strncmp(cml->action_type, "partition", MAC_S) == 0)
				retval = add_partition_scheme(cmc);
			else if (strncmp(cml->action_type, "os", MAC_S) == 0)
				printf("Adding OS not yet implemented\n");
			else if (strncmp(cml->action_type, "os_version", MAC_S) == 0)
				printf("Adding OS version not yet implemented\n");
			else if (strncmp(cml->action_type, "build_domain", MAC_S) == 0)
				printf("Adding build_domain not yet implemented\n");
			else if (strncmp(cml->action_type, "varient", MAC_S) == 0)
				printf("Adding varient not yet implemented\n");
			else
				printf("Case %s not implemented yet\n",
				 cml->action_type);
			break;
		case CREATE_CONFIG:
			printf("Creating the config not yet implemented\n");
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