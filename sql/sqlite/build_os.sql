CREATE TABLE `build_os` (
  `os_id` INTEGER PRIMARY KEY,
  `os` varchar(31) NOT NULL,
  `os_version` varchar(15) DEFAULT NULL,
  `alias` varchar(15) DEFAULT NULL,
  `ver_alias` varchar(15) NOT NULL DEFAULT 'none',
  `arch` varchar(15) DEFAULT NULL,
  `bt_id` int(7) DEFAULT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`bt_id`)
    REFERENCES `build_type`(`bt_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_build_os AFTER INSERT ON build_os
BEGIN
UPDATE build_os SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;
CREATE TRIGGER update_build_os AFTER UPDATE ON build_os
BEGIN
UPDATE build_os SET mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;
