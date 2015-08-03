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
 *  fileio.h: Function prototypes for file input/output
 */

#ifndef __FILEIO_H__
# define __FILEIO_H__

int
read_cmdb(cmdb_config_s *cmdb, char *dir);

int
read_dnsa(dnsa_config_s *dnsa, char *dir);

int
read_cbc(cbc_config_s *cbc, char *dir);

int
write_cmdb(cmdb_config_s *cmdb, char *dir);

int
write_dnsa(dnsa_config_s *dnsa, char *dir);

int
write_cbc(cbc_config_s *cbc, char *dir);

#endif // __FILEIO_H__
