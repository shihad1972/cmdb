CREATE TABLE `hardware` (
  `hard_id` INTEGER PRIMARY KEY,
  `detail` varchar(63) NOT NULL,
  `device` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL,
  `hard_type_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`hard_type_id`)
    REFERENCES `hard_type`(`hard_type_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_hardware AFTER INSERT ON hardware
BEGIN
UPDATE hardware SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE hard_id = new.hard_id;
END;
CREATE TRIGGER update_hardware AFTER UPDATE ON hardware
BEGIN
UPDATE hardware SET mtime = CURRENT_TIMESTAMP WHERE hard_id = new.hard_id;
END;
