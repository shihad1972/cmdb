/* wzf.c:
 * 
 * Contains function to write out the forward zone file from information
 * stored in the database.
 * 
 * Contains some basic error checking, and will update the zone in the
 * database as valid if these checks are passed.
 * 
 * You still need to write out the config file to have the zones implemented
 * on the name server
 * 
 * (C) Iain M Conochie 2012
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "forward.h"
#include "mysqlfunc.h"
#include "dnsa_mysql.h"

int wzf (char *domain, dnsa_config_t *dc)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res, *my_res;
	MYSQL_ROW my_row;
	zone_info_t zone_info, *zi;
	record_row_t row_data;
	size_t offset, len;
	my_ulonglong dnsa_rows;
	int error;
	char *zout, *zout2, *tmp, *zonefilename;
	const char *dnsa_query;

	if (!(zout = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zout in wzf");
	if (!(zout2 = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zout2 in wzf");
	if (!(tmp = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in wzf");
	if (!(zonefilename = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefilename in wrz");
	
	dnsa_query = tmp;
	zi = &zone_info;
	
	/* Initialise MYSQL connection and query */
	dnsa_mysql_init(dc, &dnsa);
	sprintf(tmp, "SELECT * FROM zones WHERE name = '%s'", domain);
	cmdb_mysql_query(&dnsa, dnsa_query);

	if (!(dnsa_res = mysql_store_result(&dnsa)))
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0))
		report_error(NO_DOMAIN, domain);
	
	my_row = mysql_fetch_row(dnsa_res);
	zone_info = fill_zone_data(my_row);
	create_zone_header(zout, zone_info);
	
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'MX'", zi->id);
	cmdb_mysql_query(&dnsa, dnsa_query);
	
	if (!(dnsa_res = mysql_store_result(&dnsa)))
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	if (!(dnsa_rows = mysql_num_rows(dnsa_res)) == 0) {
		while ((my_row = mysql_fetch_row(dnsa_res))) {
			add_mx_to_header(zout, my_row);
		}
	}
	
	sprintf(tmp, "$ORIGIN\t%s.\n", zi->name);
	len = strlen(tmp);
	strncat(zout, tmp, len);
	
	/** zout2 will be used to write the real zone file.
	 ** Cannot have A records for the NS and MX added twice
	 ** so save the buffer into zout2 and use that
	 */
	len = strlen(zout);
	strncpy(zout2, zout, len);
	
	add_ns_A_records_to_header(zi, dc, zout);
	error = add_MX_A_records_to_header(zi, dc, zout);
	if (error < 0) {
		; /* No MX records */
	}

	/* Check zonefile */
	sprintf(zonefilename, "%sdb1.%s", dc->dir, zi->name);
	write_fwd_zonefile(zonefilename, zout);
	check_fwd_zone(zonefilename, zi->name, dc);
	remove(zonefilename);
	
	/* Add the rest of the records */
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'A' OR zone = %d AND type = 'CNAME'", zi->id, zi->id);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(my_res = mysql_store_result(&dnsa)))
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	while ((my_row = mysql_fetch_row(my_res))) {
		row_data = fill_record_data(my_row);
		offset = add_records(row_data, zout2, offset);	
	}
	sprintf(zonefilename, "%sdb.%s", dc->dir, zi->name);
	write_fwd_zonefile(zonefilename, zout2);
	check_fwd_zone(zonefilename, zi->name, dc);
	sprintf(tmp, "UPDATE zones SET updated = 'no', valid = 'yes' WHERE name = '%s'", zi->name);
	error = mysql_query(&dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(&dnsa);
	if (dnsa_rows == 1)
		fprintf(stderr, "DB updated as zone validated\n");

	mysql_close(&dnsa);
	free(zout2);
	free(zout);
	free(tmp);
	free(zonefilename);
	return 0;
}
