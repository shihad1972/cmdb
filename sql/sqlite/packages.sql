CREATE TABLE `packages` (
  `pack_id` INTEGER PRIMARY KEY,
  `package` varchar(63) NOT NULL DEFAULT 'none',
  `varient_id` int(7) NOT NULL DEFAULT '0',
  `os_id` int(7) NOT NULL DEFAULT '0',
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`os_id`)
    REFERENCES `build_os`(`os_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`varient_id`)
    REFERENCES `varient`(`varient_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_packages AFTER INSERT ON packages
BEGIN
UPDATE packages SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE pack_id = new.pack_id;
END;
CREATE TRIGGER update_packages AFTER UPDATE ON packages
BEGIN
UPDATE packages SET mtime = CURRENT_TIMESTAMP WHERE pack_id = new.pack_id;
END;
