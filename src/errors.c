/* errors.c:
 * 
 * Error reporting functions.
 * 
 * enum constants defined in cmdb_dnsa.h
 * 
 * Part of the DNSA  program
 * 
 * (C) Iain M Conochie 2012
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"

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
			fprintf(stderr, "No delimiter %s found in string\n", errstr);
			exit(NO_DELIM);
			break;
		case NO_RECORDS:
			fprintf(stderr, "No records found for the zone %s\n", errstr);
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
			fprintf(stderr, "Unable to store MySQL result set: %s\n", errstr);
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

void display_cmdb_command_line_error(int retval, char *program)
{
	switch (retval){
		case NO_NAME:
			fprintf(stderr, "No name specified with -n\n");
			break;
		case NO_ID:
			fprintf(stderr, "No ID specified with -i\n");
			break;
		case NO_TYPE:
			fprintf(stderr, "No type specified on command line\n");
			break;
		case NO_ACTION:
			fprintf(stderr, "No action specified on command line\n");
			break;
		case NO_NAME_OR_ID:
			fprintf(stderr, "No name or ID specified on command line\n");
			break;
		case GENERIC_ERROR:
			fprintf(stderr, "Unknown command line option\n");
			break;
		case DISPLAY_USAGE:
			if ((strncmp(program, "cmdb", CONF_S) == 0))
				display_cmdb_usage();
			else if ((strncmp(program, "cbc", CONF_S) == 0))
				display_cbc_usage();
			exit(retval);
			break;
		default:
			fprintf(stderr, "Unknown error code!\n");
			break;
	}

	printf("Usage: %s [-s | -c | -t ] [-d | -l ] [-n <name> | -i <id> ]\n",
	       program);
	exit (retval);
}

void display_cmdb_usage(void)
{
	printf("CMDB: Configuration Management Database\n\n");
	printf("Action options:\n");
	printf("-d: display\n-l: list\n-a: add\n");
	printf("-t: add contact (when specified with -c for customer)\n\n");
	printf("Type options:\n");
	printf("-s: server\n-c: customer\n\n");
	printf("Name options:\n");
	printf("-n: name\n-i: uuid for server or coid for customer\n");
}

void display_cbc_usage(void)
{
	printf("cbc: Create Build Configuration\n\n");
	printf("Action options:\n");
	printf("-l: create\n-y: display\n\n");
	printf("Build type options:\n");
	printf("-k: kickstart\n-p: preseed\n\n");
	printf("Name options:\n");
	printf("-n: name\n-u: uuid for server\n-t: server_id\n");
}