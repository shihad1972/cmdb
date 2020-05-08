/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_dnsa.h: DNSA header file 
 */

#ifndef __DNSA_DATA_H__
# define __DNSA_DATA_H__
# include <ailsacmdb.h>
# include "base_sql.h"
# include "cmdb.h"

typedef struct dnsa_comm_line_s { /* Hold parsed command line args */
	short int action;
	short int type;
	unsigned long int prefix;
	char *rtype;
	char *ztype;
	char *service;
	char *protocol;
	char *domain;
	char *config;
	char *host;
	char *dest;
	char *master;
	char *glue_ip;
	char *glue_ns;
} dnsa_comm_line_s;

typedef struct record_row_s { /* Hold dns record */
	struct record_row_s *next;
	char dest[RBUFF_S];
	char host[RBUFF_S];
	char type[RANGE_S];
	char valid[RANGE_S];
	char service[RANGE_S];
	char protocol[RANGE_S];
	unsigned long int id;
	unsigned long int pri;
	unsigned long int zone;
	unsigned long int ip_addr;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} record_row_s;

typedef struct rev_record_row_s { /* Hold dns record */
	struct rev_record_row_s *next;
	char host[RBUFF_S];
	char dest[RBUFF_S];
	char valid[RANGE_S];
	unsigned long int record_id;
	unsigned long int rev_zone;
	unsigned long int ip_addr;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} rev_record_row_s;

typedef struct zone_info_s { /* Hold DNS zone */
	struct zone_info_s *next;
	char name[RBUFF_S];
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	char valid[RANGE_S];
	char updated[RANGE_S];
	char web_ip[RANGE_S];
	char ftp_ip[RANGE_S];
	char mail_ip[RANGE_S];
	char type[RANGE_S];
	char master[RBUFF_S];
	unsigned long int id;
	unsigned long int owner;
	unsigned long int serial;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} zone_info_s;

typedef struct rev_zone_info_s { /* Hold DNS zone */
	struct rev_zone_info_s *next;
	char net_range[RANGE_S];
	char net_start[RANGE_S];
	char net_finish[RANGE_S];
	char pri_dns[RBUFF_S];
	char sec_dns[RBUFF_S];
	char valid[RANGE_S];
	char updated[RANGE_S];
	char hostmaster[RBUFF_S];
	char type[RANGE_S];
	char master[RBUFF_S];
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
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} rev_zone_info_s;

typedef struct glue_zone_info_s {
	struct glue_zone_info_s *next;
	char name[RBUFF_S];
	char pri_ns[RBUFF_S];
	char sec_ns[RBUFF_S];
	char pri_dns[RANGE_S];
	char sec_dns[RANGE_S];
	unsigned long int id;
	unsigned long int zone_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
} glue_zone_info_s;

typedef struct preferred_a_s { /* Hold the preferred A records for reverse */
	struct preferred_a_s *next;
	unsigned long int prefa_id;
	unsigned long int ip_addr;
	unsigned long int record_id;
	unsigned long int cuser;
	unsigned long int muser;
	unsigned long int ctime;
	unsigned long int mtime;
	char ip[RANGE_S];
	char fqdn[RBUFF_S];
} preferred_a_s;

typedef struct zone_file_s {
	char out[RBUFF_S];
	struct zone_file_s *next;
} zone_file_s;

typedef struct dnsa_config_and_reverse_s {
	ailsa_cmdb_s *dc;
	rev_record_row_s *record;
	rev_zone_info_s *zone;
} dnsa_config_and_reverse_s;

typedef struct dnsa_s {
	struct zone_info_s *zones;
	struct rev_zone_info_s *rev_zones;
	struct record_row_s *records;
	struct rev_record_row_s *rev_records;
	struct glue_zone_info_s *glue;
	struct preferred_a_s *prefer;
	struct zone_file_s *file;
} dnsa_s;

void
init_dnsa_struct(dnsa_s *dnsa);
void
init_zone_struct(zone_info_s *zone);
void
init_rev_zone_struct(rev_zone_info_s *revzone);
void
init_record_struct(record_row_s *record);
void
init_rev_record_struct(rev_record_row_s *revrecord);
void
init_preferred_a_struct(preferred_a_s *prefer);
void
init_glue_zone_struct(glue_zone_info_s *glu);
void
dnsa_clean_list(dnsa_s *dnsa);
void
dnsa_clean_zones(zone_info_s *zone);
void
dnsa_clean_rev_zones(rev_zone_info_s *rev);
void
dnsa_clean_records(record_row_s *rec);
void
dnsa_clean_rev_records(rev_record_row_s *rev);
void
dnsa_clean_prefer(preferred_a_s *list);
void
dnsa_clean_glue(glue_zone_info_s *glu);
void
dnsa_init_config_values(ailsa_cmdb_s *dc);

#endif // __DNSA_DATA_H__
