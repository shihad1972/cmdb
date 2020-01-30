CREATE TABLE `rev_records` (
  `rev_record_id` INTEGER PRIMARY KEY,
  `rev_zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(11) NOT NULL DEFAULT 'NULL',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`rev_zone`)
    REFERENCES `rev_zones`(`rev_zone_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_rev_records AFTER INSERT ON rev_records
BEGIN
UPDATE rev_records SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE rev_record_id = new.rev_record_id;
END;
CREATE TRIGGER update_rev_records AFTER UPDATE ON rev_records
BEGIN
UPDATE rev_records SET mtime = CURRENT_TIMESTAMP WHERE rev_record_id = new.rev_record_id;
END;
