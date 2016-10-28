/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2016  Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  cmdb_sql.h
 *
 *  Header file for SQL routines.
 */

#ifndef __CMDB_SQL_H__
# define __CMDB_SQL_H__

typedef struct ailsa_sql_s {
	const char *query;
	int qtype;
	const unsigned int *args;
	const unsigned int *fields;
} ailsa_sql_s;

extern const ailsa_sql_s queries[];

enum {			/* Select statements */
	SERVER = 1,
	CUSTOMER = 2,
	CONTACT = 4,
	SERVICE = 8,
	SERVICE_TYPE = 16,
	HARDWARE = 32,
	HARDWARE_TYPE = 64,
	VM_HOST = 128
};
#endif // __CMDB_SQL_H__
