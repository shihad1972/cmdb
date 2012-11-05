/* 
 * 
 * dnsa.c: First attempt to write a multi function command line program
 * for dnsa.
 * Command line args:
 * 
 * -d :display
 * -w :write
 *   **One of these must be present but not both**
 * -f : forward zone
 * -r : Reverse zone
 *   **One of these must be present but not both**
 * -n : domain name or netblock. only /8 /16 /24 netblocks accepted
 *
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"
#include "write_zone.h"
#include "rev_zone.h"

int main(int argc, char *argv[])
{
	comm_line_t command;
	char *domain, config[CHKC + 1][CONF_S];
	int retval, id;

	/* Get command line args. See above */
	retval = parse_command_line(argc, argv, &command);
	if (retval < 0) {
		printf("Usage: %s [-d | -w | -c] [-f | -r] -n <domain/netrange>\n",
			       argv[0]);
		exit (retval);
	}
	/* Get config values from config file */
	sprintf(config[CONFIGFILE], "/etc/dnsa/dnsa.conf");
	retval = parse_config_file(config);
	if (retval < 0) {
		printf("Config file parsing failed! Using default values\n");
	}
	if (!(domain = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "domain");
	strncpy(domain, command.domain, CONF_S);
	if ((strncmp(command.action, "write", COMM_S) == 0)) {
		if ((strncmp(command.type, "forward", COMM_S) == 0)) {
			wzf(domain, config);
		} else if ((strncmp(command.type, "reverse", COMM_S) == 0)) {
			id = get_rev_id(domain, config);
			if (id < 0) {
				report_error(NO_DOMAIN, domain);
			} else {
				wrzf(id, config);
			}
		} else {
			retval = 7;
			printf("We have an invalid type: %s\n",
			       command.type);
			exit(retval);
		}
	} else if ((strncmp(command.action, "display", COMM_S) == 0)) {
		if ((strncmp(command.type, "forward", COMM_S) == 0)) {
			dzf(domain, config);
		} else if ((strncmp(command.type, "reverse", COMM_S) == 0)) {
			id = get_rev_id(domain, config);
			if (id < 0) {
				report_error(NO_DOMAIN, domain);
			} else {
				drzf(id, domain, config);
			}
		}
	} else if ((strncmp(command.action, "config", COMM_S) == 0)) {
		if ((strncmp(command.type, "forward", COMM_S) == 0)) {
			wcf(config);
		} else if ((strncmp(command.type, "reverse", COMM_S) == 0)) {
			wrcf(config);
		}
	}

	exit(0);
}