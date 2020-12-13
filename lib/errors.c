/* 
 *
 *  libailsacmdb: Configuration Management Database
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
 *  (C) Iain M Conochie 2012 - 2020
 * 
 */
#include <config.h>
#include <configmake.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
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
#include <ailsacmdb.h>
#include <ailsasql.h>

void
display_command_line_error(int retval, char *program)
{
	if (strrchr(program, '/')) {
		program = strrchr(program, '/');
		program++;
	}
	if (retval == AILSA_NO_NAME)
		fprintf(stderr, "No name specified with -n.\n");
	else if (retval == AILSA_NO_TYPE)
		fprintf(stderr, "No type specified on command line.\n");
	else if (retval == AILSA_NO_ACTION)
		fprintf(stderr, "No action specified on command line.\n");
	else if (retval == AILSA_WRONG_ACTION)
		fprintf(stderr, "Unable to complete this action.\n");
	else if (retval == AILSA_NO_NAME_OR_ID)
		fprintf(stderr, "No name or ID specified on command line.\n");
	else if (retval == AILSA_NO_SERVICE)
		fprintf(stderr, "No service provided on command line\n");
	else if (retval == AILSA_NO_DETAIL)
		fprintf(stderr, "No detail provided on command line\n");
	else if (retval == AILSA_NO_DEVICE)
		fprintf(stderr, "No device provided on command line\n");
	else if (retval == AILSA_NO_VHOST_TYPE)
		fprintf(stderr, "No virtual machine host type specified on command line\n");
	else if (retval == AILSA_NO_ID_OR_CLASS)
		fprintf(stderr, "No hardware ID or class was specified on command line\n");
	else if (retval == AILSA_NO_DOMAIN_NAME)
		fprintf(stderr, "No domain specified on command line.\n");
	else if (retval == AILSA_NO_IP_ADDRESS)
		fprintf(stderr, "No IP address specified on command line.\n");
	else if (retval == AILSA_NO_HOST_NAME)
		fprintf(stderr, "No hostname specified on command line.\n");
	else if (retval == AILSA_NO_RECORD_TYPE) 
		fprintf(stderr, "No record type specified on command line.\n");
	else if (retval == AILSA_NO_CLASS)
		fprintf(stderr, "No hardware class specified on command line.\n");
	else if (retval == AILSA_NO_ADDRESS)
		fprintf(stderr, "No address specified on command line.\n");
	else if (retval == AILSA_NO_CITY)
		fprintf(stderr, "No city specified on command line.\n");
	else if (retval == AILSA_NO_COUNTY)
		fprintf(stderr, "No county specified on command line.\n");
	else if (retval == AILSA_NO_POSTCODE)
		fprintf(stderr, "No postcode specified on command line.\n");
	else if (retval == AILSA_NO_COID)
		fprintf(stderr, "No coid specified on command line.\n");
	else if (retval == AILSA_NO_CONTACT_NAME)
		fprintf(stderr, "No name specified with -N.\n");
	else if (retval == AILSA_NO_SERVICE_URL)
		fprintf(stderr, "A service name or URL was not specified\n\
If you wish to remove all services (for a server or customer) add the -f option\n");
	else if (retval == AILSA_NO_PHONE_NUMBER)
		fprintf(stderr, "No phone no. specified with -P.\n");
	else if (retval == AILSA_NO_PACKAGE)
		fprintf(stderr, "No package supplied.\n");
	else if (retval == AILSA_NO_OS)
		fprintf(stderr, "No os or alias supplied.\n");
	else if (retval == AILSA_NO_VARIENT)
		fprintf(stderr, "No varient or valias supplied.\n");
	else if (retval == AILSA_NO_EMAIL_ADDRESS)
		fprintf(stderr, "No email address specified with -E.\n");
	else if (retval == AILSA_WRONG_TYPE_DISPLAY)
		fprintf(stderr, "Cannot display this type in cmdb\n");
	else if (retval == AILSA_DOMAIN_AND_IP_GIVEN)
		fprintf(stderr, "Both domain name and IP given on command line.\n");
	else if (retval == AILSA_NO_DOMAIN_OR_NAME)
		ailsa_syslog(LOG_ERR, "Neither build domaion nor script name was provided");
	else if (retval == AILSA_NO_PARTITION)
		fprintf(stderr, "No partition information on command line.\n");
	else if (retval == AILSA_NO_SCHEME)
		fprintf(stderr, "No scheme name was supplied on the command line.\n");
	else if (retval == AILSA_NO_BUILD_DOMAIN)
		fprintf(stderr, "No Build Domain supplied on command line\n");
	else if (retval == AILSA_NO_MOD_BUILD_DOM_NET)
		fprintf(stderr, "Cowardly refusal to modify network settings for build domain\n");
	else if (retval == AILSA_NO_MASTER)
		fprintf(stderr, "Slave zone specified but no master IP given\n");
	else if (retval == AILSA_NO_MASTER_NAME)
		fprintf(stderr, "Slave zone specified but no master name given\n");
	else if (retval == AILSA_NO_PREFIX)
		fprintf(stderr, "No reverse zone prefix was supplied\n");
	else if (retval == AILSA_NO_ARG)
		fprintf(stderr, "No arguemt was supplied\n");
	else if (retval == AILSA_NO_NUMBER)
		fprintf(stderr, "No number was supplied\n");
	else if (retval == BUILD_DOMAIN_NETWORK_INVALID)
		fprintf(stderr, "Check your network input please. It seems wrong!\n");
	else if (retval == AILSA_NO_DISK_DEV)
		ailsa_syslog(LOG_ERR, "No disk device was provided");
	else if (retval == AILSA_INVALID_DBTYPE)
		ailsa_syslog(LOG_ERR, "DB type was invalid in the query");
	else if (retval == AILSA_NO_PRIORITY)
		ailsa_syslog(LOG_ERR, "No priority was specified");
	else if (retval == AILSA_NO_PROTOCOL)
		ailsa_syslog(LOG_ERR, "No protocol was specified");
	else if (retval == AILSA_NO_SERVICE)
		ailsa_syslog(LOG_ERR, "No service was specified");
	else if (retval == AILSA_NO_FILESYSTEM)
		ailsa_syslog(LOG_ERR, "No file system type was supplied");
	else if (retval == AILSA_NO_RECORD_TYPE)
		ailsa_syslog(LOG_ERR, "No record type was specified");
	else if (retval == AILSA_NO_LOGVOL)
		ailsa_syslog(LOG_ERR, "No logical volume name supplied");
	else if ((retval >= 600) && (retval < 700))
		ailsa_syslog(LOG_ERR, "Input validation failed: %s", ailsa_comm_line_strerror(retval));
	else if (retval == AILSA_NO_OPTION)
		ailsa_syslog(LOG_ERR, "Partition option specified but no option supplied");
	else if (retval == AILSA_VERSION)
		ailsa_syslog(LOG_ERR, "%s: %s\n", program, VERSION);
	else if (retval == AILSA_NO_URI)
		ailsa_syslog(LOG_ERR, "No URI was specified for the libvirt connection");
	else if (retval == AILSA_DISPLAY_USAGE) {
		if ((strncmp(program, "cmdb", CONFIG_LEN) == 0) || (strncmp(program, "cmdb2", CONFIG_LEN) == 0))
			display_cmdb_usage();
		else if ((strncmp(program, "cbc", CONFIG_LEN) == 0))
			display_cbc_usage();
		else if ((strncmp(program, "dnsa", CONFIG_LEN) ==0))
			display_dnsa_usage();
		else if ((strncmp(program, "cbcdomain", CONFIG_LEN) == 0))
			display_cbcdomain_usage();
		else if ((strncmp(program, "cbcos", CONFIG_LEN) == 0))
			display_cbcos_usage();
		else if ((strncmp(program, "cbcvarient", CONFIG_LEN) == 0))
			display_cbcvarient_usage();
		else if ((strncmp(program, "cbcpart", CONFIG_LEN) == 0))
			display_cbcpart_usage();
		else if ((strncmp(program, "cpc", CONFIG_LEN) == 0))
			display_cpc_usage();
		else if ((strncmp(program, "cbcsysp", CONFIG_LEN) == 0))
			display_cbcsysp_usage();
		else if ((strncmp(program, "cbcscript", CONFIG_LEN) == 0))
			display_cbcscript_usage();
		else if (strncmp(program, "cbclocale", CONFIG_LEN) == 0)
			display_cbclocale_usage();
		exit(0);
	} else {
		fprintf(stderr, "Unknown error code %d!\n", retval);
		printf("Usage: run %s on its own or check man pages\n",
		  program);
	}
	exit (retval);
}

const char *
ailsa_comm_line_strerror(int error)
{
	switch(error) {
	case RTYPE_INPUT_INVALID:
		return "Resource type was invalid";
	case ZTYPE_INPUT_INVALID:
		return "Zone type was invalid";
	case SERVICE_INPUT_INVALID:
		return "Service was invalid";
	case PROTOCOL_INPUT_INVALID:
		return "Protocol was invalid";
	case DOMAIN_INPUT_INVALID:
		return "Domain name was invalid";
	case CONFIG_INPUT_INVALID:
		return "Config file location was invalid";
	case HOST_INPUT_INVALID:
		return "Hostname was invalid";
	case DEST_INPUT_INVALID:
		return "Destination was invalid";
	case MASTER_INPUT_INVALID:
		return "Master IP or hostname was invalid";
	case GLUE_IP_INPUT_INVALID:
		return "Glue IP address was invalid";
	case GLUE_NS_INPUT_INVALID:
		return "Glue hostname was invalid";
	case UUID_INPUT_INVALID:
		return "UUID supplied was invalid";
	case SERVER_NAME_INVALID:
		return "Server name supplied was invalid";
	case OS_INVALID:
		return "Operating System supplied was invalid";
	case OS_VERSION_INVALID:
		return "Operating System version was invalid";
	case PART_SCHEME_INVALID:
		return "Partition scheme name was invalid";
	case VARIENT_INVALID:
		return "Varient name supplied was invalid";
	case ARCH_INVALID:
		return "Architecture supplied was invalid";
	case NET_CARD_INVALID:
		return "Network card device name was invalid";
	case HARD_DISK_INVALID:
		return "Hard disk device name was invalid";
	case CONFIG_INVALID:
		return "Config file path was invalid";
	case LOCALE_INVALID:
		return "Locale name supplied was invalid";
	case PROTOCOL_INVALID:
		return "Protocol supplied was neither tcp nor udp";
	case VMHOST_INVALID:
		return "VM hostname supplied was invalid";
	case MAKE_INVALID:
		return "Make supplied was invalid";
	case VENDOR_INVALID:
		return "Vendor supplied was invalid";
	case CUSTOMER_NAME_INVALID:
		return "Customer name supplied was invalid";
	case ADDRESS_INVALID:
		return "Address supplied was invalid";
	case CITY_INVALID:
		return "City supplied was invalid";
	case EMAIL_ADDRESS_INVALID:
		return "Email address supplied was invalid";
	case DETAIL_INVALID:
		return "Service or hardware detail was invalid";
	case HCLASS_INVALID:
		return "Hardware class supplied was invalid";
	case URL_INVALID:
		return "URL specified was invalid";
	case TYPE_INVALID:
		return "VM host or hardware type supplied was invalid";
	case IP_INVALID:
		return "IP Supplied was invalid";
	case NTP_SERVER_INVALID:
		return "NTP server supplied was invalid";
	case LANGUAGE_INVALID:
		return "Language supplied was invalid";
	case KEYMAP_INVALID:
		return "Keymap supplied was invalid";
	case COUNTRY_INVALID:
		return "Country supplied was invalid";
	case TIMEZONE_INVALID:
		return "Timezone supplied was invalid";
	case MIN_INVALID:
		return "Number supplied for minimum invalid";
	case MAX_INVALID:
		return "Number supplied for maximum invalid";
	case PRI_INVALID:
		return "Number supplied for priority invalid";
	case FILESYSTEM_INVALID:
		return "Filesystem supplied was invalid";
	case LOG_VOL_INVALID:
		return "Logical volume name supplied was invalid";
	case FS_PATH_INVALID:
		return "Filesystem path supplied was invalid";
	case PACKAGE_FIELD_INVALID:
		return "Field for package supplied was invalid";
	case PACKAGE_ARG_INVALID:
		return "Argument for package supplied was invalid";
	case PACKAGE_NAME_INVALID:
		return "Package name supplied was invalid";
	case PACKAGE_TYPE_INVALID:
		return "Package type supplied was invalid";
	default:
		return "Unknown command line error";
	}
}

void
ailsa_display_validation_error(int error)
{
	ailsa_syslog(LOG_ERR, "Command line validation failed: %s", ailsa_comm_line_strerror(error));
}

void
display_cmdb_usage(void)
{
	printf("CMDB: Configuration Management Database\n");
	printf("Version: %s\n", VERSION);
	printf("Action options:\n");
	printf("-a: add\n-d: display\n-l: list\n-m: modify\n-r: remove\n-f: force\n");
	printf("-z: set-default (for customer)\n");
	printf("Type options:\n");
	printf("-s: server\n-u: customer\n-t: contact\n");
	printf("-e: services\n-w: hardware\n-o: virtual machine hosts\n");
	printf("-j: service types\n-g: hardware types\n");
	printf("Name options:\n");
	printf("-n: name\n-i: uuid for server or coid for customer\n");
	printf("-x: vmhost server name for adding a vm server\n");
	printf("Adding options:\n");
	printf("For server (with -s; need to add -n for name and -m for vm_host (if required))\n");
	printf("-V: Vendor\t-M: Make\t-O: Model\t-U: UUID\t-C: COID\n");
	printf("For customer (with -u; need -N for name)\n");
	printf("-A: Address\t-T: City\t-Y: County\t-Z: Postcode\t-C: COID\n");
	printf("For services (with -C COID for customer, -n name for server)\n");
	printf("-D: Detail\t-L: URL\t\t[ -I service id | -S service ]\n");
	printf("For service types\n");
	printf("-D detail\t-S service\n");
	printf("For hardware (with -n name to specify server)\n");
	printf("-D: Detail\t-B: Device\t[ -I: hardware id | -H hardware description ]\n");
	printf("For hardware types\n");
	printf("-y: type\t-H Description\n");
	printf("For Contact (with -i coid to specify customer)\n");
	printf("-N: Name\t-P: Phone\t-E: email\n");
	printf("For VM Host Server (with -n name to specify server)\n");
	printf("-y: VM Host type (e.g. libvirt. vmware)\n");
}

void
display_cbc_usage(void)
{
	printf("cbc: Create Build Configuration\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action options:\n");
	printf("-a: add build for server\n-d: display build details\n");
	printf("-l: list servers with a build\n-m: modify build options\n");
	printf("-r: remove build for server\n-u: show defaults\n");
	printf("-w: write build files\n\n");
	printf("Display and write options:\n");
	printf("cbc ( -d | -w ) -n <server name>\n\n");
	printf("Remove options:\n");
	printf("cbc -r [ -g ] -n <server name>\n");
	printf("-g will remove the build IP from DB. Dangerous if server is still online\n\n");
	printf("Create and modify options:\n");
	printf("cbc ( -a | -m ) -o<OS> -s<version> -t<arch> -b<domain> -x");
	printf("<varient> -e<locale_id>\n -p<scheme> -k<network device> -j<hardisk device> ");
	printf("-n <server name>\n\n");
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
	printf("-a: add build domain\n-d: display build domain\n");
	printf("-l: list build domain names\n-m: modify build domain\n");
	printf("-r: remove build domain\n-w: write dhcp network config\n");
	printf("-z: set default build domain\n");
	printf("All actions apart from -l and -w need -n <domain name>\n\n");
	printf("Network Details:\n");
	printf("-k: start_ip,end_ip,gateway,netmask,nameserver\n\n");
	printf("NTP server configuration:\n");
	printf("-t ntp_server\n\n");
	printf("cbcdomain ( action ) [ -n build-domain ] ( app options )\n\n");
}

void
display_cbcos_usage(void)
{
	printf("cbcos: Program to manipulate build operating systems\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action Options:\n");
	printf("-a: add OS\n-d: display OS\n-g: grab boot files\n-l: list OS\n");
	printf("-r: remove OS\n-z: set default OS\n");
	printf("All actions apart from -l and -g need -n <OS name>\n\n");
	printf("Detail Options:\n");
	printf("-e: <version alias>\n-o: <os version>\n");
	printf("-s: <alias>\n-t: <os architecture\n\n");
	printf("cbcos [ -a | -d | -l | -r | -z ] -n [ detail options ]\n\n");
}

void
display_cbcvarient_usage(void)
{
	printf("cbcvarient: Program to manipulate build varients\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action Options:\n");
	printf("-a: add varient\n-d: display varient\n-l: list varients\n");
	printf("-r: remove varient\n-z: set-default\n\n");
	printf("-d, -r and -z actions need -x <varient name> or -k <valias>\n");
	printf("-a will need both -x <varient name> and -k <valias>\n\n");
	printf("Definition Options:\n");
	printf("-g: package\n-j: varient\n\n");
	printf("Name Options:\n");
	printf("-x: <varient>\n-k: <valias>\n-p: <package>\n\n");
	printf("Detail Options:\n");
	printf("-n: <os name>\n-e: <version alias>\n-o: <os version>\n");
	printf("-s: <os alias>\n-t: <os architecture\n\n");
	printf("cbcvarient ( -a | -d | -l | -r | -z ) \
( -g | -j ) ( -x | -k ) [ detail options ]\n\n");
}

void
display_cbcpart_usage(void)
{
	printf("cbcpart: Program to manipulate build partitions & schemes\n\n");
	printf("Version: %s\n", VERSION);
	printf("Action Options:\n");
	printf("-a: add scheme / partition\n-d: display scheme\n");
	printf("-l: list schemes\n-r: remove scheme / partition\n");
	printf("-m: modify\n-z: set-default\n\n");
	printf("Definition Options:\n");
	printf("-p: partition\n-s: scheme\n-o: option\n\n");
	printf("Detail Options\n");
	printf("-j: Use lvm (when adding a scheme)\n");
	printf("-g: <logical-volume> (if using lvm)\n");
	printf("-n: <scheme name>\n\n");
	printf("Partition Details:\n");
	printf("-i: <minimum-size>\n");
	printf("-x: <maximum-size>\n");
	printf("-y: <priority>\n");
	printf("-b: <mount-option>\n");
	printf("-f: <file-system-type>\n");
	printf("-t: <mount point>\n\n");
	printf("cbcpart: ( -a | -d | -l | -m | -r | -z ) ( -p | -s | -o ) [ ( -j -g \
log vol ) ] [ -n ] ( -f -x -t [ -i ] [ -y ] [ -b ] )\n");
}

void
display_dnsa_usage(void)
{
	printf("Version: %s\n", VERSION);
	printf("Action options (and needed options)\n");
	printf("-a: add host record\n\t-t -h -i -n (-p) (-o -s)\n");
	printf("-b: build reverse zone\n\t-n\n");
	printf("-d: display zone\n\t( -F | -R | -G ) -n\n");
	printf("-e: add preferred A record for reverse DNS");
	printf("\n\t-h -n -i\n");
	printf("-g: remove preferred A record\n\t -i\n");
	printf("-l: list zones\n\t( -F | -R )\n");
	printf("-m: add CNAME to root domain\n\t-h -n [ -j top-level domain ]\n");
	printf("-r: remove record\n\t-h -n -t\n");
	printf("-u: display IP's with multiple A records\n\t( -n domain | -i ip-address )\n");
	printf("-w: commit valid zones on nameserver\n\t( -F | -R )\n");
	printf("-x: remove zone\n\t( -F |-R ) -n\n");
	printf("-z: add zone\n\t( -F | -R [-p] | -G -N [ -I ] ) [ -S -M ] -n\n\n");
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
	printf("cbcscript <action> [ <type> ] <arguments>\n");
	printf("Action options\n");
	printf("-a: add\t-d: display\t-l: list\t-r: remove\n");
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
	printf("-a: add\t-d display\t-l: list\t-r: remove\n-z: set default locale\n");
	printf("Options\n");
	printf("-g language\t-k keymap\t-o locale\t-n name\n");
	printf("-t timezone\t-u country\n");
	printf("See man page for full details\n");
	
}

void 
display_type_error(short int type)
{
	char *message;
	
	message = ailsa_calloc(CONFIG_LEN, "message in display_type_error");
	snprintf(message, HOST_LEN, "\
Unable to perform requested action on ");
	if (type == SERVER) {
		strncat(message, "server\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == CUSTOMER) {
		strncat(message, "customer\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == CONTACT) {
		strncat(message, "contact\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == SERVICE) {
		strncat(message, "service\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == HARDWARE) {
		strncat(message, "hardware\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == HARDWARE_TYPE) {
		strncat(message, "hardware type\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == SERVICE_TYPE) {
		strncat(message, "service type\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == SERVER_TYPE) {
		strncat(message, "server type\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else if (type == VM_HOST) {
		strncat(message, "virtual machine host\n", MAC_LEN);
		fprintf(stderr, "%s", message);
	} else {
		strncat(message, "unknown type ", MAC_LEN);
		fprintf(stderr, "%s", message);
		fprintf(stderr, "%d\n", type);
	}
	my_free(message);
}

const char *
ailsa_strerror(int type)
{
	const char *message = NULL;

	switch (type) {
	case AILSA_NO_QUERY:
		message = "No query was passed";
		break;
	case AILSA_NO_DBTYPE:
		message = "There is no DB type configured";
		break;
	case AILSA_INVALID_DBTYPE:
		message = "An invalid DB type was used";
		break;
	case AILSA_NO_PARAMETERS:
		message = "No parameters for MySQL bind operation";
		break;
	case AILSA_NO_QUERY_NO:
		message = "Query number passed was 0";
		break;
	case AILSA_NO_FIELDS:
		message = "No feilds to select in MySQL query";
		break;
	default:
		message = "Unknown type error";
		break;
	}
	return message;
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
