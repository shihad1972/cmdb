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
#include "base_sql.h"

void
report_error(int error, const char *errstr)
{

	if (error == ARGC_INVAL) {
		fprintf(stderr, "Argc is invalid\n");
	} else if (error == ARGV_INVAL) {
		fprintf(stderr, "Argv is invalid\n");
	} else if (error == NO_DOMAIN) {
		fprintf(stderr, "No domain %s was found\n", errstr);
	} else if (error == MULTI_DOMAIN) {
		fprintf(stderr, "Multiple records found for %s\n", errstr);
	} else if (error == NO_DELIM) {
		fprintf(stderr, "No delimiter %s found in string\n", errstr);
	} else if (error == NO_RECORDS) {
		fprintf(stderr, "No records found for the zone %s\n", errstr);
	} else if (error == NO_FORWARD_RECORDS) {
		fprintf(stderr, "No records found for forward zone %s\n", errstr);
	} else if (error == WRONG_ACTION) {
		fprintf(stderr, "Incorrect action specified\n");
	} else if (error == WRONG_TYPE) {
		fprintf(stderr, "Incorrect domain type specified\n");
	} else if (error == DOMAIN_LIST_FAIL) {
		fprintf(stderr, "No domains were found to list from the database\n");
	} else if (error == MY_INIT_FAIL) {
		fprintf(stderr, "DB initialisation failed with %s\n", errstr);
	} else if (error == MY_CONN_FAIL) {
		fprintf(stderr, "Unable to connect to database: %s\n", errstr);
	} else if (error == MY_QUERY_FAIL) {
		fprintf(stderr, "Query to database failed with error %s\n", errstr);
	} else if (error == MY_STORE_FAIL) {
		fprintf(stderr, "Unable to store DB result set: %s\n", errstr);
	} else if (error == MY_INSERT_FAIL) {
		fprintf(stderr, "Unable to insert into DB:\n%s\n", errstr);
	} else if (error == MY_STATEMENT_FAIL) {
		fprintf(stderr, "DB statment failed with %s\n", errstr);
	} else if (error == MY_BIND_FAIL) {
		fprintf(stderr, "DB bind of prepared statement failed with %s\n", errstr);
	} else if (error == FILE_O_FAIL) {
		fprintf(stderr, "Unable to open file %s\n", errstr);
	} else if (error == CHKZONE_FAIL) {
		fprintf(stderr, "Checking the zone %s failed\n", errstr);
	} else if (error == NO_ZONE_CONFIGURATION) {
		fprintf(stderr, "There are no dnsa configuration values in the database\n");
	} else if (error == CANNOT_INSERT_ZONE) {
		fprintf(stderr, "Unable to add zone %s to database", errstr);
	} else if (error == CANNOT_INSERT_RECORD) {
		fprintf(stderr, "Unable to add record %s to database", errstr);
	} else if (error == MALLOC_FAIL) {
		fprintf(stderr, "Malloc / Calloc failed for %s\n", errstr);
	} else if (error == SERVER_NOT_FOUND) {
		fprintf(stderr, "Server %s not found in database\n", errstr);
	} else if (error == MULTIPLE_SERVERS) {
		fprintf(stderr, "Multiple servers with name %s found in database\n", errstr);
	} else if (error == SERVER_ID_NOT_FOUND) {
		fprintf(stderr, "Server with id %s not found in database\n", errstr);
	} else if (error == MULTIPLE_SERVER_IDS) {
		fprintf(stderr, "Multiple servers with id %s in database\n\
		THIS SHOULD NOT HAPPEN. Fix the DB!!\n", errstr);
	} else if (error == SERVER_UUID_NOT_FOUND) {
		fprintf(stderr, "Server with uuid %s not found in database\n", errstr);
	} else if (error == MULTIPLE_SERVER_UUIDS) {
		fprintf(stderr, "Multiple servers with uuid %s in database\n\
		THIS SHOULD NOT HAPPEN. Check your database!\n", errstr);
	} else if (error == CUSTOMER_NOT_FOUND) {
		fprintf(stderr, "Customer %s not found\n", errstr);
	} else if (error == MULTIPLE_CUSTOMERS) {
		fprintf(stderr, "Multiple customers found for %s\n", errstr);
	} else if (error == SERVER_BUILD_NOT_FOUND) {
		fprintf(stderr, "Build for server id %s not found\n", errstr);
	} else if (error == MULTIPLE_SERVER_BUILDS) {
		fprintf(stderr, "Multiple builds found for server id %s\n", errstr);
	} else if (error == SERVER_PART_NOT_FOUND) {
		fprintf(stderr, "No partition information for server id %s\n", errstr);
	} else if (error == OS_NOT_FOUND) {
		fprintf(stderr, "Build Operating System %s was not found\n", errstr);
	} else if (error == NO_PARTITION_SCHEMES) {
		fprintf(stderr, "No partition schemes were found\n");
	} else if (error == NO_VM_HOSTS) {
		fprintf(stderr, "No VM server hosts were found\n");
	} else if (error == NO_CUSTOMERS) {
		fprintf(stderr, "No customers were found\n");
	} else if (error == NO_HARDWARE_TYPES) {
		fprintf(stderr, "No Hardware types were found\n");
	} else if (error == BUILD_DOMAIN_NOT_FOUND) {
		fprintf(stderr, "No build domains found\n");
	} else if (error == CREATE_BUILD_FAILED) {
		fprintf(stderr, "Create build config failed with error: %s\n", errstr);
	} else if (error == ID_INVALID) {
		fprintf(stderr, "ID Invalid\n");
	} else if (error == NAME_INVALID) {
		fprintf(stderr, "Name %s invalid\n", errstr);
	} else if (error == NO_LOCALE_FOR_OS) {
		fprintf(stderr, "No locale for OS %s\n", errstr);
	} else if (error == VARIENT_NOT_FOUND) {
		fprintf(stderr, "No varient %s found\n", errstr);
	} else if (error == MULTIPLE_VARIENTS) {
		fprintf(stderr, "Multiple varients found for %s\n", errstr);
	} else if (error == FIELDS_MISMATCH) {
		fprintf(stderr, "Query fields mismatch for %s\n", errstr);
	} else if (error == BUILD_OS_EXISTS) {
		fprintf(stderr, "Build OS %s already exists\n", errstr);
	} else if (error == NO_BUILD_PACKAGES) {
		fprintf(stderr, "No build packages in database\n");
	} else if (error == VARIENT_EXISTS) {
		fprintf(stderr, "Varient already exists in the database\n");
	} else if (error == OS_ALIAS_NEEDED) {
		fprintf(stderr, "Build os %s needs a version alias\n", errstr);
	} else if (error == BUILD_OS_IN_USE) {
		fprintf(stderr,
"Cowardly refusal to delete build os %s\n", errstr);
	} else {
		fprintf(stderr, "Unknown error code %d\n%s\n", error, errstr);
	}
	exit(error);
}

void
display_cmdb_command_line_error(int retval, char *program)
{
	if (strrchr(program, '/')) {
		program = strrchr(program, '/');
		program++;
	}
	if (retval == NO_NAME)
		fprintf(stderr, "No name specified with -n.\n");
	else if (retval == NO_ID)
		fprintf(stderr, "No ID specified with -i.\n");
	else if (retval == NO_TYPE)
		fprintf(stderr, "No type specified on command line.\n");
	else if (retval == NO_ACTION)
		fprintf(stderr, "No action specified on command line.\n");
	else if (retval == NO_NAME_OR_ID)
		fprintf(stderr, "No name or ID specified on command line.\n");
	else if (retval == GENERIC_ERROR)
		fprintf(stderr, "Unknown command line option.\n");
	else if (retval == NO_DOMAIN_NAME)
		fprintf(stderr, "No domain specified on command line.\n");
	else if (retval == NO_IP_ADDRESS)
		fprintf(stderr, "No IP address specified on command line.\n");
	else if (retval == NO_HOST_NAME)
		fprintf(stderr, "No hostname specified on command line.\n");
	else if (retval == NO_RECORD_TYPE) 
		fprintf(stderr, "No record type specified on command line.\n");
	else if (retval == NO_UUID)
		fprintf(stderr, "No UUID specified on command line.\n");
	else if (retval == NO_MAKE)
		fprintf(stderr, "No make specified on command line.\n");
	else if (retval == NO_MODEL)
		fprintf(stderr, "No model specified on command line.\n");
	else if (retval == NO_VENDOR)
		fprintf(stderr, "No vendor specified on command line.\n");
	else if (retval == NO_ADDRESS)
		fprintf(stderr, "No address specified on command line.\n");
	else if (retval == NO_CITY)
		fprintf(stderr, "No city specified on command line.\n");
	else if (retval == NO_COUNTY)
		fprintf(stderr, "No county specified on command line.\n");
	else if (retval == NO_POSTCODE)
		fprintf(stderr, "No postcode specified on command line.\n");
	else if (retval == NO_COID)
		fprintf(stderr, "No coid specified on command line.\n");
	else if (retval == NO_NAME_COID)
		fprintf(stderr, "No server name or customer coid was specified.\n");
	else if (retval == NO_CONT_NAME)
		fprintf(stderr, "No name specified with -N.\n");
	else if (retval == NO_PHONE)
		fprintf(stderr, "No phone no. specified with -P.\n");
	else if (retval == NO_PACKAGE)
		fprintf(stderr, "No package supplied.\n");
	else if (retval == NO_OS_COMM)
		fprintf(stderr, "No os or alias supplied.\n");
	else if (retval == NO_VARIENT)
		fprintf(stderr, "No varient or valias supplied.\n");
	else if (retval == NO_EMAIL)
		fprintf(stderr, "No email address specified with -E.\n");
	else if (retval == DOMAIN_AND_IP_GIVEN)
		fprintf(stderr, "Both domain name and IP given on command line.\n");
	else if (retval == NO_PARTITION_INFO)
		fprintf(stderr, "No partition information on command line.\n");
	else if (retval == NO_SCHEME_INFO)
		fprintf(stderr, "No scheme name was supplied on the command line.\n");
	else if ((retval == USER_INPUT_INVALID) &&
		 (strncmp(program, "cbcdomain", RANGE_S)) == 0)
		fprintf(stderr, "Check your network input please. It seems wrong!\n");
	else if (retval == DISPLAY_USAGE) {
		if ((strncmp(program, "cmdb", CONF_S) == 0))
			display_cmdb_usage();
		else if ((strncmp(program, "cbc", CONF_S) == 0))
			display_cbc_usage();
		else if ((strncmp(program, "dnsa", CONF_S) ==0))
			display_dnsa_usage();
		else if ((strncmp(program, "cbcdomain", CONF_S) == 0))
			display_cbcdomain_usage();
		else if ((strncmp(program, "cbcos", CONF_S) == 0))
			display_cbcos_usage();
		else if ((strncmp(program, "cbcvarient", CONF_S) == 0))
			display_cbcvarient_usage();
		else if ((strncmp(program, "cbcpack", CONF_S) == 0))
			display_cbcpack_usage();
		else if ((strncmp(program, "cbcpart", CONF_S) == 0))
			display_cbcpart_usage();
		exit(retval);
	} else
		fprintf(stderr, "Unknown error code %d!\n", retval);
	if ((strncmp(program, "cmdb", CONF_S) == 0))
		printf("Usage: run %s on its own or check man pages\n",
	       program);
	else if ((strncmp(program, "cbc", CONF_S) == 0))
		printf("Usage: %s [ -w | -d ] [ -p | -k ] [-n <name> | -u <uuid> | -i <id> ]\n",
	       program);
	else if ((strncmp(program, "dnsa", CONF_S) ==0))
		printf("Usage: %s [ -d | -w | -c | -l | -z | -a ] [ -f | -r ] -n \
<domain/netrange> -i <IP address> -h <hostname> -t <record type>\n",
	       program);
	else if ((strncmp(program, "cbcdomain", CONF_S) == 0))
		printf("Usage: %s [ -a | -d | -l | -m | -r ] -n <domain name> \
[ OPTIONS ]\n", program);
	else if ((strncmp(program, "cbcos", CONF_S) == 0))
		printf("Usage: %s [ -a | -d | -l | -r ] -n <os name> \
[ OPTIONS ]\n", program);
	else if ((strncmp(program, "cbcvarient", CONF_S) == 0))
		printf("Usage: %s [ -a | -d | -l | -r ] [ -x <varient> \
-k valias] [ OPTIONS ]\n", program);
	else if ((strncmp(program, "cbcpack", CONF_S) == 0))
		printf("Usage: %s [ -a | -r ] [ -x | -k ] [ -n | -s ] \
[ -o | -e ] [ -t ] -p <package name>\n", program);
	else if ((strncmp(program, "cbcpart", CONF_S) == 0))
		printf("Usage: %s [ -a | -d | -l | -r ] [ -p | -s ] [ -v ] \
[ -t partition-info ]\n", program);
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
	printf("-l: list servers with a build\n-d: display build details\n");
	printf("-m: modify build options\n-a: add build for server\n");
	printf("-r: remove build for server\n-w: write build files\n\n");
	printf("Display, remove and write options:\n");
	printf("cbc [ -d | -w | -r ] [ -n | -i |  -u ] <server specifier>\n\n");
	printf("Create and modify options:\n");
	printf("cbc [ -a | -m ] -o<OS> -v<version> -t<arch> -b<domain> -x");
	printf("<varient> -e<locale_id>\n -p<scheme> -k<network device> ");
	printf("[-n | -i | -u ] ");
	printf("<server_specifier>\n\n");
	printf("The various associated programs will give you the names ");
	printf("for these options.\n\n");
	printf("cbcos cbcdomain cbcvarient cbcpack cbcpart cbclocale\n");
	
}

void
display_cbcdomain_usage(void)
{
	printf("cbcdomain: Program to manipulate build domains\n\n");
	printf("Action Options:\n");
	printf("-a: add build domain\n-d: display build domain details\n");
	printf("-l: list build domain names\n-m: modify build domain\n");
	printf("-r: remove build domain\n");
	printf("All actions apart from -l need -n <domain name>\n\n");
	printf("Detail Options:\n");
	printf("LDAP:\n\t-b <basedn>\n\t-i <binddn>\n\t-s <ldapserver>");
	printf("\n\t-p use ssl for ldap connection\n\n");
	printf("Network Details:\n");
	printf("-k: start_ip,end_ip,gateway,netmask,nameserver\n\n");
	printf("Application server configurations\n");
	printf("-e smtp_server\n-f nfs_domain\n-t ntp_server\n-x xymon_server\n\n");
	printf("cbcdomain [ -a | -d | -l | -m | -r ] -n [ app options ]\n\n");
}

void
display_cbcos_usage(void)
{
	printf("cbcos: Program to manipulate build operating systems\n\n");
	printf("Action Options:\n");
	printf("-a: add OS\n-d: display OS\n-l: list OS\n-r: remove OS\n");
	printf("All actions apart from -l need -n <OS name>\n\n");
	printf("Detail Options:\n");
	printf("-e: <version alias>\n-o: <os version>\n");
	printf("-s: alias\n-t: <os architecture\n\n");
	printf("cbcos [ -a | -d | -l | -r ] -n [ detail options ]\n\n");
}

void
display_cbcvarient_usage(void)
{
	printf("cbcvarient: Program to manipulate build varients\n\n");
	printf("Action Options:\n");
	printf("-a: add varient\n-d: display varient\n-l: list varients\n");
	printf("-r: remove varient\n");
	printf("-d and -r actions need -x <varient name> or -k <valias>\n");
	printf("-a will need both -x <varient name> and -k <valias>\n\n");
	printf("Name Options:\n");
	printf("-x: <varient>\n-k: <valias>\n\n");
	printf("Detail Options:\n");
	printf("-n: <os name>\n-e: <version alias>\n-o: <os version>\n");
	printf("-s: <os alias>\n-t: <os architecture\n\n");
	printf("cbcvarient [ -a | -d | -l | -r ]  [ -x | -k ] [ detail options ]\n\n");
}

void
display_cbcpart_usage(void)
{
	printf("cbcpart: Program to manipulate build partitions & schemes\n\n");
	printf("Action Options:\n");
	printf("-a: add scheme / partition\n-d: display scheme\n");
	printf("-l: list schemes\n-r: remove scheme / partition\n\n");
	printf("Definition Options:\n");
	printf("-p: partition\n-s: scheme\n\n");
	printf("Detail Options\n");
	printf("-v: Use lvm (when adding a partition)\n");
	printf("-g: logical volume (if using lvm)\n");
	printf("-n: <scheme name>\n\n");
	printf("Partition Details:\n");
	printf("-t: min size,max size,priority,mount point,filesystem\n\n");
	printf("cbcpart: [ -a | -d | -l | -r ] [ -p | -s ] [ -v ] [ -g \
log vol ] [ -t <part def>]\n");
}

void
display_cbcpack_usage(void)
{
	printf("cbcpack: Program to manipulate build packages\n\n");
	printf("Action Options:\n");
	printf("-a: add package\n-r: remove package\n\n");
	printf("Varient Options:\n");
	printf("-x: <varient>\n-k: <valias>\n\n");
	printf("OS options:\n");;
	printf("-n: <os name>\n-s: <os alias>\n-o: <os version>\n");
	printf("-e: <version alias>\n-t: <os architecture\n\n");
	printf("Package Options:\n");
	printf("-p: <package name>\n\n");
	printf("Need at least one OS name or alias option and package name\n");
	printf("OS version / version alias, architecture and varient are optional\n");
	printf("Not including these will add the package to all of them\n\n");
	printf("cbcpack [ -a | -r ]  [ -x | -k ]  [ -n | -s ]  [ -o | -e ]  \
[ -t ] -p <package name>\n");
	
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
	else if (error == SERVER_ID_NOT_FOUND)
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
	else if (error == NO_DHCP_B_ERR)
		snprintf(errstr, MAC_S, "Cannot find dhcp details");
	else if (error == MULTI_DHCP_B_ERR)
		snprintf(errstr, MAC_S, "Multiple dhcp details");
	else if (error == NO_TFTP_B_ERR)
		snprintf(errstr, MAC_S, "Cannot find TFTP details");
	else if (error == MULTI_TFTP_B_ERR)
		snprintf(errstr, MAC_S, "Multiple TFTP details");
	else if (error == NO_NET_BUILD_ERR)
		snprintf(errstr, MAC_S, "Cannot find NET_BUILD_DETAILS");
	else if (error == MULTI_NET_BUILD_ERR)
		snprintf(errstr, MAC_S, "Multiple NET_BUILD_DETAILS");
	else if (error == NO_BUILD_MIRR_ERR)
		snprintf(errstr, MAC_S, "Cannot find BUILD_MIRROR");
	else if (error == MULTI_BUILD_MIRR_ERR)
		snprintf(errstr, MAC_S, "Multiple BUILD_MIRROR");
	else if (error == VARIENT_NOT_FOUND)
		snprintf(errstr, MAC_S, "Unknown build varient");
	else if (error == NO_NETWORK_HARDWARE)
		snprintf(errstr, MAC_S, "Network device not found");
	else if (error == OS_NOT_FOUND)
		snprintf(errstr, MAC_S, "OS not found");
	else if (error == SCHEME_NOT_FOUND)
		snprintf(errstr, MAC_S, "Partition scheme not found");
	else if (error == INSERT_NOT_CONFIGURED)
		snprintf(errstr, MAC_S, "No Database insert function");
	else if (error == CANNOT_FIND_BUILD_IP)
		snprintf(errstr, MAC_S, "Cannot find the build IP!");
	else
		snprintf(errstr, MAC_S, "Unknown error %d", error);
}

void
cbc_query_mismatch(unsigned int fields, unsigned int required, int query)
{
	char errstr[HOST_S];
	char *err = errstr;
	if (query == BOOT_LINE)
		snprintf(err, HOST_S, "boot_line query: required %d, got %d\n",
			required, fields);
	else if (query == BUILD)
		snprintf(err, HOST_S, "build query: required %d, got %d\n",
			required, fields);
	else if (query == BUILD_DOMAIN)
		snprintf(err, HOST_S, "build domain query: required %d; got %d\n",
			 required, fields);
	else if (query == BUILD_IP)
		snprintf(err, HOST_S, "build ip query: required %d; got %d\n",
			 required, fields);
	else if (query == BUILD_OS)
		snprintf(err, HOST_S, "build os query: required %d, got %d\n",
			 required, fields);
	else if (query == BUILD_TYPE)
		snprintf(err, HOST_S, "build type query: required %d, got %d\n",
			 required, fields);
	else if (query == DISK_DEV)
		snprintf(err, HOST_S, "disk dev query: required %d, got %d\n",
			 required, fields);
	else if (query == LOCALE)
		snprintf(err, HOST_S, "locale query: required %d, got %d\n",
			 required, fields);
	else if (query == BPACKAGE)
		snprintf(err, HOST_S, "package query: required %d, got %d\n",
			 required, fields);
	else if (query == DPART)
		snprintf(err, HOST_S, "\
default partition query: required %d, got %d\n", required, fields);
	else if (query == SPART)
		snprintf(err, HOST_S, "\
preseed partition query: required %d, got %d\n", required, fields);
	else if (query == SSCHEME)
		snprintf(err, HOST_S, "\
preseed scheme partition query: required %d, got %d\n", required, fields);
	else if (query == CSERVER)
		snprintf(err, HOST_S, "server query: required %d, got %d\n",
			 required, fields);
	else if (query == VARIENT)
		snprintf(err, HOST_S, "varient query: required %d, got %d\n",
			 required, fields);
	else if (query == VMHOST)
		snprintf(err, HOST_S, "\
virtual machine host query: required %d, got %d\n", required, fields);
	else
		snprintf(err, HOST_S, "\
unknown query type %d: required %d, got %d\n", query, required, fields);
	report_error(FIELDS_MISMATCH, errstr);
}

void
cmdb_query_mismatch(unsigned int fields, unsigned int required, int query)
{
	char errstr[HOST_S];
	char *err = errstr;
	if (query == SERVER)
		snprintf(err, HOST_S, "server query: required %d, got %d\n",
			 required, fields);
	else if (query == CUSTOMER)
		snprintf(err, HOST_S, "customer query: required %d, got %d\n",
			 required, fields);
	else if (query == CONTACT)
		snprintf(err, HOST_S, "contact query: required %d, got %d\n",
			 required, fields);
	else if (query == SERVICE)
		snprintf(err, HOST_S, "service query: required %d, got %d\n",
			 required, fields);
	else if (query == SERVICE_TYPE)
		snprintf(err, HOST_S, "\
service type query: required %d, got %d\n", required, fields);
	else if (query == HARDWARE)
		snprintf(err, HOST_S, "hardware query: required %d, got %d\n",
			 required, fields);
	else if (query == HARDWARE_TYPE)
		snprintf(err, HOST_S, "\
hardware type query: required %d, got %d\n", required, fields);
	else if (query == VM_HOST)
		snprintf(err, HOST_S, "\
virtual machine host query: required %d, got %d\n", required, fields);
	else
		snprintf(err, HOST_S, "\
unknown query type %d: required %d, got %d\n", query, required, fields);
	report_error(FIELDS_MISMATCH, errstr);
}

void
dnsa_query_mismatch(unsigned int fields, unsigned int required, int query)
{
	char errstr[HOST_S];
	char *err = errstr;
	if (query == ZONE)
		snprintf(err, HOST_S, "zone query: required %d, got %d\n",
			 required, fields);
	else if (query == REV_ZONE)
		snprintf(err, HOST_S, "\
reverse zone query: required %d, got %d\n", required, fields);
	else if (query == RECORD)
		snprintf(err, HOST_S, "record query: required %d, got %d\n",
			 required, fields);
	else if (query == REV_RECORD)
		snprintf(err, HOST_S, "\
reverse record query: required %d, got %d\n", required, fields);
	else if (query == ALL_A_RECORD)
		snprintf(err, HOST_S, "\
all A records query: required %d, got %d\n", required, fields);
	else if (query == DUPLICATE_A_RECORD)
		snprintf(err, HOST_S, "\
duplicate A records query: required %d, got %d\n", required, fields);
	else if (query == PREFERRED_A)
		snprintf(err, HOST_S, "\
preferred A records query: required %d, got %d\n", required, fields);
	else if (query == RECORDS_ON_CNAME_TYPE)
		snprintf(err, HOST_S, "\
CNAME records query: required %d, got %d\n", required, fields);
	else
		snprintf(err, HOST_S, "\
unknown query type %d: required %d, got %d\n", query, required, fields);
	report_error(FIELDS_MISMATCH, errstr);
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

void
init_dbdata_struct(dbdata_s *data)
{
	data->fields.number = 0;
	data->args.number = 0;
	data->next = '\0';
}

void
clean_dbdata_struct(dbdata_s *list)
{
	dbdata_s *data, *next;
	
	if (list)
		data = list;
	else
		return;
	if (data->next)
		next = data->next;
	else
		next = '\0';
	while (data) {
		free(data);
		if (next)
			data = next;
		else
			return;
		if (data->next)
			next = data->next;
		else
			next = '\0';
	}
}
