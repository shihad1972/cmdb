
DROP TABLE IF EXISTS `build_domain`;
CREATE TABLE `build_domain` (
  `bd_id` int(7) NOT NULL AUTO_INCREMENT,
  `start_ip` int(4) unsigned NOT NULL DEFAULT '0',
  `end_ip` int(4) unsigned NOT NULL DEFAULT '0',
  `netmask` int(4) unsigned NOT NULL DEFAULT '0',
  `gateway` int(4) unsigned NOT NULL DEFAULT '0',
  `ns` int(4) unsigned NOT NULL DEFAULT '0',
  `domain` varchar(150) NOT NULL DEFAULT 'no.domain',
  `country` varchar(16) NOT NULL DEFAULT 'GB',
  `language` varchar(16) NOT NULL DEFAULT 'en',
  `keymap` varchar(16) NOT NULL DEFAULT 'uk',
  `ntp_server` varchar(64) NOT NULL DEFAULT 'shihad.org',
  `config_ntp` tinyint(4) NOT NULL DEFAULT '1',
  `ldap_url` varchar(128) NOT NULL DEFAULT 'ldap01.shihad.org',
  `ldap_ssl` tinyint(4) NOT NULL DEFAULT '1',
  `ldap_dn` varchar(96) NOT NULL DEFAULT 'dc=shihad,dc=org',
  `ldap_bind` varchar(128) NOT NULL DEFAULT 'cn=thargoid,dc=shihad,dc=org',
  `config_ldap` tinyint(4) NOT NULL DEFAULT '1',
  `log_server` varchar(64) NOT NULL DEFAULT 'logger01.shihad.org',
  `config_log` tinyint(4) NOT NULL DEFAULT '1',
  `smtp_server` varchar(64) NOT NULL DEFAULT 'weezer.epl.shihad.org',
  `config_email` tinyint(4) NOT NULL DEFAULT '1',
  `xymon_server` varchar(64) NOT NULL DEFAULT '192.168.1.50',
  `config_xymon` tinyint(4) NOT NULL DEFAULT '1',
  `ldap_server` varchar(64) NOT NULL DEFAULT 'ldap01.shihad.org',
  `email_server` varchar(64) NOT NULL DEFAULT 'mail01.scots.shihad.org',
  `xymon_config` tinyint(4) NOT NULL DEFAULT '1',
  `nfs_domain` varchar(79) NOT NULL DEFAULT 'shihad.org',
  PRIMARY KEY (`bd_id`),

  INDEX (config_ntp, config_ldap, config_log),
  INDEX (config_email, config_xymon),

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `build_ip`;
CREATE TABLE `build_ip` (
  `ip_id` int(7) NOT NULL AUTO_INCREMENT,
  `ip` int(4) unsigned DEFAULT NULL,
  `hostname` varchar(30) DEFAULT NULL,
  `domainname` varchar(150) DEFAULT NULL,
  `bd_id` int(7) NOT NULL,
  PRIMARY KEY (`ip_id`),

  INDEX (bd_id),

  FOREIGN KEY(bd_id) 
    REFERENCES build_domain(bd_id)
    ON UPDATE CASCADE ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `build_os`;
CREATE TABLE `build_os` (
  `os_id` int(7) NOT NULL AUTO_INCREMENT,
  `os` varchar(25) DEFAULT NULL,
  `os_version` varchar(25) DEFAULT NULL,
  `alias` varchar(20) DEFAULT NULL,
  `ver_alias` varchar(25) NOT NULL DEFAULT 'none',
  `arch` varchar(12) DEFAULT NULL,
  `boot_id` int(7) DEFAULT NULL,
  `bt_id` int(7) DEFAULT NULL,
  PRIMARY KEY (`os_id`),

  INDEX (bt_id),

  FOREIGN KEY(bt_id)
    REFERENCES build_type(bt_id)
    ON UPDATE CASCADE ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `build`;
CREATE TABLE `build` (
  `build_id` int(7) NOT NULL AUTO_INCREMENT,
  `mac_addr` varchar(17) DEFAULT NULL,
  `varient_id` int(7) DEFAULT NULL,
  `net_inst_int` varchar(12) DEFAULT NULL,
  `server_id` int(7) DEFAULT NULL,
  `os_id` int(7) DEFAULT NULL,
  `boot_id` int(7) DEFAULT NULL,
  `ip_id` int(7) DEFAULT NULL,
  `locale_id` int(7) NOT NULL,
  PRIMARY KEY (`build_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `build_type`;
CREATE TABLE `build_type` (
  `bt_id` int(7) NOT NULL AUTO_INCREMENT,
  `alias` varchar(25) DEFAULT NULL,
  `build_type` varchar(25) NOT NULL DEFAULT 'none',
  `arg` varchar(16) NOT NULL DEFAULT 'none',
  `url` varchar(80) NOT NULL DEFAULT 'none',
  `mirror` varchar(256) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`bt_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `contacts`;
CREATE TABLE `contacts` (
  `cont_id` int(7) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) DEFAULT NULL,
  `phone` varchar(20) DEFAULT NULL,
  `email` varchar(50) DEFAULT NULL,
  `cust_id` int(7) DEFAULT NULL,
  PRIMARY KEY (`cont_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `customer`;
CREATE TABLE `customer` (
  `cust_id` int(7) NOT NULL AUTO_INCREMENT,
  `name` varchar(60) DEFAULT NULL,
  `address` varchar(100) DEFAULT NULL,
  `city` varchar(40) DEFAULT NULL,
  `county` varchar(30) DEFAULT NULL,
  `postcode` varchar(10) DEFAULT NULL,
  `coid` varchar(8) DEFAULT NULL,
  PRIMARY KEY (`cust_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `default_part`;
CREATE TABLE `default_part` (
  `def_part_id` int(7) NOT NULL AUTO_INCREMENT,
  `minimum` int(7) NOT NULL,
  `maximum` int(7) NOT NULL,
  `priority` int(7) NOT NULL DEFAULT '0',
  `mount_point` varchar(64) NOT NULL,
  `filesystem` varchar(16) NOT NULL,
  `def_scheme_id` int(7) NOT NULL,
  `logical_volume` varchar(16) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`def_part_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `disk_dev`;
CREATE TABLE `disk_dev` (
  `disk_id` int(7) NOT NULL AUTO_INCREMENT,
  `server_id` int(7) NOT NULL,
  `device` varchar(64) NOT NULL,
  `lvm` tinyint(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`disk_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `hard_type`;
CREATE TABLE `hard_type` (
  `hard_type_id` int(7) NOT NULL AUTO_INCREMENT,
  `type` varchar(50) DEFAULT NULL,
  `class` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`hard_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `hardware`;
CREATE TABLE `hardware` (
  `hard_id` int(7) NOT NULL AUTO_INCREMENT,
  `detail` varchar(50) DEFAULT NULL,
  `device` varchar(30) DEFAULT NULL,
  `server_id` int(7) DEFAULT NULL,
  `hard_type_id` int(7) DEFAULT NULL,
  PRIMARY KEY (`hard_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `locale`;
CREATE TABLE `locale` (
  `locale_id` int(7) NOT NULL AUTO_INCREMENT,
  `locale` varchar(32) NOT NULL DEFAULT 'en_GB',
  `country` varchar(16) NOT NULL DEFAULT 'GB',
  `language` varchar(16) NOT NULL DEFAULT 'en',
  `keymap` varchar(16) NOT NULL DEFAULT 'uk',
  `os_id` int(7) NOT NULL,
  `bt_id` int(7) NOT NULL,
  `timezone` varchar(63) NOT NULL DEFAULT 'Europe/London',
  PRIMARY KEY (`locale_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `options`;
CREATE TABLE `options` (
  `prefkey` varchar(255) NOT NULL,
  `preftype` varchar(255) NOT NULL DEFAULT '',
  `prefval` varchar(255) DEFAULT NULL,
  UNIQUE KEY `prefkey` (`prefkey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `packages`;
CREATE TABLE `packages` (
  `pack_id` int(7) NOT NULL AUTO_INCREMENT,
  `package` varchar(64) NOT NULL DEFAULT 'none',
  `varient_id` int(7) NOT NULL DEFAULT '0',
  `os_id` int(7) NOT NULL DEFAULT '0',
  PRIMARY KEY (`pack_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `records`;
CREATE TABLE `records` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `zone` int(11) NOT NULL DEFAULT '0',
  `host` varchar(255) NOT NULL,
  `type` varchar(255) NOT NULL,
  `pri` int(11) NOT NULL DEFAULT '0',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(255) NOT NULL DEFAULT 'unknown',
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `rev_records`;
CREATE TABLE `rev_records` (
  `rev_record_id` int(11) NOT NULL AUTO_INCREMENT,
  `rev_zone` int(11) NOT NULL DEFAULT '0',
  `host` varchar(11) NOT NULL,
  `destination` varchar(255) NOT NULL,
  `valid` varchar(255) NOT NULL DEFAULT 'unknown',
  UNIQUE KEY `rev_record_id` (`rev_record_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `rev_zones`;
CREATE TABLE `rev_zones` (
  `rev_zone_id` int(7) NOT NULL AUTO_INCREMENT,
  `net_range` varchar(255) DEFAULT NULL,
  `prefix` varchar(4) DEFAULT NULL,
  `net_start` varchar(255) DEFAULT NULL,
  `net_finish` varchar(255) DEFAULT NULL,
  `start_ip` int(4) unsigned DEFAULT NULL,
  `finish_ip` int(4) unsigned DEFAULT NULL,
  `pri_dns` varchar(255) DEFAULT NULL,
  `sec_dns` varchar(255) DEFAULT NULL,
  `serial` int(11) NOT NULL DEFAULT '0',
  `refresh` int(11) NOT NULL DEFAULT '604800',
  `retry` int(11) NOT NULL DEFAULT '86400',
  `expire` int(11) NOT NULL DEFAULT '2419200',
  `ttl` int(11) NOT NULL DEFAULT '604800',
  `valid` varchar(255) NOT NULL DEFAULT 'unknown',
  `owner` int(11) NOT NULL DEFAULT '1',
  `updated` varchar(255) NOT NULL DEFAULT 'yes',
  PRIMARY KEY (`rev_zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `seed_part`;
CREATE TABLE `seed_part` (
  `part_id` int(7) NOT NULL AUTO_INCREMENT,
  `minimum` int(7) NOT NULL,
  `maximum` int(7) NOT NULL,
  `priority` int(7) NOT NULL DEFAULT '0',
  `mount_point` varchar(64) NOT NULL,
  `filesystem` varchar(16) NOT NULL,
  `server_id` int(7) NOT NULL,
  `logical_volume` varchar(16) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`part_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `seed_schemes`;
CREATE TABLE `seed_schemes` (
  `def_scheme_id` int(7) NOT NULL AUTO_INCREMENT,
  `scheme_name` varchar(79) NOT NULL,
  `lvm` smallint(4) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`def_scheme_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `server`;
CREATE TABLE `server` (
  `server_id` int(7) NOT NULL AUTO_INCREMENT,
  `vendor` varchar(50) DEFAULT NULL,
  `make` varchar(50) DEFAULT NULL,
  `model` varchar(30) DEFAULT NULL,
  `uuid` varchar(50) DEFAULT NULL,
  `cust_id` int(7) DEFAULT NULL,
  `vm_server_id` int(7) DEFAULT NULL,
  `name` varchar(30) DEFAULT NULL,
  PRIMARY KEY (`server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `services`;
CREATE TABLE `services` (
  `service_id` int(7) NOT NULL AUTO_INCREMENT,
  `server_id` int(7) DEFAULT NULL,
  `cust_id` int(7) DEFAULT NULL,
  `service_type_id` int(7) DEFAULT NULL,
  `detail` varchar(50) DEFAULT NULL,
  `url` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`service_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `service_type`;
CREATE TABLE `service_type` (
  `service_type_id` int(7) NOT NULL AUTO_INCREMENT,
  `service` varchar(20) DEFAULT NULL,
  `detail` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`service_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `admin` varchar(255) NOT NULL DEFAULT 'no',
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `varient`;
CREATE TABLE `varient` (
  `varient_id` int(7) NOT NULL AUTO_INCREMENT,
  `varient` varchar(50) DEFAULT NULL,
  `valias` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`varient_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `vm_server_hosts`;
CREATE TABLE `vm_server_hosts` (
  `vm_server_id` int(7) NOT NULL AUTO_INCREMENT,
  `vm_server` varchar(150) DEFAULT NULL,
  `type` varchar(50) DEFAULT NULL,
  `server_id` int(7) DEFAULT NULL,
  PRIMARY KEY (`vm_server_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `zones`;
CREATE TABLE `zones` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `pri_dns` varchar(255) DEFAULT NULL,
  `sec_dns` varchar(255) DEFAULT NULL,
  `serial` int(11) NOT NULL DEFAULT '0',
  `refresh` int(11) NOT NULL DEFAULT '604800',
  `retry` int(11) NOT NULL DEFAULT '86400',
  `expire` int(11) NOT NULL DEFAULT '2419200',
  `ttl` int(11) NOT NULL DEFAULT '604800',
  `valid` varchar(255) NOT NULL DEFAULT 'unknown',
  `owner` int(11) NOT NULL DEFAULT '1',
  `updated` varchar(255) NOT NULL DEFAULT 'yes',
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

