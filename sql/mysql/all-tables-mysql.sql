
CREATE TABLE `build_type` (
  `bt_id` int(7) NOT NULL AUTO_INCREMENT,
  `alias` varchar(25) NOT NULL,
  `build_type` varchar(25) NOT NULL DEFAULT 'none',
  `arg` varchar(15) NOT NULL DEFAULT 'none',
  `url` varchar(79) NOT NULL DEFAULT 'none',
  `mirror` varchar(255) NOT NULL DEFAULT 'none',
  `boot_line` varchar(127) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`bt_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `service_type` (
  `service_type_id` int(7) NOT NULL AUTO_INCREMENT,
  `service` varchar(15) NOT NULL,
  `detail` varchar(50) NOT NULL,
  PRIMARY KEY (`service_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `hard_type` (
  `hard_type_id` int(7) NOT NULL AUTO_INCREMENT,
  `type` varchar(50) NOT NULL DEFAULT 'none',
  `class` varchar(50) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`hard_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `customer` (
  `cust_id` int(7) NOT NULL AUTO_INCREMENT,
  `name` varchar(60) NOT NULL,
  `address` varchar(63) NOT NULL DEFAULT 'none',
  `city` varchar(31) NOT NULL DEFAULT 'none',
  `county` varchar(30) NOT NULL DEFAULT 'none',
  `postcode` varchar(10) NOT NULL DEFAULT 'none',
  `coid` varchar(8) NOT NULL,
  PRIMARY KEY (`cust_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `varient` (
  `varient_id` int(7) NOT NULL AUTO_INCREMENT,
  `varient` varchar(50) NOT NULL,
  `valias` varchar(20) NOT NULL,
  PRIMARY KEY (`varient_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `seed_schemes` (
  `def_scheme_id` int(7) NOT NULL AUTO_INCREMENT,
  `scheme_name` varchar(79) NOT NULL,
  `lvm` smallint(4) NOT NULL,
  PRIMARY KEY (`def_scheme_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `build_domain` (
  `bd_id` int(7) NOT NULL AUTO_INCREMENT,
  `start_ip` int(4) unsigned NOT NULL DEFAULT '0',
  `end_ip` int(4) unsigned NOT NULL DEFAULT '0',
  `netmask` int(4) unsigned NOT NULL DEFAULT '0',
  `gateway` int(4) unsigned NOT NULL DEFAULT '0',
  `ns` int(4) unsigned NOT NULL DEFAULT '0',
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
  `nfs_domain` varchar(79) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`bd_id`),

  INDEX (`config_ntp`, `config_ldap`, `config_log`),
  INDEX (`config_email`, `config_xymon`)

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `contacts` (
  `cont_id` int(7) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) NOT NULL,
  `phone` varchar(20) NOT NULL,
  `email` varchar(50) NOT NULL,
  `cust_id` int(7) NOT NULL,
  PRIMARY KEY (`cont_id`),

  INDEX(`cust_id`),

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer` (`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `server` (
  `server_id` int(7) NOT NULL AUTO_INCREMENT,
  `vendor` varchar(63) NOT NULL DEFAULT 'none',
  `make` varchar(63) NOT NULL DEFAULT 'none',
  `model` varchar(31) NOT NULL DEFAULT 'none',
  `uuid` varchar(63) NOT NULL DEFAULT 'none',
  `cust_id` int(7) NOT NULL,
  `vm_server_id` int(7) NOT NULL DEFAULT 0,
  `name` varchar(31) NOT NULL,
  PRIMARY KEY (`server_id`),

  INDEX(`cust_id`, `vm_server_id`),

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer` (`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `services` (
  `service_id` int(7) NOT NULL AUTO_INCREMENT,
  `server_id` int(7) NOT NULL,
  `cust_id` int(7) NOT NULL,
  `service_type_id` int(7) NOT NULL,
  `detail` varchar(63) NOT NULL DEFAULT 'none',
  `url` varchar(63) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`service_id`),

  INDEX(`server_id`, `cust_id`, `service_type_id`),

  FOREIGN KEY (`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer`(`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY (`service_type_id`)
    REFERENCES `service_type`(`service_type_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `vm_server_hosts` (
  `vm_server_id` int(7) NOT NULL AUTO_INCREMENT,
  `vm_server` varchar(127) NOT NULL,
  `type` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL,
  PRIMARY KEY (`vm_server_id`),

  INDEX (`server_id`),

  FOREIGN KEY (`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `build_ip` (
  `ip_id` int(7) NOT NULL AUTO_INCREMENT,
  `ip` int(4) unsigned NOT NULL,
  `hostname` varchar(31) NOT NULL,
  `domainname` varchar(127) NOT NULL,
  `bd_id` int(7) NOT NULL,
  `server_id` int(7) NOT NULL,
  PRIMARY KEY (`ip_id`),

  INDEX (`bd_id`, `server_id`),

  FOREIGN KEY(`bd_id`) 
    REFERENCES `build_domain`(`bd_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`server_id`),
    REfERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `build_os` (
  `os_id` int(7) NOT NULL AUTO_INCREMENT,
  `os` varchar(31) NOT NULL,
  `os_version` varchar(15) DEFAULT NULL,
  `alias` varchar(15) DEFAULT NULL,
  `ver_alias` varchar(15) NOT NULL DEFAULT 'none',
  `arch` varchar(15) DEFAULT NULL,
  `bt_id` int(7) DEFAULT NULL,
  PRIMARY KEY (`os_id`),

  INDEX (`bt_id`),

  FOREIGN KEY(`bt_id`)
    REFERENCES `build_type`(`bt_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `disk_dev` (
  `disk_id` int(7) NOT NULL AUTO_INCREMENT,
  `server_id` int(7) NOT NULL,
  `device` varchar(63) NOT NULL,
  `lvm` smallint(4) NOT NULL,
  PRIMARY KEY (`disk_id`),

  INDEX (`server_id`),

  FOREIGN KEY(`server_id`)
    REFERENCES server(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `seed_part` (
  `part_id` int(7) NOT NULL AUTO_INCREMENT,
  `minimum` int(7) NOT NULL,
  `maximum` int(7) NOT NULL,
  `priority` int(7) NOT NULL DEFAULT '0',
  `mount_point` varchar(63) NOT NULL,
  `filesystem` varchar(15) NOT NULL,
  `server_id` int(7) NOT NULL,
  `logical_volume` varchar(31) NOT NULL,
  PRIMARY KEY (`part_id`),

  INDEX (`server_id`),

  FOREIGN KEY(`server_id`)
    REFERENCES server(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `default_part` (
  `def_part_id` int(7) NOT NULL AUTO_INCREMENT,
  `minimum` int(7) NOT NULL,
  `maximum` int(7) NOT NULL,
  `priority` int(7) NOT NULL DEFAULT '0',
  `mount_point` varchar(63) NOT NULL,
  `filesystem` varchar(15) NOT NULL,
  `def_scheme_id` int(7) NOT NULL,
  `logical_volume` varchar(31) NOT NULL,
  PRIMARY KEY (`def_part_id`),

  INDEX (`def_scheme_id`),

  FOREIGN KEY(`def_scheme_id`)
    REFERENCES `seed_schemes`(`def_scheme_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `hardware` (
  `hard_id` int(7) NOT NULL AUTO_INCREMENT,
  `detail` varchar(63) NOT NULL,
  `device` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL,
  `hard_type_id` int(7) NOT NULL,
  PRIMARY KEY (`hard_id`),

  INDEX (`server_id`, `hard_type_id`),

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`hard_type_id`)
    REFERENCES `hard_type`(`hard_type_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `locale` (
  `locale_id` int(7) NOT NULL AUTO_INCREMENT,
  `locale` varchar(31) NOT NULL DEFAULT 'en_GB',
  `country` varchar(15) NOT NULL DEFAULT 'GB',
  `language` varchar(15) NOT NULL DEFAULT 'en',
  `keymap` varchar(15) NOT NULL DEFAULT 'uk',
  `os_id` int(7) NOT NULL,
  `bt_id` int(7) NOT NULL,
  `timezone` varchar(63) NOT NULL DEFAULT 'Europe/London',
  PRIMARY KEY (`locale_id`),

  INDEX (`os_id`, `bt_id`),

  FOREIGN KEY(`os_id`)
    REFERENCES `build_os`(`os_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`bt_id`)
    REFERENCES `build_type`(`bt_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `packages` (
  `pack_id` int(7) NOT NULL AUTO_INCREMENT,
  `package` varchar(63) NOT NULL DEFAULT 'none',
  `varient_id` int(7) NOT NULL DEFAULT '0',
  `os_id` int(7) NOT NULL DEFAULT '0',
  PRIMARY KEY (`pack_id`),

  INDEX (`os_id`, `varient_id`),

  FOREIGN KEY(`os_id`)
    REFERENCES `build_os`(`os_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`varient_id`)
    REFERENCES `varient`(`varient_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `build` (
  `build_id` int(7) NOT NULL AUTO_INCREMENT,
  `mac_addr` varchar(17) NOT NULL,
  `varient_id` int(7) NOT NULL,
  `net_inst_int` varchar(15) NOT NULL,
  `server_id` int(7) NOT NULL,
  `os_id` int(7) NOT NULL,
  `ip_id` int(7) NOT NULL,
  `locale_id` int(7) NOT NULL,
  `def_scheme_id` int(7) NOT NULL,
  PRIMARY KEY (`build_id`),

  INDEX (`varient_id`, `os_id`, `ip_id`),
  INDEX (`server_id`, `locale_id`, `def_scheme_id`),

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

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `options` (
  `prefkey` varchar(255) NOT NULL,
  `preftype` varchar(255) NOT NULL DEFAULT '',
  `prefval` varchar(255) DEFAULT NULL,
  UNIQUE KEY `prefkey` (`prefkey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',
  `owner` int(11) NOT NULL DEFAULT '1',
  `updated` varchar(15) NOT NULL DEFAULT 'yes',
  `type` varchar(15) NOT NULL DEFAULT 'master',
  `master` varchar(255),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `rev_zones` (
  `rev_zone_id` int(7) NOT NULL AUTO_INCREMENT,
  `net_range` varchar(255) DEFAULT NULL,
  `prefix` varchar(4) DEFAULT NULL,
  `net_start` varchar(255) DEFAULT NULL,
  `net_finish` varchar(255) DEFAULT NULL,
  `start_ip` int(7) unsigned DEFAULT NULL,
  `finish_ip` int(7) unsigned DEFAULT NULL,
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
  `master` varchar(255),
  PRIMARY KEY (`rev_zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `records` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(255) NOT NULL,
  `type` varchar(255) NOT NULL,
  `pri` int(7) NOT NULL DEFAULT '0',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',
  PRIMARY KEY (`id`),

  INDEX (`zone`),

  FOREIGN KEY (`zone`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `rev_records` (
  `rev_record_id` int(7) NOT NULL AUTO_INCREMENT,
  `rev_zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(11) NOT NULL DEFAULT 'NULL',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',
  PRIMARY KEY (`rev_record_id`),

  INDEX (`rev_zone`),

  FOREIGN KEY (`rev_zone`)
    REFERENCES `rev_zones`(`rev_zone_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `glue_zones` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `zone_id` int(7) NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL,
  `pri_ns` varchar(255) NOT NULL,
  `sec_ns` varchar(255) NOT NULL DEFAULT 'none',
  `pri_dns` varchar(15) NOT NULL,
  `sec_dns` varchar(15) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`id`),

  INDEX (`name`, `zone_id`),

  FOREIGN KEY (`zone_id`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `preferred_a` (
  `prefa_id` int(7) NOT NULL AUTO_INCREMENT,
  `ip` varchar(15) NOT NULL DEFAULT '0.0.0.0',
  `ip_addr` int(4) unsigned NOT NULL DEFAULT '0',
  `record_id` int(7) NOT NULL,
  `fqdn` varchar(255) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`prefa_id`),

  INDEX (`record_id`),

  FOREIGN KEY (`record_id`)
    REFERENCES `records`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `users` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `uid` int(7) NOT NULL,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `admin` varchar(255) NOT NULL DEFAULT 'no',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


