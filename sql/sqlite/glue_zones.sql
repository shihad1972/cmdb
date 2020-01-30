CREATE TABLE `glue_zones` (
  `id` INTEGER PRIMARY KEY,
  `zone_id` int(7) NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL,
  `pri_ns` varchar(255) NOT NULL,
  `sec_ns` varchar(255) NOT NULL DEFAULT 'none',
  `pri_dns` varchar(15) NOT NULL,
  `sec_dns` varchar(15) NOT NULL DEFAULT 'none',
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`zone_id`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_glue_zones AFTER INSERT ON glue_zones
BEGIN
UPDATE glue_zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER update_glue_zones AFTER UPDATE ON glue_zones
BEGIN
UPDATE glue_zones SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
