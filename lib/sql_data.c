/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  helper.c
 *
 *  Helper functions for libailsasql
 *
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif // HAVE_STDBOOL_H
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /*HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /*HAVE_SQLITE3 */
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "base_sql.h"

void
cmdb_clean_ailsa_sql_multi(ailsa_sql_multi_s *data)
{
	if (!(data))
		return;
	if (!(data->query))
		my_free(data->query);
	if (data->fields)
		my_free(data->fields);
	my_free(data);
}
