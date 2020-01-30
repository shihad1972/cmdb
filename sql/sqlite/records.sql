CREATE TABLE `records` (
  `id` INTEGER PRIMARY KEY,
  `zone` int(7) NOT NULL DEFAULT '0',
  `host` varchar(255) NOT NULL,
  `type` varchar(255) NOT NULL,
  `protocol` varchar(15),
  `service` varchar(15),
  `pri` int(7) NOT NULL DEFAULT '0',
  `destination` varchar(255) NOT NULL,
  `valid` varchar(15) NOT NULL DEFAULT 'unknown',
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`zone`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_records AFTER INSERT ON records
BEGIN
UPDATE records SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
CREATE TRIGGER update_records AFTER UPDATE ON records
BEGIN
UPDATE records SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;
