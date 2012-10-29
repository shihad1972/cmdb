/* errors.c: Error reporting functions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"

void report_error(int error)
{
	switch (error){
		case ARGC_INVAL:
			fprintf(stderr, "Argc is invalid\n");
			exit(1);
			break;
		case ARGV_INVAL:
			fprintf(stderr, "Argv is invalid\n");
			exit(2);
			break;
		case NO_DOMAIN:
			fprintf(stderr, "No domain was found\n");
			exit(3);
			break;
		case MULTI_DOMAIN:
			fprintf(stderr, "Multiple domains found\n");
			exit(4);
			break;
		case NO_DELIM:
			fprintf(stderr, "No delimiter found in string\n");
			exit(5);
			break;
		case NO_RECORDS:
			fprintf(stderr, "No records found for the zone\n");
			exit(6);
			break;
		case WRONG_ACTION:
			fprintf(stderr, "Incorrect action specified\n");
			exit(7);
			break;
		case MY_INIT_FAIL:
			fprintf(stderr, "Initialisation of MySQL connection failed\n");
			exit(10);
			break;
		case MY_CONN_FAIL:
			fprintf(stderr, "Unable to connect to MySQL Database\n");
			exit(11);
			break;
		case MY_QUERY_FAIL:
			fprintf(stderr, "Query to MySQL database failed\n");
			exit(12);
			break;
		case MY_STORE_FAIL:
			fprintf(stderr, "Unable to store MySQL result set\n");
			exit(13);
			break;
		case FILE_O_FAIL:
			fprintf(stderr, "Unable to open file\n");
			exit(20);
			break;
		default:
			fprintf(stderr, "Unknown error code %d\n", error);
			exit(error);
			break;
	}
}