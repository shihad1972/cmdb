/* build_info.c
 * 
 * Functions to retrieve build information from the database and
 * to create the specific build files required.
 * 
 * Part of the cbc program
 * 
 * (C) 2012 Iain M. Conochie
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>	/* required for IP address conversion */
#include <sys/stat.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_mysql.h"

void
fill_build_info(cbc_build_t *cbt, MYSQL_ROW br);

void
write_config_file(char *filename, char *output);

void
read_dhcp_hosts_file(cbc_build_t *cbcbt, char *from, char *content, char *new_content);

void
add_append_line(cbc_build_t *cbcbt, char *output, char *tmp);

int
write_preseed_config(cbc_config_t *cmc, cbc_build_t *cbt);

int
write_kickstart_config(cbc_config_t *cmc, cbc_build_t *cbt);

int
write_disk_preconfig(cbc_config_t *cmc, cbc_build_t *cbt, char *out, char *buff);

int
add_preseed_packages(cbc_config_t *cmc, cbc_build_t *cbt, char *out, char *buff);

int
write_regular_preheader(char *output, char *tmp);

int
write_lvm_preheader(char *output, char *tmp, char *device);

int
check_for_special_partition(pre_disk_part_t *part_info);

int
add_partition_to_preseed(pre_disk_part_t *part_info, char *output, char *buff, int special, int lvm);

pre_disk_part_t
*part_node_create(void);

pre_disk_part_t
*part_node_add(pre_disk_part_t *head_node, MYSQL_ROW part_row);

int
part_node_delete(pre_disk_part_t head_node);

void
part_node_free(void);

void get_server_name(cbc_comm_line_t *info, cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	char *query, server_id[40];
	const char *cbc_query;
	
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_server_name");
	
	if (info->server_id != 0)
		sprintf(query, "SELECT server_id, name, uuid FROM server WHERE server_id = %ld", info->server_id);
	else if ((strncmp(info->name, "NULL", CONF_S)))
		sprintf(query, "SELECT server_id, name, uuid FROM server WHERE name = '%s'", info->name);
	else if ((strncmp(info->uuid, "NULL", CONF_S)))
		sprintf(query, "SELECT server_id, name, uuid FROM server WHERE uuid = '%s'", info->uuid);
	
	cbc_query = query;
	cbc_mysql_init(config, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	sprintf(server_id, "%ld", info->server_id);
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		if (strncmp(info->name, "NULL", CONF_S))
			report_error(SERVER_NOT_FOUND, info->name);
		else if (strncmp(info->uuid, "NULL", CONF_S))
			report_error(SERVER_UUID_NOT_FOUND, info->uuid);
		else if (info->server_id > 1)
			report_error(SERVER_ID_NOT_FOUND, server_id);
		else
			report_error(NO_NAME_UUID_ID, server_id);
	} else if (cbc_rows > 1) {
		mysql_free_result(cbc_res);
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MULTIPLE_SERVERS, info->name);
	}
	cbc_row = mysql_fetch_row(cbc_res);
	info->server_id = strtoul(cbc_row[0], NULL, 10);
	strncpy(info->name, cbc_row[1], CONF_S);
	strncpy(info->uuid, cbc_row[2], CONF_S);
	
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	free(query);
}

int get_build_info(cbc_build_t *build_info, cbc_config_t *config, unsigned long int server_id)
{
	MYSQL build;
	MYSQL_RES *build_res;
	MYSQL_ROW build_row;
	my_ulonglong build_rows;
	int retval;
	char *query;
	char sserver_id[40];
	const char *build_query;
	
	retval = NONE;
	sprintf(sserver_id, "%ld", server_id);
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_build_info");
	build_info->server_id = server_id;
	
	sprintf(query,
"SELECT arch, bo.alias, os_version, INET_NTOA(ip), mac_addr,\
 INET_NTOA(netmask), INET_NTOA(gateway), INET_NTOA(ns), hostname, domainname,\
 boot_line, valias,ver_alias, build_type, arg, url, country, locale, language,\
 keymap, net_inst_int, mirror, config_ntp, ntp_server, device, lvm FROM\
 build_ip bi LEFT JOIN (build_domain bd, build_os bo, build b, build_type bt,\
 server s, \
 boot_line bootl, varient v, locale l, disk_dev dd) ON (bi.bd_id = bd.bd_id \
 AND bt.bt_id = bootl.bt_id AND dd.server_id = s.server_id AND\
 b.ip_id = bi.ip_id AND bo.os_id = b.os_id AND s.server_id = b.server_id AND \
 bootl.boot_id = bo.boot_id AND b.varient_id = v.varient_id AND \
 l.os_id = b.os_id) WHERE s.server_id = %ld", server_id);
	build_query = query;
	cbc_mysql_init(config, &build);
	cmdb_mysql_query(&build, build_query);
	if (!(build_res = mysql_store_result(&build))) {
		mysql_close(&build);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&build));
	}
	if (((build_rows = mysql_num_rows(build_res)) == 0)){
		mysql_close(&build);
		mysql_library_end();
		free(query);
		report_error(SERVER_BUILD_NOT_FOUND, sserver_id);
	} else if (build_rows > 1) {
		mysql_free_result(build_res);
		mysql_close(&build);
		mysql_library_end();
		free(query);
		report_error(MULTIPLE_SERVER_BUILDS, sserver_id);
	}
	build_row = mysql_fetch_row(build_res);
	fill_build_info(build_info, build_row);
	mysql_free_result(build_res);
	mysql_close(&build);
	mysql_library_end();
	free(query);
	return retval;
}

void write_tftp_config(cbc_config_t *cct, cbc_build_t *cbt)
{
	uint32_t ip_addr;
	size_t len;
	char ip_address[16];
	char hex_ip[10];
	char filename[TBUFF_S];
	char *file_content;
	char *tmp;
	
	if (!(file_content = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "file_content in write_build_config");
	if (!(tmp = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in write_build_config");

	sprintf(ip_address, "%s", cbt->ip_address);
	inet_pton(AF_INET, ip_address, &ip_addr);
	ip_addr = htonl(ip_addr);
	sprintf(hex_ip, "%X", ip_addr);
	sprintf(filename, "%s%s%s", cct->tftpdir, cct->pxe, hex_ip);
	sprintf(tmp, "default %s\n\nlabel %s\nkernel vmlinuz-%s-%s-%s\n",
		cbt->hostname,
		cbt->hostname,
		cbt->alias,
		cbt->version,
		cbt->arch);
	len = strlen(tmp);
	strncpy(file_content, tmp, len);
	if ((strncmp("none", cbt->boot, CONF_S)) != 0) {
		add_append_line(cbt, file_content, tmp);
	} else {
		sprintf(tmp, "append initrd=initrd-%s-%s-%s.img\n",
		cbt->alias,
		cbt->version,
		cbt->arch);
		len = strlen(tmp);
		strncat(file_content, tmp, len);
	}
	write_config_file(filename, file_content);
	printf("TFTP file written\n");
	free(file_content);
	free(tmp);
}

void add_append_line(cbc_build_t *cbt, char *output, char *tmp)
{
	size_t len;
	if ((strncmp(cbt->build_type, "preseed", RANGE_S) == 0)) {
		sprintf(tmp, "append initrd=initrd-%s-%s-%s.img country=%s ",
			cbt->alias,
			cbt->version,
			cbt->arch,
			cbt->country);
		len = strlen(tmp);
		strncat(output, tmp, len);
		sprintf(tmp, "locale=%s keymap=%s %s %s=%s%s.cfg\n",
			cbt->locale,
			cbt->keymap,
			cbt->boot,
			cbt->arg,
			cbt->url,
			cbt->hostname);
		len = strlen(tmp);
		strncat(output, tmp, len);
	}
}

void fill_build_info(cbc_build_t *cbt, MYSQL_ROW br)
{
	sprintf(cbt->arch, "%s", br[0]);
	sprintf(cbt->alias, "%s", br[1]);
	sprintf(cbt->version, "%s", br[2]);
	sprintf(cbt->ip_address, "%s", br[3]);
	sprintf(cbt->mac_address, "%s", br[4]);
	sprintf(cbt->netmask, "%s", br[5]);
	sprintf(cbt->gateway, "%s", br[6]);
	sprintf(cbt->nameserver, "%s", br[7]);
	sprintf(cbt->hostname, "%s", br[8]);
	sprintf(cbt->domain, "%s", br[9]);
	sprintf(cbt->boot, "%s", br[10]);
	sprintf(cbt->varient, "%s", br[11]);
	sprintf(cbt->ver_alias, "%s", br[12]);
	sprintf(cbt->build_type, "%s", br[13]);
	sprintf(cbt->arg, "%s", br[14]);
	sprintf(cbt->url, "%s", br[15]);
	sprintf(cbt->country, "%s", br[16]);
	sprintf(cbt->locale, "%s", br[17]);
	sprintf(cbt->language, "%s", br[18]);
	sprintf(cbt->keymap, "%s", br[19]);
	sprintf(cbt->netdev, "%s", br[20]);
	sprintf(cbt->mirror, "%s", br[21]);
	cbt->config_ntp = ((strncmp(br[22], "0", CH_S)));
	if (br[23])
		sprintf(cbt->ntpserver, "%s", br[23]);
	sprintf(cbt->diskdev, "%s", br[24]);
	cbt->use_lvm = ((strncmp(br[25], "0", CH_S)));
}

void write_dhcp_config(cbc_config_t *cct, cbc_build_t *cbt)
{
	size_t len;
	char *dhcp_content;
	char *dhcp_new_content;
	char buff[RBUFF_S];
	long int dhcp_size;
	struct stat dhcp_stat;
	
	stat(cct->dhcpconf, &dhcp_stat);

	dhcp_size = dhcp_stat.st_size;
	
	if (!(dhcp_content = calloc((size_t)dhcp_size + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "dhcp_content in write_dhcp_config");
	if (!(dhcp_new_content = calloc((size_t)dhcp_size + TBUFF_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "dhcp_new_content in write_dhcp_config");
	
	read_dhcp_hosts_file(cbt, cct->dhcpconf, dhcp_content, dhcp_new_content);
	
	sprintf(buff, "host %s { hardware ethernet %s; fixed-address %s; option domain-name \"%s\"; }\n",
		cbt->hostname,
		cbt->mac_address,
		cbt->ip_address,
		cbt->domain);
	len = strlen(buff);
	strncat (dhcp_new_content, buff, len);
	write_config_file(cct->dhcpconf, dhcp_new_content);
	
	printf("DHCP file written\n");
	free(dhcp_content);
	free(dhcp_new_content);
}

void write_config_file(char *filename, char *output)
{
	FILE *configfile;
	if (!(configfile = fopen(filename, "w"))) {
		report_error(FILE_O_FAIL, filename);
	} else {
		fputs(output, configfile);
		fclose(configfile);
	}
}

void read_dhcp_hosts_file(cbc_build_t *cbcbt, char *from, char *content, char *new_content)
{
	FILE *dhcp;
	size_t len, conlen;
	time_t now;
	struct tm *lctime;
	int success;
	char time_string[18];
	char file_to[CONF_S];
	char buff[TBUFF_S];
	
	now = time(0);
	lctime = localtime(&now);
	sprintf(time_string, "%d%d%d%d%d%d",
		lctime->tm_year + 1900,
		lctime->tm_mon + 1,
		lctime->tm_mday,
		lctime->tm_hour,
		lctime->tm_min,
		lctime->tm_sec);
	
	sprintf(file_to, "%s-%s", from, time_string);
	if (!(dhcp = fopen(from, "r")))
		report_error(FILE_O_FAIL, from);
	while (fgets(buff, TBUFF_S, dhcp)) {
		len = strlen(buff) + 1;
		if ((conlen = strlen(content) == 0))
			strncpy(content, buff, len);
		else
			strncat(content, buff, len);
		if (!(strstr(buff, cbcbt->hostname))) {
			if (!(strstr(buff, cbcbt->ip_address))) {
				if (!(strstr(buff, cbcbt->mac_address))) {
					if ((conlen = strlen(new_content) == 0))
						strncpy(new_content, buff, len);
					else
						strncat(new_content, buff, len);
				}
			}
		}
	}
	fclose(dhcp);
	success = rename(from, file_to);
	if (success < 0)
		printf("Error backing up old config! Check permissions!\n");
}

int write_build_config(cbc_config_t *cmc, cbc_build_t *cbt)
{
	int retval;
	retval = 0;
	if ((strncmp(cbt->build_type, "preseed", RANGE_S) == 0))
		retval = write_preseed_config(cmc, cbt);
	else if ((strncmp(cbt->build_type, "kickstart", RANGE_S) == 0))
		retval = write_kickstart_config(cmc, cbt);
	return retval;
}

int write_preseed_config(cbc_config_t *cmc, cbc_build_t *cbt)
{
	size_t len, total;
	int retval;
	char *output;
	char *tmp;
	
	len = total = 0;
	retval = 0;
	
	if (!(output = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "output in write_preseed_config");
	if (!(tmp = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in write_preseed_config");
	
	snprintf(tmp, FILE_S,
	"d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string %s\n\
d-i debian-installer/language string %s\n\
d-i console-keymaps-at/keymap select %s\n\
d-i keymap select %s\n",
		cbt->locale,
		cbt->language,
		cbt->keymap,
		cbt->keymap);
	len = strlen(tmp);
	if (len + total < BUILD_S) {
		strncat(output, tmp, FILE_S);
		total = strlen(output);
	} else {
		free(output);
		free(tmp);
		retval = BUFFER_FULL;
		return retval;
	}
	
	snprintf(tmp, FILE_S,
	"d-i preseed/early_command string /bin/killall.sh; /bin/netcfg\n\
d-i netcfg/enable boolean true\n\
d-i netcfg/confirm_static boolean true\n\
d-i netcfg/disable_dhcp boolean true\n\
d-i netcfg/choose_interface select %s\n\
d-i netcfg/get_nameservers string %s\n\
d-i netcfg/get_ipaddress string %s\n\
d-i netcfg/get_netmask string %s\n\
d-i netcfg/get_gateway string %s\n\
d-i netcfg/get_hostname string %s\n\
d-i netcfg/get_domain string %s\n",
		cbt->netdev,
		cbt->nameserver,
		cbt->ip_address,
		cbt->netmask,
		cbt->gateway,
		cbt->hostname,
		cbt->domain);
	len = strlen(tmp);
	if (len + total < BUILD_S) {
		strncat(output, tmp, FILE_S);
		total = strlen(output);
	} else {
		free(output);
		free(tmp);
		retval = BUFFER_FULL;
		return retval;
	}
	
	snprintf(tmp, FILE_S,
	"d-i netcfg/wireless_wep string\n\
d-i hw-detect/load_firmware boolean true\n\
d-i mirror/country string manual\n\
d-i mirror/http/hostname string %s\n\
d-i mirror/http/directory string /debian\n\
d-i mirror/suite string %s\n\
d-i passwd/root-password-crypted password $1$d/0w8MHb$tdqENqvXIz53kZp2svuak1\n\
d-i passwd/user-fullname string Monkey User\n\
d-i passwd/username string monkey\n\
d-i passwd/user-password-crypted password $1$Hir6Ul13$.T1tAO.yfK5g7WDKSw0nI/\n\
d-i clock-setup/utc boolean true\n\
d-i time/zone string %s\n",
		cbt->mirror,
		cbt->ver_alias,
		cbt->country);
	len = strlen(tmp);
	if (len + total < BUILD_S) {
		strncat(output, tmp, FILE_S);
		total = strlen(output);
	} else {
		free(output);
		free(tmp);
		retval = BUFFER_FULL;
		return retval;
	}
	if (cbt->config_ntp == 0)
		snprintf(tmp, HOST_S, "d-i clock-setup/ntp boolean false");
	else
		snprintf(tmp, RBUFF_S, "d-i clock-setup/ntp boolean \
true\nd-i clock-setup/ntp-server string %s\n",
			cbt->ntpserver);
	len = strlen(tmp);
	if (len + total < BUILD_S) {
		strncat(output, tmp, FILE_S);
		total = strlen(output);
	} else {
		free(output);
		free(tmp);
		retval = BUFFER_FULL;
		return retval;
	}
	snprintf(tmp, FILE_S, "d-i partman-auto/disk string %s\n",
		 cbt->diskdev);
	len = strlen(tmp);
	if (len + total < BUILD_S) {
		strncat(output, tmp, FILE_S);
		total = strlen(output);
	} else {
		free(output);
		free(tmp);
		retval = BUFFER_FULL;
		return retval;
	}
	retval = write_disk_preconfig(cmc, cbt, output, tmp);
	if (retval == BUFFER_FULL) {
		free(tmp);
		free(output);
		return retval;
	}
	/* Hard coded arch. Maybe setup DB entry?? */
	if (strncmp(cbt->arch, "i386", COMM_S) == 0)
		snprintf(tmp, HOST_S, "d-i base-installer/kernel/image string linux-image-2.6-686\n");
	else if (strncmp(cbt->arch, "x86_64", COMM_S) ==0)
		snprintf(tmp, HOST_S, "d-i base-installer/kernel/image string linux-image-2.6-amd64\n");
	len = strlen(tmp);
	total = strlen(output);
	if ((total + len) > BUILD_S) {
		free(tmp);
		free(output);
		return BUFFER_FULL;
	} else {
		strncat(output, tmp, len);
	}
	snprintf(tmp, FILE_S,
"d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
d-i apt-setup/services-select multiselect security, volatile\n\
d-i apt-setup/security_host string security.debian.org\n\
d-i apt-setup/volatile_host string volatile.debian.org\n\
tasksel tasksel/first multiselect standard, web-server\n\
d-i pkgsel/include string ");
	retval = add_preseed_packages(cmc, cbt, output, tmp);
	if (retval == BUFFER_FULL) {
		free(tmp);
		free(output);
		return retval;
	}

	printf("%s", output);
	free(tmp);
	free(output);
	return retval;
}

int write_disk_preconfig(
			cbc_config_t *cmc,
			cbc_build_t *cbt,
			char *output,
			char *tmp)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	pre_disk_part_t *node, *saved;
	pre_disk_part_t *head_part = 0;
	char *query;
	char sserver_id[HOST_S];
	const char *cbc_query;
	int retval, parti;
	retval = parti = 0;
	if (cbt->use_lvm)
		retval = write_lvm_preheader(output, tmp, cbt->diskdev);
	else
		retval = write_regular_preheader(output, tmp);
	if (retval > 0) {
		mysql_library_end();
		return retval;
	}
	snprintf(sserver_id, HOST_S - 1, "%ld", cbt->server_id);
	
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in write_reg_disk_preconfig");
	snprintf(query, BUFF_S - 1,
"SELECT minimum, maximum, priority, mount_point, filesystem, part_id, \
logical_volume FROM partitions WHERE server_id = %ld ORDER BY mount_point\n",
cbt->server_id);
	cbc_query = query;
	cbc_mysql_init(cmc, &cbc);
	cmdb_mysql_query(&cbc, cbc_query);
	if (!(cbc_res = mysql_store_result(&cbc))) {
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)){
		mysql_close(&cbc);
		mysql_library_end();
		free(query);
		report_error(SERVER_PART_NOT_FOUND, sserver_id);
	}
	while ((cbc_row = mysql_fetch_row(cbc_res))){
		head_part = part_node_add(head_part, cbc_row);
	}
	mysql_free_result(cbc_res);
	mysql_close(&cbc);
	mysql_library_end();
	node = head_part;
	while ((node)) {
		parti = check_for_special_partition(node);
		retval = add_partition_to_preseed(
			node,
			output,
			tmp,
			parti,
			cbt->use_lvm);
		if (retval == BUFFER_FULL)
			return BUFFER_FULL;
		node = node->nextpart;
	}
	strncat(output, "\n\n", 3);
	for (node = head_part; node; ){
		saved = node->nextpart;
		free(node);
		node = saved;
	}
	free(node);
	free(query);
	return retval;
}
int write_regular_preheader(char *output, char *tmp)
{
	size_t len, full_len;
	snprintf(tmp, FILE_S,
"d-i partman-auto/method string regular\n\
d-i partman-auto/purge_lvm_from_device boolean true\n\
d-i partman-auto-lvm/guided_size    string 100%%\n\
d-i partman-lvm/device_remove_lvm boolean true\n\
d-i partman-lvm/device_remove_lvm_span boolean true\n\
d-i partman-lvm/confirm boolean true\n\
d-i partman-auto/choose_recipe select monkey\n\
d-i partman-md/device_remove_md boolean true\n\
d-i partman-partitioning/confirm_write_new_label boolean true\n\
d-i partman/confirm_nooverwrite boolean true\n\
d-i partman-lvm/confirm_nooverwrite boolean true\n\
d-i partman/choose_partition select Finish partitioning and write changes to disk\n\
d-i partman/confirm boolean true\n\
\n\
d-i partman-auto/expert_recipe string                         \\\n\
      monkey ::                                               \\");
	full_len = strlen(output);
	len = strlen(tmp);
	if ((len + full_len) < BUILD_S)
		strncat(output, tmp, len);
	else
		return BUFFER_FULL;
	
	return 0;
}

int write_lvm_preheader(char *output, char *tmp, char *device)
{
	size_t len, full_len;
	snprintf(tmp, FILE_S, 
"d-i partman-auto/method string lvm\n\
d-i partman-auto/purge_lvm_from_device boolean true\n\
d-i partman-auto-lvm/guided_size    string 100%%\n\
d-i partman-lvm/device_remove_lvm boolean true\n\
d-i partman-lvm/device_remove_lvm_span boolean true\n\
d-i partman-lvm/confirm boolean true\n\
d-i partman-auto/choose_recipe select monkey\n\
d-i partman-md/device_remove_md boolean true\n\
d-i partman-partitioning/confirm_write_new_label boolean true\n\
d-i partman/confirm_nooverwrite boolean true\n\
d-i partman-lvm/confirm_nooverwrite boolean true\n\
d-i partman/choose_partition select Finish partitioning and write changes to disk\n\
d-i partman/confirm boolean true\n\
\n\
d-i partman-auto/expert_recipe string                         \\\n\
      monkey ::                                               \\\n\
              100 1000 1000000000 ext3                        \\\n\
                       $defaultignore{ }                      \\\n\
                       $primary{ }                            \\\n\
                       method{ lvm }                          \\\n\
                       device{ %s }                     \\\n\
                       vg_name{ systemlv }                    \\", device);
	full_len = strlen(output);
	len = strlen(tmp);
	if ((len + full_len) < BUILD_S)
		strncat(output, tmp, len);
	else
		return BUFFER_FULL;
	
	return 0;
}

pre_disk_part_t *part_node_create(void)
{
	pre_disk_part_t *dptr;
	
	if (!(dptr = malloc(sizeof(pre_disk_part_t))))
		report_error(MALLOC_FAIL, "disk_part in part_node_create");
	dptr->nextpart = 0;
	dptr->min = dptr->max = dptr->pri = 0;
	sprintf(dptr->mount_point, "NULL");
	sprintf(dptr->filesystem, "NULL");
	return dptr;
}

pre_disk_part_t *part_node_add(pre_disk_part_t *head_node, MYSQL_ROW part_row)
{
	pre_disk_part_t *new_node, *node;
/*	new_node = part_node_create(); */
	if (!(new_node = malloc(sizeof(pre_disk_part_t))))
		report_error(MALLOC_FAIL, "disk_part in part_node_create");
	
	new_node->min = strtoul(part_row[0], NULL, 10);
	new_node->max = strtoul(part_row[1], NULL, 10);
	new_node->pri = strtoul(part_row[2], NULL, 10);
	snprintf(new_node->mount_point, HOST_S, "%s", part_row[3]);
	snprintf(new_node->filesystem, RANGE_S, "%s", part_row[4]);
	new_node->part_id = strtoul(part_row[5], NULL, 10);
	snprintf(new_node->log_vol, RANGE_S, "%s", part_row[6]);
	new_node->nextpart = NULL;
	if (!head_node) {
		head_node = new_node;
	} else {
		for (node = head_node; node->nextpart; node=node->nextpart) {
			;
		}
		node->nextpart = new_node;
	}
	return head_node;
}

int check_for_special_partition(pre_disk_part_t *part)
{
	int retval;
	retval = NONE;
	if (strncmp(part->mount_point, "/boot", COMM_S) == 0)
		retval = BOOT;
	else if (strncmp(part->mount_point, "/", COMM_S) ==0)
		retval = ROOT;
	else if (strncmp(part->mount_point, "swap", COMM_S) == 0)
		retval = SWAP;
	
	return retval;
}

int
add_partition_to_preseed(pre_disk_part_t *part, char *output, char *buff, int special, int lvm)
{
	int retval;
	size_t len, total_len;
	len = total_len = 0;
	retval = NONE;
	
	switch (special) {
		case ROOT: case NONE:
			if (lvm > 0) {
			snprintf(buff, FILE_S - 1,
"\n\t%ld %ld %ld %s\t\t\t\\\n\
\t$lvmok\t\t\t\t\\\n\
\tin_vg{ systemlv }\t\t\t\\\n\
\tlv_name{ %s }\t\t\t\\\n\
\tmethod{ format } format{ }\t\t\t\\\n\
\tuse_filesystem{ } filesystem{ %s }\t\t\\\n\
\tmountpoint{ %s }\t\t\t\\\n\t. \\",
	part->min,
	part->pri,
	part->max,
	part->filesystem,
	part->log_vol,
	part->filesystem,
	part->mount_point);
			} else {
			snprintf(buff, FILE_S - 1,
"\n\t%ld %ld %ld %s\t\t\t\\\n\
\tmethod{ format } format{ }\t\t\t\\\n\
\tuse_filesystem{ } filesystem{ %s }\t\t\\\n\
\tmountpoint{ %s }\t\t\t\\\n\t. \\",
	part->min,
	part->pri,
	part->max,
	part->filesystem,
	part->filesystem,
	part->mount_point);
			}
			break;
		case BOOT:
			snprintf(buff, FILE_S - 1,
"\n\t%ld %ld %ld %s\t\t\t\\\n\
\t$primary{ } $bootable{ }\t\t\t\\\n\
\tmethod{ format } format{ }\t\t\t\\\n\
\tuse_filesystem{ } filesystem{ %s }\t\t\\\n\
\tmountpoint{ %s }\t\t\t\t\\\n\t. \\",
	part->min,
	part->pri,
	part->max,
	part->filesystem,
	part->filesystem,
	part->mount_point);
			break;
		case SWAP:
			if (lvm > 0) {
			snprintf(buff, FILE_S - 1,
"\n\t%ld %ld %ld%% linux-swap\t\t\t\\\n\
\t$lvmok\t\t\t\t\\\n\
\tin_vg{ systemlv }\t\t\t\\\n\
\tlv_name{ swap }\t\t\t\\\n\
\tmethod{ swap } format{ }\t\t\t\\\n\t. \\",
	part->min,
	part->pri,
	part->max);
			} else {
			snprintf(buff, FILE_S - 1,
"\n\t%ld %ld %ld%% linux-swap\t\t\t\t\\\n\
\tmethod{ swap } format{ }\t\t\t\\\n\t. \\",
	part->min,
	part->pri,
	part->max);
			}
			break;
		default:
			retval = 1;
			return retval;
			break;
	}
	
	len = strlen(buff);
	total_len = strlen(output);
	if ((len + total_len) > BUILD_S) {
		retval = 1;
	} else {
		strncat(output, buff, len);
		retval = 0;
	}

	return retval;
}

int add_preseed_packages(cbc_config_t *cmc, cbc_build_t *cbt, char *out, char *buff)
{
	int retval;
	char *query;
	const char *cbc_query;
	size_t len, total, full;
	
	retval = 0;
	if(!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_preseed_packages");
	snprintf(query, BUFF_S - 1,
"select package from packages p, varient v WHERE os_id = 40 AND valias = '%s' AND v.varient_id = p.varient_id", cbt->varient);
	len = strlen(buff);
	total = strlen(out);
	
	free(query);
	return retval;
}

int write_kickstart_config(cbc_config_t *cmc, cbc_build_t *cbt)
{
	int retval;
	retval = 0;
	printf("kickstart location: %s\n", cmc->kickstart);
	printf("hostname: %s\n", cbt->hostname);
	return retval;
}