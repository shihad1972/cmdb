CREATE TABLE `varient` (
  `varient_id` INTEGER PRIMARY KEY,
  `varient` varchar(50) NOT NULL,
  `valias` varchar(20) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0);
CREATE TRIGGER insert_varient AFTER INSERT ON varient
BEGIN
UPDATE varient SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;
CREATE TRIGGER update_varient AFTER UPDATE ON varient
BEGIN
UPDATE varient SET mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;
