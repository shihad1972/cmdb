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
	unsigned int port;
	unsigned long int cliflag;
} dnsa_config_t;

typedef struct record_row_t { /* Hold dns record */
	int id;
	int pri;
	int zone;
	char dest[RBUFF_S];
	char host[RBUFF_S];
	char type[RBUFF_S];
	char valid[RBUFF_S];
	struct record_row_t *next;
} record_row_t;

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
	struct zone_info_t *next;
} zone_info_t;

typedef struct rev_zone_info_t { /* Hold DNS zone */
	int rev_zone_id;
	int owner;
	char net_range[RANGE_S];
	char net_start[RANGE_S];
	char net_finish[RANGE_S];
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	char valid[RBUFF_S];
	char updated[RBUFF_S];
	char hostmaster[RBUFF_S];
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

typedef struct rev_record_row_t { /* Hold dns record */
	char host[RBUFF_S];
	char dest[RBUFF_S];
	char valid[RBUFF_S];
	unsigned long int rev_zone;
	struct rev_record_row_t *next;
} rev_record_row_t;

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
} dnsa_t;

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
/* Struct initialisation functions */
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

#endif /* __CMDB_DNSA_H__ */
