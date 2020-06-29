/*
 *
 *  cbc: Create build config
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
 *  cbc_common.h
 *
 *  Common header file for the cbc program suite
 *
 */

#ifndef __CBC_COMMON_H__
# define __CBC_COMMON_H__
# include <config.h>
# include "cbc_data.h"

int
get_server_id(ailsa_cmdb_s *cbc, char *server, unsigned long int *id);

int
get_partition_id(ailsa_cmdb_s *cbc, char *name, char *mount, uli_t *id);

int
get_scheme_id_from_build(ailsa_cmdb_s *cbc, uli_t server_id, uli_t *id);

int
get_scheme_name(ailsa_cmdb_s *cbc, uli_t server_id, char *name);

#endif /* CBC_COMMON_H */
