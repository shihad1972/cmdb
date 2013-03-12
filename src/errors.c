/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  errors.c:
 * 
 *  Error reporting functions.
 * 
 *  enum constants defined in cmdb_dnsa.h
 * 
 *  Part of the CMDB program
 * 
 *  (C) Iain M Conochie 2012 - 2013
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"

void
report_error(int error, const char *errstr)
{
	if (error == ARGC_INVAL) {
		fprintf(stderr, "Argc is invalid\n");
		exit(ARGC_INVAL);
	} else if (error == ARGV_INVAL) {
		fprintf(stderr, "Argv is invalid\n");
		exit(ARGV_INVAL);
	} else if (error == NO_DOMAIN) {
		fprintf(stderr, "No domain %s was found\n", errstr);
		exit(NO_DOMAIN);
	} else if (error == MULTI_DOMAIN) {
		fprintf(stderr, "Multiple records found for %s\n", errstr);
		exit(MULTI_DOMAIN);
	} else if (error == NO_DELIM) {
		fprintf(stderr, "No delimiter %s found in string\n", errstr);
		exit(NO_DELIM);
	} else if (error == NO_RECORDS) {
		fprintf(stderr, "No records found for the zone %s\n", errstr);
		exit(NO_RECORDS);
	} else if (error == NO_FORWARD_RECORDS) {
		fprintf(stderr, "No records found for forward zone %s\n", errstr);
		exit(NO_FORWARD_RECORDS);
	} else if (error == WRONG_ACTION) {
		fprintf(stderr, "Incorrect action specified\n");
		exit(WRONG_ACTION);
	} else if (error == WRONG_TYPE) {
		fprintf(stderr, "Incorrect domain type specified\n");
		exit(WRONG_TYPE);
	} else if (error == DOMAIN_LIST_FAIL) {
		fprintf(stderr, "No domains were found to list from the database\n");
		exit(DOMAIN_LIST_FAIL);
	} else if (error == MY_INIT_FAIL) {
		fprintf(stderr, "DB initialisation failed with %s\n", errstr);
		exit(MY_INIT_FAIL);
	} else if (error == MY_CONN_FAIL) {
		fprintf(stderr, "Unable to connect to database: %s\n", errstr);
		exit(MY_CONN_FAIL);
	} else if (error == MY_QUERY_FAIL) {
		fprintf(stderr, "Query to database failed with error %s\n", errstr);
		exit(MY_QUERY_FAIL);
	} else if (error == MY_STORE_FAIL) {
		fprintf(stderr, "Unable to store DB result set: %s\n", errstr);
		exit(MY_STORE_FAIL);
	} else if (error == MY_INSERT_FAIL) {
		fprintf(stderr, "Unable to insert into DB:\n%s\n", errstr);
		exit(MY_INSERT_FAIL);
	} else if (error == MY_STATEMENT_FAIL) {
		fprintf(stderr, "DB statment failed with %s\n", errstr);
		exit(MY_STATEMENT_FAIL);
	} else if (error == MY_BIND_FAIL) {
		fprintf(stderr, "DB bind of prepared statement failed with %s\n", errstr);
		exit (MY_BIND_FAIL);
	} else if (error == FILE_O_FAIL) {
		fprintf(stderr, "Unable to open file %s\n", errstr);
		exit(FILE_O_FAIL);
	} else if (error == CHKZONE_FAIL) {
		fprintf(stderr, "Checking the zone %s failed\n", errstr);
		exit(CHKZONE_FAIL);
	} else if (error == NO_ZONE_CONFIGURATION) {
		fprintf(stderr, "There are no dnsa configuration values in the database\n");
		exit(NO_ZONE_CONFIGURATION);
	} else if (error == CANNOT_INSERT_ZONE) {
		fprintf(stderr, "Unable to add zone %s to database", errstr);
		exit(CANNOT_INSERT_ZONE);
	} else if (error == CANNOT_INSERT_RECORD) {
		fprintf(stderr, "Unable to add record %s to database", errstr);
		exit(CANNOT_INSERT_RECORD);
	} else if (error == MALLOC_FAIL) {
		fprintf(stderr, "Malloc / Calloc failed for %s\n", errstr);
		exit(MALLOC_FAIL);
	} else if (error == SERVER_NOT_FOUND) {
		fprintf(stderr, "Server %s not found in database\n", errstr);
		exit(SERVER_NOT_FOUND);
	} else if (error == MULTIPLE_SERVERS) {
		fprintf(stderr, "Multiple servers with name %s found in database\n", errstr);
		exit(MULTIPLE_SERVERS);
	} else if (error == SERVER_ID_NOT_FOUND) {
		fprintf(stderr, "Server with id %s not found in database\n", errstr);
		exit(SERVER_ID_NOT_FOUND);
	} else if (error == MULTIPLE_SERVER_IDS) {
		fprintf(stderr, "Multiple servers with id %s in database\n\
		THIS SHOULD NOT HAPPEN. Fix the DB!!\n", errstr);
		exit(MULTIPLE_SERVER_IDS);
	} else if (error == SERVER_UUID_NOT_FOUND) {
		fprintf(stderr, "Server with uuid %s not found in database\n", errstr);
		exit(SERVER_UUID_NOT_FOUND);
	} else if (error == MULTIPLE_SERVER_UUIDS) {
		fprintf(stderr, "Multiple servers with uuid %s in database\n\
		THIS SHOULD NOT HAPPEN. Check your database!\n", errstr);
	} else if (error == CUSTOMER_NOT_FOUND) {
		fprintf(stderr, "Customer %s not found\n", errstr);
		exit(CUSTOMER_NOT_FOUND);
	} else if (error == MULTIPLE_CUSTOMERS) {
		fprintf(stderr, "Multiple customers found for %s\n", errstr);
		exit(MULTIPLE_CUSTOMERS);
	} else if (error == SERVER_BUILD_NOT_FOUND) {
		fprintf(stderr, "Build for server id %s not found\n", errstr);
		exit(SERVER_BUILD_NOT_FOUND);
	} else if (error == MULTIPLE_SERVER_BUILDS) {
		fprintf(stderr, "Multiple builds found for server id %s\n", errstr);
		exit(MULTIPLE_SERVER_BUILDS);
	} else if (error == SERVER_PART_NOT_FOUND) {
		fprintf(stderr, "No partition information for server id %s\n", errstr);
		exit(SERVER_PART_NOT_FOUND);
	} else if (error == OS_NOT_FOUND) {
		fprintf(stderr, "No build Operating Systems were found\n");
		exit(OS_NOT_FOUND);
	} else if (error == NO_PARTITION_SCHEMES) {
		fprintf(stderr, "No partition schemes were found\n");
		exit(NO_PARTITION_SCHEMES);
	} else if (error == NO_VM_HOSTS) {
		fprintf(stderr, "No VM server hosts were found\n");
		exit(NO_VM_HOSTS);
	} else if (error == NO_CUSTOMERS) {
		fprintf(stderr, "No customers were found\n");
		exit(NO_CUSTOMERS);
	} else if (error == NO_HARDWARE_TYPES) {
		fprintf(stderr, "No Hardware types were found\n");
		exit(NO_HARDWARE_TYPES);
	} else if (error == BUILD_DOMAIN_NOT_FOUND) {
		fprintf(stderr, "No build domains found\n");
		exit(BUILD_DOMAIN_NOT_FOUND);
	} else if (error == CREATE_BUILD_FAILED) {
		fprintf(stderr, "Create build config failed with error: %s\n", errstr);
		exit(CREATE_BUILD_FAILED);
	} else if (error == ID_INVALID) {
		fprintf(stderr, "ID Invalid\n");
		exit(ID_INVALID);
	} else if (error == NAME_INVALID) {
		fprintf(stderr, "Name %s invalid\n", errstr);
		exit(NAME_INVALID);
	} else if (error == NO_LOCALE_FOR_OS) {
		fprintf(stderr, "No locale for OS %s\n", errstr);
		exit(NO_LOCALE_FOR_OS);
	} else if (error == VARIENT_NOT_FOUND) {
		fprintf(stderr, "No varient %s found\n", errstr);
		exit(VARIENT_NOT_FOUND);
	} else if (error == MULTIPLE_VARIENTS) {
		fprintf(stderr, "Multiple varients found for %s\n", errstr);
		exit(MULTIPLE_VARIENTS);
	} else {
		fprintf(stderr, "Unknown error code %d\n%s\n", error, errstr);
		exit(error);
	}
}

void
display_cmdb_command_line_error(int retval, char *program)
{
	if (strrchr(program, '/')) {
		program = strrchr(program, '/');
		program++;
	}
	if (retval == NO_NAME)
		fprintf(stderr, "No name specified with -n\n");
	else if (retval == NO_ID)
		fprintf(stderr, "No ID specified with -i\n");
	else if (retval == NO_TYPE)
		fprintf(stderr, "No type specified on command line\n");
	else if (retval == NO_ACTION)
		fprintf(stderr, "No action specified on command line\n");
	else if (retval == NO_NAME_OR_ID)
		fprintf(stderr, "No name or ID specified on command line\n");
	else if (retval == GENERIC_ERROR)
		fprintf(stderr, "Unknown command line option\n");
	else if (retval == NO_DOMAIN_NAME)
		fprintf(stderr, "No domain specified on command line\n");
	else if (retval == NO_IP_ADDRESS)
		fprintf(stderr, "No IP address specified on command line\n");
	else if (retval == NO_HOST_NAME)
		fprintf(stderr, "No hostname specified on command line\n");
	else if (retval == NO_RECORD_TYPE) 
		fprintf(stderr, "No record type specified on command line\n");
	else if (retval == NO_UUID)
		fprintf(stderr, "No UUID specified on command line\n");
	else if (retval == NO_MAKE)
		fprintf(stderr, "No make specified on command line\n");
	else if (retval == NO_MODEL)
		fprintf(stderr, "No model specified on command line\n");
	else if (retval == NO_VENDOR)
		fprintf(stderr, "No vendor specified on command line\n");
	else if (retval == NO_ADDRESS)
		fprintf(stderr, "No address specified on command line\n");
	else if (retval == NO_CITY)
		fprintf(stderr, "No city specified on command line\n");
	else if (retval == NO_COUNTY)
		fprintf(stderr, "No county specified on command line\n");
	else if (retval == NO_POSTCODE)
		fprintf(stderr, "No postcode specified on command line\n");
	else if (retval == NO_COID)
		fprintf(stderr, "No coid specified on command line\n");
	else if (retval == NO_NAME_COID)
		fprintf(stderr, "No server name or customer coid was specified\n");
	else if (retval == NO_CONT_NAME)
		fprintf(stderr, "No name specified with -N\n");
	else if (retval == NO_PHONE)
		fprintf(stderr, "No phone no. specified with -P\n");
	else if (retval == NO_EMAIL)
		fprintf(stderr, "No email address specified with -E\n");
	else if (retval == DOMAIN_AND_IP_GIVEN)
		fprintf(stderr, "Both domain name and IP given on command line\n");
	else if (retval == DISPLAY_USAGE) {
		if ((strncmp(program, "cmdb", CONF_S) == 0))
			display_cmdb_usage();
		else if ((strncmp(program, "cbc", CONF_S) == 0))
			display_cbc_usage();
		else if ((strncmp(program, "dnsa", CONF_S) ==0))
			display_dnsa_usage();
		exit(retval);
	} else
		fprintf(stderr, "Unknown error code %d!\n", retval);
	if ((strncmp(program, "cmdb", CONF_S) == 0))
		printf("Usage: run %s on its own or check man pages\n",
	       program);
	else if ((strncmp(program, "cbc", CONF_S) == 0))
		printf("Usage: %s [-w | -d ] [-p | -k ] [-n <name> | -u <uuid> | -i <id> ]\n",
	       program);
	else if ((strncmp(program, "dnsa", CONF_S) ==0))
		printf("Usage: %s [-d | -w | -c | -l | -z | -a] [-f | -r] -n \
<domain/netrange> -i <IP address> -h <hostname> -t <record type>\n",
	       program);
	exit (retval);
}

void
display_cmdb_usage(void)
{
	printf("CMDB: Configuration Management Database\n");
	printf("Action options:\n");
	printf("-d: display\n-l: list\n-a: add\n");
	printf("Type options:\n");
	printf("-s: server\n-c: customer\n-t: contact\n");
	printf("-e: services\n-h: hardware\n-v: virtual machine hosts\n");
	printf("Name options:\n");
	printf("-n: name\n-i: uuid for server or coid for customer\n");
	printf("-m: vmhost server name for adding a server\n");
	printf("Adding options:\n");
	printf("For server (with -s; need to add -n for name and -m for vm_host (if required))\n");
	printf("-V: Vendor\t-M: Make\t-O: Model\t-U: UUID\t-C: COID\n");
	printf("For customer (with -c; need -n for name)\n");
	printf("-A: Address\t-T: City\t-Y: County\t-Z: Postcode\t-C: COID\n");
	printf("For services (with -i COID for customer, -n name for server)\n");
	printf("-D: Detail\t-L: URL\t\t[ -I service_id | -S service ]\n");
	printf("For hardware (with -n name to specify server)\n");
	printf("-D: Detail\t-B: Device\t-I: hardware_id\n");
	printf("For Contact (with -i coid to specify customer)\n");
	printf("-N: Name\t-P: Phone\t-E: email\n");
}

void
display_cbc_usage(void)
{
	printf("cbc: Create Build Configuration\n\n");
	printf("Action options:\n");
	printf("-w: write build files\n-d: display build details\n");
	printf("-m: Modify build options\n-c: create build in database\n\n");
	printf("Display and write ptions:\n");
	printf("cbc [ -d | -w ] [ -n | -i |  -u ] <server specifier>\n\n");
/*	printf("Add, display and create options:\n");
	printf("-p: partition\n-o: OS\n-v: OS version\n-t: arch\n");
	printf("-b: build domain\n-l: locale\n-x: varient\n-g: packages\n\n");
	printf("Name options:\n");
	printf("-n: name\n-u: uuid for server\n-i: server_id\n");
	printf("\nWrite options:\n");
	printf("Just specify a server\n");
	printf("cbc -w [-n | -i | -u ] <server_specifier>\n\n");
	printf("Add Options:\n");
	printf("Specify which build option you would like to add\n");
	printf("cbc -a [-p | -o | -v | -b | -x (-g) | -l ]\n\n");
	printf("Display Options:\n");
	printf("One option from at least one group must be provided\n");
	printf("You can also present one option from both groups\n");
	printf("cbc -d [-p | -o | -b | -x (-g) | -l ] [-n | -i | -u ] ");
	printf("[<server_specifier>]\n\n"); */
	printf("Create and modify options:\n");
	printf("Use the Display to get these names\n");
	printf("cbc -c -p<scheme> -o<OS> -v<version> -b<domain> -x");
	printf("<varient> -l<locale_id> -a<arch> [-n | -i | -u ] ");
	printf("<server_specifier>\n\n");
}

void
display_dnsa_usage(void)
{
	printf("dnsa: Domain Name System Administratiom\n\n");
	printf("Action options (and needed options)\n");
	printf("-s: show zone\n\t[-f|-r] -n\n-l: list zones\n\t[-f|-r]\n");
	printf("-c: commit valid zones on nameserver\n\t[-f|-r]\n");
	printf("-z: add zone\n\t[-f|-r] -n (for reverse zone -p)\n");
	printf("-a: add host record\n\t-t -h -i -n (-p)\n");
	printf("-b: build reverse zone\n\t-n\n");
	printf("-m: display IP's with multiple A records\n\t-n\n");
	printf("-e: Add preferred A record for reverse DNS");
	printf("\n\t-h -n -i\n");
	printf("-d: Delete record\n\t-h -n\n");
	printf("-g: Delete preferred A record\n\t -i\n");
	printf("-x: Delete zone [-f|-r] -n\n\n");
	printf("Zone type:\n");
	printf("-f: forward zone\n-r: reverse zone\n\n");
	printf("Name options:\n");
	printf("-n: zone-name / network range\n");
	printf("-i: IP Address\n\n");
	printf("Zone options for use with adding a reverse zone:\n");
	printf("-p: prefix\n\n");
	printf("Host options for use with adding a host record:\n");
	printf("-i: IP Address\n-t: record type (A, MX etc)\n-h: host");
	printf("\n-p: priority (for MX record)\n\n");
}

void
get_error_string(int error, char *errstr)
{
	if (error == SERVER_NOT_FOUND)
		snprintf(errstr, MAC_S, "Server not found");
	else if (error == SERVER_UUID_NOT_FOUND)
		snprintf(errstr, MAC_S, "Server not found");
	else if (error ==SERVER_ID_NOT_FOUND)
		snprintf(errstr, MAC_S, "Server not found");
	else if (error == NO_NAME_UUID_ID)
		snprintf(errstr, MAC_S, "No server specifier");
	else if (error == BUILD_DOMAIN_NOT_FOUND)
		snprintf(errstr, MAC_S, "Build domain not found");
	else if (error == NO_BUILD_IP) 
		snprintf(errstr, MAC_S, "No IP's left in build domain");
	else if (error == BUILD_IP_IN_USE)
		snprintf(errstr, MAC_S, "Build IP already in use");
	else if (error == CANNOT_INSERT_IP)
		snprintf(errstr, MAC_S, "Cannot insert build IP into DB");
	else if (error == SERVER_BUILD_NOT_FOUND)
		snprintf(errstr, MAC_S, "No server build");
	else
		snprintf(errstr, MAC_S, "Unknown error %d", error);
}

void
chomp(char *input)
{
	size_t len;
	len = strlen(input);
	if (input[len -1] == '\n')
		input[len -1] = '\0';
}

void
display_action_error(short int action)
{
	if (action == NONE)
		fprintf(stderr, "No action specified\n");
	else if (action == DISPLAY)
		fprintf(stderr, "Display failed\n");
	else if (action == LIST_OBJ)
		fprintf(stderr, "Listing failed\n");
	else if (action == ADD_TO_DB)
		fprintf(stderr, "Adding to DB failed\n");
	else
		fprintf(stderr, "Unknown error code %d failed\n", action);
}

void 
display_type_error(short int type)
{
	char *message;
	
	if (!(message = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "message in display_type_error");
	snprintf(message, HOST_S, "\
Unable to perform requested action on ");
	if (type == SERVER) {
		strncat(message, "server\n", MAC_S);
		fprintf(stderr, "%s", message);
	} else if (type == CUSTOMER) {
		strncat(message, "customer\n", MAC_S);
		fprintf(stderr, "%s", message);
	} else if (type == CONTACT) {
		strncat(message, "contact\n", MAC_S);
		fprintf(stderr, "%s", message);
	} else if (type == SERVICE) {
		strncat(message, "service\n", MAC_S);
		fprintf(stderr, "%s", message);
	} else {
		strncat(message, "unknown type ", MAC_S);
		fprintf(stderr, "%s", message);
		fprintf(stderr, "%d\n", type);
	}
}

int
add_trailing_slash(char *member)
{
	size_t len;
	int retval;
	
	retval = 0;
	len = strlen(member);
	if ((member[len - 1] != '/') && len < CONF_S) {
		member[len] = '/';
		member[len + 1] = '\0';
	} else if ((member[len - 1] == '/')) {
		retval = NONE;
	} else {
		retval = -1;
	}
	
	return retval;
}

int
add_trailing_dot(char *member)
{
	size_t len;
	int retval;
	
	retval = 0;
	len = strlen(member);
	if (member[len - 1] != '.') {
		member[len] = '.';
		member[len +1] = '\0';
	} else if (member[len - 1] == '.') {
		retval = NONE;
	} else {
		retval = -1;
	}
	return retval;
}

int
write_file(char *filename, char *output)
{
	int retval;
	FILE *zonefile;
	if (!(zonefile = fopen(filename, "w"))) {
		retval = FILE_O_FAIL;
	} else {
		fputs(output, zonefile);
		fclose(zonefile);
		retval = NONE;
	}
	return retval;
}
