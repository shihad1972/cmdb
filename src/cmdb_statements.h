/* 
 *
 *  cmdb: Configuration Management Database
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
 *  cmdb_statements.h
 *
 *  This file contains the SQL statement list for the cmdb suite of programs
 */

const char *sql_select[] = { "\
SELECT server_id, vendor, make, model, uuid, cust_id, vm_server_id, name \
FROM server ORDER BY cust_id","\
SELECT cust_id, name, address, city, county, postcode, coid FROM customer \
ORDER BY coid","\
SELECT cont_id, name, phone, email, cust_id FROM contacts","\
SELECT service_id, server_id, cust_id, service_type_id, detail, url FROM \
services ORDER BY service_type_id","\
SELECT service_type_id, service, detail FROM service_type","\
SELECT hard_id, detail, device, server_id, hard_type_id FROM hardware \
ORDER BY device DESC","\
SELECT hard_type_id, type, class FROM hard_type","\
SELECT vm_server_id, vm_server, type, server_id FROM vm_server_hosts"
};

const char *sql_insert[] = { "\
INSERT INTO server (vendor, make, model, uuid, cust_id, vm_server_id) VALUES \
(?,?,?,?,?,?)","\
INSERT INTO customer (name, address, city, postcode, coid) VALUES \
(?,?,?,?,?)","\
INSERT INTO contacts (name, phone, email, cust_id) VALUES (?,?,?,?)","\
INSERT INTO services (server_id, cust_id, service_type_id, detail, url) \
VALUES (?,?,?,?,?)","\
INSERT INTO service_type (service, detail) VALUES (?,?)","\
INSERT INTO hardware (detail, device, server_id, hard_type_id) VALUES \
(?,?,?,?)","\
INSERT INTO hard_type (type, class) VALUES (?,?)","\
INSERT INTO vm_server_hosts (vm_server, type, server_id) VALUES (?,?,?)"
};

/* Number of returned fields for the above SELECT queries */
const unsigned int select_fields[] = { 8,7,5,6,3,5,3,4 };

const unsigned int insert_fields[] = { 6,5,4,5,2,4,2,3 };

enum {			/* SELECT indexes */
	SERVERS = 0,
	CUSTOMERS,
	CONTACTS,
	SERVICES,
	SERVICE_TYPES,
	HARDWARES,
	HARDWARE_TYPES,
	VM_HOSTS
};



	
 
