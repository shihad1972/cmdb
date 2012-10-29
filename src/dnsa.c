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
#include "dnsa.h"
#include "write_zone.h"
#include "rev_zone.h"

int main(int argc, char *argv[])
{
	comm_line_t command;
	char *domain, dom[CONF_S], config[CHKC][CONF_S];
	int retval, id, i;

	retval = parse_command_line(argc, argv, &command);
	if (retval < 0) {
		printf("Usage: %s [-d | -w] [-f | -r] -n <domain/netrange>\n",
			       argv[0]);
		exit (retval);
	}
	retval = parse_config_file(config);
	if (retval < 0) {
		printf("Config file parsing failed! Error code %d\n", retval);
		for (i = 0; i < 11; i++) {
			printf("%s ", config[i]);
		}
		exit(0);
	}
	
	strncpy(dom, command.domain, CONF_S);
	domain = dom;
	
	if ((strncmp(command.action, "write", COMM_S) == 0)) {
		if ((strncmp(command.type, "forward", COMM_S) == 0)) {
			wzf(domain);
		} else if ((strncmp(command.type, "reverse", COMM_S) == 0)) {
			id = get_rev_id(domain);
			if (id < 0) {
				fprintf(stderr, "Invalid reverse domain\n");
				exit(NO_DOMAIN);
			} else {
				wrzf(id);
			}
		} else {
			retval = 7;
			printf("We have an invalid type: %s\n",
			       command.type);
			exit(retval);
		}
	} else if ((strncmp(command.action, "display", COMM_S) == 0)) {
		printf("Display not yet implemented\n");
		exit(0);
	}
	printf("Recieved from config file:\n");
	for (i = 0; i < 11; i++) {
		printf("%s ", config[i]);
	}
	exit(0);
}