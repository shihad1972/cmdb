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
	int i;

	i = parse_command_line(argc, argv, &command);
	if (i >= 0) {
		printf("We got these inputs:\n");
		printf("%s %s %s\n", command.action, command.type, command.domain);
	} else {
		printf("Usage: %s [-d | -w] [-f | -r] -n <domain/netrange>\n",
			       argv[0]);
	}
	

	
	exit(0);
}