/* reverse.h: Reverse zone functions and data structures header */
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"

#ifndef __REV_ZONE_H__
#define __REV_ZONE_H__

typedef struct rev_zone_info_t { /* Hold DNS zone */
	int rev_zone_id;
	int prefix;
	int owner;
	char net_range[16];
	char net_start[16];
	char net_finish[16];
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	char valid[RBUFF_S];
	char updated[RBUFF_S];
	char hostmaster[RBUFF_S];
	unsigned long int start_ip;
	unsigned long int end_ip;
	unsigned long int serial;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
} rev_zone_info_t;

typedef struct rev_record_row_t { /* Hold dns record */
	char host[RBUFF_S];
	char dest[RBUFF_S];
} rev_record_row_t;

/* Return the data for one reverse zone */
void
fill_rev_zone_data(MYSQL_ROW my_row, rev_zone_info_t *rzi);
/* Return the size of the string with the zone header */
void
create_rev_zone_header(rev_zone_info_t *zone_info, char *rout);
/* Return data of one reverse (PTR) record */
rev_record_row_t
get_rev_row (MYSQL_ROW my_row);
/* Add the reverse (PTR) record to the output string */
void
add_rev_records(char *rout, rev_record_row_t my_row);
/* Create the in-addr.arpa zonename from network address */
void
get_in_addr_string(char *in_addr, char range[]);
/* Return the ID of the reverse domain; -1 indicates error */
int
get_rev_id(char *domain, dnsa_config_t *dc);
/* Create the string for the reverse zonefile filename */
void 
create_rev_zone_filename (char *domain, const char *net_range, dnsa_config_t *dc);
/* Check the rev zone for errors */
void
check_rev_zone(char *filename, char *domain, dnsa_config_t *dc);
/* Write out the reverse zone file */
int 
wrzf (int id, dnsa_config_t *dc);
/* Display the reverse zone file */
int
drzf (int id, char *domain, dnsa_config_t *dc);
/* Write the dnsa reverse config file */
int
wrcf(dnsa_config_t *dc);
/* List the reverse zones in the database */
int
list_rev_zones (dnsa_config_t *dc);
#endif
