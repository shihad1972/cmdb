/* reverse.h: Reverse zone functions and data structures header */
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"

#ifndef __REV_ZONE_H__
#define __REV_ZONE_H__

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
/* Get all the A records to build a reverse zone */
int
store_valid_a_record(dnsa_config_and_reverse *config, MYSQL_ROW row);
/* Store single A record for the reverse zone */
int
store_a_record(rev_record_row_t *record, MYSQL_ROW row);
/* Do we already have a reverse record for this IP address? */
int
check_if_a_record_exists(rev_record_row_t *record, char *ip);
/* Delete linked list of A records */
void
delete_A_records(rev_record_row_t *records);
/* Insert the reverse records into the database */
int
insert_rev_records(dnsa_config_t *dc, rev_record_row_t *records, rev_zone_info_t *zone);
/* Create the in-addr.arpa zonename from network address */
/*void
get_in_addr_string(char *in_addr, char range[]); */
void
get_in_addr_string2(char *in_addr, char *range, unsigned long int prefix);
/* Return the ID of the reverse domain; -1 indicates error */
int
get_rev_id(char *domain, dnsa_config_t *dc, short int action);
/* Create the string for the reverse zonefile filename */
void 
create_rev_zone_filename (char *domain, char *net_range, dnsa_config_t *dc);
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
/* Add reverse zone into database */
void
add_rev_zone(char *domain, dnsa_config_t *dc, unsigned long int prefix);
/* Initialise the rev_zone_info_t struct */
void
init_dnsa_rev_zone(rev_zone_info_t *rev_zone);
/* Initialise the rev_record_row_t struct */
void
init_dnsa_rev_record(rev_record_row_t *rev_record);
/* Print values of rev_zone_info_t struct */
void
print_rev_zone_info(rev_zone_info_t *rzi);
/* Fill rev_zone_info_t from mysql data */
void
add_rev_config(MYSQL_ROW mr, rev_zone_info_t *rzi);
/* Calculate network range values for rev_zone_info_t struct */
void
fill_range_info(rev_zone_info_t *rz);
/* Work out network range */
unsigned long int
get_net_range(unsigned long int prefix);
/* Update serial on reverse zone */
void
update_rev_zone_serial(rev_zone_info_t *zone);
/* Build the rev zone from forward IP's */
int
build_rev_zone(dnsa_config_t *dc, char *domain);
/* Get full Reverse zone information */
int
get_rev_zone_info(dnsa_config_t *dc, rev_zone_info_t *rev);
/* Get the list of reverse zone records */
int
get_rev_zone_records(dnsa_config_t *dc, rev_zone_info_t *rev, rev_record_row_t *records);
/* Convert dest from full IP address to DB insert ready for reverse zone */
int
convert_rev_records(rev_record_row_t *records, unsigned long int prefix);
#endif
