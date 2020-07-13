CREATE TABLE `default_varient` (
  `restrict` int(7) UNIQUE DEFAULT 0 CHECK(restrict = 0),
  `varient_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`varient_id`)
    REFERENCES `varient` (`varient_id`)
    ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE TRIGGER insert_default_varient AFTER INSERT ON default_varient
BEGIN
UPDATE default_varient SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;
CREATE TRIGGER update_default_varient AFTER UPDATE ON default_varient
BEGIN
UPDATE default_varient SET mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;
