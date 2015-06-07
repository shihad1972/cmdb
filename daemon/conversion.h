/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  conversion.h: Main cmdb header file
 */

#ifndef __CONVERSION_H__
# define __CONVERSION_H__

struct all_config {
	dnsa_config_s		*dnsa;
	cmdb_config_s		*cmdb;
	cbc_config_s		*cbc;
	struct cmdbd_config	*cmdbd;
};

int
convert_dnsa_config(dnsa_config_s *dcs, struct cmdbd_config *cdc);

int
convert_cmdb_config(cmdb_config_s *ccs, struct cmdbd_config *cdc);

int
convert_cbc_config(cbc_config_s *ccs, struct cmdbd_config *cdc);

int
convert_all_config(struct all_config *all);

int
read_cmdb(cmdb_config_s *cmdb);

int
write_cmdb(cmdb_config_s *cmdb);

int
read_dnsa(dnsa_config_s *dnsa, char *dir);

int
write_dnsa(dnsa_config_s *dnsa, char *dir);

#endif // __CONVERSION_H__
