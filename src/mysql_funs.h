/* mysql_funs.h: Functions to gain access to mysql */
#include <mysql.h>
#include "dnsa.h"

#ifndef __DNSA_MYSQL_H__
#define __DNSA_MYSQL_H__

typedef struct mysql_query_data_t {
	MYSQL *mycon;
	MYSQL_ROW *myrow;
	MYSQL_RES *myres;
	char *host;
	char *db;
	char *user;
	char *pass;
	unsigned int *port;
	unsigned long int *client_flag;
	const char **query;
	const char **unix_socket;
} mysql_query_data_t;



/* This function runs a basic mysql query
 * As parameters are all pointers it should fill up the data structures
 * if any failure, return code will let us know
 */
int
run_mysql_query(mysql_query_data_t *mydata, char config[][CONF_S]);

#endif