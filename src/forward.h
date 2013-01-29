/* forward.h: Function and data definitions for forward zones */

#ifndef	__WRITE_ZONE_H__
#define __WRITE_ZONE_H__
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"

typedef struct zone_info_t { /* Hold DNS zone */
	int id;
	int owner;
	char name[RBUFF_S];
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	char valid[RBUFF_S];
	char updated[RBUFF_S];
	char web_ip[RANGE_S];
	char ftp_ip[RANGE_S];
	char mail_ip[RANGE_S];
	unsigned long int serial;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
} zone_info_t;

/* Return struct containing the DNS zone data */
zone_info_t
fill_zone_data(MYSQL_ROW my_row);

/* Return struct containing DNS Record data */
/* Hopefully soon to be obselete */
record_row_t
fill_record_data(MYSQL_ROW my_row);

/* Initialise strcut with new zone data */
void
init_dnsa_zone(zone_info_t *dnsa_zone);

/* Fill zone configuration from database information */
void
fill_dnsa_config(MYSQL_ROW my_row, zone_info_t *zone);

/* Create the header of the DNS zone in a string */
void
create_zone_header(char *output, zone_info_t *zi, size_t len);

/* Add the MX records to the header string */
void
add_mx_to_header(char *output, MYSQL_ROW results);

/* Add the A records for the NS servers to the zonefile */
void
add_ns_A_records_to_header(zone_info_t *zi, dnsa_config_t *dc, char *out);

/* Add the A records for the MX servers to the zonefile; return no added */
int
add_MX_A_records_to_header(zone_info_t *zi, dnsa_config_t *dc, char *out);

/* Add the individual DNS Records to the zonefile string; return size */
size_t
add_records(record_row_t *recrow, char *output, size_t offset);

/* Check forward zone file for errors */
void
check_fwd_zone(char *filename, char *domain, dnsa_config_t *dc);

/* Write fwd zonefile to filesystem */
void
write_fwd_zonefile(char *filename, char *output);

/* Add forward zone */
void
add_fwd_zone(char *domain, dnsa_config_t *dc);

/* Write out the forward zone file */
int
wzf (char *domain, dnsa_config_t *dc);

/* Display zone data */
int
dzf (char *domain, dnsa_config_t *dc);

/* Write the dnsa forward config file */
int
wcf(dnsa_config_t *dc);

/* List the forward zones in the database */
int
list_zones (dnsa_config_t *dc);

/* Update the forward zone serial number */
void
update_fwd_zone_serial(zone_info_t *zone);
/* Print out new fwd zone config */
void
print_fwd_zone_config(zone_info_t *zone);
/* Insert new forward zone into database */
void
insert_new_fwd_zone(zone_info_t *zone, dnsa_config_t *config);
/* Insert A records for new zone */
void
insert_new_fwd_zone_records(zone_info_t *zone, dnsa_config_t *config);
/* Insert record into database */
void
add_fwd_zone_record(MYSQL *dnsa, unsigned long int zone_id, const char *name, char *dest, const char *type);
void
add_fwd_host(dnsa_config_t *dct, comm_line_t *clt);
#endif
