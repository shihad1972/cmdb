/* rev_zone.h: Write_rev_zone header */
#include <mysql.h>
#include "dnsa.h"

#ifndef __REV_ZONE_H__
#define __REV_ZONE_H__

typedef struct rev_zone_info_t { /* Hold DNS zone */
	int rev_zone_id;
	char net_range[16];
	int prefix;
	char net_start[16];
	char net_finish[16];
	unsigned int start_ip;
	unsigned int end_ip;
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	int serial;
	int refresh;
	int retry;
	int expire;
	int ttl;
	char valid[RBUFF_S];
	int owner;
	char updated[RBUFF_S];
	char hostmaster[RBUFF_S];
} rev_zone_info_t;

typedef struct rev_record_row_t { /* Hold dns record */
	char host[256];
	char dest[256];
} rev_record_row_t;


rev_zone_info_t fill_rev_zone_data(MYSQL_ROW my_row);
size_t create_rev_zone_header(rev_zone_info_t zone_info, char *rout);
rev_record_row_t get_rev_row (MYSQL_ROW my_row);
void add_rev_records(char *rout, rev_record_row_t my_row);
void get_in_addr_string(char *in_addr, char range[]);

#endif