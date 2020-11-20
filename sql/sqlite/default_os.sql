CREATE TABLE `default_os` (
  `restrict` int(7) UNIQUE DEFAULT 0 CHECK(restrict = 0),
  `os_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`os_id`)
    REFERENCES `build_os` (`os_id`)
    ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE TRIGGER insert_default_os AFTER INSERT ON default_os
BEGIN
UPDATE default_os SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;
CREATE TRIGGER update_default_os AFTER UPDATE ON default_os
BEGIN
UPDATE default_os SET mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;
