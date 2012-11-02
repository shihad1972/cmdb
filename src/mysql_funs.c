/* mysql_funs.c: Definition of the mysql functions for dnsa */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"
#include "rev_zone.h"
#include "mysql_funs.h"

int run_mysql_query(mysql_query_data_t *mydata, char config[][CONF_S])
{
	int retval = 0;
	mydata->host = config[HOST];
	mydata->db = config[DB];
	mydata->pass = config[PASS];
	mydata->user = config[USER];
	if (!(mysql_init(mydata->mycon))) {
		fprintf(stderr, "Cannot init. Out of memory?\n");
		return MY_INIT_FAIL;
	}
	if (!(mydata->myres = mysql_store_result(mydata->mycon))) {
		fprintf(stderr, "Cannot store result set\n");
		return MY_STORE_FAIL;
	}
	if (((dnsa_rows = mysql_num_rows(mydata->myres)) == 0)) {
		fprintf(stderr, "No result set\n");
		return NO_DOMAIN;
	} else if (dnsa_rows > 1) {
		fprintf(stderr, "Multiple rows found\n");
		return MULTI_DOMAIN;
	}
	mydata->myrow = mysql_fetch_row(mydata->myres);
	
	return retval;
}