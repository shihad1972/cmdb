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
  `master` varchar(255),
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0
);

CREATE TRIGGER insert_rev_zones AFTER INSERT ON rev_zones
BEGIN
UPDATE rev_zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE rev_zone_id = new.rev_zone_id;
END;
CREATE TRIGGER update_rev_zones AFTER UPDATE ON rev_zones
BEGIN
UPDATE rev_zones SET mtime = CURRENT_TIMESTAMP WHERE rev_zone_id = new.rev_zone_id;
END;
