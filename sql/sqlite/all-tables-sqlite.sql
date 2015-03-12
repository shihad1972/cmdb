CREATE TABLE `build_type` (
  `bt_id` INTEGER PRIMARY KEY,
  `alias` varchar(25) NOT NULL,
  `build_type` varchar(25) NOT NULL DEFAULT 'none',
  `arg` varchar(15) NOT NULL DEFAULT 'none',
  `url` varchar(255) NOT NULL DEFAULT 'none',
  `mirror` varchar(255) NOT NULL DEFAULT 'none',
  `boot_line` varchar(127) NOT NULL DEFAULT 'none'
);
CREATE TABLE `service_type` (
  `service_type_id` INTEGER PRIMARY KEY,
  `service` varchar(15) NOT NULL,
  `detail` varchar(31) NOT NULL
);
CREATE TABLE `hard_type` (
  `hard_type_id` INTEGER PRIMARY KEY,
  `type` varchar(31) NOT NULL DEFAULT 'none',
  `class` varchar(31) NOT NULL DEFAULT 'none'
);
CREATE TABLE `customer` (
  `cust_id` INTEGER PRIMARY KEY,
  `name` varchar(60) NOT NULL,
  `address` varchar(63) NOT NULL DEFAULT 'none',
  `city` varchar(31) NOT NULL DEFAULT 'none',
  `county` varchar(30) NOT NULL DEFAULT 'none',
  `postcode` varchar(10) NOT NULL DEFAULT 'none',
  `coid` varchar(8) NOT NULL
, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0);
CREATE TABLE `varient` (
  `varient_id` INTEGER PRIMARY KEY,
  `varient` varchar(50) NOT NULL,
  `valias` varchar(20) NOT NULL
, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0);
CREATE TABLE `seed_schemes` (
  `def_scheme_id` INTEGER PRIMARY KEY,
  `scheme_name` varchar(79) NOT NULL,
  `lvm` smallint(4) NOT NULL
, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0);
CREATE TABLE `build_domain` (
  `bd_id` INTEGER PRIMARY KEY,
  `start_ip` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `end_ip` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `netmask` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `gateway` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `ns` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `domain` varchar(255) NOT NULL DEFAULT 'no.domain',
  `ntp_server` varchar(255) NOT NULL DEFAULT 'none',
  `config_ntp` smallint(4) NOT NULL DEFAULT 0,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0);
CREATE TABLE `contacts` (
  `cont_id` INTEGER PRIMARY KEY,
  `name` varchar(50) NOT NULL,
  `phone` varchar(20) NOT NULL,
  `email` varchar(50) NOT NULL,
  `cust_id` int(7) NOT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer` (`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `server` (
  `server_id` INTEGER PRIMARY KEY,
  `vendor` varchar(63) NOT NULL DEFAULT 'none',
  `make` varchar(63) NOT NULL DEFAULT 'none',
  `model` varchar(31) NOT NULL DEFAULT 'none',
  `uuid` varchar(63) NOT NULL DEFAULT 'none',
  `cust_id` int(7) NOT NULL,
  `vm_server_id` int(7) NOT NULL DEFAULT 0,
  `name` varchar(63) NOT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer` (`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `services` (
  `service_id` INTEGER PRIMARY KEY,
  `server_id` int(7) NOT NULL,
  `cust_id` int(7) NOT NULL,
  `service_type_id` int(7) NOT NULL,
  `detail` varchar(63) NOT NULL DEFAULT 'none',
  `url` varchar(255) NOT NULL DEFAULT 'none', `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer`(`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY (`service_type_id`)
    REFERENCES `service_type`(`service_type_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `vm_server_hosts` (
  `vm_server_id` INTEGER PRIMARY KEY,
  `vm_server` varchar(255) NOT NULL,
  `type` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `build_ip` (
  `ip_id` INTEGER PRIMARY KEY,
  `ip` UNSIGNED INTEGER NOT NULL,
  `hostname` varchar(63) NOT NULL,
  `domainname` varchar(255) NOT NULL,
  `bd_id` int(7) NOT NULL,
  `server_id` int(7) NOT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`bd_id`) 
    REFERENCES `build_domain`(`bd_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `build_os` (
  `os_id` INTEGER PRIMARY KEY,
  `os` varchar(31) NOT NULL,
  `os_version` varchar(15) DEFAULT NULL,
  `alias` varchar(15) DEFAULT NULL,
  `ver_alias` varchar(15) NOT NULL DEFAULT 'none',
  `arch` varchar(15) DEFAULT NULL,
  `bt_id` int(7) DEFAULT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`bt_id`)
    REFERENCES `build_type`(`bt_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `disk_dev` (
  `disk_id` INTEGER PRIMARY KEY,
  `server_id` int(7) NOT NULL,
  `device` varchar(63) NOT NULL,
  `lvm` smallint(4) NOT NULL,

  FOREIGN KEY(`server_id`)
    REFERENCES server(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `default_part` (
  `def_part_id` INTEGER PRIMARY KEY,
  `minimum` int(7) NOT NULL,
  `maximum` int(7) NOT NULL,
  `priority` int(7) NOT NULL DEFAULT '0',
  `mount_point` varchar(63) NOT NULL,
  `filesystem` varchar(15) NOT NULL,
  `def_scheme_id` int(7) NOT NULL,
  `logical_volume` varchar(31) NOT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`def_scheme_id`)
    REFERENCES `seed_schemes`(`def_scheme_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `hardware` (
  `hard_id` INTEGER PRIMARY KEY,
  `detail` varchar(63) NOT NULL,
  `device` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL,
  `hard_type_id` int(7) NOT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`hard_type_id`)
    REFERENCES `hard_type`(`hard_type_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `locale` (
  `locale_id` INTEGER PRIMARY KEY,
  `locale` varchar(31) NOT NULL DEFAULT 'en_GB',
  `country` varchar(15) NOT NULL DEFAULT 'GB',
  `language` varchar(15) NOT NULL DEFAULT 'en',
  `keymap` varchar(15) NOT NULL DEFAULT 'uk',
  `os_id` int(7) NOT NULL,
  `bt_id` int(7) NOT NULL,
  `timezone` varchar(63) NOT NULL DEFAULT 'Europe/London', `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`os_id`)
    REFERENCES `build_os`(`os_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`bt_id`)
    REFERENCES `build_type`(`bt_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `packages` (
  `pack_id` INTEGER PRIMARY KEY,
  `package` varchar(63) NOT NULL DEFAULT 'none',
  `varient_id` int(7) NOT NULL DEFAULT '0',
  `os_id` int(7) NOT NULL DEFAULT '0', `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`os_id`)
    REFERENCES `build_os`(`os_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`varient_id`)
    REFERENCES `varient`(`varient_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `build` (
  `build_id` INTEGER PRIMARY KEY,
  `mac_addr` varchar(17) NOT NULL,
  `varient_id` int(7) NOT NULL,
  `net_inst_int` varchar(15) NOT NULL,
  `server_id` int(7) NOT NULL,
  `os_id` int(7) NOT NULL,
  `ip_id` int(7) NOT NULL,
  `locale_id` int(7) NOT NULL,
  `def_scheme_id` int(7) NOT NULL, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`varient_id`)
    REFERENCES `varient`(`varient_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`os_id`)
    REFERENCES `build_os`(`os_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`ip_id`)
    REFERENCES `build_ip`(`ip_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`locale_id`)
    REFERENCES `locale`(`locale_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`def_scheme_id`)
    REFERENCES `seed_schemes`(`def_scheme_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `options` (
  `prefkey` varchar(255) UNIQUE NOT NULL,
  `preftype` varchar(255) NOT NULL DEFAULT '',
  `prefval` varchar(255) DEFAULT NULL
);
CREATE TABLE `zones` (
  `id` INTEGER PRIMARY KEY,
  `name` varchar(255) NOT NULL,
  `pri_dns` varchar(255) DEFAULT NULL,
  `sec_dns` varchar(255) DEFAULT NULL,
  `serial` int(11) NOT NULL DEFAULT '0',
  `refresh` int(11) NOT NULL DEFAULT '604800',
  `retry` int(11) NOT NULL DEFAULT '86400',
  `expire` int(11) NOT NULL DEFAULT '2419200',
  `ttl` int(11) NOT NULL DEFAULT '604800',
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',
  `owner` int(11) NOT NULL DEFAULT '1',
  `updated` varchar(15) NOT NULL DEFAULT 'yes',
  `type` varchar(15) NOT NULL DEFAULT 'master',
  `master` varchar(255)
, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0);
CREATE TABLE `rev_zones` (
  `rev_zone_id` INTEGER PRIMARY KEY,
  `net_range` varchar(255) DEFAULT NULL,
  `prefix` varchar(4) DEFAULT NULL,
  `net_start` varchar(255) DEFAULT NULL,
  `net_finish` varchar(255) DEFAULT NULL,
  `start_ip` UNSIGNED INTEGER DEFAULT NULL,
  `finish_ip` UNSIGNED INTEGER DEFAULT NULL,
  `pri_dns` varchar(255) DEFAULT NULL,
  `sec_dns` varchar(255) DEFAULT NULL,
  `serial` int(7) NOT NULL DEFAULT '0',
  `refresh` int(7) NOT NULL DEFAULT '604800',
  `retry` int(7) NOT NULL DEFAULT '86400',
  `expire` int(7) NOT NULL DEFAULT '2419200',
  `ttl` int(7) NOT NULL DEFAULT '604800',
  `valid` varchar(15) NOT NULL DEFAULT 'yes',
  `owner` int(7) NOT NULL DEFAULT '1',
  `updated` varchar(15) NOT NULL DEFAULT 'unknown',
  `type` varchar(15) NOT NULL DEFAULT 'master',
  `master` varchar(255)
, `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0);
CREATE TABLE `records` (
  `id` INTEGER PRIMARY KEY,
  `zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(255) NOT NULL,
  `type` varchar(255) NOT NULL,
  `protocol` varchar(15),
  `service` varchar(15),	
  `pri` int(7) NOT NULL DEFAULT '0',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown', `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`zone`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `rev_records` (
  `rev_record_id` INTEGER PRIMARY KEY,
  `rev_zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(11) NOT NULL DEFAULT 'NULL',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown', `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`rev_zone`)
    REFERENCES `rev_zones`(`rev_zone_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `glue_zones` (
  `id` INTEGER PRIMARY KEY,
  `zone_id` int(7) NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL,
  `pri_ns` varchar(255) NOT NULL,
  `sec_ns` varchar(255) NOT NULL DEFAULT 'none',
  `pri_dns` varchar(15) NOT NULL,
  `sec_dns` varchar(15) NOT NULL DEFAULT 'none', `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`zone_id`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `preferred_a` (
  `prefa_id` INTEGER PRIMARY KEY,
  `ip` varchar(15) NOT NULL DEFAULT '0.0.0.0',
  `ip_addr` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `record_id` int(7) NOT NULL,
  `fqdn` varchar(255) NOT NULL DEFAULT 'none', `cuser` int(11) NOT NULL DEFAULT 0, `muser` int(11) NOT NULL DEFAULT 0, `ctime` timestamp NOT NULL DEFAULT 0, `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`record_id`)
    REFERENCES `records`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TABLE `users` (
  `id` INTEGER PRIMARY KEY,
  `uid` int(7) NOT NULL,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `admin` varchar(255) NOT NULL DEFAULT 'no'
);
CREATE TRIGGER insert_build AFTER INSERT ON build
BEGIN
UPDATE build SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE build_id = new.build_id;
END;
CREATE TRIGGER update_build AFTER UPDATE ON build
BEGIN
UPDATE build SET mtime = CURRENT_TIMESTAMP WHERE build_id = new.build_id;
END;
CREATE TRIGGER insert_build_domain AFTER INSERT ON build_domain
BEGIN
UPDATE build_domain SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;
CREATE TRIGGER update_build_domain AFTER UPDATE ON build_domain
BEGIN
UPDATE build_domain SET mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;
CREATE TRIGGER insert_build_ip AFTER INSERT ON build_ip
BEGIN
UPDATE build_ip SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE ip_id = new.ip_id;
END;
CREATE TRIGGER update_build_ip AFTER UPDATE ON build_ip
BEGIN
UPDATE build_ip SET mtime = CURRENT_TIMESTAMP WHERE ip_id = new.ip_id;
END;
CREATE TRIGGER insert_build_os AFTER INSERT ON build_os
BEGIN
UPDATE build_os SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;
CREATE TRIGGER update_build_os AFTER UPDATE ON build_os
BEGIN
UPDATE build_os SET mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;
CREATE TRIGGER insert_contacts AFTER INSERT ON contacts
BEGIN
UPDATE contacts SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE cont_id = new.cont_id;
END;
CREATE TRIGGER update_contacts AFTER UPDATE ON contacts
BEGIN
UPDATE contacts SET mtime = CURRENT_TIMESTAMP WHERE cont_id = new.cont_id;
END;
CREATE TRIGGER insert_customer AFTER INSERT ON customer
BEGIN
UPDATE customer SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;
CREATE TRIGGER update_customer AFTER UPDATE ON customer
BEGIN
UPDATE customer SET mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;
CREATE TRIGGER insert_glue_zones AFTER INSERT ON glue_zones
BEGIN
UPDATE glue_zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER update_glue_zones AFTER UPDATE ON glue_zones
BEGIN
UPDATE glue_zones SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER insert_hardware AFTER INSERT ON hardware
BEGIN
UPDATE hardware SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE hard_id = new.hard_id;
END;
CREATE TRIGGER update_hardware AFTER UPDATE ON hardware
BEGIN
UPDATE hardware SET mtime = CURRENT_TIMESTAMP WHERE hard_id = new.hard_id;
END;
CREATE TRIGGER insert_packages AFTER INSERT ON packages
BEGIN
UPDATE packages SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE pack_id = new.pack_id;
END;
CREATE TRIGGER update_packages AFTER UPDATE ON packages
BEGIN
UPDATE packages SET mtime = CURRENT_TIMESTAMP WHERE pack_id = new.pack_id;
END;
CREATE TRIGGER insert_preferred_a AFTER INSERT ON preferred_a
BEGIN
UPDATE preferred_a SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE prefa_id = new.prefa_id;
END;
CREATE TRIGGER update_preferred_a AFTER UPDATE ON preferred_a
BEGIN
UPDATE preferred_a SET mtime = CURRENT_TIMESTAMP WHERE prefa_id = new.prefa_id;
END;
CREATE TRIGGER insert_records AFTER INSERT ON records
BEGIN
UPDATE records SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER update_records AFTER UPDATE ON records
BEGIN
UPDATE records SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER insert_rev_records AFTER INSERT ON rev_records
BEGIN
UPDATE rev_records SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE rev_record_id = new.rev_record_id;
END;
CREATE TRIGGER update_rev_records AFTER UPDATE ON rev_records
BEGIN
UPDATE rev_records SET mtime = CURRENT_TIMESTAMP WHERE rev_record_id = new.rev_record_id;
END;
CREATE TRIGGER insert_rev_zones AFTER INSERT ON rev_zones
BEGIN
UPDATE rev_zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE rev_zone_id = new.rev_zone_id;
END;
CREATE TRIGGER update_rev_zones AFTER UPDATE ON rev_zones
BEGIN
UPDATE rev_zones SET mtime = CURRENT_TIMESTAMP WHERE rev_zone_id = new.rev_zone_id;
END;
CREATE TRIGGER insert_seed_schemes AFTER INSERT ON seed_schemes
BEGIN
UPDATE seed_schemes SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;
CREATE TRIGGER update_seed_schemes AFTER UPDATE ON seed_schemes
BEGIN
UPDATE seed_schemes SET mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;
CREATE TRIGGER insert_server AFTER INSERT ON server
BEGIN
UPDATE server SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE server_id = new.server_id;
END;
CREATE TRIGGER update_server AFTER UPDATE ON server
BEGIN
UPDATE server SET mtime = CURRENT_TIMESTAMP WHERE server_id = new.server_id;
END;
CREATE TRIGGER insert_services AFTER INSERT ON services
BEGIN
UPDATE services SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE service_id = new.service_id;
END;
CREATE TRIGGER update_services AFTER UPDATE ON services
BEGIN
UPDATE services SET mtime = CURRENT_TIMESTAMP WHERE service_id = new.service_id;
END;
CREATE TRIGGER insert_varient AFTER INSERT ON varient
BEGIN
UPDATE varient SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;
CREATE TRIGGER update_varient AFTER UPDATE ON varient
BEGIN
UPDATE varient SET mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;
CREATE TRIGGER insert_vm_server_hosts AFTER INSERT ON vm_server_hosts
BEGIN
UPDATE vm_server_hosts SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE vm_server_id = new.vm_server_id;
END;
CREATE TRIGGER update_vm_server_hosts AFTER UPDATE ON vm_server_hosts
BEGIN
UPDATE vm_server_hosts SET mtime = CURRENT_TIMESTAMP WHERE vm_server_id = new.vm_server_id;
END;
CREATE TRIGGER insert_zones AFTER INSERT ON zones
BEGIN
UPDATE zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER update_zones AFTER UPDATE ON zones
BEGIN
UPDATE zones SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER insert_locale AFTER INSERT ON locale
BEGIN
UPDATE locale SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER update_locale AFTER UPDATE ON locale
BEGIN
UPDATE locale SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER insert_default_part AFTER INSERT ON default_part
BEGIN
UPDATE default_part SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE def_part_id = new.def_part_id;
END;
CREATE TRIGGER update_default_part AFTER UPDATE ON default_part
BEGIN
UPDATE default_part SET mtime = CURRENT_TIMESTAMP WHERE def_part_id = new.def_part_id;
END;
CREATE TABLE `system_packages` (
`syspack_id` INTEGER PRIMARY KEY,
`name` varchar(127) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0
);
CREATE TRIGGER insert_system_packages AFTER INSERT ON system_packages
BEGIN
UPDATE system_packages SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE syspack_id = new.syspack_id;
end;
CREATE TRIGGER update_system_packages AFTER UPDATE ON system_packages
BEGIN
UPDATE system_packages SET mtime = CURRENT_TIMESTAMP WHERE syspack_id = new.syspack_id;
END;
CREATE TABLE `system_package_args` (
`syspack_arg_id` INTEGER PRIMARY KEY,
`syspack_id` int(11) NOT NULL,
`field` varchar(127) NOT NULL,
`type` varchar(31) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0,
FOREIGN KEY(`syspack_id`)
REFERENCES `system_packages`(`syspack_id`)
ON UPDATE CASCADE ON DELETE CASCADE
);
CREATE TRIGGER insert_system_package_args AFTER INSERT ON system_package_args
BEGIN
UPDATE system_package_args SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP where syspack_arg_id = new.syspack_arg_id;
END;
CREATE TRIGGER update_system_package_args AFTER UPDATE ON system_package_args
BEGIN
UPDATE system_package_args SET mtime = CURRENT_TIMESTAMP WHERE syspack_arg_id = new.syspack_arg_id;
END;
CREATE TABLE `system_package_conf` (
`syspack_conf_id` INTEGER PRIMARY KEY,
`syspack_arg_id` int(11) NOT NULL,
`syspack_id` int(11) NOT NULL,
`bd_id` int(11) NOT NULL,
`arg` varchar(255) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0,
FOREIGN KEY(`syspack_id`)
REFERENCES `system_packages`(`syspack_id`)
ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY(`syspack_arg_id`)
REFERENCES `system_package_args`(`syspack_arg_id`)                                    
ON UPDATE CASCADE ON DELETE CASCADE
);
CREATE TRIGGER insert_system_package_conf AFTER INSERT ON system_package_conf
BEGIN
UPDATE system_package_conf SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP where syspack_conf_id = new.syspack_conf_id;
END;
CREATE TRIGGER update_system_package_conf AFTER UPDATE ON system_package_conf
BEGIN
UPDATE system_package_conf SET mtime = CURRENT_TIMESTAMP WHERE syspack_conf_id = new.syspack_conf_id;
END;
