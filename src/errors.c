/* errors.c:
 * 
 * Error reporting functions for the DNSA program.
 * 
 * enum constants defined in dnsa.h
 * 
 * (C) Iain M Conochie 2012 <iain@ailsatech.net>
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"

void report_error(int error, const char *errstr)
{
	switch (error){
		case ARGC_INVAL:
			fprintf(stderr, "Argc is invalid\n");
			exit(ARGC_INVAL);
			break;
		case ARGV_INVAL:
			fprintf(stderr, "Argv is invalid\n");
			exit(ARGV_INVAL);
			break;
		case NO_DOMAIN:
			fprintf(stderr, "No domain %s was found\n", errstr);
			exit(NO_DOMAIN);
			break;
		case MULTI_DOMAIN:
			fprintf(stderr, "Multiple records found for %s\n", errstr);
			exit(MULTI_DOMAIN);
			break;
		case NO_DELIM:
			fprintf(stderr, "No delimiter found in string\n");
			exit(NO_DELIM);
			break;
		case NO_RECORDS:
			fprintf(stderr, "No records found for the zone\n");
			exit(NO_RECORDS);
			break;
		case WRONG_ACTION:
			fprintf(stderr, "Incorrect action specified\n");
			exit(WRONG_ACTION);
			break;
		case WRONG_TYPE:
			fprintf(stderr, "Incorrect domain type specified\n");
			exit(WRONG_TYPE);
			break;
		case DOMAIN_LIST_FAIL:
			fprintf(stderr, "No domains were found to list from the database\n");
			exit(DOMAIN_LIST_FAIL);
			break;
		case MY_INIT_FAIL:
			fprintf(stderr, "Initialisation of MySQL connection failed\n");
			exit(MY_INIT_FAIL);
			break;
		case MY_CONN_FAIL:
			fprintf(stderr, "Unable to connect to MySQL Database: %s\n", errstr);
			exit(MY_CONN_FAIL);
			break;
		case MY_QUERY_FAIL:
			fprintf(stderr, "Query to MySQL database failed\n");
			exit(MY_QUERY_FAIL);
			break;
		case MY_STORE_FAIL:
			fprintf(stderr, "Unable to store MySQL result set\n");
			exit(MY_STORE_FAIL);
			break;
		case FILE_O_FAIL:
			fprintf(stderr, "Unable to open file %s\n", errstr);
			exit(FILE_O_FAIL);
			break;
		case CHKZONE_FAIL:
			fprintf(stderr, "Checking the zone %s failed\n", errstr);
			exit(CHKZONE_FAIL);
			break;
		case MALLOC_FAIL:
			fprintf(stderr, "Malloc / Calloc failed for %s\n", errstr);
			exit(MALLOC_FAIL);
			break;
		default:
			fprintf(stderr, "Unknown error code %d\n", error);
			exit(error);
			break;
	}
}