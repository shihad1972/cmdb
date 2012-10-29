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
#include "mysql_funs.h"

int main(int argc, char *argv[])
{
	MYSQL dnsa;
	MYSQL_ROW dnsa_row;
	MYSQL_RES *dnsa_res;
	comm_line_t command;
	mysql_query_data_t *mydqp;
	my_ulonglong dnsa_rows;
	char *domain, *queryp, dom[CONF_S], config[CHKC + 1][CONF_S];
	const char *dquery, *unix_socket;
	int retval, id;
	unsigned int port = 3306;
	unsigned long int client_flag = 0;

	/* Get command line args. See above */
	retval = parse_command_line(argc, argv, &command);
	if (retval < 0) {
		printf("Usage: %s [-d | -w] [-f | -r] -n <domain/netrange>\n",
			       argv[0]);
		exit (retval);
	}
	/* Get config values from config file */
	sprintf(config[CONFIGFILE], "/etc/dnsa/dnsa.conf");
	retval = parse_config_file(config);
	if (retval < 0) {
		printf("Config file parsing failed! Using default values\n");
	}
	domain = &command.domain;
	if ((strncmp(command.action, "write", COMM_S) == 0)) {
		if ((strncmp(command.type, "forward", COMM_S) == 0)) {
			wzf(domain, config);
		} else if ((strncmp(command.type, "reverse", COMM_S) == 0)) {
			id = get_rev_id(domain, config);
			if (id < 0) {
				fprintf(stderr, "Invalid reverse domain\n");
				exit(NO_DOMAIN);
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
		printf("Display not yet implemented\n");
		exit(0);
	}

	exit(0);
}