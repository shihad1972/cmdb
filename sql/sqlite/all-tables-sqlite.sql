
CREATE TABLE `build_type` (
  `bt_id` INTEGER PRIMARY KEY,
  `alias` varchar(25) NOT NULL,
  `build_type` varchar(25) NOT NULL DEFAULT 'none',
  `arg` varchar(15) NOT NULL DEFAULT 'none',
  `url` varchar(79) NOT NULL DEFAULT 'none',
  `mirror` varchar(255) NOT NULL DEFAULT 'none',
  `boot_line` varchar(127) NOT NULL DEFAULT 'none'
);

CREATE TABLE `service_type` (
  `service_type_id` INTEGER PRIMARY KEY,
  `service` varchar(15) NOT NULL,
  `detail` varchar(50) NOT NULL
);

CREATE TABLE `hard_type` (
  `hard_type_id` INTEGER PRIMARY KEY,
  `type` varchar(50) NOT NULL DEFAULT 'none',
  `class` varchar(50) NOT NULL DEFAULT 'none'
);

CREATE TABLE `customer` (
  `cust_id` INTEGER PRIMARY KEY,
  `name` varchar(60) NOT NULL,
  `address` varchar(63) NOT NULL DEFAULT 'none',
  `city` varchar(31) NOT NULL DEFAULT 'none',
  `county` varchar(30) NOT NULL DEFAULT 'none',
  `postcode` varchar(10) NOT NULL DEFAULT 'none',
  `coid` varchar(8) NOT NULL
);

CREATE TABLE `varient` (
  `varient_id` INTEGER PRIMARY KEY,
  `varient` varchar(50) NOT NULL,
  `valias` varchar(20) NOT NULL
);

CREATE TABLE `seed_schemes` (
  `def_scheme_id` INTEGER PRIMARY KEY,
  `scheme_name` varchar(79) NOT NULL,
  `lvm` smallint(4) NOT NULL
);

CREATE TABLE `build_domain` (
  `bd_id` INTEGER PRIMARY KEY,
  `start_ip` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `end_ip` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `netmask` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `gateway` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `ns` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `domain` varchar(150) NOT NULL DEFAULT 'no.domain',
  `ntp_server` varchar(63) NOT NULL DEFAULT 'none',
  `config_ntp` smallint(4) NOT NULL DEFAULT 0,
  `ldap_ssl` smallint(4) NOT NULL DEFAULT 0,
  `ldap_dn` varchar(96) NOT NULL DEFAULT 'none',
  `ldap_bind` varchar(127) NOT NULL DEFAULT 'none',
  `config_ldap` smallint(4) NOT NULL DEFAULT 0,
  `log_server` varchar(63) NOT NULL DEFAULT 'none',
  `config_log` smallint(4) NOT NULL DEFAULT 0,
  `smtp_server` varchar(63) NOT NULL DEFAULT 'none',
  `config_email` smallint(4) NOT NULL DEFAULT 0,
  `xymon_server` varchar(63) NOT NULL DEFAULT 'none',
  `config_xymon` smallint(4) NOT NULL DEFAULT 0,
  `ldap_server` varchar(63) NOT NULL DEFAULT 'none',
  `nfs_domain` varchar(79) NOT NULL DEFAULT 'none'
);

CREATE TABLE `contacts` (
  `cont_id` INTEGER PRIMARY KEY,
  `name` varchar(50) NOT NULL,
  `phone` varchar(20) NOT NULL,
  `email` varchar(50) NOT NULL,
  `cust_id` int(7) NOT NULL,

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
  `name` varchar(31) NOT NULL,

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
  `url` varchar(63) NOT NULL DEFAULT 'none',

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
  `vm_server` varchar(127) NOT NULL,
  `type` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL,

  FOREIGN KEY (`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);

CREATE TABLE `build_ip` (
  `ip_id` INTEGER PRIMARY KEY,
  `ip` UNSIGNED INTEGER NOT NULL,
  `hostname` varchar(31) NOT NULL,
  `domainname` varchar(127) NOT NULL,
  `bd_id` int(7) NOT NULL,
  `server_id` int(7) NOT NULL,

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
  `bt_id` int(7) DEFAULT NULL,

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

CREATE TABLE `seed_part` (
  `part_id` INTEGER PRIMARY KEY,
  `minimum` int(7) NOT NULL,
  `maximum` int(7) NOT NULL,
  `priority` int(7) NOT NULL DEFAULT '0',
  `mount_point` varchar(63) NOT NULL,
  `filesystem` varchar(15) NOT NULL,
  `server_id` int(7) NOT NULL,
  `logical_volume` varchar(31) NOT NULL,

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
  `logical_volume` varchar(31) NOT NULL,

  FOREIGN KEY(`def_scheme_id`)
    REFERENCES `seed_schemes`(`def_scheme_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);

CREATE TABLE `hardware` (
  `hard_id` INTEGER PRIMARY KEY,
  `detail` varchar(63) NOT NULL,
  `device` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL,
  `hard_type_id` int(7) NOT NULL,

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
  `timezone` varchar(63) NOT NULL DEFAULT 'Europe/London',

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
  `os_id` int(7) NOT NULL DEFAULT '0',

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
  `def_scheme_id` int(7) NOT NULL,

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
);

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
);

CREATE TABLE `records` (
  `id` INTEGER PRIMARY KEY,
  `zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(255) NOT NULL,
  `type` varchar(255) NOT NULL,
  `pri` int(7) NOT NULL DEFAULT '0',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',

  FOREIGN KEY (`zone`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);

CREATE TABLE `rev_records` (
  `rev_record_id` INTEGER PRIMARY KEY,
  `rev_zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(11) NOT NULL DEFAULT 'NULL',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',

  FOREIGN KEY (`rev_zone`)
    REFERENCES `rev_zones`(`rev_zone_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);

CREATE TABLE `preferred_a` (
  `prefa_id` INTEGER PRIMARY KEY,
  `ip` varchar(15) NOT NULL DEFAULT '0.0.0.0',
  `ip_addr` UNSIGNED INTEGER NOT NULL DEFAULT '0',
  `record_id` int(7) NOT NULL,
  `fqdn` varchar(255) NOT NULL DEFAULT 'none',

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


