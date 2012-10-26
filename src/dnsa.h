/* dnsa.h: DNSA header file */

#ifndef __DNSA_H__
#define __DNSA_H__
enum {			/* Buffer Sizes */
	CH_S = 2,
	RANGE_S = 16,
	CONF_S = 80,
	RBUFF_S = 256,
	TBUFF_S = 512,
	BUFF_S = 1024,
	FILE_S = 4096
};

enum {			/* error codes */
	OK = 0,
	ARGC_INVAL = 1,
	ARGV_INVAL = 2,
	NO_DOMAIN = 3,
	MULTI_DOMAIN = 4,
	NO_DELIM = 5,
	NO_RECORDS = 6,
	MY_INIT_FAIL = 10,
	MY_CONN_FAIL = 11,
	MY_QUERY_FAIL = 12,
	MY_STORE_FAIL = 13,
	FILE_O_FAIL = 20
};

#endif
