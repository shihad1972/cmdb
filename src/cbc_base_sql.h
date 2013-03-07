/*
 *  cbc: Create Build config
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbc_base_sql.h
 *
 *  This file is the header file for the SQL functions for the cbc program
 */


#ifndef __CBC_BASE_SQL_H
# define __CBC_BASE_SQL_H
# include "../config.h"



# ifdef HAVE_MYSQL
#  include <mysql.h>

void
cmdb_mysql_init(cbc_config_t *dc, MYSQL *cmdb_mysql);

# endif /* HAVE_MYSQL */

#endif /* __CBC_BASE_SQL_H */