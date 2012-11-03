/* write_zone.h: Test connection to mysql database */

#include <mysql.h>
#ifndef	__WRITE_ZONE_H__
#define __WRITE_ZONE_H__

typedef struct zone_info_t { /* Hold DNS zone */
	int id;
	char name[256];
	char pri_dns[256];
	char sec_dns[256];
	int serial;
	int refresh;
	int retry;
	int expire;
	int ttl;
	char valid[256];
	int owner;
	char updated[256];
} zone_info_t;

typedef struct record_row_t { /* Hold dns record */
	int id;
	int zone;
	char host[256];
	char type[256];
	int pri;
	char dest[256];
	char valid[256];
} record_row_t;
/* Return struct containing the DNS zone data */
zone_info_t
fill_zone_data(MYSQL_ROW my_row);
/* Return struct containing DNS Record data */
record_row_t
fill_record_data(MYSQL_ROW my_row);
/* Create the header of the DNS zone in a string; return size */
size_t
create_zone_header(char *output, zone_info_t);
/* Add the MX records to the header string; return size */
size_t
add_mx_to_header(char *output, size_t offset, MYSQL_ROW results);
/* Add the individual DNS Records to the zonefile string; return size */
size_t
add_records(record_row_t, char *output, size_t offset);
/* Write out the forward zone file */
int
wzf (char *domain, char config[][CONF_S]);
/* Display zone data */
int
dzf (char *domain, char config[][CONF_S]);
#endif
