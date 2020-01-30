CREATE TABLE `default_part` (
  `def_part_id` INTEGER PRIMARY KEY,
  `minimum` int(7) NOT NULL,
  `maximum` int(7) NOT NULL,
  `priority` int(7) NOT NULL DEFAULT '0',
  `mount_point` varchar(63) NOT NULL,
  `filesystem` varchar(15) NOT NULL,
  `def_scheme_id` int(7) NOT NULL,
  `logical_volume` varchar(31) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`def_scheme_id`)
    REFERENCES `seed_schemes`(`def_scheme_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_default_part AFTER INSERT ON default_part
BEGIN
UPDATE default_part SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE def_part_id = new.def_part_id;
END;
CREATE TRIGGER update_default_part AFTER UPDATE ON default_part
BEGIN
UPDATE default_part SET mtime = CURRENT_TIMESTAMP WHERE def_part_id = new.def_part_id;
END;
