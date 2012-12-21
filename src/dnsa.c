/* 
 * 
 * dnsa.c: First attempt to write a multi function command line program
 * for dnsa.
 * Command line args:
 * 
 * -d :display
 * -w :write
 * -c :write configuration
 * -l :list zones in database.
 *   **At least one of these must be present**
 * -f : forward zone
 * -r : Reverse zone
 *   **One of these must be present but not both**
 * -n : domain name or netblock. only /8 /16 /24 netblocks accepted
 *      needed for -d and -w functionality
 *
 * (C) 2012 Iain M Conochie
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "forward.h"
#include "reverse.h"
#include "checks.h"

int main(int argc, char *argv[])
{
	comm_line_t *cm;
	dnsa_config_t *dc;
	char *domain, *config;
	int retval, id;

	if (!(domain = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "domain in dnsa.c");
	if (!(config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "config in dnsa.c");
	if (!(dc = malloc(sizeof(dnsa_config_t))))
		report_error(MALLOC_FAIL, "dc in dnsa.c");
	if (!(cm = malloc(sizeof(comm_line_t))))
		report_error(MALLOC_FAIL, "cm in dnsa.c");
	
	/* Get command line args. See above */
	retval = parse_dnsa_command_line(argc, argv, cm);
	if (retval < 0)
		display_cmdb_command_line_error(retval, argv[0]);
	
	/* Get config values from config file */	
	init_config_values(dc);
	sprintf(config, "%s", cm->config);
	retval = parse_dnsa_config_file(dc, config);
	free(config);
	if (retval > 1) {
		parse_dnsa_config_error(retval);
		exit(retval);
	}

	strncpy(domain, cm->domain, CONF_S);
	if (cm->type == FORWARD_ZONE) {
		retval = validate_user_input(domain, DOMAIN_REGEX);
		if (retval < 0) {
			printf("User input not valid!\n");
			free(domain);
			exit (retval);
		}
		if (cm->action == WRITE_ZONE) {
			wzf(domain, dc);
		} else if (cm->action == DISPLAY_ZONE) {
			dzf(domain, dc);
		} else if (cm->action == LIST_ZONES) {
			list_zones(dc);
		} else if (cm->action == CONFIGURE_ZONE) {
			wcf(dc);
		} else if (cm->action == ADD_ZONE) {
			add_fwd_zone(domain, dc);
		} else if (cm->action == ADD_HOST) {
			add_fwd_host(dc, cm);
		}
	} else if (cm->type == REVERSE_ZONE) {
		id = get_rev_id(domain, dc);
		if (id < 0)
			report_error(NO_DOMAIN, domain);
		if (cm->action == WRITE_ZONE) {
			wrzf(id, dc);
		} else if (cm->action == DISPLAY_ZONE) {
			drzf(id, domain, dc);
		} else if (cm->action == LIST_ZONES) {
			list_rev_zones(dc);
		} else if (cm->action == CONFIGURE_ZONE) {
			wrcf(dc);
		}
	} else {
		retval = WRONG_TYPE;
		printf("We have an invalid type id %d\n", cm->type);
		free(domain);
		exit(retval);
	}
	
	free(domain);
	free(cm);
	free(dc);
	exit(0);
}