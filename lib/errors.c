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
 *  (C) Iain M Conochie 2012 - 2016
 * 
 */
#include <config.h>
#include <configmake.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include "cmdb.h"
// #include "base_sql.h"

void
report_error(int error, const char *errstr)
{

	if (error == ARGC_INVAL) {
		fprintf(stderr, "Argc is invalid\n");
	} else if (error == ARGV_INVAL) {
		;
	} else if (error == CONF_ERR) {
		fprintf(stderr, "Config file error\n");
	} else if (error == PORT_ERR) {
		fprintf(stderr, "MySQL port number invalid in config file\n");
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
		fprintf(stderr, "Incorrect type specified\n");
	} else if (error == USER_INPUT_INVALID) {
		fprintf(stderr, "Input %s not valid.\n", errstr);
	} else if (error == BUFFER_TOO_SMALL) {
		fprintf(stderr, "Buffer %s too small\n", errstr);
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
	} else if ((error == MY_STATEMENT_FAIL) || (error == SQLITE_STATEMENT_FAILED)) {
		fprintf(stderr, "DB statment failed with %s\n", errstr);
	} else if (error == MY_BIND_FAIL) {
		fprintf(stderr, "DB bind of prepared statement failed with %s\n", errstr);
	} else if (error == SQLITE_BIND_FAILED) {
		fprintf(stderr, "SQLITE bind failed in %s\n", errstr);
	} else if (error == DB_WRONG_TYPE) {
		fprintf(stderr, "Wrong DB type in for query %s\n", errstr);
	} else if (error == UNKNOWN_QUERY) {
		fprintf(stderr, "Unknown query for type %s\n", errstr);
	} else if (error == NO_DB_TYPE) {
		fprintf(stderr, "No DB type configured\n");
	} else if (error == DB_TYPE_INVALID) {
		fprintf(stderr, "DB type %s invalid\n", errstr);
	} else if (error == UNKNOWN_STRUCT_DB_TABLE) {
		fprintf(stderr, "Function %s trying to use an unknown struct / db table\n", errstr);
	} else if (error == CBC_NO_DATA) {
		fprintf(stderr, "Null pointer passed for %s\n", errstr);
	} else if (error == GET_TIME_FAILED) {
		fprintf(stderr, "Call to localtime failed: %s\n", errstr);
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
	} else if (error == NO_SERVICE_URL) {
		fprintf(stderr, "No service type or URL was provided.\n");
	} else if (error == SERVER_NOT_FOUND) {
		fprintf(stderr, "Server %s not found in database\n", errstr);
	} else if (error == MULTIPLE_SERVERS) {
		fprintf(stderr, "Multiple servers with name %s found in database\n", errstr);
	} else if (error == SERVER_ID_NOT_FOUND) {
		fprintf(stderr, "Server with id %s not found in database\n", errstr);
	} else if (error == MULTIPLE_SERVER_IDS) {
		fprintf(stderr, "Multiple servers with id %s in database\n\
		THIS SHOULD NOT HAPPEN. Fix the DB!!\n", errstr);
	} else if (error == NO_MODEL) {
		fprintf(stderr, "Server does not have a model!\n");
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
	} else if (error == OS_NO_VERSION) {
		fprintf(stderr, "Version supplied with no OS\n");
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
	} else if (error == BUILD_DOMAIN_EXISTS) {
		fprintf(stderr, "Build domain %s exists\n", errstr);
	} else if (error == CREATE_BUILD_FAILED) {
		fprintf(stderr, "Create build config failed with error: %s\n", errstr);
	} else if (error == ID_INVALID) {
		fprintf(stderr, "ID Invalid\n");
	} else if (error == UNKNOWN_ZONE_TYPE) {
		fprintf(stderr, "Unknown zone type in %s\n", errstr);
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
	} else if (error == VARIENT_EXISTS) {
		fprintf(stderr, "Varient already exists in the database\n");
	} else if (error == OS_ALIAS_NEEDED) {
		fprintf(stderr, "Build os %s needs a version alias\n", errstr);
	} else if (error == NO_CONTACT_INFO) {
		fprintf(stderr, "Not enough information provided about the contact\n");
	} else if (error == NO_CONTACT) {
		fprintf(stderr, "This is not the contact you were looking for!\n");
	} else if (error == MULTI_CONTACT) {
		fprintf(stderr, "Multiple contacts found for that!\n");
	} else if (error == BUILD_OS_IN_USE) {
		fprintf(stderr,
"Cowardly refusal to delete build os %s\n", errstr);
	} else if (error == DID_NOT_MOD_BUILD_DOMAIN) {
		fprintf(stderr, "cbcdomain modified nothing??\n");
	} else if (error == BDOM_OVERLAP) {
		fprintf(stderr, "build domain %s overlaps with another in the database\n", errstr);
	} else if (error == NO_CONTACT_DATA) {
		fprintf(stderr, "Contact query to database failed\n");
	} else if (error == NOT_PRI_OR_SEC_NS) {
		fprintf(stderr,
"Something other than pri or sec ns passed to %s\n", errstr);
	} else if (error == NO_GLUE_ZONE) {
		fprintf(stderr, "No glue zone passed to %s\n", errstr);
	} else if (error == LOCALE_NOT_FOUND) {
		fprintf(stderr, "No locale for the OS and version. Did you just add it?\n");
	} else if (error == IFACE_LIST_FAILED) {
		fprintf(stderr, "Interface list failed in %s\n", errstr);
	} else if (error == IFACE_FILL) {
		fprintf(stderr, "Cannot get interface info in %s\n", errstr);
	} else if (error == NO_IFACE) {
		fprintf(stderr, "Interface list empty\n");
	} else if (error == NULL_POINTER_PASSED) {
		fprintf(stderr, "Null pointer passed\n");
	} else if (error == DNS_LOOKUP_FAILED) {
		fprintf(stderr, "There was a DNS lookup failure\n");
	} else if (error == NET_FUNC_FAILED) {
		fprintf(stderr, "A network function failed\n");
	} else if (error == BUILD_TYPE_NOT_FOUND) {
		fprintf(stderr, "No build type for that OS\n");
	} else if (error == CANNOT_UPDATE) {
		fprintf(stderr, "Database update not possible\n");
	} else if (error == PARTITON_NOT_FOUND) {
		fprintf(stderr, "Requested partition not found\n");
	} else if (error == DB_DELETE_FAILED) {
		fprintf(stderr, "Delete from database failed\n");
	} else if (error == NO_BD_CONFIG) {
		fprintf(stderr, "Unable to find build domain\n");
	} else if (error == CBC_DATA_WRONG_COUNT) {
		fprintf(stderr, "dbdata count is wrong in %s\n", errstr);
	} else if (error == NO_SYSPACK_CONF) {
		;
	} else if (error == NO_DEVICE_OR_DETAIL) {
		fprintf(stderr, "No device or detail in %s\n", errstr);
	} else if (error == NO_ARG) {
		fprintf(stderr, "Argument does not exist\n");
	} else {
		fprintf(stderr, "Unknown error code %d in %s\n", error, errstr);
	}
	exit(error);
}

void
display_command_line_error(int retval, char *program)
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
	else if (retval == WRONG_ACTION)
		fprintf(stderr, "Unable to complete this action.\n");
	else if (retval == NO_NAME_OR_ID)
		fprintf(stderr, "No name or ID specified on command line.\n");
	else if (retval == GENERIC_ERROR)
		fprintf(stderr, "Unknown command line option.\n");
	else if (retval == NO_SERVICE)
		fprintf(stderr, "No service provided on command line\n");
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
	else if (retval == CBC_NO_ADDRESS)
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
	else if (retval == NO_SERVICE_URL)
		fprintf(stderr, "A service name or URL was not specified\n\
If you wish to remove all services (for a server or customer) add the -f option\n");
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
	else if (retval == NO_OS_SPECIFIED)
		fprintf(stderr, "No OS or not enough OS options supplied on command line.\n");
	else if (retval == NO_BUILD_DOMAIN)
		fprintf(stderr, "No Build Domain supplied on command line\n");
	else if (retval == NO_BUILD_VARIENT)
		fprintf(stderr, "No Build Varient supplied on command line\n");
	else if (retval == NO_BUILD_PARTITION)
		fprintf(stderr, "No Build Partition Scheme supplied on command line\n");
	else if (retval == NO_MOD_BUILD_DOM_NET)
		fprintf(stderr, "Cowardly refusal to modify network settings for build domain\n");
	else if (retval == MULTI_BUILD_DOM_APP_MOD)
		fprintf(stderr, "Cowardly refusal to modify multiple application settings\n");
	else if (retval == NO_MASTER)
		fprintf(stderr, "Slave zone specified but no master IP given\n");
	else if (retval == NO_MASTER_NAME)
		fprintf(stderr, "Slave zone specified but no master name given\n");
	else if (retval == NO_PREFIX)
		fprintf(stderr, "No reverse zone prefix was supplied\n");
	else if (retval == NO_ARG)
		fprintf(stderr, "No arguemt was supplied\n");
	else if (retval == NO_NUMBER)
		fprintf(stderr, "No number was supplied\n");
	else if (retval == NO_NTP_SERVER)
		fprintf(stderr, "No ntp server was supplied\n");
	else if ((retval == USER_INPUT_INVALID) &&
		 (strncmp(program, "cbcdomain", RANGE_S)) == 0)
		fprintf(stderr, "Check your network input please. It seems wrong!\n");
	else if (retval == NO_FILE_SYSTEM)
		fprintf(stderr, "No file system type was supplied\n");
	else if (retval == NO_LOG_VOL)
		fprintf(stderr, "No logical volume name supplied\n");
	else if (retval == NO_OPTION)
		fprintf(stderr, "Partition option specified but no option supplied\n");
	else if (retval == CVERSION)
		fprintf(stderr, "%s: %s\n", program, VERSION);
	else if (retval == DISPLAY_USAGE) {
		if ((strncmp(program, "cmdb", CONF_S) == 0) || (strncmp(program, "cmdb2", CONF_S) == 0))
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
		else if ((strncmp(program, "cbcpart", CONF_S) == 0))
			display_cbcpart_usage();
		else if ((strncmp(program, "cpc", CONF_S) == 0))
			display_cpc_usage();
		else if ((strncmp(program, "cbcsysp", CONF_S) == 0))
			display_cbcsysp_usage();
		else if ((strncmp(program, "cbcscript", CONF_S) == 0))
			display_cbcscript_usage();
		else if (strncmp(program, "cbclocale", CONF_S) == 0)
			display_cbclocale_usage();
		exit(0);
	} else {
		fprintf(stderr, "Unknown error code %d!\n", retval);
		printf("Usage: run %s on its own or check man pages\n",
		  program);
	}
	exit (retval);
}

void
display_cmdb_usage(void)
{
	printf("CMDB: Configuration Management Database\n");
	printf("Version: %s\n", VERSION);
	printf("Action options:\n");
	printf("-a: add\n-d: display\n-l: list\n-r: remove\n-f: force\n");
	printf("Type options:\n");
	printf("-s: server\n-u: customer\n-t: contact\n");
	printf("-e: services\n-w: hardware\n-o: virtual machine hosts\n");
	printf("Name options:\n");
	printf("-n: name\n-i: uuid for server or coid for customer\n");
	printf("-m: vmhost server name for adding a server\n");
	printf("Adding options:\n");
	printf("For server (with -s; need to add -n for name and -m for vm_host (if required))\n");
	printf("-V: Vendor\t-M: Make\t-O: Model\t-U: UUID\t-C: COID\n");
	printf("For customer (with -u; need -n for name)\n");
	printf("-A: Address\t-T: City\t-Y: County\t-Z: Postcode\t-C: COID\n");
	printf("For services (with -i COID for customer, -n name for server)\n");
	printf("-D: Detail\t-L: URL\t\t[ -I service_id | -S service ]\n");
	printf("For hardware (with -n name to specify server)\n");
	printf("-D: Detail\t-B: Device\t-I: hardware_id\n");
	printf("For Contact (with -i coid to specify customer)\n");
	printf("-N: Name\t-P: Phone\t-E: email\n");
	printf("For VM Host Server (with -n name to specify server)\n");
	printf("-O: VM_Host_type\n");
}

void
display_cbc_usage(void)
{
	printf("cbc: Create Build Configuration\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action options:\n");
	printf("-a: add build for server\n-d: display build details\n");
	printf("-l: list servers with a build\n-m: modify build options\n");
	printf("-r: remove build for server\n-w: write build files\n\n");
	printf("Display and write options:\n");
	printf("cbc ( -d | -w ) ( -n | -i | -u ) <server specifier>\n\n");
	printf("Remove options:\n");
	printf("cbc -r [ -g ] ( -n | -i | -u ) <server specifier>\n");
	printf("-g will remove the build IP from DB. Dangerous if server is still online\n\n");
	printf("Create and modify options:\n");
	printf("cbc ( -a | -m ) -o<OS> -s<version> -t<arch> -b<domain> -x");
	printf("<varient> -e<locale_id>\n -p<scheme> -k<network device> -j<hardisk device> ");
	printf("(-n | -i | -u ) ");
	printf("<server_specifier>\n\n");
	printf("The various associated programs will give you the names ");
	printf("for these options.\n\n");
	printf("cbcos cbcdomain cbcvarient cbcpart cbclocale\n");
	
}

void
display_cbcdomain_usage(void)
{
	printf("cbcdomain: Program to manipulate build domains\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action Options:\n");
	printf("-a: add build domain\n");
	printf("-l: list build domain names\n-m: modify build domain\n");
	printf("-r: remove build domain\n-w: write dhcp network config\n");
	printf("All actions apart from -l and -w need -b <domain name>\n\n");
	printf("Network Details:\n");
	printf("-k: start_ip,end_ip,gateway,netmask,nameserver\n\n");
	printf("NTP server configuration:\n");
	printf("-t ntp_server\n\n");
	printf("cbcdomain ( action ) [ -b build-domain ] ( app options )\n\n");
}

void
display_cbcos_usage(void)
{
	printf("cbcos: Program to manipulate build operating systems\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action Options:\n");
	printf("-a: add OS\n-d: display OS\n-g: grab boot files\n-l: list OS\n");
	printf("-r: remove OS\n");
	printf("All actions apart from -l and -g need -n <OS name>\n\n");
	printf("Detail Options:\n");
	printf("-e: <version alias>\n-o: <os version>\n");
	printf("-s: <alias>\n-t: <os architecture\n\n");
	printf("cbcos [ -a | -d | -l | -r ] -n [ detail options ]\n\n");
}

void
display_cbcvarient_usage(void)
{
	printf("cbcvarient: Program to manipulate build varients\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action Options:\n");
	printf("-a: add varient\n-d: display varient\n-l: list varients\n");
	printf("-r: remove varient\n\n");
	printf("-d and -r actions need -x <varient name> or -k <valias>\n");
	printf("-a will need both -x <varient name> and -k <valias>\n\n");
	printf("Definition Options:\n");
	printf("-g: package\n-j: varient\n\n");
	printf("Name Options:\n");
	printf("-x: <varient>\n-k: <valias>\n-p: <package>\n\n");
	printf("Detail Options:\n");
	printf("-n: <os name>\n-e: <version alias>\n-o: <os version>\n");
	printf("-s: <os alias>\n-t: <os architecture\n\n");
	printf("cbcvarient [ -a | -d | -l | -r ] \
[ -g | -j ] [ -x | -k ] [ detail options ]\n\n");
}

void
display_cbcpart_usage(void)
{
	printf("cbcpart: Program to manipulate build partitions & schemes\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action Options:\n");
	printf("-a: add scheme / partition\n-d: display scheme\n");
	printf("-l: list schemes\n-r: remove scheme / partition\n");
	printf("-m: modify\n\n");
	printf("Definition Options:\n");
	printf("-p: partition\n-s: scheme\n-o: option\n\n");
	printf("Detail Options\n");
	printf("-u: Use lvm (when adding a scheme)\n");
	printf("-g: <logical-volume> (if using lvm)\n");
	printf("-n: <scheme name>\n\n");
	printf("Partition Details:\n");
	printf("-i: <minimum-size>\n");
	printf("-x: <maximum-size>\n");
	printf("-y: <priority>\n");
	printf("-b: <mount-option>\n");
	printf("-f: <file-system-type>\n");
	printf("-t: <mount point>\n\n");
	printf("cbcpart: ( -a | -d | -l | -m | -r ) ( -p | -s | -o ) [ ( -u -g \
log vol ) ] [ -n ] ( -f -x -t [ -i ] [ -y ] [ -b ] )\n");
}

void
display_dnsa_usage(void)
{
	printf("Version: %s\n", VERSION);
	printf("Action options (and needed options)\n");
	printf("-a: add host record\n\t-t -h -i -n (-p) (-o -s)\n");
	printf("-b: build reverse zone\n\t-n\n");
	printf("-d: display zone\n\t[ -F | -R | -G ] -n\n");
	printf("-e: add preferred A record for reverse DNS");
	printf("\n\t-h -n -i\n");
	printf("-g: remove preferred A record\n\t -i\n");
	printf("-l: list zones\n\t[ -F | -R ]\n");
	printf("-m: add CNAME to root domain\n\t-h -n\n");
	printf("-r: remove record\n\t-h -n\n");
	printf("-u: display IP's with multiple A records\n\t-n\n");
	printf("-w: commit valid zones on nameserver\n\t[ -F | -R ]\n");
	printf("-x: remove zone\n\t[-F|-R] -n\n");
	printf("-z: add zone\n\t[-F | -R (-p) | -G (-N -I) ] (-S -M) -n\n\n");
	printf("Zone details:\n");
	printf("-M: master IP address\n-N: name server(s) (comma separated)\n");
	printf("-I: IP('s) (comma separated)\n\n");
	printf("Reverse: -p\tSlave: -M\tGlue -N -I\n\n");
	printf("Options for use with adding a host record or zone:\n");
	printf("-i: IP Address\n-t: record type (A, MX etc)\n-h: host");
	printf("\n-p: priority (for MX and SRV records), prefix for reverse zone\n");
	printf("-o: protocol\n-s: service (for SRV records)\n\n");
}

void
display_cpc_usage(void)
{
	printf("cpc: create preseed config %s\n", VERSION);
	printf("Usage\n");
	printf("-d <domain>\tDNS domain name\n");
	printf("-e <ntp-server>\n");
	printf("-f <filename>\tOutput to the named file\n");
	printf("-y <keyboard county layout>\n");
	printf("-l <locale>\n");
	printf("-i <interface>\n");
	printf("-n <hostname>\tHost name\n");
	printf("-m <mirror>\n");
	printf("-p <pack1,pack2,...,packn>\n");
	printf("-s <suite>\n");
	printf("-t <timezone>\n");
	printf("-u <url>\n");
	printf("-k <disk drive id>\n");
}

void
display_ckc_usage(void)
{
	printf("ckc: create kickstart config %s\n", VERSION);
	printf("Usage\n");
	printf("-d <domain>\tDNS domain name\n");
	printf("-f <filename>\tFilename to output kickstart file to\n");
	printf("-i <ip address>\tIP address for the server\n");
	printf("-k <disk>\tDisk to install on (no /dev/ needed)\n");
	printf("-l <language>\tDefault language for system\n");
	printf("-n <name>\tHostname for the server\n");
	printf("-p <packages>\tComma separated list of extra packages\n");
	printf("-t <timezone>\tTimezone for the server\n");
	printf("-u <url>\tUrl location of kickstart file\n");
	printf("-y <country>\tCountry keyboard mapping\n");
	printf("\n");
	printf("-h\tDisplay this help message\n");
	printf("-v\tDisplay cpc version number\n");
}

void
display_cbcsysp_usage(void)
{
	printf("cbcsysp: cbc system package %s\n", VERSION);
	printf("Usage:\t");
	printf("cbcsysp <action> <type> <arguments>\n");
	printf("Action options\n");
	printf("-a: add -l: list -r: remove -m: modify\n");
	printf("Type options\n");
	printf("-p: package\t-o: config\t-y: arguments\n");
	printf("Arguments\n");
	printf("-b <domain>\t-f <field>\t-n <name>\t-g <arg>\n");
	printf("-t <type>\n");
	printf("See man page for full details\n");
}

void
display_cbcscript_usage(void)
{
	printf("cbcscript: cbc scripts %s\n", VERSION);
	printf("Usage:\t");
	printf("cbcscript <action> <type> <arguments>\n");
	printf("Action options\n");
	printf("-a: add\t-l: list -r: remove\n");
	printf("Type options\n");
	printf("-f: arg\t-s: script\n");
	printf("Arguments\n");
	printf("-b <domain>\t-o <number>\t-g <arg>\t-n <name>\n");
	printf("-t <build os>\n");
	printf("See man page for full details\n");
}

void
display_cbclocale_usage(void)
{
	printf("cbclocale: create build config locale %s\n", VERSION);
	printf("Usage:\t");
	printf("cbclocale <action> <options>\n");
	printf("Action options\n");
	printf("-a: add\t-d display\t-l: list\t-r: remove\n-x: set default locale\n");
	printf("Options\n");
	printf("-g language\t-k keymap\t-o locale\t-n name\n");
	printf("-t timezone\t-u country\n");
	printf("See man page for full details\n");
	
}

void
display_mkvm_usage(void)
{
	const char *prog = "mkvm";

	printf("%s: make virtual machine %s\n", prog, VERSION);
	printf("Usage:\t");
	printf("%s <action> <options>\n", prog);
	printf("Actions\n");
	printf("\t-a: add\n");
	printf("\t-h: help\t-v: version\n");
	printf("Options\n");
	printf("\t-n <name>: Supply VM name\n");
	printf("\t-p <pool>: Provide the storage pool name\n");
	printf("\t-g <size>: Size (in GB) of disk (default's to 10GB)\n");
}

void
display_version(char *prog)
{
	if (strrchr(prog, '/')) {
		prog = strrchr(prog, '/');
		prog++;
	}
	printf("%s: %s\n", prog, VERSION);
}

void
get_error_string(int error, char *errstr)
{
	if (error == SERVER_NOT_FOUND)
		snprintf(errstr, MAC_S, "Server not found.");
	else if (error == SERVER_UUID_NOT_FOUND)
		snprintf(errstr, MAC_S, "Server not found.");
	else if (error == SERVER_ID_NOT_FOUND)
		snprintf(errstr, MAC_S, "Server not found.");
	else if (error == NO_NAME_UUID_ID)
		snprintf(errstr, MAC_S, "No server specifier.");
	else if (error == BUILD_DOMAIN_NOT_FOUND)
		snprintf(errstr, MAC_S, "Build domain not found.");
	else if (error == NO_BUILD_IP) 
		snprintf(errstr, MAC_S, "No IP's left in build domain.");
	else if (error == BUILD_IP_IN_USE)
		snprintf(errstr, MAC_S, "Build IP already in use.");
	else if (error == CANNOT_INSERT_IP)
		snprintf(errstr, MAC_S, "Cannot insert build IP into DB.");
	else if (error == SERVER_BUILD_NOT_FOUND)
		snprintf(errstr, MAC_S, "No server build.");
	else if (error == NO_DHCP_B_ERR)
		snprintf(errstr, MAC_S, "Cannot find dhcp details.");
	else if (error == MULTI_DHCP_B_ERR)
		snprintf(errstr, MAC_S, "Multiple dhcp details.");
	else if (error == NO_TFTP_B_ERR)
		snprintf(errstr, MAC_S, "Cannot find TFTP details.");
	else if (error == MULTI_TFTP_B_ERR)
		snprintf(errstr, MAC_S, "Multiple TFTP details.");
	else if (error == NO_NET_BUILD_ERR)
		snprintf(errstr, MAC_S, "Cannot find NET_BUILD_DETAILS.");
	else if (error == MULTI_NET_BUILD_ERR)
		snprintf(errstr, MAC_S, "Multiple NET_BUILD_DETAILS.");
	else if (error == NO_BUILD_MIRR_ERR)
		snprintf(errstr, MAC_S, "Cannot find BUILD_MIRROR.");
	else if (error == MULTI_BUILD_MIRR_ERR)
		snprintf(errstr, MAC_S, "Multiple BUILD_MIRROR.");
	else if (error == VARIENT_NOT_FOUND)
		snprintf(errstr, MAC_S, "Unknown build varient.");
	else if (error == MULTIPLE_VARIENTS)
		snprintf(errstr, MAC_S, "Multiple varients found.");
	else if (error == NO_NETWORK_HARDWARE)
		snprintf(errstr, MAC_S, "Network device not found.");
	else if (error == OS_NOT_FOUND)
		snprintf(errstr, MAC_S, "OS not found.");
	else if (error == MULTIPLE_OS)
		snprintf(errstr, MAC_S, "Multiple OS found.");
	else if (error == SCHEME_NOT_FOUND)
		snprintf(errstr, MAC_S, "Partition scheme not found.");
	else if (error == INSERT_NOT_CONFIGURED)
		snprintf(errstr, MAC_S, "No Database insert function.");
	else if (error == CANNOT_FIND_BUILD_IP)
		snprintf(errstr, MAC_S, "Cannot find the build IP!");
	else if (error == BUILD_IN_DATABASE)
		snprintf(errstr, MAC_S, "Build for server already exists.");
	else if (error == CANNOT_MODIFY_BUILD_DOMAIN)
		snprintf(errstr, MAC_S, "Cannot modify build domain.");
	else if (error == LOCALE_NOT_IMPLEMENTED)
		snprintf(errstr, MAC_S, "Locale not implemented, sorry.");
	else if (error == NO_MODIFIERS)
		snprintf(errstr, MAC_S, "No modifiers supplied.");
	else if (error == PARTITIONS_NOT_FOUND)
		snprintf(errstr, MAC_S, "Partition scheme not found.");
	else if (error == NO_BASIC_DISK)
		snprintf(errstr, MAC_S, "Cannot find partitions.");
	else if (error == NO_BUILD_URL)
		snprintf(errstr, MAC_S, "No url in build domain.");
	else if (error == NO_LOG_CONFIG)
		snprintf(errstr, MAC_S, "Cannot get log config.");
	else if (error == NO_BD_CONFIG)
		snprintf(errstr, MAC_S, "Cannot get build domain config.");
	else if (error == NO_HARD_DISK_DEV)
		snprintf(errstr, MAC_S, "Cannot find disk for server");
	else if (error == NO_BUILD_PACKAGES)
		snprintf(errstr, MAC_S, "No build packages in database");
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
	else if (action == RM_FROM_DB)
		fprintf(stderr, "Removing from DB failed\n");
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

void
get_config_file_location(char *config)
{
	FILE *cnf;
	const char *conf = config;

	if (snprintf(config, CONF_S, "%s/cmdb/cmdb.conf", SYSCONFDIR) >= CONF_S)
		report_error(BUFFER_TOO_SMALL, "for config file");
	if ((cnf = fopen(conf, "r"))) {
		fclose(cnf);
	} else	{
		if (snprintf(config, CONF_S, "%s/dnsa/dnsa.conf", SYSCONFDIR) >= CONF_S)
			report_error(BUFFER_TOO_SMALL, "for config file");
		if ((cnf = fopen(conf, "r")))
			fclose(cnf);
		else
			report_error(CONF_ERR, "no config file");
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
	} else if (member[len - 1] == '/') {
		retval = NONE;
	} else {
		retval = -1;
	}
	
	return retval;
}

int
add_trailing_dot(char *member)
{
/* Maximum string size is 255 bytes */
	size_t len;
	int retval;
	
	retval = 0;
	if ((len = strlen(member)) > 254)
		return -1;
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
	int retval = 0;
// Ensure files are written group writable
	mode_t mode = S_IWOTH;
	mode_t em;
	FILE *zonefile;
	em = umask(mode);
	if (!(zonefile = fopen(filename, "w"))) {
		retval = FILE_O_FAIL;
	} else {
		fputs(output, zonefile);
		fclose(zonefile);
		retval = NONE;
	}
	umask(em);
	return retval;
}

void
convert_time(char *timestamp, unsigned long int *store)
{
	char *tmp, *line;
	struct tm timval;
	size_t len;
	time_t epoch;

	if (strncmp(timestamp, "0", COMM_S) == 0) {
		*store = 0;
		return;
	}
	memset(&timval, 0, sizeof timval);
	len = strlen(timestamp) + 1;
	line = strndup(timestamp, len - 1);
	tmp = strtok(line, "-");
	timval.tm_year = (int)strtol(tmp, NULL, 10);
	if (timval.tm_year > 1900)
		timval.tm_year -= 1900;
	tmp = strtok(NULL, "-");
	timval.tm_mon = (int)strtol(tmp, NULL, 10) - 1;
	tmp = strtok(NULL, " ");
	timval.tm_mday = (int)strtol(tmp, NULL, 10);
	tmp = strtok(NULL, ":");
	timval.tm_hour = (int)strtol(tmp, NULL, 10);
	tmp = strtok(NULL, ":");
	timval.tm_min = (int)strtol(tmp, NULL, 10);
	tmp = strtok(NULL, ":");
	timval.tm_sec = (int)strtol(tmp, NULL, 10);
	timval.tm_isdst = -1;
	epoch = mktime(&timval);
	if (epoch < 0)
		*store = 0;
	else
		*store = (unsigned long int)epoch;
	free(line);
}

char *
get_uname(unsigned long int uid)
{
	struct passwd *user;

	if ((user = getpwuid((uid_t)uid)))
		return user->pw_name;
	else
		return NULL;
}


int
get_ip_from_hostname(dbdata_s *data)
{
	int retval = 0;
	struct addrinfo hints, *srvinfo;
	void *addr;
	dbdata_s *list;

	if (!(data))
		return CBC_NO_DATA;
	list = data;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (strlen(list->fields.text) == 0) {
		if ((retval = gethostname(list->fields.text, RBUFF_S)) != 0) {
			fprintf(stderr, "%s", strerror(errno));
			return NO_NAME;
		}
	}
	if ((retval = getaddrinfo(list->fields.text, "http", &hints, &srvinfo)) != 0) {
		fprintf(stderr, "lib getaddrinfo error: %s\n", gai_strerror(retval));
		return NO_IP_ADDRESS;
	}
	if (srvinfo->ai_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)srvinfo->ai_addr;
		addr = &(ipv4->sin_addr);
	} else {
		fprintf(stderr, "ai_family %d not supported\n", srvinfo->ai_family);
		freeaddrinfo(srvinfo);
		return NO_NAME;
	}
	inet_ntop(srvinfo->ai_family, addr, list->args.text, RBUFF_S);
	freeaddrinfo(srvinfo);
	return retval;
}

void
init_dbdata_struct(dbdata_s *data)
{
	memset(data, 0, sizeof(dbdata_s));
}

void
init_multi_dbdata_struct(dbdata_s **list, unsigned int i)
{
	unsigned int max;
	dbdata_s *data = NULL, *dlist = NULL;
	*list = NULL;

	for (max = 0; max < i; max++) {
		if (!(data = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "data in init_multi_dbdata_struct");
		init_dbdata_struct(data);
		if (!(*list)) {
			*list = dlist = data;
		} else {
			while (dlist->next)
				dlist = dlist->next;
			dlist->next = data;
		}
	}
}

void
clean_dbdata_struct(dbdata_s *list)
{
	dbdata_s *data = NULL, *next = NULL;

	if (list)
		data = list;
	else
		return;
	if (data->next)
		next = data->next;
	else
		next = NULL;
	while (data) {
		free(data);
		if (next)
			data = next;
		else
			return;
		if (data->next)
			next = data->next;
		else
			next = NULL;
	}
}

void
init_string_len(string_len_s *string)
{
	string->len = BUFF_S;
	string->size = NONE;
	string->string = cmdb_malloc(BUFF_S, "string->string in init_string_len");
}

void
clean_string_len(string_len_s *string)
{
	if (string) {
		if (string->string)
			free(string->string);
		free(string);
	}
}

void
init_string_l(string_l *string)
{
	memset(string, 0, sizeof(string_l));
	if (!(string->string = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stirng->string in init_string_l");
}

void
clean_string_l(string_l *list)
{
	string_l *data, *next;

	if (list)
		data = list;
	else
		return;
	next = data->next;
	while (data) {
		free(data->string);
		free(data);
		if (next)
			data = next;
		else
			return;
		next = data->next;
	}
}

void
init_initial_string_l(string_l **string, int count)
{
	int i;
	string_l *data, *list = NULL;

	for (i = 0; i < count; i++) {
		if (!(data = malloc(sizeof(string_l))))
			report_error(MALLOC_FAIL, "data in init_initial_string_l");
		init_string_l(data);
		if (!(list)) {
			*string = list = data;
		} else {
			while (list->next)
				list = list->next;
			list ->next = data;
		}
	}
}

#ifdef HAVE_SQLITE3
# ifndef HAVE_SQLITE3_ERRSTR
const char *
sqlite3_errstr(int error)
{
	if (error == SQLITE_ERROR)
		return "SQL error or missing database";
	else if (error == SQLITE_INTERNAL)
		return "Internal logic error in SQLite";
	else if (error == SQLITE_PERM)
		return "Access permission denied";
	else if (error == SQLITE_ABORT)
		return "Callback routine requested an abort";
	else if (error == SQLITE_BUSY)
		return "The database file is locked";
	else if (error == SQLITE_NOMEM)
		return "A malloc() failed";
	else if (error == SQLITE_READONLY)
		return "Attempt to write a readonly database";
	else if (error == SQLITE_INTERRUPT)
		return "Operation terminated by sqlite3_interrupt()";
	else if (error == SQLITE_IOERR)
		return "Some kind of disk I/O error occurred";
	else if (error == SQLITE_CORRUPT)
		return "The database disk image is malformed";
	else if (error == SQLITE_NOTFOUND)
		return "Unknown opcode in sqlite3_file_control()";
	else if (error == SQLITE_FULL)
		return "Insertion failed because database is full";
	else if (error == SQLITE_CANTOPEN)
		return "Unable to open the database file";
	else if (error == SQLITE_PROTOCOL)
		return "Database lock protocol error";
	else if (error == SQLITE_EMPTY)
		return "Database is empty";
	else if (error == SQLITE_SCHEMA)
		return "The database schema changed";
	else if (error == SQLITE_TOOBIG)
		return "String or BLOB exceeds size limit";
	else if (error == SQLITE_CONSTRAINT)
		return "Abort due to constraint violation";
	else if (error == SQLITE_MISMATCH)
		return "Data type mismatch";
	else if (error == SQLITE_MISUSE)
		return "Library used incorrectly";
	else if (error == SQLITE_NOLFS)
		return "Uses OS features not supported on host";
	else if (error == SQLITE_AUTH)
		return "Authorization denied";
	else if (error == SQLITE_FORMAT)
		return "Auxiliary database format error";
	else if (error == SQLITE_RANGE)
		return "2nd parameter to sqlite3_bind out of range";
	else if (error == SQLITE_NOTADB)
		return "File opened that is not a database file";
#  ifdef SQLITE_NOTICE
	else if (error == SQLITE_NOTICE)
		return "Notifications from sqlite3_log()";
#  endif /* SQLITE_NOTICE */
#  ifdef SQLITE_WARNING
	else if (error == SQLITE_WARNING)
		return "Warnings from sqlite3_log()";
#  endif /* SQLITE_WARNING */
	else if (error == SQLITE_ROW)
		return "sqlite3_step() has another row ready";
	else if (error == SQLITE_DONE)
		return "sqlite3_step() has finished executing";
	return "Unknown error code";
}
# endif /* HAVE_SQLITE3_ERRSTR */
#endif /* HAVE_SQLITE3 */