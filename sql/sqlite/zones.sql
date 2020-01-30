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
  `master` varchar(255),
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0
);
CREATE TRIGGER insert_zones AFTER INSERT ON zones
BEGIN
UPDATE zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER update_zones AFTER UPDATE ON zones
BEGIN
UPDATE zones SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
