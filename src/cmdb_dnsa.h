/* cmdb_dnsa.h: DNSA header file */

#ifndef __CMDB_DNSA_H__
#define __CMDB_DNSA_H__

enum {			/* zone types; use NONE from action codes */
	FORWARD_ZONE = 1,
	REVERSE_ZONE = 2
};

typedef struct comm_line_t { /* Hold parsed command line args */
	short int action;
	short int type;
	unsigned long int prefix;
	char domain[CONF_S];
	char config[CONF_S];
	char host[RBUFF_S];
	char dest[RBUFF_S];
	char rtype[RANGE_S];
} comm_line_t;

typedef struct dnsa_config_t { /* Hold DNSA configuration values */
	char dbtype[RANGE_S];
	char db[CONF_S];
	char file[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char dir[CONF_S];
	char bind[CONF_S];
	char dnsa[CONF_S];
	char rev[CONF_S];
	char rndc[CONF_S];
	char chkz[CONF_S];
	char chkc[CONF_S];
	char socket[CONF_S];
	char hostmaster[RBUFF_S];
	char prins[RBUFF_S];
	char secns[RBUFF_S];
	char pridns[MAC_S];
	char secdns[MAC_S];
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
	unsigned int port;
	unsigned long int cliflag;
} dnsa_config_t;

typedef struct record_row_t { /* Hold dns record */
	char dest[RBUFF_S];
	char host[RBUFF_S];
	char type[RANGE_S];
	char valid[RANGE_S];
	unsigned long int id;
	unsigned long int pri;
	unsigned long int zone;
	unsigned long int ip_addr;
	struct record_row_t *next;
} record_row_t;

typedef struct rev_record_row_t { /* Hold dns record */
	char host[RBUFF_S];
	char dest[RBUFF_S];
	char valid[RANGE_S];
	unsigned long int record_id;
	unsigned long int rev_zone;
	unsigned long int ip_addr;
	struct rev_record_row_t *next;
} rev_record_row_t;

typedef struct zone_info_t { /* Hold DNS zone */
	char name[RBUFF_S];
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	char valid[RANGE_S];
	char updated[RANGE_S];
	char web_ip[RANGE_S];
	char ftp_ip[RANGE_S];
	char mail_ip[RANGE_S];
	unsigned long int id;
	unsigned long int owner;
	unsigned long int serial;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
	struct zone_info_t *next;
} zone_info_t;

typedef struct rev_zone_info_t { /* Hold DNS zone */
	char net_range[RANGE_S];
	char net_start[RANGE_S];
	char net_finish[RANGE_S];
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	char valid[RANGE_S];
	char updated[RANGE_S];
	char hostmaster[RBUFF_S];
	unsigned long int rev_zone_id;
	unsigned long int owner;
	unsigned long int prefix;
	unsigned long int start_ip;
	unsigned long int end_ip;
	unsigned long int serial;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
	struct rev_zone_info_t *next;
} rev_zone_info_t;

typedef struct preferred_a_t { /* Hold the preferred A records for reverse */
	unsigned long int prefa_id;
	unsigned long int ip_addr;
	unsigned long int record_id;
	char ip[RANGE_S];
	char fqdn[RBUFF_S];
	struct preferred_a_t *next;
} preferred_a_t;

typedef struct zone_file_t {
	char out[RBUFF_S];
	struct zone_file_t *next;
} zone_file_t;

typedef struct dnsa_config_and_reverse {
	dnsa_config_t *dc;
	rev_record_row_t *record;
	rev_zone_info_t *zone;
} dnsa_config_and_reverse;

typedef struct dnsa_t {
	struct zone_info_t *zones;
	struct rev_zone_info_t *rev_zones;
	struct record_row_t *records;
	struct rev_record_row_t *rev_records;
	struct preferred_a_t *prefer;
	struct zone_file_t *file;
} dnsa_t;

typedef union dbdata_u {
	char text[256];
	unsigned long int number;
} dbdata_u;

typedef struct dbdata_t {
	union dbdata_u fields;
	union dbdata_u args;
	struct dbdata_t *next;
} dbdata_t;

/* Get command line args and pass them. Put actions into the struct */
int
parse_dnsa_command_line(int argc, char **argv, comm_line_t *comm);
/* Grab config values from file */
int
parse_dnsa_config_file(dnsa_config_t *dc, char *config);
/*initialise configuration struct */
void
init_config_values(dnsa_config_t *dc);
void
parse_dnsa_config_error(int error);
/* Validate command line input */
int
validate_comm_line(comm_line_t *comm);
/* Struct initialisation and clean functions */
void
init_dnsa_struct(dnsa_t *dnsa);
void
init_zone_struct(zone_info_t *zone);
void
init_rev_zone_struct(rev_zone_info_t *revzone);
void
init_record_struct(record_row_t *record);
void
init_rev_record_struct(rev_record_row_t *revrecord);
void
init_preferred_a_struct(preferred_a_t *prefer);
void
init_dbdata_struct(dbdata_t *data);
void
init_initial_dbdata(dbdata_t **list, int type);
void
dnsa_clean_list(dnsa_t *dnsa);
void
dnsa_clean_zones(zone_info_t *zone);
void
dnsa_clean_rev_zones(rev_zone_info_t *rev);
void
dnsa_clean_records(record_row_t *rec);
void
dnsa_clean_rev_records(rev_record_row_t *rev);
void
dnsa_clean_prefer(preferred_a_t *list);
void
dnsa_clean_dbdata_list(dbdata_t *data);
/* Zone action Functions */
int
add_fwd_zone(dnsa_config_t *dc, comm_line_t *cm);
int
add_rev_zone(dnsa_config_t *dc, comm_line_t *cm);
int
commit_fwd_zones(dnsa_config_t *dc);
int
commit_rev_zones(dnsa_config_t *dc);
int
add_host(dnsa_config_t *dc, comm_line_t *cm);
int
display_multi_a_records(dnsa_config_t *dc, comm_line_t *cm);
int
mark_preferred_a_record(dnsa_config_t *dc, comm_line_t *cm);
int
build_reverse_zone(dnsa_config_t *dc, comm_line_t *cm);
int
get_correct_rev_zone_and_preferred_records(comm_line_t *cm, dnsa_t *dnsa);
int
get_rev_zone(comm_line_t *cm, dnsa_t *dnsa);
void
get_duplicate_a_records(comm_line_t *cm, dnsa_t *dnsa);
int
get_rev_records_for_range(rev_record_row_t **rev, rev_zone_info_t *zone);
/* Zone display functions */
void
list_zones(dnsa_config_t *dc);
void
list_rev_zones(dnsa_config_t *dc);
void
display_zone(char *domain, dnsa_config_t *dc);
void
print_zone(dnsa_t *dnsa, char *domain);
void
display_rev_zone(char *domain, dnsa_config_t *dc);
void
print_rev_zone(dnsa_t *dnsa, char *domain);
void
print_multiple_a_records(dnsa_config_t *dc, dbdata_t *data, dnsa_t *dnsa);
int
get_preferred_a_record(dnsa_config_t *dc, comm_line_t *cm, dnsa_t *dnsa);
/* Various zone functions */
void
get_in_addr_string(char *in_addr, char range[], unsigned long int prefix);
unsigned long int
get_zone_serial(void);
int
check_for_zone_in_db(dnsa_config_t *dc, dnsa_t *dnsa, short int type);
void
select_specific_ip(dnsa_t *dnsa, comm_line_t *cm);
int
get_a_records_for_range(record_row_t **records, rev_zone_info_t *zone);
int
get_pref_a_for_range(preferred_a_t **prefer, rev_zone_info_t *rev);
void
add_int_ip_to_records(dnsa_t *dnsa);
void
add_int_ip_to_fwd_records(record_row_t *records);
int
add_int_ip_to_rev_records(dnsa_t *dnsa);
/* Forward zone functions */
int
check_fwd_zone(char *domain, dnsa_config_t *dc);
int
create_and_write_fwd_zone(dnsa_t *dnsa, dnsa_config_t *dc, zone_info_t *zone);
int
create_fwd_config(dnsa_config_t *dc, zone_info_t *zone, char *config);
void
create_fwd_zone_header(dnsa_t *dnsa, char *hostm, unsigned long int id, char *zonfile);
void
add_records_to_fwd_zonefile(dnsa_t *dnsa, unsigned long int id, char **zonefile);
int
create_and_write_fwd_config(dnsa_config_t *dc, dnsa_t *dnsa);
void
check_for_updated_fwd_zone(dnsa_config_t *dc, zone_info_t *zone);
int
validate_fwd_zone(dnsa_config_t *dc, zone_info_t *zone, dnsa_t *dnsa);
void
fill_fwd_zone_info(zone_info_t *zone, comm_line_t *cm, dnsa_config_t *dc);
/* Reverse zone functions */
void 
print_rev_zone_info(rev_zone_info_t *zone);
int
create_and_write_rev_zone(dnsa_t *dnsa, dnsa_config_t *dc, rev_zone_info_t *zone);
void
create_rev_zone_header(dnsa_t *dnsa, char *hostm, unsigned long int id, char *zonefile);
void
add_records_to_rev_zonefile(dnsa_t *dnsa, unsigned long int id, char **zonefile);
int
check_rev_zone(char *domain, dnsa_config_t *dc);
int
create_rev_config(dnsa_config_t *dc, rev_zone_info_t *zone, char *configfile);
void
fill_rev_zone_info(rev_zone_info_t *zone, comm_line_t *cm, dnsa_config_t *dc);
unsigned long int
get_net_range(unsigned long int prefix);
int
validate_rev_zone(dnsa_config_t *dc, rev_zone_info_t *zone, dnsa_t *dnsa);
int
create_and_write_rev_config(dnsa_config_t *dc, dnsa_t *dnsa);
int
rev_records_to_add(dnsa_t *dnsa, record_row_t *rec, rev_record_row_t **rev);
void
add_a_to_duplicate(record_row_t **dups, record_row_t *list);
int
insert_into_rev_add_list(dnsa_t *dnsa, record_row_t *fwd, rev_record_row_t **rev);
void
add_dup_to_prefer_list(dnsa_t *dnsa, record_row_t *fwd);
int
check_for_duplicate(char *destination, record_row_t *duplicates);
int
get_rev_host(unsigned long int prefix, char *rev_dest, char *dest);
#endif /* __CMDB_DNSA_H__ */
