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

int main(int argc, char *argv[])
{
	comm_line_t command, *cm;
	dnsa_config_t dnsa_c, *dc;
	char *domain, *config;
	int retval, id;

	dc = &dnsa_c;
	cm = &command;
	
	/* Get command line args. See above */
	retval = parse_command_line(argc, argv, &command);
	if (retval < 0) {
		printf("Usage: %s [-d | -w | -c | -l] [-f | -r] -n <domain/netrange>\n",
			       argv[0]);
		exit (retval);
	}
	
	if (!(domain = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "domain in dnsa.c");
	if (!(config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "config in dnsa.c");
	
	/* Get config values from config file */	
	init_config_values(dc);
	sprintf(config, "/etc/dnsa/dnsa.conf");
	retval = parse_config_file(dc, config);
	if (retval < 0) {
		printf("Config file parsing failed! Using default values\n");
	}

	strncpy(domain, cm->domain, CONF_S);
	if (cm->type == FORWARD_ZONE) {
		if (cm->action == WRITE_ZONE) {
			wzf(domain, dc);
		} else if (cm->action == DISPLAY_ZONE) {
			dzf(domain, dc);
		} else if (cm->action == LIST_ZONES) {
			list_zones(dc);
		} else if (cm->action == CONFIGURE_ZONE) {
			wcf(dc);
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
		exit(retval);
	}
	
	free(config);
	free(domain);
	exit(0);
}