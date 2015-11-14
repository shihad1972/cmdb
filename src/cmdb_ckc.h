/* 
 *
 *  ckc: create kickstart config
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  cmdb_ckc.h
 *
 *  Main header file for the ckc program
 */

#ifndef __CMDB_CKC_H_
# define __CMDB_CKC_H_

typedef struct ckc_config_s {
	char *country;
	char *disk;
	char *domain;
	char *file;
	char *host;
	char *ip;
	char *language;
	char *packages;
	char *timezone;
	char *url;
	int action;
} ckc_config_s;

typedef struct ckc_package_s {
	char *package;
	struct ckc_package_s *next;
} ckc_package_s;

#endif // __CMDB_CKC_H_
