/*
 *  cmdb suite of programs
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
 *  base_sql.h
 *
 *  This file is the header file for the SQL queries for the cmdb suite of
 *  programs
 */
#ifndef __BASE_SQL_H
# define __BASE_SQL_H
# include "../config.h"
/* cbc queries */

enum {			/* SELECT statements */
	BOOT_LINE = 1,
	BUILD = 2,
	BUILD_DOMAIN = 4,
	BUILD_IP = 8,
	BUILD_OS = 16,
	BUILD_TYPE = 32,
	DISK_DEV = 64,
	LOCALE = 128,
	BPACKAGE = 256,
	DPART = 512,
	SPART = 1024,
	SSCHEME = 2048,
	CSERVER = 4096,
	VARIENT = 8192,
	VMHOST = 16384
};

enum {			/* SELECT and INSERT Indices */
	BOOT_LINES = 0,
	BUILDS,
	BUILD_DOMAINS,
	BUILD_IPS,
	BUILD_OSS,
	BUILD_TYPES,
	DISK_DEVS,
	LOCALES,
	BPACKAGES,
	DPARTS,
	SPARTS,
	SSCHEMES,
	CSERVERS,
	VARIENTS,
	VMHOSTS
};

/* cmdb queries */


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

enum {			/* SELECT and INSERT indexes */
	SERVERS = 0,
	CUSTOMERS,
	CONTACTS,
	SERVICES,
	SERVICE_TYPES,
	HARDWARES,
	HARDWARE_TYPES,
	VM_HOSTS
};

enum {			/* Search indexes and queries */
	SERVER_ID_ON_NAME = 0,
	CUST_ID_ON_COID,
	SERV_TYPE_ID_ON_SERVICE,
	HARD_TYPE_ID_ON_HCLASS,
	VM_ID_ON_NAME
};

/* dnsa queries */


enum {			/* SELECT statements to use in multiple */
	ZONE = 1,
	REV_ZONE = 2,
	RECORD = 4,
	REV_RECORD = 8,
	ALL_A_RECORD = 16,
	DUPLICATE_A_RECORD = 32,
	PREFERRED_A = 64,
	RECORDS_ON_CNAME_TYPE = 128
};

enum {			/* SELECT and INSERT indexes */
	ZONES = 0,
	REV_ZONES,
	RECORDS,
	REV_RECORDS,
	ALL_A_RECORDS,
	DUPLICATE_A_RECORDS,
	PREFERRED_AS,
	RECORDS_ON_CNAME_TYPES
};

enum {			/* Delete indexes that diverge from SELECT */
	REV_RECORDS_ON_REV_ZONE = 7,
	RECORDS_ON_FWD_ZONE = 8
};

enum {			/* Extended searches */
	RECORDS_ON_DEST_AND_ID = 0,
	RECORDS_ON_ZONE
};

enum {			/* Search indexes and queries */
	ZONE_ID_ON_NAME = 0,
	REV_ZONE_ID_ON_NET_RANGE,
	REV_ZONE_PREFIX
};

enum {			/* Update indexes */
	ZONE_VALID_YES = 0,
	ZONE_UPDATED_YES,
	ZONE_UPDATED_NO,
	ZONE_SERIAL,
	REV_ZONE_VALID_YES,
	REV_ZONE_SERIAL
};

#endif /* __BASE_SQL_H */