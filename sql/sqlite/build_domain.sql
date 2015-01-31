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
CREATE TRIGGER insert_build_domain AFTER INSERT ON build_domain
BEGIN
UPDATE build_domain SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;
CREATE TRIGGER update_build_domain AFTER UPDATE ON build_domain
BEGIN
UPDATE build_domain SET mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;
