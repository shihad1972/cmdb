CREATE TABLE `build_ip` (
  `ip_id` INTEGER PRIMARY KEY,
  `ip` UNSIGNED INTEGER NOT NULL,
  `hostname` varchar(63) NOT NULL,
  `domainname` varchar(255) NOT NULL,
  `bd_id` int(7) NOT NULL,
  `server_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`bd_id`)
    REFERENCES `build_domain`(`bd_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_build_ip AFTER INSERT ON build_ip
BEGIN
UPDATE build_ip SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE ip_id = new.ip_id;
END;
CREATE TRIGGER update_build_ip AFTER UPDATE ON build_ip
BEGIN
UPDATE build_ip SET mtime = CURRENT_TIMESTAMP WHERE ip_id = new.ip_id;
END;
